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

#include "process_manager.h"

#include <cerrno>
#include <chrono>
#include <cstring>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>
#include <utility>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

ProcessManager &ProcessManager::GetInstance()
{
    static ProcessManager instance;
    return instance;
}

bool ProcessManager::CreatePipes(SessionRecord &record)
{
    // Create pipes for stdin, stdout and stderr
    if (pipe(record.stdinPipe) != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create stdin pipe: %{public}d", errno);
        return false;
    }
    if (pipe(record.stdoutPipe) != 0) {
        close(record.stdinPipe[0]);
        record.stdinPipe[0] = -1;
        close(record.stdinPipe[1]);
        record.stdinPipe[1] = -1;
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create stdout pipe: %{public}d", errno);
        return false;
    }
    if (pipe(record.stderrPipe) != 0) {
        close(record.stdinPipe[0]);
        record.stdinPipe[0] = -1;
        close(record.stdinPipe[1]);
        record.stdinPipe[1] = -1;
        close(record.stdoutPipe[0]);
        record.stdoutPipe[0] = -1;
        close(record.stdoutPipe[1]);
        record.stdoutPipe[1] = -1;
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create stderr pipe: %{public}d", errno);
        return false;
    }
    return true;
}

void ProcessManager::CloseAllPipes(SessionRecord &record)
{
    close(record.stdinPipe[0]);
    record.stdinPipe[0] = -1;
    close(record.stdinPipe[1]);
    record.stdinPipe[1] = -1;
    close(record.stdoutPipe[0]);
    record.stdoutPipe[0] = -1;
    close(record.stdoutPipe[1]);
    record.stdoutPipe[1] = -1;
    close(record.stderrPipe[0]);
    record.stderrPipe[0] = -1;
    close(record.stderrPipe[1]);
    record.stderrPipe[1] = -1;
}

void ProcessManager::ExecuteChildProcess(std::string &cmdLine,
                                         const std::string &sandboxConfig,
                                         const std::map<std::string, std::string> &args,
                                         const std::shared_ptr<SessionRecord> &record)
{
    if (prctl(PR_SET_PDEATHSIG, SIGTERM) != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Child: prctl PR_SET_PDEATHSIG failed: %{public}s", strerror(errno));
        _exit(EXIT_FAILURE);
    }
    if (getppid() == 1) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Child: parent process died");
        _exit(EXIT_FAILURE);
    }
    close(record->stdinPipe[1]);
    close(record->stdoutPipe[0]);
    close(record->stderrPipe[0]);

    dup2(record->stdinPipe[0], STDIN_FILENO);
    dup2(record->stdoutPipe[1], STDOUT_FILENO);
    dup2(record->stderrPipe[1], STDERR_FILENO);

    close(record->stdinPipe[0]);
    close(record->stdoutPipe[1]);
    close(record->stderrPipe[1]);

    std::string clawSandbox = "/system/bin/claw_sandbox";
    std::string configPrompt = "--config";
    std::string cmdPrompt = "--cmd";
    std::vector<char*> execArgs;
    execArgs.push_back(const_cast<char *>(clawSandbox.c_str()));
    execArgs.push_back(const_cast<char *>(configPrompt.c_str()));
    execArgs.push_back(const_cast<char *>(sandboxConfig.c_str()));
    execArgs.push_back(const_cast<char *>(cmdPrompt.c_str()));
    for (const auto &[key, value] : args) {
        cmdLine += " " + key + " " + value;
    }

    execArgs.push_back(const_cast<char *>(cmdLine.c_str()));
    execArgs.push_back(nullptr);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Before execvp");
    execvp(execArgs[0], execArgs.data());
    _exit(EXIT_FAILURE);
}

bool ProcessManager::SpawnProcess(std::string &cmdLine,
                                  const std::string &sandboxConfig,
                                  const std::map<std::string, std::string> &args,
                                  std::shared_ptr<SessionRecord> record)
{
    if (CreatePipes(*record) == false) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create pipes for tool %{public}s", record->toolName.c_str());
        return false;
    }

    pid_t pid = fork();
    if (pid < 0) {
        CloseAllPipes(*record);
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ExecTool failed: fork for tool '%{public}s'", record->toolName.c_str());
        return false;
    }

    if (pid == 0) {
        ExecuteChildProcess(sandboxConfig, args, record);
    }

    // Parent process: close write ends of pipes
    close(record->stdoutPipe[1]);
    close(record->stderrPipe[1]);

    // close read
    close(record->stdinPipe[0]);
    record->processId = pid;
    return true;
}

void ProcessManager::TerminateProcess(pid_t pid, int signal)
{
    if (pid > 0 && kill(pid, signal) != 0 && errno != ESRCH) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to kill process %{public}d: %{public}s",
            pid, strerror(errno));
    }
}

} // namespace CliTool
} // namespace OHOS