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

#include "task_mgr/attribution_record.h"

#include "hilog_tag_wrapper.h"
#include "task_mgr/common.h"
#include "task_mgr/resource_control_proxy.h"

namespace OHOS {
namespace AppExecFwk {
namespace {

// Shared schedule-control helpers used by the derived records (their hard-coded schemes
// deliver control through the proxy per (taskId, pid) pair).

// Schedule-control policy hard-coded in code (no dynamic mapping-table loading; values are
// placeholders to be finalized by the perf/power review board without interface changes).
SchedulePolicy MakeSchedulePolicy(int32_t level)
{
    SchedulePolicy policy;
    switch (level) {
        case TASK_PRIORITY_HIGH:
            policy = {false, 2, 2, 2};  // high tier: lock-screen/background executable
            break;
        case TASK_PRIORITY_MEDIUM:
            policy = {false, 1, 1, 1};  // medium tier
            break;
        case TASK_PRIORITY_LOW:
            policy = {true, 0, 0, 0};   // low tier: low-power freeze group
            break;
        default:
            policy = {false, 1, 1, 1};
            break;
    }
    return policy;
}

// Applies schedule control with the hard-coded policy, then the effectiveness check.
void ApplyScheduleControl(AttributionRecord *record, const std::string &taskId, int32_t level)
{
    auto &proxy = ResourceControlProxy::GetInstance();
    ControlRequest req;
    req.taskId = taskId;
    req.managedPid = record->pid;
    req.managedType = record->tool;
    req.level = level;
    req.action = ControlAction::APPLY;
    req.policy = MakeSchedulePolicy(level);
    proxy.ApplyControl(req);
    // effectiveness check: query after applying, alert on an ineffective delivery (INV-3)
    int32_t verify = 0;
    if (proxy.QueryControlState(taskId, record->pid, verify) != 0 || verify != level) {
        TAG_LOGW(AAFwkTag::APPMGR,
            "schedule control ineffective alert, taskId: %{public}s pid: %{public}d expect: %{public}d actual: %{public}d",
            taskId.c_str(), record->pid, level, verify);
    }
}

// Revokes schedule control of the (taskId, pid) pair with a bounded retry before alerting.
void RevokeScheduleControl(AttributionRecord *record, const std::string &taskId)
{
    constexpr int32_t MAX_RETRY = 3;
    auto &proxy = ResourceControlProxy::GetInstance();
    int32_t ret = proxy.RevokeControl(taskId, record->pid);
    int32_t retry = 0;
    while (ret != 0 && retry < MAX_RETRY) {
        retry++;
        ret = proxy.RevokeControl(taskId, record->pid);
    }
    if (ret != 0) {
        TAG_LOGW(AAFwkTag::APPMGR,
            "schedule control revoke failed after %{public}d retries, task alert, taskId: %{public}s pid: %{public}d",
            MAX_RETRY, taskId.c_str(), record->pid);
    }
}

void NotifyTaskEndControl(const std::string &taskId, const std::vector<pid_t> &pids)
{
    TaskEndInfo info;
    info.taskId = taskId;
    info.pids = pids;
    ResourceControlProxy::GetInstance().NotifyTaskEnd(info);
}

}  // namespace

void InsightAttributionRecord::Schedule(const std::string &taskId, int32_t level)
{
    // INSIGHT schedule control (hard-coded scheme) delivered through the proxy.
    ApplyScheduleControl(this, taskId, level);
}

void InsightAttributionRecord::Unschedule(const std::string &taskId)
{
    RevokeScheduleControl(this, taskId);
}

void InsightAttributionRecord::NotifyTaskEnd(const std::string &taskId, const std::vector<pid_t> &pids)
{
    NotifyTaskEndControl(taskId, pids);
}

void CliAttributionRecord::Schedule(const std::string &taskId, int32_t level)
{
    // CLI schedule control (hard-coded scheme) delivered through the proxy.
    ApplyScheduleControl(this, taskId, level);
}

void CliAttributionRecord::Unschedule(const std::string &taskId)
{
    RevokeScheduleControl(this, taskId);
}

void CliAttributionRecord::NotifyTaskEnd(const std::string &taskId, const std::vector<pid_t> &pids)
{
    NotifyTaskEndControl(taskId, pids);
}

void A2aAttributionRecord::Schedule(const std::string &taskId, int32_t level)
{
    // A2A schedule control (hard-coded scheme) delivered through the proxy.
    ApplyScheduleControl(this, taskId, level);
}

void A2aAttributionRecord::Unschedule(const std::string &taskId)
{
    RevokeScheduleControl(this, taskId);
}

void A2aAttributionRecord::NotifyTaskEnd(const std::string &taskId, const std::vector<pid_t> &pids)
{
    NotifyTaskEndControl(taskId, pids);
}

void AbilityAttributionRecord::Schedule(const std::string &taskId, int32_t level)
{
    // UIAbility schedule control (hard-coded scheme) delivered through the proxy.
    ApplyScheduleControl(this, taskId, level);
}

void AbilityAttributionRecord::Unschedule(const std::string &taskId)
{
    RevokeScheduleControl(this, taskId);
}

void AbilityAttributionRecord::NotifyTaskEnd(const std::string &taskId, const std::vector<pid_t> &pids)
{
    NotifyTaskEndControl(taskId, pids);
}

std::shared_ptr<AttributionRecord> CreateAttributionRecord(AttributionTool tool)
{
    std::shared_ptr<AttributionRecord> record;
    switch (tool) {
        case AttributionTool::INSIGHT:
            record = std::make_shared<InsightAttributionRecord>();
            break;
        case AttributionTool::CLI:
            record = std::make_shared<CliAttributionRecord>();
            break;
        case AttributionTool::A2A:
            record = std::make_shared<A2aAttributionRecord>();
            break;
        case AttributionTool::ABILITY:
            record = std::make_shared<AbilityAttributionRecord>();
            break;
        default:
            record = std::make_shared<AttributionRecord>();
            break;
    }
    // Tag the created record with its tool so callers/tests can identify the derived type
    // without RTTI (the build uses -fno-rtti).
    record->tool = tool;
    return record;
}

const char *AttributionToolToString(AttributionTool tool)
{
    switch (tool) {
        case AttributionTool::INSIGHT:
            return "insight";
        case AttributionTool::CLI:
            return "cli";
        case AttributionTool::A2A:
            return "a2a";
        case AttributionTool::ABILITY:
            return "ability";
        default:
            return "unknown";
    }
}

AttributionTool AttributionToolFromString(const std::string &text)
{
    if (text == "insight") {
        return AttributionTool::INSIGHT;
    }
    if (text == "cli") {
        return AttributionTool::CLI;
    }
    if (text == "a2a") {
        return AttributionTool::A2A;
    }
    if (text == "ability") {
        return AttributionTool::ABILITY;
    }
    return AttributionTool::UNKNOWN;
}
}  // namespace AppExecFwk
}  // namespace OHOS
