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
#include "iservice_registry.h"
#undef private
#include "ability_manager_errors.h"
#include "appfreeze_manager.h"
#include "hilog_tag_wrapper.h"
#include "ability_debug_response_proxy.h"
#include "appspawn_util.h"
#include "mock_ability_debug_response_stub.h"
#include "mock_ability_token.h"
#include "mock_bundle_installer_service.h"
#include "mock_bundle_manager_service.h"
#include "mock_ipc_skeleton.h"
#include "mock_my_flag.h"
#include "mock_native_token.h"
#include "mock_permission_verification.h"
#include "mock_sa_call.h"
#include "mock_system_ability_manager.h"
#include "mock_task_handler_wrap.h"
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
constexpr int32_t TEST_PID_100 = 100;
const std::string PARAM_SPECIFIED_PROCESS_FLAG = "ohoSpecifiedProcessFlag";
const std::string TEST_FLAG = "testFlag";
const std::string TEST_PROCESS_NAME = "testProcessName";
const std::string TEST_BUNDLE_NAME = "testBundleName";
constexpr const char* DEVELOPER_MODE_STATE = "const.security.developermode.state";
constexpr int32_t BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
static constexpr int64_t NANOSECONDS = 1000000000;  // NANOSECONDS mean 10^9 nano second
static constexpr int64_t MICROSECONDS = 1000000;    // MICROSECONDS mean 10^6 millias second
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
    sptr<ISystemAbilityManager> iSystemAbilityMgr_ = nullptr;
    sptr<AppExecFwk::MockSystemAbilityManager> mockSystemAbility_ = nullptr;
    void InitAppInfo(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);
    std::shared_ptr<BundleMgrHelper> bundleMgrHelper_{ nullptr };

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
    mockSystemAbility_ = new (std::nothrow) AppExecFwk::MockSystemAbilityManager();
    iSystemAbilityMgr_ = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = mockSystemAbility_;
}

void AppMgrServiceInnerSecondTest::TearDown()
{
    SystemAbilityManagerClient::GetInstance().systemAbilityManager_ = iSystemAbilityMgr_;
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
        .WillOnce(testing::Invoke(mockGetSystemAbility))
        .WillRepeatedly(testing::Invoke(mockGetSystemAbility));
}

void AppMgrServiceInnerSecondTest::MockBundleInstaller()
{
    auto mockGetBundleInstaller = []() { return mockBundleInstaller; };
    EXPECT_CALL(*mockBundleMgr, GetBundleInstaller()).WillOnce(testing::Invoke(mockGetBundleInstaller));
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
 * @tc.name: AppMgrServiceInnerSecondTest_CheckAppFault_0100
 * @tc.desc: Test CheckAppFault
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_CheckAppFault_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckAppFault_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    FaultData faultData;

    bool ret = appMgrServiceInner->CheckAppFault(appRecord, faultData);
    EXPECT_FALSE(ret);

    faultData.timeoutMarkers = "timeout";
    appMgrServiceInner->dfxTaskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("dfx_freeze_task_queue_test");
    ret = appMgrServiceInner->CheckAppFault(appRecord, faultData);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckAppFault_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_CheckAppFault_0200
 * @tc.desc: Test CheckAppFault
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_CheckAppFault_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckAppFault_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    FaultData faultData;

    appRecord->isDebugApp_ = true;
    bool ret = appMgrServiceInner->CheckAppFault(appRecord, faultData);
    EXPECT_TRUE(ret);

    appRecord->isDebugApp_ = false;
    appRecord->isAssertPause_ = true;
    ret = appMgrServiceInner->CheckAppFault(appRecord, faultData);
    EXPECT_TRUE(ret);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckAppFault_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0100
 * @tc.desc: Test KillFaultApp
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AppFaultDataBySA faultData;

    int32_t ret = appMgrServiceInner->TransformedNotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(TEST_PID_100);
    faultData.pid = TEST_PID_100;
    ret = appMgrServiceInner->TransformedNotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);

    faultData.timeoutMarkers = "timeoutMarkers";
    ret = appMgrServiceInner->TransformedNotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);

    faultData.errorObject.name = "appRecovery";
    ret = appMgrServiceInner->TransformedNotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);

    appRecord->GetPriorityObject()->SetPid(1);
    faultData.pid = 1;
    ret = appMgrServiceInner->TransformedNotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0200
 * @tc.desc: Test KillFaultApp
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    AppFaultDataBySA faultData;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);

    appRecord->GetPriorityObject()->SetPid(TEST_PID_100);
    faultData.pid = TEST_PID_100;
    faultData.faultType = FaultDataType::APP_FREEZE;
    appMgrServiceInner->dfxTaskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("dfx_freeze_task_queue_test");
    int32_t ret = appMgrServiceInner->TransformedNotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);

    appRecord->isDebugApp_ = true;
    ret = appMgrServiceInner->TransformedNotifyAppFault(faultData);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_TransformedNotifyAppFault_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_StartNativeProcessForDebugger_0100
 * @tc.desc: Test StartNativeProcessForDebugger
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_StartNativeProcessForDebugger_0100,
    TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_StartNativeProcessForDebugger_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    Want want;

    int32_t ret = appMgrServiceInner->StartNativeProcessForDebugger(want);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    want.SetParam(AppspawnUtil::DLP_PARAMS_INDEX, 1);
    ret = appMgrServiceInner->StartNativeProcessForDebugger(want);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    MockBundleInstallerAndSA();
    EXPECT_CALL(*mockBundleMgr, GetHapModuleInfo(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(true))
        .WillRepeatedly(testing::Return(true));
    ret = appMgrServiceInner->StartNativeProcessForDebugger(want);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_StartNativeProcessForDebugger_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_GetBundleNameByPid_0100
 * @tc.desc: Test GetBundleNameByPid
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_GetBundleNameByPid_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetBundleNameByPid_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::string bundleName = "testbundleName";
    int32_t uid = 101;

    int32_t ret = appMgrServiceInner->GetBundleNameByPid(TEST_PID_100, bundleName, uid);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    AppFaultDataBySA faultData;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appRecord->GetPriorityObject()->SetPid(TEST_PID_100);
    ret = appMgrServiceInner->GetBundleNameByPid(TEST_PID_100, bundleName, uid);
    EXPECT_EQ(ret, ERR_OK);

    MyFlag::flag_ = 0;
    ret = appMgrServiceInner->GetBundleNameByPid(TEST_PID_100, bundleName, uid);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_GetBundleNameByPid_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_NotifyAbilitiesAssertDebugChange_0100
 * @tc.desc: Test NotifyAbilitiesAssertDebugChange
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_NotifyAbilitiesAssertDebugChange_0100,
    TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyAbilitiesAssertDebugChange_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    bool isAssertDebug = true;

    int32_t ret = appMgrServiceInner->NotifyAbilitiesAssertDebugChange(appRecord, isAssertDebug);
    EXPECT_EQ(ret, ERR_NO_INIT);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    ret = appMgrServiceInner->NotifyAbilitiesAssertDebugChange(appRecord, isAssertDebug);
    EXPECT_EQ(ret, ERR_NO_INIT);

    auto mockStub = new (std::nothrow) MockAbilityDebugResponseStub();
    appMgrServiceInner->abilityDebugResponse_ = new AbilityDebugResponseProxy(mockStub);
    ret = appMgrServiceInner->NotifyAbilitiesAssertDebugChange(appRecord, isAssertDebug);
    EXPECT_EQ(ret, ERR_OK);

    appRecord->hapModules_.clear();
    ret = appMgrServiceInner->NotifyAbilitiesAssertDebugChange(appRecord, isAssertDebug);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_NotifyAbilitiesAssertDebugChange_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0100
 * @tc.desc: Test ApplicationTerminatedSendProcessEvent
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0100,
    TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    appMgrServiceInner->ApplicationTerminatedSendProcessEvent(appRecord);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    std::vector<BaseSharedBundleInfo> baseSharedBundleInfoList;

    appMgrServiceInner->SetRunningSharedBundleList(TEST_BUNDLE_NAME, baseSharedBundleInfoList);
    appMgrServiceInner->ApplicationTerminatedSendProcessEvent(appRecord);
    EXPECT_EQ(appMgrServiceInner->runningSharedBundleList_.size(), 0);

    MyFlag::flag_ = MyFlag::IS_SA_CALL;
    appMgrServiceInner->appDebugManager_ = nullptr;
    appMgrServiceInner->SetRunningSharedBundleList(TEST_BUNDLE_NAME, baseSharedBundleInfoList);
    appMgrServiceInner->ApplicationTerminatedSendProcessEvent(appRecord);
    EXPECT_EQ(appMgrServiceInner->runningSharedBundleList_.size(), 1);

    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->ApplicationTerminatedSendProcessEvent(appRecord);
    EXPECT_EQ(appMgrServiceInner->runningSharedBundleList_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0200
 * @tc.desc: Test ApplicationTerminatedSendProcessEvent
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0200,
    TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);

    appMgrServiceInner->appRunningManager_->ClearAppRunningRecordMap();
    appMgrServiceInner->ApplicationTerminatedSendProcessEvent(appRecord);
    EXPECT_EQ(appMgrServiceInner->appDebugManager_->debugInfos_.size(), 0);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ApplicationTerminatedSendProcessEvent_0200 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_ClearAppRunningDataForKeepAlive_0100
 * @tc.desc: Test ClearAppRunningDataForKeepAlive
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_ClearAppRunningDataForKeepAlive_0100,
    TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ClearAppRunningDataForKeepAlive_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    appMgrServiceInner->ClearAppRunningDataForKeepAlive(appRecord);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    appMgrServiceInner->ClearAppRunningDataForKeepAlive(appRecord);

    int32_t uid = 101;
    appRecord->SetUid(uid);
    appRecord->SetKeepAliveBundle(true);
    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetSingleton(true);
    appMgrServiceInner->ClearAppRunningDataForKeepAlive(appRecord);

    uid = 200000;
    appRecord->SetUid(uid);
    appMgrServiceInner->ClearAppRunningDataForKeepAlive(appRecord);

    appMgrServiceInner->currentUserId_ = 1;
    appMgrServiceInner->ClearAppRunningDataForKeepAlive(appRecord);

    appMgrServiceInner->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler("AppMgrServiceInnerSecondTest");
    appMgrServiceInner->ClearAppRunningDataForKeepAlive(appRecord);

    appRecord->SetRestartResidentProcCount(-1);
    struct timespec t;
    t.tv_sec = 0;
    t.tv_nsec = 0;
    clock_gettime(CLOCK_MONOTONIC, &t);
    int64_t systemTimeMillis = static_cast<int64_t>(((t.tv_sec) * NANOSECONDS + t.tv_nsec) / MICROSECONDS);
    appRecord->restartTimeMillis_ = systemTimeMillis + 1000;
    appMgrServiceInner->ClearAppRunningDataForKeepAlive(appRecord);
    EXPECT_EQ(appMgrServiceInner->restartResidentTaskList_.size(), 1);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ClearAppRunningDataForKeepAlive_0100 end");
}
} // namespace AppExecFwk
} // namespace OHOS
