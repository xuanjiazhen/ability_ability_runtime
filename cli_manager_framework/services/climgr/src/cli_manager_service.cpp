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
#include "cli_tool_manager.h"
#include "cli_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include <chrono>
#include <memory>
#include <sstream>
#include <nlohmann/json.hpp>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <cstring>
#include <cerrno>
#include "ipc_skeleton.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t CLI_MANAGER_SA_ID = 8899; // TODO: Use proper SA ID
constexpr int32_t ERR_OK = 0;
}

sptr<CliManagerService> CliManagerService::instance_ = nullptr;
std::mutex CliManagerService::instanceMutex_;
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(CliManagerService::GetInstance());

sptr<CliManagerService> CliManagerService::GetInstance()
{
    std::lock_guard<std::mutex> lock(instanceMutex_);
    if (instance_ != nullptr) {
        return instance_;
    }
    instance_ = new (std::nothrow) CliManagerService();
    return instance_;
}

CliManagerService::CliManagerService() : SystemAbility(CLI_MANAGER_SA_ID, true)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService constructor called");
}

CliManagerService::~CliManagerService()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService destructor called");
}

void CliManagerService::Init()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService Init called");

    // Create CliToolManager instance (construction does initialization)
    cliToolManager_ = std::make_unique<CliToolManager>();
    if (cliToolManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create CliToolManager");
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService Init completed");
}

void CliManagerService::OnStart()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService OnStart called");

    if (state_ == ServiceRunningState::STATE_RUNNING) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService already started");
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService starting");
    Init();

    if (!Publish(CliManagerService::GetInstance())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to publish CliManagerService");
        return;
    }

    state_ = ServiceRunningState::STATE_RUNNING;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService started successfully");
}

void CliManagerService::OnStop()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService OnStop called");
    state_ = ServiceRunningState::STATE_NOT_START;
    // cliToolManager_ will be automatically destroyed when service is destroyed
}

int32_t CliManagerService::QueryTools(std::vector<ToolInfo> &tools)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService::QueryTools called");

    std::lock_guard<std::mutex> lock(mutex_);

    if (cliToolManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CliToolManager is not initialized");
        return ERR_CLI_MANAGER_NOT_INITIALIZED;
    }

    int32_t ret = cliToolManager_->QueryTools(tools);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to query tools: %{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Returned %{public}zu tools to client", tools.size());
    return ERR_OK;
}

int32_t CliManagerService::ExecTool(const std::string &name,
                                    const std::vector<std::pair<std::string, std::string>> &args,
                                    const ExecOptions &options,
                                    const std::string &challenge,
                                    SessionInfo &session)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService ExecTool called");
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: received name='%{public}s'", name.c_str());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: received args.count=%{public}zu", args.size());
    for (const auto &[key, value] : args) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService:   arg[%{public}s]='%{public}s'",
            key.c_str(), value.c_str());
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: options.background=%{public}d, yieldMs=%{public}d, timeoutMs=%{public}d",
        options.background, options.yieldMs, options.timeoutMs);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: received challenge='%{public}s'", challenge.c_str());

    std::lock_guard<std::mutex> lock(mutex_);

    // Validate tool exists BEFORE filling session info
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: validating tool exists");
    if (cliToolManager_ == nullptr || !cliToolManager_->ValidateTool(name)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Tool not found or invalid: %{public}s", name.c_str());
        // Return error without filling session info
        return ERR_CLI_TOOL_NOT_FOUND;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: tool '%{public}s' validated", name.c_str());

    auto startTimePoint = std::chrono::steady_clock::now();
    int64_t startTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        startTimePoint.time_since_epoch()).count();

    // Fill session info before executing command
    session.sessionId = std::to_string(std::hash<std::string>{}(
        name + std::to_string(startTimeMs)));
    session.toolName = name;
    session.status = "running";
    session.startTime = startTimeMs;
    session.endTime = 0;

    // 提前创建 result，以便在所有失败场景中都能记录错误信息
    session.result = std::make_shared<ExecResult>();
    if (session.result == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to allocate ExecResult");
        session.status = "failed";
        return ERR_CLI_MEMORY_ALLOC_FAILED;
    }
    session.result->taskId = session.sessionId;
    session.result->exitCode = -1;
    session.result->stdOut = "";
    session.result->stdErr = "";
    session.result->signal = 0;
    session.result->timedOut = false;
    session.result->executionTime = 0;

    // Generate sandbox configuration before executing command
    std::string sandboxConfig = GenerateSandBoxConfig(challenge);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: sandbox config: %{public}s", sandboxConfig.c_str());

    // Create pipes for stdout and stderr
    int stdoutPipe[2];
    int stderrPipe[2];

    if (pipe(stdoutPipe) == -1) {
        std::string errMsg = "Failed to create stdout pipe: " + std::string(strerror(errno));
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s", errMsg.c_str());
        session.status = "failed";
        session.endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        session.result->stdErr = errMsg;
        return ERR_CLI_CREATE_STDOUT_PIPE_FAILED;
    }

    if (pipe(stderrPipe) == -1) {
        std::string errMsg = "Failed to create stderr pipe: " + std::string(strerror(errno));
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s", errMsg.c_str());
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        session.status = "failed";
        session.endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        session.result->stdErr = errMsg;
        return ERR_CLI_CREATE_STDERR_PIPE_FAILED;
    }

    // Fork child process
    pid_t pid = fork();
    if (pid == -1) {
        std::string errMsg = "Failed to fork: " + std::string(strerror(errno));
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s", errMsg.c_str());
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        close(stderrPipe[0]);
        close(stderrPipe[1]);
        session.status = "failed";
        session.endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        session.result->stdErr = errMsg;
        return ERR_CLI_FORK_FAILED;
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

        TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService[CHILD]: building execArgs, current count=%{public}zu",
            execArgs.size() - 1);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService[CHILD]:   arg[0]='%{public}s'", name.c_str());

        for (const auto &[key, value] : args) {
            execArgs.push_back(const_cast<char*>(key.c_str()));
            TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService[CHILD]:   arg[%{public}zu]='%{public}s'",
                execArgs.size() - 1, key.c_str());
            if (!value.empty()) {
                execArgs.push_back(const_cast<char*>(value.c_str()));
                TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService[CHILD]:   arg[%{public}zu]='%{public}s'",
                    execArgs.size() - 1, value.c_str());
            }
        }
        execArgs.push_back(nullptr);

        // Print final execArgs array
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService[CHILD]: ========== Final execArgs ==========");
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService[CHILD]: execArgs.size()=%{public}zu", execArgs.size());
        for (size_t i = 0; i < execArgs.size(); i++) {
            if (execArgs[i] != nullptr) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService[CHILD]: execArgs[%{public}zu]='%{public}s'",
                    i, execArgs[i]);
            } else {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService[CHILD]: execArgs[%{public}zu]=NULL",
                    i);
            }
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService[CHILD]: Calling execvp('%{public}s')", name.c_str());
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService[CHILD]: Command will be: %{public}s", name.c_str());

        // Execute command directly (no sandbox wrapper)
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
        std::string errMsg = "waitpid failed: " + std::string(strerror(errno));
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s", errMsg.c_str());
        session.status = "failed";
        session.endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        session.result->stdErr = errMsg;
        return ERR_CLI_WAITPID_FAILED;
    }

    auto endTimePoint = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTimePoint - startTimePoint);
    int64_t endTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTimePoint.time_since_epoch()).count();

    // 填充 result（已在函数开始时创建）
    session.result->exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    session.result->stdOut = stdoutOutput;
    session.result->stdErr = stderrOutput;
    session.result->signal = WIFSIGNALED(status) ? WTERMSIG(status) : 0;
    session.result->timedOut = false;
    session.result->executionTime = duration.count();

    // Update session status
    session.status = (session.result->exitCode == 0) ? "completed" : "failed";
    session.endTime = endTimeMs;

    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: command execution completed");
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: session.sessionId='%{public}s'", session.sessionId.c_str());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: session.status='%{public}s'", session.status.c_str());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: session.result.exitCode=%{public}d", session.result->exitCode);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: session.result.stdOut.len=%{public}zu", session.result->stdOut.length());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: session.result.stdErr.len=%{public}zu", session.result->stdErr.length());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: session.result.executionTime=%{public}ldms", session.result->executionTime);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: returning ERR_OK");
    return ERR_OK;
}

std::string CliManagerService::GenerateSandBoxConfig(const std::string& challenge)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService::GenerateSandBoxConfig called");

    // Get calling information via IPC
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    uint32_t callingTokenId = IPCSkeleton::GetCallingTokenID();
    int32_t callingPid = IPCSkeleton::GetCallingPid();

    TAG_LOGI(AAFwkTag::ABILITYMGR, "GenerateSandBoxConfig: callingUid=%{public}d, callingTokenId=%{public}u, callingPid=%{public}d",
        callingUid, callingTokenId, callingPid);

    // Get GID and groups
    // Note: In OpenHarmony, app's GID usually equals its UID
    int32_t gid = callingUid;  // GID = UID for app sandbox

    // Get supplementary groups (if any)
    // Currently no BundleManager API to query groups, use empty array
    std::vector<int32_t> groupsVec;  // Empty groups list
    groupsVec.push_back(callingUid);  // Add calling UID as a supplementary group

    // Build JSON object
    nlohmann::json config;
    config["uid"] = callingUid;
    config["gid"] = gid;
    config["groups"] = groupsVec;
    config["callerTokenId"] = callingTokenId;
    config["challenge"] = challenge;

    // Get bundle name from UID (placeholder - would need BundleManager)
    config["bundleName"] = "com.huawei.hmos.vassistant";

    // Get permissions (placeholder - would need AccessTokenKit)
    config["permissions"] = std::vector<std::string>{};

    std::string result = config.dump();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GenerateSandBoxConfig: result=%{public}s", result.c_str());

    return result;
}

} // namespace AbilityRuntime
} // namespace OHOS
