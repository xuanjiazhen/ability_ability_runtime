/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <unistd.h>

#define private public
#include "app_mgr_service_inner.h"
#include "business_manager.h"
#include "app_running_record.h"
#undef private
#include "application_info.h"
#include "fault_data.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "mock_task_handler_wrap.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t RECORD_ID = 1;
}  // namespace

class BusinessManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void BusinessManagerTest::SetUpTestCase(void) {}
void BusinessManagerTest::TearDownTestCase(void) {}
void BusinessManagerTest::SetUp() {}
void BusinessManagerTest::TearDown() {}

/**
 * @tc.name: GetInstance_001
 * @tc.desc: Verify that GetInstance returns the same singleton reference across calls.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, GetInstance_001, TestSize.Level1)
{
    auto &inst1 = BusinessManager::GetInstance();
    auto &inst2 = BusinessManager::GetInstance();
    EXPECT_EQ(&inst1, &inst2);
}

/**
 * @tc.name: FaultTypeToString_001
 * @tc.desc: Verify FaultTypeToString maps each known FaultDataType to its string name.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, FaultTypeToString_001, TestSize.Level1)
{
    EXPECT_EQ(BusinessManager::FaultTypeToString(FaultDataType::CPP_CRASH), "CPP_CRASH");
    EXPECT_EQ(BusinessManager::FaultTypeToString(FaultDataType::JS_ERROR), "JS_ERROR");
    EXPECT_EQ(BusinessManager::FaultTypeToString(FaultDataType::APP_FREEZE), "APP_FREEZE");
    EXPECT_EQ(BusinessManager::FaultTypeToString(FaultDataType::PERFORMANCE_CONTROL), "PERFORMANCE_CONTROL");
    EXPECT_EQ(BusinessManager::FaultTypeToString(FaultDataType::RESOURCE_CONTROL), "RESOURCE_CONTROL");
    EXPECT_EQ(BusinessManager::FaultTypeToString(static_cast<FaultDataType>(-1)), "UNKNOWN");
}

/**
 * @tc.name: IsAsanEnabled_001
 * @tc.desc: IsAsanEnabled returns false for a nullptr record / nullptr applicationInfo.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, IsAsanEnabled_001, TestSize.Level1)
{
    EXPECT_FALSE(BusinessManager::IsAsanEnabled(nullptr));
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "asan_test_process");
    EXPECT_FALSE(BusinessManager::IsAsanEnabled(appRecord));
    appInfo->asanEnabled = true;
    EXPECT_TRUE(BusinessManager::IsAsanEnabled(appRecord));
}

/**
 * @tc.name: UpdateForeground_001
 * @tc.desc: UpdateForeground returns true only when the app is foreground/focus for block events.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, UpdateForeground_001, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "UpdateForeground_001");
    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    EXPECT_TRUE(BusinessManager::UpdateForeground(appRecord, AppFreezeType::THREAD_BLOCK_3S));
    appRecord->SetState(ApplicationState::APP_STATE_FOCUS);
    EXPECT_TRUE(BusinessManager::UpdateForeground(appRecord, AppFreezeType::THREAD_BLOCK_3S));
    appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    EXPECT_FALSE(BusinessManager::UpdateForeground(appRecord, AppFreezeType::THREAD_BLOCK_3S));
}

/**
 * @tc.name: UpdateForeground_002
 * @tc.desc: UpdateForeground returns true when transitioning to foreground for non-block events.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, UpdateForeground_002, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "UpdateForeground_002");
    appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    appRecord->SetApplicationPendingState(ApplicationPendingState::FOREGROUNDING);
    EXPECT_TRUE(BusinessManager::UpdateForeground(appRecord, AppFreezeType::LIFECYCLE_TIMEOUT));
    appRecord->SetApplicationPendingState(ApplicationPendingState::BACKGROUNDING);
    EXPECT_FALSE(BusinessManager::UpdateForeground(appRecord, AppFreezeType::LIFECYCLE_TIMEOUT));
}

/**
 * @tc.name: CheckAppFault_001
 * @tc.desc: CheckAppFault returns true when a non-empty timeoutMarkers task is still pending.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, CheckAppFault_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner, nullptr);
    inner->dfxTaskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("dfx_freeze_task_queue_test");
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "CheckAppFault_001");
    FaultData faultData;
    EXPECT_FALSE(BusinessManager::CheckAppFault(inner, appRecord, faultData));
    faultData.timeoutMarkers = "timeout";
    EXPECT_TRUE(BusinessManager::CheckAppFault(inner, appRecord, faultData));
}

/**
 * @tc.name: AppRecoveryNotifyApp_001
 * @tc.desc: AppRecoveryNotifyApp runs the non-APP_FREEZE path without crashing.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, AppRecoveryNotifyApp_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner, nullptr);
    int32_t pid = 0;
    std::string bundleName = "com.is.hiserice";
    BusinessManager::AppRecoveryNotifyApp(inner, pid, bundleName, FaultDataType::RESOURCE_CONTROL, "appRecovery",
        -1);
    BusinessManager::AppRecoveryNotifyApp(inner, pid, bundleName, FaultDataType::APP_FREEZE, "recovery", -1);
}

/**
 * @tc.name: RecordAppRecoveryNotifyAppReason_001
 * @tc.desc: RecordAppRecoveryNotifyAppReason logs and skips when no app record is found.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, RecordAppRecoveryNotifyAppReason_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner, nullptr);
    BusinessManager::RecordAppRecoveryNotifyAppReason(inner, getpid());
    BusinessManager::RecordAppRecoveryNotifyAppReason(inner, 1);
}

/**
 * @tc.name: KillFaultApp_001
 * @tc.desc: KillFaultApp with isNeedExit submits the kill task and returns ERR_OK.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, KillFaultApp_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner, nullptr);
    inner->SetTaskHandler(MockTaskHandlerWrap::CreateQueueHandler("app_mgr_task_queue"));
    FaultData faultData;
    faultData.errorObject.name = "1234";
    faultData.faultType = FaultDataType::APP_FREEZE;
    EXPECT_EQ(BusinessManager::KillFaultApp(inner, 0, "bundleName", faultData, true), ERR_OK);
    sleep(3);
}

/**
 * @tc.name: KillFaultApp_002
 * @tc.desc: KillFaultApp with forceExit && !waitSaveState triggers the kill branch.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, KillFaultApp_002, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner, nullptr);
    inner->SetTaskHandler(MockTaskHandlerWrap::CreateQueueHandler("app_mgr_task_queue"));
    FaultData faultData;
    faultData.isInForeground = false;
    faultData.errorObject.name = AppFreezeType::THREAD_BLOCK_6S;
    faultData.forceExit = true;
    faultData.waitSaveState = false;
    EXPECT_EQ(BusinessManager::KillFaultApp(inner, 0, "KillFaultApp_002", faultData, false), ERR_OK);
}

/**
 * @tc.name: GetExceptionTimerId_001
 * @tc.desc: GetExceptionTimerId returns -1 when HICOLLIE is disabled; with a nullptr
 *           appRecord the surrounding SubmitDfxFaultTask path is not exercised here.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, GetExceptionTimerId_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner, nullptr);
    FaultData faultData;
    int result = BusinessManager::GetExceptionTimerId(inner, faultData, "", nullptr, 0, 0);
    EXPECT_EQ(result, -1);
}

/**
 * @tc.name: SubmitDfxFaultTask_001
 * @tc.desc: SubmitDfxFaultTask returns ERR_INVALID_VALUE for a nullptr appRecord.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, SubmitDfxFaultTask_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner, nullptr);
    FaultData faultData;
    EXPECT_EQ(BusinessManager::SubmitDfxFaultTask(inner, faultData, "SubmitDfxFaultTask_001", nullptr, getpid()),
        ERR_INVALID_VALUE);
}

/**
 * @tc.name: TimeoutNotifyApp_001
 * @tc.desc: TimeoutNotifyApp exercises the non-dialog branch without crashing.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, TimeoutNotifyApp_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner, nullptr);
    inner->SetTaskHandler(MockTaskHandlerWrap::CreateQueueHandler("app_mgr_task_queue"));
    FaultData faultData;
    faultData.errorObject.name = "1234";
    faultData.faultType = FaultDataType::APP_FREEZE;
    BusinessManager::TimeoutNotifyApp(inner, 0, 0, "test_processName", "test_processName", faultData, -1);
}

/**
 * @tc.name: NotifyAppFault_Delegate_001
 * @tc.desc: NotifyAppFault with no matching app record returns ERR_INVALID_VALUE.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, NotifyAppFault_Delegate_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner, nullptr);
    FaultData faultData;
    faultData.errorObject.name = "1234";
    faultData.timeoutMarkers = "456";
    EXPECT_EQ(BusinessManager::NotifyAppFault(inner, faultData), ERR_INVALID_VALUE);
}

/**
 * @tc.name: NotifyAppFaultBySA_Entry_001
 * @tc.desc: NotifyAppFaultBySA keeps the security gate at the entry; with no
 *           remoteClientManager it returns ERR_NO_INIT before reaching BusinessManager.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, NotifyAppFaultBySA_Entry_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner, nullptr);
    inner->remoteClientManager_ = nullptr;
    AppFaultDataBySA faultData;
    faultData.pid = 8142;
    faultData.errorObject.name = "appRecovery";
    EXPECT_EQ(inner->NotifyAppFaultBySA(faultData), ERR_NO_INIT);
}

/**
 * @tc.name: SetAppFreezeFilter_Delegate_001
 * @tc.desc: SetAppFreezeFilter with no matching caller record returns false.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, SetAppFreezeFilter_Delegate_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner, nullptr);
    EXPECT_FALSE(BusinessManager::SetAppFreezeFilter(inner, 0));
}

/**
 * @tc.name: SendProcessKillEvent_001
 * @tc.desc: SendProcessKillEvent with nullptr appRecord does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, SendProcessKillEvent_001, TestSize.Level1)
{
    BusinessManager::SendProcessKillEvent(nullptr);
}

/**
 * @tc.name: SendProcessKillEvent_002
 * @tc.desc: SendProcessKillEvent with default-constructed ApplicationInfo does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, SendProcessKillEvent_002, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "SendProcessKillEvent_002");
    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appRecord->SetKillId(0);
    appRecord->SetKillMsg("test_kill_msg");
    appRecord->SetInnerMsg("test_inner_msg");
    BusinessManager::SendProcessKillEvent(appRecord);
}

/**
 * @tc.name: SendProcessKillEvent_003
 * @tc.desc: SendProcessKillEvent with a fully populated appRecord sends the event.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, SendProcessKillEvent_003, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = "com.test.sendkill";
    appInfo->versionCode = 1;
    appInfo->versionName = "1.0.0";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "SendProcessKillEvent_003");
    appRecord->SetState(ApplicationState::APP_STATE_FOCUS);
    appRecord->SetKillId(100);
    appRecord->SetKillMsg("KillReason:focus");
    appRecord->SetInnerMsg("innerMsg:focus");
    appRecord->SetRssValue(12345);
    appRecord->SetPssValue(67890);
    BusinessManager::SendProcessKillEvent(appRecord);
}

/**
 * @tc.name: SendProcessKillEvent_004
 * @tc.desc: SendProcessKillEvent with BACKGROUND state exercises the non-foreground branch.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, SendProcessKillEvent_004, TestSize.Level1)
{
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = "com.test.background";
    appInfo->versionCode = 2;
    appInfo->versionName = "2.0.0";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "SendProcessKillEvent_004");
    appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    appRecord->SetKillId(-1);
    appRecord->SetKillMsg("KillReason:bg");
    BusinessManager::SendProcessKillEvent(appRecord);
}

/**
 * @tc.name: SendProcessKillEvent_Delegate_001
 * @tc.desc: AppMgrServiceInner::SendProcessKillEvent delegates to BusinessManager and
 *           does not crash for nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, SendProcessKillEvent_Delegate_001, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner, nullptr);
    inner->SendProcessKillEvent(nullptr);
}

/**
 * @tc.name: SendProcessKillEvent_Delegate_002
 * @tc.desc: AppMgrServiceInner::SendProcessKillEvent delegates to BusinessManager for a
 *           valid appRecord.
 * @tc.type: FUNC
 */
HWTEST_F(BusinessManagerTest, SendProcessKillEvent_Delegate_002, TestSize.Level1)
{
    auto inner = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner, nullptr);
    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = "com.test.delegate";
    appInfo->versionCode = 3;
    appInfo->versionName = "3.0.0";
    auto appRecord = std::make_shared<AppRunningRecord>(appInfo, RECORD_ID, "SendProcessKillEvent_Delegate_002");
    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);
    appRecord->SetKillId(200);
    appRecord->SetKillMsg("delegate_kill_msg");
    inner->SendProcessKillEvent(appRecord);
}
}  // namespace AppExecFwk
}  // namespace OHOS
