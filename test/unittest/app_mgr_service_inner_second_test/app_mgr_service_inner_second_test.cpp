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
 * @tc.name: GetMultiInstanceInfo_001
 * @tc.desc: Test GetMultiInstanceInfo.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppMgrServiceInnerSecondTest, GetMultiInstanceInfo_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "GetMultiInstanceInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(
        loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    EXPECT_NE(appRecord, nullptr);

    RunningMultiAppInfo info;
    EXPECT_NO_THROW(appMgrServiceInner->GetMultiInstanceInfo(nullptr, info));
    EXPECT_EQ(info.runningMultiIntanceInfos.size(), 0);

    EXPECT_NO_THROW(appMgrServiceInner->GetMultiInstanceInfo(appRecord, info));
    EXPECT_EQ(info.runningMultiIntanceInfos.size(), 1);

    info.runningMultiIntanceInfos.emplace_back(RunningMultiInstanceInfo());
    EXPECT_NO_THROW(appMgrServiceInner->GetMultiInstanceInfo(appRecord, info));
    EXPECT_EQ(info.runningMultiIntanceInfos.size(), 2);

    EXPECT_NO_THROW(appMgrServiceInner->GetMultiInstanceInfo(appRecord, info));
    EXPECT_EQ(info.runningMultiIntanceInfos.size(), 2);

    TAG_LOGI(AAFwkTag::TEST, "GetMultiInstanceInfo_001 end");
}

/**
 * @tc.name: GetRunningMultiInstanceKeys_001
 * @tc.desc: Test GetRunningMultiInstanceKeys.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppMgrServiceInnerSecondTest, GetRunningMultiInstanceKeys_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "GetRunningMultiInstanceKeys_001 start");
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    std::vector<std::string> instanceKeys;
    AppMgrServiceInner appMgrServiceInner;
    appMgrServiceInner.GetRunningMultiInstanceKeys(appRecord, instanceKeys);
    EXPECT_TRUE(instanceKeys.empty());

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner.CreateAppRunningRecord(
        loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner.GetRunningMultiInstanceKeys(appRecord, instanceKeys);
    EXPECT_FALSE(instanceKeys.empty());

    instanceKeys.push_back(appRecord->GetInstanceKey());
    appMgrServiceInner.GetRunningMultiInstanceKeys(appRecord, instanceKeys);
    EXPECT_FALSE(instanceKeys.size() == 1);
    
    appRecord->priorityObject_ = nullptr;
    appMgrServiceInner.GetRunningMultiInstanceKeys(appRecord, instanceKeys);
    EXPECT_FALSE(instanceKeys.size() == 1 && instanceKeys[0] == appRecord->GetInstanceKey());
    TAG_LOGI(AAFwkTag::TEST, "GetRunningMultiInstanceKeys_001 end");
}

/**
 * @tc.name: UpdateAbilityState_001
 * @tc.desc: update ability state.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerSecondTest, UpdateAbilityState_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateAbilityState_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->UpdateAbilityState(nullptr, AbilityState::ABILITY_STATE_CREATE);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->UpdateAbilityState(token, AbilityState::ABILITY_STATE_CREATE);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token;
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, nullptr, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->UpdateAbilityState(token, AbilityState::ABILITY_STATE_CREATE);

    OHOS::sptr<IRemoteObject> token1 = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    loadParam->token = token1;
    std::shared_ptr<AppRunningRecord> appRecord1 = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord1, nullptr);

    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_READY);

    auto abilityRecord1 =
        appMgrServiceInner->GetAppRunningRecordByAbilityToken(token1)->GetAbilityRunningRecordByToken(token1);
    EXPECT_NE(abilityRecord1, nullptr);
    abilityRecord1->info_ = nullptr;
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_READY);
    EXPECT_EQ(abilityRecord1->GetAbilityInfo(), nullptr);

    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_CREATE);
    
    abilityRecord1 =
        appMgrServiceInner->GetAppRunningRecordByAbilityToken(token1)->GetAbilityRunningRecordByToken(token1);
    abilityRecord1->SetState(AbilityState::ABILITY_STATE_TERMINATED);
    appMgrServiceInner->UpdateAbilityState(token1, AbilityState::ABILITY_STATE_TERMINATED);
    EXPECT_EQ(abilityRecord1->GetState(), AbilityState::ABILITY_STATE_TERMINATED);

    TAG_LOGI(AAFwkTag::TEST, "UpdateAbilityState_001 end");
}

/**
 * @tc.name: KillProcessByAbilityToken_001
 * @tc.desc: kill process by ability token.
 * @tc.type: FUNC
 * @tc.require: issueI5W4S7
 */
HWTEST_F(AppMgrServiceInnerSecondTest, KillProcessByAbilityToken_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "KillProcessByAbilityToken_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    appMgrServiceInner->KillProcessByAbilityToken(nullptr);

    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    appMgrServiceInner->KillProcessByAbilityToken(token);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    std::string processName = "test_processName";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token;
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam,
        applicationInfo_, abilityInfo_, processName, bundleInfo, hapModuleInfo, want);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->KillProcessByAbilityToken(token);
    
    appRecord->GetPriorityObject()->SetPid(1);
    appRecord->SetKeepAliveEnableState(true);
    appRecord->SetEmptyKeepAliveAppState(true);
    appMgrServiceInner->KillProcessByAbilityToken(token);

    TAG_LOGI(AAFwkTag::TEST, "KillProcessByAbilityToken_001 end");
}

/**
 * @tc.name: SetOverlayInfo_001
 * @tc.desc: SetOverlayInfo.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppMgrServiceInnerSecondTest, SetOverlayInfo_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "SetOverlayInfo_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    appMgrServiceInner->remoteClientManager_ = nullptr;
    AppSpawnStartMsg startMsg;
    appMgrServiceInner->SetOverlayInfo("testBundleName", 1, startMsg);
    EXPECT_EQ(startMsg.flags, 0);
    EXPECT_EQ(startMsg.overlayInfo, "");

    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    appMgrServiceInner->SetOverlayInfo("testBundleName", 1, startMsg);
    EXPECT_EQ(startMsg.flags, 0);
    EXPECT_EQ(startMsg.overlayInfo, "");

    std::shared_ptr<BundleMgrHelper> bundleManager = nullptr;
    appMgrServiceInner->remoteClientManager_->SetBundleManagerHelper(bundleManager);
    appMgrServiceInner->SetOverlayInfo("testBundleName", 1, startMsg);
    EXPECT_EQ(startMsg.flags, 0);
    EXPECT_EQ(startMsg.overlayInfo, "");
    
    TAG_LOGI(AAFwkTag::TEST, "SetOverlayInfo_001 end");
}

/**
 * @tc.name: StartProcessVerifyPermission_001
 * @tc.desc: Test StartProcessVerifyPermission.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppMgrServiceInnerSecondTest, StartProcessVerifyPermission_001, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "StartProcessVerifyPermission_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo bundleInfo;
    bool hasAccessBundleDirReq;
    uint8_t setAllowInternet;
    uint8_t allowInternet;
    std::vector<int32_t> gids;
    bundleInfo.reqPermissions.push_back("ohos.permission.ACCESS_BUNDLE_DIR");
    appMgrServiceInner->StartProcessVerifyPermission(
        bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids);
    EXPECT_FALSE(hasAccessBundleDirReq);

    bundleInfo.reqPermissions.clear();
    bundleInfo.reqPermissions.push_back("ohos.permission.INTERNET");
    appMgrServiceInner->StartProcessVerifyPermission(
        bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids);
    EXPECT_FALSE(hasAccessBundleDirReq);

    bundleInfo.reqPermissions.clear();
    bundleInfo.reqPermissions.push_back("ohos.permission.INTERNET");
    bundleInfo.applicationInfo.accessTokenId = 1;
    appMgrServiceInner->StartProcessVerifyPermission(
        bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids);
    EXPECT_EQ(setAllowInternet, 1);
    EXPECT_EQ(allowInternet, 0);

    bundleInfo.reqPermissions.clear();
    bundleInfo.reqPermissions.push_back("ohos.permission.MANAGE_VPN");
    bundleInfo.applicationInfo.accessTokenId = 0;
    appMgrServiceInner->StartProcessVerifyPermission(
        bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids);
    EXPECT_EQ(setAllowInternet, 1);
    EXPECT_EQ(allowInternet, 0);

    bundleInfo.reqPermissions.clear();
    bundleInfo.reqPermissions.push_back("ohos.permission.ACCESS_BUNDLE_DIR");
    bundleInfo.applicationInfo.accessTokenId = 1;
    appMgrServiceInner->StartProcessVerifyPermission(
        bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids);
    EXPECT_FALSE(std::find(gids.begin(), gids.end(), 1097) != gids.end());

    bundleInfo.reqPermissions.clear();
    bundleInfo.applicationInfo.accessTokenId = 1;
    appMgrServiceInner->StartProcessVerifyPermission(
        bundleInfo, hasAccessBundleDirReq, setAllowInternet, allowInternet, gids);
    EXPECT_TRUE(std::find(gids.begin(), gids.end(), 1097) == gids.end());

    TAG_LOGI(AAFwkTag::TEST, "StartProcessVerifyPermission_001 end");
}

/**
 * @tc.name: ProcessAppDebug_0010
 * @tc.desc: Test ProcessAppDebug.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppMgrServiceInnerSecondTest, ProcessAppDebug_0010, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "ProcessAppDebug_0010 start");
    ApplicationInfo applicationInfo;
    applicationInfo.name = "hiservcie";
    applicationInfo.bundleName = "com.ix.hiservcie";
    std::shared_ptr<ApplicationInfo> applicationInfo_ = std::make_shared<ApplicationInfo>(applicationInfo);
    BundleInfo info;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord;
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    
    appMgrServiceInner->appDebugManager_ = nullptr;
    appMgrServiceInner->ProcessAppDebug(nullptr, true);
    EXPECT_EQ(appMgrServiceInner->appDebugManager_, nullptr);
    
    appMgrServiceInner->appDebugManager_ = std::make_shared<AppDebugManager>();
    appMgrServiceInner->ProcessAppDebug(nullptr, true);
    EXPECT_NE(appMgrServiceInner->appDebugManager_, nullptr);
  
    appMgrServiceInner->appDebugManager_ = nullptr;
    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    appRecord = appMgrServiceInner->CreateAppRunningRecord(loadParam, applicationInfo_, abilityInfo_,
        TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    EXPECT_NE(appRecord, nullptr);
    appMgrServiceInner->ProcessAppDebug(appRecord, true);
    EXPECT_EQ(appRecord->IsDebugApp(), false);

    appMgrServiceInner->appDebugManager_ = std::make_shared<AppDebugManager>();
    appMgrServiceInner->ProcessAppDebug(appRecord, true);
    EXPECT_EQ(appRecord->IsDebugApp(), true);

    appRecord->SetDebugApp(false);
    appMgrServiceInner->ProcessAppDebug(appRecord, true);
    EXPECT_EQ(appRecord->IsDebugApp(), true);

    appRecord->SetDebugApp(true);
    appMgrServiceInner->ProcessAppDebug(appRecord, true);
    EXPECT_EQ(appRecord->IsDebugApp(), true);

    AppDebugInfo adinfo;
    adinfo.bundleName = "com.ix.hiservcie";
    appMgrServiceInner->appDebugManager_->debugInfos_.push_back(adinfo);
    appRecord->SetDebugApp(false);
    appMgrServiceInner->ProcessAppDebug(appRecord, false);
    EXPECT_EQ(appRecord->IsDebugApp(), false);

    appRecord->SetDebugApp(true);
    appMgrServiceInner->ProcessAppDebug(appRecord, false);
    EXPECT_EQ(appRecord->IsDebugApp(), true);
    
    TAG_LOGI(AAFwkTag::TEST, "ProcessAppDebug_0010 end");
}

/**
 * @tc.name: FinishUserTest_001
 * @tc.desc: Test FinishUserTest.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(AppMgrServiceInnerSecondTest, FinishUserTest_0010, TestSize.Level0)
{
    TAG_LOGI(AAFwkTag::TEST, "FinishUserTest_0010 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    
    pid_t pid = 0;
    appMgrServiceInner->FinishUserTest("", 0, "", pid);

    std::string msg = "testmsg";
    std::string bundleName = "test_bundle_name";
    appMgrServiceInner->FinishUserTest("", 0, bundleName, pid);
    appMgrServiceInner->FinishUserTest(msg, 0, "", pid);
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);

    BundleInfo bundleInfo;
    HapModuleInfo hapModuleInfo;
    std::shared_ptr<AAFwk::Want> want;
    sptr<IRemoteObject> token = new MockAbilityToken();
    std::string processName = "test_processName";
    auto loadParam = std::make_shared<AbilityRuntime::LoadParam>();
    loadParam->token = token_;
    loadParam->preToken = preToken_;
    std::shared_ptr<AppRunningRecord> appRecord = appMgrServiceInner->CreateAppRunningRecord(
        loadParam, applicationInfo_, abilityInfo_, TEST_PROCESS_NAME, bundleInfo, hapModuleInfo, want_, false);
    EXPECT_NE(appRecord, nullptr);
    pid = appRecord->GetPriorityObject()->GetPid();
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);

    std::shared_ptr<UserTestRecord> record = std::make_shared<UserTestRecord>();
    appRecord->SetUserTestInfo(record);
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);

    appMgrServiceInner->remoteClientManager_ = nullptr;
    appMgrServiceInner->FinishUserTest(msg, 0, bundleName, pid);
    
    TAG_LOGI(AAFwkTag::TEST, "FinishUserTest_0010 end");
}
} // namespace AppExecFwk
} // namespace OHOS
