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
constexpr pid_t TEST_CREATOR_PID = 3268;
constexpr int32_t TEST_CREATOR_UID = 20010001;

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

void ResetSchedule()
{
    ProcessSchedule::GetInstance().ResetForTest();
}

class ProcessScheduleTest : public testing::Test {
protected:
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
 * @tc.number: ResourceControlProxy_StubOnly_0100
 * @tc.name: EmitEvent only logs and updates notification state without recursion
 * @tc.desc: direct emission does not call Apply/Restore or any real resource-schedule interface.
 */
HWTEST_F(ProcessScheduleTest, ResourceControlProxy_StubOnly_0100, TestSize.Level1)
{
    auto &sched = ProcessSchedule::GetInstance();
    auto &proxy = ResourceControlProxy::GetInstance();
    auto info = MakeInfo("stub_only", TASK_PRIORITY_HIGH, TEST_PID_A, AttributionTool::CLI);
    sched.OnAttributionAdded(info);
    sched.WaitForIdle();
    auto snapshot = sched.BuildSnapshot(TEST_PID_A, sched.pidEntries_[TEST_PID_A]);
    ProcessScheduleEvent event;
    event.action = ScheduleAction::START;
    event.trigger = ScheduleTrigger::ATTRIBUTION_ADDED;
    event.managedPid = TEST_PID_A;
    event.managedUid = TEST_CREATOR_UID;
    event.previousEffectiveLevel = 0;
    event.effectiveLevel = snapshot.effectiveLevel;
    event.previousPrimaryTool = AttributionTool::UNKNOWN;
    event.currentPrimaryTool = snapshot.primaryTool;
    event.previousTasks = {};
    event.currentTasks = snapshot.tasks;
    proxy.SetEventRecordingForTest(true);
    proxy.EmitEvent(event);
    EXPECT_EQ(proxy.GetLastNotifiedLevel(TEST_PID_A), TASK_PRIORITY_HIGH);
    EXPECT_EQ(proxy.GetLastNotifiedPrimaryTool(TEST_PID_A), AttributionTool::CLI);
    EXPECT_EQ(proxy.GetPendingCount(TEST_PID_A), 0u);
    auto events = proxy.GetEventHistoryForTest(TEST_PID_A);
    ASSERT_EQ(events.size(), 1u);
    EXPECT_EQ(events.back().managedUid, TEST_CREATOR_UID);
    ASSERT_EQ(events.back().currentTasks.size(), 1u);
    EXPECT_EQ(events.back().currentTasks.back().tools,
        std::set<AttributionTool>{AttributionTool::CLI});
}
}  // namespace
}  // namespace AppExecFwk
}  // namespace OHOS
