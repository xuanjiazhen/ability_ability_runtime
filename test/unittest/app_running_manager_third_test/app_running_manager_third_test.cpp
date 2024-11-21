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
#include "app_running_manager.h"
#include "app_running_record.h"
#include "child_process_record.h"
#include "app_lifecycle_deal.h"
#undef private

#include "app_record_id.h"
#include "exit_resident_process_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_application.h"
#include "mock_ability_token.h"
#include "quick_fix_callback_proxy.h"
#include "quick_fix_callback_stub.h"
#include "app_mgr_service_dump_error_code.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
using Configuration = AppExecFwk::Configuration;
namespace {
constexpr int32_t USR_ID_100 = 100;
constexpr int32_t USR_ID_101 = 101;
constexpr int32_t RECORD_ID = 1;
constexpr int32_t DEBUGINFOS_SIZE = 0;
const std::string BUNDLE_NAME = "testBundleName";
const std::string PROCESS_NAME = "testProcessName";

class QuickFixCallbackImpl : public QuickFixCallbackStub {
public:
    QuickFixCallbackImpl() = default;
    virtual ~QuickFixCallbackImpl() = default;

    void OnLoadPatchDone(int32_t resultCode, [[maybe_unused]] int32_t recordId) override
    {}

    void OnUnloadPatchDone(int32_t resultCode, [[maybe_unused]] int32_t recordId) override
    {}

    void OnReloadPageDone(int32_t resultCode, [[maybe_unused]] int32_t recordId) override
    {}
};
}

class AppRunningManagerThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

protected:
    static BundleInfo bundleInfo;
    static std::shared_ptr<ApplicationInfo> appInfo_;
    static std::shared_ptr<MockAppMgrServiceInner> appServiceInner_;
    static sptr<MockApplication> mockApp1_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AMSEventHandler> eventHandler_;
    std::shared_ptr<AppMgrServiceInner> serviceInner_;
};

BundleInfo AppRunningManagerThirdTest::bundleInfo;
std::shared_ptr<ApplicationInfo> AppRunningManagerThirdTest::appInfo_ = nullptr;
std::shared_ptr<MockAppMgrServiceInner> AppRunningManagerThirdTest::appServiceInner_ = nullptr;
sptr<MockApplication> AppRunningManagerThirdTest::mockApp1_ = nullptr;

void AppRunningManagerThirdTest::SetUpTestCase(void)
{
    appInfo_ = std::make_shared<ApplicationInfo>();
    appInfo_->bundleName = BUNDLE_NAME;
    appServiceInner_ = std::make_shared<MockAppMgrServiceInner>();
    mockApp1_ = sptr<MockApplication>::MakeSptr();
}

void AppRunningManagerThirdTest::TearDownTestCase(void)
{}

void AppRunningManagerThirdTest::SetUp()
{}

void AppRunningManagerThirdTest::TearDown()
{}

/**
 * @tc.name: AppRunningManager_GetAppRunningRecordByAbilityToken_0100
 * @tc.desc: Test GetAppRunningRecordByAbilityToken
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_GetAppRunningRecordByAbilityToken_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetAppRunningRecordByAbilityToken_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    appInfo_->bundleName = BUNDLE_NAME;
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    appRunningManager->appRunningRecordMap_.emplace(record->GetRecordId(), record);
    sptr<IRemoteObject> abilityToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    EXPECT_NE(abilityToken, nullptr);
    auto res = appRunningManager->GetAppRunningRecordByAbilityToken(abilityToken);
    EXPECT_EQ(res, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetAppRunningRecordByAbilityToken_0100 end");
}

/**
 * @tc.name: AppRunningManager_ProcessExitByBundleNameAndUid_0100
 * @tc.desc: Test ProcessExitByBundleNameAndUid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_ProcessExitByBundleNameAndUid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_ProcessExitByBundleNameAndUid_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::list<pid_t> pids;
    bool clearPageStack = false;
    appRunningManager->appRunningRecordMap_.emplace(AppRecordId::Create(), nullptr);
    bool res = appRunningManager->ProcessExitByBundleNameAndUid(BUNDLE_NAME, 1, pids, clearPageStack); // 1 means uid
    EXPECT_FALSE(res);
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    std::map<std::string, std::shared_ptr<ApplicationInfo>> appInfos_;
    auto applicationInfo = std::make_shared<OHOS::AppExecFwk::ApplicationInfo>();
    applicationInfo->bundleName = BUNDLE_NAME;
    applicationInfo->uid = 1;
    record->appInfos_.emplace("test", applicationInfo);
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->priorityObject_->SetPid(1); // 1 means pid
    appRunningManager->appRunningRecordMap_.emplace(AppRecordId::Create(), record);
    res = appRunningManager->ProcessExitByBundleNameAndUid(BUNDLE_NAME, 1, pids, clearPageStack); // 1 means uid
    EXPECT_TRUE(res);
    clearPageStack = true;
    std::shared_ptr<AppRunningRecord> record1 =
        appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record1->appInfos_.emplace("test", applicationInfo);
    record1->priorityObject_ = std::make_shared<PriorityObject>();
    appRunningManager->appRunningRecordMap_.emplace(AppRecordId::Create(), record1);
    res = appRunningManager->ProcessExitByBundleNameAndUid(BUNDLE_NAME, 1, pids, clearPageStack); // 1 means uid
    EXPECT_TRUE(res);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_ProcessExitByBundleNameAndUid_0100 end");
}

/**
 * @tc.name: AppRunningManager_UpdateConfigurationByBundleName_0100
 * @tc.desc: Test UpdateConfigurationByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_UpdateConfigurationByBundleName_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_UpdateConfigurationByBundleName_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    Configuration config;
    auto res = appRunningManager->UpdateConfigurationByBundleName(config, BUNDLE_NAME);
    EXPECT_EQ(res, ERR_OK);
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->SetState(ApplicationState::APP_STATE_CREATE);
    appRunningManager->appRunningRecordMap_.emplace(AppRecordId::Create(), record);
    res = appRunningManager->UpdateConfigurationByBundleName(config, BUNDLE_NAME);
    EXPECT_EQ(res, ERR_OK);
    appRunningManager->appRunningRecordMap_.clear();
    std::shared_ptr<AppRunningRecord> record1 =
        appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record1->mainBundleName_ = BUNDLE_NAME;
    record1->appInfo_ = std::make_shared<OHOS::AppExecFwk::ApplicationInfo>();
    record1->appInfo_->name = BUNDLE_NAME;
    appRunningManager->appRunningRecordMap_.emplace(AppRecordId::Create(), record1);
    res = appRunningManager->UpdateConfigurationByBundleName(config, BUNDLE_NAME);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_UpdateConfigurationByBundleName_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpFfrt_0100
 * @tc.desc: Test DumpFfrt
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpFfrt_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpFfrt_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::vector<int32_t> pids;
    std::string result = "";
    auto res = appRunningManager->DumpFfrt(pids, result);
    EXPECT_EQ(res, DumpErrorCode::ERR_INVALID_PID_ERROR);

    pids.push_back(1); // 1 means pid
    std::shared_ptr<AppRunningRecord> record1 =
        appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record1->priorityObject_ = std::make_shared<PriorityObject>();
    record1->priorityObject_->SetPid(1); // 1 means pid
    appRunningManager->appRunningRecordMap_.emplace(AppRecordId::Create(), record1);
    res = appRunningManager->DumpFfrt(pids, result);
    EXPECT_EQ(res, DumpErrorCode::ERR_INTERNAL_ERROR);

    pids.clear();
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->priorityObject_->SetPid(1); // 1 means pid
    appRunningManager->appRunningRecordMap_.emplace(AppRecordId::Create(), record);
    pids.push_back(1); // 1 means pid
    result = "test";
    res = appRunningManager->DumpFfrt(pids, result);
    EXPECT_EQ(res, DumpErrorCode::ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpFfrt_0100 end");
}

/**
 * @tc.name: AppRunningManager_HandleUserRequestClean_0100
 * @tc.desc: Test HandleUserRequestClean
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_HandleUserRequestClean_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_HandleUserRequestClean_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    sptr<IRemoteObject> abilityToken;
    pid_t pid;
    int32_t uid;
    auto res = appRunningManager->HandleUserRequestClean(abilityToken, pid, uid);
    EXPECT_FALSE(res); // null abilityToken

    abilityToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    res = appRunningManager->HandleUserRequestClean(abilityToken, pid, uid);
    EXPECT_FALSE(res); // null appRecord
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_HandleUserRequestClean_0100 end");
}

/**
 * @tc.name: AppRunningManager_UpdateConfigurationDelayed_0100
 * @tc.desc: Test UpdateConfigurationDelayed
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_UpdateConfigurationDelayed_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_UpdateConfigurationDelayed_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::shared_ptr<AppRunningRecord> appRecord =
        appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    auto res = appRunningManager->UpdateConfigurationDelayed(appRecord); // null appRecord
    EXPECT_EQ(res, ERR_OK);

    appRecord->appRecordId_ = AppRecordId::Create();
    appRunningManager->updateConfigurationDelayedMap_.emplace(appRecord->appRecordId_, true);
    appRecord->SetUid(BASE_USER_RANGE); // Make appRecord->GetUid() / BASE_USER_RANGE != conditions fulfilled.
    appRunningManager->multiUserConfigurationMgr_ = std::make_shared<MultiUserConfigurationMgr>();
    res = appRunningManager->UpdateConfigurationDelayed(appRecord);
    EXPECT_EQ(res, ERR_INVALID_VALUE);

    appRunningManager->updateConfigurationDelayedMap_.erase(appRecord->appRecordId_);
    appRecord->appRecordId_ = AppRecordId::Create();
    appRunningManager->updateConfigurationDelayedMap_.emplace(appRecord->appRecordId_, true);
    appRecord->SetUid(0); // 0 means uid, to be satisfied appRecord->GetUid() / BASE_USER_RANGE == 0
    res = appRunningManager->UpdateConfigurationDelayed(appRecord);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_UpdateConfigurationDelayed_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpIpcAllStart_0100
 * @tc.desc: Test DumpIpcAllStart
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcAllStart_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStart_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string result;
    std::shared_ptr<AppRunningRecord> record =
        appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->priorityObject_->SetPid(1); // 1 means pid
    appRunningManager->appRunningRecordMap_.emplace(AppRecordId::Create(), record);
    auto res = appRunningManager->DumpIpcAllStart(result);
    EXPECT_EQ(res, DumpErrorCode::ERR_INTERNAL_ERROR);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStart_0100 end");
}


/**
 * @tc.name: AppRunningManager_DumpIpcAllStop_0100
 * @tc.desc: Test the state of DumpIpcAllStop
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcAllStop_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStop_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, isAttachDebug);
    int32_t res = appRunningManager->DumpIpcAllStop(bundleName);
    EXPECT_NE(res, DEBUGINFOS_SIZE);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStop_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpIpcAllStop_0200
 * @tc.desc: Test the state of DumpIpcAllStop
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcAllStop_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStop_0200 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    EXPECT_NE(appRunningRecord->appLifeCycleDeal_, nullptr);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, isAttachDebug);
    int32_t res = appRunningManager->DumpIpcAllStop(bundleName);
    EXPECT_NE(res, DEBUGINFOS_SIZE);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStop_0200 end");
}

/**
 * @tc.name: AppRunningManager_DumpIpcAllStat_0100
 * @tc.desc: Test the state of DumpIpcAllStat
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcAllStat_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStat_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, isAttachDebug);
    int32_t res = appRunningManager->DumpIpcAllStat(bundleName);
    EXPECT_NE(res, DEBUGINFOS_SIZE);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStat_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpIpcAllStat_0200
 * @tc.desc: Test the state of DumpIpcAllStat
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcAllStat_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStat_0200 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningRecord->appLifeCycleDeal_ = std::make_shared<AppLifeCycleDeal>();
    EXPECT_NE(appRunningRecord->appLifeCycleDeal_, nullptr);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, isAttachDebug);
    int32_t res = appRunningManager->DumpIpcAllStat(bundleName);
    EXPECT_NE(res, DEBUGINFOS_SIZE);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcAllStat_0200 end");
}

/**
 * @tc.name: AppRunningManager_DumpIpcStart_0100
 * @tc.desc: Test the state of DumpIpcStart
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcStart_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcStart_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, isAttachDebug);
    int32_t pid = RECORD_ID;
    int32_t res = appRunningManager->DumpIpcStart(pid, bundleName);
    EXPECT_NE(res, DEBUGINFOS_SIZE);
    pid = 0;
    res = appRunningManager->DumpIpcStart(pid, bundleName);
    EXPECT_NE(res, DEBUGINFOS_SIZE);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcStart_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpIpcStop_0100
 * @tc.desc: Test the state of DumpIpcStop
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpIpcStop_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcStop_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, isAttachDebug);
    int32_t pid = RECORD_ID;
    int32_t res = appRunningManager->DumpIpcStop(pid, bundleName);
    EXPECT_NE(res, DEBUGINFOS_SIZE);
    pid = 0;
    res = appRunningManager->DumpIpcStop(pid, bundleName);
    EXPECT_NE(res, DEBUGINFOS_SIZE);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpIpcStop_0100 end");
}

/**
 * @tc.name: AppRunningManager_GetAppRunningUniqueIdByPid_0100
 * @tc.desc: Test the state of GetAppRunningUniqueIdByPid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_GetAppRunningUniqueIdByPid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetAppRunningUniqueIdByPid_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, isAttachDebug);
    int32_t pid = RECORD_ID;
    int32_t res = appRunningManager->GetAppRunningUniqueIdByPid(pid, bundleName);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    pid = 0;
    res = appRunningManager->GetAppRunningUniqueIdByPid(pid, bundleName);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetAppRunningUniqueIdByPid_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpJsHeapMemory_0100
 * @tc.desc: Test the state of DumpJsHeapMemory
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpJsHeapMemory_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpJsHeapMemoryd_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, isAttachDebug);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.pid = 1;
    info.tid = 1;
    info.needGc = false;
    info.needSnapshot = true;
    info.needLeakobj = false;
    int32_t res = appRunningManager->DumpJsHeapMemory(info);
    EXPECT_EQ(res, ERR_INVALID_VALUE);
    info.pid = 0;
    res = appRunningManager->DumpJsHeapMemory(info);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpJsHeapMemoryd_0100 end");
}

/**
 * @tc.name: AppRunningManager_AssignRunningProcessInfoByAppRecord_0100
 * @tc.desc: Test the state of AssignRunningProcessInfoByAppRecor
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_AssignRunningProcessInfoByAppRecord_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_AssignRunningProcessInfoByAppRecord_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    appInfo->multiAppMode.multiAppModeType = AppExecFwk::MultiAppModeType::APP_CLONE;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, isAttachDebug);
    AppExecFwk::RunningProcessInfo info;
    int32_t res = appRunningManager->AssignRunningProcessInfoByAppRecord(nullptr, info);
    EXPECT_EQ(res, ERR_INVALID_OPERATION);
    res = appRunningManager->AssignRunningProcessInfoByAppRecord(appRunningRecord, info);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_AssignRunningProcessInfoByAppRecord_0100 end");
}

/**
 * @tc.name: AppRunningManager_AssignRunningProcessInfoByAppRecord_0200
 * @tc.desc: Test the state of AssignRunningProcessInfoByAppRecor
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_AssignRunningProcessInfoByAppRecord_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_AssignRunningProcessInfoByAppRecord_0200 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = nullptr;
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, isAttachDebug);
    AppExecFwk::RunningProcessInfo info;
    int32_t res = appRunningManager->AssignRunningProcessInfoByAppRecord(appRunningRecord, info);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_AssignRunningProcessInfoByAppRecord_0200 end");
}

/**
 * @tc.name: AppRunningManager_AssignRunningProcessInfoByAppRecord_0300
 * @tc.desc: Test the state of AssignRunningProcessInfoByAppRecor
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_AssignRunningProcessInfoByAppRecord_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_AssignRunningProcessInfoByAppRecord_0300 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    std::string bundleName;
    bool isAttachDebug = true;
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    int32_t recordId = RECORD_ID;
    std::string processName;
    auto appRunningRecord = std::make_shared<AppRunningRecord>(appInfo, recordId, processName);
    appRunningManager->appRunningRecordMap_.insert(make_pair(RECORD_ID, appRunningRecord));
    appRunningManager->SetAttachAppDebug(bundleName, isAttachDebug);
    AppExecFwk::RunningProcessInfo info;
    int32_t res = appRunningManager->AssignRunningProcessInfoByAppRecord(appRunningRecord, info);
    EXPECT_EQ(res, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_AssignRunningProcessInfoByAppRecord_0300 end");
}
} // namespace AppExecFwk
} // namespace OHOS
