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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_RESOURCE_CONTROL_PROXY_H
#define OHOS_APP_EXECFWK_TASK_MGR_RESOURCE_CONTROL_PROXY_H

#include <cstdint>
#include <map>
#include <memory>
#include <optional>

#include "ffrt.h"
#include "task_handler_wrap.h"
#include "task_mgr/process_schedule.h"

namespace OHOS {
namespace AppExecFwk {
class ResourceControlProxy {
public:
    static ResourceControlProxy &GetInstance();

    // ProcessSchedule injects the single shared queue before the first event.
    void SetEventQueue(const std::shared_ptr<AAFwk::TaskHandlerWrap> &queue);

    // START candidates: new or upgraded control is immediate; downgrade is debounced.
    void ApplyControl(const ProcessScheduleEvent &event, const EventProvider &provider);
    // END candidates: process death is immediate; other final removal is debounced.
    void RestoreControl(const ProcessScheduleEvent &event, const EventProvider &provider);

    // Drops an obsolete pending lifecycle without changing last-notified state.
    void DiscardPendingControl(pid_t pid);

    // Log-only exit. Never calls ApplyControl/RestoreControl recursively or real RSS.
    void EmitEvent(const ProcessScheduleEvent &event);

    // Fallback-timer entry; marshals expiry back to the single schedule queue.
    void SubmitDebounceExpired(uint64_t epoch, const ProcessScheduleEvent &windowStart,
        ScheduleTrigger latestTrigger, const EventProvider &provider);

    // Test support: clears pending work without resetting the process-wide epoch.
    void ResetForTest();
    void SetDebounceWindowMsForTest(int64_t delayMillis);
    void FailNextTimerForTest();
    int32_t GetLastNotifiedLevel(pid_t pid) const;
    AttributionTool GetLastNotifiedPrimaryTool(pid_t pid) const;
    size_t GetPendingCount(pid_t pid) const;
    uint64_t GetEventCountForTest(pid_t pid) const;
    void SetEventRecordingForTest(bool enabled);
    std::vector<ProcessScheduleEvent> GetEventHistoryForTest(pid_t pid) const;
    AAFwk::TaskHandle GetDebounceTaskForTest(pid_t pid);
    int64_t GetDebounceWindowMsForTest() const;

private:
    struct PendingControl {
        uint64_t epoch = 0;
        ProcessScheduleEvent windowStart;
        ScheduleTrigger latestTrigger = ScheduleTrigger::ATTRIBUTION_ADDED;
        EventProvider provider;
        AAFwk::TaskHandle timerTask;
        ffrt_timer_t fallbackTimer = ffrt_error;
    };

    ResourceControlProxy() = default;
    ~ResourceControlProxy() = default;
    ResourceControlProxy(const ResourceControlProxy &) = delete;
    ResourceControlProxy &operator=(const ResourceControlProxy &) = delete;

    static bool IsImmediate(const ProcessScheduleEvent &event);
    void ScheduleDebounce(const ProcessScheduleEvent &event, const EventProvider &provider);
    void HandleDebounceExpired(uint64_t epoch, const ProcessScheduleEvent &windowStart,
        ScheduleTrigger latestTrigger, const EventProvider &provider);
    void CancelDebounce(pid_t pid);
    void LogEvent(const ProcessScheduleEvent &event) const;

    std::shared_ptr<AAFwk::TaskHandlerWrap> eventQueue_;
    std::map<pid_t, PendingControl> pending_;
    std::map<pid_t, int32_t> lastNotifiedLevel_;
    std::map<pid_t, AttributionTool> lastNotifiedPrimaryTool_;
    std::map<pid_t, uint64_t> emittedEventCount_;
    std::map<pid_t, std::vector<ProcessScheduleEvent>> eventHistoryForTest_;
    bool recordEventsForTest_ = false;
    int64_t debounceWindowMs_ = 1000;
    bool failNextTimer_ = false;
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_APP_EXECFWK_TASK_MGR_RESOURCE_CONTROL_PROXY_H
