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
#define private public
#include "ability_context_impl.h"
#define protected public
#include "mock_context.h"
#include "mock_my_flag.h"
#include "ability_manager_errors.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AAFwk;
namespace {
const std::string TEST_BUNDLE_NAME = "com.test.bundle";
const std::string TEST_ABILITY_NAME = "TestAbility";
const std::string TEST_MODULE_NAME = "entry";
const int32_t TEST_REQUEST_CODE = 100;
const int32_t TEST_ACCOUNT_ID = 100;
const int32_t USER_CANCEL = -7;
}

class AbilityContextImplThirdTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
public:
    std::shared_ptr<AbilityContextImpl> context_ = nullptr;
    std::shared_ptr<MockContext> mock_ = nullptr;
};

void AbilityContextImplThirdTest::SetUpTestCase(void) {}

void AbilityContextImplThirdTest::TearDownTestCase(void) {}

void AbilityContextImplThirdTest::SetUp(void)
{
    context_ = std::make_shared<AbilityContextImpl>();
    mock_ = std::make_shared<MockContext>();
}

void AbilityContextImplThirdTest::TearDown(void) {}

/**
 * @tc.number: AbilityContextImplThirdTest_SetToken_0100
 * @tc.name: SetToken and GetToken
 * @tc.desc: Test SetToken and GetToken with valid token.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetToken_0100, Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetToken(), nullptr);
    sptr<IRemoteObject> token = new (std::nothrow) IPCObjectStub();
    ASSERT_NE(token, nullptr);
    context_->SetToken(token);
    EXPECT_EQ(context_->GetToken(), token);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetToken_0200
 * @tc.name: SetToken and GetToken
 * @tc.desc: Test SetToken with nullptr.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetToken_0200, Function | MediumTest | Level1)
{
    sptr<IRemoteObject> token = new (std::nothrow) IPCObjectStub();
    context_->SetToken(token);
    EXPECT_EQ(context_->GetToken(), token);
    context_->SetToken(nullptr);
    EXPECT_EQ(context_->GetToken(), nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetAbilityRecordId_0100
 * @tc.name: SetAbilityRecordId and GetAbilityRecordId
 * @tc.desc: Test SetAbilityRecordId and GetAbilityRecordId with valid id.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetAbilityRecordId_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetAbilityRecordId(), 0);
    int32_t recordId = 12345;
    context_->SetAbilityRecordId(recordId);
    EXPECT_EQ(context_->GetAbilityRecordId(), recordId);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetAbilityRecordId_0200
 * @tc.name: SetAbilityRecordId and GetAbilityRecordId
 * @tc.desc: Test SetAbilityRecordId with negative value.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetAbilityRecordId_0200,
    Function | MediumTest | Level1)
{
    int32_t recordId = -1;
    context_->SetAbilityRecordId(recordId);
    EXPECT_EQ(context_->GetAbilityRecordId(), recordId);
}

/**
 * @tc.number: AbilityContextImplThirdTest_RestoreWindowStage_0100
 * @tc.name: RestoreWindowStage(void*)
 * @tc.desc: Test RestoreWindowStage with void* contentStorage.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_RestoreWindowStage_0100,
    Function | MediumTest | Level1)
{
    int testData = 42;
    void *contentStorage = &testData;
    ErrCode ret = context_->RestoreWindowStage(contentStorage);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(context_->GetEtsContentStorage(), contentStorage);
}

/**
 * @tc.number: AbilityContextImplThirdTest_RestoreWindowStage_0200
 * @tc.name: RestoreWindowStage(void*)
 * @tc.desc: Test RestoreWindowStage with nullptr.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_RestoreWindowStage_0200,
    Function | MediumTest | Level1)
{
    ErrCode ret = context_->RestoreWindowStage(nullptr);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(context_->GetEtsContentStorage(), nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_RestoreWindowStage_0300
 * @tc.name: RestoreWindowStage(void*)
 * @tc.desc: Test RestoreWindowStage returns ERR_NOT_SUPPORTED when isHook_ is true.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_RestoreWindowStage_0300,
    Function | MediumTest | Level1)
{
    context_->isHook_ = true;
    int testData = 42;
    void *contentStorage = &testData;
    ErrCode ret = context_->RestoreWindowStage(contentStorage);
    // ERR_NOT_SUPPORTED = -2, defined in ability_context_impl.cpp
    EXPECT_EQ(ret, -2);
}

/**
 * @tc.number: AbilityContextImplThirdTest_StartSelf_0100
 * @tc.name: StartSelf
 * @tc.desc: Test StartSelf calls AbilityManagerClient, verifies context is not destroyed.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_StartSelf_0100, Function | MediumTest | Level1)
{
    sptr<IRemoteObject> token = new (std::nothrow) IPCObjectStub();
    context_->SetToken(token);
    ErrCode ret = context_->StartSelf();
    // AbilityManagerClient mock returns ERR_OK for StartSelf
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetHook_0100
 * @tc.name: SetHook and IsHook
 * @tc.desc: Test SetHook and IsHook default and modified values.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetHook_0100, Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->IsHook(), false);
    context_->SetHook(true);
    EXPECT_EQ(context_->IsHook(), true);
    context_->SetHook(false);
    EXPECT_EQ(context_->IsHook(), false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetHookOff_0100
 * @tc.name: SetHookOff and GetHookOff
 * @tc.desc: Test SetHookOff and GetHookOff default and modified values.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetHookOff_0100, Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetHookOff(), false);
    context_->SetHookOff(true);
    EXPECT_EQ(context_->GetHookOff(), true);
    context_->SetHookOff(false);
    EXPECT_EQ(context_->GetHookOff(), false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_IsTerminating_0100
 * @tc.name: IsTerminating and SetTerminating
 * @tc.desc: Test IsTerminating initial state and after setting.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_IsTerminating_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->IsTerminating(), false);
    context_->SetTerminating(true);
    EXPECT_EQ(context_->IsTerminating(), true);
    context_->SetTerminating(false);
    EXPECT_EQ(context_->IsTerminating(), false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetStageContext_0100
 * @tc.name: SetStageContext
 * @tc.desc: Test SetStageContext with valid context and verify getters work.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetStageContext_0100,
    Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    EXPECT_EQ(context_->GetBundleName(), "com.test.bundleName");
    EXPECT_NE(context_->GetApplicationInfo(), nullptr);
    EXPECT_EQ(context_->GetBundleCodePath(), "codePath");
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetStageContext_0200
 * @tc.name: SetStageContext with nullptr
 * @tc.desc: Test SetStageContext with nullptr and verify getters return defaults.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetStageContext_0200,
    Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    EXPECT_EQ(context_->GetBundleName(), "");
    EXPECT_EQ(context_->GetApplicationInfo(), nullptr);
    EXPECT_EQ(context_->GetBundleCodePath(), "");
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetAbilityInfo_0100
 * @tc.name: SetAbilityInfo and GetAbilityInfo
 * @tc.desc: Test SetAbilityInfo with valid info.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetAbilityInfo_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetAbilityInfo(), nullptr);
    auto abilityInfo = std::make_shared<AppExecFwk::AbilityInfo>();
    abilityInfo->name = TEST_ABILITY_NAME;
    abilityInfo->bundleName = TEST_BUNDLE_NAME;
    context_->SetAbilityInfo(abilityInfo);
    EXPECT_EQ(context_->GetAbilityInfo(), abilityInfo);
    EXPECT_EQ(context_->GetAbilityInfo()->name, TEST_ABILITY_NAME);
    EXPECT_EQ(context_->GetAbilityInfo()->bundleName, TEST_BUNDLE_NAME);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetConfiguration_0100
 * @tc.name: SetConfiguration and GetConfiguration
 * @tc.desc: Test SetConfiguration and GetConfiguration.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetConfiguration_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetConfiguration(), nullptr);
    auto config = std::make_shared<AppExecFwk::Configuration>();
    config->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "zh_CN");
    context_->SetConfiguration(config);
    EXPECT_EQ(context_->GetConfiguration(), config);
}

/**
 * @tc.number: AbilityContextImplThirdTest_GetWant_0100
 * @tc.name: GetWant
 * @tc.desc: Test GetWant when abilityCallback_ is nullptr.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_GetWant_0100, Function | MediumTest | Level1)
{
    std::weak_ptr<AppExecFwk::IAbilityCallback> emptyCallback;
    context_->RegisterAbilityCallback(emptyCallback);
    auto want = context_->GetWant();
    EXPECT_EQ(want, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_InsertRemoveResultCallbackTask_0100
 * @tc.name: InsertResultCallbackTask and RemoveResultCallbackTask
 * @tc.desc: Test insert and remove result callback task.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_InsertRemoveResultCallbackTask_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->resultCallbacks_.size(), 0u);
    RuntimeTask task = [](int32_t code, const AAFwk::Want& want, bool isInner) {};
    int requestCode = TEST_REQUEST_CODE;
    context_->InsertResultCallbackTask(requestCode, std::move(task));
    EXPECT_EQ(context_->resultCallbacks_.size(), 1u);
    context_->RemoveResultCallbackTask(requestCode);
    EXPECT_EQ(context_->resultCallbacks_.size(), 0u);
}

/**
 * @tc.number: AbilityContextImplThirdTest_RemoveResultCallbackTask_0100
 * @tc.name: RemoveResultCallbackTask
 * @tc.desc: Test remove non-existent result callback task.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_RemoveResultCallbackTask_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->resultCallbacks_.size(), 0u);
    context_->RemoveResultCallbackTask(999);
    EXPECT_EQ(context_->resultCallbacks_.size(), 0u);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetAbilityResourceManager_0100
 * @tc.name: SetAbilityResourceManager and GetResourceManager
 * @tc.desc: Test SetAbilityResourceManager overrides stageContext resource manager.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetAbilityResourceManager_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->abilityResourceMgr_, nullptr);
    auto resourceMgr = std::shared_ptr<Global::Resource::ResourceManager>(Global::Resource::CreateResourceManager());
    ASSERT_NE(resourceMgr, nullptr);
    context_->SetAbilityResourceManager(resourceMgr);
    EXPECT_EQ(context_->abilityResourceMgr_, resourceMgr);
    auto retrieved = context_->GetResourceManager();
    EXPECT_EQ(retrieved, resourceMgr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_RegisterAbilityConfigUpdateCallback_0100
 * @tc.name: RegisterAbilityConfigUpdateCallback
 * @tc.desc: Test RegisterAbilityConfigUpdateCallback with valid callback.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_RegisterAbilityConfigUpdateCallback_0100,
    Function | MediumTest | Level1)
{
    bool called = false;
    auto callback = [&called](AppExecFwk::Configuration& config) {
        called = true;
    };
    context_->RegisterAbilityConfigUpdateCallback(callback);
    EXPECT_NE(context_->abilityConfigUpdateCallback_, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_GetAbilityConfiguration_0100
 * @tc.name: GetAbilityConfiguration
 * @tc.desc: Test GetAbilityConfiguration default and after setting.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_GetAbilityConfiguration_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetAbilityConfiguration(), nullptr);
    AppExecFwk::Configuration config;
    config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE, "dark");
    context_->SetAbilityConfiguration(config);
    auto abilityConfig = context_->GetAbilityConfiguration();
    ASSERT_NE(abilityConfig, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetAbilityColorMode_0100
 * @tc.name: SetAbilityColorMode
 * @tc.desc: Test SetAbilityColorMode with invalid color mode values.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetAbilityColorMode_0100,
    Function | MediumTest | Level1)
{
    bool called = false;
    auto callback = [&called](AppExecFwk::Configuration& config) {
        called = true;
    };
    context_->abilityConfigUpdateCallback_ = callback;
    context_->SetAbilityColorMode(-2);
    EXPECT_EQ(called, false);
    context_->SetAbilityColorMode(2);
    EXPECT_EQ(called, false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetAbilityColorMode_0200
 * @tc.name: SetAbilityColorMode
 * @tc.desc: Test SetAbilityColorMode with valid color mode and callback.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetAbilityColorMode_0200,
    Function | MediumTest | Level1)
{
    bool called = false;
    auto callback = [&called](AppExecFwk::Configuration& config) {
        called = true;
    };
    context_->abilityConfigUpdateCallback_ = callback;
    context_->SetAbilityColorMode(0);
    EXPECT_EQ(called, true);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetAbilityColorMode_0300
 * @tc.name: SetAbilityColorMode
 * @tc.desc: Test SetAbilityColorMode with null callback.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetAbilityColorMode_0300,
    Function | MediumTest | Level1)
{
    context_->abilityConfigUpdateCallback_ = nullptr;
    context_->SetAbilityColorMode(0);
    // Should not crash, callback is null
    EXPECT_EQ(context_->abilityConfigUpdateCallback_, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_AddCompletionHandler_0100
 * @tc.name: AddCompletionHandler
 * @tc.desc: Test AddCompletionHandler with duplicate requestId returns ERR_OK and does not add duplicate.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_AddCompletionHandler_0100,
    Function | MediumTest | Level1)
{
    std::string requestId = "dup_request_id";
    OnRequestResult onRequestSucc = [](const AppExecFwk::ElementName&, const std::string&) {};
    OnRequestResult onRequestFail = [](const AppExecFwk::ElementName&, const std::string&) {};
    auto ret = context_->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(context_->onRequestResults_.size(), 1u);
    ret = context_->AddCompletionHandler(requestId, onRequestSucc, onRequestFail);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(context_->onRequestResults_.size(), 1u);
    context_->onRequestResults_.clear();
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetRestoreEnabled_0100
 * @tc.name: SetRestoreEnabled and GetRestoreEnabled
 * @tc.desc: Test SetRestoreEnabled and GetRestoreEnabled with normal context.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetRestoreEnabled_0100,
    Function | MediumTest | Level1)
{
    EXPECT_EQ(context_->GetRestoreEnabled(), false);
    context_->SetRestoreEnabled(true);
    EXPECT_EQ(context_->GetRestoreEnabled(), true);
    context_->SetRestoreEnabled(false);
    EXPECT_EQ(context_->GetRestoreEnabled(), false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetRestoreEnabled_0200
 * @tc.name: SetRestoreEnabled when isHook_
 * @tc.desc: Test SetRestoreEnabled is skipped when context is hook module.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetRestoreEnabled_0200,
    Function | MediumTest | Level1)
{
    context_->isHook_ = true;
    context_->restoreEnabled_.store(false);
    context_->SetRestoreEnabled(true);
    EXPECT_EQ(context_->GetRestoreEnabled(), false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_OnAbilityResultInner_0100
 * @tc.name: OnAbilityResultInner
 * @tc.desc: Test OnAbilityResultInner with registered callback, verifying isInner is true.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_OnAbilityResultInner_0100,
    Function | MediumTest | Level1)
{
    int requestCode = TEST_REQUEST_CODE;
    bool innerReceived = false;
    RuntimeTask task = [&innerReceived](int32_t code, const AAFwk::Want& want, bool isInner) {
        innerReceived = isInner;
    };
    context_->resultCallbacks_.emplace(std::make_pair(requestCode, std::move(task)));
    AAFwk::Want resultData;
    context_->OnAbilityResultInner(requestCode, 0, resultData);
    EXPECT_EQ(innerReceived, true);
    EXPECT_EQ(context_->resultCallbacks_.size(), 0u);
}

/**
 * @tc.number: AbilityContextImplThirdTest_OnAbilityResult_0100
 * @tc.name: OnAbilityResult
 * @tc.desc: Test OnAbilityResult with registered callback, verifying isInner is false.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_OnAbilityResult_0100,
    Function | MediumTest | Level1)
{
    int requestCode = TEST_REQUEST_CODE;
    bool innerReceived = true;
    RuntimeTask task = [&innerReceived](int32_t code, const AAFwk::Want& want, bool isInner) {
        innerReceived = isInner;
    };
    context_->resultCallbacks_.emplace(std::make_pair(requestCode, std::move(task)));
    AAFwk::Want resultData;
    context_->OnAbilityResult(requestCode, 0, resultData);
    EXPECT_EQ(innerReceived, false);
    EXPECT_EQ(context_->resultCallbacks_.size(), 0u);
}

/**
 * @tc.number: AbilityContextImplThirdTest_OnAbilityResult_0200
 * @tc.name: OnAbilityResult
 * @tc.desc: Test OnAbilityResult with no registered callback.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_OnAbilityResult_0200,
    Function | MediumTest | Level1)
{
    int requestCode = TEST_REQUEST_CODE;
    AAFwk::Want resultData;
    context_->OnAbilityResult(requestCode, 0, resultData);
    EXPECT_EQ(context_->resultCallbacks_.size(), 0u);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetWeakSessionToken_0100
 * @tc.name: SetWeakSessionToken and GetSessionToken
 * @tc.desc: Test SetWeakSessionToken with valid token and GetSessionToken.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetWeakSessionToken_0100,
    Function | MediumTest | Level1)
{
    sptr<IRemoteObject> token = new (std::nothrow) IPCObjectStub();
    ASSERT_NE(token, nullptr);
    context_->SetWeakSessionToken(token);
    auto result = context_->GetSessionToken();
    EXPECT_EQ(result, token);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetWeakSessionToken_0200
 * @tc.name: SetWeakSessionToken and GetSessionToken
 * @tc.desc: Test GetSessionToken returns nullptr when no token set.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetWeakSessionToken_0200,
    Function | MediumTest | Level1)
{
    context_->SetWeakSessionToken(nullptr);
    auto result = context_->GetSessionToken();
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_SetOnNewWantSkipScenarios_0100
 * @tc.name: SetOnNewWantSkipScenarios
 * @tc.desc: Test SetOnNewWantSkipScenarios calls AbilityManagerClient without crash and returns ErrCode.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_SetOnNewWantSkipScenarios_0100,
    Function | MediumTest | Level1)
{
    sptr<IRemoteObject> token = new (std::nothrow) IPCObjectStub();
    context_->SetToken(token);
    auto ret = context_->SetOnNewWantSkipScenarios(0);
    EXPECT_EQ(ret, ERR_OK);
    ret = context_->SetOnNewWantSkipScenarios(1);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_NotifyCancelGamePreLaunch_0100
 * @tc.name: NotifyCancelGamePreLaunch
 * @tc.desc: Test NotifyCancelGamePreLaunch calls AbilityManagerClient and returns ERR_OK.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_NotifyCancelGamePreLaunch_0100,
    Function | MediumTest | Level1)
{
    sptr<IRemoteObject> token = new (std::nothrow) IPCObjectStub();
    context_->SetToken(token);
    auto ret = context_->NotifyCancelGamePreLaunch();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_NotifyCompleteGamePreLaunch_0100
 * @tc.name: NotifyCompleteGamePreLaunch
 * @tc.desc: Test NotifyCompleteGamePreLaunch calls AbilityManagerClient and returns ERR_OK.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_NotifyCompleteGamePreLaunch_0100,
    Function | MediumTest | Level1)
{
    sptr<IRemoteObject> token = new (std::nothrow) IPCObjectStub();
    context_->SetToken(token);
    auto ret = context_->NotifyCompleteGamePreLaunch();
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_CreateModuleContext_0100
 * @tc.name: CreateModuleContext
 * @tc.desc: Test CreateModuleContext with bundle name and module name when stageContext_ is null.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_CreateModuleContext_0100,
    Function | MediumTest | Level1)
{
    auto result = context_->CreateModuleContext(TEST_BUNDLE_NAME, TEST_MODULE_NAME);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_CreateModuleContext_0200
 * @tc.name: CreateModuleContext
 * @tc.desc: Test CreateModuleContext with only module name when stageContext_ is null.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_CreateModuleContext_0200,
    Function | MediumTest | Level1)
{
    auto result = context_->CreateModuleContext(TEST_MODULE_NAME);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_CreateBundleContext_0100
 * @tc.name: CreateBundleContext
 * @tc.desc: Test CreateBundleContext when stageContext_ is null.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_CreateBundleContext_0100,
    Function | MediumTest | Level1)
{
    auto result = context_->CreateBundleContext(TEST_BUNDLE_NAME);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_CreateModuleResourceManager_0100
 * @tc.name: CreateModuleResourceManager
 * @tc.desc: Test CreateModuleResourceManager when stageContext_ is null.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_CreateModuleResourceManager_0100,
    Function | MediumTest | Level1)
{
    auto result = context_->CreateModuleResourceManager(TEST_BUNDLE_NAME, TEST_MODULE_NAME);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_CreateSystemHspModuleResourceManager_0100
 * @tc.name: CreateSystemHspModuleResourceManager
 * @tc.desc: Test CreateSystemHspModuleResourceManager when stageContext_ is null.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_CreateSystemHspModuleResourceManager_0100,
    Function | MediumTest | Level1)
{
    std::shared_ptr<Global::Resource::ResourceManager> resourceMgr = nullptr;
    auto ret = context_->CreateSystemHspModuleResourceManager(TEST_BUNDLE_NAME, TEST_MODULE_NAME, resourceMgr);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: AbilityContextImplThirdTest_CreateAreaModeContext_0100
 * @tc.name: CreateAreaModeContext
 * @tc.desc: Test CreateAreaModeContext when stageContext_ is null.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_CreateAreaModeContext_0100,
    Function | MediumTest | Level1)
{
    auto result = context_->CreateAreaModeContext(0);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_GetArea_0100
 * @tc.name: GetArea
 * @tc.desc: Test GetArea when stageContext_ is null returns EL_DEFAULT.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_GetArea_0100, Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    int area = context_->GetArea();
    // EL_DEFAULT = 1, defined in ContextImpl
    EXPECT_EQ(area, 1);
}

/**
 * @tc.number: AbilityContextImplThirdTest_GetArea_0200
 * @tc.name: GetArea
 * @tc.desc: Test GetArea when stageContext_ is set.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_GetArea_0200, Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    int mode = 2;
    context_->SwitchArea(mode);
    int area = context_->GetArea();
    EXPECT_EQ(area, mode);
}

/**
 * @tc.number: AbilityContextImplThirdTest_StartExtensionAbilityWithExtensionType_0100
 * @tc.name: StartExtensionAbilityWithExtensionType
 * @tc.desc: Test StartExtensionAbilityWithExtensionType with SERVICE type.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_StartExtensionAbilityWithExtensionType_0100,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    want.SetElementName(TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    auto result = context_->StartExtensionAbilityWithExtensionType(
        want, AppExecFwk::ExtensionAbilityType::SERVICE);
    // mock returns ERR_OK
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_StopExtensionAbilityWithExtensionType_0100
 * @tc.name: StopExtensionAbilityWithExtensionType
 * @tc.desc: Test StopExtensionAbilityWithExtensionType with SERVICE type.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_StopExtensionAbilityWithExtensionType_0100,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    want.SetElementName(TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    auto result = context_->StopExtensionAbilityWithExtensionType(
        want, AppExecFwk::ExtensionAbilityType::SERVICE);
    // mock returns ERR_OK
    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_NotifyBindingObjectConfigUpdate_0100
 * @tc.name: NotifyBindingObjectConfigUpdate
 * @tc.desc: Test NotifyBindingObjectConfigUpdate when config_ is nullptr, callback should not be invoked.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_NotifyBindingObjectConfigUpdate_0100,
    Function | MediumTest | Level1)
{
    bool callbackInvoked = false;
    context_->bindingObjectConfigUpdateCallback_ =
        [&callbackInvoked](std::shared_ptr<AppExecFwk::Configuration> config) {
            callbackInvoked = true;
        };
    context_->config_ = nullptr;
    context_->NotifyBindingObjectConfigUpdate();
    // config_ is null, so callback should NOT be called
    EXPECT_EQ(callbackInvoked, false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_NotifyBindingObjectConfigUpdate_0200
 * @tc.name: NotifyBindingObjectConfigUpdate
 * @tc.desc: Test NotifyBindingObjectConfigUpdate with config and valid callback, callback is invoked.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_NotifyBindingObjectConfigUpdate_0200,
    Function | MediumTest | Level1)
{
    bool callbackInvoked = false;
    std::shared_ptr<AppExecFwk::Configuration> receivedConfig;
    auto testConfig = std::make_shared<AppExecFwk::Configuration>();
    testConfig->AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_LANGUAGE, "en");
    context_->bindingObjectConfigUpdateCallback_ =
        [&callbackInvoked, &receivedConfig](std::shared_ptr<AppExecFwk::Configuration> config) {
            callbackInvoked = true;
            receivedConfig = config;
        };
    context_->config_ = testConfig;
    context_->NotifyBindingObjectConfigUpdate();
    EXPECT_EQ(callbackInvoked, true);
    EXPECT_EQ(receivedConfig, testConfig);
}

/**
 * @tc.number: AbilityContextImplThirdTest_StartSelfUIAbilityInCurrentProcess_0100
 * @tc.name: StartSelfUIAbilityInCurrentProcess
 * @tc.desc: Test StartSelfUIAbilityInCurrentProcess with hasOptions false.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_StartSelfUIAbilityInCurrentProcess_0100,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    want.SetElementName(TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    std::string specifiedFlag = "testFlag";
    AAFwk::StartOptions startOptions;
    auto ret = context_->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, false);
    // AbilityManagerClient mock returns ERR_OK
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_StartSelfUIAbilityInCurrentProcess_0200
 * @tc.name: StartSelfUIAbilityInCurrentProcess
 * @tc.desc: Test StartSelfUIAbilityInCurrentProcess with hasOptions true.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_StartSelfUIAbilityInCurrentProcess_0200,
    Function | MediumTest | Level1)
{
    AAFwk::Want want;
    want.SetElementName(TEST_BUNDLE_NAME, TEST_ABILITY_NAME);
    std::string specifiedFlag;
    AAFwk::StartOptions startOptions;
    auto ret = context_->StartSelfUIAbilityInCurrentProcess(want, specifiedFlag, startOptions, true);
    // AbilityManagerClient mock returns ERR_OK
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.number: AbilityContextImplThirdTest_GetProcessName_0100
 * @tc.name: GetProcessName
 * @tc.desc: Test GetProcessName with stageContext_ set returns correct value.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_GetProcessName_0100,
    Function | MediumTest | Level1)
{
    context_->SetStageContext(mock_);
    EXPECT_EQ(context_->GetProcessName(), "processName");
}

/**
 * @tc.number: AbilityContextImplThirdTest_GetProcessName_0200
 * @tc.name: GetProcessName
 * @tc.desc: Test GetProcessName with stageContext_ null returns empty string.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_GetProcessName_0200,
    Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    EXPECT_EQ(context_->GetProcessName(), "");
}

/**
 * @tc.number: AbilityContextImplThirdTest_GetHapModuleInfo_0100
 * @tc.name: GetHapModuleInfo
 * @tc.desc: Test GetHapModuleInfo returns nullptr when stageContext_ is null.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_GetHapModuleInfo_0100,
    Function | MediumTest | Level1)
{
    context_->SetStageContext(nullptr);
    auto ret = context_->GetHapModuleInfo();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.number: AbilityContextImplThirdTest_EraseUIExtension_0100
 * @tc.name: EraseUIExtension
 * @tc.desc: Test EraseUIExtension with non-existent sessionId.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_EraseUIExtension_0100,
    Function | MediumTest | Level1)
{
    context_->uiExtensionMap_.clear();
    AAFwk::Want want;
    context_->uiExtensionMap_.emplace(1, want);
    context_->EraseUIExtension(999);
    EXPECT_EQ(context_->uiExtensionMap_.size(), 1u);
    context_->EraseUIExtension(1);
    EXPECT_EQ(context_->uiExtensionMap_.size(), 0u);
}

/**
 * @tc.number: AbilityContextImplThirdTest_IsUIExtensionExist_0100
 * @tc.name: IsUIExtensionExist
 * @tc.desc: Test IsUIExtensionExist with matching and non-matching wants.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_IsUIExtensionExist_0100,
    Function | MediumTest | Level1)
{
    AAFwk::Want storedWant;
    storedWant.SetElementName(TEST_BUNDLE_NAME, TEST_MODULE_NAME, TEST_ABILITY_NAME);
    context_->uiExtensionMap_.clear();
    context_->uiExtensionMap_.emplace(1, storedWant);

    AAFwk::Want matchWant;
    matchWant.SetElementName(TEST_BUNDLE_NAME, TEST_MODULE_NAME, TEST_ABILITY_NAME);
    EXPECT_EQ(context_->IsUIExtensionExist(matchWant), true);

    AAFwk::Want noMatchWant;
    noMatchWant.SetElementName("other.bundle", "other.module", "OtherAbility");
    EXPECT_EQ(context_->IsUIExtensionExist(noMatchWant), false);
}

/**
 * @tc.number: AbilityContextImplThirdTest_OnRequestFailure_0100
 * @tc.name: OnRequestFailure with USER_CANCEL resultCode
 * @tc.desc: Test GetFailureInfoByMessage when resultCode is USER_CANCEL.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_OnRequestFailure_0100,
    Function | MediumTest | Level1)
{
    std::string message = "test message";
    int32_t failureCode = 0;
    std::string failureMessage;
    context_->GetFailureInfoByMessage(message, failureCode, failureMessage, USER_CANCEL);
    EXPECT_EQ(failureCode, 1);
    EXPECT_EQ(failureMessage, "The user canceled this startup");
}

/**
 * @tc.number: AbilityContextImplThirdTest_OnRequestFailure_0200
 * @tc.name: OnRequestFailure with user refused message
 * @tc.desc: Test GetFailureInfoByMessage when message contains "User refused redirection".
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_OnRequestFailure_0200,
    Function | MediumTest | Level1)
{
    std::string message = "User refused redirection to app";
    int32_t failureCode = 0;
    std::string failureMessage;
    context_->GetFailureInfoByMessage(message, failureCode, failureMessage, 0);
    EXPECT_EQ(failureCode, 2);
    EXPECT_EQ(failureMessage, "User refused redirection");
}

/**
 * @tc.number: AbilityContextImplThirdTest_OnRequestFailure_0300
 * @tc.name: OnRequestFailure with generic error
 * @tc.desc: Test GetFailureInfoByMessage for system malfunction case.
 */
HWTEST_F(AbilityContextImplThirdTest, AbilityContextImplThirdTest_OnRequestFailure_0300,
    Function | MediumTest | Level1)
{
    std::string message = "Some unknown error";
    int32_t failureCode = 0;
    std::string failureMessage;
    context_->GetFailureInfoByMessage(message, failureCode, failureMessage, 0);
    EXPECT_EQ(failureCode, 0);
    EXPECT_EQ(failureMessage, "A system error occurred");
}
} // namespace AppExecFwk
} // namespace OHOS
