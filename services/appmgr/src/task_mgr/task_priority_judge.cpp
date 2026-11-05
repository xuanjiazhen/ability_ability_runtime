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

#include "task_mgr/task_priority_judge.h"

#include "hilog_tag_wrapper.h"
#include "task_mgr/common.h"

namespace OHOS {
namespace AppExecFwk {

namespace {
constexpr int32_t FUSION_LEVEL_HIGH_LOAD_MIN = 5;
constexpr int32_t FUSION_LEVEL_HIGH_LOAD_MAX = 7;
}

TaskPriorityJudge &TaskPriorityJudge::GetInstance()
{
    static TaskPriorityJudge instance;
    return instance;
}

bool TaskPriorityJudge::IsHighLoad(int32_t loadLevel)
{
    return loadLevel >= FUSION_LEVEL_HIGH_LOAD_MIN && loadLevel <= FUSION_LEVEL_HIGH_LOAD_MAX;
}

int32_t TaskPriorityJudge::DetermineLevel(bool foreground, int32_t loadLevel)
{
    if (foreground) {
        return TASK_PRIORITY_HIGH;
    }
    if (IsHighLoad(loadLevel)) {
        return TASK_PRIORITY_LOW;
    }
    return TASK_PRIORITY_MEDIUM;
}

void TaskPriorityJudge::OnForegroundAbilityChanged(pid_t pid, bool foreground)
{
    if (pid <= 0) {
        return;
    }
    std::lock_guard<std::mutex> lock(judgeMutex_);
    int32_t count = fgAbilityCounts_[pid] + (foreground ? 1 : -1);
    if (count <= 0) {
        fgAbilityCounts_.erase(pid);
    } else {
        fgAbilityCounts_[pid] = count;
    }
}

void TaskPriorityJudge::OnSystemLoadLevelChanged(int32_t loadLevel)
{
    std::lock_guard<std::mutex> lock(judgeMutex_);
    if (loadLevel < 0 || loadLevel > FUSION_LEVEL_HIGH_LOAD_MAX) {
        TAG_LOGW(AAFwkTag::APPMGR, "systemload level out of range: %{public}d", loadLevel);
        loadLevel_ = -1;
        return;
    }
    loadLevel_ = loadLevel;
}

bool TaskPriorityJudge::IsForeground(pid_t pid) const
{
    std::lock_guard<std::mutex> lock(judgeMutex_);
    auto it = fgAbilityCounts_.find(pid);
    return it != fgAbilityCounts_.end() && it->second > 0;
}

int32_t TaskPriorityJudge::CurrentLoadLevel() const
{
    std::lock_guard<std::mutex> lock(judgeMutex_);
    return loadLevel_;
}

int32_t TaskPriorityJudge::JudgeLevel(pid_t pid) const
{
    std::lock_guard<std::mutex> lock(judgeMutex_);
    auto it = fgAbilityCounts_.find(pid);
    bool foreground = (it != fgAbilityCounts_.end() && it->second > 0);
    return DetermineLevel(foreground, loadLevel_);
}

void TaskPriorityJudge::SetLoadProvider(const std::shared_ptr<ISystemLoadProvider> &provider)
{
    std::lock_guard<std::mutex> lock(judgeMutex_);
    loadProvider_ = provider;
    loadListenerRegistered_ = false;
}

bool TaskPriorityJudge::RegisterSystemLoadListener()
{
    std::shared_ptr<ISystemLoadProvider> provider;
    {
        std::lock_guard<std::mutex> lock(judgeMutex_);
        if (loadListenerRegistered_) {
            return true;
        }
        if (loadProvider_ == nullptr) {
            loadProvider_ = CreateDefaultSystemLoadProvider();
        }
        provider = loadProvider_;
    }
    bool ok = provider != nullptr &&
        provider->RegisterListener([](int32_t level) {
            TaskPriorityJudge::GetInstance().OnSystemLoadLevelChanged(level);
            TaskPriorityJudge::GetInstance().NotifyLevelChanged(level);
        });
    std::lock_guard<std::mutex> lock(judgeMutex_);
    loadListenerRegistered_ = ok;
    if (!ok) {
        TAG_LOGE(AAFwkTag::APPMGR, "register systemload listener failed, judge falls back");
    }
    return ok;
}

void TaskPriorityJudge::SetLevelChangedHook(std::function<void(int32_t)> hook)
{
    std::lock_guard<std::mutex> lock(judgeMutex_);
    levelChangedHook_ = std::move(hook);
}

void TaskPriorityJudge::NotifyLevelChanged(int32_t level)
{
    std::function<void(int32_t)> hook;
    {
        std::lock_guard<std::mutex> lock(judgeMutex_);
        hook = levelChangedHook_;
    }
    if (hook != nullptr) {
        hook(level);
    }
}

void TaskPriorityJudge::UnregisterSystemLoadListener()
{
    std::shared_ptr<ISystemLoadProvider> provider;
    {
        std::lock_guard<std::mutex> lock(judgeMutex_);
        if (!loadListenerRegistered_) {
            return;
        }
        loadListenerRegistered_ = false;
        provider = loadProvider_;
    }
    if (provider != nullptr) {
        provider->UnregisterListener();
    }
}

std::string TaskPriorityJudge::DumpJudgeState() const
{
    std::lock_guard<std::mutex> lock(judgeMutex_);
    std::string result = "Judge state:\n";
    result += "      foreground ability counts [";
    bool first = true;
    for (const auto &item : fgAbilityCounts_) {
        if (item.second <= 0) {
            continue;
        }
        if (!first) {
            result += " ";
        }
        result += "pid:" + std::to_string(item.first) + " count:" + std::to_string(item.second);
        first = false;
    }
    if (first) {
        result += "empty";
    }
    result += "]\n";
    result += "      cached systemload level [" + std::to_string(loadLevel_) + "]\n";
    result += std::string("      load listener [") + (loadListenerRegistered_ ? "registered" : "not registered") +
        "]\n";
    return result;
}

}  // namespace AppExecFwk
}  // namespace OHOS
