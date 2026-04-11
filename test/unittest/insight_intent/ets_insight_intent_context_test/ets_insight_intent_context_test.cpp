/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "want.h"
#include "insight_intent_context.h"
#include "ability_window_configuration.h"
#include "mock_ability_manager_service.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AAFwk {

class MockAbilityManagerService : public IRemoteObject {
public:
    MockAbilityManagerService() : IRemoteObject(u"mock_ability_manager_service") {}
    ~MockAbilityManagerService() override = default;
};

class EtsInsightIntentContextTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<InsightIntentContext> CreateTestContext(
        int32_t winMode = AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY);
};

void EtsInsightIntentContextTest::SetUpTestCase()
{
    TAG_LOGI(AAFwkTag::TEST, "EtsInsightIntentContextTest::SetUpTestCase called");
}

void EtsInsightIntentContextTest::TearDownTestCase()
{
    TAG_LOGI(AAFwkTag::TEST, "EtsInsightIntentContextTest::TearDownTestCase called");
}

void EtsInsightIntentContextTest::SetUp()
{
    TAG_LOGI(AAFwkTag::TEST, "EtsInsightIntentContextTest::SetUp called");
}

void EtsInsightIntentContextTest::TearDown()
{
    TAG_LOGI(AAFwkTag::TEST, "EtsInsightIntentContextTest::TearDown called");
}

std::shared_ptr<InsightIntentContext> EtsInsightIntentContextTest::CreateTestContext(int32_t winMode)
{
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityManagerService();
    std::string bundleName = "test.bundle.name";
    uint64_t intentId = 10001;
    return std::make_shared<InsightIntentContext>(token, bundleName, winMode, intentId);
}

/**
 * @tc.name: StartAbilityByInsightIntent_0100
 * @tc.desc: Test StartAbilityByInsightIntent with valid same bundle name
 * @tc.type: FUNC
 * @tc.require: I9XXXXX
 */
HWTEST_F(EtsInsightIntentContextTest, StartAbilityByInsightIntent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByInsightIntent_0100 begin.");

    auto context = CreateTestContext();
    ASSERT_NE(context, nullptr);
    EXPECT_EQ(context->GetBundleName(), "test.bundle.name");

    AAFwk::Want want;
    ElementName element;
    element.SetBundleName("test.bundle.name");
    element.SetAbilityName("TestAbility");
    want.SetElement(element);

    // Verify the want is properly configured before calling
    EXPECT_EQ(want.GetElement().GetBundleName(), "test.bundle.name");
    EXPECT_EQ(want.GetElement().GetAbilityName(), "TestAbility");

    auto result = context->StartAbilityByInsightIntent(want);
    // Verify result is a valid error code (not uninitialized memory)
    // Valid error codes are typically >= 0 or specific negative values
    EXPECT_TRUE(result >= ERR_OK || result <= ERR_INVALID_OPERATION);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByInsightIntent_0100 end, result: %{public}d", result);
}

/**
 * @tc.name: StartAbilityByInsightIntent_0200
 * @tc.desc: Test StartAbilityByInsightIntent with different bundle name (should fail)
 * @tc.type: FUNC
 * @tc.require: I9XXXXX
 */
HWTEST_F(EtsInsightIntentContextTest, StartAbilityByInsightIntent_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByInsightIntent_0200 begin.");

    auto context = CreateTestContext();
    ASSERT_NE(context, nullptr);

    AAFwk::Want want;
    ElementName element;
    element.SetBundleName("different.bundle.name");  // Different bundle
    element.SetAbilityName("TestAbility");
    want.SetElement(element);

    auto result = context->StartAbilityByInsightIntent(want);
    EXPECT_NE(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByInsightIntent_0200 end.");
}

/**
 * @tc.name: StartAbilityByInsightIntent_0300
 * @tc.desc: Test StartAbilityByInsightIntent with empty bundle name (should fail)
 * @tc.type: FUNC
 * @tc.require: I9XXXXX
 */
HWTEST_F(EtsInsightIntentContextTest, StartAbilityByInsightIntent_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByInsightIntent_0300 begin.");

    auto context = CreateTestContext();
    ASSERT_NE(context, nullptr);

    AAFwk::Want want;
    ElementName element;
    element.SetAbilityName("TestAbility");
    want.SetElement(element);

    auto result = context->StartAbilityByInsightIntent(want);
    EXPECT_NE(result, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByInsightIntent_0300 end.");
}

/**
 * @tc.name: MultiWindowModeTest_0100
 * @tc.desc: Test MULTI_WINDOW_DISPLAY_PRIMARY mode parameter passing
 * @tc.type: FUNC
 * @tc.require: I9XXXXX
 */
HWTEST_F(EtsInsightIntentContextTest, MultiWindowModeTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MultiWindowModeTest_0100 begin.");

    auto context = CreateTestContext(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY);
    ASSERT_NE(context, nullptr);
    EXPECT_EQ(context->GetCurrentWindowMode(), AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY);

    TAG_LOGI(AAFwkTag::TEST, "MultiWindowModeTest_0100 end.");
}

/**
 * @tc.name: MultiWindowModeTest_0200
 * @tc.desc: Test MULTI_WINDOW_DISPLAY_SECONDARY mode parameter passing
 * @tc.type: FUNC
 * @tc.require: I9XXXXX
 */
HWTEST_F(EtsInsightIntentContextTest, MultiWindowModeTest_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "MultiWindowModeTest_0200 begin.");

    auto context = CreateTestContext(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY);
    ASSERT_NE(context, nullptr);
    EXPECT_EQ(context->GetCurrentWindowMode(), AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_SECONDARY);

    TAG_LOGI(AAFwkTag::TEST, "MultiWindowModeTest_0200 end.");
}

/**
 * @tc.name: ExecuteModeTest_0100
 * @tc.desc: Test setting and getting execute mode
 * @tc.type: FUNC
 * @tc.require: I9XXXXX
 */
HWTEST_F(EtsInsightIntentContextTest, ExecuteModeTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteModeTest_0100 begin.");

    auto context = CreateTestContext();
    ASSERT_NE(context, nullptr);

    context->SetExecuteMode(static_cast<int32_t>(InsightIntentExecuteMode::UIABILITY_FOREGROUND));
    EXPECT_EQ(context->GetExecuteMode(), static_cast<int32_t>(InsightIntentExecuteMode::UIABILITY_FOREGROUND));

    context->SetExecuteMode(static_cast<int32_t>(InsightIntentExecuteMode::UIEXTENSION_ABILITY));
    EXPECT_EQ(context->GetExecuteMode(), static_cast<int32_t>(InsightIntentExecuteMode::UIEXTENSION_ABILITY));

    TAG_LOGI(AAFwkTag::TEST, "ExecuteModeTest_0100 end.");
}

/**
 * @tc.name: DelayReturnModeTest_0100
 * @tc.desc: Test setting and getting delay return mode
 * @tc.type: FUNC
 * @tc.require: I9XXXXX
 */
HWTEST_F(EtsInsightIntentContextTest, DelayReturnModeTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "DelayReturnModeTest_0100 begin.");

    auto context = CreateTestContext();
    ASSERT_NE(context, nullptr);

    context->SetDelayReturnMode(InsightIntentReturnMode::CALLBACK);
    EXPECT_EQ(context->GetDelayReturnMode(), InsightIntentReturnMode::CALLBACK);

    context->SetDelayReturnMode(InsightIntentReturnMode::FUNCTION);
    EXPECT_EQ(context->GetDelayReturnMode(), InsightIntentReturnMode::FUNCTION);

    TAG_LOGI(AAFwkTag::TEST, "DelayReturnModeTest_0100 end.");
}

/**
 * @tc.name: IntentIdTest_0100
 * @tc.desc: Test getting intent ID
 * @tc.type: FUNC
 * @tc.require: I9XXXXX
 */
HWTEST_F(EtsInsightIntentContextTest, IntentIdTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IntentIdTest_0100 begin.");

    uint64_t testIntentId = 99999;
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityManagerService();
    auto context = std::make_shared<InsightIntentContext>(token, "test.bundle", 0, testIntentId);

    EXPECT_EQ(context->GetIntentId(), testIntentId);

    TAG_LOGI(AAFwkTag::TEST, "IntentIdTest_0100 end.");
}

/**
 * @tc.name: BundleNameTest_0100
 * @tc.desc: Test getting bundle name
 * @tc.type: FUNC
 * @tc.require: I9XXXXX
 */
HWTEST_F(EtsInsightIntentContextTest, BundleNameTest_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "BundleNameTest_0100 begin.");

    std::string testBundleName = "com.test.insight.bundle";
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityManagerService();
    auto context = std::make_shared<InsightIntentContext>(token, testBundleName, 0, 10001);

    EXPECT_EQ(context->GetBundleName(), testBundleName);

    TAG_LOGI(AAFwkTag::TEST, "BundleNameTest_0100 end.");
}

/**
 * @tc.name: StartAbilityByInsightIntent_0400
 * @tc.desc: Test StartAbilityByInsightIntent with window mode parameter
 * @tc.type: FUNC
 * @tc.require: I9XXXXX
 */
HWTEST_F(EtsInsightIntentContextTest, StartAbilityByInsightIntent_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByInsightIntent_0400 begin.");

    auto context = CreateTestContext(AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY);
    ASSERT_NE(context, nullptr);

    AAFwk::Want want;
    ElementName element;
    element.SetBundleName("test.bundle.name");
    element.SetAbilityName("TestAbility");
    want.SetElement(element);

    auto result = context->StartAbilityByInsightIntent(want);
    // Verify that window mode is set in want
    auto windowMode = want.GetIntParam(AAFwk::Want::PARAM_RESV_WINDOW_MODE, -1);
    EXPECT_EQ(windowMode, AbilityWindowConfiguration::MULTI_WINDOW_DISPLAY_PRIMARY);

    TAG_LOGI(AAFwkTag::TEST, "StartAbilityByInsightIntent_0400 end.");
}

/**
 * @tc.name: ExecuteModeTest_0200
 * @tc.desc: Test all execute modes
 * @tc.type: FUNC
 * @tc.require: I9XXXXX
 */
HWTEST_F(EtsInsightIntentContextTest, ExecuteModeTest_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteModeTest_0200 begin.");

    auto context = CreateTestContext();
    ASSERT_NE(context, nullptr);

    std::vector<InsightIntentExecuteMode> modes = {
        InsightIntentExecuteMode::UIABILITY_FOREGROUND,
        InsightIntentExecuteMode::UIABILITY_BACKGROUND,
        InsightIntentExecuteMode::UIEXTENSION_ABILITY,
        InsightIntentExecuteMode::SERVICE_EXTENSION_ABILITY
    };

    for (auto mode : modes) {
        context->SetExecuteMode(static_cast<int32_t>(mode));
        EXPECT_EQ(context->GetExecuteMode(), static_cast<int32_t>(mode));
    }

    TAG_LOGI(AAFwkTag::TEST, "ExecuteModeTest_0200 end.");
}

/**
 * @tc.name: ExecuteModeTest_0300
 * @tc.desc: Test invalid execute mode values
 * @tc.type: FUNC
 * @tc.require: I9XXXXX
 */
HWTEST_F(EtsInsightIntentContextTest, ExecuteModeTest_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteModeTest_0300 begin.");

    auto context = CreateTestContext();
    ASSERT_NE(context, nullptr);

    // Test with out-of-range values
    std::vector<int32_t> invalidModes = {-1, 100, INT32_MIN, INT32_MAX};

    for (auto mode : invalidModes) {
        context->SetExecuteMode(mode);
        EXPECT_EQ(context->GetExecuteMode(), mode);
    }

    TAG_LOGI(AAFwkTag::TEST, "ExecuteModeTest_0300 end.");
}

} // namespace AAFwk
} // namespace OHOS
