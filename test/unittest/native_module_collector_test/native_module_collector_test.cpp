/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/stat.h>

#include "native_module_collector.h"

using namespace testing;
using namespace testing::ext;
using OHOS::AppExecFwk::NativeModuleCollector;

namespace {
constexpr mode_t STORE_FILE_MODE = 0600;

std::string TempBaseDir(const std::string& caseName)
{
    return (std::filesystem::temp_directory_path() / ("native_module_collector_" + caseName)).string();
}

void RemoveDirRecursively(const std::string& dir)
{
    std::error_code ec;
    std::filesystem::remove_all(dir, ec);
}

mode_t FilePermission(const std::string& path)
{
    struct stat st {};
    if (stat(path.c_str(), &st) != 0) {
        return 0;
    }
    return st.st_mode & 0777;
}
} // namespace

class NativeModuleCollectorTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: GetStorePath_Format_001
 * @tc.desc: store path uses hidden dot dir, system namespace prefix, 8-char hex hash and version suffix
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, GetStorePath_Format_001, TestSize.Level1)
{
    std::string path = NativeModuleCollector::GetStorePath("/data/storage/el2/base/cache", "com.example.app");

    const std::string expectedPrefix =
        "/data/storage/el2/base/cache/.ohos.ability_runtime/native_modules_";
    EXPECT_EQ(path.compare(0, expectedPrefix.size(), expectedPrefix), 0);

    ASSERT_TRUE(path.size() > expectedPrefix.size());
    std::string tail = path.substr(expectedPrefix.size());
    const std::string suffix = ".v2.json";
    ASSERT_TRUE(tail.size() > suffix.size());
    EXPECT_EQ(tail.compare(tail.size() - suffix.size(), suffix.size(), suffix), 0);

    std::string hash = tail.substr(0, tail.size() - suffix.size());
    EXPECT_EQ(hash.size(), 8u);
    for (char c : hash) {
        EXPECT_TRUE((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f')) << "hash char not lowercase hex: " << c;
    }
}

/**
 * @tc.name: ProcessNameHash_StableAndDistinct_002
 * @tc.desc: same process name yields stable hash; different process names yield different hashes
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, ProcessNameHash_StableAndDistinct_002, TestSize.Level1)
{
    std::string main1 = NativeModuleCollector::ProcessNameHash("com.example.app");
    std::string main2 = NativeModuleCollector::ProcessNameHash("com.example.app");
    std::string remote = NativeModuleCollector::ProcessNameHash("com.example.app:remote");

    EXPECT_EQ(main1, main2);
    EXPECT_NE(main1, remote);

    std::string mainPath = NativeModuleCollector::GetStorePath("/cache", "com.example.app");
    std::string remotePath = NativeModuleCollector::GetStorePath("/cache", "com.example.app:remote");
    EXPECT_NE(mainPath, remotePath);
}

/**
 * @tc.name: EnsureDir_CreatesNestedAndIdempotent_003
 * @tc.desc: EnsureDir creates nested directories, is idempotent, rejects empty input
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, EnsureDir_CreatesNestedAndIdempotent_003, TestSize.Level1)
{
    std::string base = TempBaseDir("ensuredir");
    RemoveDirRecursively(base);

    std::string nested = base + "/a/b/.ohos.ability_runtime";
    EXPECT_TRUE(NativeModuleCollector::EnsureDir(nested));
    EXPECT_TRUE(std::filesystem::is_directory(nested));

    // second call meets EEXIST on every level and must still succeed
    EXPECT_TRUE(NativeModuleCollector::EnsureDir(nested));

    EXPECT_FALSE(NativeModuleCollector::EnsureDir(""));

    RemoveDirRecursively(base);
}

/**
 * @tc.name: WriteFileWithPermission_ContentAndMode_004
 * @tc.desc: file content round-trips, permission is 0600 and gets tightened on rewrite
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, WriteFileWithPermission_ContentAndMode_004, TestSize.Level1)
{
    std::string base = TempBaseDir("writefile");
    RemoveDirRecursively(base);
    ASSERT_TRUE(NativeModuleCollector::EnsureDir(base));
    std::string file = base + "/native_modules_a1b2c3d4.v2.json";

    const std::string content = "{\"version\":2,\"predicted\":[]}";
    EXPECT_TRUE(NativeModuleCollector::WriteFileWithPermission(file, content));
    EXPECT_EQ(FilePermission(file), STORE_FILE_MODE);

    std::ifstream in(file);
    std::stringstream ss;
    ss << in.rdbuf();
    EXPECT_EQ(ss.str(), content);

    // loosen the mode, then rewrite: mode must be tightened back to 0600
    ASSERT_EQ(chmod(file.c_str(), 0644), 0);
    EXPECT_TRUE(NativeModuleCollector::WriteFileWithPermission(file, content));
    EXPECT_EQ(FilePermission(file), STORE_FILE_MODE);

    // write into a missing directory must fail, not crash
    EXPECT_FALSE(NativeModuleCollector::WriteFileWithPermission(base + "/no/such/dir/f.json", content));

    RemoveDirRecursively(base);
}

/**
 * @tc.name: IsEnabled_DefaultDisabledNoFileIO_005
 * @tc.desc: switch placeholder is disabled; checkpoint stubs perform no file I/O
 * @tc.type: FUNC
 */
HWTEST_F(NativeModuleCollectorTest, IsEnabled_DefaultDisabledNoFileIO_005, TestSize.Level1)
{
    EXPECT_FALSE(NativeModuleCollector::IsEnabled());

    std::string base = TempBaseDir("disabled");
    RemoveDirRecursively(base);
    ASSERT_TRUE(NativeModuleCollector::EnsureDir(base));

    NativeModuleCollector::SampleAtAppReady();
    NativeModuleCollector::CollectMergeAndSave();

    EXPECT_TRUE(std::filesystem::is_empty(base));

    RemoveDirRecursively(base);
}
