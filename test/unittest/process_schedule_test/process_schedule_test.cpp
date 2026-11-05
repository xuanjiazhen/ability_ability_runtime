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
#include "process_schedule/process_schedule.h"
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
const std::string TEST_TASK_HIGH = "sched_t1_high";
const std::string TEST_TASK_MEDIUM = "sched_t2_medium";
const std::string TEST_TASK_LOW = "sched_t3_low";

void ResetSchedule()
{
    auto &sched = ProcessSchedule::GetInstance();
    std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
    sched.taskToPids_.clear();
    sched.pidEntries_.clear();
    sched.exemptions_.clear();
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
 * @tc.number: ProcessSchedule_ExemptionIdempotent_0100
 * @tc.name: exemptions are keyed by taskId+pid and reference counted (INV-3)
 * @tc.desc: three tasks on one pid hold three exemption keys; each removal drops only its
 *           own key; the pid entry release clears its exemptions without leaking.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_ExemptionIdempotent_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnAttributionAdded(TEST_TASK_LOW, TASK_PRIORITY_LOW, TEST_PID_A, AttributionTool::CLI);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.exemptions_[TEST_TASK_HIGH][TEST_PID_A], 1);
        EXPECT_EQ(sched.exemptions_[TEST_TASK_LOW][TEST_PID_A], 1);
    }
    std::string dump = sched.DumpSchedule();
    EXPECT_NE(dump.find("Exemption summary: total=2"), std::string::npos);

    sched.OnAttributionRemoved(TEST_TASK_HIGH, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.exemptions_.count(TEST_TASK_HIGH), 0);
        EXPECT_EQ(sched.exemptions_[TEST_TASK_LOW][TEST_PID_A], 1);
    }
    // duplicate removal of the same attribution is a safe no-op (idempotent channel)
    sched.OnAttributionRemoved(TEST_TASK_HIGH, TEST_PID_A);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.exemptions_.count(TEST_TASK_HIGH), 0);
        EXPECT_EQ(sched.exemptions_[TEST_TASK_LOW][TEST_PID_A], 1);
    }
}

/**
 * @tc.number: ProcessSchedule_ProcessDeathCleanup_0100
 * @tc.name: a died process is removed with its exemptions, other processes untouched (AC-6.3)
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
        EXPECT_EQ(sched.exemptions_.count(TEST_TASK_HIGH), 1);
        EXPECT_EQ(sched.exemptions_[TEST_TASK_HIGH].count(TEST_PID_B), 0);
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
        EXPECT_TRUE(sched.exemptions_.empty());
    }
}

/**
 * @tc.number: ProcessSchedule_Concurrent10Tasks_0100
 * @tc.name: ten concurrent tasks add/remove attributions without corrupting state (AC-6.4)
 * @tc.desc: ten threads each register and then remove one task-pid pair; afterwards the
 *           index, the entries and the exemptions must all be empty.
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
        EXPECT_TRUE(sched.exemptions_.empty());
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
        EXPECT_TRUE(sched.exemptions_.empty());
    }
}

/**
 * @tc.number: ProcessSchedule_OnAttributionAdded_0200
 * @tc.name: a duplicated attribution is tracked once (already-tracked branch).
 * @tc.desc: adding the same taskId+pid twice must not duplicate the task level nor the
 *           exemption count; the pid keeps a single entry.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_OnAttributionAdded_0200, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    {
        std::lock_guard<std::mutex> lock(sched.scheduleMutex_);
        EXPECT_EQ(sched.pidEntries_[TEST_PID_A].taskLevels.size(), 1);
        EXPECT_EQ(sched.taskToPids_[TEST_TASK_HIGH].size(), 1);
        EXPECT_EQ(sched.exemptions_[TEST_TASK_HIGH][TEST_PID_A], 1);
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
        EXPECT_EQ(sched.exemptions_[TEST_TASK_HIGH][TEST_PID_A], 1);
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
 * @tc.desc: empty state prints zero totals; populated state prints the task/pid index lines
 *           and the exemption key lines.
 */
HWTEST_F(ProcessScheduleTest, ProcessSchedule_DumpSchedule_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    std::string emptyDump = sched.DumpSchedule();
    EXPECT_NE(emptyDump.find("ProcessSchedule index: tasks=0 pids=0"), std::string::npos);
    EXPECT_NE(emptyDump.find("Exemption summary: total=0"), std::string::npos);

    sched.OnAttributionAdded(TEST_TASK_HIGH, TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::ABILITY);
    std::string dump = sched.DumpSchedule();
    EXPECT_NE(dump.find("tasks=1 pids=1"), std::string::npos);
    EXPECT_NE(dump.find("task #" + TEST_TASK_HIGH + " -> pids [" + std::to_string(TEST_PID_A) + "]"),
        std::string::npos);
    EXPECT_NE(dump.find("pid " + std::to_string(TEST_PID_A) + " -> tasks [" + TEST_TASK_HIGH + "]"),
        std::string::npos);
    EXPECT_NE(dump.find("Exemption summary: total=1"), std::string::npos);
    EXPECT_NE(dump.find("keys:[" + TEST_TASK_HIGH + "+" + std::to_string(TEST_PID_A) + "+CPU]"),
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
