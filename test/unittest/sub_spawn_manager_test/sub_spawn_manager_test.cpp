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
#include <vector>
#include <string>

#include "child_process_info.h"
#include "sub_spawn_manager.h"
#include "sub_spawn_record.h"

using namespace testing;
using namespace OHOS;
using namespace OHOS::AppExecFwk;

class SubSpawnManagerTest : public ::testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

/**
 * @tc.name: ShouldUseSubSpawn_001
 * @tc.desc: Test ShouldUseSubSpawn returns false when preloadSoList is empty
 */
HWTEST_F(SubSpawnManagerTest, ShouldUseSubSpawn_001, TestSize.Level1)
{
    std::vector<std::string> emptyList;
    EXPECT_FALSE(SubSpawnManager::GetInstance().ShouldUseSubSpawn(
        CHILD_PROCESS_TYPE_NATIVE_ARGS, emptyList, "libs/arm64", true));
}

/**
 * @tc.name: ShouldUseSubSpawn_002
 * @tc.desc: Test ShouldUseSubSpawn returns false when not isolated mode
 */
HWTEST_F(SubSpawnManagerTest, ShouldUseSubSpawn_002, TestSize.Level1)
{
    std::vector<std::string> soList = {"libentry.so"};
    EXPECT_FALSE(SubSpawnManager::GetInstance().ShouldUseSubSpawn(
        CHILD_PROCESS_TYPE_NATIVE_ARGS, soList, "libs/arm64", false));
}

/**
 * @tc.name: ShouldUseSubSpawn_003
 * @tc.desc: Test ShouldUseSubSpawn returns false when both conditions not met
 */
HWTEST_F(SubSpawnManagerTest, ShouldUseSubSpawn_003, TestSize.Level1)
{
    std::vector<std::string> emptyList;
    EXPECT_FALSE(SubSpawnManager::GetInstance().ShouldUseSubSpawn(
        CHILD_PROCESS_TYPE_NATIVE_ARGS, emptyList, "", false));
}

/**
 * @tc.name: ShouldUseSubSpawn_004
 * @tc.desc: Test ShouldUseSubSpawn returns false when child process type is not native args
 */
HWTEST_F(SubSpawnManagerTest, ShouldUseSubSpawn_004, TestSize.Level1)
{
    std::vector<std::string> soList = {"libentry.so"};
    EXPECT_FALSE(SubSpawnManager::GetInstance().ShouldUseSubSpawn(
        CHILD_PROCESS_TYPE_JS, soList, "libs/arm64", true));
    EXPECT_FALSE(SubSpawnManager::GetInstance().ShouldUseSubSpawn(
        CHILD_PROCESS_TYPE_NATIVE, soList, "libs/arm64", true));
    EXPECT_FALSE(SubSpawnManager::GetInstance().ShouldUseSubSpawn(
        CHILD_PROCESS_TYPE_ARK, soList, "libs/arm64", true));
}

/**
 * @tc.name: ShouldUseSubSpawn_005
 * @tc.desc: Test ShouldUseSubSpawn returns false when nativeLibraryPath is empty
 */
HWTEST_F(SubSpawnManagerTest, ShouldUseSubSpawn_005, TestSize.Level1)
{
    std::vector<std::string> soList = {"libentry.so"};
    EXPECT_FALSE(SubSpawnManager::GetInstance().ShouldUseSubSpawn(
        CHILD_PROCESS_TYPE_NATIVE_ARGS, soList, "", true));
}

/**
 * @tc.name: OnSubSpawnExited_001
 * @tc.desc: Test OnSubSpawnExited with unknown pid does not crash
 */
HWTEST_F(SubSpawnManagerTest, OnSubSpawnExited_001, TestSize.Level1)
{
    SubSpawnManager::GetInstance().OnSubSpawnExited(99999);
    SUCCEED();
}

/**
 * @tc.name: OnChildProcessCreated_001
 * @tc.desc: Test OnChildProcessCreated with unknown uid does not crash
 */
HWTEST_F(SubSpawnManagerTest, OnChildProcessCreated_001, TestSize.Level1)
{
    SubSpawnManager::GetInstance().OnChildProcessCreated(99999, 88888);
    SUCCEED();
}

/**
 * @tc.name: OnChildProcessExited_001
 * @tc.desc: Test OnChildProcessExited with unknown uid does not crash
 */
HWTEST_F(SubSpawnManagerTest, OnChildProcessExited_001, TestSize.Level1)
{
    SubSpawnManager::GetInstance().OnChildProcessExited(99999, 88888);
    SUCCEED();
}

/**
 * @tc.name: OnAppProcessDied_001
 * @tc.desc: Test OnAppProcessDied with unknown uid does not crash
 */
HWTEST_F(SubSpawnManagerTest, OnAppProcessDied_001, TestSize.Level1)
{
    SubSpawnManager::GetInstance().OnAppProcessDied(99999);
    SUCCEED();
}

/**
 * @tc.name: StartChildViaSubSpawn_001
 * @tc.desc: Test StartChildViaSubSpawn returns <= 0 when no subspawn record exists
 */
HWTEST_F(SubSpawnManagerTest, StartChildViaSubSpawn_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    startMsg.procName = "test";
    startMsg.uid = 10000;
    pid_t result = SubSpawnManager::GetInstance().StartChildViaSubSpawn(10000, startMsg);
    EXPECT_LE(result, 0);
}

/**
 * @tc.name: HandleIdleTimeout_001
 * @tc.desc: Test HandleIdleTimeout with unknown uid does not crash
 */
HWTEST_F(SubSpawnManagerTest, HandleIdleTimeout_001, TestSize.Level1)
{
    SubSpawnManager::GetInstance().HandleIdleTimeout(99999);
    SUCCEED();
}

/**
 * @tc.name: SubSpawnRecord_Create_001
 * @tc.desc: Test SubSpawnRecord creation with valid params
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_Create_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20000, "com.test.app", startMsg);
    EXPECT_NE(record, nullptr);
    EXPECT_EQ(record->GetUid(), 20000);
    EXPECT_EQ(record->GetBundleName(), "com.test.app");
}

/**
 * @tc.name: SubSpawnRecord_Create_002
 * @tc.desc: Test SubSpawnRecord creation with invalid params returns nullptr
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_Create_002, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    EXPECT_EQ(SubSpawnRecord::CreateSubSpawnRecord(0, "com.test.app", startMsg), nullptr);
    EXPECT_EQ(SubSpawnRecord::CreateSubSpawnRecord(20000, "", startMsg), nullptr);
}

/**
 * @tc.name: SubSpawnRecord_State_001
 * @tc.desc: Test SubSpawnRecord state transitions
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_State_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20000, "com.test.app", startMsg);
    ASSERT_NE(record, nullptr);
    EXPECT_FALSE(record->IsReady());
    EXPECT_FALSE(record->IsCreating());
    EXPECT_FALSE(record->IsTerminating());
    record->SetState(SubSpawnState::CREATING);
    EXPECT_TRUE(record->IsCreating());
    EXPECT_FALSE(record->IsReady());
    record->SetState(SubSpawnState::READY);
    EXPECT_TRUE(record->IsReady());
    EXPECT_FALSE(record->IsCreating());
    record->SetState(SubSpawnState::TERMINATING);
    EXPECT_TRUE(record->IsTerminating());
    EXPECT_FALSE(record->IsReady());
}

/**
 * @tc.name: SubSpawnRecord_ChildPids_001
 * @tc.desc: Test SubSpawnRecord child pid tracking
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_ChildPids_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20000, "com.test.app", startMsg);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->GetChildProcessCount(), 0u);
    EXPECT_FALSE(record->HasChildProcesses());
    record->AddChildPid(3000);
    record->AddChildPid(3001);
    EXPECT_EQ(record->GetChildProcessCount(), 2u);
    EXPECT_TRUE(record->HasChildProcesses());
    record->RemoveChildPid(3000);
    EXPECT_EQ(record->GetChildProcessCount(), 1u);
    record->RemoveChildPid(3001);
    EXPECT_EQ(record->GetChildProcessCount(), 0u);
    EXPECT_FALSE(record->HasChildProcesses());
}

/**
 * @tc.name: SubSpawnRecord_Diagnostics_001
 * @tc.desc: Test SubSpawnRecord kill reason default empty and set
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_Diagnostics_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20000, "com.test.app", startMsg);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->GetKillReason(), "");
    record->SetKillReason("app_died");
    EXPECT_EQ(record->GetKillReason(), "app_died");
}

/**
 * @tc.name: SubSpawnRecord_CreatedNotified_001
 * @tc.desc: Test SubSpawnRecord created-notified flag defaults false and toggles
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_CreatedNotified_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20000, "com.test.app", startMsg);
    ASSERT_NE(record, nullptr);
    EXPECT_FALSE(record->IsCreatedNotified());
    record->SetCreatedNotified(true);
    EXPECT_TRUE(record->IsCreatedNotified());
    record->SetCreatedNotified(false);
    EXPECT_FALSE(record->IsCreatedNotified());
}

// ======================== helper for found-path tests ========================

namespace {
AppSpawnStartMsg g_testStartMsg;

std::shared_ptr<SubSpawnRecord> MakeReadyRecord(int32_t uid, const std::string &bundleName,
    pid_t subSpawnPid = 5000, const std::string &socketName = "test_socket")
{
    auto record = SubSpawnRecord::CreateSubSpawnRecord(uid, bundleName, g_testStartMsg);
    if (!record) {
        return nullptr;
    }
    record->SetSubSpawnPid(subSpawnPid);
    record->SetSocketName(socketName);
    record->SetState(SubSpawnState::READY);
    record->SetCreatedNotified(true);
    return record;
}

void InsertRecord(int32_t uid, const std::shared_ptr<SubSpawnRecord> &record)
{
    auto &mgr = SubSpawnManager::GetInstance();
    mgr.subSpawnMap_[uid] = record;
    if (record->GetSubSpawnPid() > 0) {
        mgr.pidToRecordMap_[record->GetSubSpawnPid()] = record;
    }
}

void CleanupMaps()
{
    auto &mgr = SubSpawnManager::GetInstance();
    std::lock_guard<std::mutex> lock(mgr.mutex_);
    mgr.subSpawnMap_.clear();
    mgr.pidToRecordMap_.clear();
}
} // namespace

// ======================== SerializePreloadSoList tests ========================

// NOTE: SerializePreloadSoList is a file-local function in app_spawn_client.cpp
// (anonymous namespace), not directly callable from this test target.
// It is tested indirectly through the AppspawnSetExtMsgMore path in
// app_mgr_service_inner tests. Direct TDD for it requires a separate
// test target compiling app_spawn_client.cpp.

// ======================== OnSubSpawnExited found-path ========================

/**
 * @tc.name: OnSubSpawnExited_002
 * @tc.desc: Record found, CreatedNotified=true → Died notify dispatched, record erased from both maps
 */
HWTEST_F(SubSpawnManagerTest, OnSubSpawnExited_002, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20001, "com.test.exited");
    ASSERT_NE(record, nullptr);
    InsertRecord(20001, record);

    SubSpawnManager::GetInstance().OnSubSpawnExited(5000);

    EXPECT_EQ(SubSpawnManager::GetInstance().subSpawnMap_.find(20001),
        SubSpawnManager::GetInstance().subSpawnMap_.end());
    EXPECT_EQ(SubSpawnManager::GetInstance().pidToRecordMap_.find(5000),
        SubSpawnManager::GetInstance().pidToRecordMap_.end());
    CleanupMaps();
}

/**
 * @tc.name: OnSubSpawnExited_003
 * @tc.desc: Record found, CreatedNotified=false → Died notify skipped, record still erased
 */
HWTEST_F(SubSpawnManagerTest, OnSubSpawnExited_003, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20002, "com.test.skipnotify");
    ASSERT_NE(record, nullptr);
    record->SetCreatedNotified(false);
    InsertRecord(20002, record);

    SubSpawnManager::GetInstance().OnSubSpawnExited(5000);

    EXPECT_EQ(SubSpawnManager::GetInstance().subSpawnMap_.find(20002),
        SubSpawnManager::GetInstance().subSpawnMap_.end());
    EXPECT_EQ(SubSpawnManager::GetInstance().pidToRecordMap_.find(5000),
        SubSpawnManager::GetInstance().pidToRecordMap_.end());
    CleanupMaps();
}

/**
 * @tc.name: OnSubSpawnExited_004
 * @tc.desc: Record found, killReason empty → set to "external kill"
 */
HWTEST_F(SubSpawnManagerTest, OnSubSpawnExited_004, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20003, "com.test.extkill");
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->GetKillReason(), "");
    InsertRecord(20003, record);

    SubSpawnManager::GetInstance().OnSubSpawnExited(5000);

    EXPECT_EQ(record->GetKillReason(), "external kill");
    CleanupMaps();
}

/**
 * @tc.name: OnSubSpawnExited_005
 * @tc.desc: Record found, killReason already set → preserved, not overwritten
 */
HWTEST_F(SubSpawnManagerTest, OnSubSpawnExited_005, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20004, "com.test.preserve");
    ASSERT_NE(record, nullptr);
    record->SetKillReason("app_died");
    InsertRecord(20004, record);

    SubSpawnManager::GetInstance().OnSubSpawnExited(5000);

    EXPECT_EQ(record->GetKillReason(), "app_died");
    CleanupMaps();
}

// ======================== StartChildViaSubSpawn state branches ========================

/**
 * @tc.name: StartChildViaSubSpawn_002
 * @tc.desc: Record in CREATING state → return -1 (ERR_CREATING)
 */
HWTEST_F(SubSpawnManagerTest, StartChildViaSubSpawn_002, TestSize.Level1)
{
    CleanupMaps();
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20005, "com.test.creating", g_testStartMsg);
    ASSERT_NE(record, nullptr);
    record->SetState(SubSpawnState::CREATING);
    InsertRecord(20005, record);

    AppSpawnStartMsg startMsg;
    pid_t result = SubSpawnManager::GetInstance().StartChildViaSubSpawn(20005, startMsg);
    EXPECT_EQ(result, -1);
    CleanupMaps();
}

/**
 * @tc.name: StartChildViaSubSpawn_003
 * @tc.desc: Record in TERMINATING state → return -1 (ERR_TERMINATING)
 */
HWTEST_F(SubSpawnManagerTest, StartChildViaSubSpawn_003, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20006, "com.test.term");
    ASSERT_NE(record, nullptr);
    record->SetState(SubSpawnState::TERMINATING);
    InsertRecord(20006, record);

    AppSpawnStartMsg startMsg;
    pid_t result = SubSpawnManager::GetInstance().StartChildViaSubSpawn(20006, startMsg);
    EXPECT_EQ(result, -1);
    CleanupMaps();
}

/**
 * @tc.name: StartChildViaSubSpawn_004
 * @tc.desc: Record READY but socketName empty → return -1 (ERR_SOCKET_NAME_EMPTY)
 */
HWTEST_F(SubSpawnManagerTest, StartChildViaSubSpawn_004, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20007, "com.test.nosock", 5000, "");
    ASSERT_NE(record, nullptr);
    InsertRecord(20007, record);

    AppSpawnStartMsg startMsg;
    pid_t result = SubSpawnManager::GetInstance().StartChildViaSubSpawn(20007, startMsg);
    EXPECT_EQ(result, -1);
    CleanupMaps();
}

// ======================== OnChildProcessCreated/Exited found-path ========================

/**
 * @tc.name: OnChildProcessCreated_002
 * @tc.desc: READY record → child pid added, idle timer cancelled
 */
HWTEST_F(SubSpawnManagerTest, OnChildProcessCreated_002, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20008, "com.test.created");
    ASSERT_NE(record, nullptr);
    record->SetIdleTimerActive(true);
    InsertRecord(20008, record);

    SubSpawnManager::GetInstance().OnChildProcessCreated(20008, 6001);

    EXPECT_EQ(record->GetChildProcessCount(), 1u);
    EXPECT_TRUE(record->HasChildProcesses());
    EXPECT_FALSE(record->IsIdleTimerActive());
    CleanupMaps();
}

/**
 * @tc.name: OnChildProcessCreated_003
 * @tc.desc: Record in CREATING (not READY) → no-op, child pid not added
 */
HWTEST_F(SubSpawnManagerTest, OnChildProcessCreated_003, TestSize.Level1)
{
    CleanupMaps();
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20009, "com.test.notready", g_testStartMsg);
    ASSERT_NE(record, nullptr);
    record->SetState(SubSpawnState::CREATING);
    InsertRecord(20009, record);

    SubSpawnManager::GetInstance().OnChildProcessCreated(20009, 6002);

    EXPECT_EQ(record->GetChildProcessCount(), 0u);
    CleanupMaps();
}

/**
 * @tc.name: OnChildProcessExited_002
 * @tc.desc: READY record, last child exits → idle timer started
 */
HWTEST_F(SubSpawnManagerTest, OnChildProcessExited_002, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20010, "com.test.lastchild");
    ASSERT_NE(record, nullptr);
    record->AddChildPid(6003);
    InsertRecord(20010, record);

    SubSpawnManager::GetInstance().OnChildProcessExited(20010, 6003);

    EXPECT_EQ(record->GetChildProcessCount(), 0u);
    EXPECT_FALSE(record->HasChildProcesses());
    EXPECT_TRUE(record->IsIdleTimerActive());
    CleanupMaps();
}

/**
 * @tc.name: OnChildProcessExited_003
 * @tc.desc: READY record, other children remain → no idle timer
 */
HWTEST_F(SubSpawnManagerTest, OnChildProcessExited_003, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20011, "com.test.remains");
    ASSERT_NE(record, nullptr);
    record->AddChildPid(6004);
    record->AddChildPid(6005);
    InsertRecord(20011, record);

    SubSpawnManager::GetInstance().OnChildProcessExited(20011, 6004);

    EXPECT_EQ(record->GetChildProcessCount(), 1u);
    EXPECT_TRUE(record->HasChildProcesses());
    EXPECT_FALSE(record->IsIdleTimerActive());
    CleanupMaps();
}

// ======================== OnAppProcessDied found-path ========================

/**
 * @tc.name: OnAppProcessDied_002
 * @tc.desc: Record already TERMINATING → early return, record NOT erased
 */
HWTEST_F(SubSpawnManagerTest, OnAppProcessDied_002, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20012, "com.test.alreadyterm");
    ASSERT_NE(record, nullptr);
    record->SetState(SubSpawnState::TERMINATING);
    InsertRecord(20012, record);

    SubSpawnManager::GetInstance().OnAppProcessDied(20012);

    // Record should still exist (early return, no cleanup)
    EXPECT_NE(SubSpawnManager::GetInstance().subSpawnMap_.find(20012),
        SubSpawnManager::GetInstance().subSpawnMap_.end());
    CleanupMaps();
}

/**
 * @tc.name: OnAppProcessDied_003
 * @tc.desc: READY record with valid pid → record erased from both maps, killReason=app_died
 */
HWTEST_F(SubSpawnManagerTest, OnAppProcessDied_003, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20013, "com.test.appdied");
    ASSERT_NE(record, nullptr);
    InsertRecord(20013, record);

    SubSpawnManager::GetInstance().OnAppProcessDied(20013);

    EXPECT_EQ(SubSpawnManager::GetInstance().subSpawnMap_.find(20013),
        SubSpawnManager::GetInstance().subSpawnMap_.end());
    EXPECT_EQ(SubSpawnManager::GetInstance().pidToRecordMap_.find(5000),
        SubSpawnManager::GetInstance().pidToRecordMap_.end());
    EXPECT_EQ(record->GetKillReason(), "app_died");
    EXPECT_EQ(record->GetState(), SubSpawnState::TERMINATING);
    CleanupMaps();
}

/**
 * @tc.name: OnAppProcessDied_004
 * @tc.desc: Record with subSpawnPid=0 (CREATING) → no kill attempt, record erased
 */
HWTEST_F(SubSpawnManagerTest, OnAppProcessDied_004, TestSize.Level1)
{
    CleanupMaps();
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20014, "com.test.creatingdied", g_testStartMsg);
    ASSERT_NE(record, nullptr);
    record->SetState(SubSpawnState::CREATING);
    record->SetCreatedNotified(true);
    // Do NOT set subSpawnPid (stays 0) → no pidToRecordMap_ insertion
    {
        auto &mgr = SubSpawnManager::GetInstance();
        std::lock_guard<std::mutex> lock(mgr.mutex_);
        mgr.subSpawnMap_[20014] = record;
    }

    SubSpawnManager::GetInstance().OnAppProcessDied(20014);

    EXPECT_EQ(SubSpawnManager::GetInstance().subSpawnMap_.find(20014),
        SubSpawnManager::GetInstance().subSpawnMap_.end());
    EXPECT_EQ(record->GetKillReason(), "app_died");
    CleanupMaps();
}

// ======================== HandleIdleTimeout found-path ========================

/**
 * @tc.name: HandleIdleTimeout_002
 * @tc.desc: Record has children → skip, record preserved
 */
HWTEST_F(SubSpawnManagerTest, HandleIdleTimeout_002, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20015, "com.test.haschildren");
    ASSERT_NE(record, nullptr);
    record->SetIdleTimerActive(true);
    record->AddChildPid(6006);
    InsertRecord(20015, record);

    SubSpawnManager::GetInstance().HandleIdleTimeout(20015);

    // Record should still exist (skipped due to children)
    EXPECT_NE(SubSpawnManager::GetInstance().subSpawnMap_.find(20015),
        SubSpawnManager::GetInstance().subSpawnMap_.end());
    CleanupMaps();
}

/**
 * @tc.name: HandleIdleTimeout_003
 * @tc.desc: Record has no children but idle timer inactive → skip
 */
HWTEST_F(SubSpawnManagerTest, HandleIdleTimeout_003, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20016, "com.test.inactive");
    ASSERT_NE(record, nullptr);
    record->SetIdleTimerActive(false);
    InsertRecord(20016, record);

    SubSpawnManager::GetInstance().HandleIdleTimeout(20016);

    EXPECT_NE(SubSpawnManager::GetInstance().subSpawnMap_.find(20016),
        SubSpawnManager::GetInstance().subSpawnMap_.end());
    CleanupMaps();
}

/**
 * @tc.name: HandleIdleTimeout_004
 * @tc.desc: Record already TERMINATING → skip, preserved
 */
HWTEST_F(SubSpawnManagerTest, HandleIdleTimeout_004, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20017, "com.test.idleterm");
    ASSERT_NE(record, nullptr);
    record->SetIdleTimerActive(true);
    record->SetState(SubSpawnState::TERMINATING);
    InsertRecord(20017, record);

    SubSpawnManager::GetInstance().HandleIdleTimeout(20017);

    EXPECT_NE(SubSpawnManager::GetInstance().subSpawnMap_.find(20017),
        SubSpawnManager::GetInstance().subSpawnMap_.end());
    CleanupMaps();
}

/**
 * @tc.name: HandleIdleTimeout_005
 * @tc.desc: Valid timeout: no children, timer active → record erased, killReason=idle_timeout
 */
HWTEST_F(SubSpawnManagerTest, HandleIdleTimeout_005, TestSize.Level1)
{
    CleanupMaps();
    auto record = MakeReadyRecord(20018, "com.test.idletimeout");
    ASSERT_NE(record, nullptr);
    record->SetIdleTimerActive(true);
    InsertRecord(20018, record);

    SubSpawnManager::GetInstance().HandleIdleTimeout(20018);

    EXPECT_EQ(SubSpawnManager::GetInstance().subSpawnMap_.find(20018),
        SubSpawnManager::GetInstance().subSpawnMap_.end());
    EXPECT_EQ(SubSpawnManager::GetInstance().pidToRecordMap_.find(5000),
        SubSpawnManager::GetInstance().pidToRecordMap_.end());
    EXPECT_EQ(record->GetKillReason(), "idle_timeout");
    EXPECT_EQ(record->GetState(), SubSpawnState::TERMINATING);
    CleanupMaps();
}

// ======================== SubSpawnRecord additional coverage ========================

/**
 * @tc.name: SubSpawnRecord_SocketName_001
 * @tc.desc: SocketName getter/setter and initial empty state
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_SocketName_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20019, "com.test.sock", startMsg);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->GetSocketName(), "");
    record->SetSocketName("test_socket_name");
    EXPECT_EQ(record->GetSocketName(), "test_socket_name");
}

/**
 * @tc.name: SubSpawnRecord_SubSpawnPid_001
 * @tc.desc: SubSpawnPid getter/setter and initial zero state
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_SubSpawnPid_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20020, "com.test.pid", startMsg);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->GetSubSpawnPid(), 0);
    record->SetSubSpawnPid(12345);
    EXPECT_EQ(record->GetSubSpawnPid(), 12345);
}

/**
 * @tc.name: SubSpawnRecord_IdleTimer_001
 * @tc.desc: IdleTimerActive flag defaults false and toggles
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_IdleTimer_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20021, "com.test.idle", startMsg);
    ASSERT_NE(record, nullptr);
    EXPECT_FALSE(record->IsIdleTimerActive());
    record->SetIdleTimerActive(true);
    EXPECT_TRUE(record->IsIdleTimerActive());
    record->SetIdleTimerActive(false);
    EXPECT_FALSE(record->IsIdleTimerActive());
}

/**
 * @tc.name: SubSpawnRecord_StartTime_001
 * @tc.desc: StartTime is set at construction, not epoch
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_StartTime_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    auto before = std::chrono::system_clock::now();
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20022, "com.test.time", startMsg);
    auto after = std::chrono::system_clock::now();
    ASSERT_NE(record, nullptr);
    EXPECT_GE(record->GetStartTime(), before);
    EXPECT_LE(record->GetStartTime(), after);
}

/**
 * @tc.name: SubSpawnRecord_StartMsg_001
 * @tc.desc: StartMsg is stored and retrievable
 */
HWTEST_F(SubSpawnManagerTest, SubSpawnRecord_StartMsg_001, TestSize.Level1)
{
    AppSpawnStartMsg startMsg;
    startMsg.procName = "test_proc";
    startMsg.uid = 20023;
    auto record = SubSpawnRecord::CreateSubSpawnRecord(20023, "com.test.msg", startMsg);
    ASSERT_NE(record, nullptr);
    EXPECT_EQ(record->GetStartMsg().procName, "test_proc");
    EXPECT_EQ(record->GetStartMsg().uid, 20023);
}
