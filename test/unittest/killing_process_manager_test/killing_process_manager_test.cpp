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
#include <unistd.h>

#define private public
#include "killing_process_manager.h"
#undef private
#include "app_running_manager.h"
#include "app_running_record.h"
#include "hilog_tag_wrapper.h"
#include "mock_native_token.h"
#include "permission_constants.h"
#include "permission_verification.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t CLEAR_CALLER_KEY_DELAY_TIME = 6;
} // namespace

class KillingProcessManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    void InitKillingProcessManager();
};

void KillingProcessManagerTest::SetUpTestCase()
{
    MockNativeToken::SetNativeToken();
}

void KillingProcessManagerTest::TearDownTestCase()
{}

void KillingProcessManagerTest::SetUp()
{}

void KillingProcessManagerTest::TearDown()
{}

void KillingProcessManagerTest::InitKillingProcessManager()
{
    auto appRunningManager = std::make_shared<AppRunningManager>();
    auto taskHandler = AAFwk::TaskHandlerWrap::GetFfrtHandler();
    auto remoteClientManager = std::make_shared<RemoteClientManager>();
    KillingProcessManager::GetInstance().Init(appRunningManager, taskHandler, remoteClientManager);
}

/**
 * @tc.name: IsCallerKilling_001
 * @tc.desc: test IsCallerKilling function with non-existent key.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, IsCallerKilling_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest IsCallerKilling_001 start");
    std::string callerKey = "testcallerKey";
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    bool ret = killingProcessManager.IsCallerKilling(callerKey);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest IsCallerKilling_001 end");
}

/**
 * @tc.name: IsCallerKilling_002
 * @tc.desc: test IsCallerKilling function with existing key.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, IsCallerKilling_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest IsCallerKilling_002 start");
    InitKillingProcessManager();
    std::string callerKey = "testcallerKey";
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    killingProcessManager.AddKillingCallerKey(callerKey);
    bool ret = killingProcessManager.IsCallerKilling(callerKey);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest IsCallerKilling_002 end");
}

/**
 * @tc.name: IsCallerKilling_003
 * @tc.desc: test IsCallerKilling function with empty key.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, IsCallerKilling_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest IsCallerKilling_003 start");
    std::string callerKey = "";
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    bool ret = killingProcessManager.IsCallerKilling(callerKey);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest IsCallerKilling_003 end");
}

/**
 * @tc.name: AddKillingCallerKey_001
 * @tc.desc: test AddKillingCallerKey function.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, AddKillingCallerKey_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest AddKillingCallerKey_001 start");
    std::string nullKey = "";
    std::string callerKey = "testcallerKey";
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    killingProcessManager.AddKillingCallerKey(nullKey);
    EXPECT_EQ(killingProcessManager.killingCallerKeySet_.count(nullKey), 0);

    killingProcessManager.RemoveKillingCallerKey(nullKey);
    EXPECT_EQ(killingProcessManager.killingCallerKeySet_.count(nullKey), 0);

    killingProcessManager.AddKillingCallerKey(callerKey);
    EXPECT_EQ(killingProcessManager.killingCallerKeySet_.count(callerKey), 1);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest AddKillingCallerKey_001 end");
}

/**
 * @tc.name: AddKillingCallerKey_002
 * @tc.desc: test AddKillingCallerKey with duplicate key.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, AddKillingCallerKey_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest AddKillingCallerKey_002 start");
    InitKillingProcessManager();
    std::string callerKey = "testcallerKey";
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    killingProcessManager.AddKillingCallerKey(callerKey);
    auto size1 = killingProcessManager.killingCallerKeySet_.size();
    killingProcessManager.AddKillingCallerKey(callerKey);
    auto size2 = killingProcessManager.killingCallerKeySet_.size();
    EXPECT_EQ(size1, size2);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest AddKillingCallerKey_002 end");
}

/**
 * @tc.name: RemoveKillingCallerKey_001
 * @tc.desc: test RemoveKillingCallerKey function.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, RemoveKillingCallerKey_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest RemoveKillingCallerKey_001 start");
    std::string callerKey = "testcallerKey";
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    killingProcessManager.killingCallerKeySet_.insert(callerKey);
    EXPECT_EQ(killingProcessManager.killingCallerKeySet_.count(callerKey), 1);

    killingProcessManager.RemoveKillingCallerKey(callerKey);
    EXPECT_EQ(killingProcessManager.killingCallerKeySet_.count(callerKey), 0);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest RemoveKillingCallerKey_001 end");
}

/**
 * @tc.name: RemoveKillingCallerKey_002
 * @tc.desc: test RemoveKillingCallerKey with empty key.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, RemoveKillingCallerKey_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest RemoveKillingCallerKey_002 start");
    std::string emptyKey = "";
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    auto sizeBefore = killingProcessManager.killingCallerKeySet_.size();
    killingProcessManager.RemoveKillingCallerKey(emptyKey);
    auto sizeAfter = killingProcessManager.killingCallerKeySet_.size();
    EXPECT_EQ(sizeBefore, sizeAfter);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest RemoveKillingCallerKey_002 end");
}

/**
 * @tc.name: Init_001
 * @tc.desc: test Init function.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, Init_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest Init_001 start");
    auto appRunningManager = std::make_shared<AppRunningManager>();
    auto taskHandler = AAFwk::TaskHandlerWrap::GetFfrtHandler();
    auto remoteClientManager = std::make_shared<RemoteClientManager>();
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    killingProcessManager.Init(appRunningManager, taskHandler, remoteClientManager);
    EXPECT_EQ(killingProcessManager.appRunningManager_, appRunningManager);
    EXPECT_EQ(killingProcessManager.taskHandler_, taskHandler);
    EXPECT_EQ(killingProcessManager.remoteClientManager_, remoteClientManager);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest Init_001 end");
}

/**
 * @tc.name: Init_002
 * @tc.desc: test Init function with nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, Init_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest Init_002 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    killingProcessManager.Init(nullptr, nullptr, nullptr);
    EXPECT_EQ(killingProcessManager.appRunningManager_, nullptr);
    EXPECT_EQ(killingProcessManager.taskHandler_, nullptr);
    EXPECT_EQ(killingProcessManager.remoteClientManager_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest Init_002 end");
}

/**
 * @tc.name: KillApplicationByBundleName_001
 * @tc.desc: test KillApplicationByBundleName without appRunningManager.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, KillApplicationByBundleName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillApplicationByBundleName_001 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    killingProcessManager.appRunningManager_ = nullptr;
    int32_t ret = killingProcessManager.KillApplicationByBundleName("test.bundle", 0, false, "test");
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillApplicationByBundleName_001 end");
}

/**
 * @tc.name: SystemTimeMillisecond_001
 * @tc.desc: test SystemTimeMillisecond function.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, SystemTimeMillisecond_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest SystemTimeMillisecond_001 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    int64_t time1 = killingProcessManager.SystemTimeMillisecond();
    EXPECT_GT(time1, 0);

    usleep(1000);  // Sleep 1ms
    int64_t time2 = killingProcessManager.SystemTimeMillisecond();
    EXPECT_GT(time2, time1);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest SystemTimeMillisecond_001 end");
}

/**
 * @tc.name: VerifyAPL_001
 * @tc.desc: test VerifyAPL without appRunningManager.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, VerifyAPL_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest VerifyAPL_001 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    killingProcessManager.appRunningManager_ = nullptr;
    bool ret = killingProcessManager.VerifyAPL();
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest VerifyAPL_001 end");
}

/**
 * @tc.name: CheckIsThreadInFoundation_001
 * @tc.desc: test CheckIsThreadInFoundation with invalid pid.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, CheckIsThreadInFoundation_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest CheckIsThreadInFoundation_001 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    bool ret = killingProcessManager.CheckIsThreadInFoundation(0);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest CheckIsThreadInFoundation_001 end");
}

/**
 * @tc.name: CheckIsThreadInFoundation_002
 * @tc.desc: test CheckIsThreadInFoundation with current pid.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, CheckIsThreadInFoundation_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest CheckIsThreadInFoundation_002 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    pid_t currentPid = getpid();
    bool ret = killingProcessManager.CheckIsThreadInFoundation(currentPid);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest CheckIsThreadInFoundation_002 end");
}

/**
 * @tc.name: AddToKillProcessMap_001
 * @tc.desc: test AddToKillProcessMap function.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, AddToKillProcessMap_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest AddToKillProcessMap_001 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    auto sizeBefore = killingProcessManager.killedProcessMap_.size();
    std::string processName = "testProcess";
    killingProcessManager.AddToKillProcessMap(processName);
    auto sizeAfter = killingProcessManager.killedProcessMap_.size();
    EXPECT_GT(sizeAfter, sizeBefore);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest AddToKillProcessMap_001 end");
}

/**
 * @tc.name: AddToKillProcessMap_002
 * @tc.desc: test AddToKillProcessMap with multiple process names.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, AddToKillProcessMap_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest AddToKillProcessMap_002 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    std::string processName1 = "testProcess1";
    std::string processName2 = "testProcess2";
    killingProcessManager.AddToKillProcessMap(processName1);
    killingProcessManager.AddToKillProcessMap(processName2);
    EXPECT_EQ(killingProcessManager.killedProcessMap_.size(), 2);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest AddToKillProcessMap_002 end");
}

/**
 * @tc.name: KillProcessByPidForExit_001
 * @tc.desc: test KillProcessByPidForExit with non-existent pid.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, KillProcessByPidForExit_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillProcessByPidForExit_001 start");
    InitKillingProcessManager();
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    int32_t pid = 99999;
    std::string reason = "test";
    int32_t ret = killingProcessManager.KillProcessByPidForExit(pid, reason);
    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillProcessByPidForExit_001 end");
}

/**
 * @tc.name: KillProcessByPidForExit_002
 * @tc.desc: test KillProcessByPidForExit without appRunningManager.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, KillProcessByPidForExit_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillProcessByPidForExit_002 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    killingProcessManager.appRunningManager_ = nullptr;
    int32_t pid = 99999;
    std::string reason = "test";
    int32_t ret = killingProcessManager.KillProcessByPidForExit(pid, reason);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillProcessByPidForExit_002 end");
}

/**
 * @tc.name: SendProcessKillEvent_001
 * @tc.desc: test SendProcessKillEvent with null appRecord.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, SendProcessKillEvent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest SendProcessKillEvent_001 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    killingProcessManager.SendProcessKillEvent(nullptr, "test reason");
    // Should not crash, test passes if we reach here
    EXPECT_TRUE(true);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest SendProcessKillEvent_001 end");
}

/**
 * @tc.name: SendProcessKillEvent_002
 * @tc.desc: test SendProcessKillEvent with valid appRecord.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, SendProcessKillEvent_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest SendProcessKillEvent_002 start");
    InitKillingProcessManager();
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = "testApp";
    appInfo->bundleName = "com.test.app";
    appInfo->versionCode = 1;
    appInfo->versionName = "1.0";

    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(appInfo, 1, "testProcess");
    appRecord->SetPid(1001);
    appRecord->SetState(ApplicationState::APP_STATE_FOREGROUND);

    killingProcessManager.SendProcessKillEvent(appRecord, "test reason");
    // Should not crash, test passes if we reach here
    EXPECT_TRUE(true);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest SendProcessKillEvent_002 end");
}

/**
 * @tc.name: SendProcessKillEvent_003
 * @tc.desc: test SendProcessKillEvent with appRecord in background.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, SendProcessKillEvent_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest SendProcessKillEvent_003 start");
    InitKillingProcessManager();
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();

    std::shared_ptr<ApplicationInfo> appInfo = std::make_shared<ApplicationInfo>();
    appInfo->name = "testApp";
    appInfo->bundleName = "com.test.app";

    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(appInfo, 1, "testProcess");
    appRecord->SetPid(1002);
    appRecord->SetState(ApplicationState::APP_STATE_BACKGROUND);
    appRecord->SetKillReason("test reason");

    killingProcessManager.SendProcessKillEvent(appRecord, "default reason");
    // Should not crash, test passes if we reach here
    EXPECT_TRUE(true);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest SendProcessKillEvent_003 end");
}

/**
 * @tc.name: KillSubProcessBypidInner_001
 * @tc.desc: test KillSubProcessBypidInner with invalid pid.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, KillSubProcessBypidInner_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillSubProcessBypidInner_001 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    pid_t pid = 0;
    std::string reason = "test";
    AAFwk::EventInfo eventInfo;
    int32_t ret = killingProcessManager.KillSubProcessBypidInner(pid, reason, eventInfo);
    EXPECT_EQ(ret, AAFwk::ERR_KILL_PROCESS_NOT_EXIST);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillSubProcessBypidInner_001 end");
}

/**
 * @tc.name: KillSubProcessBypidInner_002
 * @tc.desc: test KillSubProcessBypidInner with large pid.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, KillSubProcessBypidInner_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillSubProcessBypidInner_002 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    pid_t pid = 65536;
    std::string reason = "test";
    AAFwk::EventInfo eventInfo;
    int32_t ret = killingProcessManager.KillSubProcessBypidInner(pid, reason, eventInfo);
    EXPECT_NE(ret, 0);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillSubProcessBypidInner_002 end");
}

/**
 * @tc.name: KillSubProcessBypidInner_003
 * @tc.desc: test KillSubProcessBypidInner with forked process.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, KillSubProcessBypidInner_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillSubProcessBypidInner_003 start");
    InitKillingProcessManager();
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    pid_t pid = fork();
    if (pid == -1) {
        TAG_LOGI(AAFwkTag::TEST, "fork process failed");
        EXPECT_TRUE(true);  // Skip test if fork fails
    } else if (pid == 0) {
        TAG_LOGI(AAFwkTag::TEST, "fork process success");
        sleep(3);
        exit(0);
    } else {
        std::string reason = "test";
        AAFwk::EventInfo eventInfo;
        int32_t ret = killingProcessManager.KillSubProcessBypidInner(pid, reason, eventInfo);
        EXPECT_EQ(ret, 0);
    }
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillSubProcessBypidInner_003 end");
}

/**
 * @tc.name: KillSubProcessBypid_001
 * @tc.desc: test KillSubProcessBypid without appRunningManager.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, KillSubProcessBypid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillSubProcessBypid_001 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    killingProcessManager.appRunningManager_ = nullptr;
    pid_t pid = 111;
    std::string reason = "test";
    int32_t ret = killingProcessManager.KillSubProcessBypid(pid, reason);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillSubProcessBypid_001 end");
}

/**
 * @tc.name: KillProcessByPid_001
 * @tc.desc: test KillProcessByPid with non-existent process.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, KillProcessByPid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillProcessByPid_001 start");
    InitKillingProcessManager();
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    int32_t pid = 99999;
    std::string reason = "test";
    int32_t ret = killingProcessManager.KillProcessByPid(pid, reason);
    EXPECT_EQ(ret, AAFwk::ERR_KILL_PROCESS_NOT_EXIST);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillProcessByPid_001 end");
}

/**
 * @tc.name: KillProcessByPid_002
 * @tc.desc: test KillProcessByPid with isKillPrecedeStart flag.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, KillProcessByPid_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillProcessByPid_002 start");
    InitKillingProcessManager();
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    int32_t pid = 99999;
    std::string reason = "test";
    int32_t ret = killingProcessManager.KillProcessByPid(pid, reason, true);
    EXPECT_EQ(ret, AAFwk::ERR_KILL_PROCESS_NOT_EXIST);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest KillProcessByPid_002 end");
}

/**
 * @tc.name: WaitForRemoteProcessExit_001
 * @tc.desc: test WaitForRemoteProcessExit with empty pid list.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, WaitForRemoteProcessExit_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest WaitForRemoteProcessExit_001 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    std::list<pid_t> pids;
    int64_t startTime = killingProcessManager.SystemTimeMillisecond();
    bool ret = killingProcessManager.WaitForRemoteProcessExit(pids, startTime);
    EXPECT_EQ(ret, true);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest WaitForRemoteProcessExit_001 end");
}

/**
 * @tc.name: WaitForRemoteProcessExit_002
 * @tc.desc: test WaitForRemoteProcessExit with non-existent pid.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, WaitForRemoteProcessExit_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest WaitForRemoteProcessExit_002 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    std::list<pid_t> pids;
    pids.push_back(99999);
    int64_t startTime = killingProcessManager.SystemTimeMillisecond();
    bool ret = killingProcessManager.WaitForRemoteProcessExit(pids, startTime);
    EXPECT_EQ(ret, false);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest WaitForRemoteProcessExit_002 end");
}

/**
 * @tc.name: WaitProcessesExitAndKill_001
 * @tc.desc: test WaitProcessesExitAndKill without appRunningManager.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, WaitProcessesExitAndKill_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest WaitProcessesExitAndKill_001 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    killingProcessManager.appRunningManager_ = nullptr;
    std::list<pid_t> pids;
    int64_t startTime = killingProcessManager.SystemTimeMillisecond();
    int32_t ret = killingProcessManager.WaitProcessesExitAndKill(pids, startTime, "test");
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest WaitProcessesExitAndKill_001 end");
}

/**
 * @tc.name: VerifyKillProcessPermission_001
 * @tc.desc: test VerifyKillProcessPermission by bundleName without appRunningManager.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, VerifyKillProcessPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest VerifyKillProcessPermission_001 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    killingProcessManager.appRunningManager_ = nullptr;
    int32_t ret = killingProcessManager.VerifyKillProcessPermission("test.bundle");
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest VerifyKillProcessPermission_001 end");
}

/**
 * @tc.name: VerifyKillProcessPermission_002
 * @tc.desc: test VerifyKillProcessPermission by token without appRunningManager.
 * @tc.type: FUNC
 */
HWTEST_F(KillingProcessManagerTest, VerifyKillProcessPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest VerifyKillProcessPermission_002 start");
    KillingProcessManager &killingProcessManager = KillingProcessManager::GetInstance();
    killingProcessManager.appRunningManager_ = nullptr;
    sptr<IRemoteObject> token = nullptr;
    int32_t ret = killingProcessManager.VerifyKillProcessPermission(token);
    EXPECT_EQ(ret, ERR_NO_INIT);
    TAG_LOGI(AAFwkTag::TEST, "KillingProcessManagerTest VerifyKillProcessPermission_002 end");
}

} // namespace AppExecFwk
} // namespace OHOS
