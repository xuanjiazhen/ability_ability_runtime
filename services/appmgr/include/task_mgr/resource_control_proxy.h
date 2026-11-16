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

#include <atomic>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <vector>

#include "task_mgr/attribution_record.h"
namespace OHOS {
namespace AppExecFwk {

// Action of a control delivery to the resource-schedule service.
enum class ControlAction : int32_t {
    APPLY = 0,
    REVOKE = 1,
};

// One schedule-control policy tier: the freeze/schedGroup/qos/speed combination of a level.
// Hard-coded in each derived record (no dynamic mapping-table loading).
struct SchedulePolicy {
    bool freeze = false;
    int32_t schedGroup = 0;
    int32_t qos = 0;
    int32_t speed = 0;
};

// Per (task, managed process) control payload, mirroring the cross-requirement event
// contract of 20260914692273. Control is applied/revoked per (taskId, managedPid) pair;
// level is the merged effective level (max over tasks).
struct ControlRequest {
    std::string taskId;          // task id of this (taskId, managedPid) control pair
    int32_t callerPid = -1;      // task creator pid
    int32_t callerUid = -1;      // task creator uid
    int32_t managedPid = -1;     // managed process pid
    int32_t managedUid = -1;     // managed process uid
    AttributionTool managedType = AttributionTool::UNKNOWN;  // INSIGHT/CLI/A2A/ABILITY
    int32_t level = 0;           // 100(LOW)/200(MEDIUM)/300(HIGH)
    ControlAction action = ControlAction::APPLY;
    SchedulePolicy policy;       // schedule-control policy, hard-coded by the derived record
};

// Task-level degrade payload: notify the resource-schedule service to clean task-level policy.
struct TaskEndInfo {
    std::string taskId;
    std::vector<pid_t> pids;
};

/**
 * @brief Sole exit to the resource-schedule service (ADR-2). Used by the per-tool derived
 *        AttributionRecord objects to deliver their schedule control. Phase-1 is a Stub that
 *        keeps an in-memory control registry keyed by (taskId, managedPid) and emits key-path
 *        logs/alerts with the probe switch off by default and no persistence; the real RSS
 *        Impl replaces the Stub once the RSS design is finalized, with zero impact on callers.
 */
class ResourceControlProxy {
public:
    static ResourceControlProxy &GetInstance();

    // Applies the control of a (taskId, managedPid) pair and verifies it took effect via
    // QueryControlState; an ineffective delivery records a task alert log. 0 on success.
    int32_t ApplyControl(const ControlRequest &req);
    // Revokes the control of a (taskId, managedPid) pair (process-level degrade).
    int32_t RevokeControl(const std::string &taskId, pid_t managedPid);
    // Effectiveness query: fills the registered level of the (taskId, managedPid) pair.
    // Returns 0 when registered, non-zero when the pair has no control entry.
    int32_t QueryControlState(const std::string &taskId, pid_t managedPid, int32_t &level);
    // Task-level degrade: notifies the resource-schedule service to clean task-level policy.
    void NotifyTaskEnd(const TaskEndInfo &info);

private:
    ResourceControlProxy() = default;
    ~ResourceControlProxy() = default;
    ResourceControlProxy(const ResourceControlProxy &) = delete;
    ResourceControlProxy &operator=(const ResourceControlProxy &) = delete;

    // Registry key of a (taskId, managedPid) control pair.
    static std::string MakeKey(const std::string &taskId, pid_t managedPid);

    mutable std::mutex registryMutex_;
    // In-memory control registry keyed by (taskId, managedPid) (no persistence).
    std::map<std::string, ControlRequest> controlRegistry_;
    // Probe switch (phase-1 default off): when on, emits extra probe logs.
    bool probeEnabled_ = false;

    // Fault-injection flags for unit tests (phase-1 Stub): force the corresponding delivery
    // to fail so the effectiveness check and the retry paths can be verified.
    bool failQueryForTest_ = false;
    bool failRevokeForTest_ = false;
    bool failNotifyForTest_ = false;
    // Counts RevokeControl invocations so tests can assert the retry budget (1 + 3 retries).
    std::atomic<int32_t> revokeCallsForTest_{0};
};

}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // OHOS_APP_EXECFWK_TASK_MGR_RESOURCE_CONTROL_PROXY_H
