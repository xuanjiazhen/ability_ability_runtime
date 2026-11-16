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

#ifndef OHOS_APP_EXECFWK_TASK_MGR_ATTRIBUTION_RECORD_H
#define OHOS_APP_EXECFWK_TASK_MGR_ATTRIBUTION_RECORD_H

#include <cstdint>
#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {

// Wire values of the registering tool; the enum is not exported across the service boundary,
// so CLI is mirrored as plain int32_t constants in climgr (task_attribution_manager.h) and the
// AMS skill executor (skill_execute_manager.cpp); the climgr copy is pinned by the static_assert
// in task_attribution_manager_test.cpp. Reordering breaks the mirrors silently: never reshuffle,
// only append.
enum class AttributionTool : int32_t {
    UNKNOWN = 0,
    INSIGHT = 1,
    CLI = 2,
    A2A = 3,
    ABILITY = 4,
};

// Server-assigned monotonic id of a managed attribution; handed out to the registering
// tool as the handle for later operations (e.g. unregister). 0 is the invalid sentinel.
using AttributionId = int64_t;

// Polymorphic schedule-control base: the task manager keeps the value-semantic record for its
// registry/dump, while the process schedule creates a per-tool derived object through
// CreateAttributionRecord() and dispatches schedule control via the base pointer without
// switching on the tool type (ADR-1). Each derived record hard-codes its own schedule-control
// scheme and delivers it through the ResourceControlProxy per (taskId, pid) pair (the process
// schedule itself only maintains the ownership index and forwards events). Phase-1 schedule
// control is an empty implementation with key-path logging; the real control is wired in later.
struct AttributionRecord {
    // Pid of the attributed target process, not the caller's.
    pid_t pid = -1;
    // Channel that created this attribution.
    AttributionTool tool = AttributionTool::UNKNOWN;
    // Registration time, assigned by the task manager.
    int64_t attributedTimeMillis = 0;

    virtual ~AttributionRecord() = default;

    // Schedule control (apply) for the (taskId, pid) pair, delivered via the proxy.
    // Base implementation is the UNKNOWN-tool no-op with logging. The base virtuals are
    // defined inline so the vtable is emitted wherever the value-semantic record is used
    // (other targets construct the base record without linking attribution_record.cpp).
    virtual void Schedule(const std::string &taskId, int32_t level)
    {
        TAG_LOGI(AAFwkTag::APPMGR,
            "schedule control(stub) tool: %{public}d taskId: %{public}s pid: %{public}d level: %{public}d",
            static_cast<int32_t>(tool), taskId.c_str(), pid, level);
    }
    // Schedule control (revoke) for the (taskId, pid) pair. Base no-op with logging.
    virtual void Unschedule(const std::string &taskId)
    {
        TAG_LOGI(AAFwkTag::APPMGR,
            "schedule control revoke(stub) tool: %{public}d taskId: %{public}s pid: %{public}d",
            static_cast<int32_t>(tool), taskId.c_str(), pid);
    }
    // Schedule control (task-level degrade) for the task. Base no-op with logging.
    virtual void NotifyTaskEnd(const std::string &taskId, const std::vector<pid_t> &pids)
    {
        TAG_LOGI(AAFwkTag::APPMGR,
            "schedule control task end(stub) tool: %{public}d taskId: %{public}s pids: %{public}zu",
            static_cast<int32_t>(tool), taskId.c_str(), pids.size());
    }
};

// Derived per-tool schedule-control records. Each tool hard-codes its own schedule-control
// scheme and delivers it through the ResourceControlProxy; phase-1 overrides only emit logs.
struct InsightAttributionRecord : public AttributionRecord {
    void Schedule(const std::string &taskId, int32_t level) override;
    void Unschedule(const std::string &taskId) override;
    void NotifyTaskEnd(const std::string &taskId, const std::vector<pid_t> &pids) override;
};

struct CliAttributionRecord : public AttributionRecord {
    void Schedule(const std::string &taskId, int32_t level) override;
    void Unschedule(const std::string &taskId) override;
    void NotifyTaskEnd(const std::string &taskId, const std::vector<pid_t> &pids) override;
};

struct A2aAttributionRecord : public AttributionRecord {
    void Schedule(const std::string &taskId, int32_t level) override;
    void Unschedule(const std::string &taskId) override;
    void NotifyTaskEnd(const std::string &taskId, const std::vector<pid_t> &pids) override;
};

struct AbilityAttributionRecord : public AttributionRecord {
    void Schedule(const std::string &taskId, int32_t level) override;
    void Unschedule(const std::string &taskId) override;
    void NotifyTaskEnd(const std::string &taskId, const std::vector<pid_t> &pids) override;
};

// Factory: creates the derived scheduling record for the tool; UNKNOWN yields the base record.
std::shared_ptr<AttributionRecord> CreateAttributionRecord(AttributionTool tool);

const char *AttributionToolToString(AttributionTool tool);
// Inverse of AttributionToolToString (dump command texts); returns UNKNOWN on no match.
AttributionTool AttributionToolFromString(const std::string &text);
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_APP_EXECFWK_TASK_MGR_ATTRIBUTION_RECORD_H
