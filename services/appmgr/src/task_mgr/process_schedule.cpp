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

#include <sstream>

#include "hilog_tag_wrapper.h"
#include "task_mgr/common.h"

namespace OHOS {
namespace AppExecFwk {

ProcessSchedule &ProcessSchedule::GetInstance()
{
    static ProcessSchedule instance;
    return instance;
}

int32_t ProcessSchedule::MergeLevel(const PidEntry &entry)
{
    int32_t level = 0;
    for (const auto &item : entry.taskLevels) {
        level = std::max(level, item.second);
    }
    return level;
}

void ProcessSchedule::OnAttributionAdded(const std::string &taskId, int32_t taskLevel,
    pid_t pid, AttributionTool tool)
{
    if (pid <= 0) {
        return;
    }
    std::vector<ControlEvent> events;
    {
        std::lock_guard<std::mutex> lock(scheduleMutex_);
        auto &entry = pidEntries_[pid];
        int32_t refs = ++entry.taskRefs[taskId];
        if (refs > 1) {
            // the merge state already reflects this (taskId, pid); only the reference grew
            TAG_LOGD(AAFwkTag::APPMGR, "attribution refcount grown, taskId: %{public}s pid: %{public}d refs: %{public}d",
                taskId.c_str(), pid, refs);
            return;
        }
        // create the per-tool derived scheduling object via the factory (ADR-1)
        auto record = CreateAttributionRecord(tool);
        record->pid = pid;
        entry.attrObjects[taskId] = record;
        entry.taskLevels.emplace(taskId, taskLevel);
        taskToPids_[taskId].insert(pid);
        int32_t newLevel = MergeLevel(entry);
        TAG_LOGI(AAFwkTag::APPMGR,
            "sched attribution added, taskId: %{public}s pid: %{public}d tool: %{public}s level: %{public}d "
            "effective: %{public}d",
            taskId.c_str(), pid, AttributionToolToString(tool), taskLevel, newLevel);
        CollectEventLocked(pid, newLevel, ControlEvent::Type::APPLY, events);
        entry.level = newLevel;
    }
    DispatchEvents(events);
}

void ProcessSchedule::OnAttributionRemoved(const std::string &taskId, pid_t pid)
{
    if (pid <= 0) {
        return;
    }
    std::vector<ControlEvent> events;
    {
        std::lock_guard<std::mutex> lock(scheduleMutex_);
        RemoveAttributionLocked(taskId, pid, events);
    }
    DispatchEvents(events);
}

void ProcessSchedule::OnTaskLevelChanged(const std::string &taskId, int32_t newLevel)
{
    std::vector<ControlEvent> events;
    {
        std::lock_guard<std::mutex> lock(scheduleMutex_);
        auto it = taskToPids_.find(taskId);
        if (it == taskToPids_.end()) {
            return;
        }
        std::vector<pid_t> pids(it->second.begin(), it->second.end());
        for (pid_t pid : pids) {
            auto entryIt = pidEntries_.find(pid);
            if (entryIt == pidEntries_.end()) {
                continue;
            }
            auto tlIt = entryIt->second.taskLevels.find(taskId);
            if (tlIt == entryIt->second.taskLevels.end()) {
                continue;
            }
            int32_t oldLevel = entryIt->second.level;
            tlIt->second = newLevel;
            int32_t merged = MergeLevel(entryIt->second);
            if (merged == oldLevel) {
                continue;  // first-line anti-jitter: no propagation when the merge result is unchanged
            }
            CollectEventLocked(pid, merged, ControlEvent::Type::APPLY, events);
            entryIt->second.level = merged;
        }
    }
    DispatchEvents(events);
}

void ProcessSchedule::OnProcessDied(pid_t pid)
{
    if (pid <= 0) {
        return;
    }
    std::vector<ControlEvent> events;
    {
        std::lock_guard<std::mutex> lock(scheduleMutex_);
        auto entryIt = pidEntries_.find(pid);
        if (entryIt == pidEntries_.end()) {
            return;
        }
        std::vector<std::string> taskIds;
        for (const auto &item : entryIt->second.taskLevels) {
            taskIds.push_back(item.first);
        }
        CollectEventLocked(pid, entryIt->second.level, ControlEvent::Type::REVOKE_PID, events);
        // keep the died process's records for the task-level degrade on task end
        for (const auto &obj : entryIt->second.attrObjects) {
            endedRecords_[obj.first].push_back(obj.second);
        }
        for (const auto &taskId : taskIds) {
            auto ttIt = taskToPids_.find(taskId);
            if (ttIt != taskToPids_.end()) {
                ttIt->second.erase(pid);
                if (ttIt->second.empty()) {
                    taskToPids_.erase(ttIt);
                }
            }
        }
        pidEntries_.erase(entryIt);
        TAG_LOGI(AAFwkTag::APPMGR,
            "sched process died cleaned, pid: %{public}d tasks: %{public}zu", pid, taskIds.size());
    }
    DispatchEvents(events);
}

void ProcessSchedule::OnTaskEnd(const std::string &taskId, const std::vector<pid_t> &pids)
{
    std::vector<std::shared_ptr<AttributionRecord>> records;
    {
        std::lock_guard<std::mutex> lock(scheduleMutex_);
        auto it = endedRecords_.find(taskId);
        if (it != endedRecords_.end()) {
            records = std::move(it->second);
            endedRecords_.erase(it);
        }
    }
    // task-level degrade is delivered by the derived records themselves via the proxy (ADR-5)
    for (const auto &record : records) {
        record->NotifyTaskEnd(taskId, pids);
    }
}

void ProcessSchedule::RemoveAttributionLocked(const std::string &taskId, pid_t pid,
    std::vector<ControlEvent> &events)
{
    auto entryIt = pidEntries_.find(pid);
    if (entryIt == pidEntries_.end()) {
        return;
    }
    auto refIt = entryIt->second.taskRefs.find(taskId);
    if (refIt == entryIt->second.taskRefs.end()) {
        return;
    }
    if (--refIt->second > 0) {
        // one of several attribution records of this (taskId, pid) is gone; the merge state
        // is unchanged, only the reference shrinks
        TAG_LOGD(AAFwkTag::APPMGR, "attribution refcount shrunk, taskId: %{public}s pid: %{public}d refs: %{public}d",
            taskId.c_str(), pid, refIt->second);
        return;
    }
    entryIt->second.taskRefs.erase(refIt);
    entryIt->second.taskLevels.erase(taskId);
    int32_t oldLevel = entryIt->second.level;
    int32_t newLevel = MergeLevel(entryIt->second);
    if (entryIt->second.taskRefs.empty()) {
        // no task left on this pid: unschedule every derived object (including the removed
        // task's, still attached); keep the records for the task-level degrade on task end
        CollectEventLocked(pid, oldLevel, ControlEvent::Type::REVOKE_PID, events);
        for (const auto &obj : entryIt->second.attrObjects) {
            endedRecords_[obj.first].push_back(obj.second);
        }
        pidEntries_.erase(entryIt);
    } else {
        // other tasks keep the pid controlled: unschedule only the removed task's object
        auto objIt = entryIt->second.attrObjects.find(taskId);
        if (objIt != entryIt->second.attrObjects.end()) {
            ControlEvent unscheduleEvent;
            unscheduleEvent.pid = pid;
            unscheduleEvent.type = ControlEvent::Type::UNSCHEDULE;
            unscheduleEvent.records.emplace_back(taskId, objIt->second);
            events.push_back(unscheduleEvent);
            // keep the removed record for the task-level degrade on task end
            endedRecords_[taskId].push_back(objIt->second);
            entryIt->second.attrObjects.erase(objIt);
        }
        if (newLevel != oldLevel) {
            // downgrade: re-apply the remaining objects at the new merged level
            CollectEventLocked(pid, newLevel, ControlEvent::Type::APPLY, events);
            entryIt->second.level = newLevel;
        }
    }
    auto ttIt = taskToPids_.find(taskId);
    if (ttIt != taskToPids_.end()) {
        ttIt->second.erase(pid);
        if (ttIt->second.empty()) {
            taskToPids_.erase(ttIt);
        }
    }
}

void ProcessSchedule::CollectEventLocked(pid_t pid, int32_t newLevel, ControlEvent::Type type,
    std::vector<ControlEvent> &events)
{
    auto it = pidEntries_.find(pid);
    if (it == pidEntries_.end()) {
        return;
    }
    ControlEvent event;
    event.pid = pid;
    event.level = newLevel;
    event.type = type;
    for (const auto &obj : it->second.attrObjects) {
        event.records.emplace_back(obj.first, obj.second);
    }
    if (event.records.empty()) {
        // a pid with no attribution object produces no control event (INV-6: no taskId, no-op)
        return;
    }
    events.push_back(std::move(event));
}

void ProcessSchedule::DispatchEvents(const std::vector<ControlEvent> &events)
{
    // The process schedule only forwards events: each derived record delivers its own
    // schedule control through the proxy (Schedule/Unschedule per (taskId, pid) pair).
    for (const auto &event : events) {
        if (event.type == ControlEvent::Type::APPLY) {
            for (const auto &item : event.records) {
                item.second->Schedule(item.first, event.level);
            }
            continue;
        }
        // UNSCHEDULE / REVOKE_PID: Unschedule the (taskId, pid) pairs via the derived records
        for (const auto &item : event.records) {
            item.second->Unschedule(item.first);
        }
    }
}

std::string ProcessSchedule::DumpSchedule() const
{
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    std::ostringstream oss;
    size_t pidCount = pidEntries_.size();
    oss << "ProcessSchedule index: tasks=" << taskToPids_.size() << " pids=" << pidCount << "\n";
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
        oss << "      pid " << item.first << " -> tasks [";
        bool first = true;
        for (const auto &tl : item.second.taskLevels) {
            if (!first) {
                oss << " ";
            }
            oss << tl.first;
            first = false;
        }
        oss << "] effectiveLevel [" << TaskPriorityToString(item.second.level) << "]\n";
    }
    return oss.str();
}

}  // namespace AppExecFwk
}  // namespace OHOS
