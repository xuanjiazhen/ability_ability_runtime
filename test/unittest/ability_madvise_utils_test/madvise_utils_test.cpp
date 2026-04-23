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
 * @tc.number: GetReleasedRegions_0100
 * @tc.desc: Test GetReleasedRegions and ClearReleasedRegions
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, GetReleasedRegions_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "GetReleasedRegions_0100 start.");

    MadviseUtil::ClearReleasedRegions();
    auto& regions = MadviseUtil::GetReleasedRegions();
    EXPECT_EQ(regions.size(), 0);
    MadviseUtil::ClearReleasedRegions();
    EXPECT_EQ(regions.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "GetReleasedRegions_0100 end.");
}

/**
 * @tc.number: DetectReloadedPages_0100
 * @tc.desc: Test DetectReloadedPages returns empty when no regions released
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, DetectReloadedPages_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "DetectReloadedPages_0100 start.");

    MadviseUtil::ClearReleasedRegions();
    auto result = MadviseUtil::DetectReloadedPages();
    EXPECT_EQ(result.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "DetectReloadedPages_0100 end.");
}

/**
 * @tc.number: MadviseFilePagesAtOffsets_0100
 * @tc.desc: Test MadviseFilePagesAtOffsets with empty filename
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseFilePagesAtOffsets_0100, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MadviseFilePagesAtOffsets_0100 start.");

    std::vector<uint64_t> offsets = {0x1000};
    int32_t result = MadviseUtil::MadviseFilePagesAtOffsets("", offsets);
    EXPECT_EQ(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "MadviseFilePagesAtOffsets_0100 end.");
}

/**
 * @tc.number: MadviseFilePagesAtOffsets_0200
 * @tc.desc: Test MadviseFilePagesAtOffsets with empty offsets
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseFilePagesAtOffsets_0200, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MadviseFilePagesAtOffsets_0200 start.");

    std::vector<uint64_t> offsets;
    int32_t result = MadviseUtil::MadviseFilePagesAtOffsets("test.so", offsets);
    EXPECT_EQ(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "MadviseFilePagesAtOffsets_0200 end.");
}

/**
 * @tc.number: MadviseFilePagesAtOffsets_0300
 * @tc.desc: Test MadviseFilePagesAtOffsets with nonexistent file
 * @tc.type: FUNC
 */
HWTEST_F(MadviseUtilsTest, MadviseFilePagesAtOffsets_0300, TestSize.Level2)
{
    TAG_LOGI(AAFwkTag::TEST, "MadviseFilePagesAtOffsets_0300 start.");

    std::vector<uint64_t> offsets = {0x1000, 0x2000};
    int32_t result = MadviseUtil::MadviseFilePagesAtOffsets("nonexistent_test.so", offsets);
    EXPECT_EQ(result, 0);

    TAG_LOGI(AAFwkTag::TEST, "MadviseFilePagesAtOffsets_0300 end.");
}
}  // namespace AbilityRuntime
}  // namespace OHOS
