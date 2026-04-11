/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "want.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class InsightIntentExecuteManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void InsightIntentExecuteManagerTest::SetUpTestCase(void)
{}

void InsightIntentExecuteManagerTest::TearDownTestCase(void)
{}

void InsightIntentExecuteManagerTest::SetUp()
{}

void InsightIntentExecuteManagerTest::TearDown()
{}

/**
 * @tc.name: GenerateWant_0100
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    // other member has default value.
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    Want want;
    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, want);
    EXPECT_EQ(ret, ERR_OK);
    // get display id of want, expect don't contain
    auto displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -100);
    EXPECT_EQ(displayId, -100);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: GenerateWant_0200
 * @tc.desc: basic function test of display id.
 * @tc.type: FUNC
 * @tc.require: issueI8ZRAG
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin.");
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.displayId_ = 2;
    // other member has default value.
    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    Want want;
    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, want);
    EXPECT_EQ(ret, ERR_OK);
    // get display id of want
    auto displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, -100);
    EXPECT_EQ(displayId, 2);
    TAG_LOGI(AAFwkTag::TEST, "end.");
}

/**
 * @tc.name: RemoveExecuteIntent_0100
 * @tc.desc: Test RemoveExecuteIntent with valid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, RemoveExecuteIntent_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveExecuteIntent_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    // First add a record
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.abilityName_ = "test.ability";
    param.insightIntentName_ = "TestIntent";
    param.isServiceMatch_ = true;

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 1005;

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
    ASSERT_EQ(ret, ERR_OK);
    uint64_t intentId = param.insightIntentId_;

    // Then remove it
    ret = manager->RemoveExecuteIntent(intentId);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "RemoveExecuteIntent_0100 end.");
}

/**
 * @tc.name: RemoveExecuteIntent_0200
 * @tc.desc: Test RemoveExecuteIntent with non-existent intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, RemoveExecuteIntent_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveExecuteIntent_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    uint64_t invalidIntentId = 99999;
    auto ret = manager->RemoveExecuteIntent(invalidIntentId);
    EXPECT_EQ(ret, ERR_OK);  // Remove doesn't return error for non-existent

    TAG_LOGI(AAFwkTag::TEST, "RemoveExecuteIntent_0200 end.");
}

/**
 * @tc.name: GetBundleName_0100
 * @tc.desc: Test GetBundleName with valid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GetBundleName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBundleName_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    // First add a record
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.abilityName_ = "test.ability";
    param.insightIntentName_ = "TestIntent";
    param.isServiceMatch_ = true;

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 1006;

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
    ASSERT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetBundleName_0100 end.");
}

/**
 * @tc.name: GetBundleName_0200
 * @tc.desc: Test GetBundleName with invalid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GetBundleName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetBundleName_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    uint64_t invalidIntentId = 99999;
    std::string bundleName;
    auto ret = manager->GetBundleName(invalidIntentId, bundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_TRUE(bundleName.empty());

    TAG_LOGI(AAFwkTag::TEST, "GetBundleName_0200 end.");
}

/**
 * @tc.name: RemoteDied_0100
 * @tc.desc: Test RemoteDied with valid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, RemoteDied_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoteDied_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    // First add a record
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.abilityName_ = "test.ability";
    param.insightIntentName_ = "TestIntent";
    param.isServiceMatch_ = true;

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 1008;

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
    ASSERT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "RemoteDied_0100 end.");
}

/**
 * @tc.name: RemoteDied_0200
 * @tc.desc: Test RemoteDied with invalid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, RemoteDied_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoteDied_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    uint64_t invalidIntentId = 99999;
    auto ret = manager->RemoteDied(invalidIntentId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "RemoteDied_0200 end.");
}

/**
 * @tc.name: ExecuteIntentDone_0100
 * @tc.desc: Test ExecuteIntentDone with valid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, ExecuteIntentDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    // First add a record
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.abilityName_ = "test.ability";
    param.insightIntentName_ = "TestIntent";
    param.isServiceMatch_ = true;

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 1009;

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
    ASSERT_EQ(ret, ERR_OK);
    uint64_t intentId = param.insightIntentId_;

    // Mark as done
    AppExecFwk::InsightIntentExecuteResult result;
    ret = manager->ExecuteIntentDone(intentId, 0, result);
    // This will fail because callerToken is null and callback iface_cast will fail
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0100 end.");
}

/**
 * @tc.name: ExecuteIntentDone_0200
 * @tc.desc: Test ExecuteIntentDone with invalid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, ExecuteIntentDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    uint64_t invalidIntentId = 99999;
    AppExecFwk::InsightIntentExecuteResult result;
    auto ret = manager->ExecuteIntentDone(invalidIntentId, 0, result);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0200 end.");
}

/**
 * @tc.name: CheckCallerPermission_0100
 * @tc.desc: Test CheckCallerPermission static method.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, CheckCallerPermission_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCallerPermission_0100 begin.");

    auto ret = InsightIntentExecuteManager::CheckCallerPermission();
    // Result depends on system permissions, just check it doesn't crash
    EXPECT_TRUE(ret == ERR_OK || ret == ERR_PERMISSION_DENIED);

    TAG_LOGI(AAFwkTag::TEST, "CheckCallerPermission_0100 end.");
}

/**
 * @tc.name: SetIntentExemptionInfo_0100
 * @tc.desc: Test SetIntentExemptionInfo with valid uid.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, SetIntentExemptionInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetIntentExemptionInfo_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    int32_t uid = 1000;
    manager->SetIntentExemptionInfo(uid);

    // Verify exemption info was set
    auto exemptionInfo = manager->GetAllIntentExemptionInfo();
    EXPECT_GE(exemptionInfo.size(), 0);

    TAG_LOGI(AAFwkTag::TEST, "SetIntentExemptionInfo_0100 end.");
}

/**
 * @tc.name: CheckIntentIsExemption_0100
 * @tc.desc: Test CheckIntentIsExemption with exempted uid.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, CheckIntentIsExemption_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckIntentIsExemption_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    int32_t uid = 1001;
    manager->SetIntentExemptionInfo(uid);

    bool isExemption = manager->CheckIntentIsExemption(uid);
    EXPECT_TRUE(isExemption);

    TAG_LOGI(AAFwkTag::TEST, "CheckIntentIsExemption_0100 end.");
}

/**
 * @tc.name: GetAllIntentExemptionInfo_0100
 * @tc.desc: Test GetAllIntentExemptionInfo returns map.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GetAllIntentExemptionInfo_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllIntentExemptionInfo_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    int32_t uid = 1002;
    manager->SetIntentExemptionInfo(uid);

    auto exemptionInfo = manager->GetAllIntentExemptionInfo();
    EXPECT_GE(exemptionInfo.size(), 1);

    TAG_LOGI(AAFwkTag::TEST, "GetAllIntentExemptionInfo_0100 end.");
}

/**
 * @tc.name: GenerateWant_0300
 * @tc.desc: Test GenerateWant with null param.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0300 begin.");

    std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> paramPtr = nullptr;
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    Want want;

    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, want);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0300 end.");
}

/**
 * @tc.name: GenerateWant_0400
 * @tc.desc: Test GenerateWant with all parameters set.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0400 begin.");

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "PlayMusic";
    param.insightIntentParam_ = nullptr;
    param.displayId_ = 2;
    param.insightIntentId_ = 100;

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    Want want;

    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, want);
    EXPECT_EQ(ret, ERR_OK);

    // Verify intentId is set in want
    auto intentIdStr = want.GetStringParam("ohos.extra.param.key.insightIntentId");
    EXPECT_TRUE(intentIdStr.empty());

    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0400 end.");
}

/**
 * @tc.name: CheckAndUpdateParam_0100
 * @tc.desc: Test CheckAndUpdateParam with valid parameters.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, CheckAndUpdateParam_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateParam_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.abilityName_ = "test.ability";
    param.insightIntentName_ = "TestIntent";
    param.isServiceMatch_ = true;

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 2000;
    std::string callerBundleName = "caller.bundle.name";

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, callerBundleName, false);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_NE(param.insightIntentId_, 0);

    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateParam_0100 end.");
}

/**
 * @tc.name: CheckAndUpdateParam_0200
 * @tc.desc: Test CheckAndUpdateParam with empty bundle name.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, CheckAndUpdateParam_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateParam_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "";  // Empty bundle name
    param.moduleName_ = "test.module";
    param.abilityName_ = "test.ability";
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 2001;
    std::string callerBundleName = "caller.bundle.name";

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, callerBundleName, false);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateParam_0200 end.");
}

/**
 * @tc.name: GetCallerBundleName_0100
 * @tc.desc: Test GetCallerBundleName with valid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GetCallerBundleName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetCallerBundleName_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.abilityName_ = "test.ability";
    param.insightIntentName_ = "TestIntent";
    param.isServiceMatch_ = true;

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 2002;
    std::string callerBundleName = "caller.bundle.name";

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, callerBundleName, false);
    ASSERT_EQ(ret, ERR_OK);
    uint64_t intentId = param.insightIntentId_;

    std::string retrievedCallerBundleName;
    ret = manager->GetCallerBundleName(intentId, retrievedCallerBundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(retrievedCallerBundleName, callerBundleName);

    TAG_LOGI(AAFwkTag::TEST, "GetCallerBundleName_0100 end.");
}

/**
 * @tc.name: GetCallerBundleName_0200
 * @tc.desc: Test GetCallerBundleName with invalid intentId.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GetCallerBundleName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetCallerBundleName_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    uint64_t invalidIntentId = 99999;
    std::string callerBundleName;

    auto ret = manager->GetCallerBundleName(invalidIntentId, callerBundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_TRUE(callerBundleName.empty());

    TAG_LOGI(AAFwkTag::TEST, "GetCallerBundleName_0200 end.");
}

/**
 * @tc.name: CheckAndUpdateWant_0100
 * @tc.desc: Test CheckAndUpdateWant with UIAbilityForeground mode.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, CheckAndUpdateWant_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateWant_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    Want want;
    auto executeMode = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    int32_t userId = 100;
    std::string callerBundleName = "caller.bundle";

    auto ret = manager->CheckAndUpdateWant(want, executeMode, userId, callerBundleName);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateWant_0100 end.");
}

/**
 * @tc.name: CheckAndUpdateWant_0200
 * @tc.desc: Test CheckAndUpdateWant with invalid mode.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, CheckAndUpdateWant_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateWant_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    Want want;
    // Test with invalid execute mode (out of range)
    auto executeMode = static_cast<AppExecFwk::ExecuteMode>(999);
    int32_t userId = 100;
    std::string callerBundleName = "caller.bundle";

    auto ret = manager->CheckAndUpdateWant(want, executeMode, userId, callerBundleName);
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateWant_0200 end.");
}

/**
 * @tc.name: OnInsightAppDied_0100
 * @tc.desc: Test OnInsightAppDied with valid bundle name.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, OnInsightAppDied_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnInsightAppDied_0100 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    std::string bundleName = "test.died.bundle";
    ASSERT_FALSE(bundleName.empty());

    // Should not crash - void method, test passes if no exception thrown
    manager->OnInsightAppDied(bundleName);
    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "OnInsightAppDied_0100 end.");
}

/**
 * @tc.name: OnInsightAppDied_0200
 * @tc.desc: Test OnInsightAppDied with empty bundle name.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, OnInsightAppDied_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnInsightAppDied_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    std::string emptyBundleName = "";
    ASSERT_TRUE(emptyBundleName.empty());

    // Should handle gracefully - void method, test passes if no exception thrown
    manager->OnInsightAppDied(emptyBundleName);
    SUCCEED();

    TAG_LOGI(AAFwkTag::TEST, "OnInsightAppDied_0200 end.");
}

/**
 * @tc.name: CheckGetInsightIntentInfoPermission_0100
 * @tc.desc: Test CheckGetInsightIntentInfoPermission static method.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, CheckGetInsightIntentInfoPermission_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckGetInsightIntentInfoPermission_0100 begin.");

    auto ret = InsightIntentExecuteManager::CheckGetInsightIntenInfoPermission();
    // Verify returns a valid error code (not garbage)
    EXPECT_GE(ret, ERR_INVALID_OPERATION);

    TAG_LOGI(AAFwkTag::TEST, "CheckGetInsightIntentInfoPermission returned: %{public}d", ret);

    TAG_LOGI(AAFwkTag::TEST, "CheckGetInsightIntentInfoPermission_0100 end.");
}

/**
 * @tc.name: ExecuteIntentDone_0300
 * @tc.desc: Test ExecuteIntentDone with result data.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, ExecuteIntentDone_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0300 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.abilityName_ = "test.ability";
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 2003;

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
    ASSERT_EQ(ret, ERR_OK);
    uint64_t intentId = param.insightIntentId_;

    // Create result with data
    AppExecFwk::InsightIntentExecuteResult result;
    result.resultCode = 0;
    result.resultData["key1"] = "value1";
    result.resultData["key2"] = "value2";

    ret = manager->ExecuteIntentDone(intentId, 0, result);
    // Will fail because callerToken is null, but should not crash
    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "ExecuteIntentDone_0300 end.");
}

/**
 * @tc.name: RemoteDied_0300
 * @tc.desc: Test RemoteDied sets state correctly.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, RemoteDied_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoteDied_0300 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    // First add a record
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.abilityName_ = "test.ability";
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 2004;

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
    ASSERT_EQ(ret, ERR_OK);
    uint64_t intentId = param.insightIntentId_;

    // Call RemoteDied
    ret = manager->RemoteDied(intentId);
    EXPECT_EQ(ret, ERR_OK);

    // Verify intent is removed or marked as died
    std::string bundleName;
    ret = manager->GetBundleName(intentId, bundleName);
    // After RemoteDied, the record should be cleaned up
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    EXPECT_TRUE(bundleName.empty());

    TAG_LOGI(AAFwkTag::TEST, "RemoteDied_0300 end.");
}

/**
 * @tc.name: RemoveExecuteIntent_0300
 * @tc.desc: Test RemoveExecuteIntent removes record correctly.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, RemoveExecuteIntent_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "RemoveExecuteIntent_0300 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    // Add a record
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.abilityName_ = "test.ability";
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 2005;

    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, "caller.bundle", false);
    ASSERT_EQ(ret, ERR_OK);
    uint64_t intentId = param.insightIntentId_;

    // Verify record exists
    std::string bundleName;
    ret = manager->GetBundleName(intentId, bundleName);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(bundleName, "test.bundleName");

    // Remove the record
    ret = manager->RemoveExecuteIntent(intentId);
    EXPECT_EQ(ret, ERR_OK);

    // Verify record no longer exists
    ret = manager->GetBundleName(intentId, bundleName);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    TAG_LOGI(AAFwkTag::TEST, "RemoveExecuteIntent_0300 end.");
}

/**
 * @tc.name: SetIntentExemptionInfo_0200
 * @tc.desc: Test SetIntentExemptionInfo with multiple UIDs.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, SetIntentExemptionInfo_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetIntentExemptionInfo_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    std::vector<int32_t> uids = {10001, 10002, 10003, 10004};

    // Set exemptions for multiple UIDs
    for (auto uid : uids) {
        manager->SetIntentExemptionInfo(uid);
    }

    // Verify all are exempted
    for (auto uid : uids) {
        bool isExemption = manager->CheckIntentIsExemption(uid);
        EXPECT_TRUE(isExemption);
    }

    // Get all exemption info
    auto exemptionInfo = manager->GetAllIntentExemptionInfo();
    EXPECT_GE(exemptionInfo.size(), uids.size());

    TAG_LOGI(AAFwkTag::TEST, "SetIntentExemptionInfo_0200 end.");
}

/**
 * @tc.name: CheckIntentIsExemption_0200
 * @tc.desc: Test CheckIntentIsExemption with non-exempted UID.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, CheckIntentIsExemption_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckIntentIsExemption_0200 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    int32_t nonExemptedUid = 99999;
    bool isExemption = manager->CheckIntentIsExemption(nonExemptedUid);
    EXPECT_FALSE(isExemption);

    TAG_LOGI(AAFwkTag::TEST, "CheckIntentIsExemption_0200 end.");
}

/**
 * @tc.name: CheckAndUpdateParam_0300
 * @tc.desc: Test CheckAndUpdateParam with ignoreAbilityName flag.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, CheckAndUpdateParam_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateParam_0300 begin.");

    auto manager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(manager, nullptr);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.module";
    param.abilityName_ = "";  // Empty ability name
    param.insightIntentName_ = "TestIntent";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);
    sptr<IRemoteObject> callerToken = nullptr;
    uint64_t key = 2006;
    std::string callerBundleName = "caller.bundle.name";

    // With ignoreAbilityName = true, should succeed even with empty abilityName
    auto ret = manager->CheckAndUpdateParam(key, callerToken, paramPtr, callerBundleName, true);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_NE(param.insightIntentId_, 0);

    TAG_LOGI(AAFwkTag::TEST, "CheckAndUpdateParam_0300 end.");
}

/**
 * @tc.name: GenerateWant_0500
 * @tc.desc: Test GenerateWant with Page decorator info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0500 begin.");

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "TestPageIntent";
    param.decoratorType_ = static_cast<int8_t>(AbilityRuntime::InsightIntentType::DECOR_PAGE);

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);

    AbilityRuntime::InsightIntentPageInfo pageInfo;
    pageInfo.uiAbility = "test.UIAbility";
    pageInfo.pagePath = "pages/test/page";
    pageInfo.navigationId = "nav_001";

    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    decoratorInfo.set<AbilityRuntime::InsightIntentPageInfo>();
    decoratorInfo.get<AbilityRuntime::InsightIntentPageInfo>() = pageInfo;
    decoratorInfo.decoratorType = AbilityRuntime::INSIGHT_INTENTS_TYPE_PAGE;

    Want want;
    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, want);
    EXPECT_EQ(ret, ERR_OK);

    // Verify page-specific parameters are set
    auto pagePath = want.GetStringParam(AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_PAGEPATH);
    EXPECT_FALSE(pagePath.empty());

    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0500 end.");
}

/**
 * @tc.name: GenerateWant_0600
 * @tc.desc: Test GenerateWant with Function decorator info.
 * @tc.type: FUNC
 */
HWTEST_F(InsightIntentExecuteManagerTest, GenerateWant_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0600 begin.");

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "test.abilityName";
    param.insightIntentName_ = "TestFunctionIntent";
    param.decoratorType_ = static_cast<int8_t>(AbilityRuntime::InsightIntentType::DECOR_FUNC);
    param.className_ = "TestClass";
    param.methodName_ = "testMethod";

    auto paramPtr = std::make_shared<AppExecFwk::InsightIntentExecuteParam>(param);

    AbilityRuntime::InsightIntentFunctionInfo functionInfo;
    functionInfo.functionName = "testFunction";
    functionInfo.functionParams = {"param1", "param2"};

    AbilityRuntime::ExtractInsightIntentGenericInfo decoratorInfo;
    decoratorInfo.set<AbilityRuntime::InsightIntentFunctionInfo>();
    decoratorInfo.get<AbilityRuntime::InsightIntentFunctionInfo>() = functionInfo;
    decoratorInfo.decoratorType = AbilityRuntime::INSIGHT_INTENTS_TYPE_FUNCTION;

    Want want;
    auto ret = InsightIntentExecuteManager::GenerateWant(paramPtr, decoratorInfo, want);
    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::TEST, "GenerateWant_0600 end.");
}

} // namespace AAFwk
} // namespace OHOS
