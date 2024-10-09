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
#include "app_utils.h"
#include "iservice_registry.h"
#undef private
#include "ability_manager_errors.h"
#include "appfreeze_manager.h"
#include "appspawn_util.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_bundle_installer_service.h"
#include "mock_bundle_manager_service.h"
#include "mock_ipc_skeleton.h"
#include "mock_kia_interceptor.h"
#include "mock_my_flag.h"
#include "mock_native_token.h"
#include "mock_parameters.h"
#include "mock_permission_verification.h"
#include "mock_sa_call.h"
#include "mock_system_ability_manager.h"
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
constexpr int32_t PID_1000 = 1000;
const std::string PARAM_SPECIFIED_PROCESS_FLAG = "ohoSpecifiedProcessFlag";
const std::string TEST_FLAG = "testFlag";
const std::string TEST_PROCESS_NAME = "testProcessName";
const std::string TEST_BUNDLE_NAME = "testBundleName";
constexpr int32_t BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
sptr<MockBundleInstallerService> mockBundleInstaller = new (std::nothrow) MockBundleInstallerService();
sptr<MockBundleManagerService> mockBundleMgr = new (std::nothrow) MockBundleManagerService();
}

class AppMgrServiceInnerSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    void MockBundleInstallerAndSA();
    void MockBundleInstaller();
    void InitAppInfo(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

public:
    std::shared_ptr<AbilityInfo> abilityInfo_;
    std::shared_ptr<ApplicationInfo> applicationInfo_;
    std::shared_ptr<BundleMgrHelper> bundleMgrHelper_{ nullptr };
    std::shared_ptr<Want> want_;
    sptr<MockAbilityToken> token_;
    sptr<MockAbilityToken> preToken_;
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;
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
    mockSystemAbility_ = new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
}

void AppMgrServiceInnerSecondTest::MockBundleInstallerAndSA()
{
    auto mockGetBundleInstaller = []() { return mockBundleInstaller; };
    auto mockGetSystemAbility = [bms = mockBundleMgr, saMgr = iSystemAbilityMgr_](int32_t systemAbilityId) {
        if (systemAbilityId == BUNDLE_MGR_SERVICE_SYS_ABILITY_ID) {
            return bms->AsObject();
        } else {
            return saMgr->GetSystemAbility(systemAbilityId);
        }
    };
    EXPECT_CALL(*mockBundleMgr, GetBundleInstaller()).WillOnce(testing::Invoke(mockGetBundleInstaller));
    EXPECT_CALL(*mockSystemAbility_, CheckSystemAbility(testing::_))
        .WillRepeatedly(testing::Invoke(mockGetSystemAbility));
}

void AppMgrServiceInnerSecondTest::MockBundleInstaller()
{
    auto mockGetBundleInstaller = []() { return mockBundleInstaller; };
    EXPECT_CALL(*mockBundleMgr, GetBundleInstaller()).WillOnce(testing::Invoke(mockGetBundleInstaller));
}

void AppMgrServiceInnerSecondTest::TearDown()
{
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
}

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
 * @tc.name: GetAllRunningInstanceKeysBySelf_0100
 * @tc.desc: Test GetAllRunningInstanceKeysBySelf
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, GetAllRunningInstanceKeysBySelf_0100,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysBySelf_0100 start");
    RunningMultiAppInfo info;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::vector<std::string> instanceKeys;
    auto ret = appMgrServiceInner->GetAllRunningInstanceKeysBySelf(instanceKeys);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysBySelf_0100 end");
}

/**
 * @tc.name: GetAllRunningInstanceKeysByBundleName_0100
 * @tc.desc: Test GetAllRunningInstanceKeysByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, GetAllRunningInstanceKeysByBundleName_0100,
    TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleName_0100 start");
    RunningMultiAppInfo info;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "testBundleName";
    std::vector<std::string> instanceKeys;
    auto ret = appMgrServiceInner->GetAllRunningInstanceKeysByBundleName(bundleName, instanceKeys);
    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "GetAllRunningInstanceKeysByBundleName_0100 end");
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
 * @tc.name: AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100
 * @tc.desc: Test JudgeSelfCalledByToken
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    PageStateData pageStateData;
    auto ret = appMgrServiceInner->JudgeSelfCalledByToken(nullptr, pageStateData);
    EXPECT_FALSE(ret);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 step1");
    ret = appMgrServiceInner->JudgeSelfCalledByToken(token_, pageStateData);
    EXPECT_FALSE(ret);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 step2");
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = TEST_PROCESS_NAME;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ret = appMgrServiceInner->JudgeSelfCalledByToken(token_, pageStateData);
    EXPECT_FALSE(ret);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 step3");
    const int32_t accessTokenId = 123; // 123 means tokenid
    applicationInfo_->accessTokenId = accessTokenId;
    IPCSkeleton::SetCallingTokenID(accessTokenId);
    ret = appMgrServiceInner->JudgeSelfCalledByToken(token_, pageStateData);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 step4");
    pageStateData.bundleName = TEST_BUNDLE_NAME;
    ret = appMgrServiceInner->JudgeSelfCalledByToken(token_, pageStateData);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 step5");
    pageStateData.moduleName = "entry";
    ret = appMgrServiceInner->JudgeSelfCalledByToken(token_, pageStateData);
    EXPECT_FALSE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 step6");
    pageStateData.abilityName = "ServiceAbility";
    ret = appMgrServiceInner->JudgeSelfCalledByToken(token_, pageStateData);
    EXPECT_TRUE(ret);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_JudgeSelfCalledByToken_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_StartChildProcessPreCheck_0100
 * @tc.desc: Test StartChildProcessPreCheck
 * @tc.type: FUNC
 */
const char* MULTI_PROCESS_MODEL = "persist.sys.abilityms.multi_process_model";
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_StartChildProcessPreCheck_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_StartChildProcessPreCheck_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    pid_t callingPid = IPCSkeleton::GetCallingPid();
    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = TEST_PROCESS_NAME;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ChildProcessRequest childRequest;
    auto childProcessRecord = std::make_shared<ChildProcessRecord>(appRecord->GetRecordId(), childRequest, appRecord);
    childProcessRecord->SetPid(PID_1000);
    callingPid = childProcessRecord->GetPid();
    appRecord->AddChildProcessRecord(callingPid, childProcessRecord);
    auto ret = appMgrServiceInner->StartChildProcessPreCheck(callingPid, 1);
    EXPECT_EQ(ret, ERR_ALREADY_IN_CHILD_PROCESS);

    auto appRecord2 = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    auto pid = appRecord2->GetPriorityObject()->GetPid();
    ret = appMgrServiceInner->StartChildProcessPreCheck(pid, 2);
    EXPECT_EQ(ret, ERR_NOT_SUPPORT_CHILD_PROCESS);
    ret = appMgrServiceInner->StartChildProcessPreCheck(pid, 1);
    EXPECT_EQ(ret, ERR_NOT_SUPPORT_CHILD_PROCESS);

    system::SetBoolParameter(MULTI_PROCESS_MODEL, true);
    auto& utils = AAFwk::AppUtils::GetInstance();
    utils.isMultiProcessModel_.isLoaded = false;
    ret = appMgrServiceInner->StartChildProcessPreCheck(pid, 1);
    EXPECT_EQ(ret, ERR_CHILD_PROCESS_REACH_LIMIT);

    utils.maxChildProcess_.isLoaded = true;
    utils.maxChildProcess_.value = 1000000;
    ret = appMgrServiceInner->StartChildProcessPreCheck(pid, 1);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_StartChildProcessPreCheck_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_StartChildProcess_0100
 * @tc.desc: Test StartChildProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_StartChildProcess_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_StartChildProcess_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    pid_t callingPid = PID_1000;
    pid_t childPid = PID_1000;
    ChildProcessRequest request;
    auto ret = appMgrServiceInner->StartChildProcess(callingPid, childPid, request);
    EXPECT_EQ(ret, ERR_NULL_OBJECT);

    const BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = TEST_PROCESS_NAME;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    auto appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ChildProcessRequest childRequest;
    childRequest.childProcessType = 1;
    auto childProcessRecord = std::make_shared<ChildProcessRecord>(appRecord->GetRecordId(), childRequest, appRecord);
    childProcessRecord->SetPid(PID_1000 + 1);
    childPid = childProcessRecord->GetPid();
    appRecord->AddChildProcessRecord(childPid, childProcessRecord);
    auto pid = appRecord->GetPriorityObject()->GetPid();
    system::SetBoolParameter(MULTI_PROCESS_MODEL, true);
    auto& utils = AAFwk::AppUtils::GetInstance();
    utils.isMultiProcessModel_.isLoaded = false;
    utils.maxChildProcess_.isLoaded = true;
    utils.maxChildProcess_.value = 1000000;
    ret = appMgrServiceInner->StartChildProcess(pid, childPid, request);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    appRecord->GetPriorityObject()->SetPid(1000);
    pid = appRecord->GetPriorityObject()->GetPid();
    ret = appMgrServiceInner->StartChildProcess(pid, childPid, request);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    request.srcEntry = "1001";
    ret = appMgrServiceInner->StartChildProcess(pid, childPid, request);
    EXPECT_EQ(ret, ERR_APPEXECFWK_BAD_APPSPAWN_CLIENT);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_StartChildProcess_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_KillChildProcess_0100
 * @tc.desc: Test KillChildProcess
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_KillChildProcess_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillChildProcess_0100 start");
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

    appMgrServiceInner->KillChildProcess(nullptr);
    EXPECT_EQ(appRecord->GetChildProcessRecordMap().size(), 0);

    appMgrServiceInner->KillChildProcess(appRecord);
    EXPECT_EQ(appRecord->GetChildProcessRecordMap().size(), 0);

    ChildProcessRequest childRequest;
    childRequest.childProcessType = 1;
    auto childAppRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    childAppRecord->GetPriorityObject()->SetPid(1);
    auto childProcessRecord = std::make_shared<ChildProcessRecord>(childAppRecord->GetRecordId(), childRequest,
        childAppRecord);
    childProcessRecord->SetPid(1);
    auto childPid = childProcessRecord->GetPid();
    appRecord->AddChildProcessRecord(childPid, childProcessRecord);
    appMgrServiceInner->KillChildProcess(appRecord);
    EXPECT_EQ(appMgrServiceInner->killedProcessMap_.size(), 1);
    
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillChildProcess_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_CreateAbilityInfo_0100
 * @tc.desc: Test CreateAbilityInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_CreateAbilityInfo_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CreateAbilityInfo_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AbilityInfo abilityInfo;
    Want want;
    auto ret = appMgrServiceInner->CreateAbilityInfo(want, abilityInfo);
    EXPECT_EQ(ret, false);

    MockBundleInstallerAndSA();
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfo(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(false));
    ret = appMgrServiceInner->CreateAbilityInfo(want, abilityInfo);
    EXPECT_EQ(ret, true);

    MockBundleInstallerAndSA();
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfo(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(true));
    ret = appMgrServiceInner->CreateAbilityInfo(want, abilityInfo);
    EXPECT_EQ(ret, true);

    MockBundleInstallerAndSA();
    EXPECT_CALL(*mockBundleMgr, QueryAbilityInfo(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(false));
    want.SetParam(AppspawnUtil::DLP_PARAMS_INDEX, 1);
    ret = appMgrServiceInner->CreateAbilityInfo(want, abilityInfo);
    EXPECT_EQ(ret, true);

    want.SetParam(AppspawnUtil::DLP_PARAMS_INDEX, AbilityRuntime::GlobalConstant::MAX_APP_CLONE_INDEX + 1);
    MockBundleInstallerAndSA();
    EXPECT_CALL(*mockBundleMgr, GetSandboxExtAbilityInfos(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(ERR_OK));
    ret = appMgrServiceInner->CreateAbilityInfo(want, abilityInfo);
    EXPECT_EQ(ret, false);

    std::vector<AppExecFwk::ExtensionAbilityInfo> extensionInfos;
    AppExecFwk::ExtensionAbilityInfo extensionInfo;
    extensionInfos.push_back(extensionInfo);
    MockBundleInstallerAndSA();
    EXPECT_CALL(*mockBundleMgr, GetSandboxExtAbilityInfos(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(DoAll(SetArgReferee<4>(extensionInfos),
            testing::Return(ERR_APPEXECFWK_SANDBOX_INSTALL_PARAM_ERROR)));
    ret = appMgrServiceInner->CreateAbilityInfo(want, abilityInfo);
    EXPECT_EQ(ret, true);

    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CreateAbilityInfo_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyMemMgrPriorityChanged_0100
 * @tc.desc: Test NotifyMemMgrPriorityChanged
 * @tc.type: FUNC
 */
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
} // namespace AppExecFwk
} // namespace OHOS
