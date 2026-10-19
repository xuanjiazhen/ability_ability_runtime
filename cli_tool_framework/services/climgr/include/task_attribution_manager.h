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

#ifndef OHOS_ABILITY_RUNTIME_TASK_ATTRIBUTION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_TASK_ATTRIBUTION_MANAGER_H

#include <cstdint>
#include <string>
#include <sys/types.h>

namespace OHOS {
namespace CliTool {

// Attribution tool value aligned with AppExecFwk::AttributionTool::CLI in the appmgr
// task_mgr (the enum lives in a service-internal header that is not exported to climgr,
// so the numeric contract is kept here; keep in sync).
constexpr int32_t ATTRIBUTION_TOOL_CLI = 2;

/**
 * @brief Consumer-side helper of the task attribution IPC face self-built by issue
 *        20260914692191 (ADR-4): AppMgrClient RegisterManagedProcess (149) /
 *        UnregisterManagedProcess (150) / RegisterInAppSkillProcess (151).
 *
 * Wraps the appmgr task-manager IPC used by climgr: process attach after fork, detach on
 * session teardown/process death, and the in-app-skill server-side target resolution.
 * Ownership and liveness are validated atomically by the server inside the attach call,
 * so these methods only interpret the result: any failure means "not attributed" plus a
 * security log, and the return value must never make the caller reject or roll back the
 * tool/command execution (ADR-2, non-blocking). Stateless singleton: the authoritative
 * taskId -> process mapping lives in appmgr; callerUid is read by the caller from its
 * session record (SessionRecord.callerUid) and forwarded, never threaded through setup
 * signatures.
 */
class TaskAttributionManager {
public:
    static TaskAttributionManager &GetInstance();

    TaskAttributionManager(const TaskAttributionManager &) = delete;
    TaskAttributionManager &operator=(const TaskAttributionManager &) = delete;
    TaskAttributionManager(TaskAttributionManager &&) = delete;
    TaskAttributionManager &operator=(TaskAttributionManager &&) = delete;

    /**
     * @brief Attributes a tool/command child process into the task. The server performs
     *        the ownership check (creator uid) and the liveness check atomically with the
     *        registration, so the result only tells whether the process is attributed.
     * @param taskId Indicates the attribution key carried by the options (code channel).
     * @param pid Indicates the pid returned by the successful fork (one-hop anchor).
     * @param callerUid Indicates the forensic uid read from the session record.
     * @return Returns ERR_OK when the process is attributed; INVALID_PARAMETERS_ERR on
     *         invalid parameters; ERR_TASK_NOT_FOUND when the task does not exist or is
     *         owned by another uid (not distinguished, to prevent enumeration);
     *         INNER_ERR when appmgr is unreachable. The caller logs and continues.
     */
    int32_t AttachProcess(const std::string &taskId, pid_t pid, int32_t callerUid);

    /**
     * @brief Removes a process from the managed set of the task; idempotent, the
     *        unregister object is the attributed pid (SessionRecord.attributedPid).
     * @param taskId Indicates the attribution key of the session.
     * @param pid Indicates the attributed pid; invalid (<= 0) means "no attribution"
     *            and is a silent no-op.
     * @param callerUid Indicates the forensic uid read from the session record.
     * @return Returns ERR_OK on success or no-op; ERR_TASK_NOT_FOUND when the task does
     *         not exist or is owned by another uid; INNER_ERR when appmgr is
     *         unreachable. The caller logs and continues.
     */
    int32_t DetachProcess(const std::string &taskId, pid_t pid, int32_t callerUid);

    /**
     * @brief In-app skill attribution: appmgr locates the target application main
     *        process by the reserved bundle name server-side and attributes it.
     * @param taskId Indicates the attribution key of the session.
     * @param bundleName Indicates the reserved parameter used only for process location.
     * @param outPid Outputs the attributed pid on success (for the later detach), or -1
     *               on any failure (no attribution object, no side effect).
     * @param callerUid Indicates the forensic uid read from the session record.
     * @return Returns ERR_OK on success; INNER_ERR when the target application is not
     *         running or its on-demand startup has not finished (known cold-start
     *         boundary, the skill execution itself is unaffected); ERR_TASK_NOT_FOUND
     *         when the task does not exist or is owned by another uid. The caller logs
     *         and continues.
     */
    int32_t AttachInAppSkillProcess(const std::string &taskId, const std::string &bundleName, pid_t &outPid,
        int32_t callerUid);

private:
    TaskAttributionManager() = default;
    ~TaskAttributionManager() = default;
};
} // namespace CliTool
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_TASK_ATTRIBUTION_MANAGER_H
