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

#include "tool_pid_resolver.h"

#include <string>

#include "cli_tool_manager_service.h"
#include "ffrt.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr int64_t MS_TO_US = 1000; // ffrt task_attr::delay takes microseconds
} // namespace

ToolPidResolver &ToolPidResolver::GetInstance()
{
    static ToolPidResolver instance;
    return instance;
}

void ToolPidResolver::ResetForTest()
{
    // Fence first: a scheduled task that captured the old epoch aborts on the
    // mismatch, so a delayed registration can never run against a reset boundary.
    epoch_.fetch_add(1);
    scheduler_ = nullptr;
}

void ToolPidResolver::Resolve(const std::shared_ptr<SessionRecord> &record)
{
    if (record == nullptr || record->taskId.empty()) {
        return; // unattributed sessions pay zero cost
    }
    // The reaper invalidates processId at any moment; this atomic read is lock-free by contract.
    pid_t anchor = record->processId.load();
    if (anchor <= 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "ToolPidResolver skipped: invalid host pid, sessionId=%{public}s, taskId=%{public}s",
            record->sessionId.c_str(), record->taskId.c_str());
        return;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "ToolPidResolver delayed attach scheduled, sessionId=%{public}s, hostPid=%{public}d, "
        "taskId=%{public}s, delayMs=%{public}lld", record->sessionId.c_str(), anchor,
        record->taskId.c_str(), static_cast<long long>(RESOLVE_DELAY_MS));
    uint64_t epoch = epoch_.load();
    // Weak capture keeps the session table the record's sole owner: once teardown has
    // destroyed the record, the delayed task ends here instead of attaching a dead
    // session. The anchor snapshot and id copies keep the give-up line observable
    // after the record is gone.
    auto fire = [weakRecord = std::weak_ptr<SessionRecord>(record), sessionId = record->sessionId,
                    taskId = record->taskId, anchor, this, epoch]() {
        if (epoch_.load() != epoch) {
            return; // fenced by ResetForTest: drop the stale task silently
        }
        std::shared_ptr<SessionRecord> liveRecord = weakRecord.lock();
        if (liveRecord == nullptr) {
            TAG_LOGW(AAFwkTag::CLI_TOOL,
                "ToolPidResolver give up: session ended before delayed attach, no tool attribution, "
                "lastHostPid=%{public}d, sessionId=%{public}s, taskId=%{public}s",
                anchor, sessionId.c_str(), taskId.c_str());
            return;
        }
        TAG_LOGI(AAFwkTag::CLI_TOOL,
            "ToolPidResolver delayed attach fired, sessionId=%{public}s, hostPid=%{public}d, "
            "taskId=%{public}s", sessionId.c_str(), anchor, taskId.c_str());
        // AttachTaskAttribution carries the HOST pid; the appmgr wrapper re-anchors it
        // onto the live tool child (or falls back to this pid — the pre-discovery
        // behavior). The attach state machine (PENDING/relay/compensation) owns the
        // exactly-once semantics for the IPC window after this point.
        auto service = CliToolManagerService::GetInstance();
        if (service == nullptr) {
            TAG_LOGW(AAFwkTag::CLI_TOOL,
                "ToolPidResolver attach skipped: service unavailable, sessionId=%{public}s",
                sessionId.c_str());
            return;
        }
        service->AttachTaskAttribution(liveRecord, anchor);
    };
    if (scheduler_) {
        scheduler_(RESOLVE_DELAY_MS, fire);
        return;
    }
    // Single delayed submit, fire-and-forget: failure semantics are owned by the
    // attach state machine (FAILED terminal state, idempotent detach), not by pacing.
    ffrt::submit(std::move(fire), ffrt::task_attr().delay(static_cast<uint64_t>(RESOLVE_DELAY_MS) * MS_TO_US));
}
} // namespace CliTool
} // namespace OHOS
