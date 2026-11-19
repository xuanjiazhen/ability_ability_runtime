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

#ifndef OHOS_ABILITY_RUNTIME_PROCESS_MANAGER_H
#define OHOS_ABILITY_RUNTIME_PROCESS_MANAGER_H

#include <csignal>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "session_record.h"

namespace OHOS {
namespace CliTool {
class ExecCmdParam;
class ExecToolParam;
class ToolInfo;

class ProcessManager {
public:
    static ProcessManager &GetInstance();

    ProcessManager(const ProcessManager &) = delete;
    ProcessManager &operator=(const ProcessManager &) = delete;
    ProcessManager(ProcessManager &&) = delete;
    ProcessManager &operator=(ProcessManager &&) = delete;

    int32_t CreateChildProcess(const ExecToolParam &param, const std::string &sandboxConfig,
        const ToolInfo &toolInfo, std::shared_ptr<SessionRecord> record) const;

    int32_t CreateShellProcess(const ExecCmdParam &param, const std::string &sandboxConfig,
        std::shared_ptr<SessionRecord> record) const;

    bool Killpg(pid_t pid) const;

    /**
     * @brief Kill the session's whole process tree, tolerating the sandbox pid-namespace fork.
     *
     * claw_sandbox (processId) may have already forked the real command process
     * (commandPid) after unshare(CLONE_NEWPID). When commandPid is resolved, its process
     * group covers the command and all its descendants (seccomp blocks setpgid/setsid in the
     * sandbox, so nothing escapes) and the sandbox monitor exits on its own once the command
     * is gone. When unresolved, live children of the sandbox are group-killed and directly
     * killed (pre-setpgid window), then the sandbox itself is killed both ways — in the
     * pid-ns form the sandbox never leads a group, so Killpg alone would miss it.
     *
     * @param processId Indicates the claw_sandbox anchor pid (ignored when <= 0).
     * @param commandPid Indicates the discovered command pid, or -1 when unresolved.
     * @return Returns true when at least one kill was delivered.
     */
    bool KillSessionProcess(pid_t processId, pid_t commandPid) const;

private:
    ProcessManager() = default;
    ~ProcessManager() = default;

    bool CreatePipes(SessionRecord &record) const;
    void CloseAllPipes(SessionRecord &record) const;
    void CloseNonStdFds() const;
    void SetupChildPipesAndExec(const SessionRecord &record,
        std::vector<char *> &execArgs, std::vector<char *> &envp) const;
    // Direct SIGKILL to one process (no process-group semantics); returns false on ESRCH/EPERM.
    bool Kill(pid_t pid) const;

    static const char *clawSandboxPath_;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PROCESS_MANAGER_H
