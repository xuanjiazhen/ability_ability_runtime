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

#include "ability_manager_errors.h"
#include "app_mgr_service_inner.h"
#include "app_utils.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "mock_my_flag.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceInnerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrServiceInnerTest::SetUpTestCase(void)
{}

void AppMgrServiceInnerTest::TearDownTestCase(void)
{}

void AppMgrServiceInnerTest::SetUp()
{}

void AppMgrServiceInnerTest::TearDown()
{}

/**
 * @tc.name: SetStartMsgCustomSandboxFlag_0100
 * @tc.type: FUNC
 * @tc.Function: SetStartMsgCustomSandboxFlag
 * @tc.SubFunction: NA
 * @tc.EnvConditions: NA
 */
HWTEST_F(AppMgrServiceInnerTest, SetStartMsgCustomSandboxFlag_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SetStartMsgCustomSandboxFlag_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    AppSpawnStartMsg startMsg = {};
    uint32_t accessTokenId = MOCKTOKENID::TOKENID_TWO;
    appMgrServiceInner->SetStartMsgCustomSandboxFlag(startMsg, accessTokenId);
    EXPECT_EQ(startMsg.isCustomSandboxFlag, false);

    AAFwk::AppUtils::isStartOptionsWithAnimation_ = true;
    appMgrServiceInner->SetStartMsgCustomSandboxFlag(startMsg, accessTokenId);
    EXPECT_EQ(startMsg.isCustomSandboxFlag, false);

    accessTokenId = MOCKTOKENID::TOKENID_ONE;
    appMgrServiceInner->SetStartMsgCustomSandboxFlag(startMsg, accessTokenId);
    EXPECT_EQ(startMsg.isCustomSandboxFlag, true);
    TAG_LOGI(AAFwkTag::TEST, "SetStartMsgCustomSandboxFlag_0100 end");
}
/**
 * @tc.name: IsUIExtensionLoadingForPid_NullAppRecord_0100
 * @tc.type: FUNC
 * @tc.Function: IsUIExtensionLoadingForPid
 */
HWTEST_F(AppMgrServiceInnerTest, IsUIExtensionLoadingForPid_NullAppRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionLoadingForPid_NullAppRecord_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    pid_t pid = 1234;
    auto result = appMgrServiceInner->IsUIExtensionLoadingForPid(pid, nullptr);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionLoadingForPid_NullAppRecord_0100 end");
}

/**
 * @tc.name: IsUIExtensionLoadingForPid_NoAbilities_0200
 * @tc.type: FUNC
 * @tc.Function: IsUIExtensionLoadingForPid
 */
HWTEST_F(AppMgrServiceInnerTest, IsUIExtensionLoadingForPid_NoAbilities_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionLoadingForPid_NoAbilities_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.noabilities";
    auto applicationInfo = std::make_shared<ApplicationInfo>(appInfo);
    int64_t recordId = 1;
    std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, recordId, processName);
    ASSERT_NE(appRecord, nullptr);
    pid_t pid = 1234;
    auto result = appMgrServiceInner->IsUIExtensionLoadingForPid(pid, appRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionLoadingForPid_NoAbilities_0200 end");
}

/**
 * @tc.name: IsUIExtensionLoadingForPid_LoadingTrue_0300
 * @tc.type: FUNC
 * @tc.Function: IsUIExtensionLoadingForPid
 */
HWTEST_F(AppMgrServiceInnerTest, IsUIExtensionLoadingForPid_LoadingTrue_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionLoadingForPid_LoadingTrue_0300 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.loading";
    auto applicationInfo = std::make_shared<ApplicationInfo>(appInfo);
    int64_t recordId = 1;
    std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, recordId, processName);
    ASSERT_NE(appRecord, nullptr);
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    ASSERT_NE(token, nullptr);
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(applicationInfo, nullptr);
    appRecord->hapModules_[applicationInfo->bundleName].emplace_back(moduleRecord);
    moduleRecord->AddAbility(token,
        std::make_shared<AbilityRunningRecord>(abilityInfoPtr, token, 1));
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetUIExtensionLoading(true);
    pid_t pid = 1234;
    auto result = appMgrServiceInner->IsUIExtensionLoadingForPid(pid, appRecord);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionLoadingForPid_LoadingTrue_0300 end");
}

/**
 * @tc.name: IsUIExtensionLoadingForPid_LoadingFalse_0400
 * @tc.type: FUNC
 * @tc.Function: IsUIExtensionLoadingForPid
 */
HWTEST_F(AppMgrServiceInnerTest, IsUIExtensionLoadingForPid_LoadingFalse_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionLoadingForPid_LoadingFalse_0400 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.notloading";
    auto applicationInfo = std::make_shared<ApplicationInfo>(appInfo);
    int64_t recordId = 1;
    std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, recordId, processName);
    ASSERT_NE(appRecord, nullptr);
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    ASSERT_NE(token, nullptr);
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(applicationInfo, nullptr);
    appRecord->hapModules_[applicationInfo->bundleName].emplace_back(moduleRecord);
    moduleRecord->AddAbility(token,
        std::make_shared<AbilityRunningRecord>(abilityInfoPtr, token, 1));
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetUIExtensionLoading(false);
    pid_t pid = 1234;
    auto result = appMgrServiceInner->IsUIExtensionLoadingForPid(pid, appRecord);
    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionLoadingForPid_LoadingFalse_0400 end");
}

/**
 * @tc.name: IsUIExtensionLoadingForPid_MixedAbilities_0500
 * @tc.type: FUNC
 * @tc.Function: IsUIExtensionLoadingForPid
 */
HWTEST_F(AppMgrServiceInnerTest, IsUIExtensionLoadingForPid_MixedAbilities_0500, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionLoadingForPid_MixedAbilities_0500 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.mixed";
    auto applicationInfo = std::make_shared<ApplicationInfo>(appInfo);
    int64_t recordId = 1;
    std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, recordId, processName);
    ASSERT_NE(appRecord, nullptr);
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(applicationInfo, nullptr);
    appRecord->hapModules_[applicationInfo->bundleName].emplace_back(moduleRecord);
    // ability1: UIExtension loading=false
    AbilityInfo abilityInfo1;
    abilityInfo1.type = AbilityType::EXTENSION;
    abilityInfo1.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityInfoPtr1 = std::make_shared<AbilityInfo>(abilityInfo1);
    sptr<IRemoteObject> token1 = new (std::nothrow) MockAbilityToken();
    moduleRecord->AddAbilityRecord(token1,
        std::make_shared<AbilityRunningRecord>(abilityInfoPtr1, token1, 1));
    // ability2: UIExtension loading=true
    AbilityInfo abilityInfo2;
    abilityInfo2.type = AbilityType::EXTENSION;
    abilityInfo2.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityInfoPtr2 = std::make_shared<AbilityInfo>(abilityInfo2);
    sptr<IRemoteObject> token2 = new (std::nothrow) MockAbilityToken();
    moduleRecord->AddAbilityRecord(token2,
        std::make_shared<AbilityRunningRecord>(abilityInfoPtr2, token2, 2));
    auto abilityRecord2 = appRecord->GetAbilityRunningRecordByToken(token2);
    ASSERT_NE(abilityRecord2, nullptr);
    abilityRecord2->SetUIExtensionLoading(true);
    pid_t pid = 1234;
    auto result = appMgrServiceInner->IsUIExtensionLoadingForPid(pid, appRecord);
    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::TEST, "IsUIExtensionLoadingForPid_MixedAbilities_0500 end");
}

/**
 * @tc.name: KillProcessByPidInner_Loading_ReturnForeground_0200
 * @tc.type: FUNC
 * @tc.Function: KillProcessByPidInner
 */
HWTEST_F(AppMgrServiceInnerTest, KillProcessByPidInner_Loading_ReturnForeground_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessByPidInner_Loading_ReturnForeground_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.killloading";
    auto applicationInfo = std::make_shared<ApplicationInfo>(appInfo);
    int64_t recordId = 1;
    std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, recordId, processName);
    ASSERT_NE(appRecord, nullptr);
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(applicationInfo, nullptr);
    appRecord->hapModules_[applicationInfo->bundleName].emplace_back(moduleRecord);
    moduleRecord->AddAbility(token,
        std::make_shared<AbilityRunningRecord>(abilityInfoPtr, token, 1));
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetUIExtensionLoading(true);
    pid_t pid = 1234;
    auto ret = appMgrServiceInner->KillProcessByPidInner(
        pid, "ReasonTdd", "KillReasonTdd", appRecord, false);
    EXPECT_EQ(ret, AAFwk::ERR_KILL_APP_WHILE_FOREGROUND);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessByPidInner_Loading_ReturnForeground_0200 end");
}

/**
 * @tc.name: KillProcessByPidInner_NotLoading_Proceed_0300
 * @tc.type: FUNC
 * @tc.Function: KillProcessByPidInner
 */
HWTEST_F(AppMgrServiceInnerTest, KillProcessByPidInner_NotLoading_Proceed_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessByPidInner_NotLoading_Proceed_0300 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    ApplicationInfo appInfo;
    appInfo.bundleName = "com.test.killnotloading";
    auto applicationInfo = std::make_shared<ApplicationInfo>(appInfo);
    int64_t recordId = 1;
    std::string processName = "test_process";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo, recordId, processName);
    ASSERT_NE(appRecord, nullptr);
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(applicationInfo, nullptr);
    appRecord->hapModules_[applicationInfo->bundleName].emplace_back(moduleRecord);
    moduleRecord->AddAbility(token,
        std::make_shared<AbilityRunningRecord>(abilityInfoPtr, token, 1));
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetUIExtensionLoading(false);
    // pid that does not exist, kill will fail with ERR_KILL_PROCESS_NOT_EXIST
    pid_t pid = 99999999;
    auto ret = appMgrServiceInner->KillProcessByPidInner(
        pid, "ReasonTdd", "KillReasonTdd", appRecord, false);
    EXPECT_EQ(ret, AAFwk::ERR_KILL_PROCESS_NOT_EXIST);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessByPidInner_NotLoading_Proceed_0300 end");
}

/**
 * @tc.name: AfterLoadAbility_UIExtension_Reuse_SetLoading_0600
 * @tc.type: FUNC
 * @tc.Function: AfterLoadAbility
 */
HWTEST_F(AppMgrServiceInnerTest, AfterLoadAbility_UIExtension_Reuse_SetLoading_0600, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AfterLoadAbility_UIExtension_Reuse_SetLoading_0600 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo info;
    std::string processName = "test_process";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(
            std::make_shared<ApplicationInfo>(), processName, info, "");
    ASSERT_NE(appRecord, nullptr);
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = new (std::nothrow) MockAbilityToken();
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(
        std::make_shared<ApplicationInfo>(), nullptr);
    appRecord->hapModules_[""].emplace_back(moduleRecord);
    moduleRecord->AddAbilityRecord(loadParam->token,
        std::make_shared<AbilityRunningRecord>(abilityInfoPtr, loadParam->token, 1));
    appMgrServiceInner->taskHandler_ = nullptr;
    appMgrServiceInner->AfterLoadAbility(appRecord, abilityInfoPtr, loadParam, true);
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(loadParam->token);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_TRUE(abilityRecord->IsUIExtensionLoading());
    TAG_LOGI(AAFwkTag::TEST, "AfterLoadAbility_UIExtension_Reuse_SetLoading_0600 end");
}

/**
 * @tc.name: AfterLoadAbility_UIExtension_Cold_NotSetLoading_0700
 * @tc.type: FUNC
 * @tc.Function: AfterLoadAbility
 */
HWTEST_F(AppMgrServiceInnerTest, AfterLoadAbility_UIExtension_Cold_NotSetLoading_0700, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AfterLoadAbility_UIExtension_Cold_NotSetLoading_0700 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo info;
    std::string processName = "test_process";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(
            std::make_shared<ApplicationInfo>(), processName, info, "");
    ASSERT_NE(appRecord, nullptr);
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = new (std::nothrow) MockAbilityToken();
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(
        std::make_shared<ApplicationInfo>(), nullptr);
    appRecord->hapModules_[""].emplace_back(moduleRecord);
    moduleRecord->AddAbilityRecord(loadParam->token,
        std::make_shared<AbilityRunningRecord>(abilityInfoPtr, loadParam->token, 1));
    appMgrServiceInner->taskHandler_ = nullptr;
    appMgrServiceInner->AfterLoadAbility(appRecord, abilityInfoPtr, loadParam, false);
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(loadParam->token);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_FALSE(abilityRecord->IsUIExtensionLoading());
    TAG_LOGI(AAFwkTag::TEST, "AfterLoadAbility_UIExtension_Cold_NotSetLoading_0700 end");
}

/**
 * @tc.name: AfterLoadAbility_NonUIExt_Reuse_NotSetLoading_0800
 * @tc.type: FUNC
 * @tc.Function: AfterLoadAbility
 */
HWTEST_F(AppMgrServiceInnerTest, AfterLoadAbility_NonUIExt_Reuse_NotSetLoading_0800, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AfterLoadAbility_NonUIExt_Reuse_NotSetLoading_0800 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo info;
    std::string processName = "test_process";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(
            std::make_shared<ApplicationInfo>(), processName, info, "");
    ASSERT_NE(appRecord, nullptr);
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = new (std::nothrow) MockAbilityToken();
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(
        std::make_shared<ApplicationInfo>(), nullptr);
    appRecord->hapModules_[""].emplace_back(moduleRecord);
    moduleRecord->AddAbilityRecord(loadParam->token,
        std::make_shared<AbilityRunningRecord>(abilityInfoPtr, loadParam->token, 1));
    appMgrServiceInner->taskHandler_ = nullptr;
    appMgrServiceInner->AfterLoadAbility(appRecord, abilityInfoPtr, loadParam, true);
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(loadParam->token);
    ASSERT_NE(abilityRecord, nullptr);
    EXPECT_FALSE(abilityRecord->IsUIExtensionLoading());
    TAG_LOGI(AAFwkTag::TEST, "AfterLoadAbility_NonUIExt_Reuse_NotSetLoading_0800 end");
}

/**
 * @tc.name: AfterLoadAbility_NullAppRecord_0900
 * @tc.type: FUNC
 * @tc.Function: AfterLoadAbility
 */
HWTEST_F(AppMgrServiceInnerTest, AfterLoadAbility_NullAppRecord_0900, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AfterLoadAbility_NullAppRecord_0900 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    appMgrServiceInner->AfterLoadAbility(nullptr, abilityInfoPtr, loadParam, false);
    TAG_LOGI(AAFwkTag::TEST, "AfterLoadAbility_NullAppRecord_0900 end");
}

/**
 * @tc.name: UpdateExtensionState_LoadingTrue_Clear_1000
 * @tc.type: FUNC
 * @tc.Function: UpdateExtensionState
 */
HWTEST_F(AppMgrServiceInnerTest, UpdateExtensionState_LoadingTrue_Clear_1000, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateExtensionState_LoadingTrue_Clear_1000 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo info;
    std::string processName = "test_process";
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(
            applicationInfo, processName, info, "");
    ASSERT_NE(appRecord, nullptr);
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(applicationInfo, nullptr);
    appRecord->hapModules_[applicationInfo->bundleName].emplace_back(moduleRecord);
    moduleRecord->AddAbility(token,
        std::make_shared<AbilityRunningRecord>(abilityInfoPtr, token, 1));
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetUIExtensionLoading(true);
    EXPECT_TRUE(abilityRecord->IsUIExtensionLoading());
    appMgrServiceInner->UpdateExtensionState(token, ExtensionState::EXTENSION_STATE_CREATE);
    EXPECT_FALSE(abilityRecord->IsUIExtensionLoading());
    TAG_LOGI(AAFwkTag::TEST, "UpdateExtensionState_LoadingTrue_Clear_1000 end");
}

/**
 * @tc.name: UpdateExtensionState_LoadingFalse_NoChange_1100
 * @tc.type: FUNC
 * @tc.Function: UpdateExtensionState
 */
HWTEST_F(AppMgrServiceInnerTest, UpdateExtensionState_LoadingFalse_NoChange_1100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateExtensionState_LoadingFalse_NoChange_1100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo info;
    std::string processName = "test_process";
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(
            applicationInfo, processName, info, "");
    ASSERT_NE(appRecord, nullptr);
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(applicationInfo, nullptr);
    appRecord->hapModules_[applicationInfo->bundleName].emplace_back(moduleRecord);
    moduleRecord->AddAbility(token,
        std::make_shared<AbilityRunningRecord>(abilityInfoPtr, token, 1));
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetUIExtensionLoading(false);
    appMgrServiceInner->UpdateExtensionState(token, ExtensionState::EXTENSION_STATE_CREATE);
    EXPECT_FALSE(abilityRecord->IsUIExtensionLoading());
    TAG_LOGI(AAFwkTag::TEST, "UpdateExtensionState_LoadingFalse_NoChange_1100 end");
}

/**
 * @tc.name: UpdateExtensionState_NonUIExt_NotClear_1200
 * @tc.type: FUNC
 * @tc.Function: UpdateExtensionState
 */
HWTEST_F(AppMgrServiceInnerTest, UpdateExtensionState_NonUIExt_NotClear_1200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateExtensionState_NonUIExt_NotClear_1200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo info;
    std::string processName = "test_process";
    auto applicationInfo = std::make_shared<ApplicationInfo>();
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(
            applicationInfo, processName, info, "");
    ASSERT_NE(appRecord, nullptr);
    AbilityInfo abilityInfo;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.extensionAbilityType = ExtensionAbilityType::SERVICE;
    auto abilityInfoPtr = std::make_shared<AbilityInfo>(abilityInfo);
    sptr<IRemoteObject> token = new (std::nothrow) MockAbilityToken();
    auto moduleRecord = std::make_shared<ModuleRunningRecord>(applicationInfo, nullptr);
    appRecord->hapModules_[applicationInfo->bundleName].emplace_back(moduleRecord);
    moduleRecord->AddAbility(token,
        std::make_shared<AbilityRunningRecord>(abilityInfoPtr, token, 1));
    auto abilityRecord = appRecord->GetAbilityRunningRecordByToken(token);
    ASSERT_NE(abilityRecord, nullptr);
    abilityRecord->SetUIExtensionLoading(true);
    appMgrServiceInner->UpdateExtensionState(token, ExtensionState::EXTENSION_STATE_CREATE);
    // Non-UIExtension should not clear IsUIExtensionLoading
    EXPECT_TRUE(abilityRecord->IsUIExtensionLoading());
    TAG_LOGI(AAFwkTag::TEST, "UpdateExtensionState_NonUIExt_NotClear_1200 end");
}

/**
 * @tc.name: KillProcessesByPids_LowMemNotForeground_1300
 * @tc.type: FUNC
 * @tc.Function: KillProcessesByPids
 */
HWTEST_F(AppMgrServiceInnerTest, KillProcessesByPids_LowMemNotForeground_1300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessesByPids_LowMemNotForeground_1300 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    // LOW_MEMORY_KILL + !shouldKillForeground → acquire uiExtensionLoadLock_
    std::vector<int32_t> pids = { 99999999 };
    std::string reason = "LowMemoryKill";
    auto ret = appMgrServiceInner->KillProcessesByPids(
        pids, reason, false, true, false);
    // pid does not exist, expect ERR_KILL_PROCESS_NOT_EXIST
    EXPECT_EQ(ret, AAFwk::ERR_KILL_PROCESS_NOT_EXIST);
    TAG_LOGI(AAFwkTag::TEST, "KillProcessesByPids_LowMemNotForeground_1300 end");
}
} // namespace AppExecFwk
} // namespace OHOS