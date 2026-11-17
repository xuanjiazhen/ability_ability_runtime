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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_PROCESS_SCHEDULE_H
#define OHOS_APP_EXECFWK_TASK_MGR_PROCESS_SCHEDULE_H

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "task_handler_wrap.h"
#include "task_mgr/attribution_record.h"

namespace OHOS {
namespace AppExecFwk {
enum class ScheduleAction : int32_t {
    START = 0,
    END = 1,
};

enum class ScheduleTrigger : int32_t {
    ATTRIBUTION_ADDED = 0,
    ATTRIBUTION_REMOVED = 1,
    TASK_FINISHED = 2,
    TASK_TIMEOUT = 3,
    CREATOR_DIED = 4,
    MANAGED_PROCESS_DIED = 5,
    FOREGROUND_CHANGED = 6,
    SYSTEM_LOAD_CHANGED = 7,
};

struct ScheduleAttributionInfo {
    std::string taskId;
    int32_t taskLevel = 0;
    pid_t creatorPid = -1;
    int32_t creatorUid = -1;
    AttributionRecord attribution;
};

struct TaskScheduleContext {
    std::string taskId;
    pid_t creatorPid = -1;
    int32_t creatorUid = -1;
    int32_t taskLevel = 0;
    std::set<AttributionTool> tools;
};

struct ProcessScheduleEvent {
    uint64_t epoch = 0;
    ScheduleAction action = ScheduleAction::START;
    ScheduleTrigger trigger = ScheduleTrigger::ATTRIBUTION_ADDED;
    bool debounceExpired = false;
    bool uidRecovered = false;
    pid_t managedPid = -1;
    int32_t managedUid = -1;
    int32_t previousEffectiveLevel = 0;
    int32_t effectiveLevel = 0;
    AttributionTool previousPrimaryTool = AttributionTool::UNKNOWN;
    AttributionTool currentPrimaryTool = AttributionTool::UNKNOWN;
    std::vector<TaskScheduleContext> previousTasks;
    std::vector<TaskScheduleContext> currentTasks;
};

using EventProvider = std::function<std::optional<ProcessScheduleEvent>(
    const ProcessScheduleEvent &windowStart, ScheduleTrigger latestTrigger)>;

class ProcessSchedule {
public:
    static ProcessSchedule &GetInstance();

    // Only CLI and VIRTUAL_SCREEN_ABILITY participate in scheduling decisions.
    static bool IsControlledTool(AttributionTool tool);

    // All entry methods only submit immutable values to the single schedule queue.
    void OnAttributionAdded(const ScheduleAttributionInfo &info);
    void OnAttributionRemoved(const ScheduleAttributionInfo &info, ScheduleTrigger trigger);
    void OnTaskLevelChanged(const std::string &taskId, int32_t newLevel, ScheduleTrigger trigger);
    void OnManagedUidResolved(const ScheduleAttributionInfo &info, ScheduleTrigger trigger);
    void OnProcessDied(pid_t pid, int32_t uid);

    // Called by ResourceControlProxy on the same queue when a debounce window expires.
    std::optional<ProcessScheduleEvent> BuildLatestEvent(
        const ProcessScheduleEvent &windowStart, ScheduleTrigger latestTrigger);

    std::string DumpSchedule() const;

    // Test-only queue barrier and state reset. Production code must not use either to infer timing.
    void WaitForIdle();
    void ResetForTest();

    struct Snapshot {
        pid_t managedPid = -1;
        int32_t managedUid = -1;
        int32_t effectiveLevel = 0;
        AttributionTool primaryTool = AttributionTool::UNKNOWN;
        std::vector<TaskScheduleContext> tasks;
    };

private:
    struct TaskState {
        int32_t level = 0;
        pid_t creatorPid = -1;
        int32_t creatorUid = -1;
        std::map<AttributionTool, int32_t> toolRefs;
    };

    struct PidEntry {
        int32_t level = 0;
        int32_t uid = -1;
        std::map<std::string, int32_t> taskLevels;
        std::map<std::string, int32_t> taskRefs;
        std::map<std::string, TaskState> taskStates;
    };

    ProcessSchedule();
    ~ProcessSchedule() = default;
    ProcessSchedule(const ProcessSchedule &) = delete;
    ProcessSchedule &operator=(const ProcessSchedule &) = delete;

    static int32_t MergeLevel(const PidEntry &entry);
    static AttributionTool MergePrimaryTool(const PidEntry &entry);
    static Snapshot BuildSnapshot(pid_t pid, const PidEntry &entry);
    static bool HasValidIdentity(const ScheduleAttributionInfo &info);
    static bool HasValidIdentity(const ProcessScheduleEvent &event);
    static bool IsImmediateStart(const ProcessScheduleEvent &event);

    void HandleAttributionAdded(const ScheduleAttributionInfo &info);
    void HandleAttributionRemoved(const ScheduleAttributionInfo &info, ScheduleTrigger trigger);
    void HandleTaskLevelChanged(const std::string &taskId, int32_t newLevel, ScheduleTrigger trigger);
    void HandleManagedUidResolved(const ScheduleAttributionInfo &info, ScheduleTrigger trigger);
    void HandleProcessDied(pid_t pid, int32_t uid);
    void RemoveAttributionLocked(const ScheduleAttributionInfo &info, ScheduleTrigger trigger);
    void ApplyControl(const ProcessScheduleEvent &event);
    void RestoreControl(const ProcessScheduleEvent &event);
    void LogContext(const ProcessScheduleEvent &event) const;
    uint64_t NextEpoch();

    mutable std::mutex scheduleMutex_;
    std::map<std::string, std::set<pid_t>> taskToPids_;
    std::map<pid_t, PidEntry> pidEntries_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> scheduleQueue_;
    uint64_t nextEpoch_ = 0;
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_APP_EXECFWK_TASK_MGR_PROCESS_SCHEDULE_H
