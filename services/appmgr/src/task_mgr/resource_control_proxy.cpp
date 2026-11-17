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

#include "task_mgr/resource_control_proxy.h"

#include <future>
#include <sstream>
#include <utility>

#include "ffrt.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
struct FallbackTimerContext {
    ResourceControlProxy *proxy = nullptr;
    uint64_t epoch = 0;
    ProcessScheduleEvent windowStart;
    ScheduleTrigger latestTrigger = ScheduleTrigger::ATTRIBUTION_ADDED;
    EventProvider provider;
};

void OnFallbackTimer(void *data)
{
    auto *context = static_cast<FallbackTimerContext *>(data);
    if (context == nullptr) {
        return;
    }
    auto proxy = context->proxy;
    auto epoch = context->epoch;
    auto windowStart = context->windowStart;
    auto latestTrigger = context->latestTrigger;
    auto provider = context->provider;
    delete context;
    if (proxy != nullptr) {
        proxy->SubmitDebounceExpired(epoch, windowStart, latestTrigger, provider);
    }
}

void LogEventTasks(uint64_t epoch, const char *phase, const std::vector<TaskScheduleContext> &tasks)
{
    for (const auto &task : tasks) {
        std::ostringstream tools;
        for (const auto tool : task.tools) {
            if (tools.tellp() > 0) {
                tools << ",";
            }
            tools << AttributionToolToString(tool);
        }
        TAG_LOGI(AAFwkTag::APPMGR,
            "sched control task epoch: %{public}" PRIu64 " phase: %{public}s taskId: %{public}s "
            "creatorPid: %{public}d creatorUid: %{public}d level: %{public}d tools: %{public}s",
            epoch, phase, task.taskId.c_str(), task.creatorPid, task.creatorUid, task.taskLevel,
            tools.str().c_str());
    }
}

template<typename T>
T ExecuteTestTaskSync(const std::shared_ptr<AAFwk::TaskHandlerWrap> &queue, std::function<T()> task)
{
    auto promise = std::make_shared<std::promise<T>>();
    auto future = promise->get_future();
    queue->SubmitTask([promise, task = std::move(task)]() mutable {
        promise->set_value(task());
    });
    return future.get();
}

void ExecuteTestTaskSync(const std::shared_ptr<AAFwk::TaskHandlerWrap> &queue, std::function<void()> task)
{
    auto promise = std::make_shared<std::promise<void>>();
    auto future = promise->get_future();
    queue->SubmitTask([promise, task = std::move(task)]() mutable {
        task();
        promise->set_value();
    });
    future.get();
}
}  // namespace

ResourceControlProxy &ResourceControlProxy::GetInstance()
{
    static ResourceControlProxy instance;
    return instance;
}

void ResourceControlProxy::SetEventQueue(const std::shared_ptr<AAFwk::TaskHandlerWrap> &queue)
{
    if (eventQueue_ != nullptr && eventQueue_ != queue) {
        TAG_LOGW(AAFwkTag::APPMGR, "resource control queue replaced");
    }
    eventQueue_ = queue;
}

bool ResourceControlProxy::IsImmediate(const ProcessScheduleEvent &event)
{
    if (event.trigger == ScheduleTrigger::MANAGED_PROCESS_DIED) {
        return true;
    }
    return event.action == ScheduleAction::START &&
        (event.uidRecovered || event.previousEffectiveLevel < event.effectiveLevel ||
         (event.previousEffectiveLevel == 0 && !event.currentTasks.empty()));
}

void ResourceControlProxy::ApplyControl(const ProcessScheduleEvent &event, const EventProvider &provider)
{
    if (event.managedUid <= 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "control apply suppressed for invalid uid, pid: %{public}d", event.managedPid);
        return;
    }
    if (IsImmediate(event)) {
        CancelDebounce(event.managedPid);
        EmitEvent(event);
        return;
    }
    ScheduleDebounce(event, provider);
}

void ResourceControlProxy::RestoreControl(const ProcessScheduleEvent &event, const EventProvider &provider)
{
    if (event.managedUid <= 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "control restore suppressed for invalid uid, pid: %{public}d", event.managedPid);
        return;
    }
    if (IsImmediate(event)) {
        CancelDebounce(event.managedPid);
        EmitEvent(event);
        return;
    }
    ScheduleDebounce(event, provider);
}

void ResourceControlProxy::ScheduleDebounce(const ProcessScheduleEvent &event, const EventProvider &provider)
{
    auto &pending = pending_[event.managedPid];
    // Every rearm uses the latest candidate as the immediate previous context. Its new epoch
    // also makes stale fallback callbacks unable to expire the new window.
    pending.epoch = event.epoch;
    pending.windowStart = event;
    pending.latestTrigger = event.trigger;
    pending.provider = provider;
    if (pending.timerTask) {
        pending.timerTask.Cancel();
    }
    if (pending.fallbackTimer != ffrt_error) {
        // Generation makes an old callback a no-op; do not stop it in the queue thread.
        pending.fallbackTimer = ffrt_error;
    }
    if (failNextTimer_) {
        failNextTimer_ = false;
        TAG_LOGW(AAFwkTag::APPMGR, "debounce timer creation failed, pending retained, pid: %{public}d",
            event.managedPid);
        return;
    }
    const uint64_t epoch = pending.epoch;
    const auto windowStart = pending.windowStart;
    const auto latestTrigger = pending.latestTrigger;
    pending.timerTask = eventQueue_->SubmitTask(
        [this, epoch, windowStart, latestTrigger, provider] {
            HandleDebounceExpired(epoch, windowStart, latestTrigger, provider);
        },
        debounceWindowMs_);
    if (pending.timerTask) {
        return;
    }

    auto context = std::make_unique<FallbackTimerContext>();
    context->proxy = this;
    context->epoch = epoch;
    context->windowStart = windowStart;
    context->latestTrigger = latestTrigger;
    context->provider = provider;
    pending.fallbackTimer = ffrt_timer_start(ffrt_qos_default, static_cast<uint64_t>(debounceWindowMs_),
        context.get(), [](void *data) { OnFallbackTimer(data); }, false);
    if (pending.fallbackTimer == ffrt_error) {
        TAG_LOGW(AAFwkTag::APPMGR, "debounce fallback timer creation failed, pending retained, pid: %{public}d",
            event.managedPid);
        return;
    }
    context.release();
}

void ResourceControlProxy::SubmitDebounceExpired(uint64_t epoch, const ProcessScheduleEvent &windowStart,
    ScheduleTrigger latestTrigger, const EventProvider &provider)
{
    eventQueue_->SubmitTask([this, epoch, windowStart, latestTrigger, provider] {
        HandleDebounceExpired(epoch, windowStart, latestTrigger, provider);
    });
}

void ResourceControlProxy::HandleDebounceExpired(uint64_t epoch, const ProcessScheduleEvent &windowStart,
    ScheduleTrigger latestTrigger, const EventProvider &provider)
{
    auto it = pending_.find(windowStart.managedPid);
    if (it == pending_.end() || it->second.epoch != epoch) {
        return;
    }
    PendingControl pending = std::move(it->second);
    pending_.erase(it);
    auto latest = provider(windowStart, latestTrigger);
    if (latest.has_value()) {
        EmitEvent(latest.value());
    }
}

void ResourceControlProxy::DiscardPendingControl(pid_t pid)
{
    CancelDebounce(pid);
}

void ResourceControlProxy::CancelDebounce(pid_t pid)
{
    auto it = pending_.find(pid);
    if (it == pending_.end()) {
        return;
    }
    if (it->second.timerTask) {
        it->second.timerTask.Cancel();
    }
    pending_.erase(it);
}

void ResourceControlProxy::EmitEvent(const ProcessScheduleEvent &event)
{
    if (event.managedPid <= 0 || event.managedUid <= 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "sched incomplete event rejected at exit, pid: %{public}d uid: %{public}d",
            event.managedPid, event.managedUid);
        return;
    }
    LogEvent(event);
    if (recordEventsForTest_) {
        emittedEventCount_[event.managedPid]++;
        eventHistoryForTest_[event.managedPid].emplace_back(event);
    }
    if (event.action == ScheduleAction::END) {
        lastNotifiedLevel_.erase(event.managedPid);
        lastNotifiedPrimaryTool_.erase(event.managedPid);
        return;
    }
    lastNotifiedLevel_[event.managedPid] = event.effectiveLevel;
    lastNotifiedPrimaryTool_[event.managedPid] = event.currentPrimaryTool;
}

void ResourceControlProxy::LogEvent(const ProcessScheduleEvent &event) const
{
    TAG_LOGI(AAFwkTag::APPMGR,
        "sched control epoch: %{public}" PRIu64 " action: %{public}d trigger: %{public}d "
        "pid: %{public}d uid: %{public}d level: %{public}d -> %{public}d primary: %{public}d -> %{public}d "
        "tasks: %{public}zu -> %{public}zu expired: %{public}d recovered: %{public}d",
        event.epoch, static_cast<int>(event.action), static_cast<int>(event.trigger), event.managedPid,
        event.managedUid, event.previousEffectiveLevel, event.effectiveLevel,
        static_cast<int>(event.previousPrimaryTool), static_cast<int>(event.currentPrimaryTool),
        event.previousTasks.size(), event.currentTasks.size(), event.debounceExpired, event.uidRecovered);
    LogEventTasks(event.epoch, "previous", event.previousTasks);
    LogEventTasks(event.epoch, "current", event.currentTasks);
}

void ResourceControlProxy::ResetForTest()
{
    ExecuteTestTaskSync(eventQueue_, [this] {
        for (auto &item : pending_) {
            if (item.second.timerTask) {
                item.second.timerTask.Cancel();
            }
        }
        pending_.clear();
        lastNotifiedLevel_.clear();
        lastNotifiedPrimaryTool_.clear();
        emittedEventCount_.clear();
        eventHistoryForTest_.clear();
        recordEventsForTest_ = false;
        debounceWindowMs_ = 1000;
        failNextTimer_ = false;
    });
}

void ResourceControlProxy::SetDebounceWindowMsForTest(int64_t delayMillis)
{
    ExecuteTestTaskSync(eventQueue_, [this, delayMillis] {
        if (delayMillis > 0) {
            debounceWindowMs_ = delayMillis;
        }
    });
}

void ResourceControlProxy::FailNextTimerForTest()
{
    ExecuteTestTaskSync(eventQueue_, [this] {
        failNextTimer_ = true;
    });
}

int32_t ResourceControlProxy::GetLastNotifiedLevel(pid_t pid) const
{
    return ExecuteTestTaskSync<int32_t>(eventQueue_, [this, pid] {
        auto it = lastNotifiedLevel_.find(pid);
        return it == lastNotifiedLevel_.end() ? 0 : it->second;
    });
}

AttributionTool ResourceControlProxy::GetLastNotifiedPrimaryTool(pid_t pid) const
{
    return ExecuteTestTaskSync<AttributionTool>(eventQueue_, [this, pid] {
        auto it = lastNotifiedPrimaryTool_.find(pid);
        return it == lastNotifiedPrimaryTool_.end() ? AttributionTool::UNKNOWN : it->second;
    });
}

size_t ResourceControlProxy::GetPendingCount(pid_t pid) const
{
    return ExecuteTestTaskSync<size_t>(eventQueue_, [this, pid] {
        return pending_.find(pid) == pending_.end() ? 0U : 1U;
    });
}

uint64_t ResourceControlProxy::GetEventCountForTest(pid_t pid) const
{
    return ExecuteTestTaskSync<uint64_t>(eventQueue_, [this, pid] {
        auto it = emittedEventCount_.find(pid);
        return it == emittedEventCount_.end() ? 0ULL : it->second;
    });
}

void ResourceControlProxy::SetEventRecordingForTest(bool enabled)
{
    ExecuteTestTaskSync(eventQueue_, [this, enabled] {
        eventHistoryForTest_.clear();
        recordEventsForTest_ = enabled;
    });
}

std::vector<ProcessScheduleEvent> ResourceControlProxy::GetEventHistoryForTest(pid_t pid) const
{
    return ExecuteTestTaskSync<std::vector<ProcessScheduleEvent>>(eventQueue_, [this, pid] {
        auto it = eventHistoryForTest_.find(pid);
        return it == eventHistoryForTest_.end() ? std::vector<ProcessScheduleEvent>{} : it->second;
    });
}

AAFwk::TaskHandle ResourceControlProxy::GetDebounceTaskForTest(pid_t pid)
{
    return ExecuteTestTaskSync<AAFwk::TaskHandle>(eventQueue_, [this, pid] {
        auto it = pending_.find(pid);
        return it == pending_.end() ? AAFwk::TaskHandle{} : it->second.timerTask;
    });
}

int64_t ResourceControlProxy::GetDebounceWindowMsForTest() const
{
    return ExecuteTestTaskSync<int64_t>(eventQueue_, [this] {
        return debounceWindowMs_;
    });
}

}  // namespace AppExecFwk
}  // namespace OHOS
