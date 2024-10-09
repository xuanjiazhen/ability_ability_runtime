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
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_bundle_installer_service.h"
#include "mock_bundle_manager_service.h"
#include "mock_ipc_skeleton.h"
#include "mock_my_flag.h"
#include "mock_native_token.h"
#include "mock_kia_interceptor.h"
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
constexpr int32_t USER_SCALE = 200000;
const std::string PARAM_SPECIFIED_PROCESS_FLAG = "ohoSpecifiedProcessFlag";
const std::string TEST_FLAG = "testFlag";
const std::string TEST_PROCESS_NAME = "testProcessName";
const std::string TEST_BUNDLE_NAME = "testBundleName";
constexpr const char* KEY_WATERMARK_BUSINESS_NAME = "com.ohos.param.watermarkBusinessName";
constexpr const char* KEY_IS_WATERMARK_ENABLED = "com.ohos.param.isWatermarkEnabled";
constexpr const char* UIEXTENSION_ABILITY_ID = "ability.want.params.uiExtensionAbilityId";
constexpr int32_t BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
sptr<MockBundleInstallerService> mockBundleInstaller = new (std::nothrow) MockBundleInstallerService();
sptr<MockBundleManagerService> mockBundleMgr = new (std::nothrow) MockBundleManagerService();
}
int32_t g_recordId = 0;
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
class IKiaInterceptorTest : public IKiaInterceptor {
    public:
    IKiaInterceptorTest() = default;
    virtual ~IKiaInterceptorTest() = default;
    int OnIntercept(AAFwk::Want &want) override { return 0; }
};

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
 * @tc.name: AppMgrServiceInnerSecondTest_MakeKiaProcess_0100
 * @tc.desc: Test BuildEventInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_MakeKiaProcess_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeKiaProcess_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AAFwk::Want> want = nullptr;
    bool isKia = false;
    std::string watermarkBusinessName = "123";
    bool isWatermarkEnabled = false;
    bool isFileUri = false;
    std::string processName = "MakeKiaProcess";
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.value = false;
    int32_t res = appMgrServiceInner->MakeKiaProcess(want, isKia, watermarkBusinessName,
        isWatermarkEnabled, isFileUri, processName);
    EXPECT_EQ(res, ERR_OK);
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.value = true;
    res = appMgrServiceInner->MakeKiaProcess(want, isKia, watermarkBusinessName,
        isWatermarkEnabled, isFileUri, processName);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    want = std::make_shared<AAFwk::Want>();
    isFileUri = true;
    Uri uri("file://docs/DestTop/Text/test_001.txt");
    want->SetUri(uri);
    appMgrServiceInner->kiaInterceptor_ = new MockKiaInterceptor();
    EXPECT_NE(appMgrServiceInner->kiaInterceptor_, nullptr);
    want->SetParam(KEY_WATERMARK_BUSINESS_NAME, std::string("com.ohos.param.watermarkBusinessName"));
    want->SetParam(KEY_IS_WATERMARK_ENABLED, true);
    res = appMgrServiceInner->MakeKiaProcess(want, isKia, watermarkBusinessName,
        isWatermarkEnabled, isFileUri, processName);
    EXPECT_EQ(res, ERR_OK);
    want->SetParam(KEY_IS_WATERMARK_ENABLED, false);
    res = appMgrServiceInner->MakeKiaProcess(want, isKia, watermarkBusinessName,
        isWatermarkEnabled, isFileUri, processName);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeKiaProcess_0100 end");
}

/**
 * @tc.name: AppMgrServiceInnerSecondTest_MakeKiaProcess_0200
 * @tc.desc: Test BuildEventInfo
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_MakeKiaProcess_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeKiaProcess_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    bool isKia = false;
    std::string watermarkBusinessName = "123";
    bool isWatermarkEnabled = false;
    bool isFileUri = true;
    std::string processName = "MakeKiaProcess";
    Uri uri("file");
    want->SetUri(uri);
    appMgrServiceInner->kiaInterceptor_ = new MockKiaInterceptor();
    EXPECT_NE(appMgrServiceInner->kiaInterceptor_, nullptr);
    want->SetParam(KEY_WATERMARK_BUSINESS_NAME, std::string("com.ohos.param.watermarkBusinessName"));
    want->SetParam(KEY_IS_WATERMARK_ENABLED, true);
    int32_t res = appMgrServiceInner->MakeKiaProcess(want, isKia, watermarkBusinessName,
                                                    isWatermarkEnabled, isFileUri, processName);
    EXPECT_EQ(res, ERR_OK);
    Uri urk("file://docs/DestTop/Text/test_001.txt");
    want->SetUri(urk);
    appMgrServiceInner->kiaInterceptor_ = nullptr;
    res = appMgrServiceInner->MakeKiaProcess(want, isKia, watermarkBusinessName,
                                            isWatermarkEnabled, isFileUri, processName);
    EXPECT_EQ(res, ERR_OK);
    Uri urj("file");
    want->SetUri(urj);
    appMgrServiceInner->kiaInterceptor_ = nullptr;
    res = appMgrServiceInner->MakeKiaProcess(want, isKia, watermarkBusinessName,
                                            isWatermarkEnabled, isFileUri, processName);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeKiaProcess_0200 end");
}

/**
 * @tc.name: LoadAbility_001
 * @tc.desc: load ability.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_LoadAbility_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LoadAbility_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_NE(abilityInfo_, nullptr);
    EXPECT_NE(applicationInfo_, nullptr);
    appMgrServiceInner->LoadAbility(abilityInfo_, applicationInfo_, nullptr, nullptr);
    auto loadParamPtr = std::make_shared<AbilityRuntime::LoadParam>();
    loadParamPtr->token = nullptr;
    EXPECT_EQ(loadParamPtr->token, nullptr);
    appMgrServiceInner->LoadAbility(abilityInfo_, applicationInfo_, nullptr, loadParamPtr);
    loadParamPtr->token = token_;
    EXPECT_NE(loadParamPtr->token, nullptr);
    EXPECT_NE(abilityInfo_->name.c_str(), applicationInfo_->name.c_str());
    abilityInfo_->type = AppExecFwk::AbilityType::PAGE;
    appMgrServiceInner->LoadAbility(abilityInfo_, applicationInfo_, nullptr, loadParamPtr);
    appMgrServiceInner->appRunningManager_ = nullptr;
    appMgrServiceInner->LoadAbility(abilityInfo_, applicationInfo_, nullptr, loadParamPtr);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    EXPECT_NE(appMgrServiceInner->appRunningManager_, nullptr);
    appMgrServiceInner->LoadAbility(abilityInfo_, applicationInfo_, nullptr, loadParamPtr);
    int32_t userId = 101;
    appMgrServiceInner->SetEnableStartProcessFlagByUserId(userId, false);
    applicationInfo_->uid = USER_SCALE * 101;
    appMgrServiceInner->LoadAbility(abilityInfo_, applicationInfo_, nullptr, loadParamPtr);
    applicationInfo_->uid = 1;
    std::shared_ptr<AAFwk::Want> want = std::make_shared<AAFwk::Want>();
    MockBundleInstallerAndSA();
    EXPECT_CALL(*mockBundleMgr, GetBundleInfoV9(testing::_, testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(ERR_OK));
    EXPECT_CALL(*mockBundleMgr, GetHapModuleInfo(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(true));
    appMgrServiceInner->LoadAbility(abilityInfo_, applicationInfo_, want, loadParamPtr);
    want = nullptr;
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.value = true;
    appMgrServiceInner->LoadAbility(abilityInfo_, applicationInfo_, want, loadParamPtr);
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.value = false;
    want = std::make_shared<AAFwk::Want>();
    abilityInfo_->type = AppExecFwk::AbilityType::PAGE;
    appMgrServiceInner->LoadAbility(abilityInfo_, applicationInfo_, want, loadParamPtr);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LoadAbility_0100 end");
}

/**
 * @tc.name: MakeProcessName_001
 * @tc.desc: MakeProcessName.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_MakeProcessName_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeProcessName_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_NE(abilityInfo_, nullptr);
    EXPECT_NE(applicationInfo_, nullptr);
    HapModuleInfo hapModuleInfo;
    int32_t appIndex = 0;
    std::string specifiedProcessFlag = "akeProcessName";
    std::string processName = "akeProcessName";
    appMgrServiceInner->MakeProcessName(abilityInfo_, applicationInfo_, hapModuleInfo,
                                        appIndex, specifiedProcessFlag, processName);
    abilityInfo_->process = "akeProcessName";
    appMgrServiceInner->MakeProcessName(abilityInfo_, applicationInfo_, hapModuleInfo,
                                        appIndex, specifiedProcessFlag, processName);
    abilityInfo_->process = "";
    appIndex = 1;
    appMgrServiceInner->MakeProcessName(abilityInfo_, applicationInfo_, hapModuleInfo,
                                        appIndex, specifiedProcessFlag, processName);
    abilityInfo_ = nullptr;
    appMgrServiceInner->MakeProcessName(abilityInfo_, applicationInfo_, hapModuleInfo,
                                        appIndex, specifiedProcessFlag, processName);
    applicationInfo_ = nullptr;
    appMgrServiceInner->MakeProcessName(abilityInfo_, applicationInfo_, hapModuleInfo,
                                        appIndex, specifiedProcessFlag, processName);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeProcessName_0100 end");
}

/**
 * @tc.name: MakeProcessName_001
 * @tc.desc: MakeProcessName.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_MakeProcessName_0200, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeProcessName_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_NE(abilityInfo_, nullptr);
    EXPECT_NE(applicationInfo_, nullptr);
    HapModuleInfo hapModuleInfo;
    std::string processName = "akeProcessName";
    std::shared_ptr<ApplicationInfo> applicationInfo = nullptr;
    appMgrServiceInner->MakeProcessName(applicationInfo, hapModuleInfo, processName);
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = "akeProcessName";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = "";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    hapModuleInfo.isStageBasedModel = false;
    applicationInfo_->process = "";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    applicationInfo_->process = "akeProcessName";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = applicationInfo_->bundleName;
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = "akeProcessName";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    hapModuleInfo.isStageBasedModel = true;
    hapModuleInfo.process = "";
    hapModuleInfo.process = applicationInfo_->bundleName;
    hapModuleInfo.isolationMode = IsolationMode::ISOLATION_FIRST;
    appMgrServiceInner->supportIsolationMode_  = "true";
    appMgrServiceInner->MakeProcessName(applicationInfo_, hapModuleInfo, processName);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_MakeProcessName_0200 end");
}

/**
 * @tc.name: CheckIsolationMode_0100
 * @tc.desc: CheckIsolationMode.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_CheckIsolationMode_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckIsolationMode_0200 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    EXPECT_NE(abilityInfo_, nullptr);
    EXPECT_NE(applicationInfo_, nullptr);
    HapModuleInfo hapModuleInfo;
    appMgrServiceInner->supportIsolationMode_ = "true";
    hapModuleInfo.isolationMode = IsolationMode::ISOLATION_FIRST;
    bool res = appMgrServiceInner->CheckIsolationMode(hapModuleInfo);
    EXPECT_EQ(res, true);
    hapModuleInfo.isolationMode = IsolationMode::ISOLATION_ONLY;
    res = appMgrServiceInner->CheckIsolationMode(hapModuleInfo);
    hapModuleInfo.isolationMode = IsolationMode::NONISOLATION_FIRST;
    EXPECT_EQ(res, true);
    res = appMgrServiceInner->CheckIsolationMode(hapModuleInfo);
    EXPECT_EQ(res, false);
    appMgrServiceInner->supportIsolationMode_  = "false";
    res = appMgrServiceInner->CheckIsolationMode(hapModuleInfo);
    EXPECT_EQ(res, false);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_CheckIsolationMode_0100 end");
}

/**
 * @tc.name: LaunchApplication_001
 * @tc.desc: launch application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_LaunchApplication_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LaunchApplication_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->LaunchApplication(nullptr);
    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, info, "");
    g_recordId += 1;
    appMgrServiceInner->LaunchApplication(appRecord);
    int32_t uid = USER_SCALE * 101;
    appRecord->SetUid(uid);
    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetEmptyKeepAliveAppState(true);
    appRecord->isEmptyKeepAliveApp_ = true;
    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appRecord->SetEmptyKeepAliveAppState(false);
    appRecord->specifiedRequestId_ = 1;
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetEmptyKeepAliveAppState(true);
    appRecord->SetKeepAliveEnableState(false);
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetEmptyKeepAliveAppState(false);
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetEmptyKeepAliveAppState(true);
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetKeepAliveEnableState(false);
    appRecord->SetEmptyKeepAliveAppState(false);
    appMgrServiceInner->LaunchApplication(appRecord);
    Want want;
    appRecord->SetSpecifiedAbilityFlagAndWant(-1, want, "");
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetSpecifiedAbilityFlagAndWant(1, want, "");
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    appMgrServiceInner->LaunchApplication(appRecord);
    appMgrServiceInner->configuration_ = nullptr;
    appMgrServiceInner->LaunchApplication(appRecord);
    appRecord->appInfo_ = nullptr;
    appMgrServiceInner->LaunchApplication(appRecord);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_LaunchApplication_0100 end");
}

/**
 * @tc.name: KillApplication_001
 * @tc.desc: kill application.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_KillApplication_0100, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillApplication_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName = "test_bundleName";
    std::string processName = "test_processName";
    bool clearPageStack = true;
    appMgrServiceInner->appRunningManager_ = nullptr;
    int32_t res = appMgrServiceInner->KillApplication(bundleName, clearPageStack);
    EXPECT_EQ(res, ERR_NO_INIT);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    res = appMgrServiceInner->KillApplication(bundleName, clearPageStack);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_KillApplication_0100 end");
}

/**
 * @tc.name: ApplicationForegrounded_001
 * @tc.desc: application foregrounded.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerSecondTest, AppMgrServiceInnerSecondTest_ApplicationForegrounded_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ApplicationForegrounded_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->ApplicationForegrounded(99);

    std::string processName = "test_processName";
    auto appRecord = std::make_shared<AppRunningRecord>(applicationInfo_, ++g_recordId, processName);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(g_recordId, appRecord);
    appMgrServiceInner->ApplicationForegrounded(g_recordId);
    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_FOREGROUNDING);
    appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    appMgrServiceInner->ApplicationForegrounded(g_recordId);

    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_FOREGROUNDING);
    appRecord->SetState(ApplicationState::APP_STATE_READY);
    appMgrServiceInner->ApplicationForegrounded(g_recordId);

    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_FOREGROUNDING);
    appRecord->SetState(ApplicationState::APP_STATE_TERMINATED);
    appMgrServiceInner->ApplicationForegrounded(g_recordId);

    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_FOREGROUNDING);
    appRecord->SetApplicationPendingState(ApplicationPendingState::BACKGROUNDING);
    appMgrServiceInner->ApplicationForegrounded(g_recordId);

    appRecord->SetApplicationScheduleState(ApplicationScheduleState::SCHEDULE_FOREGROUNDING);
    appRecord->SetApplicationPendingState(ApplicationPendingState::FOREGROUNDING);
    int32_t callerPid = 1;
    appRecord->SetCallerPid(callerPid);
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.emplace(callerPid, appRecord);
    appRecord->GetPriorityObject()->SetPid(1);
    appMgrServiceInner->ApplicationForegrounded(g_recordId);
    TAG_LOGI(AAFwkTag::TEST, "AppMgrServiceInnerSecondTest_ApplicationForegrounded_001 end");
}

} // namespace AppExecFwk
} // namespace OHOS
