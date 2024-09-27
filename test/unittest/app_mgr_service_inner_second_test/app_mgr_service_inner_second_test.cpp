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

#define private public
#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "remote_client_manager.h"
#undef private
#include "ability_manager_errors.h"
#include "appfreeze_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "mock_ipc_skeleton.h"
#include "mock_my_flag.h"
#include "mock_native_token.h"
#include "mock_permission_verification.h"
#include "mock_sa_call.h"
#include "mock_app_mgr_service_inner.h"
#include "param.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t APP_DEBUG_INFO_PID = 0;
constexpr int32_t APP_DEBUG_INFO_UID = 0;
constexpr int32_t DEFAULT_INVAL_VALUE = -1;
const std::string PARAM_SPECIFIED_PROCESS_FLAG = "ohoSpecifiedProcessFlag";
const std::string TEST_FLAG = "testFlag";
const std::string TEST_PROCESS_NAME = "testProcessName";
const std::string TEST_BUNDLE_NAME = "testBundleName";
}

class AppMgrServiceInnerSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void InitAppInfo(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

public:
    std::shared_ptr<AbilityInfo> abilityInfo_;
    std::shared_ptr<ApplicationInfo> applicationInfo_;
    std::shared_ptr<Want> want_;
    sptr<MockAbilityToken> token_;
    sptr<MockAbilityToken> preToken_;
};

void AppMgrServiceInnerSecondTest::InitAppInfo(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ApplicationInfo applicationInfo;
    applicationInfo.name = appName;
    applicationInfo.bundleName = bundleName;
    applicationInfo_ = std::make_shared<ApplicationInfo>(applicationInfo);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::EXTENSION;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    abilityInfo_ = std::make_shared<AbilityInfo>(abilityInfo);
    want_ = std::make_shared<Want>();
    token_ = sptr<MockAbilityToken>::MakeSptr();
    preToken_ = sptr<MockAbilityToken>::MakeSptr();
}

void AppMgrServiceInnerSecondTest::SetUpTestCase(void)
{
    MockNativeToken::SetNativeToken();
}

void AppMgrServiceInnerSecondTest::TearDownTestCase(void)
{}

void AppMgrServiceInnerSecondTest::SetUp()
{
    // init test app info
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = TEST_BUNDLE_NAME;
    std::string moduleName = "entry";
    InitAppInfo(deviceName, abilityName, appName, bundleName, moduleName);
}

void AppMgrServiceInnerSecondTest::TearDown()
{}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0100
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0100 start");
    std::shared_ptr<AbilityInfo> abilityInfo = nullptr;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    AppMgrServiceInner appMgrServiceInner;
    EXPECT_EQ(appMgrServiceInner.GetSpecifiedProcessFlag(abilityInfo, want), "");
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0200
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0200 start");
    std::shared_ptr<AAFwk::Want> want = nullptr;
    AppMgrServiceInner appMgrServiceInner;
    EXPECT_EQ(appMgrServiceInner.GetSpecifiedProcessFlag(abilityInfo_, want), "");
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0200 start");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0300
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0300 start");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    want->SetParam(PARAM_SPECIFIED_PROCESS_FLAG, TEST_FLAG);
    AppMgrServiceInner appMgrServiceInner;
    EXPECT_EQ(appMgrServiceInner.GetSpecifiedProcessFlag(abilityInfo, want), "");
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0300 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0400
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0400 start");
    std::shared_ptr<AbilityInfo> abilityInfo = std::make_shared<AbilityInfo>();
    abilityInfo->type == AppExecFwk::AbilityType::PAGE;
    abilityInfo->isStageBasedModel = true;
    abilityInfo->isolationProcess = true;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    want->SetParam(PARAM_SPECIFIED_PROCESS_FLAG, TEST_FLAG);
    AppMgrServiceInner appMgrServiceInner;
    EXPECT_EQ(appMgrServiceInner.GetSpecifiedProcessFlag(abilityInfo, want), "");
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetSpecifiedProcessFlag_0400 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0100
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = "testMainProcess";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ASSERT_NE(appRecord, nullptr);
    appRecord->SetEmptyKeepAliveAppState(true);
    appRecord->SetMainProcess(true);

    appMgrServiceInner->LoadAbilityNoAppRecord(appRecord, true, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        TEST_FLAG, bundleInfo, hapModuleInfo, want_, false, false, token_);
    EXPECT_EQ(appRecord->GetSpecifiedProcessFlag(), TEST_FLAG);
    EXPECT_FALSE(appRecord->IsEmptyKeepAliveApp());
    EXPECT_FALSE(appRecord->IsMainProcess());
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0200
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = TEST_PROCESS_NAME;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ASSERT_NE(appRecord, nullptr);
    appRecord->SetEmptyKeepAliveAppState(true);

    appMgrServiceInner->LoadAbilityNoAppRecord(appRecord, true, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        "", bundleInfo, hapModuleInfo, want_, false, false, token_);
    EXPECT_EQ(appRecord->GetSpecifiedProcessFlag(), "");
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LoadAbilityNoAppRecord_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0100
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0100 start");
    int32_t appIndex = 0;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->ForceKillApplicationInner(TEST_BUNDLE_NAME, DEFAULT_INVAL_VALUE, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0200
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0200 start");
    int32_t appIndex = 0;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(1); // kill Init process
    auto ret = appMgrServiceInner->ForceKillApplicationInner(TEST_BUNDLE_NAME, DEFAULT_INVAL_VALUE, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0300
 * @tc.desc: Test GetSpecifiedProcessFlag
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0300 start");
    int32_t appIndex = 0;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(INT_MAX);

    // kill not exist pid, expect ERR_KILL_PROCESS_NOT_EXIST
    auto ret = appMgrServiceInner->ForceKillApplicationInner(TEST_BUNDLE_NAME, DEFAULT_INVAL_VALUE, appIndex);
    EXPECT_EQ(ret, ERR_KILL_PROCESS_NOT_EXIST);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ForceKillApplicationInner_0300 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0100
 * @tc.desc: Test KillProcessesByAccessTokenId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    MyFlag::flag_ = 0;
    auto ret = appMgrServiceInner->KillProcessesByAccessTokenId(0);
    EXPECT_EQ(ret, ERR_NOT_SYSTEM_APP);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0200
 * @tc.desc: Test KillProcessesByAccessTokenId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0200 start");
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->KillProcessesByAccessTokenId(0);

    //kill not exist pid
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0300
 * @tc.desc: Test KillProcessesByAccessTokenId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0300 start");
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    const int32_t accessTokenId = 123; // 123 means tokenid
    applicationInfo_->accessTokenId = accessTokenId;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->SetSpawned();
    appRecord->GetPriorityObject()->SetPid(INT_MAX);
    auto ret = appMgrServiceInner->KillProcessesByAccessTokenId(accessTokenId);

    //kill not exist pid
    EXPECT_EQ(ret, ERR_KILL_PROCESS_NOT_EXIST);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0300 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0400
 * @tc.desc: Test KillProcessesByAccessTokenId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0400, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0400 start");
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    const int32_t accessTokenId = 123;
    applicationInfo_->accessTokenId = accessTokenId;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->SetSpawned();
    appRecord->GetPriorityObject()->SetPid(1);
    auto ret = appMgrServiceInner->KillProcessesByAccessTokenId(accessTokenId);

    //kill init process
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillProcessesByAccessTokenId_0400 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0100
 * @tc.desc: Test KillProcessesByAccessTokenId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0100,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0100 start");
    RunningMultiAppInfo info;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->GetRunningMultiAppInfoByBundleName("", info);
    EXPECT_EQ(ret, INVALID_PARAMETERS_ERR);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0200
 * @tc.desc: Test GetRunningMultiAppInfoByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0200,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0200 start");
    RunningMultiAppInfo info;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->GetRunningMultiAppInfoByBundleName(TEST_BUNDLE_NAME, info);
    EXPECT_EQ(ret, ERR_BUNDLE_NOT_EXIST);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetRunningMultiAppInfoByBundleName_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0100
 * @tc.desc: Test GetAllChildrenProcesses
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ChildProcessRequest request;
    auto record1 = std::make_shared<ChildProcessRecord>(IPCSkeleton::GetCallingPid(), request, appRecord);
    appRecord->AddChildProcessRecord(1, record1);
    MyFlag::flag_ = MyFlag::IS_SA_CALL;

    std::vector<ChildProcessInfo> info;
    auto ret = appMgrServiceInner->GetAllChildrenProcesses(info);
    EXPECT_EQ(info.size(), 1);
    EXPECT_EQ(info[0].bundleName, TEST_BUNDLE_NAME);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0200
 * @tc.desc: Test GetAllChildrenProcesses
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    const int32_t accessTokenId = 123; // 123 means tokenid
    applicationInfo_->accessTokenId = accessTokenId;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ChildProcessRequest request;
    auto record1 = std::make_shared<ChildProcessRecord>(IPCSkeleton::GetCallingPid(), request, appRecord);
    appRecord->AddChildProcessRecord(1, record1);
    MyFlag::flag_ = 0;

    IPCSkeleton::SetCallingTokenID(accessTokenId);
    std::vector<ChildProcessInfo> info;
    auto ret = appMgrServiceInner->GetAllChildrenProcesses(info);
    EXPECT_EQ(info.size(), 1);
    EXPECT_EQ(info[0].bundleName, TEST_BUNDLE_NAME);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAllChildrenProcesses_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0100
 * @tc.desc: Test NotifyMemoryLevel
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0100 start");
    MyFlag::flag_ = 0;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->NotifyMemoryLevel(1);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0200
 * @tc.desc: Test NotifyMemoryLevel
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0100 start");
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->NotifyMemoryLevel(4); // 4 means invalid level
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0300
 * @tc.desc: Test NotifyMemoryLevel
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0100 start");
    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->NotifyMemoryLevel(0);
    EXPECT_EQ(ret, ERR_OK);
    ret = appMgrServiceInner->NotifyMemoryLevel(1);
    EXPECT_EQ(ret, ERR_OK);
    ret = appMgrServiceInner->NotifyMemoryLevel(2);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemoryLevel_0300 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyAppFault_0100
 * @tc.desc: Test NotifyAppFault
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_NotifyAppFault_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyAppFault_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(1);
    appRecord->SetState(ApplicationState::APP_STATE_TERMINATED);

    FaultData faultData;
    auto ret = appMgrServiceInner->NotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);

    appRecord->SetState(ApplicationState::APP_STATE_END);
    ret = appMgrServiceInner->NotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);

    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    // expect in appfreezeManager return OK
    AppfreezeManager::GetInstance()->CancelAppFreezeDetect(1, TEST_BUNDLE_NAME);
    ret = appMgrServiceInner->NotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyAppFault_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_BuildEventInfo_0100
 * @tc.desc: Test BuildEventInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_BuildEventInfo_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_BuildEventInfo_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto info = appMgrServiceInner->BuildEventInfo(appRecord);
    EXPECT_EQ(info.bundleName, "");
    EXPECT_EQ(info.versionName, "");

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);

    AAFwk::EventInfo eventInfo {.bundleName = applicationInfo_->name,
        .versionName = applicationInfo_->versionName,
        .versionCode = applicationInfo_->versionCode,
        .bundleType = static_cast<int32_t>(applicationInfo_->bundleType),
        .processName = appRecord->GetProcessName()
    };
    info = appMgrServiceInner->BuildEventInfo(appRecord);
    EXPECT_EQ(info.bundleName, eventInfo.bundleName);
    EXPECT_EQ(info.versionName, eventInfo.versionName);
    EXPECT_EQ(info.versionCode, eventInfo.versionCode);
    EXPECT_EQ(info.bundleType, eventInfo.bundleType);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_BuildEventInfo_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_UpdateRenderState_0100
 * @tc.desc: Test UpdateRenderState
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_UpdateRenderState_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_UpdateRenderState_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->UpdateRenderState(1, 0);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(1);
    ret = appMgrServiceInner->UpdateRenderState(INT_MAX, 0);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    auto renderRecord = std::make_shared<RenderRecord>(1, "", -1, -1, -1, appRecord);
    renderRecord->SetPid(100); // 100 means pid
    appRecord->AddRenderRecord(renderRecord);
    ret = appMgrServiceInner->UpdateRenderState(100, 1); // 100 means pid
    EXPECT_EQ(renderRecord->GetState(), 1);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_UpdateRenderState_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyMemMgrPriorityChanged_0100
 * @tc.desc: Test NotifyMemMgrPriorityChanged
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_NotifyMemMgrPriorityChanged_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemMgrPriorityChanged_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto ret = appMgrServiceInner->NotifyMemMgrPriorityChanged(nullptr);
    EXPECT_FALSE(ret);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);

    ret = appMgrServiceInner->NotifyMemMgrPriorityChanged(appRecord);
    EXPECT_FALSE(ret); // stub err
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyMemMgrPriorityChanged_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_KillApplicationByUid_0100
 * @tc.desc: Test KillApplicationByUid
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, KillApplicationByUid_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillApplicationByUid_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    int uid = 1;
    appMgrServiceInner->appRunningManager_ = nullptr;
    auto ret = appMgrServiceInner->KillApplicationByUid(TEST_BUNDLE_NAME, uid);
    EXPECT_EQ(ret, ERR_NO_INIT); //appRunningManager_ null

    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    MyFlag::flag_ = 0;
    ret = appMgrServiceInner->KillApplicationByUid(TEST_BUNDLE_NAME, uid);
    EXPECT_EQ(ret, ERR_NOT_SYSTEM_APP); //permission verification fail

    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    appMgrServiceInner->remoteClientManager_ = nullptr;
    ret = appMgrServiceInner->KillApplicationByUid(TEST_BUNDLE_NAME, uid);
    EXPECT_EQ(ret, ERR_NO_INIT); //remoteClientManager_ null

    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    ret = appMgrServiceInner->KillApplicationByUid(TEST_BUNDLE_NAME, uid);
    EXPECT_EQ(ret, ERR_OK); //unstart

    AppRunningManager appRunningManager;
    applicationInfo_->uid = uid;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(
        loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(INT_MAX);
    appRecord->appInfos_.emplace("test", applicationInfo_);
    appRunningManager.appRunningRecordMap_.emplace(1, appRecord);
    ret = appMgrServiceInner->KillApplicationByUid(TEST_BUNDLE_NAME, uid);
    EXPECT_EQ(ret, ERR_OK); //remote process exited successs

    appRecord->GetPriorityObject()->SetPid(1);
    ret = appMgrServiceInner->KillApplicationByUid(TEST_BUNDLE_NAME, uid);
    EXPECT_EQ(ret, ERR_OK); //KillProcessByPid
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillApplicationByUid_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_SendProcessExitEventTask_0100
 * @tc.desc: Test SendProcessExitEventTask
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, SendProcessExitEventTask_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_SendProcessExitEventTask_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    time_t exitTime = 0;
    int32_t count = 2;
    appMgrServiceInner->SendProcessExitEventTask(appRecord, exitTime, count);
    EXPECT_EQ(appRecord, nullptr); //appRecord null

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        bundleInfo, hapModuleInfo, want_, false);
    appRecord->priorityObject_ = nullptr;
    appMgrServiceInner->SendProcessExitEventTask(appRecord, exitTime, count);
    EXPECT_NE(appRecord, nullptr);
    EXPECT_EQ(appRecord->priorityObject_, nullptr); //priorityObject null
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_SendProcessExitEventTask_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_SendProcessExitEventTask_0200
 * @tc.desc: Test SendProcessExitEventTask
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, SendProcessExitEventTask_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_SendProcessExitEventTask_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    time_t exitTime = 0;
    int32_t count = 2;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(INT_MAX);
    appMgrServiceInner->SendProcessExitEventTask(appRecord, exitTime, count);
    EXPECT_NE(appRecord->priorityObject_, nullptr); //exitResult = true
    EXPECT_FALSE(--count <= 0);
    count = 1;
    appRecord->GetPriorityObject()->SetPid(1);
    appMgrServiceInner->SendProcessExitEventTask(appRecord, exitTime, count);
    EXPECT_TRUE(--count <= 0); //--count <= 0
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_SendProcessExitEventTask_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ClearUpApplicationData_0100
 * @tc.desc: Test ClearUpApplicationData
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, ClearUpApplicationData_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ClearUpApplicationData_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string bundleName = TEST_BUNDLE_NAME;
    int32_t callerUid = 1;
    pid_t callerPid = 1;
    int32_t appCloneIndex = 0;
    int32_t userId = DEFAULT_INVAL_VALUE;
    appMgrServiceInner->ClearUpApplicationData(bundleName, callerUid, callerPid, appCloneIndex, userId);
    EXPECT_EQ(userId, DEFAULT_INVAL_VALUE);
    callerUid = -1;
    appMgrServiceInner->ClearUpApplicationData(bundleName, callerUid, callerPid, appCloneIndex, userId);
    EXPECT_EQ(userId, DEFAULT_INVAL_VALUE);
    userId = 1;
    appMgrServiceInner->ClearUpApplicationData(bundleName, callerUid, callerPid, appCloneIndex, userId);
    EXPECT_NE(userId, DEFAULT_INVAL_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ClearUpApplicationData_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ClearUpApplicationDataByUserId_0100
 * @tc.desc: Test ClearUpApplicationDataByUserId
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, ClearUpApplicationDataByUserId_0100, TestSize.Level0)
{
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string bundleName = TEST_BUNDLE_NAME;
    int32_t callerUid = 1;
    pid_t callerPid = -1;
    int32_t appCloneIndex = 0;
    int32_t userId = 1;
    bool isBySelf = false;
    auto res = appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex,
                                                                  userId, isBySelf);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    callerPid = 1;
    callerUid = -1;
    res = appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex, userId,
                                                             isBySelf);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    callerUid = 1;
    res = appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex, userId,
                                                             isBySelf);
    EXPECT_EQ(res, ERR_APP_CLONE_INDEX_INVALID); //clearUserGrantedPermissionState fail
    Security::AccessToken::HapInfoParams info;
    info.userID = userId;
    info.bundleName = bundleName;
    info.instIndex = 0;
    info.dlpType = 0;
    info.appIDDesc = "test_bundle";
    info.apiVersion = 1;
    Security::AccessToken::HapPolicyParams policy;
    policy.domain = "com.ohos.test";
    policy.apl = Security::AccessToken::TypeATokenAplEnum::APL_NORMAL;
    Security::AccessToken::AccessTokenIDEx fullTokenId;
    fullTokenId.tokenIdExStruct.tokenID = 1;
    Security::AccessToken::AccessTokenKit::InitHapToken(info, policy, fullTokenId);
    res = appMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex, userId,
                                                             isBySelf);
    EXPECT_EQ(res, ERR_APP_CLONE_INDEX_INVALID); //delete user data fail
    MyFlag::myFlag_ = true;
    auto mockAppMgrServiceInner = std::make_shared<MockAppMgrServiceInner>();
    EXPECT_CALL(*mockAppMgrServiceInner, KillApplicationByUserId(_, _, _, _)).Times(1).WillOnce(Return(-1));
    res = mockAppMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex,
                                                                 userId, isBySelf);
    EXPECT_EQ(res, ERR_INVALID_OPERATION); //kill app fail by KillApplicationByUserId
    isBySelf = true;
    EXPECT_CALL(*mockAppMgrServiceInner, KillApplicationSelf(_)).Times(1).WillOnce(Return(-1));
    res = mockAppMgrServiceInner->ClearUpApplicationDataByUserId(bundleName, callerUid, callerPid, appCloneIndex,
                                                                 userId, isBySelf);
    EXPECT_EQ(res, ERR_INVALID_OPERATION); //kill app fail by KillApplicationSelf
    MyFlag::reset();
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetAllRunningInstanceKeysByBundleName_0100
 * @tc.desc: Test GetAllRunningInstanceKeysByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, GetAllRunningInstanceKeysByBundleName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAllRunningInstanceKeysByBundleName_0100 start");
    std::string bundleName = "";
    std::vector<std::string> instanceKeys;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    auto res = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys);
    EXPECT_EQ(res, AAFwk::INVALID_PARAMETERS_ERR);
    bundleName = TEST_BUNDLE_NAME;
    appMgrServiceInner->remoteClientManager_ = nullptr;
    res = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    res = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys);
    EXPECT_EQ(res, AAFwk::ERR_BUNDLE_NOT_EXIST);
    MyFlag::myFlag_ = true;
    res = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys);
    EXPECT_EQ(res, AAFwk::ERR_MULTI_INSTANCE_NOT_SUPPORTED);
    MyFlag::isMultiInstance_ = true;
    appMgrServiceInner->appRunningManager_ = nullptr;
    res = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    res = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys);
    EXPECT_EQ(res, ERR_OK);
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        bundleInfo, hapModuleInfo, want_, false);
    appRecord->mainBundleName_ = "";
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.clear();
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(1, appRecord);
    res = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys);
    EXPECT_EQ(res, ERR_OK);
    appRecord->mainBundleName_ = bundleName;
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.clear();
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(1, appRecord);
    res = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys);
    EXPECT_EQ(res, ERR_OK);
    MyFlag::reset();
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAllRunningInstanceKeysByBundleName_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetRunningCloneAppInfo_0100
 * @tc.desc: Test GetRunningCloneAppInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, GetRunningCloneAppInfo_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetRunningCloneAppInfo_0100 start");
    std::string bundleName = "";
    std::vector<std::string> instanceKeys;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        bundleInfo, hapModuleInfo, want_, false);
    RunningMultiAppInfo info;
    info.mode = static_cast<int32_t>(MultiAppModeType::APP_CLONE);
    appMgrServiceInner->GetRunningCloneAppInfo(appRecord, info);
    EXPECT_EQ(info.mode, static_cast<int32_t>(MultiAppModeType::APP_CLONE));
    info.mode = static_cast<int32_t>(MultiAppModeType::MULTI_INSTANCE);
    appMgrServiceInner->GetRunningCloneAppInfo(appRecord, info);
    EXPECT_EQ(info.mode, static_cast<int32_t>(MultiAppModeType::MULTI_INSTANCE));
    info.mode = 0;
    appMgrServiceInner->GetRunningCloneAppInfo(appRecord, info);
    EXPECT_EQ(info.mode, 0);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetRunningCloneAppInfo_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_CheckAppRecordAndPriorityObject_0100
 * @tc.desc: Test CheckAppRecordAndPriorityObject
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, CheckAppRecordAndPriorityObject_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckAppRecordAndPriorityObject_0100 start");
    std::string bundleName = "";
    std::vector<std::string> instanceKeys;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto res = appMgrServiceInner->CheckAppRecordAndPriorityObject(appRecord);
    EXPECT_EQ(appRecord, nullptr);
    EXPECT_EQ(res, false);
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        bundleInfo, hapModuleInfo, want_, false);
    appRecord->priorityObject_ = nullptr;
    res = appMgrServiceInner->CheckAppRecordAndPriorityObject(appRecord);
    EXPECT_NE(appRecord, nullptr);
    EXPECT_EQ(res, false);
    appRecord->priorityObject_ = std::make_shared<PriorityObject>();
    res = appMgrServiceInner->CheckAppRecordAndPriorityObject(appRecord);
    EXPECT_EQ(res, true);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckAppRecordAndPriorityObject_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetAppCloneInfo_0100
 * @tc.desc: Test GetAppCloneInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, GetAppCloneInfo_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAppCloneInfo_0100 start");
    std::string bundleName = "";
    std::vector<std::string> instanceKeys;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    RunningMultiAppInfo info;
    appMgrServiceInner->GetAppCloneInfo(appRecord, info);
    EXPECT_EQ(appMgrServiceInner->CheckAppRecordAndPriorityObject(appRecord), false);
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME,
        bundleInfo, hapModuleInfo, want_, false);
    appMgrServiceInner->GetAppCloneInfo(appRecord, info);
    EXPECT_EQ(appMgrServiceInner->CheckAppRecordAndPriorityObject(appRecord), true);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetAppCloneInfo_0100 end");
}
} // namespace AppExecFwk
} // namespace OHOS
