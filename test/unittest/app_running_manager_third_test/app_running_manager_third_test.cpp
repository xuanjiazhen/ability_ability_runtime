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

#include "app_running_manager.h"
#include "app_running_record.h"
#ifdef SUPPORT_CHILD_PROCESS
#include "child_process_record.h"
#endif // SUPPORT_CHILD_PROCESS

#include "app_record_id.h"
#include "exit_resident_process_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_app_mgr_service_inner.h"
#include "mock_application.h"
#include "quick_fix_callback_proxy.h"
#include "quick_fix_callback_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t USR_ID_100 = 100;
constexpr int32_t USR_ID_101 = 101;
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
 * @tc.name: AppRunningManager_ProcessExitByTokenIdAndInstance_0100
 * @tc.desc: Test ProcessExitByTokenIdAndInstance
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_ProcessExitByTokenIdAndInstance_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_ProcessExitByTokenIdAndInstance_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    uint32_t accessTokenId = 0;
    std::string instanceKey;
    std::list<pid_t> pids;
    bool clearPageStack = false;
    auto recordId = AppRecordId::Create();
    appRunningManager->appRunningRecordMap_.emplace(recordId, nullptr);

    auto recordIdOne = AppRecordId::Create();
    std::shared_ptr<AppRunningRecord> record =
    appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = nullptr;
    appRunningManager->appRunningRecordMap_.emplace(recordIdOne, record);

    auto recordIdTwo = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->accessTokenId = 1;
    appRunningManager->appRunningRecordMap_.emplace(recordIdTwo, record);

    auto recordIdThree = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->accessTokenId = accessTokenId;
    record->appInfo_->multiAppMode.multiAppModeType = MultiAppModeType::UNSPECIFIED;
    appRunningManager->appRunningRecordMap_.emplace(recordIdThree, record);

    auto recordIdFour = AppRecordId::Create();
    instanceKey = "123";
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->accessTokenId = accessTokenId;
    record->appInfo_->multiAppMode.multiAppModeType = MultiAppModeType::MULTI_INSTANCE;
    record->instanceKey_ = "456";
    appRunningManager->appRunningRecordMap_.emplace(recordIdFour, record);

    auto ret = appRunningManager->ProcessExitByTokenIdAndInstance(accessTokenId, instanceKey, pids, clearPageStack);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_ProcessExitByTokenIdAndInstance_0100 end");
}

/**
 * @tc.name: AppRunningManager_ProcessExitByTokenIdAndInstance_0200
 * @tc.desc: Test ProcessExitByTokenIdAndInstance
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_ProcessExitByTokenIdAndInstance_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_ProcessExitByTokenIdAndInstance_0200 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    uint32_t accessTokenId = 0;
    std::string instanceKey;
    std::list<pid_t> pids;
    bool clearPageStack = false;
    auto recordIdFive = AppRecordId::Create();
    auto record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->accessTokenId = accessTokenId;
    record->appInfo_->multiAppMode.multiAppModeType = MultiAppModeType::MULTI_INSTANCE;
    record->priorityObject_ = nullptr;
    record->instanceKey_ = instanceKey;
    appRunningManager->appRunningRecordMap_.emplace(recordIdFive, record);

    auto recordIdSix = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->accessTokenId = accessTokenId;
    record->appInfo_->multiAppMode.multiAppModeType = MultiAppModeType::MULTI_INSTANCE;
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->instanceKey_ = instanceKey;
    record->priorityObject_->pid_ = 0;
    appRunningManager->appRunningRecordMap_.emplace(recordIdSix, record);

    auto recordIdSeven = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->accessTokenId = accessTokenId;
    record->appInfo_->multiAppMode.multiAppModeType = MultiAppModeType::MULTI_INSTANCE;
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->instanceKey_ = instanceKey;
    record->priorityObject_->pid_ = 1;
    appRunningManager->appRunningRecordMap_.emplace(recordIdSeven, record);
    auto ret = appRunningManager->ProcessExitByTokenIdAndInstance(accessTokenId, instanceKey, pids, clearPageStack);
    EXPECT_EQ(ret, true);
    clearPageStack = true;
    ret = appRunningManager->ProcessExitByTokenIdAndInstance(accessTokenId, instanceKey, pids, clearPageStack);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_ProcessExitByTokenIdAndInstance_0200 end");
}

/**
 * @tc.name: AppRunningManager_GetRunningProcessInfoByPid_0100
 * @tc.desc: Test GetRunningProcessInfoByPid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_GetRunningProcessInfoByPid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetRunningProcessInfoByPid_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    pid_t pid = 0;
    OHOS::AppExecFwk::RunningProcessInfo info;
    auto ret = appRunningManager->GetRunningProcessInfoByPid(pid, info);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    pid = -1;
    ret = appRunningManager->GetRunningProcessInfoByPid(pid, info);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    pid = 1;
    ret = appRunningManager->GetRunningProcessInfoByPid(pid, info);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetRunningProcessInfoByPid_0100 end");
}

/**
 * @tc.name: AppRunningManager_GetRunningProcessInfoByChildProcessPid_0100
 * @tc.desc: Test GetRunningProcessInfoByChildProcessPid
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_GetRunningProcessInfoByChildProcessPid_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetRunningProcessInfoByChildProcessPid_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    pid_t childPid = -1;
    OHOS::AppExecFwk::RunningProcessInfo info;
    auto ret = appRunningManager->GetRunningProcessInfoByChildProcessPid(childPid, info);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    childPid = 0;
    ret = appRunningManager->GetRunningProcessInfoByChildProcessPid(childPid, info);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    childPid = 1;
    ret = appRunningManager->GetRunningProcessInfoByChildProcessPid(childPid, info);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    auto recordId = AppRecordId::Create();
    auto record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->priorityObject_->pid_ = 1;
    appRunningManager->appRunningRecordMap_.emplace(recordId, record);
    ret = appRunningManager->GetRunningProcessInfoByChildProcessPid(childPid, info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_GetRunningProcessInfoByChildProcessPid_0100 end");
}

/**
 * @tc.name: AppRunningManager_DumpJsHeapMemory_0100
 * @tc.desc: Test DumpJsHeapMemory
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_DumpJsHeapMemory_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpJsHeapMemory_0100 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    OHOS::AppExecFwk::JsHeapDumpInfo info;
    info.pid = 0;
    auto ret = appRunningManager->DumpJsHeapMemory(info);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    info.pid = 1;
    auto recordId = AppRecordId::Create();
    auto record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->priorityObject_ = std::make_shared<PriorityObject>();
    record->priorityObject_->pid_ = 1;
    appRunningManager->appRunningRecordMap_.emplace(recordId, record);
    ret = appRunningManager->DumpJsHeapMemory(info);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_DumpJsHeapMemory_0100 end");
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
    const std::string name = "";
    int32_t appIndex = 0;
    auto recordId = AppRecordId::Create();
    auto record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->SetState(ApplicationState::APP_STATE_CREATE);
    appRunningManager->appRunningRecordMap_.emplace(recordId, record);

    auto recordIdOne = AppRecordId::Create();
    appRunningManager->appRunningRecordMap_.emplace(recordIdOne, nullptr);

    auto recordIdTwo = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->SetState(ApplicationState::APP_STATE_READY);
    appRunningManager->appRunningRecordMap_.emplace(recordIdTwo, record);

    auto ret = appRunningManager->UpdateConfigurationByBundleName(config, name, appIndex);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_UpdateConfigurationByBundleName_0100 end");
}

/**
 * @tc.name: AppRunningManager_UpdateConfigurationByBundleName_0200
 * @tc.desc: Test UpdateConfigurationByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(AppRunningManagerThirdTest, AppRunningManager_UpdateConfigurationByBundleName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_UpdateConfigurationByBundleName_0200 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    EXPECT_NE(appRunningManager, nullptr);
    Configuration config;
    const std::string name = "123";
    int32_t appIndex = 0;
    auto recordIdOne = AppRecordId::Create();
    appRunningManager->appRunningRecordMap_.emplace(recordIdOne, nullptr);
    auto recordIdTwo = AppRecordId::Create();
    auto record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->SetState(ApplicationState::APP_STATE_READY);
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->name = "123";
    record->mainBundleName_ = "123";
    record->appIndex_ = 0;
    appRunningManager->appRunningRecordMap_.emplace(recordIdTwo, record);
    auto recordIdThree = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->SetState(ApplicationState::APP_STATE_READY);
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->name = "1234";
    record->mainBundleName_ = "123";
    record->appIndex_ = 0;
    appRunningManager->appRunningRecordMap_.emplace(recordIdThree, record);
    auto recordIdFour = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->SetState(ApplicationState::APP_STATE_READY);
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->name = "123";
    record->mainBundleName_ = "123";
    record->appIndex_ = 1;
    appRunningManager->appRunningRecordMap_.emplace(recordIdFour, record);
    auto recordIdFive = AppRecordId::Create();
    record = appRunningManager->CreateAppRunningRecord(appInfo_, PROCESS_NAME, bundleInfo, "");
    record->SetState(ApplicationState::APP_STATE_READY);
    record->appInfo_ = std::make_shared<ApplicationInfo>();
    record->appInfo_->name = "123";
    record->mainBundleName_ = "1234";
    record->appIndex_ = 1;
    appRunningManager->appRunningRecordMap_.emplace(recordIdFive, record);
    auto ret = appRunningManager->UpdateConfigurationByBundleName(config, name, appIndex);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "AppRunningManager_UpdateConfigurationByBundleName_0200 end");
}

} // namespace AppExecFwk
} // namespace OHOS