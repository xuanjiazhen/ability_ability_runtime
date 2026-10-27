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

#include "ani_common_execute_param.h"
#include "mock_ani_env.h"
#include "string_wrapper.h"

using namespace testing::ext;

namespace OHOS::AbilityRuntime {
namespace {
constexpr const char *BUNDLE_NAME = "com.example.intent";
constexpr const char *VERIFY_BUNDLE_NAME = "com.example.caller";
}

class EtsInsightIntentVerifyCallerTest : public testing::Test {
protected:
    void SetUp() override
    {
        param_ = env_.NewObject();
        auto &properties = MockInsightIntentAniEnv::Get(param_).properties;
        properties["bundleName"] = env_.NewString(BUNDLE_NAME);
        properties["moduleName"] = env_.NewString("entry");
        properties["abilityName"] = env_.NewString("IntentAbility");
        properties["insightIntentName"] = env_.NewString("QueryWeather");
        properties["deviceId"] = env_.NewString("device-001");
        auto mode = env_.NewObject();
        MockInsightIntentAniEnv::Get(mode).numbers["value"] = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
        properties["executeMode"] = mode;
        auto record = env_.NewObject();
        MockInsightIntentAniEnv::Get(record).wantParams.SetParam("city", AAFwk::String::Box("Shanghai"));
        properties["insightIntentParam"] = record;
    }

    void SetVerifyCallerBundleName(ani_ref value)
    {
        MockInsightIntentAniEnv::Get(param_).properties["verifyCallerBundleName"] = value;
    }

    void ExpectExistingParams(const AppExecFwk::InsightIntentExecuteParam &param)
    {
        EXPECT_EQ(param.bundleName_, BUNDLE_NAME);
        EXPECT_EQ(param.moduleName_, "entry");
        EXPECT_EQ(param.abilityName_, "IntentAbility");
        EXPECT_EQ(param.insightIntentName_, "QueryWeather");
        EXPECT_EQ(param.deviceId_, "device-001");
        EXPECT_EQ(param.executeMode_, AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND);
        ASSERT_NE(param.insightIntentParam_, nullptr);
        EXPECT_EQ(param.insightIntentParam_->GetStringParam("city"), "Shanghai");
    }

    MockInsightIntentAniEnv env_;
    ani_object param_ = nullptr;
};

/**
 * @tc.name: UnwrapVerifyCallerBundleName_0100
 * @tc.desc: A non-empty declared bundle name is injected into the reserved want param key.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentVerifyCallerTest, UnwrapVerifyCallerBundleName_0100, TestSize.Level1)
{
    SetVerifyCallerBundleName(env_.NewString(VERIFY_BUNDLE_NAME));
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    ASSERT_NE(param.insightIntentParam_, nullptr);
    EXPECT_TRUE(param.insightIntentParam_->HasParam(AppExecFwk::INSIGHT_INTENT_VERIFY_CALLER_BUNDLE_NAME));
    EXPECT_EQ(param.insightIntentParam_->GetStringParam(AppExecFwk::INSIGHT_INTENT_VERIFY_CALLER_BUNDLE_NAME),
        VERIFY_BUNDLE_NAME);
    ExpectExistingParams(param);
}

/**
 * @tc.name: UnwrapVerifyCallerBundleName_0200
 * @tc.desc: An empty string is treated as unspecified and injects no key.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentVerifyCallerTest, UnwrapVerifyCallerBundleName_0200, TestSize.Level1)
{
    SetVerifyCallerBundleName(env_.NewString(""));
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    ASSERT_NE(param.insightIntentParam_, nullptr);
    EXPECT_FALSE(param.insightIntentParam_->HasParam(AppExecFwk::INSIGHT_INTENT_VERIFY_CALLER_BUNDLE_NAME));
    ExpectExistingParams(param);
}

/**
 * @tc.name: UnwrapVerifyCallerBundleName_0300
 * @tc.desc: Legacy input without the property still parses successfully and injects no key.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentVerifyCallerTest, UnwrapVerifyCallerBundleName_0300, TestSize.Level1)
{
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    ASSERT_NE(param.insightIntentParam_, nullptr);
    EXPECT_FALSE(param.insightIntentParam_->HasParam(AppExecFwk::INSIGHT_INTENT_VERIFY_CALLER_BUNDLE_NAME));
    ExpectExistingParams(param);
}

/**
 * @tc.name: UnwrapVerifyCallerBundleName_0400
 * @tc.desc: A non-string value is ignored, matching the optional-field behavior of toolCallId.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentVerifyCallerTest, UnwrapVerifyCallerBundleName_0400, TestSize.Level1)
{
    SetVerifyCallerBundleName(env_.NewObject());
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    ASSERT_NE(param.insightIntentParam_, nullptr);
    EXPECT_FALSE(param.insightIntentParam_->HasParam(AppExecFwk::INSIGHT_INTENT_VERIFY_CALLER_BUNDLE_NAME));
    ExpectExistingParams(param);
}

/**
 * @tc.name: UnwrapVerifyCallerBundleName_0500
 * @tc.desc: The property does not shadow a same-named business key inside insightIntentParam.
 * @tc.type: FUNC
 */
HWTEST_F(EtsInsightIntentVerifyCallerTest, UnwrapVerifyCallerBundleName_0500, TestSize.Level1)
{
    auto &record = MockInsightIntentAniEnv::Get(param_).properties.at("insightIntentParam");
    MockInsightIntentAniEnv::Get(record).wantParams.SetParam(
        AppExecFwk::INSIGHT_INTENT_VERIFY_CALLER_BUNDLE_NAME, AAFwk::String::Box("business-value"));
    SetVerifyCallerBundleName(env_.NewString(VERIFY_BUNDLE_NAME));
    AppExecFwk::InsightIntentExecuteParam param;
    ASSERT_TRUE(UnwrapExecuteParam(&env_, param_, param));
    ASSERT_NE(param.insightIntentParam_, nullptr);
    EXPECT_EQ(param.insightIntentParam_->GetStringParam(AppExecFwk::INSIGHT_INTENT_VERIFY_CALLER_BUNDLE_NAME),
        VERIFY_BUNDLE_NAME);
    ExpectExistingParams(param);
}
} // namespace OHOS::AbilityRuntime
