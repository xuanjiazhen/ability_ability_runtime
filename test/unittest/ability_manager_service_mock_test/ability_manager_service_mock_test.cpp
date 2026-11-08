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

#include <chrono>
#include <deque>

#define private public
#define protected public
#include "ability_manager_service.h"
#include "extract_insight_intent_profile.h"
#include "insight_intent_execute_manager.h"
#include "session_info.h"
#include "sub_managers_helper.h"
#include "insight_intent_query_param.h"
#include "insight_intent_execute_lite_param.h"
#undef private
#undef protected
#include "ability_manager_errors.h"
#include "bundle_mgr_helper.h"
#include "start_ability_utils.h"
#include "message_parcel.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
    constexpr char INSIGHT_INTENT_EXECUTE_PARAM_NAME[] = "ohos.insightIntent.executeParam.name";
}
class AbilityManagerServiceMockTest : public Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};
    
void AbilityManagerServiceMockTest::SetUpTestCase() {}
    
void AbilityManagerServiceMockTest::TearDownTestCase() {}
    
void AbilityManagerServiceMockTest::SetUp()
{
    AppExecFwk::BundleMgrHelper::mockExtensionInfoSuccess = false;
    AppExecFwk::BundleMgrHelper::mockDualModeAppIndexZero = false;
}
    
void AbilityManagerServiceMockTest::TearDown() {}

/**
* @tc.name: ExecuteIntent_0400
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0100, TestSize.Level1)
{
    Want want;
    auto ret = AppExecFwk::InsightIntentExecuteParam::IsInsightIntentExecute(want);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name: InsightIntentExecuteParam_0200
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0200, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, 1);
    auto ret = AppExecFwk::InsightIntentExecuteParam::IsInsightIntentExecute(want);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name: InsightIntentExecuteParam_0300
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0300, TestSize.Level1)
{
    Want want;
    auto ret = AppExecFwk::InsightIntentExecuteParam::IsInsightIntentPage(want);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name: InsightIntentExecuteParam_0400
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0400, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_DECORATOR_TYPE, 1);
    auto ret = AppExecFwk::InsightIntentExecuteParam::IsInsightIntentPage(want);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name: InsightIntentExecuteParam_0500
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0500, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_DECORATOR_TYPE, 2);
    auto ret = AppExecFwk::InsightIntentExecuteParam::IsInsightIntentPage(want);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name: InsightIntentExecuteParam_0600
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0600, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_DECORATOR_TYPE, 2);
    auto ret = AppExecFwk::InsightIntentExecuteParam::IsInsightIntentPage(want);
    EXPECT_EQ(ret, true);
}

/**
* @tc.name: InsightIntentExecuteParam_0700
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0700, TestSize.Level1)
{
    Want want;
    AppExecFwk::InsightIntentExecuteParam param;
    auto ret = AppExecFwk::InsightIntentExecuteParam::GenerateFromWant(want, param);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name: InsightIntentExecuteParam_0800
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0800, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, std::string("error"));
    AppExecFwk::InsightIntentExecuteParam param;
    auto ret = AppExecFwk::InsightIntentExecuteParam::GenerateFromWant(want, param);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name: InsightIntentExecuteParam_0900
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_0900, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, std::string("error"));
    AppExecFwk::InsightIntentExecuteParam param;
    auto ret = AppExecFwk::InsightIntentExecuteParam::GenerateFromWant(want, param);
    EXPECT_EQ(ret, false);
}

/**
* @tc.name: InsightIntentExecuteParam_1000
* @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
* @tc.type: FUNC
*/
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteParam_1000, TestSize.Level1)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_MODE, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_PARAM, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_SRC_ENTRY, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_URI, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_FLAGS, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_OPENLINK_FLAG, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_DECORATOR_TYPE, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_SRC_ENTRANCE, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_FUNC_PARAM_CLASSNAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_FUNC_PARAM_METHODNAME, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_FUNC_PARAM_METHODPARAMS, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_PAGEPATH, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_NAVIGATIONID, 1);
    want.SetParam(AppExecFwk::INSIGHT_INTENT_PAGE_PARAM_NAVDESTINATIONNAME, 1);

    auto ret = AppExecFwk::InsightIntentExecuteParam::RemoveInsightIntent(want);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: ExecuteIntent_0100
 * @tc.desc: Test  StartAbilityByCallWithInsightIntent error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntent_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "ability1";
    param.insightIntentName_ = "test1";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    std::shared_ptr<AbilityRecord> ability = nullptr;
    sptr<IRemoteObject> callerToken = new OHOS::AAFwk::Token(ability);
    uint64_t key = 1;
    auto ret = abilityMs->ExecuteIntent(key, callerToken, param);
    EXPECT_EQ(ret, RESOLVE_ABILITY_ERR);
}

/**
 * @tc.name: ExecuteIntent_0200
 * @tc.desc: Test  StartAbilityWithInsightIntent error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntent_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "ability1";
    param.insightIntentName_ = "test1";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    std::shared_ptr<AbilityRecord> ability = nullptr;
    sptr<IRemoteObject> callerToken = new OHOS::AAFwk::Token(ability);
    uint64_t key = 1;
    auto ret = abilityMs->ExecuteIntent(key, callerToken, param);
    EXPECT_EQ(ret, RESOLVE_ABILITY_ERR);
}

/**
 * @tc.name: ExecuteIntent_0300
 * @tc.desc: Test  StartExtensionAbilityWithInsightIntent error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntent_0300, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "ability1";
    param.insightIntentName_ = "test1";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.executeMode_ = AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY;
    std::shared_ptr<AbilityRecord> ability = nullptr;
    sptr<IRemoteObject> callerToken = new OHOS::AAFwk::Token(ability);
    uint64_t key = 1;
    auto ret = abilityMs->ExecuteIntent(key, callerToken, param);
    EXPECT_EQ(ret, RESOLVE_ABILITY_ERR);
}

/**
 * @tc.name: ExecuteIntent_0400
 * @tc.desc: Test ExecuteIntent distributed branch permission denied
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntent_0400, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "ability1";
    param.insightIntentName_ = "test1";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    param.deviceId_ = "remoteDevice";

    std::shared_ptr<AbilityRecord> ability = nullptr;
    sptr<IRemoteObject> callerToken = new OHOS::AAFwk::Token(ability);
    uint64_t key = 1;

    auto ret = abilityMs->ExecuteIntent(key, callerToken, param);
    EXPECT_EQ(ret, ERR_INTENT_CONNECTION_FAILED);
}

/**
 * @tc.name: ExecuteIntent_0500
 * @tc.desc: Test ExecuteIntent distributed branch with flood attack
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntent_0500, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    AppExecFwk::InsightIntentExecuteParam param;
    param.bundleName_ = "test.bundleName";
    param.moduleName_ = "test.entry";
    param.abilityName_ = "ability1";
    param.insightIntentName_ = "test1";
    param.insightIntentParam_ = std::make_shared<WantParams>();
    param.executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;
    param.deviceId_ = "remoteDevice";

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    int64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    abilityMs->floodAttackStatistics_[callerUid] = std::deque<int64_t>(10, now);

    std::shared_ptr<AbilityRecord> ability = nullptr;
    sptr<IRemoteObject> callerToken = new OHOS::AAFwk::Token(ability);
    uint64_t key = 1;
    auto ret = abilityMs->ExecuteIntent(key, callerToken, param);
    EXPECT_EQ(ret, INNER_ERR);
}

/**
 * @tc.name: ExecuteIntentForDistributed_0100
 * @tc.desc: Test ExecuteIntentForDistributed with invalid want
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntentForDistributed_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    Want want;
    uint64_t requestCode = 1;
    uint64_t specifiedFullTokenId = 0;
    auto ret = abilityMs->ExecuteIntentForDistributed(want, "deviceA", requestCode, specifiedFullTokenId);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: ExecuteIntentForDistributed_0200
 * @tc.desc: Test ExecuteIntentForDistributed enters common path and returns invalid for incomplete element
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntentForDistributed_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    Want want;
    want.SetElementName("", "com.example.bundle", "MainAbility");
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_NAME, std::string("intent.test"));
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, std::string("1"));

    uint64_t requestCode = 1;
    uint64_t specifiedFullTokenId = 0;
    auto ret = abilityMs->ExecuteIntentForDistributed(want, "deviceA", requestCode, specifiedFullTokenId);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: ExecuteIntentCommon_0100
 * @tc.desc: Test ExecuteIntentCommon with nullptr param
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntentCommon_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    AbilityRuntime::ExtractInsightIntentGenericInfo infos;
    AbilityRuntime::ExecuteIntentCommonOptions options(true, infos, 1);
    auto ret = abilityMs->ExecuteIntentCommon(nullptr, nullptr, "", options);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: ExecuteIntentCommon_0200
 * @tc.desc: Test ExecuteIntentCommon with invalid required fields
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntentCommon_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    auto param = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    param->bundleName_ = "";
    param->moduleName_ = "test.entry";
    param->abilityName_ = "MainAbility";
    param->insightIntentName_ = "intent.test";

    AbilityRuntime::ExtractInsightIntentGenericInfo infos;
    AbilityRuntime::ExecuteIntentCommonOptions options(false, infos, 1);
    auto ret = abilityMs->ExecuteIntentCommon(nullptr, param, "", options);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: ExecuteIntentCommon_0300
 * @tc.desc: Test ExecuteIntentCommon switch UI_ABILITY_FOREGROUND branch
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, ExecuteIntentCommon_0300, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    auto param = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    param->bundleName_ = "test.bundleName";
    param->moduleName_ = "test.entry";
    param->abilityName_ = "ability1";
    param->insightIntentName_ = "test1";
    param->insightIntentParam_ = std::make_shared<WantParams>();
    param->executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_FOREGROUND;

    AbilityRuntime::ExtractInsightIntentGenericInfo infos;
    AbilityRuntime::ExecuteIntentCommonOptions options(false, infos, 1);
    auto ret = abilityMs->ExecuteIntentCommon(nullptr, param, "", options);
    EXPECT_NE(ret, ERR_OK);

    param->executeMode_ = AppExecFwk::ExecuteMode::UI_ABILITY_BACKGROUND;
    ret = abilityMs->ExecuteIntentCommon(nullptr, param, "", options);
    EXPECT_NE(ret, ERR_OK);

    param->executeMode_ = AppExecFwk::ExecuteMode::UI_EXTENSION_ABILITY;
    ret = abilityMs->ExecuteIntentCommon(nullptr, param, "", options);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    param->executeMode_ = AppExecFwk::ExecuteMode::SERVICE_EXTENSION_ABILITY;
    ret = abilityMs->ExecuteIntentCommon(nullptr, param, "", options);
    EXPECT_NE(ret, ERR_OK);

    param->executeMode_ = 999;
    ret = abilityMs->ExecuteIntentCommon(nullptr, param, "", options);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: PreloadUIExtensionAbilityInner_0100
 * @tc.desc: Test  GetConnectManagerByUserId error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, PreloadUIExtensionAbilityInner_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    std::string bundleName = "testBundleName";
    std::string abilityName_ = "testAbility";
    Want want;
    want.SetElementName(bundleName, abilityName_);
    AppExecFwk::BundleMgrHelper::mockExtensionInfoSuccess = true;
    AppExecFwk::BundleMgrHelper::mockDualModeAppIndexZero = true;
    abilityMs->subManagersHelper_ = nullptr;
    int32_t preloadId = 1;
    int32_t userId = 1;
    auto ret = abilityMs->PreloadUIExtensionAbilityInner(want, bundleName, preloadId, userId);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: StartUIExtensionAbility_0100
 * @tc.desc: Test  getabilityInfos error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, StartUIExtensionAbility_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    want.SetParam(AAFwk::SCREEN_MODE_KEY, AAFwk::EMBEDDED_FULL_SCREEN_MODE);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->want = want;
    auto ret = abilityMs->StartUIExtensionAbility(sessionInfo, 1);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: StartUIExtensionAbility_0200
 * @tc.desc: Test  CheckAndUpdateWant error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, StartUIExtensionAbility_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    want.SetParam(INSIGHT_INTENT_EXECUTE_PARAM_NAME, true);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->want = want;
    auto ret = abilityMs->StartUIExtensionAbility(sessionInfo, 1);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: StartUIExtensionAbility_0300
 * @tc.desc: Test getabilityInfos error
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, StartUIExtensionAbility_0300, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    want.SetParam(AAFwk::SCREEN_MODE_KEY, AAFwk::EMBEDDED_HALF_SCREEN_MODE);
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->want = want;
    auto ret = abilityMs->StartUIExtensionAbility(sessionInfo, 1);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: QueryEntityInfo_0100
 * @tc.desc: Test QueryEntityInfo with executeManager nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, QueryEntityInfo_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    uint64_t key = 123;
    sptr<IRemoteObject> callerToken = nullptr;
    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundle";
    param.moduleName_ = "test.module";
    param.intentName_ = "test.intent";
    param.className_ = "test.class";
    param.userId_ = 100;

    auto ret = abilityMs->QueryEntityInfo(key, callerToken, param);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: QueryEntityInfo_0200
 * @tc.desc: Test QueryEntityInfo with empty bundleName
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, QueryEntityInfo_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    uint64_t key = 123;
    sptr<IRemoteObject> callerToken = nullptr;
    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "";
    param.moduleName_ = "test.module";
    param.intentName_ = "test.intent";
    param.className_ = "test.class";
    param.userId_ = 100;

    auto ret = abilityMs->QueryEntityInfo(key, callerToken, param);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: QueryEntityInfo_0300
 * @tc.desc: Test QueryEntityInfo with empty moduleName
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, QueryEntityInfo_0300, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    uint64_t key = 123;
    sptr<IRemoteObject> callerToken = nullptr;
    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundle";
    param.moduleName_ = "";
    param.intentName_ = "test.intent";
    param.className_ = "test.class";
    param.userId_ = 100;

    auto ret = abilityMs->QueryEntityInfo(key, callerToken, param);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: QueryEntityInfo_0400
 * @tc.desc: Test QueryEntityInfo with empty intentName
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, QueryEntityInfo_0400, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    uint64_t key = 123;
    sptr<IRemoteObject> callerToken = nullptr;
    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundle";
    param.moduleName_ = "test.module";
    param.intentName_ = "";
    param.className_ = "test.class";
    param.userId_ = 100;

    auto ret = abilityMs->QueryEntityInfo(key, callerToken, param);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: QueryEntityInfo_0500
 * @tc.desc: Test QueryEntityInfo with empty className
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, QueryEntityInfo_0500, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    uint64_t key = 123;
    sptr<IRemoteObject> callerToken = nullptr;
    AppExecFwk::InsightIntentQueryParam param;
    param.bundleName_ = "test.bundle";
    param.moduleName_ = "test.module";
    param.intentName_ = "test.intent";
    param.className_ = "";
    param.userId_ = 100;

    auto ret = abilityMs->QueryEntityInfo(key, callerToken, param);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.name: ExecuteUIAbilityForegroundIntentWithSpecifyTokenId_0100
 * @tc.desc: Test UID guard rejects non-foundation caller.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest,
    ExecuteUIAbilityForegroundIntentWithSpecifyTokenId_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    Want want;
    AppExecFwk::InsightIntentExecuteLiteParam param;
    param.insightIntentName = "test.intent";
    auto ret = abilityMs->ExecuteUIAbilityForegroundIntentWithSpecifyTokenId(
        want, nullptr, param, 0);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: InsightIntentExecuteLiteParam_0100
 * @tc.desc: Test LiteParam marshalling roundtrip.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteLiteParam_0100, TestSize.Level1)
{
    AppExecFwk::InsightIntentExecuteLiteParam param1;
    param1.key = 42;
    param1.insightIntentName = "test.intent";

    MessageParcel parcel;
    EXPECT_TRUE(param1.Marshalling(parcel));

    auto param2 = AppExecFwk::InsightIntentExecuteLiteParam::Unmarshalling(parcel);
    ASSERT_NE(param2, nullptr);
    EXPECT_EQ(param2->key, 42U);
    EXPECT_EQ(param2->insightIntentName, "test.intent");
    delete param2;
}

/**
 * @tc.name: InsightIntentExecuteLiteParam_0200
 * @tc.desc: Test LiteParam default values.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteLiteParam_0200, TestSize.Level1)
{
    AppExecFwk::InsightIntentExecuteLiteParam param;
    EXPECT_EQ(param.key, 0U);
    EXPECT_TRUE(param.insightIntentName.empty());
    EXPECT_EQ(param.insightIntentParam.Size(), 0);
    EXPECT_EQ(param.insightIntentHostClient, nullptr);
}

/**
 * @tc.name: BuildExecuteParamFromWant_0100
 * @tc.desc: Test BuildExecuteParamFromWant maps want element correctly.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, BuildExecuteParamFromWant_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    Want want;
    want.SetElementName("", "com.test.bundle", "MainAbility", "com.test.module");
    AppExecFwk::InsightIntentExecuteLiteParam liteParam;
    liteParam.insightIntentName = "test.intent";

    auto result = abilityMs->BuildExecuteParamFromWant(want, liteParam);
    ASSERT_NE(result, nullptr);
    EXPECT_EQ(result->bundleName_, "com.test.bundle");
    EXPECT_EQ(result->abilityName_, "MainAbility");
    EXPECT_EQ(result->moduleName_, "com.test.module");
    EXPECT_EQ(result->insightIntentName_, "test.intent");
}

/**
 * @tc.name: BuildExecuteParamFromWant_0200
 * @tc.desc: Test BuildExecuteParamFromWant with empty want element.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, BuildExecuteParamFromWant_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    Want want;
    AppExecFwk::InsightIntentExecuteLiteParam liteParam;
    auto result = abilityMs->BuildExecuteParamFromWant(want, liteParam);
    ASSERT_NE(result, nullptr);
    EXPECT_TRUE(result->bundleName_.empty());
    EXPECT_TRUE(result->moduleName_.empty());
    EXPECT_TRUE(result->abilityName_.empty());
    EXPECT_TRUE(result->insightIntentName_.empty());
}

/**
 * @tc.name: ExecuteUIAbilityForegroundIntentWithSpecifyTokenId_0200
 * @tc.desc: Test with empty intent name.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest,
    ExecuteUIAbilityForegroundIntentWithSpecifyTokenId_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);

    Want want;
    AppExecFwk::InsightIntentExecuteLiteParam param;
    auto ret = abilityMs->ExecuteUIAbilityForegroundIntentWithSpecifyTokenId(
        want, nullptr, param, 0);
    EXPECT_EQ(ret, CHECK_PERMISSION_FAILED);
}

/**
 * @tc.name: InsightIntentExecuteLiteParam_0300
 * @tc.desc: Test LiteParam marshalling with insightIntentParam content.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteLiteParam_0300, TestSize.Level1)
{
    AppExecFwk::InsightIntentExecuteLiteParam param1;
    param1.key = 100;
    param1.insightIntentName = "intent.with.params";
    WantParams innerParams;
    param1.insightIntentParam = innerParams;

    MessageParcel parcel;
    EXPECT_TRUE(param1.Marshalling(parcel));

    auto param2 = AppExecFwk::InsightIntentExecuteLiteParam::Unmarshalling(parcel);
    ASSERT_NE(param2, nullptr);
    EXPECT_EQ(param2->key, 100U);
    EXPECT_EQ(param2->insightIntentName, "intent.with.params");
    delete param2;
}

/**
 * @tc.name: InsightIntentExecuteLiteParam_0400
 * @tc.desc: Test LiteParam marshalling with null hostClient.
 * @tc.type: FUNC
 */
HWTEST_F(AbilityManagerServiceMockTest, InsightIntentExecuteLiteParam_0400, TestSize.Level1)
{
    AppExecFwk::InsightIntentExecuteLiteParam param1;
    param1.key = 0;
    param1.insightIntentHostClient = nullptr;

    MessageParcel parcel;
    EXPECT_TRUE(param1.Marshalling(parcel));

    auto param2 = AppExecFwk::InsightIntentExecuteLiteParam::Unmarshalling(parcel);
    ASSERT_NE(param2, nullptr);
    EXPECT_EQ(param2->key, 0U);
    EXPECT_EQ(param2->insightIntentHostClient, nullptr);
    delete param2;
}

/**
 * @tc.name: IsInsightIntentCall_0100
 * @tc.desc: Want without intentId returns false.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceMockTest, IsInsightIntentCall_0100, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    EXPECT_FALSE(abilityMs->IsInsightIntentCall(want, ""));
}

/**
 * @tc.name: IsInsightIntentCall_0200
 * @tc.desc: Want with invalid intentId string returns false.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceMockTest, IsInsightIntentCall_0200, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, std::string("invalid"));
    EXPECT_FALSE(abilityMs->IsInsightIntentCall(want, ""));
}

namespace {
constexpr uint64_t INTENT_ID_CALLER_0100 = 90001;
constexpr uint64_t INTENT_ID_CALLER_0200 = 90002;
constexpr uint64_t INTENT_ID_CALLER_0300 = 90003;
constexpr uint64_t INTENT_ID_CALLER_0400 = 90004;
constexpr uint64_t INTENT_ID_CALLER_0500 = 90005;
constexpr uint64_t INTENT_ID_CALLER_0600 = 90006;

std::shared_ptr<InsightIntentExecuteRecord> MakeIntentRecord(const std::string &bundleName, uint32_t callerUid,
    uint32_t callerTokenId, bool isDistributed = false)
{
    auto record = std::make_shared<InsightIntentExecuteRecord>();
    record->bundleName = bundleName;
    record->callerUid = callerUid;
    record->callerTokenId = callerTokenId;
    record->isDistributed = isDistributed;
    return record;
}

Want MakeIntentCallWant(uint64_t intentId)
{
    Want want;
    want.SetParam(AppExecFwk::INSIGHT_INTENT_EXECUTE_PARAM_ID, std::to_string(intentId));
    return want;
}
} // namespace

/**
 * @tc.name: IsInsightIntentCall_0300
 * @tc.desc: Record owned by the current caller passes both the bundle and the identity check.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceMockTest, IsInsightIntentCall_0300, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    auto executeManager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(executeManager, nullptr);
    auto callerUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());
    auto callerTokenId = static_cast<uint32_t>(IPCSkeleton::GetCallingTokenID());
    executeManager->records_[INTENT_ID_CALLER_0100] =
        MakeIntentRecord("com.test.target", callerUid, callerTokenId);
    EXPECT_TRUE(abilityMs->IsInsightIntentCall(MakeIntentCallWant(INTENT_ID_CALLER_0100), "com.test.target"));
    executeManager->records_.erase(INTENT_ID_CALLER_0100);
}

/**
 * @tc.name: IsInsightIntentCall_0400
 * @tc.desc: Record created by another caller is rejected even if bundle matches.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceMockTest, IsInsightIntentCall_0400, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    auto executeManager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(executeManager, nullptr);
    auto callerTokenId = static_cast<uint32_t>(IPCSkeleton::GetCallingTokenID());
    // Token id that cannot belong to the current test caller.
    executeManager->records_[INTENT_ID_CALLER_0200] =
        MakeIntentRecord("com.test.target", 12345, callerTokenId + 1);
    EXPECT_FALSE(abilityMs->IsInsightIntentCall(MakeIntentCallWant(INTENT_ID_CALLER_0200), "com.test.target"));
    executeManager->records_.erase(INTENT_ID_CALLER_0200);
}

/**
 * @tc.name: IsInsightIntentCall_0500
 * @tc.desc: Record without captured caller identity is rejected, fail closed.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceMockTest, IsInsightIntentCall_0500, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    auto executeManager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(executeManager, nullptr);
    executeManager->records_[INTENT_ID_CALLER_0300] = MakeIntentRecord("com.test.target", 0, 0);
    EXPECT_FALSE(abilityMs->IsInsightIntentCall(MakeIntentCallWant(INTENT_ID_CALLER_0300), "com.test.target"));
    executeManager->records_.erase(INTENT_ID_CALLER_0300);
}

/**
 * @tc.name: IsInsightIntentCall_0600
 * @tc.desc: Bundle name mismatch between the want and the record returns false.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceMockTest, IsInsightIntentCall_0600, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    auto executeManager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(executeManager, nullptr);
    auto callerUid = static_cast<uint32_t>(IPCSkeleton::GetCallingUid());
    auto callerTokenId = static_cast<uint32_t>(IPCSkeleton::GetCallingTokenID());
    executeManager->records_[INTENT_ID_CALLER_0400] =
        MakeIntentRecord("com.test.other", callerUid, callerTokenId);
    EXPECT_FALSE(abilityMs->IsInsightIntentCall(MakeIntentCallWant(INTENT_ID_CALLER_0400), "com.test.target"));
    executeManager->records_.erase(INTENT_ID_CALLER_0400);
}

/**
 * @tc.name: IsInsightIntentCall_0700
 * @tc.desc: Distributed record skips the caller identity check and passes the bundle match.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceMockTest, IsInsightIntentCall_0700, TestSize.Level1)
{
    auto abilityMs = std::make_shared<AbilityManagerService>();
    ASSERT_NE(abilityMs, nullptr);
    auto executeManager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(executeManager, nullptr);
    // Remote callers cannot share the local token id, the distributed path is protected by
    // PERMISSION_EXECUTE_DISTRIBUTED_INTENT instead.
    executeManager->records_[INTENT_ID_CALLER_0500] =
        MakeIntentRecord("com.test.target", 12345, 67890, true);
    EXPECT_TRUE(abilityMs->IsInsightIntentCall(MakeIntentCallWant(INTENT_ID_CALLER_0500), "com.test.target"));
    executeManager->records_.erase(INTENT_ID_CALLER_0500);
}

/**
 * @tc.name: GetRecordCallerInfo_0100
 * @tc.desc: GetRecordCallerInfo returns the identity captured at record creation.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceMockTest, GetRecordCallerInfo_0100, TestSize.Level1)
{
    auto executeManager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(executeManager, nullptr);
    executeManager->records_[INTENT_ID_CALLER_0600] = MakeIntentRecord("com.test.callerinfo", 4321, 8765);
    std::string bundleName;
    uint32_t callerUid = 0;
    uint32_t callerTokenId = 0;
    bool isDistributed = true;
    EXPECT_EQ(executeManager->GetRecordCallerInfo(INTENT_ID_CALLER_0600, bundleName, callerUid, callerTokenId,
        isDistributed), ERR_OK);
    EXPECT_EQ(bundleName, "com.test.callerinfo");
    EXPECT_EQ(callerUid, 4321);
    EXPECT_EQ(callerTokenId, 8765);
    EXPECT_FALSE(isDistributed);
    executeManager->records_.erase(INTENT_ID_CALLER_0600);
}

/**
 * @tc.name: GetRecordCallerInfo_0200
 * @tc.desc: GetRecordCallerInfo returns ERR_INVALID_VALUE for a missing record.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AbilityManagerServiceMockTest, GetRecordCallerInfo_0200, TestSize.Level1)
{
    auto executeManager = DelayedSingleton<InsightIntentExecuteManager>::GetInstance();
    ASSERT_NE(executeManager, nullptr);
    std::string bundleName;
    uint32_t callerUid = 0;
    uint32_t callerTokenId = 0;
    bool isDistributed = false;
    EXPECT_EQ(executeManager->GetRecordCallerInfo(999999999, bundleName, callerUid, callerTokenId, isDistributed),
        ERR_INVALID_VALUE);
}
}
}
