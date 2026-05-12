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

#include "hilog_tag_wrapper.h"
#include "string_wrapper.h"
#include "want_params.h"
#include "skill/skill_execute_result.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {

const int32_t TEST_CODE = 0;
const int32_t TEST_ERROR_CODE = -1;
const uint32_t TEST_FLAGS = 1;
const std::string TEST_URI = "file://test/doc.txt";

}  // namespace

class SkillExecuteResultTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SkillExecuteResultTest::SetUpTestCase(void) {}
void SkillExecuteResultTest::TearDownTestCase(void) {}
void SkillExecuteResultTest::SetUp() {}
void SkillExecuteResultTest::TearDown() {}

/**
 * @tc.name: SkillExecuteResult_Marshalling_0100
 * @tc.desc: Test Marshalling with default values
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteResultTest, SkillExecuteResult_Marshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteResult_Marshalling_0100 start");

    SkillExecuteResult src;
    src.code = TEST_CODE;
    src.result = nullptr;
    src.flags = 0;

    Parcel parcel;
    bool ret = src.Marshalling(parcel);
    EXPECT_TRUE(ret);

    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteResult_Marshalling_0100 end");
}

/**
 * @tc.name: SkillExecuteResult_Unmarshalling_0100
 * @tc.desc: Test Unmarshalling with code and empty result
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteResultTest, SkillExecuteResult_Unmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteResult_Unmarshalling_0100 start");

    SkillExecuteResult src;
    src.code = TEST_CODE;
    src.result = std::make_shared<AAFwk::WantParams>();
    src.flags = 0;

    Parcel parcel;
    bool ret = src.Marshalling(parcel);
    EXPECT_TRUE(ret);

    auto *dest = SkillExecuteResult::Unmarshalling(parcel);
    ASSERT_NE(dest, nullptr);
    EXPECT_EQ(dest->code, TEST_CODE);
    EXPECT_NE(dest->result, nullptr);
    EXPECT_TRUE(dest->uris.empty());
    EXPECT_EQ(dest->flags, 0u);

    delete dest;
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteResult_Unmarshalling_0100 end");
}

/**
 * @tc.name: SkillExecuteResult_Unmarshalling_0200
 * @tc.desc: Test Unmarshalling with uris and flags
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteResultTest, SkillExecuteResult_Unmarshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteResult_Unmarshalling_0200 start");

    SkillExecuteResult src;
    src.code = TEST_ERROR_CODE;
    src.result = std::make_shared<AAFwk::WantParams>();
    src.uris.push_back(TEST_URI);
    src.uris.push_back("file://test/image.png");
    src.flags = TEST_FLAGS;

    Parcel parcel;
    bool ret = src.Marshalling(parcel);
    EXPECT_TRUE(ret);

    auto *dest = SkillExecuteResult::Unmarshalling(parcel);
    ASSERT_NE(dest, nullptr);
    EXPECT_EQ(dest->code, TEST_ERROR_CODE);
    EXPECT_EQ(dest->uris.size(), 2u);
    EXPECT_EQ(dest->uris[0], TEST_URI);
    EXPECT_EQ(dest->uris[1], "file://test/image.png");
    EXPECT_EQ(dest->flags, TEST_FLAGS);

    delete dest;
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteResult_Unmarshalling_0200 end");
}

/**
 * @tc.name: SkillExecuteResult_Unmarshalling_0300
 * @tc.desc: Test Unmarshalling with result containing string params
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteResultTest, SkillExecuteResult_Unmarshalling_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteResult_Unmarshalling_0300 start");

    SkillExecuteResult src;
    src.code = TEST_CODE;
    src.result = std::make_shared<AAFwk::WantParams>();
    src.result->SetParam("answer", AAFwk::String::Box("done"));
    src.result->SetParam("count", AAFwk::Integer::Box(42));
    src.flags = 0;

    Parcel parcel;
    bool ret = src.Marshalling(parcel);
    EXPECT_TRUE(ret);

    auto *dest = SkillExecuteResult::Unmarshalling(parcel);
    ASSERT_NE(dest, nullptr);
    EXPECT_EQ(dest->code, TEST_CODE);
    ASSERT_NE(dest->result, nullptr);
    EXPECT_EQ(dest->result->GetStringParam("answer"), "done");
    EXPECT_EQ(dest->result->GetIntParam("count"), 42);

    delete dest;
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteResult_Unmarshalling_0300 end");
}

/**
 * @tc.name: SkillExecuteResult_Marshalling_0200
 * @tc.desc: Test Marshalling/Unmarshalling round-trip with full data
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteResultTest, SkillExecuteResult_Marshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteResult_Marshalling_0200 start");

    SkillExecuteResult src;
    src.code = 100;
    src.result = std::make_shared<AAFwk::WantParams>();
    src.result->SetParam("key1", AAFwk::String::Box("value1"));
    src.uris = {"uri1", "uri2", "uri3"};
    src.flags = 0xABCD;

    Parcel parcel;
    ASSERT_TRUE(src.Marshalling(parcel));

    auto *dest = SkillExecuteResult::Unmarshalling(parcel);
    ASSERT_NE(dest, nullptr);
    EXPECT_EQ(dest->code, 100);
    ASSERT_NE(dest->result, nullptr);
    EXPECT_EQ(dest->result->GetStringParam("key1"), "value1");
    EXPECT_EQ(dest->uris.size(), 3u);
    EXPECT_EQ(dest->flags, 0xABCDu);

    delete dest;
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteResult_Marshalling_0200 end");
}

}  // namespace AppExecFwk
}  // namespace OHOS
