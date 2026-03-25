/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "cli_manager_service.h"
#include "hilog_tag_wrapper.h"
#include "event_runner.h"
#include <chrono>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <errno.h>

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t CLI_MANAGER_SA_ID = 9999; // TODO: Use proper SA ID
constexpr const char* CLI_MANAGER_SERVICE_NAME = "CliManagerService";
constexpr int32_t DEFAULT_TOOL_TIMEOUT = 30000; // 30 seconds
constexpr int32_t ERR_OK = 0;
}

IMPLEMENT_DELAYED_SINGLETON(CliManagerService)

CliManagerService::CliManagerService(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService constructor called");
}

void CliManagerService::OnStart()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService OnStart called");

    if (serviceStarted_) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "CliManagerService already started");
        return;
    }

    if (!Init()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to init CliManagerService");
        return;
    }

    serviceStarted_ = true;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService started successfully");
}

void CliManagerService::OnStop()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService OnStop called");
    serviceStarted_ = false;
    tools_.clear();
}

bool CliManagerService::Init()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService Init called");

    // Create event handler
    auto runner = AppExecFwk::EventRunner::Create(CLI_MANAGER_SERVICE_NAME);
    if (!runner) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create EventRunner");
        return false;
    }

    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    if (!eventHandler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create EventHandler");
        return false;
    }

    // Load tool configurations
    LoadToolsFromConfig();

    // Publish the service
    bool res = Publish(this);
    if (!res) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to publish CliManagerService");
        return false;
    }

    return true;
}

void CliManagerService::LoadToolsFromConfig()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "LoadToolsFromConfig called");

    // TODO: Load from actual configuration file
    // For now, add some example tools

    ToolInfo tool1;
    tool1.name = "ls";
    tool1.version = "1.0.0";
    tool1.description = "List directory contents";
    tool1.permissions = {};
    tool1.timeout = 5000;
    tool1.supportedEvents = {"stdout", "stderr", "exit"};
    tools_.push_back(tool1);

    ToolInfo tool2;
    tool2.name = "cat";
    tool2.version = "1.0.0";
    tool2.description = "Concatenate and print files";
    tool2.permissions = {};
    tool2.timeout = 10000;
    tool2.supportedEvents = {"stdout", "stderr", "exit"};
    tools_.push_back(tool2);

    ToolInfo tool3;
    tool3.name = "grep";
    tool3.version = "1.0.0";
    tool3.description = "Search text patterns";
    tool3.permissions = {};
    tool3.timeout = 15000;
    tool3.supportedEvents = {"stdout", "stderr", "exit"};
    tools_.push_back(tool3);

    ToolInfo tool4;
    tool4.name = "hdc shell aa start";
    tool4.version = "1.0.0";
    tool4.description = "aa tools";
    tool4.permissions = {};
    tool4.timeout = 15000;
    tool4.supportedEvents = {"startAbility", "stderr", "exit"};
    tools_.push_back(tool4);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Loaded %{public}zu tools", tools_.size());
}

int32_t CliManagerService::QueryTools(std::vector<ToolInfo> &tools)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "QueryTools called");

    std::lock_guard<std::mutex> lock(mutex_);

    if (!serviceStarted_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Service not started");
        return -1;
    }

    tools = tools_;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Returned %{public}zu tools", tools.size());

    return ERR_OK;
}

int32_t CliManagerService::ExecTool(const std::string &name,
                                    const std::map<std::string, std::string> &args,
                                    const ExecOptions &options,
                                    ExecResult &result)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecTool called with tool: %{public}s", name.c_str());

    std::lock_guard<std::mutex> lock(mutex_);

    if (!serviceStarted_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Service not started");
        return -1;
    }

    // Validate tool exists
    auto it = std::find_if(tools_.begin(), tools_.end(),
        [&name](const ToolInfo &tool) { return tool.name == name; });

    if (it == tools_.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Tool not found: %{public}s", name.c_str());
        return -1;
    }

    auto startTime = std::chrono::steady_clock::now();

    // Create pipes for stdout and stderr
    int stdoutPipe[2];
    int stderrPipe[2];

    if (pipe(stdoutPipe) == -1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create stdout pipe: %{public}s", strerror(errno));
        result.taskId = std::to_string(std::hash<std::string>{}(name));
        result.exitCode = -1;
        result.stdOut = "";
        result.stdErr = "Failed to create stdout pipe";
        result.signal = 0;
        result.timedOut = false;
        result.executionTime = 0;
        return -1;
    }

    if (pipe(stderrPipe) == -1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create stderr pipe: %{public}s", strerror(errno));
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        result.taskId = std::to_string(std::hash<std::string>{}(name));
        result.exitCode = -1;
        result.stdOut = "";
        result.stdErr = "Failed to create stderr pipe";
        result.signal = 0;
        result.timedOut = false;
        result.executionTime = 0;
        return -1;
    }

    // Fork child process
    pid_t pid = fork();
    if (pid == -1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to fork: %{public}s", strerror(errno));
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        close(stderrPipe[0]);
        close(stderrPipe[1]);
        result.taskId = std::to_string(std::hash<std::string>{}(name));
        result.exitCode = -1;
        result.stdOut = "";
        result.stdErr = "Failed to fork child process";
        result.signal = 0;
        result.timedOut = false;
        result.executionTime = 0;
        return -1;
    }

    // Child process
    if (pid == 0) {
        // Close read ends of pipes
        close(stdoutPipe[0]);
        close(stderrPipe[0]);

        // Redirect stdout and stderr to pipes
        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);

        // Close write ends after dup2 (duplicated descriptors remain)
        close(stdoutPipe[1]);
        close(stderrPipe[1]);

        // Prepare arguments for execvp
        std::vector<char*> execArgs;
        execArgs.push_back(const_cast<char*>(name.c_str()));

        for (const auto &[key, value] : args) {
            execArgs.push_back(const_cast<char*>(key.c_str()));
            if (!value.empty()) {
                execArgs.push_back(const_cast<char*>(value.c_str()));
            }
        }
        execArgs.push_back(nullptr);

        TAG_LOGI(AAFwkTag::ABILITYMGR, "Child process executing: %{public}s", name.c_str());

        // Execute command
        execvp(name.c_str(), execArgs.data());

        // If execvp returns, it failed
        TAG_LOGE(AAFwkTag::ABILITYMGR, "execvp failed: %{public}s", strerror(errno));
        _exit(127); // Exit with code 127 indicating command not found
    }

    // Parent process
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Parent process waiting for child PID: %{public}d", pid);

    // Close write ends of pipes (parent only reads)
    close(stdoutPipe[1]);
    close(stderrPipe[1]);

    // Read output from pipes
    std::string stdoutOutput;
    std::string stderrOutput;
    char buffer[4096];
    ssize_t bytesRead;

    // Read stdout
    while ((bytesRead = read(stdoutPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        stdoutOutput += buffer;
    }

    // Read stderr
    while ((bytesRead = read(stderrPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        stderrOutput += buffer;
    }

    // Close read ends
    close(stdoutPipe[0]);
    close(stderrPipe[0]);

    // Wait for child process to finish
    int status;
    pid_t waitResult = waitpid(pid, &status, 0);

    if (waitResult == -1) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "waitpid failed: %{public}s", strerror(errno));
        result.taskId = std::to_string(std::hash<std::string>{}(name));
        result.exitCode = -1;
        result.stdOut = stdoutOutput;
        result.stdErr = stderrOutput;
        result.signal = 0;
        result.timedOut = false;
        result.executionTime = 0;
        return -1;
    }

    auto endTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);

    // Fill result
    result.taskId = std::to_string(std::hash<std::string>{}(name + std::to_string(startTime.time_since_epoch().count())));
    result.exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    result.stdOut = stdoutOutput;
    result.stdErr = stderrOutput;
    result.signal = WIFSIGNALED(status) ? WTERMSIG(status) : 0;
    result.timedOut = false;
    result.executionTime = duration.count();

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Command executed successfully, exitCode: %{public}d, signal: %{public}d, time: %{public}lldms",
        result.exitCode, result.signal, result.executionTime);

    return ERR_OK;
}

} // namespace AbilityRuntime
} // namespace OHOS
