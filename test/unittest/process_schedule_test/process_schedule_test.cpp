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

#include <string>
#include <thread>
#include <vector>

// Workaround to access private members for testing
// NOLINTNEXTLINE(g.nam.03-cpp): Redefining C++ keyword 'private' for unit testing
#define private public
#include "task_mgr/process_schedule.h"
#include "task_mgr/resource_control_proxy.h"
#undef private

#include "task_mgr/common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr pid_t TEST_PID_A = 4312;
constexpr pid_t TEST_PID_B = 4401;
constexpr int32_t TEST_PID_C = 4500;
constexpr pid_t TEST_CREATOR_PID = 3268;
constexpr int32_t TEST_CREATOR_UID = 20010001;
const std::string TEST_TASK_HIGH = "sched_t1_high";
const std::string TEST_TASK_MEDIUM = "sched_t2_medium";
const std::string TEST_TASK_LOW = "sched_t3_low";

ScheduleAttributionInfo MakeInfo(const std::string &taskId, int32_t taskLevel, pid_t pid, AttributionTool tool)
{
    ScheduleAttributionInfo info;
    info.taskId = taskId;
    info.taskLevel = taskLevel;
    info.creatorPid = TEST_CREATOR_PID;
    info.creatorUid = TEST_CREATOR_UID;
    info.attribution.pid = pid;
    info.attribution.uid = TEST_CREATOR_UID;
    info.attribution.tool = tool;
    return info;
}

void AddAttribution(ProcessSchedule &sched, const std::string &taskId, int32_t taskLevel,
    pid_t pid, AttributionTool tool)
{
    sched.OnAttributionAdded(MakeInfo(taskId, taskLevel, pid, tool));
    sched.WaitForIdle();
}

void RemoveAttribution(ProcessSchedule &sched, const std::string &taskId, pid_t pid,
    AttributionTool tool = AttributionTool::VIRTUAL_SCREEN_ABILITY)
{
    sched.OnAttributionRemoved(MakeInfo(taskId, TASK_PRIORITY_MEDIUM, pid, tool),
        ScheduleTrigger::ATTRIBUTION_REMOVED);
    sched.WaitForIdle();
}

void ChangeTaskLevel(ProcessSchedule &sched, const std::string &taskId, int32_t newLevel)
{
    sched.OnTaskLevelChanged(taskId, newLevel, ScheduleTrigger::SYSTEM_LOAD_CHANGED);
    sched.WaitForIdle();
}

void ProcessDeath(ProcessSchedule &sched, pid_t pid)
{
    sched.OnProcessDied(pid, TEST_CREATOR_UID);
    sched.WaitForIdle();
}

void WaitForDebounce(ProcessSchedule &sched)
{
    auto handle = ResourceControlProxy::GetInstance().GetDebounceTaskForTest(TEST_PID_A);
    if (handle) {
        handle.Sync();
    }
    sched.WaitForIdle();
}

void ResetSchedule()
{
    ProcessSchedule::GetInstance().ResetForTest();
}

ProcessScheduleEvent LastEvent(pid_t pid)
{
    auto events = ResourceControlProxy::GetInstance().GetEventHistoryForTest(pid);
    EXPECT_FALSE(events.empty());
    return events.empty() ? ProcessScheduleEvent{} : events.back();
}

const TaskScheduleContext *FindTask(const std::vector<TaskScheduleContext> &tasks, const std::string &taskId)
{
    for (const auto &task : tasks) {
        if (task.taskId == taskId) {
            return &task;
        }
    }
    return nullptr;
}

const TaskScheduleContext *FindCurrentTask(const ProcessScheduleEvent &event, const std::string &taskId)
{
    return FindTask(event.currentTasks, taskId);
}

const TaskScheduleContext *FindPreviousTask(const ProcessScheduleEvent &event, const std::string &taskId)
{
    return FindTask(event.previousTasks, taskId);
}

void ResetScenario()
{
    ResetSchedule();
    ResourceControlProxy::GetInstance().SetEventRecordingForTest(true);
}
}  // namespace

class ProcessScheduleTest : public testing::Test {
public:
    static void SetUpTestCase()
    {
    }
    static void TearDownTestCase()
    {
    }
    void SetUp() override
    {
        ResetSchedule();
        ResourceControlProxy::GetInstance().SetEventRecordingForTest(true);
    }
    void TearDown() override
    {
        ResetSchedule();
    }
};

/**
 * @tc.number: ProcessSchedule_MultiTaskHighestPriority_0100
 * @tc.name: one process managed by several tasks takes the highest level (AC-6.1)
 * @tc.desc: T1(HIGH) + T2(LOW) on pid A -> effectiveLevel HIGH; adding T3(MEDIUM) keeps HIGH.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_MultiTaskHighestPriority_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    AddAttribution(sched, TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::CLI);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_HIGH);
    }
    std::string dump = sched.DumpSchedule();
    EXPECT_NE(dump.find("effectiveLevel [HIGH(300)]"), std::string::npos);
    AddAttribution(sched, TEST_TASK_MEDIUM, TASK_PRIORITY_MEDIUM, TEST_PID_A, AttributionTool::A2A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_HIGH);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskLevels.size(), 2);
    }
}

/**
 * @tc.number: ProcessSchedule_DegradeToNextPriority_0100
 * @tc.name: removing the highest task degrades to the next level; the last removal fully
 *           restores the process (AC-6.2)
 * @tc.desc: three tasks on one pid; remove HIGH -> MEDIUM, remove MEDIUM -> LOW, remove LOW
 *           -> pid entry disappears (natural restore).
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_DegradeToNextPriority_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    AddAttribution(sched, TEST_TASK_MEDIUM, TASK_PRIORITY_MEDIUM, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    AddAttribution(sched, TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);

    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_MEDIUM);
    }
    RemoveAttribution(sched, TEST_TASK_MEDIUM, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_LOW);
    }
    RemoveAttribution(sched, TEST_TASK_LOW, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_TRUE(sched.pidEntries_.empty());
        EXPECT_TRUE(sched.taskToPids_.empty());
    }
    std::string dump = sched.DumpSchedule();
    EXPECT_NE(dump.find("ProcessSchedule index: tasks=0 pids=0"), std::string::npos);
}

/**
 * @tc.number: ProcessSchedule_TaskRefIdempotent_0100
 * @tc.name: duplicate attribution registration/removal keeps the index consistent.
 * @tc.desc: registering two tasks on one pid builds two index entries; removing one task
 *           drops only its entry and a duplicate removal of the same attribution is a
 *           safe no-op (idempotent channel).
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_TaskRefIdempotent_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    AddAttribution(sched, TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::CLI);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.taskToPids_[TEST_TASK_HIGH].count(TEST_PID_A), 1);
        EXPECT_EQ(sched.taskToPids_[TEST_TASK_LOW].count(TEST_PID_A), 1);
    }
    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.taskToPids_.count(TEST_TASK_HIGH), 0);
        EXPECT_EQ(sched.taskToPids_[TEST_TASK_LOW].count(TEST_PID_A), 1);
    }
    // duplicate removal of the same attribution is a safe no-op (idempotent channel)
    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.taskToPids_.count(TEST_TASK_HIGH), 0);
        EXPECT_EQ(sched.taskToPids_[TEST_TASK_LOW].count(TEST_PID_A), 1);
    }
}

/**
 * @tc.number: ProcessSchedule_ProcessDeathCleanup_0100
 * @tc.name: a died process is removed with its control references, other processes untouched
 * @tc.desc: two pids share one task; killing pid B cleans only pid B and keeps pid A intact.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_ProcessDeathCleanup_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_B, AttributionTool::CLI);
    AddAttribution(sched, TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_B, AttributionTool::INSIGHT);

    ProcessDeath(sched, TEST_PID_B);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_.count(TEST_PID_B), 0);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_HIGH);
        EXPECT_EQ(sched.taskToPids_[TEST_TASK_HIGH].count(TEST_PID_A), 1);
        EXPECT_EQ(sched.taskToPids_[TEST_TASK_HIGH].count(TEST_PID_B), 0);
        EXPECT_EQ(sched.taskToPids_.count(TEST_TASK_LOW), 0);
    }
}

/**
 * @tc.number: ProcessSchedule_AllProcessesDeadTaskAlive_0100
 * @tc.name: killing every managed process leaves no dangling schedule state (AC-6.3 / R-8)
 * @tc.desc: after all pids of a task die the task index entry is gone while the service
 *           side (task records) is out of scope here and asserted by the integration case.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_AllProcessesDeadTaskAlive_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_B, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    ProcessDeath(sched, TEST_PID_A);
    ProcessDeath(sched, TEST_PID_B);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_TRUE(sched.pidEntries_.empty());
        EXPECT_TRUE(sched.taskToPids_.empty());
    }
}

/**
 * @tc.number: ProcessSchedule_Concurrent10Tasks_0100
 * @tc.name: ten concurrent tasks add/remove attributions without corrupting state (AC-6.4)
 * @tc.desc: ten threads each register and then remove one task-pid pair; afterwards the
 *           index and the entries must all be empty.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_Concurrent10Tasks_0100, TestSize.Level3)
{
    auto &sched = ProcessSchedule::GetInstance();
    std::vector<std::thread> threads;
    for (int32_t i = 0; i < 10; i++) {
        threads.emplace_back([&sched, i]() {
            std::string taskId = "sched_concurrent_" + std::to_string(i);
            pid_t pid = TEST_PID_C + i;
            AddAttribution(sched, taskId, TASK_PRIORITY_MEDIUM, pid, AttributionTool::VIRTUAL_SCREEN_ABILITY);
            RemoveAttribution(sched, taskId, pid);
        });
    }
    for (auto &thread : threads) {
        thread.join();
    }
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_TRUE(sched.taskToPids_.empty());
        EXPECT_TRUE(sched.pidEntries_.empty());
    }
}

/**
 * @tc.number: ProcessSchedule_OnAttributionAdded_0100
 * @tc.name: OnAttributionAdded rejects non-positive pids (early-return branch).
 * @tc.desc: pid 0 and negative pids must not create any index entry.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_OnAttributionAdded_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, 0, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, -1, AttributionTool::CLI);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_TRUE(sched.taskToPids_.empty());
        EXPECT_TRUE(sched.pidEntries_.empty());
    }
}

/**
 * @tc.number: ProcessSchedule_OnAttributionAdded_0200
 * @tc.name: a duplicated attribution grows the reference count, not the merge state.
 * @tc.desc: adding the same taskId+pid twice mirrors the two task-side attribution records
 *           (refs=2) while the task level entry stays single.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_OnAttributionAdded_0200, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    proxy.SetDebounceWindowMsForTest(20);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskRefs[TEST_TASK_HIGH], 2);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskLevels.size(), 1);
        EXPECT_EQ(sched.taskToPids_[TEST_TASK_HIGH].size(), 1);
    }
    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskRefs[TEST_TASK_HIGH], 1);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskStates[TEST_TASK_HIGH].toolRefs.size(), 1u);
    }
    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_A);
    WaitForDebounce(sched);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_TRUE(sched.pidEntries_.empty());
        EXPECT_TRUE(sched.taskToPids_.empty());
    }
    EXPECT_EQ(LastEvent(TEST_PID_A).action, ScheduleAction::END);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 2u);
}

/**
 * @tc.number: ProcessSchedule_RefCountRemoval_0100
 * @tc.name: removing one of two same-pid attributions keeps the merge state (F-01 regression).
 * @tc.desc: register the same taskId+pid twice, remove once: the pid must stay in the index
 *           with its level and one live reference; only the second removal releases it.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_RefCountRemoval_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        ASSERT_EQ(sched.pidEntries_.count(TEST_PID_A), 1);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_HIGH);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskRefs[TEST_TASK_HIGH], 1);
    }
    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_A, AttributionTool::CLI);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_TRUE(sched.pidEntries_.empty());
        EXPECT_TRUE(sched.taskToPids_.empty());
    }
}

/**
 * @tc.number: ProcessSchedule_OnProcessDied_0200
 * @tc.name: OnProcessDied guards: non-positive pid and unknown pid (no-op branches).
 * @tc.desc: both calls must leave state untouched and must not crash.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_OnProcessDied_0200, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    ProcessDeath(sched, 0);
    ProcessDeath(sched, -1);
    ProcessDeath(sched, TEST_PID_B);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    ProcessDeath(sched, TEST_PID_B);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_.count(TEST_PID_A), 1);
    }
}

/**
 * @tc.number: ProcessSchedule_OnAttributionRemoved_0100
 * @tc.name: OnAttributionRemoved guards: non-positive pid, unknown pid, unknown taskId.
 * @tc.desc: All three no-op branches must leave the existing state untouched.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_OnAttributionRemoved_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    RemoveAttribution(sched, TEST_TASK_HIGH, 0);
    RemoveAttribution(sched, TEST_TASK_HIGH, -1);
    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_B);
    RemoveAttribution(sched, std::string("no_such_task"), TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        ASSERT_EQ(sched.pidEntries_.count(TEST_PID_A), 1);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskLevels.count(TEST_TASK_HIGH), 1);
    }
}

/**
 * @tc.number: ProcessSchedule_OnTaskLevelChanged_0100
 * @tc.name: OnTaskLevelChanged no-ops on an unknown task (miss branch).
 * @tc.desc: changing the level of a task that has no index entry must not create state.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_OnTaskLevelChanged_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    ChangeTaskLevel(sched, std::string("ghost_task"), TASK_PRIORITY_HIGH);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_TRUE(sched.taskToPids_.empty());
        EXPECT_TRUE(sched.pidEntries_.empty());
    }
}

/**
 * @tc.number: ProcessSchedule_MidRemovalKeepsLevel_0100
 * @tc.name: removing one of two same-level tasks keeps the merged level (unchanged-merge branch).
 * @tc.desc: two HIGH tasks share a pid; removing one leaves the pid at HIGH (no downgrade),
 *           the remaining task still owns the pid.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_MidRemovalKeepsLevel_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    AddAttribution(sched, std::string("sched_t1b_high"), TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        ASSERT_EQ(sched.pidEntries_.count(TEST_PID_A), 1);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_HIGH);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskLevels.size(), 1);
    }
    std::string dump = sched.DumpSchedule();
    EXPECT_NE(dump.find("effectiveLevel [HIGH(300)]"), std::string::npos);
}

/**
 * @tc.number: ProcessSchedule_DumpSchedule_0100
 * @tc.name: DumpSchedule renders the empty and populated branches.
 * @tc.desc: empty state prints zero totals; populated state prints the task/pid index
 *           lines with the effective levels.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_DumpSchedule_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    std::string emptyDump = sched.DumpSchedule();
    EXPECT_NE(emptyDump.find("ProcessSchedule index: tasks=0 pids=0"), std::string::npos);

    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    std::string dump = sched.DumpSchedule();
    EXPECT_NE(dump.find("tasks=1 pids=1"), std::string::npos);
    EXPECT_NE(dump.find("task #" + TEST_TASK_HIGH + " -> pids [" + std::to_string(TEST_PID_A) + "]"),
        std::string::npos);
    EXPECT_NE(dump.find("pid " + std::to_string(TEST_PID_A) + " uid " + std::to_string(TEST_CREATOR_UID) +
        " -> tasks [" + TEST_TASK_HIGH + "]"), std::string::npos);
}

/**
 * @tc.number: ProcessSchedule_TaskLevelChanged_0100
 * @tc.name: a re-judged task level re-merges its pids; unchanged merges are not propagated
 * @tc.desc: two tasks share pid A; raising the LOW task to HIGH lifts the pid; changing the
 *           HIGH task to MEDIUM drops the pid to MEDIUM when it is the only HIGH left.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_TaskLevelChanged_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    AddAttribution(sched, TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    ChangeTaskLevel(sched, TEST_TASK_LOW, TASK_PRIORITY_HIGH);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_HIGH);
    }
    ChangeTaskLevel(sched, TEST_TASK_HIGH, TASK_PRIORITY_MEDIUM);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_HIGH);
    }
    ChangeTaskLevel(sched, TEST_TASK_LOW, TASK_PRIORITY_MEDIUM);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_MEDIUM);
    }
}
/**
 * @tc.number: ProcessSchedule_ControlledEventContext_0100
 * @tc.name: controlled tasks carry complete fields and primary tool selection
 * @tc.desc: CLI and Virtual Screen Ability are controlled; other tools never affect max or events.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_ControlledEventContext_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    proxy.SetDebounceWindowMsForTest(20);
    AddAttribution(sched, TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    AddAttribution(sched, TEST_TASK_MEDIUM, TASK_PRIORITY_MEDIUM, TEST_PID_A, AttributionTool::A2A);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_HIGH);
    EXPECT_EQ(proxy.GetLastNotifiedPrimaryTool(TEST_PID_A), AttributionTool::CLI);
    auto initial = LastEvent(TEST_PID_A);
    EXPECT_EQ(initial.action, ScheduleAction::START);
    EXPECT_EQ(initial.trigger, ScheduleTrigger::ATTRIBUTION_ADDED);
    EXPECT_EQ(initial.managedPid, TEST_PID_A);
    EXPECT_EQ(initial.managedUid, TEST_CREATOR_UID);
    EXPECT_EQ(initial.previousEffectiveLevel, TASK_PRIORITY_LOW);
    EXPECT_EQ(initial.effectiveLevel, TASK_PRIORITY_HIGH);
    ASSERT_EQ(initial.currentTasks.size(), 2u);
    const auto *lowTask = FindCurrentTask(initial, TEST_TASK_LOW);
    const auto *highTask = FindCurrentTask(initial, TEST_TASK_HIGH);
    ASSERT_NE(lowTask, nullptr);
    ASSERT_NE(highTask, nullptr);
    EXPECT_EQ(lowTask->creatorPid, TEST_CREATOR_PID);
    EXPECT_EQ(lowTask->creatorUid, TEST_CREATOR_UID);
    EXPECT_EQ(lowTask->tools, std::set<AttributionTool>{AttributionTool::VIRTUAL_SCREEN_ABILITY});
    EXPECT_EQ(highTask->tools, std::set<AttributionTool>{AttributionTool::CLI});
    EXPECT_EQ(FindCurrentTask(initial, TEST_TASK_MEDIUM), nullptr);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskStates.size(), 2);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskLevels.size(), 2);
    }
    ChangeTaskLevel(sched, TEST_TASK_LOW, TASK_PRIORITY_HIGH);
    EXPECT_EQ(proxy.GetLastNotifiedPrimaryTool(TEST_PID_A), AttributionTool::CLI);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 2u);
    ChangeTaskLevel(sched, TEST_TASK_HIGH, TASK_PRIORITY_MEDIUM);
    ChangeTaskLevel(sched, TEST_TASK_LOW, TASK_PRIORITY_MEDIUM);
    WaitForDebounce(sched);
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_MEDIUM);
    EXPECT_EQ(proxy.GetLastNotifiedPrimaryTool(TEST_PID_A), AttributionTool::VIRTUAL_SCREEN_ABILITY);
    auto latest = LastEvent(TEST_PID_A);
    EXPECT_EQ(latest.previousEffectiveLevel, TASK_PRIORITY_HIGH);
    EXPECT_EQ(latest.effectiveLevel, TASK_PRIORITY_MEDIUM);
    EXPECT_EQ(latest.currentPrimaryTool, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    EXPECT_TRUE(latest.debounceExpired);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 3u);
}

/**
 * @tc.number: ProcessSchedule_ControlledTaskMatrix_0100
 * @tc.name: controlled task and creator matrix survives process-death END
 * @tc.desc: different creators and controlled tools are preserved; uncontrolled tools are excluded.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_ControlledTaskMatrix_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    const std::string cliTask = "matrix_cli";
    const std::string virtualTask = "matrix_virtual";
    auto cli = MakeInfo(cliTask, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    auto virtualAbility = MakeInfo(virtualTask, TASK_PRIORITY_HIGH, TEST_PID_A,
        AttributionTool::VIRTUAL_SCREEN_ABILITY);
    virtualAbility.creatorPid = TEST_CREATOR_PID + 1;
    virtualAbility.creatorUid = TEST_CREATOR_UID + 1;
    sched.OnAttributionAdded(cli);
    sched.OnAttributionAdded(virtualAbility);
    AddAttribution(sched, TEST_TASK_MEDIUM, TASK_PRIORITY_MEDIUM, TEST_PID_A, AttributionTool::A2A);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 1u);
    ProcessSchedule::Snapshot latest;
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        latest = sched.BuildSnapshot(TEST_PID_A, sched.pidEntries_[TEST_PID_A]);
    }
    EXPECT_EQ(latest.effectiveLevel, TASK_PRIORITY_HIGH);
    EXPECT_EQ(latest.primaryTool, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    ASSERT_EQ(latest.tasks.size(), 2u);
    const auto *cliContext = FindTask(latest.tasks, cliTask);
    const auto *virtualContext = FindTask(latest.tasks, virtualTask);
    ASSERT_NE(cliContext, nullptr);
    ASSERT_NE(virtualContext, nullptr);
    EXPECT_EQ(cliContext->creatorPid, TEST_CREATOR_PID);
    EXPECT_EQ(cliContext->creatorUid, TEST_CREATOR_UID);
    EXPECT_EQ(virtualContext->creatorPid, TEST_CREATOR_PID + 1);
    EXPECT_EQ(virtualContext->creatorUid, TEST_CREATOR_UID + 1);
    EXPECT_EQ(FindTask(latest.tasks, TEST_TASK_MEDIUM), nullptr);

    ProcessDeath(sched, TEST_PID_A);
    auto final = LastEvent(TEST_PID_A);
    EXPECT_EQ(final.action, ScheduleAction::END);
    EXPECT_EQ(final.previousPrimaryTool, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    EXPECT_EQ(final.previousEffectiveLevel, TASK_PRIORITY_HIGH);
    EXPECT_EQ(final.effectiveLevel, 0);
    ASSERT_EQ(final.previousTasks.size(), 2u);
    EXPECT_TRUE(final.currentTasks.empty());
    EXPECT_NE(FindPreviousTask(final, cliTask), nullptr);
    EXPECT_NE(FindPreviousTask(final, virtualTask), nullptr);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 2u);
}

/**
 * @tc.number: ProcessSchedule_ControlledEndPreviousContext_0100
 * @tc.name: final removal END keeps the immediately previous controlled context
 * @tc.desc: downgrade emits latest START and the last removal emits END with previous tasks.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_ControlledEndPreviousContext_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    proxy.SetDebounceWindowMsForTest(20);
    AddAttribution(sched, TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::CLI);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A,
        AttributionTool::VIRTUAL_SCREEN_ABILITY);
    auto high = MakeInfo(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A,
        AttributionTool::VIRTUAL_SCREEN_ABILITY);
    sched.OnAttributionRemoved(high, ScheduleTrigger::TASK_FINISHED);
    WaitForDebounce(sched);
    auto downgrade = LastEvent(TEST_PID_A);
    EXPECT_EQ(downgrade.action, ScheduleAction::START);
    EXPECT_EQ(downgrade.effectiveLevel, TASK_PRIORITY_LOW);
    ASSERT_EQ(downgrade.currentTasks.size(), 1u);
    EXPECT_NE(FindCurrentTask(downgrade, TEST_TASK_LOW), nullptr);

    auto low = MakeInfo(TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::CLI);
    sched.OnAttributionRemoved(low, ScheduleTrigger::ATTRIBUTION_REMOVED);
    WaitForDebounce(sched);
    auto final = LastEvent(TEST_PID_A);
    EXPECT_EQ(final.action, ScheduleAction::END);
    EXPECT_EQ(final.trigger, ScheduleTrigger::ATTRIBUTION_REMOVED);
    EXPECT_TRUE(final.debounceExpired);
    EXPECT_EQ(final.previousEffectiveLevel, TASK_PRIORITY_LOW);
    EXPECT_EQ(final.effectiveLevel, 0);
    ASSERT_EQ(final.previousTasks.size(), 1u);
    EXPECT_TRUE(final.currentTasks.empty());
    const auto *previous = FindPreviousTask(final, TEST_TASK_LOW);
    ASSERT_NE(previous, nullptr);
    EXPECT_EQ(previous->creatorPid, TEST_CREATOR_PID);
    EXPECT_EQ(previous->creatorUid, TEST_CREATOR_UID);
    EXPECT_EQ(previous->tools, std::set<AttributionTool>{AttributionTool::CLI});
}

/**
 * @tc.number: ProcessSchedule_TriggerReasonMatrix_0100
 * @tc.name: every scheduling trigger and debounce fact is preserved
 * @tc.desc: registration, foreground, load, removal, finish, timeout, creator death and process death.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_TriggerReasonMatrix_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    auto reset = [&proxy]() {
        ResetScenario();
        proxy.SetDebounceWindowMsForTest(20);
    };
    auto expectLast = [](ScheduleTrigger trigger, ScheduleAction action, bool expired) {
        auto event = LastEvent(TEST_PID_A);
        EXPECT_EQ(event.trigger, trigger);
        EXPECT_EQ(event.action, action);
        EXPECT_EQ(event.debounceExpired, expired);
    };
    auto addHigh = [&sched]() {
        AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    };
    auto change = [&sched](int32_t level, ScheduleTrigger trigger) {
        sched.OnTaskLevelChanged(TEST_TASK_HIGH, level, trigger);
        WaitForDebounce(sched);
    };
    auto remove = [&sched](ScheduleTrigger trigger) {
        auto info = MakeInfo(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
        sched.OnAttributionRemoved(info, trigger);
        WaitForDebounce(sched);
    };

    reset();
    addHigh();
    expectLast(ScheduleTrigger::ATTRIBUTION_ADDED, ScheduleAction::START, false);

    reset();
    addHigh();
    change(TASK_PRIORITY_MEDIUM, ScheduleTrigger::FOREGROUND_CHANGED);
    expectLast(ScheduleTrigger::FOREGROUND_CHANGED, ScheduleAction::START, true);

    reset();
    addHigh();
    change(TASK_PRIORITY_LOW, ScheduleTrigger::SYSTEM_LOAD_CHANGED);
    expectLast(ScheduleTrigger::SYSTEM_LOAD_CHANGED, ScheduleAction::START, true);

    reset();
    addHigh();
    remove(ScheduleTrigger::ATTRIBUTION_REMOVED);
    expectLast(ScheduleTrigger::ATTRIBUTION_REMOVED, ScheduleAction::END, true);

    reset();
    addHigh();
    remove(ScheduleTrigger::TASK_FINISHED);
    expectLast(ScheduleTrigger::TASK_FINISHED, ScheduleAction::END, true);

    reset();
    addHigh();
    remove(ScheduleTrigger::TASK_TIMEOUT);
    expectLast(ScheduleTrigger::TASK_TIMEOUT, ScheduleAction::END, true);

    reset();
    addHigh();
    remove(ScheduleTrigger::CREATOR_DIED);
    expectLast(ScheduleTrigger::CREATOR_DIED, ScheduleAction::END, true);

    reset();
    addHigh();
    ProcessDeath(sched, TEST_PID_A);
    expectLast(ScheduleTrigger::MANAGED_PROCESS_DIED, ScheduleAction::END, false);
}

/**
 * @tc.number: ProcessSchedule_UncontrolledToolsNoEvent_0100
 * @tc.name: uncontrolled tools never produce proxy events
 * @tc.desc: ABILITY/A2A/INSIGHT are ignored, and their changes do not affect controlled state.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_UncontrolledToolsNoEvent_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    AddAttribution(sched, TEST_TASK_MEDIUM, TASK_PRIORITY_MEDIUM, TEST_PID_A, AttributionTool::A2A);
    AddAttribution(sched, TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::INSIGHT);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), 0);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 0u);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_TRUE(sched.pidEntries_.empty());
    }
    AddAttribution(sched, TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::CLI);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_LOW);
    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    RemoveAttribution(sched, TEST_TASK_MEDIUM, TEST_PID_A, AttributionTool::A2A);
    RemoveAttribution(sched, TEST_TASK_LOW, TEST_PID_A, AttributionTool::INSIGHT);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_LOW);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 1u);
}

/**
 * @tc.number: ProcessSchedule_ControlledContextOnlyMatrix_0100
 * @tc.name: context-only changes do not emit events, later events use latest context and primary
 * @tc.desc: same-level task/tool changes update snapshots without producing duplicate events.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_ControlledContextOnlyMatrix_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    proxy.SetDebounceWindowMsForTest(20);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    AddAttribution(sched, TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::CLI);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_HIGH);
    EXPECT_EQ(proxy.GetLastNotifiedPrimaryTool(TEST_PID_A), AttributionTool::CLI);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetLastNotifiedPrimaryTool(TEST_PID_A), AttributionTool::CLI);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 1u);
    ChangeTaskLevel(sched, TEST_TASK_HIGH, TASK_PRIORITY_MEDIUM);
    WaitForDebounce(sched);
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_MEDIUM);
    EXPECT_EQ(proxy.GetLastNotifiedPrimaryTool(TEST_PID_A), AttributionTool::VIRTUAL_SCREEN_ABILITY);
    auto latest = LastEvent(TEST_PID_A);
    const auto *task = FindCurrentTask(latest, TEST_TASK_HIGH);
    ASSERT_NE(task, nullptr);
    EXPECT_EQ(task->tools, (std::set<AttributionTool>{AttributionTool::CLI,
        AttributionTool::VIRTUAL_SCREEN_ABILITY}));
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 2u);
}

/**
 * @tc.number: ProcessSchedule_ImmediateStartAndDeathEnd_0100
 * @tc.name: immediate start/death end and pending cancellation
 * @tc.desc: a pending downgrade does not suppress a later upgrade or re-attribution.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_ImmediateStartAndDeathEnd_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    proxy.SetDebounceWindowMsForTest(20);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    sched.WaitForIdle();
    ChangeTaskLevel(sched, TEST_TASK_HIGH, TASK_PRIORITY_LOW);
    ChangeTaskLevel(sched, TEST_TASK_HIGH, TASK_PRIORITY_MEDIUM);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_MEDIUM);
    EXPECT_EQ(proxy.GetPendingCount(TEST_PID_A), 0u);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 2u);
    ProcessDeath(sched, TEST_PID_A);
    WaitForDebounce(sched);
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), 0);
    EXPECT_EQ(proxy.GetPendingCount(TEST_PID_A), 0u);
    auto final = LastEvent(TEST_PID_A);
    EXPECT_EQ(final.action, ScheduleAction::END);
    EXPECT_EQ(final.trigger, ScheduleTrigger::MANAGED_PROCESS_DIED);
    EXPECT_FALSE(final.debounceExpired);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 3u);
}

/**
 * @tc.number: ProcessSchedule_DebounceDownAndEnd_0100
 * @tc.name: downgrade and final removal are emitted after the fixed window
 * @tc.desc: fixed 1000ms trailing debounce emits only the latest state and final END.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_DebounceDownAndEnd_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    EXPECT_EQ(proxy.GetDebounceWindowMsForTest(), 1000);
    proxy.SetDebounceWindowMsForTest(20);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    sched.WaitForIdle();
    ChangeTaskLevel(sched, TEST_TASK_HIGH, TASK_PRIORITY_MEDIUM);
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_HIGH);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 1u);
    WaitForDebounce(sched);
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_MEDIUM);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 2u);
    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_A, AttributionTool::CLI);
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_MEDIUM);
    WaitForDebounce(sched);
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), 0);
    EXPECT_EQ(proxy.GetLastNotifiedPrimaryTool(TEST_PID_A), AttributionTool::UNKNOWN);
    EXPECT_EQ(proxy.GetPendingCount(TEST_PID_A), 0u);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 3u);
}

/**
 * @tc.number: ProcessSchedule_PendingDownThenEndUsesLatestPrevious_0100
 * @tc.name: pending downgrade followed by final removal keeps the latest previous context
 * @tc.desc: no intermediate START is emitted and END contains only the final remaining task.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_PendingDownThenEndUsesLatestPrevious_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    proxy.SetDebounceWindowMsForTest(20);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A,
        AttributionTool::VIRTUAL_SCREEN_ABILITY);
    AddAttribution(sched, TEST_TASK_MEDIUM, TASK_PRIORITY_MEDIUM, TEST_PID_A, AttributionTool::CLI);
    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_A, AttributionTool::VIRTUAL_SCREEN_ABILITY);
    RemoveAttribution(sched, TEST_TASK_MEDIUM, TEST_PID_A, AttributionTool::CLI);
    WaitForDebounce(sched);
    auto final = LastEvent(TEST_PID_A);
    EXPECT_EQ(final.action, ScheduleAction::END);
    EXPECT_TRUE(final.debounceExpired);
    EXPECT_EQ(final.previousEffectiveLevel, TASK_PRIORITY_MEDIUM);
    EXPECT_EQ(final.effectiveLevel, 0);
    EXPECT_EQ(final.previousPrimaryTool, AttributionTool::CLI);
    ASSERT_EQ(final.previousTasks.size(), 1u);
    EXPECT_NE(FindPreviousTask(final, TEST_TASK_MEDIUM), nullptr);
    EXPECT_EQ(FindPreviousTask(final, TEST_TASK_HIGH), nullptr);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 2u);
}

/**
 * @tc.number: ProcessSchedule_DebounceLatestState_0100
 * @tc.name: debounce emits only latest state and re-attribution cancels pending END
 * @tc.desc: continuous downgrade, upgrade interruption, final removal, re-attribution and death.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_DebounceLatestState_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    proxy.SetDebounceWindowMsForTest(20);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    ChangeTaskLevel(sched, TEST_TASK_HIGH, TASK_PRIORITY_MEDIUM);
    ChangeTaskLevel(sched, TEST_TASK_HIGH, TASK_PRIORITY_LOW);
    ChangeTaskLevel(sched, TEST_TASK_HIGH, TASK_PRIORITY_MEDIUM);
    WaitForDebounce(sched);
    auto events = proxy.GetEventHistoryForTest(TEST_PID_A);
    ASSERT_EQ(events.size(), 2u);
    EXPECT_EQ(events.back().effectiveLevel, TASK_PRIORITY_MEDIUM);
    EXPECT_EQ(proxy.GetPendingCount(TEST_PID_A), 0u);

    ChangeTaskLevel(sched, TEST_TASK_HIGH, TASK_PRIORITY_LOW);
    WaitForDebounce(sched);
    events = proxy.GetEventHistoryForTest(TEST_PID_A);
    ASSERT_EQ(events.size(), 3u);
    EXPECT_EQ(events.back().action, ScheduleAction::START);
    EXPECT_EQ(events.back().effectiveLevel, TASK_PRIORITY_LOW);

    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_A, AttributionTool::CLI);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::CLI);
    WaitForDebounce(sched);
    events = proxy.GetEventHistoryForTest(TEST_PID_A);
    ASSERT_EQ(events.size(), 4u);
    EXPECT_EQ(events.back().action, ScheduleAction::START);
    EXPECT_EQ(events.back().effectiveLevel, TASK_PRIORITY_LOW);
    EXPECT_FALSE(events.back().currentTasks.empty());

    ProcessDeath(sched, TEST_PID_A);
    WaitForDebounce(sched);
    events = proxy.GetEventHistoryForTest(TEST_PID_A);
    ASSERT_EQ(events.size(), 5u);
    EXPECT_EQ(events.back().action, ScheduleAction::END);
    EXPECT_EQ(events.back().trigger, ScheduleTrigger::MANAGED_PROCESS_DIED);
}

/**
 * @tc.number: ProcessSchedule_FinalStateAction_0100
 * @tc.name: final controlled state selects downgrade START or final END exactly once
 * @tc.desc: finish, timeout, creator death, unregister and managed death do not affect another pid.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_FinalStateAction_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    proxy.SetDebounceWindowMsForTest(20);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A,
        AttributionTool::VIRTUAL_SCREEN_ABILITY);
    AddAttribution(sched, TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::CLI);
    auto high = MakeInfo(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A,
        AttributionTool::VIRTUAL_SCREEN_ABILITY);
    sched.OnAttributionRemoved(high, ScheduleTrigger::TASK_FINISHED);
    WaitForDebounce(sched);
    EXPECT_EQ(LastEvent(TEST_PID_A).action, ScheduleAction::START);
    EXPECT_EQ(LastEvent(TEST_PID_A).effectiveLevel, TASK_PRIORITY_LOW);

    auto low = MakeInfo(TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::CLI);
    sched.OnAttributionRemoved(low, ScheduleTrigger::TASK_TIMEOUT);
    WaitForDebounce(sched);
    EXPECT_EQ(LastEvent(TEST_PID_A).action, ScheduleAction::END);
    EXPECT_EQ(LastEvent(TEST_PID_A).trigger, ScheduleTrigger::TASK_TIMEOUT);

    sched.OnAttributionAdded(high);
    sched.OnAttributionAdded(low);
    sched.WaitForIdle();
    sched.OnAttributionRemoved(high, ScheduleTrigger::CREATOR_DIED);
    WaitForDebounce(sched);
    EXPECT_EQ(LastEvent(TEST_PID_A).action, ScheduleAction::START);
    EXPECT_EQ(LastEvent(TEST_PID_A).trigger, ScheduleTrigger::CREATOR_DIED);
    sched.OnAttributionRemoved(low, ScheduleTrigger::ATTRIBUTION_REMOVED);
    WaitForDebounce(sched);
    EXPECT_EQ(LastEvent(TEST_PID_A).action, ScheduleAction::END);

    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_B, AttributionTool::CLI);
    const auto otherEvents = proxy.GetEventHistoryForTest(TEST_PID_B).size();
    ProcessDeath(sched, TEST_PID_A);
    WaitForDebounce(sched);
    EXPECT_EQ(LastEvent(TEST_PID_A).action, ScheduleAction::END);
    EXPECT_EQ(LastEvent(TEST_PID_A).trigger, ScheduleTrigger::MANAGED_PROCESS_DIED);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_B).size(), otherEvents);
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_B), TASK_PRIORITY_HIGH);
}

/**
 * @tc.number: ProcessSchedule_UidFailureRecovery_0100
 * @tc.name: uid failure is suppressed and recovered with original reason
 * @tc.desc: incomplete events are suppressed, and a successful retry restores complete events.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_UidFailureRecovery_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    proxy.SetDebounceWindowMsForTest(20);
    auto info = MakeInfo(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    info.attribution.uid = -1;
    sched.OnAttributionAdded(info);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), 0);
    info.attribution.uid = TEST_CREATOR_UID;
    sched.OnManagedUidResolved(info, ScheduleTrigger::ATTRIBUTION_ADDED);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_HIGH);
    auto endInfo = MakeInfo(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    endInfo.attribution.uid = -1;
    sched.OnAttributionRemoved(endInfo, ScheduleTrigger::TASK_FINISHED);
    WaitForDebounce(sched);
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), 0);
}

/**
 * @tc.number: ProcessSchedule_InvalidUidCancelsPending_0100
 * @tc.name: invalid uid re-attribution invalidates the previous pending lifecycle
 * @tc.desc: no stale END or incomplete START is emitted; valid retry recovers with its original reason.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_InvalidUidCancelsPending_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    proxy.SetDebounceWindowMsForTest(20);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_A, AttributionTool::CLI);
    auto invalid = MakeInfo(TEST_TASK_HIGH, TASK_PRIORITY_MEDIUM, TEST_PID_A, AttributionTool::CLI);
    invalid.attribution.uid = -1;
    sched.OnAttributionAdded(invalid);
    WaitForDebounce(sched);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 1u);
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_HIGH);
    EXPECT_EQ(proxy.GetPendingCount(TEST_PID_A), 0u);
    sched.OnProcessDied(TEST_PID_A, -1);
    WaitForDebounce(sched);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 1u);

    ResetScenario();
    proxy.SetDebounceWindowMsForTest(20);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    RemoveAttribution(sched, TEST_TASK_HIGH, TEST_PID_A, AttributionTool::CLI);
    sched.OnAttributionAdded(invalid);
    sched.WaitForIdle();
    auto valid = invalid;
    valid.attribution.uid = TEST_CREATOR_UID;
    sched.OnManagedUidResolved(valid, ScheduleTrigger::FOREGROUND_CHANGED);
    WaitForDebounce(sched);
    auto recovered = LastEvent(TEST_PID_A);
    EXPECT_EQ(recovered.action, ScheduleAction::START);
    EXPECT_EQ(recovered.trigger, ScheduleTrigger::FOREGROUND_CHANGED);
    EXPECT_EQ(recovered.effectiveLevel, TASK_PRIORITY_MEDIUM);
    EXPECT_TRUE(recovered.uidRecovered);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 2u);
}

/**
 * @tc.number: ProcessSchedule_UidFailureRecoveryMatrix_0100
 * @tc.name: uid persistent failure, recovery, death compensation and late retry matrix
 * @tc.desc: incomplete events are suppressed while valid death context and original triggers are kept.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_UidFailureRecoveryMatrix_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    auto reset = [&proxy]() {
        ResetScenario();
        proxy.SetDebounceWindowMsForTest(20);
    };
    auto invalid = MakeInfo(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    invalid.attribution.uid = -1;
    auto valid = invalid;
    valid.attribution.uid = TEST_CREATOR_UID;

    reset();
    sched.OnAttributionAdded(invalid);
    sched.WaitForIdle();
    sched.OnTaskLevelChanged(TEST_TASK_HIGH, TASK_PRIORITY_MEDIUM, ScheduleTrigger::SYSTEM_LOAD_CHANGED);
    sched.WaitForIdle();
    invalid.taskLevel = TASK_PRIORITY_MEDIUM;
    sched.OnAttributionRemoved(invalid, ScheduleTrigger::TASK_FINISHED);
    WaitForDebounce(sched);
    EXPECT_TRUE(proxy.GetEventHistoryForTest(TEST_PID_A).empty());
    invalid.taskLevel = TASK_PRIORITY_HIGH;

    reset();
    sched.OnAttributionAdded(invalid);
    sched.WaitForIdle();
    sched.OnManagedUidResolved(valid, ScheduleTrigger::FOREGROUND_CHANGED);
    sched.WaitForIdle();
    auto recovered = LastEvent(TEST_PID_A);
    EXPECT_EQ(recovered.action, ScheduleAction::START);
    EXPECT_EQ(recovered.trigger, ScheduleTrigger::FOREGROUND_CHANGED);
    EXPECT_TRUE(recovered.uidRecovered);
    sched.OnAttributionRemoved(invalid, ScheduleTrigger::TASK_FINISHED);
    WaitForDebounce(sched);
    EXPECT_EQ(LastEvent(TEST_PID_A).action, ScheduleAction::END);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 2u);

    reset();
    sched.OnAttributionAdded(invalid);
    sched.WaitForIdle();
    sched.OnProcessDied(TEST_PID_A, -1);
    WaitForDebounce(sched);
    EXPECT_TRUE(proxy.GetEventHistoryForTest(TEST_PID_A).empty());
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_.count(TEST_PID_A), 0u);
    }

    reset();
    sched.OnAttributionAdded(invalid);
    sched.WaitForIdle();
    sched.OnProcessDied(TEST_PID_A, TEST_CREATOR_UID);
    sched.WaitForIdle();
    auto death = LastEvent(TEST_PID_A);
    EXPECT_EQ(death.action, ScheduleAction::END);
    EXPECT_EQ(death.managedUid, TEST_CREATOR_UID);
    ASSERT_EQ(death.previousTasks.size(), 1u);
    sched.OnManagedUidResolved(valid, ScheduleTrigger::ATTRIBUTION_ADDED);
    WaitForDebounce(sched);
    EXPECT_EQ(proxy.GetEventHistoryForTest(TEST_PID_A).size(), 1u);
}

/**
 * @tc.number: ProcessSchedule_UidRecoveryBySecondAttribution_0100
 * @tc.name: a later complete attribution recovers the pid exactly once
 * @tc.desc: same-level uid recovery emits START immediately and duplicate resolution is a no-op.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_UidRecoveryBySecondAttribution_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    auto first = MakeInfo(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    first.attribution.uid = -1;
    sched.OnAttributionAdded(first);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetEventCountForTest(TEST_PID_A), 0u);

    auto second = MakeInfo(TEST_TASK_MEDIUM, TASK_PRIORITY_HIGH, TEST_PID_A,
        AttributionTool::VIRTUAL_SCREEN_ABILITY);
    sched.OnAttributionAdded(second);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_HIGH);
    EXPECT_EQ(proxy.GetLastNotifiedPrimaryTool(TEST_PID_A), AttributionTool::VIRTUAL_SCREEN_ABILITY);
    EXPECT_EQ(proxy.GetEventCountForTest(TEST_PID_A), 1u);

    sched.OnManagedUidResolved(second, ScheduleTrigger::ATTRIBUTION_ADDED);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetEventCountForTest(TEST_PID_A), 1u);
}

/**
 * @tc.number: ProcessSchedule_SelfManagedDeath_0100
 * @tc.name: self-managed death is immediate and re-attribution starts a new lifecycle
 * @tc.desc: creator and managed process are the same pid; death has exactly one final END.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_SelfManagedDeath_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_CREATOR_PID, AttributionTool::CLI);
    sched.WaitForIdle();
    ProcessDeath(sched, TEST_CREATOR_PID);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_CREATOR_PID), 0);
    EXPECT_EQ(proxy.GetPendingCount(TEST_CREATOR_PID), 0u);
    EXPECT_EQ(proxy.GetEventCountForTest(TEST_CREATOR_PID), 2u);
    AddAttribution(sched, TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_CREATOR_PID, AttributionTool::CLI);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_CREATOR_PID), TASK_PRIORITY_LOW);
    EXPECT_EQ(proxy.GetEventCountForTest(TEST_CREATOR_PID), 3u);
}

/**
 * @tc.number: ProcessSchedule_TimerFailureRecovery_0100
 * @tc.name: timer creation failure retains pending and a later candidate recovers
 * @tc.desc: failed timer creation never produces a fake immediate event.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_TimerFailureRecovery_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    proxy.SetDebounceWindowMsForTest(20);
    AddAttribution(sched, TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    sched.WaitForIdle();
    proxy.FailNextTimerForTest();
    ChangeTaskLevel(sched, TEST_TASK_HIGH, TASK_PRIORITY_LOW);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_HIGH);
    EXPECT_EQ(proxy.GetPendingCount(TEST_PID_A), 1u);
    ChangeTaskLevel(sched, TEST_TASK_HIGH, TASK_PRIORITY_MEDIUM);
    sched.WaitForIdle();
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_MEDIUM);
    EXPECT_EQ(proxy.GetPendingCount(TEST_PID_A), 0u);
}

}  // namespace AppExecFwk
}  // namespace OHOS
