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

#include "task_mgr/process_schedule.h"

#include <algorithm>
#include <sstream>
#include <utility>

#include "hilog_tag_wrapper.h"
#include "task_mgr/common.h"
#include "task_mgr/resource_control_proxy.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
bool IsValidLevel(int32_t level)
{
    return level == 0 || level == TASK_PRIORITY_LOW || level == TASK_PRIORITY_MEDIUM || level == TASK_PRIORITY_HIGH;
}

ProcessScheduleEvent MakeEvent(ScheduleAction action, ScheduleTrigger trigger,
    const ProcessSchedule::Snapshot &previous, const ProcessSchedule::Snapshot &current,
    bool uidRecovered = false)
{
    ProcessScheduleEvent event;
    event.action = action;
    event.trigger = trigger;
    event.uidRecovered = uidRecovered;
    event.managedPid = current.managedPid;
    event.managedUid = current.managedUid;
    event.previousEffectiveLevel = previous.effectiveLevel;
    event.effectiveLevel = current.effectiveLevel;
    event.previousPrimaryTool = previous.primaryTool;
    event.currentPrimaryTool = current.primaryTool;
    event.previousTasks = previous.tasks;
    event.currentTasks = current.tasks;
    return event;
}
}  // namespace

ProcessSchedule &ProcessSchedule::GetInstance()
{
    static ProcessSchedule instance;
    return instance;
}

ProcessSchedule::ProcessSchedule()
    : scheduleQueue_(AAFwk::TaskHandlerWrap::CreateQueueHandler("process_schedule"))
{
    ResourceControlProxy::GetInstance().SetEventQueue(scheduleQueue_);
}

bool ProcessSchedule::IsControlledTool(AttributionTool tool)
{
    return tool == AttributionTool::CLI || tool == AttributionTool::VIRTUAL_SCREEN_ABILITY;
}

int32_t ProcessSchedule::MergeLevel(const PidEntry &entry)
{
    int32_t level = 0;
    for (const auto &item : entry.taskLevels) {
        level = std::max(level, item.second);
    }
    return level;
}

AttributionTool ProcessSchedule::MergePrimaryTool(const PidEntry &entry)
{
    AttributionTool primary = AttributionTool::UNKNOWN;
    for (const auto &item : entry.taskStates) {
        if (item.second.level != entry.level) {
            continue;
        }
        if (item.second.toolRefs.count(AttributionTool::VIRTUAL_SCREEN_ABILITY) > 0) {
            return AttributionTool::VIRTUAL_SCREEN_ABILITY;
        }
        if (item.second.toolRefs.count(AttributionTool::CLI) > 0) {
            primary = AttributionTool::CLI;
        }
    }
    return primary;
}

ProcessSchedule::Snapshot ProcessSchedule::BuildSnapshot(pid_t pid, const PidEntry &entry)
{
    Snapshot snapshot;
    snapshot.managedPid = pid;
    snapshot.managedUid = entry.uid;
    snapshot.effectiveLevel = entry.level;
    snapshot.primaryTool = MergePrimaryTool(entry);
    snapshot.tasks.reserve(entry.taskStates.size());
    for (const auto &item : entry.taskStates) {
        TaskScheduleContext context;
        context.taskId = item.first;
        context.creatorPid = item.second.creatorPid;
        context.creatorUid = item.second.creatorUid;
        context.taskLevel = item.second.level;
        for (const auto &tool : item.second.toolRefs) {
            if (tool.second > 0) {
                context.tools.insert(tool.first);
            }
        }
        snapshot.tasks.emplace_back(std::move(context));
    }
    return snapshot;
}

bool ProcessSchedule::HasValidIdentity(const ScheduleAttributionInfo &info)
{
    return !info.taskId.empty() && info.creatorPid > 0 && info.creatorUid > 0 &&
        info.attribution.pid > 0 && info.taskLevel > 0 && IsValidLevel(info.taskLevel) &&
        IsControlledTool(info.attribution.tool);
}

bool ProcessSchedule::HasValidIdentity(const ProcessScheduleEvent &event)
{
    if (event.managedPid <= 0 || event.managedUid <= 0 ||
        !IsValidLevel(event.previousEffectiveLevel) || !IsValidLevel(event.effectiveLevel)) {
        return false;
    }
    auto validTasks = [](const std::vector<TaskScheduleContext> &tasks) {
        for (const auto &task : tasks) {
            if (task.taskId.empty() || task.creatorPid <= 0 || task.creatorUid <= 0 ||
                !IsValidLevel(task.taskLevel) || task.tools.empty()) {
                return false;
            }
            for (const auto &tool : task.tools) {
                if (!IsControlledTool(tool)) {
                    return false;
                }
            }
        }
        return true;
    };
    return validTasks(event.previousTasks) && validTasks(event.currentTasks);
}

bool ProcessSchedule::IsImmediateStart(const ProcessScheduleEvent &event)
{
    return event.uidRecovered || event.previousEffectiveLevel < event.effectiveLevel;
}

void ProcessSchedule::OnAttributionAdded(const ScheduleAttributionInfo &info)
{
    scheduleQueue_->SubmitTask([this, info] { HandleAttributionAdded(info); });
}

void ProcessSchedule::OnAttributionRemoved(const ScheduleAttributionInfo &info, ScheduleTrigger trigger)
{
    scheduleQueue_->SubmitTask([this, info, trigger] { HandleAttributionRemoved(info, trigger); });
}

void ProcessSchedule::OnTaskLevelChanged(const std::string &taskId, int32_t newLevel, ScheduleTrigger trigger)
{
    scheduleQueue_->SubmitTask([this, taskId, newLevel, trigger] {
        HandleTaskLevelChanged(taskId, newLevel, trigger);
    });
}

void ProcessSchedule::OnManagedUidResolved(const ScheduleAttributionInfo &info, ScheduleTrigger trigger)
{
    scheduleQueue_->SubmitTask([this, info, trigger] { HandleManagedUidResolved(info, trigger); });
}

void ProcessSchedule::OnProcessDied(pid_t pid, int32_t uid)
{
    scheduleQueue_->SubmitTask([this, pid, uid] { HandleProcessDied(pid, uid); });
}

void ProcessSchedule::HandleAttributionAdded(const ScheduleAttributionInfo &info)
{
    if (!HasValidIdentity(info)) {
        return;
    }
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    auto &entry = pidEntries_[info.attribution.pid];
    Snapshot previous = BuildSnapshot(info.attribution.pid, entry);
    if (entry.uid <= 0) {
        entry.uid = info.attribution.uid;
    } else if (entry.uid != info.attribution.uid) {
        TAG_LOGW(AAFwkTag::APPMGR, "sched uid mismatch, pid: %{public}d cached: %{public}d new: %{public}d",
            info.attribution.pid, entry.uid, info.attribution.uid);
    }
    const auto taskId = info.taskId;
    entry.taskRefs[taskId]++;
    entry.taskStates[taskId].toolRefs[info.attribution.tool]++;
    entry.taskStates[taskId].level = info.taskLevel;
    entry.taskStates[taskId].creatorPid = info.creatorPid;
    entry.taskStates[taskId].creatorUid = info.creatorUid;
    entry.taskLevels[taskId] = info.taskLevel;
    taskToPids_[taskId].insert(info.attribution.pid);
    entry.level = MergeLevel(entry);
    Snapshot current = BuildSnapshot(info.attribution.pid, entry);
    bool uidRecovered = !previous.tasks.empty() && previous.managedUid <= 0 && current.managedUid > 0;
    ProcessScheduleEvent event = MakeEvent(ScheduleAction::START, ScheduleTrigger::ATTRIBUTION_ADDED,
        previous, current, uidRecovered);
    event.epoch = NextEpoch();
    LogContext(event);
    if (IsImmediateStart(event)) {
        ApplyControl(event);
    }
}

void ProcessSchedule::HandleAttributionRemoved(const ScheduleAttributionInfo &info, ScheduleTrigger trigger)
{
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    RemoveAttributionLocked(info, trigger);
}

void ProcessSchedule::RemoveAttributionLocked(const ScheduleAttributionInfo &info, ScheduleTrigger trigger)
{
    auto entryIt = pidEntries_.find(info.attribution.pid);
    if (entryIt == pidEntries_.end()) {
        return;
    }
    auto &entry = entryIt->second;
    auto taskIt = entry.taskStates.find(info.taskId);
    if (taskIt == entry.taskStates.end()) {
        return;
    }
    auto toolIt = taskIt->second.toolRefs.find(info.attribution.tool);
    if (toolIt == taskIt->second.toolRefs.end()) {
        return;
    }
    Snapshot previous = BuildSnapshot(info.attribution.pid, entry);
    auto refsIt = entry.taskRefs.find(info.taskId);
    if (refsIt == entry.taskRefs.end()) {
        return;
    }
    if (toolIt->second > 0) {
        toolIt->second--;
    }
    if (refsIt->second > 0) {
        refsIt->second--;
    }
    if (toolIt->second <= 0) {
        taskIt->second.toolRefs.erase(toolIt);
    }
    if (refsIt->second <= 0) {
        entry.taskRefs.erase(refsIt);
        entry.taskStates.erase(taskIt);
        entry.taskLevels.erase(info.taskId);
        auto pidsIt = taskToPids_.find(info.taskId);
        if (pidsIt != taskToPids_.end()) {
            pidsIt->second.erase(info.attribution.pid);
            if (pidsIt->second.empty()) {
                taskToPids_.erase(pidsIt);
            }
        }
    }
    entry.level = MergeLevel(entry);
    Snapshot current = BuildSnapshot(info.attribution.pid, entry);
    if (current.tasks.empty()) {
        pidEntries_.erase(entryIt);
    }
    ProcessScheduleEvent event = MakeEvent(current.tasks.empty() ? ScheduleAction::END : ScheduleAction::START,
        trigger, previous, current);
    event.epoch = NextEpoch();
    LogContext(event);
    if (current.tasks.empty()) {
        RestoreControl(event);
    } else if (current.effectiveLevel < previous.effectiveLevel) {
        ApplyControl(event);
    }
}

void ProcessSchedule::HandleTaskLevelChanged(const std::string &taskId, int32_t newLevel,
    ScheduleTrigger trigger)
{
    if (newLevel <= 0 || !IsValidLevel(newLevel)) {
        return;
    }
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    auto taskIt = taskToPids_.find(taskId);
    if (taskIt == taskToPids_.end()) {
        return;
    }
    std::vector<pid_t> pids(taskIt->second.begin(), taskIt->second.end());
    for (pid_t pid : pids) {
        auto entryIt = pidEntries_.find(pid);
        if (entryIt == pidEntries_.end()) {
            continue;
        }
        auto levelIt = entryIt->second.taskLevels.find(taskId);
        if (levelIt == entryIt->second.taskLevels.end()) {
            continue;
        }
        Snapshot previous = BuildSnapshot(pid, entryIt->second);
        levelIt->second = newLevel;
        entryIt->second.taskStates[taskId].level = newLevel;
        entryIt->second.level = MergeLevel(entryIt->second);
        Snapshot current = BuildSnapshot(pid, entryIt->second);
        ProcessScheduleEvent event = MakeEvent(ScheduleAction::START, trigger, previous, current);
        event.epoch = NextEpoch();
        LogContext(event);
        if (current.effectiveLevel > previous.effectiveLevel) {
            ApplyControl(event);
        } else if (current.effectiveLevel < previous.effectiveLevel) {
            ApplyControl(event);
        }
    }
}

void ProcessSchedule::HandleManagedUidResolved(const ScheduleAttributionInfo &info, ScheduleTrigger trigger)
{
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    auto entryIt = pidEntries_.find(info.attribution.pid);
    if (entryIt == pidEntries_.end() || entryIt->second.uid > 0 || info.attribution.uid <= 0) {
        return;
    }
    entryIt->second.uid = info.attribution.uid;
    Snapshot current = BuildSnapshot(info.attribution.pid, entryIt->second);
    Snapshot previous;
    previous.managedPid = info.attribution.pid;
    previous.managedUid = -1;
    ProcessScheduleEvent event = MakeEvent(current.tasks.empty() ? ScheduleAction::END : ScheduleAction::START,
        trigger, previous, current, true);
    event.epoch = NextEpoch();
    LogContext(event);
    if (current.tasks.empty()) {
        RestoreControl(event);
    } else {
        ApplyControl(event);
    }
}

void ProcessSchedule::HandleProcessDied(pid_t pid, int32_t uid)
{
    if (pid <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    auto entryIt = pidEntries_.find(pid);
    if (entryIt == pidEntries_.end()) {
        return;
    }
    if (entryIt->second.uid <= 0) {
        entryIt->second.uid = uid;
    }
    Snapshot previous = BuildSnapshot(pid, entryIt->second);
    Snapshot current;
    current.managedPid = pid;
    current.managedUid = previous.managedUid;
    ProcessScheduleEvent event = MakeEvent(ScheduleAction::END, ScheduleTrigger::MANAGED_PROCESS_DIED,
        previous, current);
    event.epoch = NextEpoch();
    LogContext(event);
    RestoreControl(event);
    std::vector<std::string> taskIds;
    for (const auto &item : entryIt->second.taskLevels) {
        taskIds.emplace_back(item.first);
    }
    pidEntries_.erase(entryIt);
    for (const auto &taskId : taskIds) {
        auto taskIt = taskToPids_.find(taskId);
        if (taskIt != taskToPids_.end()) {
            taskIt->second.erase(pid);
            if (taskIt->second.empty()) {
                taskToPids_.erase(taskIt);
            }
        }
    }
}

void ProcessSchedule::ApplyControl(const ProcessScheduleEvent &event)
{
    if (!HasValidIdentity(event)) {
        ResourceControlProxy::GetInstance().DiscardPendingControl(event.managedPid);
        TAG_LOGW(AAFwkTag::APPMGR, "sched incomplete event suppressed, pid: %{public}d uid: %{public}d",
            event.managedPid, event.managedUid);
        return;
    }
    ResourceControlProxy::GetInstance().ApplyControl(event,
        [this](const ProcessScheduleEvent &windowStart, ScheduleTrigger latestTrigger) {
            return BuildLatestEvent(windowStart, latestTrigger);
        });
}

void ProcessSchedule::RestoreControl(const ProcessScheduleEvent &event)
{
    if (!HasValidIdentity(event)) {
        ResourceControlProxy::GetInstance().DiscardPendingControl(event.managedPid);
        TAG_LOGW(AAFwkTag::APPMGR, "sched incomplete event suppressed, pid: %{public}d uid: %{public}d",
            event.managedPid, event.managedUid);
        return;
    }
    ResourceControlProxy::GetInstance().RestoreControl(event,
        [this](const ProcessScheduleEvent &windowStart, ScheduleTrigger latestTrigger) {
            return BuildLatestEvent(windowStart, latestTrigger);
        });
}

std::optional<ProcessScheduleEvent> ProcessSchedule::BuildLatestEvent(
    const ProcessScheduleEvent &windowStart, ScheduleTrigger latestTrigger)
{
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    auto entryIt = pidEntries_.find(windowStart.managedPid);
    Snapshot current;
    current.managedPid = windowStart.managedPid;
    current.managedUid = windowStart.managedUid;
    if (entryIt != pidEntries_.end()) {
        current = BuildSnapshot(windowStart.managedPid, entryIt->second);
    }
    if (current.managedUid <= 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "sched debounce event suppressed for invalid uid, pid: %{public}d",
            windowStart.managedPid);
        return std::nullopt;
    }
    if (current.tasks.empty() && windowStart.previousEffectiveLevel <= 0) {
        return std::nullopt;
    }
    if (!current.tasks.empty() && current.effectiveLevel == windowStart.previousEffectiveLevel) {
        return std::nullopt;
    }

    // Keep the last-notified baseline from windowStart and replace only the latest state.
    ProcessScheduleEvent event = windowStart;
    event.action = current.tasks.empty() ? ScheduleAction::END : ScheduleAction::START;
    event.trigger = latestTrigger;
    event.debounceExpired = true;
    event.managedPid = current.managedPid;
    event.managedUid = current.managedUid;
    event.effectiveLevel = current.effectiveLevel;
    event.currentPrimaryTool = current.primaryTool;
    event.currentTasks = current.tasks;
    event.epoch = NextEpoch();
    return event;
}

void ProcessSchedule::LogContext(const ProcessScheduleEvent &event) const
{
    TAG_LOGI(AAFwkTag::APPMGR,
        "sched context epoch: %{public}" PRIu64 " action: %{public}d trigger: %{public}d "
        "pid: %{public}d uid: %{public}d level: %{public}d -> %{public}d primary: %{public}d -> %{public}d "
        "tasks: %{public}zu -> %{public}zu expired: %{public}d recovered: %{public}d",
        event.epoch, static_cast<int>(event.action), static_cast<int>(event.trigger), event.managedPid,
        event.managedUid, event.previousEffectiveLevel, event.effectiveLevel,
        static_cast<int>(event.previousPrimaryTool), static_cast<int>(event.currentPrimaryTool),
        event.previousTasks.size(), event.currentTasks.size(), event.debounceExpired, event.uidRecovered);
}

uint64_t ProcessSchedule::NextEpoch()
{
    return ++nextEpoch_;
}

void ProcessSchedule::WaitForIdle()
{
    auto handle = scheduleQueue_->SubmitTask([] {});
    handle.Sync();
}

void ProcessSchedule::ResetForTest()
{
    WaitForIdle();
    ResourceControlProxy::GetInstance().ResetForTest();
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    taskToPids_.clear();
    pidEntries_.clear();
}

std::string ProcessSchedule::DumpSchedule() const
{
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    std::ostringstream oss;
    oss << "ProcessSchedule index: tasks=" << taskToPids_.size() << " pids=" << pidEntries_.size() << "\n";
    for (const auto &item : taskToPids_) {
        oss << "      task #" << item.first << " -> pids [";
        bool first = true;
        for (pid_t pid : item.second) {
            if (!first) {
                oss << " ";
            }
            oss << pid;
            first = false;
        }
        oss << "]\n";
    }
    for (const auto &item : pidEntries_) {
        oss << "      pid " << item.first << " uid " << item.second.uid << " -> tasks [";
        bool first = true;
        for (const auto &task : item.second.taskLevels) {
            if (!first) {
                oss << " ";
            }
            oss << task.first;
            first = false;
        }
        oss << "] effectiveLevel [" << TaskPriorityToString(item.second.level) << "] primary ["
            << AttributionToolToString(MergePrimaryTool(item.second)) << "]\n";
    }
    return oss.str();
}

}  // namespace AppExecFwk
}  // namespace OHOS
