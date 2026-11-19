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

#ifndef OHOS_ABILITY_RUNTIME_TOOL_PID_RESOLVER_H
#define OHOS_ABILITY_RUNTIME_TOOL_PID_RESOLVER_H

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include "session_record.h"

namespace OHOS {
namespace CliTool {

// Short-term bridge that switches the task-attribution anchor from the (near-idle)
// claw_sandbox HOST onto its real tool child. The tool is born inside the sandbox and
// is invisible to climgr — the aimgr /proc view is hidepid-clipped (enumeration AND
// by-name access both return ENOENT for other-uid entries), so the discovery PROBE
// itself lives on the appmgr side: RegisterManagedProcess(CLI) re-anchors the pid
// there (host pid -> live tool child; no child found -> the host pid, exactly the
// pre-discovery behavior, so a miss never leaves the task without a record).
// This resolver's remaining job is only PACING: registering right after fork would
// always probe before the sandbox finishes its setup (config parse + constraints +
// privilege drop + spawn), so a single delayed submit covers that window.
// Retires wholesale when the pgid group attribution lands.
class ToolPidResolver {
public:
    static ToolPidResolver &GetInstance();

    ToolPidResolver(const ToolPidResolver &) = delete;
    ToolPidResolver &operator=(const ToolPidResolver &) = delete;
    ToolPidResolver(ToolPidResolver &&) = delete;
    ToolPidResolver &operator=(ToolPidResolver &&) = delete;

    // Starts the delayed single-shot registration after the sandbox host fork
    // succeeded. No-op for sessions without a taskId: unattributed sessions pay
    // zero cost.
    void Resolve(const std::shared_ptr<SessionRecord> &record);

    // Test seam only: replaces the delayed submit. scheduler(delayMs, next) must
    // eventually run next(); tests inject recorders/drivers instead of ffrt.
    void SetSchedulerForTest(std::function<void(int64_t, std::function<void()>)> scheduler)
    {
        scheduler_ = std::move(scheduler);
    }

    // Test seam only: restores the production scheduler.
    void ResetForTest();

    // Sandbox setup + spawn window. When the window is missed, the server-side probe
    // falls back to the host pid, so an over-estimation costs nothing but a late
    // anchor chance (the tool resource would land on the host record instead).
    static constexpr int64_t RESOLVE_DELAY_MS = 300;

private:
    ToolPidResolver() = default;
    ~ToolPidResolver() = default;

    // Empty means production: the real ffrt delayed submit.
    std::function<void(int64_t, std::function<void()>)> scheduler_;
    // Bumped by ResetForTest. The scheduled task captures the current value and aborts
    // on mismatch, so a delayed registration can never run against a reset/destroying
    // test boundary (UB guard).
    std::atomic<uint64_t> epoch_ { 0 };
};
} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_TOOL_PID_RESOLVER_H
