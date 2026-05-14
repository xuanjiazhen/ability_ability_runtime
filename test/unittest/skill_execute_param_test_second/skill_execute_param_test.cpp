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
#include "integer_wrapper.h"
#include "string_wrapper.h"
#include "want.h"
#include "want_params.h"
#include "skill/skill_execute_param.h"

using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {

const std::string BUNDLE_NAME = "com.example.test";
const std::string MODULE_NAME = "entry";
const std::string SKILL_NAME = "testSkill";
const std::string SCRIPT_PATH = "ets/skill/TestSkill";
const std::string FUNC_NAME = "handleSkill";
const std::string REQUEST_CODE = "req_123";
const std::string HAP_PATH = "/data/app/test.hap";

}  // namespace

class SkillExecuteParamTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void SkillExecuteParamTest::SetUpTestCase(void) {}
void SkillExecuteParamTest::TearDownTestCase(void) {}
void SkillExecuteParamTest::SetUp() {}
void SkillExecuteParamTest::TearDown() {}

/**
 * @tc.name: SkillExecuteParam_Marshalling_0100
 * @tc.desc: Test Marshalling with all fields populated
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SkillExecuteParam_Marshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_Marshalling_0100 start");

    SkillExecuteParam src;
    src.bundleName_ = BUNDLE_NAME;
    src.moduleName_ = MODULE_NAME;
    src.skillName_ = SKILL_NAME;
    src.scriptPath_ = SCRIPT_PATH;
    src.functionName_ = FUNC_NAME;
    src.skillArgs_ = std::make_shared<AAFwk::WantParams>();
    src.skillArgs_->SetParam("param1", AAFwk::String::Box("hello"));
    src.skillArgs_->SetParam("param2", AAFwk::Integer::Box(42));
    src.srcEntries_ = {"ets/skill/TestSkill.ets"};
    src.requestCode_ = REQUEST_CODE;
    src.hapPath_ = HAP_PATH;

    Parcel parcel;
    bool ret = src.Marshalling(parcel);
    EXPECT_TRUE(ret);

    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_Marshalling_0100 end");
}

/**
 * @tc.name: SkillExecuteParam_Unmarshalling_0100
 * @tc.desc: Test Unmarshalling round-trip with all fields
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SkillExecuteParam_Unmarshalling_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_Unmarshalling_0100 start");

    SkillExecuteParam src;
    src.bundleName_ = BUNDLE_NAME;
    src.moduleName_ = MODULE_NAME;
    src.skillName_ = SKILL_NAME;
    src.scriptPath_ = SCRIPT_PATH;
    src.functionName_ = FUNC_NAME;
    src.skillArgs_ = std::make_shared<AAFwk::WantParams>();
    src.srcEntries_ = {"ets/skill/TestSkill.ets", "ets/skill/Common.ets"};
    src.requestCode_ = REQUEST_CODE;
    src.hapPath_ = HAP_PATH;

    Parcel parcel;
    ASSERT_TRUE(src.Marshalling(parcel));

    auto *dest = SkillExecuteParam::Unmarshalling(parcel);
    ASSERT_NE(dest, nullptr);
    EXPECT_EQ(dest->bundleName_, BUNDLE_NAME);
    EXPECT_EQ(dest->moduleName_, MODULE_NAME);
    EXPECT_EQ(dest->skillName_, SKILL_NAME);
    EXPECT_EQ(dest->scriptPath_, SCRIPT_PATH);
    EXPECT_EQ(dest->functionName_, FUNC_NAME);
    EXPECT_NE(dest->skillArgs_, nullptr);
    EXPECT_EQ(dest->srcEntries_.size(), 2u);
    EXPECT_EQ(dest->requestCode_, REQUEST_CODE);
    EXPECT_EQ(dest->hapPath_, HAP_PATH);

    delete dest;
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_Unmarshalling_0100 end");
}

/**
 * @tc.name: SkillExecuteParam_Unmarshalling_0200
 * @tc.desc: Test Unmarshalling with empty optional fields
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SkillExecuteParam_Unmarshalling_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_Unmarshalling_0200 start");

    SkillExecuteParam src;
    src.bundleName_ = BUNDLE_NAME;
    src.moduleName_ = MODULE_NAME;
    src.skillName_ = SKILL_NAME;
    src.scriptPath_ = "";
    src.functionName_ = "";
    src.skillArgs_ = nullptr;
    src.srcEntries_ = {};
    src.requestCode_ = "";
    src.hapPath_ = "";

    Parcel parcel;
    ASSERT_TRUE(src.Marshalling(parcel));

    auto *dest = SkillExecuteParam::Unmarshalling(parcel);
    ASSERT_NE(dest, nullptr);
    EXPECT_EQ(dest->scriptPath_, "");
    EXPECT_EQ(dest->functionName_, "");
    EXPECT_NE(dest->skillArgs_, nullptr);
    EXPECT_TRUE(dest->srcEntries_.empty());

    delete dest;
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_Unmarshalling_0200 end");
}

/**
 * @tc.name: SkillExecuteParam_IsSkillExecute_0100
 * @tc.desc: Test IsSkillExecute returns true when skill param present
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SkillExecuteParam_IsSkillExecute_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_IsSkillExecute_0100 start");

    AAFwk::Want want;
    want.SetParam(SKILL_EXECUTE_PARAM_SKILL_NAME, SKILL_NAME);
    EXPECT_TRUE(SkillExecuteParam::IsSkillExecute(want));

    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_IsSkillExecute_0100 end");
}

/**
 * @tc.name: SkillExecuteParam_IsSkillExecute_0200
 * @tc.desc: Test IsSkillExecute returns false when skill param absent
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SkillExecuteParam_IsSkillExecute_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_IsSkillExecute_0200 start");

    AAFwk::Want want;
    EXPECT_FALSE(SkillExecuteParam::IsSkillExecute(want));

    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_IsSkillExecute_0200 end");
}

/**
 * @tc.name: SkillExecuteParam_WriteToWantAndGenerate_0100
 * @tc.desc: Test WriteToWant then GenerateFromWant round-trip
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SkillExecuteParam_WriteToWantAndGenerate_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_WriteToWantAndGenerate_0100 start");

    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    skillArgs->SetParam("arg1", AAFwk::String::Box("value1"));
    skillArgs->SetParam("arg2", AAFwk::Integer::Box(10));

    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, BUNDLE_NAME, MODULE_NAME, SKILL_NAME,
        SCRIPT_PATH, FUNC_NAME, skillArgs, {"ets/skill/Test.ets"}, REQUEST_CODE, HAP_PATH);

    EXPECT_TRUE(SkillExecuteParam::IsSkillExecute(want));

    SkillExecuteParam dest;
    bool ret = SkillExecuteParam::GenerateFromWant(want, dest);
    EXPECT_TRUE(ret);
    EXPECT_EQ(dest.bundleName_, BUNDLE_NAME);
    EXPECT_EQ(dest.moduleName_, MODULE_NAME);
    EXPECT_EQ(dest.skillName_, SKILL_NAME);
    EXPECT_EQ(dest.scriptPath_, SCRIPT_PATH);
    EXPECT_EQ(dest.functionName_, FUNC_NAME);
    ASSERT_NE(dest.skillArgs_, nullptr);
    EXPECT_EQ(dest.skillArgs_->GetStringParam("arg1"), "value1");
    EXPECT_EQ(dest.skillArgs_->GetIntParam("arg2"), 10);
    EXPECT_EQ(dest.srcEntries_.size(), 1u);
    EXPECT_EQ(dest.requestCode_, REQUEST_CODE);
    EXPECT_EQ(dest.hapPath_, HAP_PATH);

    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_WriteToWantAndGenerate_0100 end");
}

/**
 * @tc.name: SkillExecuteParam_WriteToWantAndGenerate_0200
 * @tc.desc: Test WriteToWant with empty optional fields
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SkillExecuteParam_WriteToWantAndGenerate_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_WriteToWantAndGenerate_0200 start");

    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, BUNDLE_NAME, MODULE_NAME, SKILL_NAME);

    SkillExecuteParam dest;
    bool ret = SkillExecuteParam::GenerateFromWant(want, dest);
    EXPECT_TRUE(ret);
    EXPECT_EQ(dest.bundleName_, BUNDLE_NAME);
    EXPECT_EQ(dest.skillName_, SKILL_NAME);
    EXPECT_EQ(dest.scriptPath_, "");
    EXPECT_EQ(dest.functionName_, "");

    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_WriteToWantAndGenerate_0200 end");
}

/**
 * @tc.name: SkillExecuteParam_RemoveSkillParam_0100
 * @tc.desc: Test RemoveSkillParam removes all skill-related params from Want
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SkillExecuteParam_RemoveSkillParam_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_RemoveSkillParam_0100 start");

    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    skillArgs->SetParam("data", AAFwk::String::Box("test"));

    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, BUNDLE_NAME, MODULE_NAME, SKILL_NAME,
        SCRIPT_PATH, FUNC_NAME, skillArgs, {"src1"}, REQUEST_CODE, HAP_PATH);

    EXPECT_TRUE(SkillExecuteParam::IsSkillExecute(want));

    bool ret = SkillExecuteParam::RemoveSkillParam(want);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(SkillExecuteParam::IsSkillExecute(want));

    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_RemoveSkillParam_0100 end");
}

/**
 * @tc.name: SkillExecuteParam_GenerateFromWant_0100
 * @tc.desc: Test GenerateFromWant returns false when no skill name
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SkillExecuteParam_GenerateFromWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_GenerateFromWant_0100 start");

    AAFwk::Want want;
    SkillExecuteParam dest;
    bool ret = SkillExecuteParam::GenerateFromWant(want, dest);
    EXPECT_FALSE(ret);

    TAG_LOGI(AAFwkTag::TEST, "SkillExecuteParam_GenerateFromWant_0100 end");
}

/**
 * @tc.name: SkillExecuteParam_WriteToWantAndGenerate_0300
 * @tc.desc: Test WriteToWant with multiple skillArgs and multiple srcEntries
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SkillExecuteParam_WriteToWantAndGenerate_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WriteToWantAndGenerate_0300 start");

    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    skillArgs->SetParam("key1", AAFwk::String::Box("val1"));
    skillArgs->SetParam("key2", AAFwk::Integer::Box(100));
    skillArgs->SetParam("key3", AAFwk::String::Box("val3"));

    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, BUNDLE_NAME, MODULE_NAME, SKILL_NAME,
        SCRIPT_PATH, FUNC_NAME, skillArgs,
        {"ets/skill/A.ets", "ets/skill/B.ets", "ets/skill/C.ets"},
        REQUEST_CODE, HAP_PATH);

    SkillExecuteParam dest;
    bool ret = SkillExecuteParam::GenerateFromWant(want, dest);
    EXPECT_TRUE(ret);
    ASSERT_NE(dest.skillArgs_, nullptr);
    EXPECT_EQ(dest.skillArgs_->GetStringParam("key1"), "val1");
    EXPECT_EQ(dest.skillArgs_->GetIntParam("key2"), 100);
    EXPECT_EQ(dest.skillArgs_->GetStringParam("key3"), "val3");
    EXPECT_EQ(dest.srcEntries_.size(), 3u);
    EXPECT_EQ(dest.srcEntries_[0], "ets/skill/A.ets");
    EXPECT_EQ(dest.srcEntries_[2], "ets/skill/C.ets");

    TAG_LOGI(AAFwkTag::TEST, "WriteToWantAndGenerate_0300 end");
}

/**
 * @tc.name: SkillExecuteParam_WriteToWantAndGenerate_0400
 * @tc.desc: Test WriteToWant with null skillArgs
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SkillExecuteParam_WriteToWantAndGenerate_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WriteToWantAndGenerate_0400 start");

    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, BUNDLE_NAME, MODULE_NAME, SKILL_NAME,
        SCRIPT_PATH, FUNC_NAME, nullptr, {}, REQUEST_CODE, HAP_PATH);

    SkillExecuteParam dest;
    bool ret = SkillExecuteParam::GenerateFromWant(want, dest);
    EXPECT_TRUE(ret);
    EXPECT_EQ(dest.bundleName_, BUNDLE_NAME);
    EXPECT_EQ(dest.skillName_, SKILL_NAME);
    // skillArgs_ should be nullptr since no args were written
    EXPECT_TRUE(dest.skillArgs_ == nullptr ||
        dest.skillArgs_->GetParams().empty());

    TAG_LOGI(AAFwkTag::TEST, "WriteToWantAndGenerate_0400 end");
}

/**
 * @tc.name: SkillExecuteParam_WriteToWantAndGenerate_0500
 * @tc.desc: Test WriteToWant with empty skillArgs
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SkillExecuteParam_WriteToWantAndGenerate_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "WriteToWantAndGenerate_0500 start");

    auto emptyArgs = std::make_shared<AAFwk::WantParams>();

    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, BUNDLE_NAME, MODULE_NAME, SKILL_NAME,
        SCRIPT_PATH, FUNC_NAME, emptyArgs, {}, REQUEST_CODE, HAP_PATH);

    SkillExecuteParam dest;
    bool ret = SkillExecuteParam::GenerateFromWant(want, dest);
    EXPECT_TRUE(ret);
    // No argsKeys should be written for empty skillArgs
    EXPECT_TRUE(dest.skillArgs_ == nullptr ||
        dest.skillArgs_->GetParams().empty());

    TAG_LOGI(AAFwkTag::TEST, "WriteToWantAndGenerate_0500 end");
}

/**
 * @tc.name: SkillExecuteParam_RemoveSkillParam_0200
 * @tc.desc: Test RemoveSkillParam with Want having only base params (no args/srcEntries)
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SkillExecuteParam_RemoveSkillParam_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveSkillParam_0200 start");

    AAFwk::Want want;
    SkillExecuteParam::WriteToWant(want, BUNDLE_NAME, MODULE_NAME, SKILL_NAME);

    bool ret = SkillExecuteParam::RemoveSkillParam(want);
    EXPECT_TRUE(ret);
    EXPECT_FALSE(SkillExecuteParam::IsSkillExecute(want));

    TAG_LOGI(AAFwkTag::TEST, "RemoveSkillParam_0200 end");
}

/**
 * @tc.name: SkillExecuteParam_ReadFromParcel_0100
 * @tc.desc: Test ReadFromParcel with null skillArgs in parcel
 * @tc.type: FUNC
 */
HWTEST_F(SkillExecuteParamTest, SkillExecuteParam_ReadFromParcel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ReadFromParcel_0100 start");

    Parcel parcel;
    parcel.WriteString16(Str8ToStr16(BUNDLE_NAME));
    parcel.WriteString16(Str8ToStr16(MODULE_NAME));
    parcel.WriteString16(Str8ToStr16(SKILL_NAME));
    parcel.WriteString16(Str8ToStr16(SCRIPT_PATH));
    parcel.WriteString16(Str8ToStr16(FUNC_NAME));
    parcel.WriteParcelable(nullptr); // null skillArgs
    parcel.WriteInt32(0); // srcCount
    parcel.WriteString16(Str8ToStr16(REQUEST_CODE));
    parcel.WriteString16(Str8ToStr16(HAP_PATH));

    SkillExecuteParam param;
    bool ret = param.ReadFromParcel(parcel);
    EXPECT_TRUE(ret);
    EXPECT_NE(param.skillArgs_, nullptr);
    EXPECT_TRUE(param.skillArgs_->GetParams().empty());

    TAG_LOGI(AAFwkTag::TEST, "ReadFromParcel_0100 end");
}

}  // namespace AppExecFwk
}  // namespace OHOS
