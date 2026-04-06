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
#include <random>
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
    cliToolManager_ = std::make_shared<CliToolManager>();
    if (cliToolManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to create CliToolManager");
        return;
    }

    // Create and start TimeoutManager
    timeoutManager_ = std::make_shared<TimeoutManager>();
    timeoutManager_->Start();

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
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: options.background=%{public}d, yieldMs=%{public}d, timeout=%{public}d",
        options.background, options.yieldMs, options.timeout);
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

    // Check if maximum session count reached
    if (sessionRecords_.size() >= MAX_SESSION_COUNT) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Too many concurrent sessions: %{public}zu (max: %{public}zu)",
            sessionRecords_.size(), MAX_SESSION_COUNT);
        return ERR_CLI_TOO_MANY_SESSIONS;
    }

    // Generate sessionId on service side
    std::string sessionId = GenerateSessionId();

    // Record start time for duration calculation
    auto startTimePoint = std::chrono::steady_clock::now();

    // Create session record
    auto record = CreateSessionRecord(sessionId, name, challenge);

    // Store in map
    sessionRecords_[sessionId] = record;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Stored session record: %{public}s", sessionId.c_str());

    // Create result object for session record
    record->result = std::make_shared<ExecResult>();
    if (record->result == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to allocate ExecResult");
        record->status = "failed";
        record->endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        // Build session info for error response
        session = BuildSessionInfo(record);
        return ERR_CLI_MEMORY_ALLOC_FAILED;
    }
    record->result->exitCode = -1;
    record->result->outputText = "";
    record->result->errorText = "";
    record->result->signalNumber = 0;
    record->result->timedOut = false;
    record->result->executionTime = 0;

    // Generate sandbox configuration before executing command
    std::string sandboxConfig = GenerateSandBoxConfig(challenge);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: sandbox config: %{public}s", sandboxConfig.c_str());

    // Create pipes for stdout and stderr
    int stdoutPipe[2];
    int stderrPipe[2];

    if (pipe(stdoutPipe) == -1) {
        std::string errMsg = "Failed to create stdout pipe: " + std::string(strerror(errno));
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s", errMsg.c_str());
        auto failEndTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        // Update session record
        record->status = "failed";
        record->endTime = failEndTime;
        record->result->errorText = errMsg;
        // Build session info for error response
        session = BuildSessionInfo(record);
        // Clean up session record
        sessionRecords_.erase(sessionId);
        return ERR_CLI_CREATE_STDOUT_PIPE_FAILED;
    }

    if (pipe(stderrPipe) == -1) {
        std::string errMsg = "Failed to create stderr pipe: " + std::string(strerror(errno));
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s", errMsg.c_str());
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        auto failEndTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        // Update session record
        record->status = "failed";
        record->endTime = failEndTime;
        record->result->errorText = errMsg;
        // Build session info for error response
        session = BuildSessionInfo(record);
        // Clean up session record
        sessionRecords_.erase(sessionId);
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
        auto failEndTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        // Update session record
        record->status = "failed";
        record->endTime = failEndTime;
        record->result->errorText = errMsg;
        // Build session info for error response
        session = BuildSessionInfo(record);
        // Clean up session record
        sessionRecords_.erase(sessionId);
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
        std::vector<char *> execArgs;
        std::string claw_sandbox = "/system/bin/claw_sandbox";
        std::string config_prompt = "--config";
        std::string cmd_prompt = "--cmd";
        std::string cmdline = name;
        execArgs.push_back(const_cast<char*>(claw_sandbox.c_str()));
        execArgs.push_back(const_cast<char*>(config_prompt.c_str()));
        execArgs.push_back(const_cast<char*>(sandboxConfig.c_str()));
        execArgs.push_back(const_cast<char*>(cmd_prompt.c_str()));

        for (const auto &[key, value] : args) {
            cmdline += " " + key + " " + value;
        }
        execArgs.push_back(const_cast<char*>(cmdline.c_str()));
        execArgs.push_back(nullptr);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService[CHILD]: Command will be: %{public}s", cmdline.c_str());

        for (size_t i = 0; i < execArgs.size(); i++) {
            if (execArgs[i] != nullptr) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService[CHILD]: execArgs[%{public}zu]='%{public}s'",
                    i, execArgs[i]);
            } else {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService[CHILD]: execArgs[%{public}zu]=NULL",
                    i);
            }
        }

        // Execute command
        execvp(execArgs[0], execArgs.data());

        // If execvp returns, it failed
        TAG_LOGE(AAFwkTag::ABILITYMGR, "execvp failed: %{public}s", strerror(errno));
        _exit(127); // Exit with code 127 indicating command not found
    }

    // Parent process
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Parent process waiting for child PID: %{public}d", pid);

    // Update session record with process ID
    record->processId = pid;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Updated session %{public}s with pid=%{public}d", sessionId.c_str(), pid);

    // Create timeout context and add to timeout manager
    auto timeoutContext = std::make_shared<TimeoutContext>();
    timeoutContext->sessionId = sessionId;
    timeoutContext->processId = pid;

    // Resolve timeout with priority
    uint32_t timeoutMs = ResolveTimeout(options, name);
    uint32_t actualTimeoutMs = timeoutManager_->AddTimeoutTask(
        sessionId, pid, timeoutContext, timeoutMs);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Added timeout monitoring: %{public}s, timeout=%{public}u ms",
        sessionId.c_str(), actualTimeoutMs);

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

    // Cancel timeout monitoring (task completed or timeout occurred)
    timeoutManager_->CancelTimeoutTask(sessionId);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Cancelled timeout monitoring for %{public}s", sessionId.c_str());

    if (waitResult == -1) {
        std::string errMsg = "waitpid failed: " + std::string(strerror(errno));
        TAG_LOGE(AAFwkTag::ABILITYMGR, "%{public}s", errMsg.c_str());
        auto failEndTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        // Update session record
        record->status = "failed";
        record->endTime = failEndTime;
        record->result->errorText = errMsg;
        // Build session info for error response
        session = BuildSessionInfo(record);
        // Clean up session record
        sessionRecords_.erase(sessionId);
        return ERR_CLI_WAITPID_FAILED;
    }

    auto endTimePoint = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTimePoint - startTimePoint);
    int64_t endTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTimePoint.time_since_epoch()).count();

    // Update session record with execution result
    record->result->exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    record->result->outputText = stdoutOutput;
    record->result->errorText = stderrOutput;
    record->result->signalNumber = WIFSIGNALED(status) ? WTERMSIG(status) : 0;
    record->result->executionTime = duration.count();

    // Check if task timed out
    if (timeoutContext->isTimedOut.load()) {
        record->result->timedOut = true;
        record->result->errorText = "Execution timed out";
        record->status = "failed";
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Session %{public}s timed out", sessionId.c_str());
    } else {
        record->result->timedOut = false;
        record->status = (record->result->exitCode == 0) ? "completed" : "failed";
    }
    record->endTime = endTimeMs;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Updated session %{public}s: status=%{public}s", sessionId.c_str(), record->status.c_str());

    // Build session info for client response
    session = BuildSessionInfo(record);

    // Clean up session record after task completion
    sessionRecords_.erase(sessionId);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Removed session record: %{public}s", sessionId.c_str());

    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: command execution completed");
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: session.sessionId='%{public}s'", session.sessionId.c_str());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: session.status='%{public}s'", session.status.c_str());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: session.result.exitCode=%{public}d", session.result->exitCode);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: session.result.outputText.len=%{public}zu", session.result->outputText.length());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: session.result.errorText.len=%{public}zu", session.result->errorText.length());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: session.result.executionTime=%{public}ldms", session.result->executionTime);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService: returning ERR_OK");
    return ERR_OK;
}

std::string CliManagerService::GenerateSessionId()
{
    // Generate unique session ID using nanosecond time (similar to startTime pattern)
    auto now = std::chrono::system_clock::now();
    auto nanos = std::chrono::duration_cast<std::chrono::nanoseconds>(
        now.time_since_epoch()).count();

    std::string sessionId = "sess_" + std::to_string(nanos);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Generated sessionId: %{public}s", sessionId.c_str());
    return sessionId;
}

std::shared_ptr<SessionRecord> CliManagerService::CreateSessionRecord(
    const std::string& sessionId,
    const std::string& toolName,
    const std::string& challenge)
{
    // Get timestamp
    auto startTimePoint = std::chrono::system_clock::now();
    int64_t startTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        startTimePoint.time_since_epoch()).count();

    // Get calling information
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    int32_t callingPid = IPCSkeleton::GetCallingPid();
    uint64_t callingTokenId = IPCSkeleton::GetCallingFullTokenID();

    // Create SessionRecord
    auto record = std::make_shared<SessionRecord>();
    record->sessionId = sessionId;
    record->toolName = toolName;
    record->status = "running";
    record->startTime = startTimeMs;
    record->endTime = 0;
    record->processId = -1;
    record->callingUid = callingUid;
    record->callingPid = callingPid;
    record->callingTokenId = callingTokenId;
    record->challenge = challenge;
    record->result = nullptr;

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Created session record: %{public}s for tool '%{public}s'",
        sessionId.c_str(), toolName.c_str());

    return record;
}

SessionInfo CliManagerService::BuildSessionInfo(const std::shared_ptr<SessionRecord>& record)
{
    SessionInfo session;
    session.sessionId = record->sessionId;
    session.toolName = record->toolName;
    session.status = record->status;
    session.startTime = record->startTime;
    session.endTime = record->endTime;
    session.result = record->result;
    return session;
}

std::string CliManagerService::GenerateSandBoxConfig(const std::string& challenge)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerService::GenerateSandBoxConfig called");

    // Get calling information via IPC
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    uint64_t callingTokenId = IPCSkeleton::GetCallingFullTokenID();
    int32_t callingPid = IPCSkeleton::GetCallingPid();

    TAG_LOGI(AAFwkTag::ABILITYMGR, "GenerateSandBoxConfig: callingUid=%{public}d, callingTokenId=%{public}lu, callingPid=%{public}d",
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

uint32_t CliManagerService::ResolveTimeout(const ExecOptions& options, const std::string& toolName)
{
    // Priority 1: User specified timeout in options (in seconds, 0 means no timeout)
    if (options.timeout > 0) {
        uint32_t timeoutMs = static_cast<uint32_t>(options.timeout) * 1000;
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Using user specified timeout: %{public}d sec (%{public}u ms)",
            options.timeout, timeoutMs);
        return timeoutMs;
    }

    // Priority 2: Tool's default timeout from CliToolManager (in seconds)
    if (cliToolManager_ != nullptr && !toolName.empty()) {
        int32_t toolTimeout = 0;
        if (cliToolManager_->GetToolTimeout(toolName, toolTimeout) == 0 && toolTimeout > 0) {
            uint32_t timeoutMs = static_cast<uint32_t>(toolTimeout) * 1000;
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Using tool default timeout for '%{public}s': %{public}d sec (%{public}u ms)",
                toolName.c_str(), toolTimeout, timeoutMs);
            return timeoutMs;
        }
    }

    // Priority 3: Default timeout (30 minutes)
    constexpr int32_t DEFAULT_TIMEOUT_SEC = 30 * 60;
    uint32_t timeoutMs = DEFAULT_TIMEOUT_SEC * 1000;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Using default timeout: %{public}d sec (%{public}u ms)",
        DEFAULT_TIMEOUT_SEC, timeoutMs);
    return timeoutMs;
}

} // namespace AbilityRuntime
} // namespace OHOS
