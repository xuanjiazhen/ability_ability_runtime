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

#include "process_schedule/process_schedule.h"

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

const char *ProcessSchedule::ActionText(bool apply)
{
    return apply ? "APPLY" : "REVOKE";
}

void ProcessSchedule::OnAttributionAdded(const std::string &taskId, int32_t taskLevel,
    pid_t pid, AttributionTool tool)
{
    if (pid <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    auto &entry = pidEntries_[pid];
    auto it = entry.taskLevels.find(taskId);
    if (it != entry.taskLevels.end()) {
        TAG_LOGW(AAFwkTag::APPMGR, "attribution already tracked, taskId: %{public}s pid: %{public}d",
            taskId.c_str(), pid);
        return;
    }
    entry.taskLevels.emplace(taskId, taskLevel);
    taskToPids_[taskId].insert(pid);
    int32_t newLevel = MergeLevel(entry);
    exemptions_[taskId][pid]++;
    TAG_LOGI(AAFwkTag::APPMGR,
        "sched attribution added, taskId: %{public}s pid: %{public}d tool: %{public}s level: %{public}d, "
        "exemption applied (taskId+pid+type)", taskId.c_str(), pid, AttributionToolToString(tool), taskLevel);
    ApplyControlLocked(pid, newLevel, entry.level);
    entry.level = newLevel;
}

void ProcessSchedule::OnAttributionRemoved(const std::string &taskId, pid_t pid)
{
    if (pid <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    RemoveAttributionLocked(taskId, pid);
}

void ProcessSchedule::OnTaskLevelChanged(const std::string &taskId, int32_t newLevel)
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
        ApplyControlLocked(pid, merged, oldLevel);
        entryIt->second.level = merged;
    }
}

void ProcessSchedule::OnProcessDied(pid_t pid)
{
    if (pid <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(scheduleMutex_);
    auto entryIt = pidEntries_.find(pid);
    if (entryIt == pidEntries_.end()) {
        return;
    }
    std::vector<std::string> taskIds;
    for (const auto &item : entryIt->second.taskLevels) {
        taskIds.push_back(item.first);
    }
    int32_t oldLevel = entryIt->second.level;
    for (const auto &taskId : taskIds) {
        RevokeExemptionLocked(pid, taskId);
    }
    RevokeControlLocked(pid, oldLevel);
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
        "sched process died cleaned, pid: %{public}d tasks: %{public}zu, task records kept alive",
        pid, taskIds.size());
}

void ProcessSchedule::RemoveAttributionLocked(const std::string &taskId, pid_t pid)
{
    auto entryIt = pidEntries_.find(pid);
    if (entryIt == pidEntries_.end()) {
        return;
    }
    if (entryIt->second.taskLevels.erase(taskId) == 0) {
        return;
    }
    int32_t oldLevel = entryIt->second.level;
    int32_t newLevel = MergeLevel(entryIt->second);
    RevokeExemptionLocked(pid, taskId);
    if (entryIt->second.taskLevels.empty()) {
        RevokeControlLocked(pid, oldLevel);
        pidEntries_.erase(entryIt);
    } else {
        if (newLevel != oldLevel) {
            ApplyControlLocked(pid, newLevel, oldLevel);
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

void ProcessSchedule::RevokeExemptionLocked(pid_t pid, const std::string &taskId)
{
    auto exIt = exemptions_.find(taskId);
    if (exIt == exemptions_.end()) {
        return;
    }
    auto pidIt = exIt->second.find(pid);
    if (pidIt == exIt->second.end()) {
        return;
    }
    if (--pidIt->second <= 0) {
        exIt->second.erase(pidIt);
        TAG_LOGI(AAFwkTag::APPMGR, "sched exemption revoked, taskId: %{public}s pid: %{public}d",
            taskId.c_str(), pid);
    }
    if (exIt->second.empty()) {
        exemptions_.erase(exIt);
    }
}

void ProcessSchedule::ApplyControlLocked(pid_t pid, int32_t newLevel, int32_t oldLevel)
{
    // Stub by design (20260914692273): the real delivery lands with 20260914692272.
    TAG_LOGI(AAFwkTag::APPMGR,
        "sched control apply (stub) pid: %{public}d %{public}s -> %{public}s",
        pid, TaskPriorityToString(oldLevel), TaskPriorityToString(newLevel));
    (void)ActionText(true);
}

void ProcessSchedule::RevokeControlLocked(pid_t pid, int32_t oldLevel)
{
    // Stub by design (20260914692273): the real delivery lands with 20260914692272.
    TAG_LOGI(AAFwkTag::APPMGR,
        "sched control revoke (stub) pid: %{public}d %{public}s",
        pid, TaskPriorityToString(oldLevel));
    (void)ActionText(false);
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
    size_t total = 0;
    for (const auto &item : exemptions_) {
        total += item.second.size();
    }
    oss << "Exemption summary: total=" << total << "\n";
    for (const auto &item : exemptions_) {
        for (const auto &pidItem : item.second) {
            oss << "      pid:" << pidItem.first << " count:" << pidItem.second
                << " keys:[" << item.first << "+" << pidItem.first << "+CPU]\n";
        }
    }
    return oss.str();
}

}  // namespace AppExecFwk
}  // namespace OHOS
