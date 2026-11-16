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

#include "task_mgr/attribution_record.h"
#include "task_mgr/common.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr pid_t TEST_PID_A = 4312;
constexpr pid_t TEST_PID_B = 4401;
constexpr int32_t TEST_PID_C = 4500;
const std::string TEST_TASK_HIGH = "sched_t1_high";
const std::string TEST_TASK_MEDIUM = "sched_t2_medium";
const std::string TEST_TASK_LOW = "sched_t3_low";

void ResetSchedule()
{
    auto &sched = ProcessSchedule::GetInstance();
    std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
    sched.taskToPids_.clear();
    sched.pidEntries_.clear();
    auto &proxy = ResourceControlProxy::GetInstance();
    std::lock_guard<std::mutex> proxyLock(proxy.registryMutex_);
    proxy.controlRegistry_.clear();
    proxy.failQueryForTest_ = false;
    proxy.failRevokeForTest_ = false;
    proxy.failNotifyForTest_ = false;
    proxy.revokeCallsForTest_.store(0);
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
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnAttributionAdded(TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::CLI);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_HIGH);
    }
    std::string dump = sched.DumpSchedule();
    EXPECT_NE(dump.find("effectiveLevel [HIGH(300)]"), std::string::npos);
    sched.OnAttributionAdded(TEST_TASK_MEDIUM, TASK_PRIORITY_MEDIUM, TEST_PID_A, AttributionTool::A2A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_HIGH);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskLevels.size(), 3);
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
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnAttributionAdded(TEST_TASK_MEDIUM, TASK_PRIORITY_MEDIUM, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnAttributionAdded(TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::ABILITY);

    sched.OnAttributionRemoved(TEST_TASK_HIGH, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_MEDIUM);
    }
    sched.OnAttributionRemoved(TEST_TASK_MEDIUM, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_LOW);
    }
    sched.OnAttributionRemoved(TEST_TASK_LOW, TEST_PID_A);
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
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnAttributionAdded(TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::CLI);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.taskToPids_[TEST_TASK_HIGH].count(TEST_PID_A), 1);
        EXPECT_EQ(sched.taskToPids_[TEST_TASK_LOW].count(TEST_PID_A), 1);
    }
    sched.OnAttributionRemoved(TEST_TASK_HIGH, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.taskToPids_.count(TEST_TASK_HIGH), 0);
        EXPECT_EQ(sched.taskToPids_[TEST_TASK_LOW].count(TEST_PID_A), 1);
    }
    // duplicate removal of the same attribution is a safe no-op (idempotent channel)
    sched.OnAttributionRemoved(TEST_TASK_HIGH, TEST_PID_A);
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
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_B, AttributionTool::CLI);
    sched.OnAttributionAdded(TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_B, AttributionTool::INSIGHT);

    sched.OnProcessDied(TEST_PID_B);
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
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_B, AttributionTool::ABILITY);
    sched.OnProcessDied(TEST_PID_A);
    sched.OnProcessDied(TEST_PID_B);
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
            sched.OnAttributionAdded(taskId, TASK_PRIORITY_MEDIUM, pid, AttributionTool::ABILITY);
            sched.OnAttributionRemoved(taskId, pid);
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
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, 0, AttributionTool::ABILITY);
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, -1, AttributionTool::CLI);
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
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskRefs[TEST_TASK_HIGH], 2);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskLevels.size(), 1);
        EXPECT_EQ(sched.taskToPids_[TEST_TASK_HIGH].size(), 1);
    }
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
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    sched.OnAttributionRemoved(TEST_TASK_HIGH, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        ASSERT_EQ(sched.pidEntries_.count(TEST_PID_A), 1);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_HIGH);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskRefs[TEST_TASK_HIGH], 1);
    }
    sched.OnAttributionRemoved(TEST_TASK_HIGH, TEST_PID_A);
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
    sched.OnProcessDied(0);
    sched.OnProcessDied(-1);
    sched.OnProcessDied(TEST_PID_B);
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnProcessDied(TEST_PID_B);
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
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnAttributionRemoved(TEST_TASK_HIGH, 0);
    sched.OnAttributionRemoved(TEST_TASK_HIGH, -1);
    sched.OnAttributionRemoved(TEST_TASK_HIGH, TEST_PID_B);
    sched.OnAttributionRemoved(std::string("no_such_task"), TEST_PID_A);
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
    sched.OnTaskLevelChanged(std::string("ghost_task"), TASK_PRIORITY_HIGH);
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
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnAttributionAdded(std::string("sched_t1b_high"), TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    sched.OnAttributionRemoved(TEST_TASK_HIGH, TEST_PID_A);
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

    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    std::string dump = sched.DumpSchedule();
    EXPECT_NE(dump.find("tasks=1 pids=1"), std::string::npos);
    EXPECT_NE(dump.find("task #" + TEST_TASK_HIGH + " -> pids [" + std::to_string(TEST_PID_A) + "]"),
        std::string::npos);
    EXPECT_NE(dump.find("pid " + std::to_string(TEST_PID_A) + " -> tasks [" + TEST_TASK_HIGH + "]"),
        std::string::npos);
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
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnAttributionAdded(TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnTaskLevelChanged(TEST_TASK_LOW, TASK_PRIORITY_HIGH);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_HIGH);
    }
    sched.OnTaskLevelChanged(TEST_TASK_HIGH, TASK_PRIORITY_MEDIUM);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_HIGH);
    }
    sched.OnTaskLevelChanged(TEST_TASK_LOW, TASK_PRIORITY_MEDIUM);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_MEDIUM);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS

namespace OHOS {
namespace AppExecFwk {
/**
 * @tc.number: AttributionPolymorphic_0100
 * @tc.name: factory creates the derived record per tool; dispatch is polymorphic (ADR-1).
 * @tc.desc: each tool maps to its derived class tagged via the tool field (no RTTI in build);
 *           UNKNOWN yields the base record; base-pointer dispatch invokes Schedule/Unschedule
 *           without switching on the tool type.
 */
HWTEST_F(ProcessScheduleTest, AttributionPolymorphic_0100, TestSize.Level1)
{
    auto insight = CreateAttributionRecord(AttributionTool::INSIGHT);
    auto cli = CreateAttributionRecord(AttributionTool::CLI);
    auto a2a = CreateAttributionRecord(AttributionTool::A2A);
    auto ability = CreateAttributionRecord(AttributionTool::ABILITY);
    auto unknown = CreateAttributionRecord(AttributionTool::UNKNOWN);
    ASSERT_NE(insight, nullptr);
    ASSERT_NE(cli, nullptr);
    ASSERT_NE(a2a, nullptr);
    ASSERT_NE(ability, nullptr);
    ASSERT_NE(unknown, nullptr);
    EXPECT_EQ(insight->tool, AttributionTool::INSIGHT);
    EXPECT_EQ(cli->tool, AttributionTool::CLI);
    EXPECT_EQ(a2a->tool, AttributionTool::A2A);
    EXPECT_EQ(ability->tool, AttributionTool::ABILITY);
    EXPECT_EQ(unknown->tool, AttributionTool::UNKNOWN);
    // polymorphic dispatch via the base pointer (phase-1 empty impl + logging, must not crash)
    std::shared_ptr<AttributionRecord> base = cli;
    base->pid = TEST_PID_A;
    base->Schedule(TEST_TASK_HIGH, TASK_PRIORITY_HIGH);
    base->Unschedule(TEST_TASK_HIGH);
}

/**
 * @tc.number: ProxyStubAction_0100
 * @tc.name: ResourceControlProxy Stub registers, queries, revokes and cleans (ADR-2).
 * @tc.desc: ApplyControl registers the level; QueryControlState reads it back; RevokeControl
 *           and NotifyTaskEnd remove the entry so the query then fails.
 */
HWTEST_F(ProcessScheduleTest, ProxyStubAction_0100, TestSize.Level1)
{
    auto &proxy = ResourceControlProxy::GetInstance();
    ControlRequest req;
    req.taskId = TEST_TASK_HIGH;
    req.managedPid = TEST_PID_A;
    req.managedType = AttributionTool::CLI;
    req.level = TASK_PRIORITY_HIGH;
    req.action = ControlAction::APPLY;
    EXPECT_EQ(proxy.ApplyControl(req), 0);
    int32_t level = 0;
    EXPECT_EQ(proxy.QueryControlState(TEST_TASK_HIGH, TEST_PID_A, level), 0);
    EXPECT_EQ(level, TASK_PRIORITY_HIGH);
    EXPECT_EQ(proxy.RevokeControl(TEST_TASK_HIGH, TEST_PID_A), 0);
    EXPECT_NE(proxy.QueryControlState(TEST_TASK_HIGH, TEST_PID_A, level), 0);
    // task-level cleanup via NotifyTaskEnd
    EXPECT_EQ(proxy.ApplyControl(req), 0);
    TaskEndInfo info;
    info.taskId = TEST_TASK_HIGH;
    info.pids = {TEST_PID_A};
    proxy.NotifyTaskEnd(info);
    EXPECT_NE(proxy.QueryControlState(TEST_TASK_HIGH, TEST_PID_A, level), 0);
    // invalid pid rejected
    ControlRequest badReq;
    badReq.managedPid = 0;
    EXPECT_NE(proxy.ApplyControl(badReq), 0);
}


/**
 * @tc.number: DispatchPolymorphic_0100
 * @tc.name: attribution add/remove dispatches Schedule/Unschedule and registers the proxy.
 * @tc.desc: with a ready mapping, adding an attribution creates the derived object and
 *           registers the effective level at the proxy; removing it revokes the registration.
 */
HWTEST_F(ProcessScheduleTest, DispatchPolymorphic_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        ASSERT_EQ(sched.pidEntries_[TEST_PID_A].attrObjects.count(TEST_TASK_HIGH), 1);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].attrObjects[TEST_TASK_HIGH]->tool, AttributionTool::CLI);
    }
    int32_t level = 0;
    EXPECT_EQ(proxy.QueryControlState(TEST_TASK_HIGH, TEST_PID_A, level), 0);
    EXPECT_EQ(level, TASK_PRIORITY_HIGH);
    sched.OnAttributionRemoved(TEST_TASK_HIGH, TEST_PID_A);
    EXPECT_NE(proxy.QueryControlState(TEST_TASK_HIGH, TEST_PID_A, level), 0);
}

/**
 * @tc.number: NoTaskIdBaseline_0100
 * @tc.name: no taskId yields no control event; a managed pid gets the hard-coded control (INV-6).
 * @tc.desc: without any attribution the proxy holds no registration (no taskId, no-op); once an
 *           attribution is added, the derived record applies its hard-coded schedule control.
 */
HWTEST_F(ProcessScheduleTest, NoTaskIdBaseline_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    int32_t level = 0;
    // no taskId (no attribution): no control event, no registration at the proxy
    EXPECT_NE(proxy.QueryControlState(TEST_TASK_HIGH, TEST_PID_A, level), 0);
    // managed pid: the derived record applies the hard-coded schedule control
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    EXPECT_EQ(proxy.QueryControlState(TEST_TASK_HIGH, TEST_PID_A, level), 0);
    EXPECT_EQ(level, TASK_PRIORITY_HIGH);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_HIGH);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].attrObjects.count(TEST_TASK_HIGH), 1);
    }
}

/**
 * @tc.number: ControlApplyVerify_0100
 * @tc.name: the effectiveness check confirms the applied level; a failed query means an
 *           ineffective delivery (INV-3).
 * @tc.desc: after applying, QueryControlState returns the effective level; injecting a query
 *           failure flips the check to the ineffective branch.
 */
HWTEST_F(ProcessScheduleTest, ControlApplyVerify_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    int32_t level = 0;
    // effectiveness confirmed: the applied level is queryable
    EXPECT_EQ(proxy.QueryControlState(TEST_TASK_HIGH, TEST_PID_A, level), 0);
    EXPECT_EQ(level, TASK_PRIORITY_HIGH);
    // inject an ineffective delivery: the query now fails
    proxy.failQueryForTest_ = true;
    EXPECT_NE(proxy.QueryControlState(TEST_TASK_HIGH, TEST_PID_A, level), 0);
    proxy.failQueryForTest_ = false;
}

/**
 * @tc.number: DegradeRetry_0100
 * @tc.name: a failing revoke is retried within the budget (1 + 3) then alerts (INV-4).
 * @tc.desc: with revoke fault injection on, removing the attribution drives RevokeControl
 *           four times (initial + 3 retries) while the registry entry survives (failed revoke).
 */
HWTEST_F(ProcessScheduleTest, DegradeRetry_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    proxy.revokeCallsForTest_.store(0);
    proxy.failRevokeForTest_ = true;
    sched.OnAttributionRemoved(TEST_TASK_HIGH, TEST_PID_A);
    // initial attempt + 3 retries
    EXPECT_EQ(proxy.revokeCallsForTest_.load(), 4);
    // the index is cleaned even though the revoke failed
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_TRUE(sched.pidEntries_.empty());
    }
    proxy.failRevokeForTest_ = false;
}

/**
 * @tc.number: TaskEndDegrade_0100
 * @tc.name: dual-channel degrade: process-level revoke plus task-level NotifyTaskEnd (ADR-5).
 * @tc.desc: after removing one attribution (process-level) and ending the task (task-level),
 *           every managed pid registration is cleaned from the proxy.
 */
HWTEST_F(ProcessScheduleTest, TaskEndDegrade_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_B, AttributionTool::ABILITY);
    // process-level channel: revoke one attribution
    sched.OnAttributionRemoved(TEST_TASK_HIGH, TEST_PID_A);
    // task-level channel: the task ends and its level policy is cleaned
    sched.OnTaskEnd(TEST_TASK_HIGH, {TEST_PID_A, TEST_PID_B});
    int32_t level = 0;
    EXPECT_NE(proxy.QueryControlState(TEST_TASK_HIGH, TEST_PID_A, level), 0);
    EXPECT_NE(proxy.QueryControlState(TEST_TASK_HIGH, TEST_PID_B, level), 0);
}

/**
 * @tc.number: DispatchConcurrent_0100
 * @tc.name: out-of-lock dispatch stays consistent under concurrent add/remove (INV-2).
 * @tc.desc: ten threads each add then remove one task-pid pair with a ready mapping; the
 *           index and the proxy registry both end up empty.
 */
HWTEST_F(ProcessScheduleTest, DispatchConcurrent_0100, TestSize.Level3)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    std::vector<std::thread> threads;
    for (int32_t i = 0; i < 10; i++) {
        threads.emplace_back([&sched, i]() {
            std::string taskId = "sched_dispatch_" + std::to_string(i);
            pid_t pid = TEST_PID_C + i;
            sched.OnAttributionAdded(taskId, TASK_PRIORITY_MEDIUM, pid, AttributionTool::CLI);
            sched.OnAttributionRemoved(taskId, pid);
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
    int32_t level = 0;
    EXPECT_NE(proxy.QueryControlState("sched_dispatch_0", TEST_PID_C, level), 0);
}

/**
 * @tc.number: UnscheduleRemovedTask_0100
 * @tc.name: removing one task unschedules only its object and degrades the pid (UNSCHEDULE).
 * @tc.desc: two tasks on one pid; removing the HIGH task drops only its scheduling object,
 *           the LOW task keeps its object and the pid degrades to LOW at the proxy.
 */
HWTEST_F(ProcessScheduleTest, UnscheduleRemovedTask_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    sched.OnAttributionAdded(TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnAttributionRemoved(TEST_TASK_HIGH, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        // the removed task's object is gone, the remaining task's object stays
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].attrObjects.count(TEST_TASK_HIGH), 0);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].attrObjects.count(TEST_TASK_LOW), 1);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].level, TASK_PRIORITY_LOW);
    }
    // the pid degrades to the remaining LOW task at the proxy
    int32_t level = 0;
    EXPECT_EQ(proxy.QueryControlState(TEST_TASK_LOW, TEST_PID_A, level), 0);
    EXPECT_EQ(level, TASK_PRIORITY_LOW);
}
}  // namespace AppExecFwk
}  // namespace OHOS
