/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include <cstdarg>
#include <climits>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

#include "extractor.h"
#include "file_mapper.h"
#include "js_environment_impl.h"
#define private public
#define protected public
#include "js_worker.h"
#undef private
#undef protected
#include "native_engine.h"
#include "worker_info.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class JsWorkerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void TestSetGetAssetFunc(GetAssetFunc func);
    GetAssetFunc TestGetGetAssetFunc() const;

private:
    GetAssetFunc getAssetFunc_ = nullptr;
};

void JsWorkerTest::SetUpTestCase()
{}

void JsWorkerTest::TearDownTestCase()
{}

void JsWorkerTest::SetUp()
{}

void JsWorkerTest::TearDown()
{}

void JsWorkerTest::TestSetGetAssetFunc(GetAssetFunc func)
{
    getAssetFunc_ = func;
}

GetAssetFunc JsWorkerTest::TestGetGetAssetFunc() const
{
    return getAssetFunc_;
}

/**
 * @tc.name: AssetHelper_0100
 * @tc.desc: Asset helper.
 * @tc.type: FUNC
 * @tc.require: issue#I948D4
 */
HWTEST_F(JsWorkerTest, AssetHelper_0100, TestSize.Level1)
{
    std::shared_ptr<JsEnv::WorkerInfo> workerInfo = std::make_shared<JsEnv::WorkerInfo>();
    workerInfo->codePath = panda::panda_file::StringPacProtect("/data/test/codePath");
    workerInfo->packagePathStr = "/data/test/packagePath";
    workerInfo->hapPath = panda::panda_file::StringPacProtect("/data/test/hapPath");
    workerInfo->moduleName = "moduleName";
    TestSetGetAssetFunc(AssetHelper(workerInfo));

    std::string uri = "/data";
    uint8_t *buff = nullptr;
    size_t buffSize;
    std::vector<uint8_t> content;
    std::string ami;
    bool useSecureMem;
    bool isRestricted = false;
    auto func = TestGetGetAssetFunc();
    std::unique_ptr<AbilityBase::FileMapper> fileMapper = std::make_unique<AbilityBase::FileMapper>();
    void* mapper = static_cast<void*>(fileMapper.get());
    func("/data", &buff, &buffSize, content, ami, useSecureMem, &mapper, isRestricted);
    EXPECT_EQ(useSecureMem, false);
}

/**
 * @tc.name: AssetHelper_0200
 * @tc.desc: Asset helper GetSafeData.
 * @tc.type: FUNC
 * @tc.require: issue#I948D4
 */
HWTEST_F(JsWorkerTest, AssetHelper_0200, TestSize.Level1)
{
    std::shared_ptr<JsEnv::WorkerInfo> workerInfo = std::make_shared<JsEnv::WorkerInfo>();
    workerInfo->codePath = panda::panda_file::StringPacProtect("/data/test/codePath");
    workerInfo->packagePathStr = "/data/test/packagePath";
    workerInfo->hapPath = panda::panda_file::StringPacProtect("/data/test/hapPath");
    workerInfo->moduleName = "moduleName";
    AssetHelper helper = AssetHelper(workerInfo);

    FILE *fp = nullptr;
    fp = fopen("test.txt", "w+");
    ASSERT_NE(fp, nullptr);
    fclose(fp);

    uint8_t *buff = nullptr;
    size_t buffSize;
    std::unique_ptr<AbilityBase::FileMapper> fileMapper = std::make_unique<AbilityBase::FileMapper>();
    void* mapper = static_cast<void*>(fileMapper.get());
    auto ret = helper.GetSafeData("test.txt", &buff, &buffSize, &mapper);
    EXPECT_EQ(ret, false);
}

namespace {
std::string MakeTempDir()
{
    char tmp1[] = "/data/local/tmp/codepath_XXXXXX";
    char tmp2[] = "/tmp/codepath_XXXXXX";
    char tmp3[] = "codepath_XXXXXX";
    char *templates[] = {tmp1, tmp2, tmp3};
    for (auto &t : templates) {
        if (mkdtemp(t) != nullptr) {
            return std::string(t);
        }
    }
    return std::string();
}

bool MakeFile(const std::string &path)
{
    FILE *fp = fopen(path.c_str(), "w+");
    if (fp == nullptr) {
        return false;
    }
    if (fclose(fp) != 0) {
        return false;
    }
    return true;
}

void CleanupFile(const std::string &path)
{
    unlink(path.c_str());
}

void CleanupDir(const std::string &path)
{
    rmdir(path.c_str());
}

std::shared_ptr<JsEnv::WorkerInfo> MakeWorkerInfo(const std::string &codePath)
{
    auto workerInfo = std::make_shared<JsEnv::WorkerInfo>();
    workerInfo->codePath = panda::panda_file::StringPacProtect(codePath);
    workerInfo->hapPath = panda::panda_file::StringPacProtect(codePath);
    workerInfo->moduleName = "moduleName";
    return workerInfo;
}
} // namespace

/**
 * @tc.name: IsPathWithinCodePath_0100
 * @tc.desc: Resolved path inside codePath should be allowed.
 * @tc.type: FUNC
 * @tc.require: issue#I948D4
 */
HWTEST_F(JsWorkerTest, IsPathWithinCodePath_0100, TestSize.Level1)
{
    std::string codePath = MakeTempDir();
    ASSERT_FALSE(codePath.empty());
    std::string innerFile = codePath + "/module.abc";
    ASSERT_TRUE(MakeFile(innerFile));

    AssetHelper helper(MakeWorkerInfo(codePath));
    char resolved[PATH_MAX] = {0};
    ASSERT_NE(realpath(innerFile.c_str(), resolved), nullptr);
    EXPECT_EQ(helper.IsPathWithinCodePath(std::string(resolved)), true);

    CleanupFile(innerFile);
    CleanupDir(codePath);
}

/**
 * @tc.name: IsPathWithinCodePath_0200
 * @tc.desc: Resolved path outside codePath (../ traversal) should be rejected.
 * @tc.type: FUNC
 * @tc.require: issue#I948D4
 */
HWTEST_F(JsWorkerTest, IsPathWithinCodePath_0200, TestSize.Level1)
{
    std::string base = MakeTempDir();
    ASSERT_FALSE(base.empty());
    std::string codePath = base + "/codePath";
    ASSERT_EQ(mkdir(codePath.c_str(), 0700), 0);
    std::string outsideFile = base + "/outside.abc";
    ASSERT_TRUE(MakeFile(outsideFile));

    AssetHelper helper(MakeWorkerInfo(codePath));
    char codeResolved[PATH_MAX] = {0};
    ASSERT_NE(realpath(codePath.c_str(), codeResolved), nullptr);
    char resolved[PATH_MAX] = {0};
    ASSERT_NE(realpath(outsideFile.c_str(), resolved), nullptr);
    EXPECT_NE(std::string(resolved).rfind(codeResolved, 0), 0);
    EXPECT_EQ(helper.IsPathWithinCodePath(std::string(resolved)), false);

    CleanupFile(outsideFile);
    CleanupDir(codePath);
    CleanupDir(base);
}

/**
 * @tc.name: IsPathWithinCodePath_0300
 * @tc.desc: Similar-prefix sibling dir (codePathevil) must not match codePath prefix.
 * @tc.type: FUNC
 * @tc.require: issue#I948D4
 */
HWTEST_F(JsWorkerTest, IsPathWithinCodePath_0300, TestSize.Level1)
{
    std::string base = MakeTempDir();
    ASSERT_FALSE(base.empty());
    std::string codePath = base + "/codePath";
    ASSERT_EQ(mkdir(codePath.c_str(), 0700), 0);
    std::string evilDir = base + "/codePathevil";
    ASSERT_EQ(mkdir(evilDir.c_str(), 0700), 0);
    std::string evilFile = evilDir + "/file.abc";
    ASSERT_TRUE(MakeFile(evilFile));

    AssetHelper helper(MakeWorkerInfo(codePath));
    char codeResolved[PATH_MAX] = {0};
    ASSERT_NE(realpath(codePath.c_str(), codeResolved), nullptr);
    char resolved[PATH_MAX] = {0};
    ASSERT_NE(realpath(evilFile.c_str(), resolved), nullptr);
    EXPECT_NE(std::string(resolved).rfind(codeResolved, 0), 0);
    EXPECT_EQ(helper.IsPathWithinCodePath(std::string(resolved)), false);

    CleanupFile(evilFile);
    CleanupDir(evilDir);
    CleanupDir(codePath);
    CleanupDir(base);
}

/**
 * @tc.name: IsPathWithinCodePath_0400
 * @tc.desc: Resolved path equal to codePath dir itself (not a file under it) should be rejected.
 * @tc.type: FUNC
 * @tc.require: issue#I948D4
 */
HWTEST_F(JsWorkerTest, IsPathWithinCodePath_0400, TestSize.Level1)
{
    std::string codePath = MakeTempDir();
    ASSERT_FALSE(codePath.empty());

    AssetHelper helper(MakeWorkerInfo(codePath));
    char resolved[PATH_MAX] = {0};
    ASSERT_NE(realpath(codePath.c_str(), resolved), nullptr);
    EXPECT_EQ(helper.IsPathWithinCodePath(std::string(resolved)), false);

    CleanupDir(codePath);
}

/**
 * @tc.name: IsPathWithinCodePath_0500
 * @tc.desc: Empty/nonexistent codePath (realpath fails) should reject any path.
 * @tc.type: FUNC
 * @tc.require: issue#I948D4
 */
HWTEST_F(JsWorkerTest, IsPathWithinCodePath_0500, TestSize.Level1)
{
    auto workerInfo = std::make_shared<JsEnv::WorkerInfo>();
    workerInfo->codePath = panda::panda_file::StringPacProtect(std::string(""));
    AssetHelper helper(workerInfo);
    EXPECT_EQ(helper.IsPathWithinCodePath("/any/path/file.abc"), false);
}

/**
 * @tc.name: GetSafeData_SecurityCheck_0100
 * @tc.desc: GetSafeData must reject ami with ../ escaping codePath (end-to-end).
 * @tc.type: FUNC
 * @tc.require: issue#I948D4
 */
HWTEST_F(JsWorkerTest, GetSafeData_SecurityCheck_0100, TestSize.Level1)
{
    std::string base = MakeTempDir();
    ASSERT_FALSE(base.empty());
    std::string codePath = base + "/codePath";
    ASSERT_EQ(mkdir(codePath.c_str(), 0700), 0);
    ASSERT_EQ(mkdir((codePath + "/sub").c_str(), 0700), 0);
    std::string outsideFile = base + "/outside.abc";
    ASSERT_TRUE(MakeFile(outsideFile));

    AssetHelper helper(MakeWorkerInfo(codePath));
    std::string ami = codePath + "/sub/../../outside.abc";
    char amiResolved[PATH_MAX] = {0};
    ASSERT_NE(realpath(ami.c_str(), amiResolved), nullptr);
    uint8_t *buff = nullptr;
    size_t buffSize = 0;
    void *mapper = nullptr;
    EXPECT_EQ(helper.GetSafeData(ami, &buff, &buffSize, &mapper), false);
    EXPECT_EQ(buff, nullptr);

    CleanupFile(outsideFile);
    CleanupDir(codePath + "/sub");
    CleanupDir(codePath);
    CleanupDir(base);
}

/**
 * @tc.name: ReadAmiData_SecurityCheck_0100
 * @tc.desc: ReadAmiData must reject ami with ../ escaping codePath (end-to-end).
 * @tc.type: FUNC
 * @tc.require: issue#I948D4
 */
HWTEST_F(JsWorkerTest, ReadAmiData_SecurityCheck_0100, TestSize.Level1)
{
    std::string base = MakeTempDir();
    ASSERT_FALSE(base.empty());
    std::string codePath = base + "/codePath";
    ASSERT_EQ(mkdir(codePath.c_str(), 0700), 0);
    ASSERT_EQ(mkdir((codePath + "/sub").c_str(), 0700), 0);
    std::string outsideFile = base + "/outside.abc";
    ASSERT_TRUE(MakeFile(outsideFile));

    AssetHelper helper(MakeWorkerInfo(codePath));
    std::string ami = codePath + "/sub/../../outside.abc";
    char amiResolved[PATH_MAX] = {0};
    ASSERT_NE(realpath(ami.c_str(), amiResolved), nullptr);
    uint8_t *buff = nullptr;
    size_t buffSize = 0;
    std::vector<uint8_t> content;
    bool useSecureMem = false;
    void *mapper = nullptr;
    EXPECT_EQ(helper.ReadAmiData(ami, &buff, &buffSize, content, useSecureMem, false, &mapper), false);

    CleanupFile(outsideFile);
    CleanupDir(codePath + "/sub");
    CleanupDir(codePath);
    CleanupDir(base);
}
} // namespace AbilityRuntime
} // namespace OHOS
