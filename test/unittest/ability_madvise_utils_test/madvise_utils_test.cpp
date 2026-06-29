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
#include <gmock/gmock.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <string>

#include "hilog_tag_wrapper.h"
#include "madvise_utils.h"

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace AbilityRuntime {
class MadviseUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MadviseUtilsTest::SetUpTestCase(void)
{}

void MadviseUtilsTest::TearDownTestCase(void)
{}

void MadviseUtilsTest::SetUp()
{}

void MadviseUtilsTest::TearDown()
{}

/**
 * @tc.number: MadviseSingleLibrary_0100
 * @tc.desc: Test MadviseSingleLibrary works
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseSingleLibrary_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MadviseSingleLibrary_0100 start.");

    const char* libName = "";
    bool result = MadviseUtil::MadviseSingleLibrary(libName);
    EXPECT_EQ(result, false);

    TAG_LOGI(AAFwkTag::TEST, "MadviseSingleLibrary_0100 end.");
}

/**
 * @tc.number: MadviseSingleLibrary_0200
 * @tc.desc: Test MadviseSingleLibrary works
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseSingleLibrary_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MadviseSingleLibrary_0200 start.");

    const char* libName = "testtest.so";
    bool result = MadviseUtil::MadviseSingleLibrary(libName);
    EXPECT_EQ(result, false);

    TAG_LOGI(AAFwkTag::TEST, "MadviseSingleLibrary_0200 end.");
}

/**
 * @tc.number: MadviseGeneralFiles_0100
 * @tc.desc: Test MadviseGeneralFiles works
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseGeneralFiles_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MadviseGeneralFiles_0100 start.");

    std::vector<std::string> filenames ;
    int32_t result = MadviseUtil::MadviseGeneralFiles(filenames);
    EXPECT_EQ(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "MadviseGeneralFiles_0100 end.");
}

/**
 * @tc.number: MadviseGeneralFiles_0200
 * @tc.desc: Test MadviseGeneralFiles works
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseGeneralFiles_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MadviseGeneralFiles_0200 start.");

    std::vector<std::string> filenames = { "testtest.hsp" };
    int32_t result = MadviseUtil::MadviseGeneralFiles(filenames);
    EXPECT_EQ(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "MadviseGeneralFiles_0200 end.");
}

/**
 * @tc.number: MadviseWithConfigFile_0100
 * @tc.desc: Test MadviseWithConfigFile works
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseWithConfigFile_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MadviseWithConfigFile_0100 start.");

    const char* bundleName = "";
    int32_t result = MadviseUtil::MadviseWithConfigFile(bundleName);
    EXPECT_EQ(result, -1);

    TAG_LOGI(AAFwkTag::TEST, "MadviseWithConfigFile_0100 end.");
}

/**
 * @tc.number: MadviseFilePagesAtOffsets_0100
 * @tc.desc: Empty inputs return 0 and touch nothing.
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseFilePagesAtOffsets_0100, TestSize.Level2)
{
    EXPECT_EQ(MadviseUtil::MadviseFilePagesAtOffsets("", {}), 0);
    EXPECT_EQ(MadviseUtil::MadviseFilePagesAtOffsets("no_such_file.hap", {{0, 1}}), 0);
}

/**
 * @tc.number: MadviseFilePagesAtOffsets_0200
 * @tc.desc: For a real mmap'd temp file, releasing a sub-span succeeds and
 *           returns a non-negative count; offset parsing must be honored so
 *           that a span outside the mapping releases nothing.
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseFilePagesAtOffsets_0200, TestSize.Level2)
{
    char path[] = "/tmp/madvise_ut_XXXXXX";
    int fd = mkstemp(path);
    ASSERT_GE(fd, 0);
    long page = sysconf(_SC_PAGESIZE);
    size_t fileSize = static_cast<size_t>(page) * 4; // 4 pages
    std::vector<uint8_t> buf(fileSize, 0xAB);
    ASSERT_EQ(write(fd, buf.data(), fileSize), static_cast<ssize_t>(fileSize));
    void* map = mmap(nullptr, fileSize, PROT_READ, MAP_SHARED, fd, 0);
    ASSERT_NE(map, MAP_FAILED);
    // touch pages so they are resident before release
    volatile uint8_t v = 0;
    for (size_t i = 0; i < fileSize; i += static_cast<size_t>(page)) {
        v |= reinterpret_cast<uint8_t*>(map)[i];
    }
    (void)v;

    std::string baseName = std::strrchr(path, '/') + 1;
    // span inside the mapping -> should release something
    MadviseUtil::FileOffsetSpan inSpan{static_cast<uint64_t>(page),
                                       static_cast<uint64_t>(page) * 3};
    int32_t released = MadviseUtil::MadviseFilePagesAtOffsets(baseName, {inSpan});
    EXPECT_GE(released, 0);

    // span entirely beyond the mapping -> must release nothing
    MadviseUtil::FileOffsetSpan outSpan{fileSize + static_cast<uint64_t>(page),
                                        fileSize + static_cast<uint64_t>(page) * 2};
    EXPECT_EQ(MadviseUtil::MadviseFilePagesAtOffsets(baseName, {outSpan}), 0);

    munmap(map, fileSize);
    close(fd);
    unlink(path);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
