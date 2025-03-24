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

#include "app_mgr_service_inner.h"
#include "app_running_record.h"
#include "app_utils.h"
#include "remote_client_manager.h"

#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "app_scheduler.h"
#include "appspawn_util.h"
#include "app_spawn_client.h"
#include "event_handler.h"
#include "mock_ability_token.h"
#include "mock_ipc_skeleton.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "param.h"
#include "parameters.h"
#include "render_state_observer_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t RECORD_ID = 1;
constexpr int32_t APP_DEBUG_INFO_PID = 0;
constexpr int32_t APP_DEBUG_INFO_UID = 0;
constexpr int32_t ROOT_UID = 0;
constexpr int32_t FUN_TEST_PID = 2;
constexpr const char* PERMISSION_PROTECT_SCREEN_LOCK_DATA_TEST = "ohos.permission.PROTECT_SCREEN_LOCK_DATA";
}
class AppMgrServiceInnerFourTest : public testing::Test {
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
};

class RenderStateObserverMock : public RenderStateObserverStub {
public:
    RenderStateObserverMock() = default;
    virtual ~RenderStateObserverMock() = default;
    void OnRenderStateChanged(const RenderStateData &renderStateData) override
    {}
};

void AppMgrServiceInnerFourTest::InitAppInfo(const std::string& deviceName,
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
}

void AppMgrServiceInnerFourTest::SetUpTestCase(void)
{}

void AppMgrServiceInnerFourTest::TearDownTestCase(void)
{}

void AppMgrServiceInnerFourTest::SetUp()
{
    // init test app info
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    InitAppInfo(deviceName, abilityName, appName, bundleName, moduleName);
}

void AppMgrServiceInnerFourTest::TearDown()
{}

/**
 * @tc.name: GetEventTypeAndMsg_0100
 * @tc.Function: GetEventTypeAndMsg
 */
HWTEST_F(AppMgrServiceInnerFourTest, GetEventTypeAndMsg_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetEventTypeAndMsg_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    std::shared_ptr<AppRunningRecord> appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    int32_t eventId = AMSEventHandler::TERMINATE_ABILITY_HALF_TIMEOUT_MSG;
    appMgrServiceInner->SendHiSysEvent(eventId, appRecord);
    eventId = AMSEventHandler::TERMINATE_APPLICATION_HALF_TIMEOUT_MSG;
    appMgrServiceInner->SendHiSysEvent(eventId, appRecord);
    eventId = AMSEventHandler::ADD_ABILITY_STAGE_INFO_HALF_TIMEOUT_MSG;
    appMgrServiceInner->SendHiSysEvent(eventId, appRecord);
    eventId = AMSEventHandler::START_PROCESS_SPECIFIED_ABILITY_HALF_TIMEOUT_MSG;
    appMgrServiceInner->SendHiSysEvent(eventId, appRecord);
    eventId = AMSEventHandler::START_SPECIFIED_ABILITY_HALF_TIMEOUT_MSG;
    appMgrServiceInner->SendHiSysEvent(eventId, appRecord);
    eventId = AMSEventHandler::START_SPECIFIED_PROCESS_HALF_TIMEOUT_MSG;
    appMgrServiceInner->SendHiSysEvent(eventId, appRecord);
    eventId = -1;
    appMgrServiceInner->SendHiSysEvent(eventId, appRecord);
    EXPECT_NE(appRecord, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetEventTypeAndMsg_0100 end");
}

/**
 * @tc.name: ProcessKia_0100
 * @tc.Function: ProcessKia
 */
HWTEST_F(AppMgrServiceInnerFourTest, ProcessKia_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "ProcessKia_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    bool isKia = false;
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    const std::string watermarkBusinessName;
    bool isWatermarkEnabled = false;
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.value = false;
    auto res = appMgrServiceInner->ProcessKia(isKia, appRecord, watermarkBusinessName, isWatermarkEnabled);
    EXPECT_EQ(res, ERR_OK);
    isKia = true;
    res = appMgrServiceInner->ProcessKia(isKia, appRecord, watermarkBusinessName, isWatermarkEnabled);
    EXPECT_EQ(res, ERR_OK);
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.isLoaded = false;
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.value = true;
    res = appMgrServiceInner->ProcessKia(isKia, appRecord, watermarkBusinessName, isWatermarkEnabled);
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.isLoaded = true;
    AAFwk::AppUtils::GetInstance().isStartOptionsWithAnimation_.value = true;
    isKia = true;
    res = appMgrServiceInner->ProcessKia(isKia, appRecord, watermarkBusinessName, isWatermarkEnabled);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    BundleInfo bundleInfo;
    std::string processName = "test_processName";
    appRecord =
        appMgrServiceInner->appRunningManager_->CreateAppRunningRecord(applicationInfo_, processName, bundleInfo, "");
    res = appMgrServiceInner->ProcessKia(isKia, appRecord, watermarkBusinessName, isWatermarkEnabled);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "ProcessKia_0100 end");
}

/**
 * @tc.name: IsMainProcess_0100
 * @tc.Function: IsMainProcess
 */
HWTEST_F(AppMgrServiceInnerFourTest, IsMainProcess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    HapModuleInfo hapModuleInfo;
    hapModuleInfo.moduleName = "module123";
    applicationInfo_->process = "processName2";
    EXPECT_EQ(appMgrServiceInner->IsMainProcess(applicationInfo_, "processName2"), true);
    EXPECT_EQ(appMgrServiceInner->IsMainProcess(applicationInfo_, "processName3"), false);
    TAG_LOGI(AAFwkTag::TEST, "IsMainProcess_0100 end");
}

/**
 * @tc.name: CheckLoadAbilityConditions_0100
 * @tc.Function: CheckLoadAbilityConditions
 */
HWTEST_F(AppMgrServiceInnerFourTest, CheckLoadAbilityConditions_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckLoadAbilityConditions_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    OHOS::sptr<IRemoteObject> token = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(token, nullptr);
    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, nullptr, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, abilityInfo_, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, nullptr, applicationInfo_);

    appMgrServiceInner->CheckLoadAbilityConditions(token, nullptr, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, nullptr);

    appMgrServiceInner->CheckLoadAbilityConditions(nullptr, abilityInfo_, applicationInfo_);

    appMgrServiceInner->CheckLoadAbilityConditions(token, nullptr, applicationInfo_);

    auto res = appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, applicationInfo_);
    EXPECT_EQ(res, true);
    abilityInfo_->name = "";
    applicationInfo_->name = "";
    res = appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, applicationInfo_);
    EXPECT_EQ(res, false);
    applicationInfo_->name = "applicationInfo";
    abilityInfo_->name = "";
    res = appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, applicationInfo_);
    EXPECT_EQ(res, false);
    abilityInfo_->name = "abilityInfoName";
    applicationInfo_->name = "";
    res = appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, applicationInfo_);
    EXPECT_EQ(res, false);
    abilityInfo_->name = "abilityInfoName";
    applicationInfo_->name = "applicationInfo";
    abilityInfo_->applicationName = "applicationInfo";
    res = appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, applicationInfo_);
    EXPECT_EQ(res, true);
    abilityInfo_->applicationName = "applicationInfo";
    applicationInfo_->name = "applicationInfoName";
    res = appMgrServiceInner->CheckLoadAbilityConditions(token, abilityInfo_, applicationInfo_);
    EXPECT_EQ(res, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckLoadAbilityConditions_0100 end");
}

/**
 * @tc.name: UpdateApplicationInfoInstalled_0100
 * @tc.Function: UpdateApplicationInfoInstalled
 */
HWTEST_F(AppMgrServiceInnerFourTest, UpdateApplicationInfoInstalled_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_0100 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);
    std::string bundleName;
    int uid = 0;
    std::string moduleName;
    appMgrServiceInner->appRunningManager_ = nullptr;
    auto res = appMgrServiceInner->UpdateApplicationInfoInstalled(bundleName, uid, moduleName);
    EXPECT_EQ(res, ERR_NO_INIT);
    appMgrServiceInner->appRunningManager_ = std::make_shared<AppRunningManager>();
    IPCSkeleton::SetCallingUid(RECORD_ID);
    res = appMgrServiceInner->UpdateApplicationInfoInstalled(bundleName, uid, moduleName);
    EXPECT_EQ(res, ERR_PERMISSION_DENIED);
    IPCSkeleton::SetCallingUid(ROOT_UID);
    appMgrServiceInner->remoteClientManager_ = nullptr;
    res = appMgrServiceInner->UpdateApplicationInfoInstalled(bundleName, uid, moduleName);
    EXPECT_EQ(res, ERR_NO_INIT);
    appMgrServiceInner->remoteClientManager_ = std::make_shared<RemoteClientManager>();
    appMgrServiceInner->remoteClientManager_->SetBundleManagerHelper(nullptr);
    res = appMgrServiceInner->UpdateApplicationInfoInstalled(bundleName, uid, moduleName);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "UpdateApplicationInfoInstalled_0100 end");
}

/**
 * @tc.name: KillAppSelfWithInstanceKey_001
 * @tc.Function: KillAppSelfWithInstanceKey
 */
HWTEST_F(AppMgrServiceInnerFourTest, KillAppSelfWithInstanceKey_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillAppSelfWithInstanceKey_001 start");
    auto appMgrServiceInner = std::make_shared<AppMgrServiceInner>();
    EXPECT_NE(appMgrServiceInner, nullptr);

    std::string instanceKey;
    bool clearPageStack = false;
    std::string reason;
    BundleInfo bundleInfo;
    std::string processName = "com.test.bundle";
    int32_t pid = 1;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();

    int32_t ret = appMgrServiceInner->KillAppSelfWithInstanceKey(instanceKey, clearPageStack, reason);
    EXPECT_EQ(ret, ERR_OK);

    IPCSkeleton::SetCallingTokenID(FUN_TEST_PID);
    appInfo->accessTokenId = FUN_TEST_PID;
    appInfo->multiAppMode.multiAppModeType= MultiAppModeType::MULTI_INSTANCE;
    std::shared_ptr<AppRunningManager> appRunningManager = std::make_shared<AppRunningManager>();
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager->CreateAppRunningRecord(appInfo, processName, bundleInfo, "");
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->priorityObject_->SetPid(pid);
    auto recordId = AppRecordId::Create();
    appMgrServiceInner->appRunningManager_->appRunningRecordMap_.insert(std::make_pair(recordId, record));
    ret = appMgrServiceInner->KillAppSelfWithInstanceKey(instanceKey, clearPageStack, reason);
    EXPECT_EQ(ret, ERR_OK);

    appMgrServiceInner->appRunningManager_ = nullptr;
    ret = appMgrServiceInner->KillAppSelfWithInstanceKey(instanceKey, clearPageStack, reason);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "KillAppSelfWithInstanceKey_001 end");
}
} // namespace AppExecFwk
} // namespace OHOS
