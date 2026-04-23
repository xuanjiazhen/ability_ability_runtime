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

#include "cli_tool_manager_service.h"

#include "cli_error_code.h"
#include "event_dispatcher.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "process_manager.h"
#include "tool_util.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t MAX_CONCURRENT_SESSIONS = 8;

constexpr const char* PERMISSION_EXEC_CLI_TOOL = "ohos.permission.EXEC_CLI_TOOL";
} // namespace

std::mutex g_mutex;
sptr<CliToolManagerService> CliToolManagerService::instance_ = nullptr;
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(CliToolManagerService::GetInstance().GetRefPtr());

sptr<CliToolManagerService> CliToolManagerService::GetInstance()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (instance_ != nullptr) {
        return instance_;
    }
    instance_ = new (std::nothrow) CliToolManagerService();
    return instance_;
}

int32_t CliToolManagerService::RegisterScheduler(const sptr<IRemoteObject> &scheduler)
{
    if (EventDispatcher::GetInstance().RegisterScheduler(IPCSkeleton::GetCallingPid(), scheduler)) {
        return ERR_OK;
    }
    return ERR_NO_INIT;
}

int32_t CliToolManagerService::UnregisterScheduler()
{
    EventDispatcher::GetInstance().UnregisterScheduler(IPCSkeleton::GetCallingPid());
    return ERR_OK;
}

void CliToolManagerService::HandleProcessTimeout(const std::string &sessionId)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleProcessTimeout skipped: sessionId=%{public}s not found", sessionId.c_str());
        return;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "HandleProcessTimeout: sessionId=%{public}s", sessionId.c_str());
    record->SetTimedOut(true);
    EventDispatcher::GetInstance().DispatchErrorEvent(sessionId, "session timed out");
}

void CliToolManagerService::HandleProcessYieldTimeout(const std::string &sessionId)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleProcessYieldTimeout skipped: sessionId=%{public}s not found", sessionId.c_str());
        return;
    }
    auto oldBackground = record->SetBackground(true);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "HandleProcessYieldTimeout: sessionId=%{public}s, background=%{public}d",
        sessionId.c_str(), oldBackground);
    if (oldBackground == false) {
        CliSessionInfo session;
        record->BuildSessionInfo(session);
        EventDispatcher::GetInstance().DispatchExecToolReplyEvent(
            record->callerPid, record->eventId, ERR_OK, session);
    }
}

void CliToolManagerService::HandleProcessExit(const std::string &sessionId, int status)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleProcessExit skipped: sessionId=%{public}s not found, status=%{public}d",
            sessionId.c_str(), status);
        return;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "HandleProcessExit: sessionId=%{public}s, status=%{public}d",
        sessionId.c_str(), status);
    record->SetTerminalResult(status);
    if (record->OutputDrained()) {
        FinalizeBackgroundSession(record);
    }
}

void CliToolManagerService::HandleOutputClosed(const std::string &sessionId, bool isStdout)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleOutputClosed skipped: sessionId=%{public}s not found, isStdout=%{public}d",
            sessionId.c_str(), isStdout);
        return;
    }
    if (isStdout) {
        record->MarkStdoutClosed();
    } else {
        record->MarkStderrClosed();
    }
    if (record->HasProcessExited() && record->OutputDrained()) {
        FinalizeBackgroundSession(record);
    }
}

void CliToolManagerService::HandleOutputDrained(const std::string &sessionId)
{
    auto record = GetSessionRecord(sessionId);
    if (record != nullptr && record->HasProcessExited()) {
        FinalizeBackgroundSession(record);
    }
}

void CliToolManagerService::FinalizeBackgroundSession(const std::shared_ptr<SessionRecord> &record)
{
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "FinalizeBackgroundSession skipped: record is null");
        return;
    }
    const auto &sessionId = record->sessionId;
    if (!record->BeginCleanup()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "FinalizeBackgroundSession skipped: cleanup already started for sessionId=%{public}s",
            sessionId.c_str());
        return;
    }

    auto oldBackground = record->SetBackground(true);
    if (oldBackground == false) {
        CliSessionInfo session;
        record->BuildSessionInfo(session);
        EventDispatcher::GetInstance().DispatchExecToolReplyEvent(
            record->callerPid, record->eventId, ERR_OK, session);
    }

    const int status = record->GetTerminalStatus();
    const int32_t exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Finalizing background session: sessionId=%{public}s, exitCode=%{public}d",
        sessionId.c_str(), exitCode);

    if (timeoutManager_) {
        timeoutManager_->CancelTimeoutTask(sessionId);
    }
    if (ioMonitor_) {
        ioMonitor_->UnregisterSession(sessionId);
    }
    processMonitor_.UnregisterSession(sessionId);

    EventDispatcher::GetInstance().DispatchExitEvent(sessionId, exitCode);
    EventDispatcher::GetInstance().ClearSessionSubscribers(sessionId);
    RemoveSessionRecord(sessionId);
}

void CliToolManagerService::Init()
{
    if (initialized_) {
        return;
    }

    timeoutManager_ = std::make_shared<TimeoutManager>();
    if (timeoutManager_ != nullptr) {
        timeoutManager_->SetTimeoutCallback([this](const std::string &sessionId) {
            HandleProcessTimeout(sessionId);
        });
        timeoutManager_->SetYieldTimeoutCallback([this](const std::string &sessionId) {
            HandleProcessYieldTimeout(sessionId);
        });
        timeoutManager_->Start();
    }

    processMonitor_.SetExitCallback([this](const std::string &sessionId, int status) {
        HandleProcessExit(sessionId, status);
    });
    processMonitor_.Start();

    ioMonitor_ = IOMonitor::Create();
    if (ioMonitor_ != nullptr) {
        ioMonitor_->SetOutputCallback([](const std::string &sessionId, bool isStdout, const std::string &data) {
            auto record = CliToolManagerService::GetInstance()->GetSessionRecord(sessionId);
            if (record != nullptr) {
                record->AppendOutput(isStdout, data);
            }
            EventDispatcher::GetInstance().DispatchIOEvent(sessionId, isStdout ? "stdout" : "stderr", data);
        });
        ioMonitor_->SetInputReplyCallback([](const std::string &sessionId,
            const std::string &eventId, bool result) {
            auto record = CliToolManagerService::GetInstance()->GetSessionRecord(sessionId);
            if (record == nullptr) {
                TAG_LOGI(AAFwkTag::CLI_TOOL,
                    "Failed ioMonitor Input: session not found %{public}s: %{public}d", eventId.c_str(), result);
                return;
            }
            
            EventDispatcher::GetInstance().DispatchInputReplyEvent(record->callerPid, eventId,
                result ? ERR_OK : ERR_CLI_SEND_MESSAGE);
        });
        ioMonitor_->SetSessionClosedCallback([this](const std::string &sessionId, bool isStdout) {
            HandleOutputClosed(sessionId, isStdout);
        });
        ioMonitor_->SetSessionDrainedCallback([this](const std::string &sessionId) {
            HandleOutputDrained(sessionId);
        });
        ioMonitor_->Start();
    }
    initialized_ = true;
}

void CliToolManagerService::OnStart()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "climgr start");

    Init();

    // Publish the service
    auto cliService = CliToolManagerService::GetInstance();
    if (cliService == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "cli service is nullptr");
        return;
    }

    int32_t ret = CliToolDataManager::GetInstance().EnsureToolsLoaded();
    if (ret != 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to load: %{public}d", ret);
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully loaded");
    }

    if (!Publish(cliService)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Publish failed");
        return;
    }
}

void CliToolManagerService::OnStop()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "climgr stop");
    initialized_ = false;

    // Collect active PIDs before clearing sessions
    std::vector<pid_t> activePids;
    {
        std::lock_guard<std::mutex> lock(sessionsMutex_);
        for (const auto &[sessionId, record] : sessionRecords_) {
            if (record != nullptr && record->processId > 0) {
                activePids.push_back(record->processId);
            }
        }
        sessionRecords_.clear();
    }

    // Clear event dispatcher
    EventDispatcher::GetInstance().ClearAll();

    // Kill all active processes
    auto &processManager = ProcessManager::GetInstance();
    for (pid_t pid : activePids) {
        processManager.TerminateProcess(pid, SIGKILL);
    }

    if (ioMonitor_ != nullptr) {
        ioMonitor_->Stop();
    }
    processMonitor_.Stop();
    if (timeoutManager_ != nullptr) {
        timeoutManager_->Stop();
    }
}

void CliToolManagerService::AddSessionRecord(const std::shared_ptr<SessionRecord> &record)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    sessionRecords_[record->sessionId] = record;
}

std::shared_ptr<SessionRecord> CliToolManagerService::GetSessionRecord(const std::string &sessionId)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    auto it = sessionRecords_.find(sessionId);
    if (it == sessionRecords_.end()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "GetSessionRecord failed: sessionId=%{public}s not found", sessionId.c_str());
        return nullptr;
    }
    return it->second;
}

void CliToolManagerService::RemoveSessionRecord(const std::string &sessionId)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    sessionRecords_.erase(sessionId);
}

bool CliToolManagerService::RegisterSessionWithMonitors(const std::shared_ptr<SessionRecord> &record,
                                                        bool background,
                                                        int32_t yieldMs)
{
    if (ioMonitor_ == nullptr || ioMonitor_->RegisterSession(record->sessionId, record->stdoutPipe[0],
            record->stderrPipe[0], record->stdinPipe[1]) == false) {
        close(record->stdoutPipe[0]);
        close(record->stderrPipe[0]);
        close(record->stdinPipe[1]);
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ioMonitor registration failed: sessionId=%{public}s, pid=%{public}d",
            record->sessionId.c_str(), record->processId);
        return false;
    }

    processMonitor_.RegisterSession(record->processId, record->sessionId);

    if (timeoutManager_ != nullptr) {
        timeoutManager_->AddTimeoutTask(record->sessionId, record->processId,
            static_cast<uint32_t>(record->timeoutMs),
            background ? 0 : static_cast<uint32_t>(yieldMs));
    }
    return true;
}

void CliToolManagerService::UnregisterSessionWithMonitors(const std::string &sessionId)
{
    if (ioMonitor_) {
        ioMonitor_->UnregisterSession(sessionId);
    }
    processMonitor_.UnregisterSession(sessionId);
    if (timeoutManager_) {
        timeoutManager_->CancelTimeoutTask(sessionId);
    }
}

int32_t CliToolManagerService::PreExecuteValidation(const ExecToolParam &param,
                                                    const std::map<std::string, std::string> &args)
{
    if (ToolUtil::VerifySystemApp() == false) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Verify system app failed");
        return ERR_NOT_SYSTEM_APP;
    }
    if (ToolUtil::VerifyCallingPermission(PERMISSION_EXEC_CLI_TOOL) == false) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Verify PERMISSION_EXEC_CLI_TOOL failed");
        return ERR_PERMISSION_DENIED;
    }

    if (activeSessionCount_.load() >= MAX_CONCURRENT_SESSIONS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Session limit exceeded: %{public}d", MAX_CONCURRENT_SESSIONS);
        return ERR_SESSION_LIMIT_EXCEEDED;
    }

    ToolInfo toolInfo;
    if (GetToolInfoByName(param.toolName, toolInfo) != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Tool not found");
        return ERR_TOOL_NOT_EXIST;
    }

    auto checkPramRet = ToolUtil::ValidateInputSchemaProperties(toolInfo.inputSchema, param.subcommand, args);
    if (checkPramRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Input schema validation failed");
        return checkPramRet;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::GetAllToolInfos(std::vector<ToolInfo> &tools)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllToolInfos called");
    return CliToolDataManager::GetInstance().GetAllTools(tools);
}

int32_t CliToolManagerService::GetAllToolSummaries(std::vector<ToolSummary> &summaries)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllToolSummaries called");
    return CliToolDataManager::GetInstance().QueryToolSummaries(summaries);
}

int32_t CliToolManagerService::GetToolInfoByName(const std::string &name, ToolInfo &tool)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetToolInfoByName called, name='%{public}s'", name.c_str());
    return CliToolDataManager::GetInstance().GetToolByName(name, tool);
}

int32_t CliToolManagerService::RegisterTool(const ToolInfo &tool)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "RegisterTool called, tool name='%{public}s'", tool.name.c_str());
    return CliToolDataManager::GetInstance().RegisterTool(tool);
}

int32_t CliToolManagerService::ExecTool(const ExecToolParam &param,
                                        const std::map<std::string, std::string> &args,
                                        const std::string &eventId)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecTool called: toolName=%{public}s, subcommand=%{public}s",
        param.toolName.c_str(), param.subcommand.c_str());
    int32_t errCode = PreExecuteValidation(param, args);
    if (errCode != ERR_OK) {
        return errCode;
    }

    // Create and initialize session record
    std::shared_ptr<SessionRecord> record = CreateSessionRecord(param);
    if (record == nullptr) {
        return ERR_NO_INIT;
    }
    record->eventId = eventId;

    std::string sandboxConfig;
    if (!ToolUtil::GenerateSandboxConfig(param.challenge, sandboxConfig)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "caller is not hap");
        return ERR_NOT_HAP;
    }
    auto createRet = ProcessManager::GetInstance().CreateChildProcess(param, sandboxConfig, args, record);
    if (createRet != ERR_OK) {
        return createRet;
    }

    activeSessionCount_.fetch_add(1, std::memory_order_relaxed);
    AddSessionRecord(record);

    if (RegisterSessionWithMonitors(record, param.options.background, param.options.yieldMs) == false) {
        ProcessManager::GetInstance().TerminateProcess(record->processId, SIGKILL);
        RemoveSessionRecord(record->sessionId);
        return ERR_NO_INIT;
    }

    // Background session, immediately notify session info
    if (param.options.background) {
        CliSessionInfo session;
        record->BuildSessionInfo(session);
        if (!EventDispatcher::GetInstance().DispatchExecToolReplyEvent(record->callerPid, eventId, ERR_OK, session)) {
            ProcessManager::GetInstance().TerminateProcess(record->processId, SIGKILL);
            UnregisterSessionWithMonitors(record->sessionId);
            RemoveSessionRecord(record->sessionId);
            return ERR_NO_INIT;
        }
    }

    // Frontground session, after waiting for the yieldMs timeout, notify the session info
    return ERR_OK;
}

std::shared_ptr<SessionRecord> CliToolManagerService::CreateSessionRecord(const ExecToolParam &param)
{
    auto record = std::make_shared<SessionRecord>();
    if (record == nullptr) {
        return nullptr;
    }
    int32_t timeoutMs = param.options.timeout;
    record->startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    record->callerPid = IPCSkeleton::GetCallingPid();
    record->sessionId = ToolUtil::GenerateCliSessionId(param.toolName);
    record->toolName = param.toolName;
    record->timeoutMs = timeoutMs;
    record->SetState(SessionState::RUNNING);
    record->SetBackground(param.options.background);
    return record;
}

int32_t CliToolManagerService::ClearSession(const std::string &sessionId)
{
    if (ToolUtil::VerifySystemApp() == false) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Verify system app failed");
        return ERR_NOT_SYSTEM_APP;
    }
    if (ToolUtil::VerifyCallingPermission(PERMISSION_EXEC_CLI_TOOL) == false) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Verify PERMISSION_EXEC_CLI_TOOL failed");
        return ERR_PERMISSION_DENIED;
    }

    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ClearSession failed: sessionId=%{public}s not found", sessionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    if (record->GetState() != SessionState::RUNNING && record->GetState() != SessionState::SPAWNING) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "ClearSession failed: sessionId=%{public}s state=%{public}d is not cancellable",
            sessionId.c_str(), static_cast<int32_t>(record->GetState()));
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ClearSession: sessionId=%{public}s, pid=%{public}d",
        sessionId.c_str(), record->processId);
    if (!ProcessManager::GetInstance().TerminateProcess(record->processId, SIGTERM)) {
        return ERR_PERMISSION_DENIED;
    }
    record->SetState(SessionState::CANCELLING);
    return ERR_OK;
}

int32_t CliToolManagerService::SubscribeSession(const std::string &sessionId, const std::string &subscriptionId)
{
    if (ToolUtil::VerifySystemApp() == false) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Verify system app failed");
        return ERR_NOT_SYSTEM_APP;
    }
    if (ToolUtil::VerifyCallingPermission(PERMISSION_EXEC_CLI_TOOL) == false) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Verify PERMISSION_EXEC_CLI_TOOL failed");
        return ERR_PERMISSION_DENIED;
    }

    if (sessionId.empty() || subscriptionId.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "SubscribeSession failed: invalid args sessionId=%{public}s, subscriptionId=%{public}s",
            sessionId.c_str(), subscriptionId.c_str());
        return ERR_INVALID_PARAM;
    }
    if (GetSessionRecord(sessionId) == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "SubscribeSession failed: sessionId=%{public}s not found, subscriptionId=%{public}s",
            sessionId.c_str(), subscriptionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    if (!EventDispatcher::GetInstance().RegisterSubscriber(
        sessionId, subscriptionId, IPCSkeleton::GetCallingPid())) {
        return ERR_NO_INIT;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId)
{
    if (ToolUtil::VerifySystemApp() == false) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Verify system app failed");
        return ERR_NOT_SYSTEM_APP;
    }
    if (ToolUtil::VerifyCallingPermission(PERMISSION_EXEC_CLI_TOOL) == false) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Verify PERMISSION_EXEC_CLI_TOOL failed");
        return ERR_PERMISSION_DENIED;
    }

    if (!EventDispatcher::GetInstance().UnregisterSubscriber(
        sessionId, subscriptionId, IPCSkeleton::GetCallingPid())) {
        return ERR_NO_INIT;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::QuerySession(const std::string &sessionId, CliSessionInfo &session)
{
    if (ToolUtil::VerifySystemApp() == false) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Verify system app failed");
        return ERR_NOT_SYSTEM_APP;
    }
    if (ToolUtil::VerifyCallingPermission(PERMISSION_EXEC_CLI_TOOL) == false) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Verify PERMISSION_EXEC_CLI_TOOL failed");
        return ERR_PERMISSION_DENIED;
    }

    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "QuerySession failed: sessionId=%{public}s not found", sessionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    record->BuildSessionInfo(session);
    return ERR_OK;
}

int32_t CliToolManagerService::SendMessage(const std::string &sessionId,
                                           const std::string &inputText,
                                           const std::string &eventId)
{
    if (ToolUtil::VerifySystemApp() == false) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Verify system app failed");
        return ERR_NOT_SYSTEM_APP;
    }
    if (ToolUtil::VerifyCallingPermission(PERMISSION_EXEC_CLI_TOOL) == false) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Verify PERMISSION_EXEC_CLI_TOOL failed");
        return ERR_PERMISSION_DENIED;
    }

    if (ioMonitor_ == nullptr) {
        return ERR_CLI_SEND_MESSAGE;
    }

    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "SendMessage failed: sessionId=%{public}s not found", sessionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }

    // Check session state
    if (record->GetState() != SessionState::RUNNING) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "SendMessage failed: sessionId=%{public}s is not running", record->sessionId.c_str());
        return ERR_CLI_SEND_MESSAGE;
    }

    ioMonitor_->SendMessage(sessionId, inputText, eventId);
    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
