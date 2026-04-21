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

#include "cli_tool_exec_impl.h"

#include <ctime>
#include <sstream>
#include <unistd.h>
#include <sys/wait.h>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliTool {

int32_t CliToolExecutor::Execute(const std::string &cliName,
    const std::vector<std::string> &args,
    const ExecOptions &options,
    CliSessionInfo &session)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Execute CLI tool: %{public}s", cliName.c_str());

    // 1. Validate the CLI tool
    if (!ValidateCliTool(cliName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid CLI tool: %{public}s", cliName.c_str());
        return 2; // ERR_TOOL_NOT_EXIST
    }

    // 2. Check permission
    if (!CheckPermission(cliName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Permission denied for tool: %{public}s", cliName.c_str());
        return 5; // ERR_PERMISSION_DENIED
    }

    // 3. Initialize session
    session.sessionId = GenerateSessionId();
    session.startTime = std::time(nullptr) * 1000; // Convert to milliseconds
    session.isCompleted = false;

    // 4. Build command line
    std::string cmdLine = BuildCommandLine(cliName, args);
    TAG_LOGD(AAFwkTag::CLI_TOOL, "Command line: %{public}s", cmdLine.c_str());

    // 5. Execute the tool (fork + exec)
    int pipeStdout[2];
    int pipeStderr[2];

    if (options.enableStdout && pipe(pipeStdout) == -1) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create stdout pipe");
        return 3; // ERR_EXECUTION_FAILED
    }

    if (options.enableStderr && pipe(pipeStderr) == -1) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create stderr pipe");
        if (options.enableStdout) {
            close(pipeStdout[0]);
            close(pipeStdout[1]);
        }
        return 3; // ERR_EXECUTION_FAILED
    }

    pid_t pid = fork();
    if (pid == -1) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to fork process");
        if (options.enableStdout) {
            close(pipeStdout[0]);
            close(pipeStdout[1]);
        }
        if (options.enableStderr) {
            close(pipeStderr[0]);
            close(pipeStderr[1]);
        }
        return 3; // ERR_EXECUTION_FAILED
    }

    if (pid == 0) {
        // Child process
        if (options.enableStdout) {
            dup2(pipeStdout[1], STDOUT_FILENO);
            close(pipeStdout[0]);
            close(pipeStdout[1]);
        }
        if (options.enableStderr) {
            dup2(pipeStderr[1], STDERR_FILENO);
            close(pipeStderr[0]);
            close(pipeStderr[1]);
        }

        // Change working directory if specified
        if (!options.workingDir.empty()) {
            chdir(options.workingDir.c_str());
        }

        // Build argv array
        std::vector<char*> argv;
        argv.push_back(const_cast<char*>(cliName.c_str()));
        for (const auto& arg : args) {
            argv.push_back(const_cast<char*>(arg.c_str()));
        }
        argv.push_back(nullptr);

        // Execute
        execvp(cliName.c_str(), argv.data());
        exit(127); // If execvp returns, it failed
    } else {
        // Parent process
        if (options.enableStdout) {
            close(pipeStdout[1]);
        }
        if (options.enableStderr) {
            close(pipeStderr[1]);
        }

        // Read output
        char buffer[4096];
        ssize_t bytesRead;

        if (options.enableStdout) {
            while ((bytesRead = read(pipeStdout[0], buffer, sizeof(buffer))) > 0) {
                session.stdoutData.append(buffer, bytesRead);
            }
            close(pipeStdout[0]);
        }

        if (options.enableStderr) {
            while ((bytesRead = read(pipeStderr[0], buffer, sizeof(buffer))) > 0) {
                session.stderrData.append(buffer, bytesRead);
            }
            close(pipeStderr[0]);
        }

        // Wait for process to complete
        int status;
        waitpid(pid, &status, 0);

        session.exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
        session.endTime = std::time(nullptr) * 1000;
        session.isCompleted = true;

        TAG_LOGI(AAFwkTag::CLI_TOOL, "Tool completed: %{public}s, exit code: %{public}d",
            cliName.c_str(), session.exitCode);
    }

    return 0; // ERR_OK
}

std::string CliToolExecutor::BuildCommandLine(const std::string &cliName,
    const std::vector<std::string> &args)
{
    std::stringstream ss;
    ss << cliName;
    for (const auto &arg : args) {
        ss << " " << arg;
    }
    return ss.str();
}

bool CliToolExecutor::ValidateCliTool(const std::string &cliName)
{
    // Check if CLI tool name is not empty and contains valid characters
    if (cliName.empty() || cliName.length() > 256) {
        return false;
    }

    // Check if tool exists in PATH
    // For simplicity, just check if it starts with alphanumeric char
    if (!std::isalpha(cliName[0])) {
        return false;
    }

    return true;
}

bool CliToolExecutor::CheckPermission(const std::string &cliName)
{
    // TODO: Implement proper permission checking
    // For now, allow all tools
    return true;
}

std::string CliToolExecutor::GenerateSessionId()
{
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    std::stringstream ss;
    ss << "cli_session_" << timestamp << "_" << getpid();
    return ss.str();
}

} // namespace CliTool
} // namespace OHOS
