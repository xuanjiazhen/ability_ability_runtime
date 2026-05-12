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

}  // namespace AppExecFwk
}  // namespace OHOS
