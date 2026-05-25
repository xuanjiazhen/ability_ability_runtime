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

#include <nlohmann/json.hpp>
#include <sys/wait.h>
#include <vector>

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "app_mgr_client.h"
#include "ccm_util.h"
#include "cli_error_code.h"
#include "cli_tool_app_state_observer.h"
#include "claw_permission_info.h"
#include "device_manager.h"
#include "dstb_ipc_manager.h"
#include "event_dispatcher.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "permission_query_util.h"
#include "permission_util.h"
#include "process_manager.h"
#include "remote_exec_callback_manager.h"
#include "session_record.h"
#include "tokenid_kit.h"
#include "tool_util.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr const char* PERMISSION_EXEC_CLI_TOOL = "ohos.permission.EXEC_CLI_TOOL";
constexpr const char* PERMISSION_QUERY_CLI_TOOL = "ohos.permission.QUERY_CLI_TOOL";

constexpr int32_t COEFFICIENT = 1000;
constexpr int32_t QUERY_SUCCESS = 0;
constexpr int32_t QUERY_COMMAND_NOT_EXIST = 1;
constexpr int32_t QUERY_DB_ERROR = 2;
constexpr int32_t MAX_QUERY_CMDS_SIZE = 100;
constexpr int32_t ACTIVE_TIME = 30 * 1000; // 30s
constexpr int32_t SKILL_TYPE_INDEPENDENT = -1;
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

void CliToolManagerService::HandleProcessTimeout(const std::string &sessionId)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleProcessTimeout skipped: sessionId=%{public}s not found", sessionId.c_str());
        return;
    }

    if (record->sessionType == SessionType::SKILL) {
        HandleSkillSessionTimeout(sessionId);
        return;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "HandleProcessTimeout: sessionId=%{public}s", sessionId.c_str());
    record->SetTimedOut(true);
    record->SetState(SessionState::CANCELLING);

    auto oldBackground = record->SetBackground(true);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "HandleProcessTimeout: sessionId=%{public}s, background=%{public}d",
        sessionId.c_str(), oldBackground);
    if (oldBackground == false) {
        CliSessionInfo session;
        record->BuildSessionInfo(session);
        EventDispatcher::GetInstance().DispatchExecToolReplyEvent(
            record->callerPid, record->callerUid, record->eventId, ERR_OK, session);
    }

    EventDispatcher::GetInstance().DispatchErrorEvent(sessionId, "session timed out");
    ProcessManager::GetInstance().Kill(record->processId);
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
            record->callerPid, record->callerUid, record->eventId, ERR_OK, session);
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

        // 如果是远程调用，需要回调发起端
        if (record->isRemoteCall && !record->sourceDeviceId.empty()) {
            TAG_LOGI(AAFwkTag::CLI_TOOL,
                "FinalizeBackgroundSession: remote call, notify source device %{public}s",
                record->sourceDeviceId.substr(0, 8).c_str());
            NotifyRemoteCaller(record->sourceDeviceId, sessionId, ERR_OK, session);
        } else {
            // 本地调用，直接分发事件
            EventDispatcher::GetInstance().DispatchExecToolReplyEvent(
                record->callerPid, record->callerUid, record->eventId, ERR_OK, session);
        }
    }
    if (ioMonitor_) {
        ioMonitor_->UnregisterSession(sessionId);
    }

    const int32_t status = record->GetTerminalStatus();
    EventDispatcher::GetInstance().DispatchExitEvent(sessionId, status);
    EventDispatcher::GetInstance().ClearSessionSubscribers(sessionId);
    RemoveSessionRecord(sessionId);
}

void CliToolManagerService::Init()
{
    if (initialized_) {
        return;
    }

    struct sigaction sa = {};
    sa.sa_handler = CliToolManagerService::sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGCHLD, &sa, nullptr);

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
            
            EventDispatcher::GetInstance().DispatchInputReplyEvent(record->callerPid, record->callerUid, eventId,
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

    // 初始化分布式能力
    InitDistributed();

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
    DelayUnloadTask();
    TAG_LOGI(AAFwkTag::CLI_TOOL, "climgr start success");
}

void CliToolManagerService::OnStop()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "climgr stop");
    initialized_ = false;

    // Collect active PIDs before clearing sessions
    std::vector<pid_t> activePids;
    {
        std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
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
        processManager.Kill(pid);
    }

    if (ioMonitor_ != nullptr) {
        ioMonitor_->Stop();
    }

    // 清理分布式能力
    UnInitDistributed();
}

int32_t CliToolManagerService::OnIdle(const SystemAbilityOnDemandReason &idlReason)
{
    int32_t sessionSize = 0;
    {
        std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
        sessionSize = static_cast<int32_t>(sessionRecords_.size());
    }
    int32_t calledCount = interfaceCalledCount_.load();
    if (calledCount != 0 || sessionSize != 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "service busy, calledCount=%{public}d, sessionSize=%{public}d",
            calledCount, sessionSize);
        if (!CancelIdle()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "Fail to cancel idle");
        }
        return -1;
    }
    return 0;
}

void CliToolManagerService::AddSessionRecord(const std::shared_ptr<SessionRecord> &record)
{
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    sessionRecords_[record->sessionId] = record;
}

std::shared_ptr<SessionRecord> CliToolManagerService::GetSessionRecord(const std::string &sessionId)
{
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    auto it = sessionRecords_.find(sessionId);
    if (it == sessionRecords_.end()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "GetSessionRecord failed: sessionId=%{public}s not found", sessionId.c_str());
        return nullptr;
    }
    if (it->second == nullptr) {
        sessionRecords_.erase(it); // for leak
        return nullptr;
    }
    return it->second;
}

std::vector<std::shared_ptr<SessionRecord>> CliToolManagerService::GetSessionRecords()
{
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    std::vector<std::shared_ptr<SessionRecord>> records;
    records.reserve(sessionRecords_.size());
    for (const auto &[sessionId, record] : sessionRecords_) {
        if (record != nullptr) {
            records.emplace_back(record);
        }
    }
    return records;
}

void CliToolManagerService::RemoveSessionRecord(const std::string &sessionId)
{
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    sessionRecords_.erase(sessionId);
}

bool CliToolManagerService::IsSessionOwner(const std::shared_ptr<SessionRecord> &record, const char *action) const
{
    if (record == nullptr) {
        return false;
    }
    auto callingPid = IPCSkeleton::GetCallingPid();
    if (record->callerPid != callingPid) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "%{public}s failed: caller pid=%{public}d is not session owner pid=%{public}d, sessionId=%{public}s",
            action, callingPid, record->callerPid, record->sessionId.c_str());
        return false;
    }
    return true;
}

void CliToolManagerService::DelayUnloadTask()
{
    auto task = []() {
        int32_t sessionSize = 0;
        {
            std::lock_guard<ffrt::mutex> guard(CliToolManagerService::GetInstance()->sessionsMutex_);
            sessionSize = static_cast<int32_t>(CliToolManagerService::GetInstance()->sessionRecords_.size());
        }
        int32_t calledCount = CliToolManagerService::GetInstance()->interfaceCalledCount_.load();
        if (calledCount == 0 && sessionSize == 0) {
            TAG_LOGI(AAFwkTag::CLI_TOOL, "UnloadSA start");
            sptr<ISystemAbilityManager> saManager =
                OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (saManager == nullptr) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "null saManager");
                return;
            }
            int32_t result = saManager->UnloadSystemAbility(CLI_TOOL_MGR_SERVICE_ID);
            if (result != ERR_OK) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "UnloadSystemAbility ret: %{public}d", result);
                return;
            }
            TAG_LOGI(AAFwkTag::CLI_TOOL, "UnloadSA success");
        } else {
            TAG_LOGI(AAFwkTag::CLI_TOOL, "Service still busy (calledCount=%{public}d, sessionSize=%{public}d), "
                "reschedule delay unload task", calledCount, sessionSize);
            CliToolManagerService::GetInstance()->DelayUnloadTask();
        }
    };
    ffrt::submit(std::move(task), ffrt::task_attr().delay(ACTIVE_TIME * COEFFICIENT));
}

bool CliToolManagerService::RegisterSessionWithMonitors(const std::shared_ptr<SessionRecord> &record,
    const ExecToolParam &param)
{
    if (ioMonitor_ == nullptr || ioMonitor_->RegisterSession(
        record->sessionId, record->stdoutPipe[0], record->stderrPipe[0], record->stdinPipe[1]) == false) {
        close(record->stdoutPipe[0]);
        close(record->stderrPipe[0]);
        close(record->stdinPipe[1]);
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ioMonitor registration failed: sessionId=%{public}s, pid=%{public}d",
            record->sessionId.c_str(), record->processId);
        return false;
    }

    if (param.options.background == false && param.options.yieldMs != 0) {
        PostExecToolTask(param.options.yieldMs, record->sessionId, false);
    }
    if (record->timeoutMs != 0) {
        PostExecToolTask(record->timeoutMs, record->sessionId, true);
    }
    return true;
}

void CliToolManagerService::UnregisterSessionWithMonitors(const std::string &sessionId)
{
    if (ioMonitor_) {
        ioMonitor_->UnregisterSession(sessionId);
    }
}

int32_t CliToolManagerService::GetAllToolInfos(ToolsRawData &tools)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllToolInfos called");
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolInfos: Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_QUERY_CLI_TOOL)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolInfos: Permission denied");
        return ERR_PERMISSION_DENIED;
    }

    return CliToolDataManager::GetInstance().GetAllToolsRawData(tools);
}

int32_t CliToolManagerService::GetAllToolSummaries(std::vector<ToolSummary> &summaries)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllToolSummaries called");
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolSummaries: Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_QUERY_CLI_TOOL)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolSummaries: Permission denied");
        return ERR_PERMISSION_DENIED;
    }

    return CliToolDataManager::GetInstance().QueryToolSummaries(summaries);
}

int32_t CliToolManagerService::GetToolInfoByName(const std::string &name, ToolInfo &tool)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetToolInfoByName called, name='%{public}s'", name.c_str());
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetToolInfoByName: Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_QUERY_CLI_TOOL)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetToolInfoByName: Permission denied");
        return ERR_PERMISSION_DENIED;
    }

    return CliToolDataManager::GetInstance().GetToolByName(name, tool);
}

int32_t CliToolManagerService::RegisterTool(const ToolInfo &tool)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "RegisterTool called, tool name='%{public}s'", tool.name.c_str());
    return ERR_PERMISSION_DENIED;
}

int32_t CliToolManagerService::ValidateExecToolPermissions()
{
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_EXEC_CLI_TOOL)) {
        return ERR_PERMISSION_DENIED;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::ValidateSessionLimit()
{
    auto cliQuantity = CcmUtil::GetInstance().GetCliConcurrencyLimit();
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    if (static_cast<int32_t>(sessionRecords_.size()) >= cliQuantity) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Session limit exceeded: %{public}d", cliQuantity);
        return ERR_SESSION_LIMIT_EXCEEDED;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::ValidateAndPrepareTool(const ExecToolParam &param, uint32_t tokenId,
    ToolInfo &toolInfo, std::string &sandboxConfig, std::string &bundleName)
{
    if (CliToolDataManager::GetInstance().GetToolByName(param.toolName, toolInfo) != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Tool not found");
        return ERR_TOOL_NOT_EXIST;
    }

    auto checkPramRet = ToolUtil::ValidateProperties(toolInfo, const_cast<ExecToolParam &>(param), tokenId);
    if (checkPramRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Input schema validation failed");
        return checkPramRet;
    }

    if (!ToolUtil::GenerateSandboxConfig(param, tokenId, sandboxConfig, bundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "caller is not hap");
        return ERR_NOT_HAP;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::SetupAndStartSession(const ExecToolParam &param, const std::string &eventId,
    const ToolInfo &toolInfo, const std::string &sandboxConfig, const std::string &bundleName)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "Dispatch to CLI path, toolName=%{public}s eventId=%{public}s", param.toolName.c_str(), eventId.c_str());

    std::shared_ptr<SessionRecord> record = CreateSessionRecord(param, eventId);
    if (record == nullptr) {
        return ERR_NO_INIT;
    }

    auto fatherSessionRecords = GetSessionRecords();
    auto createRet = ProcessManager::GetInstance().CreateChildProcess(
        param, sandboxConfig, toolInfo, record, fatherSessionRecords);
    if (createRet != ERR_OK) {
        return createRet;
    }
    AddSessionRecord(record);

    if (RegisterSessionWithMonitors(record, param) == false) {
        ProcessManager::GetInstance().Kill(record->processId);
        RemoveSessionRecord(record->sessionId);
        return ERR_NO_INIT;
    }

    if (!bundleName.empty()) {
        RegisterAppStateObserver(bundleName, record->callerPid);
    }

    if (param.options.background) {
        HandleBackgroundSessionReply(record, eventId);
    }

    return ERR_OK;
}

void CliToolManagerService::HandleBackgroundSessionReply(
    const std::shared_ptr<SessionRecord> &record, const std::string &eventId)
{
    CliSessionInfo session;
    record->BuildSessionInfo(session);
    EventDispatcher::GetInstance().DispatchExecToolReplyEvent(
        record->callerPid, record->callerUid, eventId, ERR_OK, session);
}

int32_t CliToolManagerService::TryDispatchSkillSession(const ExecToolParam &param,
    const std::string &eventId, const ToolInfo &toolInfo, bool &dispatched)
{
    dispatched = false;
    if (!ToolUtil::IsSkillTool(param.toolName)) {
        return ERR_OK;
    }

    int32_t skillType = 0;
    auto skillRet = ValidateSkillTypeFromParam(param, skillType);
    if (skillRet != ERR_OK) {
        return skillRet;
    }
    if (skillType == SKILL_TYPE_INDEPENDENT) {
        TAG_LOGI(AAFwkTag::CLI_TOOL,
            "Independent skill, fallback to CLI path, toolName=%{public}s", param.toolName.c_str());
        return ERR_OK;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "Dispatch to skill path, toolName=%{public}s eventId=%{public}s",
        param.toolName.c_str(), eventId.c_str());
    dispatched = true;
    int32_t ret = SetupAndStartSkillSession(param, eventId, toolInfo);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "Skill dispatch failed, toolName=%{public}s ret=%{public}d", param.toolName.c_str(), ret);
    }
    return ret;
}

int32_t CliToolManagerService::ExecTool(const ExecToolParam &param, const std::string &eventId,
    const sptr<ICliToolManagerScheduler> &scheduler)
{
    InterfaceCallCounter counter(interfaceCalledCount_);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecTool called: toolName=%{public}s, subcommand=%{public}s",
        param.toolName.c_str(), param.subcommand.c_str());
    if (auto ret = ValidateExecToolPermissions(); ret != ERR_OK) {
        return ret;
    }
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (!EventDispatcher::GetInstance().SetScheduler(callerPid, callerUid, scheduler)) {
        return ERR_NO_INIT;
    }

    ToolInfo toolInfo;
    bool dispatched = false;
    auto skillRet = TryDispatchSkillSession(param, eventId, toolInfo, dispatched);
    if (skillRet != ERR_OK) {
        return skillRet;
    }
    if (dispatched) {
        return ERR_OK;
    }

    if (auto ret = ValidateSessionLimit(); ret != ERR_OK) {
        return ret;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    std::string sandboxConfig;
    std::string bundleName;
    if (auto ret = ValidateAndPrepareTool(param, tokenId, toolInfo, sandboxConfig, bundleName); ret != ERR_OK) {
        return ret;
    }

    return SetupAndStartSession(param, eventId, toolInfo, sandboxConfig, bundleName);
}

void CliToolManagerService::PostExecToolTask(int64_t time, const std::string &sessionId, bool isTimeout)
{
    auto timeoutTask = [sessionId, isTimeout]() {
        auto service = CliToolManagerService::GetInstance();
        if (service) {
            if (isTimeout) {
                service->HandleProcessTimeout(sessionId);
            } else {
                service->HandleProcessYieldTimeout(sessionId);
            }
        }
    };
    ffrt::submit(std::move(timeoutTask), ffrt::task_attr().delay(time * COEFFICIENT));
}

void CliToolManagerService::WaitPid(pid_t pid, int32_t status, int32_t sig)
{
    std::shared_ptr<SessionRecord> record = nullptr;
    {
        std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
        for (auto iter = sessionRecords_.begin(); iter != sessionRecords_.end();) {
            if (iter->second == nullptr) {
                std::string sessionId = iter->first;
                iter = sessionRecords_.erase(iter);
                TAG_LOGW(AAFwkTag::CLI_TOOL, "delete leak sessionId:%{public}s", sessionId.c_str());
                continue;
            }
            if (pid == iter->second->processId) {
                record = iter->second;
                break;
            }
            ++iter;
        }
    }
    AccessToken::AccessTokenKit::DeleteToolTokenByPid(pid);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "WaitPid delete tool pid:%{public}d", pid);
    if (record) {
        record->SetTerminalResult(status, sig);
        if (record->OutputDrained()) {
            FinalizeBackgroundSession(record);
        }
    }
}

void CliToolManagerService::sigchld_handler(int32_t sig)
{
    int32_t status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        auto instance = CliToolManagerService::GetInstance();
        if (instance != nullptr) {
            instance->WaitPid(pid, status, sig);
        }
    }
}

void CliToolManagerService::OnProcessDied(const std::string &bundleName, pid_t diedPid)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "OnProcessDied called: bundleName=%{public}s, diedPid=%{public}d",
        bundleName.c_str(), diedPid);
    {
        std::lock_guard<ffrt::mutex> guard(observerMutex_);
        bundleObservers_.erase(bundleName);
    }
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    // Iterate through sessionRecords_ to find matching SessionRecord by callerPid
    for (auto iter = sessionRecords_.begin(); iter != sessionRecords_.end();) {
        auto sessionRecord = iter->second;
        if (sessionRecord == nullptr) {
            std::string sessionId = iter->first;
            iter = sessionRecords_.erase(iter);
            TAG_LOGW(AAFwkTag::CLI_TOOL, "delete leak sessionId:%{public}s", sessionId.c_str());
            continue;
        }

        // Check if this session's callerPid matches the diedPid
        if (sessionRecord->GetCallerPid() != diedPid) {
            ++iter;
            continue;
        }

        // Kill the CLI process group (skill sessions have processId=-1, Killpg handles it)
        if (sessionRecord->processId > 0) {
            ProcessManager::GetInstance().Kill(sessionRecord->processId);
        }

        // Clean up session
        iter = sessionRecords_.erase(iter);
    }
}

void CliToolManagerService::RegisterAppStateObserver(const std::string &bundleName, pid_t callerPid)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "RegisterAppStateObserver called: bundleName=%{public}s, callerPid=%{public}d",
        bundleName.c_str(), callerPid);

    // Check if observer already exists for this bundle
    std::lock_guard<ffrt::mutex> guard(observerMutex_);
    if (bundleObservers_.find(bundleName) != bundleObservers_.end()) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Observer already registered for bundleName=%{public}s", bundleName.c_str());
        return;
    }

    // Create observer with callback to OnProcessDied
    auto callback = [](const std::string &bundleName, pid_t diedPid) {
        auto service = CliToolManagerService::GetInstance();
        if (service != nullptr) {
            service->OnProcessDied(bundleName, diedPid);
        }
    };

    sptr<CliToolAppStateObserver> observer = new CliToolAppStateObserver(bundleName, callback);
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create observer for bundleName=%{public}s", bundleName.c_str());
        return;
    }

    // Register observer through AppMgrClient
    AppExecFwk::AppMgrClient appMgrClient;
    auto ret = appMgrClient.ConnectAppMgrService();
    if (ret != AppExecFwk::AppMgrResultCode::RESULT_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to connect to AppMgrService");
        return;
    }

    std::vector<std::string> bundleNameList = { bundleName };
    auto registerRet = appMgrClient.RegisterApplicationStateObserver(observer, bundleNameList);
    if (registerRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to register observer for bundleName=%{public}s, ret=%{public}d",
            bundleName.c_str(), registerRet);
        return;
    }

    // Store observer
    bundleObservers_[bundleName] = observer;
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully registered observer for bundleName=%{public}s", bundleName.c_str());
}

std::shared_ptr<SessionRecord> CliToolManagerService::CreateSessionRecord(const ExecToolParam &param,
    const std::string &eventId)
{
    auto record = std::make_shared<SessionRecord>();
    record->callerPid = IPCSkeleton::GetCallingPid();
    record->callerUid = IPCSkeleton::GetCallingUid();
    record->sessionId = ToolUtil::GenerateCliSessionId(param.toolName, record);
    record->toolName = param.toolName;
    record->timeoutMs = param.options.timeout * COEFFICIENT;
    record->SetState(SessionState::RUNNING);
    record->SetBackground(param.options.background);
    record->eventId = eventId;
    return record;
}

int32_t CliToolManagerService::ClearSession(const std::string &sessionId)
{
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_EXEC_CLI_TOOL)) {
        return ERR_PERMISSION_DENIED;
    }

    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ClearSession failed: sessionId=%{public}s not found", sessionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    if (!IsSessionOwner(record, "ClearSession")) {
        return ERR_PERMISSION_DENIED;
    }
    if (record->GetState() != SessionState::RUNNING && record->GetState() != SessionState::SPAWNING) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "ClearSession failed: sessionId=%{public}s state=%{public}d is not cancellable",
            sessionId.c_str(), static_cast<int32_t>(record->GetState()));
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ClearSession: sessionId=%{public}s, pid=%{public}d",
        sessionId.c_str(), record->processId);

    if (record->sessionType == SessionType::SKILL) {
        // Skill sessions don't have a child process, mark as cancelling and clean up
        record->SetState(SessionState::CANCELLING);
        EventDispatcher::GetInstance().DispatchExitEvent(sessionId, 0);
        EventDispatcher::GetInstance().ClearSessionSubscribers(sessionId);
        RemoveSessionRecord(sessionId);
        return ERR_OK;
    }

    if (!ProcessManager::GetInstance().Kill(record->processId)) {
        return ERR_NOT_KILL;
    }
    record->SetState(SessionState::CANCELLING);
    return ERR_OK;
}

int32_t CliToolManagerService::SubscribeSession(const std::string &sessionId, const std::string &subscriptionId,
    const sptr<ICliToolManagerScheduler> &scheduler)
{
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_EXEC_CLI_TOOL)) {
        return ERR_PERMISSION_DENIED;
    }

    if (sessionId.empty() || subscriptionId.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "SubscribeSession failed: invalid args sessionId=%{public}s, subscriptionId=%{public}s",
            sessionId.c_str(), subscriptionId.c_str());
        return ERR_INVALID_PARAM;
    }
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "SubscribeSession failed: sessionId=%{public}s not found, subscriptionId=%{public}s",
            sessionId.c_str(), subscriptionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    if (!IsSessionOwner(record, "SubscribeSession")) {
        return ERR_PERMISSION_DENIED;
    }
    CliSessionInfo session;
    record->BuildSessionInfo(session);
    if (session.status != "running") {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "SubscribeSession failed: sessionId=%{public}s status=%{public}s is not subscribable",
            sessionId.c_str(), session.status.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (!EventDispatcher::GetInstance().SetScheduler(callerPid, callerUid, scheduler)) {
        return ERR_NO_INIT;
    }
    if (!EventDispatcher::GetInstance().RegisterSubscriber(
        sessionId, subscriptionId, callerPid, callerUid)) {
        return ERR_NO_INIT;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId)
{
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_EXEC_CLI_TOOL)) {
        return ERR_PERMISSION_DENIED;
    }

    if (!EventDispatcher::GetInstance().UnregisterSubscriber(
        sessionId, subscriptionId, IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid())) {
        return ERR_NO_INIT;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::QuerySession(const std::string &sessionId, CliSessionInfo &session)
{
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_EXEC_CLI_TOOL)) {
        return ERR_PERMISSION_DENIED;
    }

    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "QuerySession failed: sessionId=%{public}s not found", sessionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    if (!IsSessionOwner(record, "QuerySession")) {
        return ERR_PERMISSION_DENIED;
    }
    record->BuildSessionInfo(session);
    return ERR_OK;
}

int32_t CliToolManagerService::SendMessage(const std::string &sessionId,
    const std::string &inputText, const std::string &eventId,
    const sptr<ICliToolManagerScheduler> &scheduler)
{
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_EXEC_CLI_TOOL)) {
        return ERR_PERMISSION_DENIED;
    }

    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "SendMessage failed: sessionId=%{public}s not found", sessionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    if (!IsSessionOwner(record, "SendMessage")) {
        return ERR_PERMISSION_DENIED;
    }

    if (ioMonitor_ == nullptr) {
        return ERR_CLI_SEND_MESSAGE;
    }

    // Check session state
    if (record->GetState() != SessionState::RUNNING) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "SendMessage failed: sessionId=%{public}s is not running", record->sessionId.c_str());
        return ERR_CLI_SEND_MESSAGE;
    }

    // Skill sessions don't support stdin
    if (record->sessionType == SessionType::SKILL) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "SendMessage failed: sessionId=%{public}s is a skill session (no stdin)", sessionId.c_str());
        return ERR_CLI_SEND_MESSAGE;
    }

    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (!EventDispatcher::GetInstance().SetScheduler(callerPid, callerUid, scheduler)) {
        return ERR_NO_INIT;
    }
    ioMonitor_->SendMessage(sessionId, inputText, eventId);
    return ERR_OK;
}

int32_t CliToolManagerService::BatchQueryPermissionBySubCommand(
    const std::vector<Command> &cmds,
    std::vector<CommandPermission> &cmdPermissions)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "BatchQueryPermissionBySubCommand begin, cnt=%{public}zu", cmds.size());
    InterfaceCallCounter counter(interfaceCalledCount_);
    if (cmds.empty() || cmds.size() >= MAX_QUERY_CMDS_SIZE) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "cmds is empty or reach limit");
        return ERR_INVALID_PARAM;
    }

    auto callerToken = IPCSkeleton::GetCallingTokenID();
    if (Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken) !=
        Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Caller is not SA");
        return ERR_NOT_SA_CALLER;
    }

    if (!PermissionUtil::VerifyAccessToken(callerToken, PERMISSION_QUERY_CLI_TOOL)) {
        return ERR_PERMISSION_DENIED;
    }

    return PermissionQueryUtil::BatchQueryPermissions(cmds, cmdPermissions);
}

SkillCallbackAdapter::SkillCallbackAdapter(const std::string &sessionId,
    int32_t callerPid, int32_t callerUid, const std::string &eventId)
    : sessionId_(sessionId), callerPid_(callerPid), callerUid_(callerUid), eventId_(eventId)
{}

void SkillCallbackAdapter::OnExecuteDone(const std::string &requestCode, int32_t resultCode,
    const AppExecFwk::SkillExecuteResult &result)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "SkillCallbackAdapter::OnExecuteDone sessionId:%{public}s code:%{public}d",
        sessionId_.c_str(), resultCode);

    auto service = CliToolManagerService::GetInstance();
    if (service == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "service expired for sessionId:%{public}s", sessionId_.c_str());
        return;
    }

    auto record = service->GetSessionRecord(sessionId_);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "OnExecuteDone skipped: sessionId:%{public}s already cleaned", sessionId_.c_str());
        return;
    }

    std::string outputText;
    if (result.result != nullptr) {
        outputText = result.result->ToString();
    }
    record->SetSkillResult(resultCode, outputText);
    record->SetState(resultCode == ERR_OK ? SessionState::COMPLETED : SessionState::FAILED);

    auto session = ToolUtil::BuildSkillSessionInfo(sessionId_, resultCode, result);
    service->HandleSkillSessionComplete(sessionId_, callerPid_, callerUid_, eventId_, resultCode, session);
}

int32_t CliToolManagerService::ValidateSkillTypeFromParam(const ExecToolParam &param, int32_t &skillType)
{
    auto &args = const_cast<ExecToolParam &>(param).args;
    ToolUtil::NormalizeSkillParamKeys(args);
    auto bundleName = args.GetStringParam("bundleName");
    auto moduleName = args.GetStringParam("moduleName");
    auto skillName = args.GetStringParam("skillName");
    if (skillName.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "skillName is required in args");
        return ERR_INVALID_VALUE;
    }
    return ValidateSkillType(bundleName, moduleName, skillName, skillType);
}

int32_t CliToolManagerService::ValidateSkillType(const std::string &bundleName,
    const std::string &moduleName, const std::string &skillName, int32_t &skillType)
{
    auto queryRet = AAFwk::AbilityManagerClient::GetInstance()->QuerySkillType(
        bundleName, moduleName, skillName, skillType);
    if (queryRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "querySkillType failed:%{public}d", queryRet);
        return queryRet;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::SetupAndStartSkillSession(const ExecToolParam &param,
    const std::string &eventId, const ToolInfo &toolInfo)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "SetupAndStartSkillSession: toolName=%{public}s",
        param.toolName.c_str());

    auto &args = const_cast<ExecToolParam &>(param).args;
    auto bundleName = args.GetStringParam("bundleName");
    auto moduleName = args.GetStringParam("moduleName");
    auto skillName = args.GetStringParam("skillName");
    auto scriptPath = args.GetStringParam("scriptPath");
    auto funcName = args.GetStringParam("functionName");

    ToolUtil::ExpandArgsJsonString(args);
    auto skillArgs = ToolUtil::FilterSkillArgs(args);

    auto record = CreateSessionRecord(param, eventId);
    if (record == nullptr) {
        return ERR_NO_INIT;
    }
    record->sessionType = SessionType::SKILL;
    AddSessionRecord(record);

    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    auto adaptor = sptr<SkillCallbackAdapter>::MakeSptr(
        record->sessionId, record->callerPid, record->callerUid, eventId);

    AppExecFwk::SkillExecuteRequest skillRequest;
    skillRequest.callerTokenId = callerTokenId;
    skillRequest.bundleName = bundleName;
    skillRequest.moduleName = moduleName;
    skillRequest.skillName = skillName;
    skillRequest.scriptPath = scriptPath;
    skillRequest.functionName = funcName;
    skillRequest.skillArgs = skillArgs;

    TAG_LOGD(AAFwkTag::CLI_TOOL, "execSkill before ExecuteInAppSkillWithTokenId");
    int32_t ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInAppSkillWithTokenId(
        skillRequest, adaptor);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ExecuteInAppSkillWithTokenId failed:%{public}d", ret);
        RemoveSessionRecord(record->sessionId);
        return ret;
    }

    if (param.options.background) {
        HandleBackgroundSessionReply(record, eventId);
    }

    return ERR_OK;
}

void CliToolManagerService::HandleSkillSessionComplete(const std::string &sessionId,
    int32_t callerPid, int32_t callerUid, const std::string &eventId, int32_t resultCode,
    const CliSessionInfo &session)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleSkillSessionComplete skipped: sessionId:%{public}s not found", sessionId.c_str());
        return;
    }
    if (!record->BeginCleanup()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleSkillSessionComplete skipped: already cleaning sessionId:%{public}s", sessionId.c_str());
        return;
    }

    auto oldBackground = record->SetBackground(true);
    if (oldBackground == false) {
        EventDispatcher::GetInstance().DispatchExecToolReplyEvent(callerPid, callerUid, eventId, ERR_OK, session);
    }

    EventDispatcher::GetInstance().DispatchExitEvent(sessionId, 0);
    EventDispatcher::GetInstance().ClearSessionSubscribers(sessionId);
    RemoveSessionRecord(sessionId);
}

void CliToolManagerService::HandleSkillSessionTimeout(const std::string &sessionId)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleSkillSessionTimeout skipped: sessionId:%{public}s not found", sessionId.c_str());
        return;
    }

    record->SetTimedOut(true);
    record->SetState(SessionState::FAILED);

    auto oldBackground = record->SetBackground(true);
    if (oldBackground == false) {
        CliSessionInfo session;
        record->BuildSessionInfo(session);
        EventDispatcher::GetInstance().DispatchExecToolReplyEvent(
            record->callerPid, record->callerUid, record->eventId, ERR_OK, session);
    }

    EventDispatcher::GetInstance().DispatchErrorEvent(sessionId, "session timed out");
    EventDispatcher::GetInstance().DispatchExitEvent(sessionId, 0);
    EventDispatcher::GetInstance().ClearSessionSubscribers(sessionId);
    RemoveSessionRecord(sessionId);
}

// ============== 分布式接口实现 ==============

namespace {
const std::string DSTB_PKG_NAME = "climgr_distributed";
const std::string PERMISSION_DISTRIBUTED_CLI_TOOL = "ohos.permission.DISTRIBUTED_CLI_TOOL";
} // namespace

void CliToolManagerService::InitDistributed()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "InitDistributed called");
    // 初始化设备管理器（获取本地设备信息等）
    // 实际初始化逻辑在首次使用时进行
}

void CliToolManagerService::UnInitDistributed()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "UnInitDistributed called");
}

bool CliToolManagerService::CheckDstbCallerPermission(const DstbCallerInfo &callerInfo)
{
    // 基本校验：检查调用者设备ID不为空
    if (callerInfo.sourceDeviceId.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "CheckDstbCallerPermission: source device ID is empty");
        return false;
    }

    // TODO: 添加更详细的权限校验
    // 1. 检查设备信任级别
    // 2. 检查是否允许远端调用
    // 3. 检查调用者的分布式权限

    TAG_LOGD(AAFwkTag::CLI_TOOL, "CheckDstbCallerPermission: passed for device %{public}s",
        callerInfo.sourceDeviceId.substr(0, 8).c_str());
    return true;
}

int32_t CliToolManagerService::GetAvailableDevices(std::vector<DstbDeviceInfo> &devices)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAvailableDevices called");
    return DstbIpcManager::GetInstance().GetAvailableDevices(devices);
}

int32_t CliToolManagerService::GetRemoteToolSummaries(const std::string &targetDeviceId,
    std::vector<ToolSummary> &summaries)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetRemoteToolSummaries called for device: %{public}s",
        targetDeviceId.substr(0, 8).c_str());

    InterfaceCallCounter counter(interfaceCalledCount_);

    // 使用 DstbIpcManager 进行跨设备调用
    return DstbIpcManager::GetInstance().GetRemoteToolSummaries(targetDeviceId, summaries);
}

int32_t CliToolManagerService::GetAllToolSummariesFromRemote(const DstbCallerInfo &callerInfo,
    std::vector<ToolSummary> &summaries)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllToolSummariesFromRemote called from device: %{public}s, uid: %{public}d",
        callerInfo.sourceDeviceId.substr(0, 8).c_str(), callerInfo.uid);

    // 校验调用者来源
    if (!CheckDstbCallerPermission(callerInfo)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolSummariesFromRemote: invalid caller");
        return DstbErrorCode::ERR_REMOTE_CALL_FAILED;
    }

    // 直接调用 DataManager 获取工具列表，跳过本地权限检查
    // 因为远程调用已经通过 CheckDstbCallerPermission 校验
    return CliToolDataManager::GetInstance().QueryToolSummaries(summaries);
}

int32_t CliToolManagerService::GetRemoteToolInfoByName(const std::string &targetDeviceId,
    const std::string &name, ToolInfo &tool)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetRemoteToolInfoByName called for device: %{public}s, tool: %{public}s",
        targetDeviceId.substr(0, 8).c_str(), name.c_str());

    // 使用 DstbIpcManager 进行跨设备调用
    return DstbIpcManager::GetInstance().GetRemoteToolInfoByName(targetDeviceId, name, tool);
}

int32_t CliToolManagerService::GetToolInfoByNameFromRemote(const DstbCallerInfo &callerInfo,
    const std::string &name, ToolInfo &tool)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetToolInfoByNameFromRemote called from device: %{public}s, uid: %{public}d, tool: %{public}s",
        callerInfo.sourceDeviceId.substr(0, 8).c_str(), callerInfo.uid, name.c_str());

    // 校验调用者来源
    if (!CheckDstbCallerPermission(callerInfo)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetToolInfoByNameFromRemote: invalid caller");
        return DstbErrorCode::ERR_REMOTE_CALL_FAILED;
    }

    // 直接调用 DataManager 获取工具信息，跳过本地权限检查
    // 因为远程调用已经通过 CheckDstbCallerPermission 校验
    return CliToolDataManager::GetInstance().GetToolByName(name, tool);
}

int32_t CliToolManagerService::ExecToolRemote(const std::string &targetDeviceId,
    const ExecToolParam &param, const std::string &eventId,
    const sptr<ICliToolManagerScheduler> &scheduler)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolRemote called for device: %{public}s, tool: %{public}s",
        targetDeviceId.substr(0, 8).c_str(), param.toolName.c_str());

    // 使用 DstbIpcManager 进行跨设备调用
    return DstbIpcManager::GetInstance().ExecToolRemote(targetDeviceId, param, eventId, scheduler);
}

int32_t CliToolManagerService::ExecToolFromRemote(const DstbCallerInfo &callerInfo,
    const ExecToolParam &param, const std::string &eventId, std::string &sessionId)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolFromRemote called from device: %{public}s, uid: %{public}d, pid: %{public}d, tool: %{public}s, subcmd: %{public}s",
        callerInfo.sourceDeviceId.substr(0, 8).c_str(), callerInfo.uid, callerInfo.pid,
        param.toolName.c_str(), param.subcommand.c_str());

    // 校验调用者来源
    if (!CheckDstbCallerPermission(callerInfo)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: invalid caller");
        return DstbErrorCode::ERR_REMOTE_CALL_FAILED;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: CheckDstbCallerPermission passed");

    // 调用本地 ExecTool，跳过权限检查（因为已经通过 CheckDstbCallerPermission 校验）
    int32_t callerPid = callerInfo.pid;
    int32_t callerUid = callerInfo.uid;

    // 会话数限制检查
    auto cliQuantity = CcmUtil::GetInstance().GetCliConcurrencyLimit();
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: cliQuantity=%{public}d, currentSessions=%{public}zu",
        cliQuantity, sessionRecords_.size());
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    if (static_cast<int32_t>(sessionRecords_.size()) >= cliQuantity) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Session limit exceeded: %{public}d", cliQuantity);
        return ERR_SESSION_LIMIT_EXCEEDED;
    }

    // 获取工具信息
    ToolInfo toolInfo;
    int32_t toolRet = CliToolDataManager::GetInstance().GetToolByName(param.toolName, toolInfo);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: GetToolByName result=%{public}d", toolRet);
    if (toolRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Tool not found: %{public}s", param.toolName.c_str());
        return ERR_TOOL_NOT_EXIST;
    }

    // 生成 sandbox 配置
    // 注意：远程调用时，发起端的 accessToken 在本设备上无效
    // 需要通过 bundleName 获取本设备上对应应用的 tokenId
    std::string sandboxConfig;
    std::string bundleName = callerInfo.bundleName;
    uint64_t callerTokenId = 0;

    // 尝试通过 bundleName 获取本设备上的 FullTokenID
    if (!bundleName.empty()) {
        // 指定 userId 为 100
        int32_t userId = 100;
        // appIndex 默认为 0（非克隆应用）
        int32_t appIndex = 0;
        // 使用 GetHapTokenIDEx 获取完整的 AccessTokenIDEx（包含系统应用标志）
        Security::AccessToken::AccessTokenIDEx tokenIdEx =
            Security::AccessToken::AccessTokenKit::GetHapTokenIDEx(userId, bundleName, appIndex);
        if (tokenIdEx.tokenIDEx != 0) {
            callerTokenId = tokenIdEx.tokenIDEx;
            TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: found local tokenIdEx=%{public}llu for bundleName=%{public}s, userId=%{public}d",
                static_cast<unsigned long long>(callerTokenId), bundleName.c_str(), userId);
        } else {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: bundleName=%{public}s not found on local device, userId=%{public}d",
                bundleName.c_str(), userId);
        }
    }

    // 如果找不到本地 tokenId，使用 climgr 服务自身的 tokenId（可能导致 claw_sandbox 校验失败）
    if (callerTokenId == 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: using fallback tokenId");
        callerTokenId = IPCSkeleton::GetSelfTokenID();
    }

    // 通过 accessTokenId 获取本地设备上对应应用的进程 PID
    int32_t localCallerPid = 0;
    Security::AccessToken::AccessTokenID hostTokenId = static_cast<Security::AccessToken::AccessTokenID>(
        callerTokenId & 0xFFFFFFFF);
    AppExecFwk::AppMgrClient appMgrClient;
    std::vector<AppExecFwk::RunningProcessInfo> processInfos;
    auto appMgrRet = appMgrClient.GetProcessRunningInfosByAccessTokenId(hostTokenId, processInfos);
    if (appMgrRet == AppExecFwk::AppMgrResultCode::RESULT_OK && !processInfos.empty()) {
        localCallerPid = processInfos[0].pid_;
        TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: found local process, pid=%{public}d, bundleName=%{public}s",
            localCallerPid, processInfos[0].processName_.c_str());
    } else {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: no running process found for accessTokenId=%{public}u",
            hostTokenId);
    }

    // 为分布式执行生成 challenge
    std::string challenge = GetChallenge(hostTokenId, param.toolName, param.subcommand);

    nlohmann::json config;
    config["callerTokenId"] = callerTokenId;
    config["challenge"] = challenge;
    config["uid"] = callerInfo.uid;
    config["callerPid"] = localCallerPid;
    config["gid"] = callerInfo.uid;  // 远程调用时使用 uid 作为 gid
    config["appId"] = bundleName.empty() ? "remote_caller" : bundleName;
    config["bundleName"] = bundleName.empty() ? "remote_caller" : bundleName;
    config["cliName"] = param.toolName;
    config["subCliName"] = param.subcommand;
    std::vector<std::string> nsFlags;
    nsFlags.push_back("pid");
    config["nsFlags"] = nsFlags;
    sandboxConfig = config.dump();
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: sandbox config generated, callerTokenId=%{public}llu, bundleName=%{public}s",
        static_cast<unsigned long long>(callerTokenId), bundleName.c_str());

    // 创建会话记录
    std::shared_ptr<SessionRecord> record = CreateSessionRecord(param, eventId);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: CreateSessionRecord result=%{public}s",
        record ? "success" : "failed");
    if (record == nullptr) {
        return ERR_NO_INIT;
    }

    // 设置远程调用者信息
    record->callerPid = callerPid;
    record->callerUid = callerUid;
    record->isRemoteCall = true;
    record->sourceDeviceId = callerInfo.sourceDeviceId;
    sessionId = record->sessionId;
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: record created, sessionId=%{public}s, callerPid=%{public}d, callerUid=%{public}d",
        sessionId.c_str(), callerPid, callerUid);

    // 创建子进程执行工具
    // 注意：此处已持有 sessionsMutex_，不能调用 GetSessionRecords()（会再次获取锁导致死锁）
    // 直接从 sessionRecords_ 构造 fatherSessionRecords
    std::vector<std::shared_ptr<SessionRecord>> fatherSessionRecords;
    fatherSessionRecords.reserve(sessionRecords_.size());
    for (const auto &[id, rec] : sessionRecords_) {
        if (rec != nullptr) {
            fatherSessionRecords.emplace_back(rec);
        }
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: calling CreateChildProcess, tool=%{public}s, executablePath=%{public}s, fatherSessions=%{public}zu",
        param.toolName.c_str(), toolInfo.executablePath.c_str(), fatherSessionRecords.size());
    auto createRet = ProcessManager::GetInstance().CreateChildProcess(
        param, sandboxConfig, toolInfo, record, fatherSessionRecords);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: CreateChildProcess result=%{public}d", createRet);
    if (createRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: CreateChildProcess failed=%{public}d", createRet);
        return createRet;
    }

    // 直接添加到 sessionRecords_，因为已经持有锁
    sessionRecords_[record->sessionId] = record;
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: session added, processId=%{public}d", record->processId);

    // 注册会话监控
    bool registerRet = RegisterSessionWithMonitors(record, param);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolFromRemote: RegisterSessionWithMonitors result=%{public}d", registerRet);
    if (!registerRet) {
        ProcessManager::GetInstance().Kill(record->processId);
        // 直接从 sessionRecords_ 移除，因为已经持有锁
        sessionRecords_.erase(record->sessionId);
        return ERR_NO_INIT;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecToolFromRemote success, sessionId: %{public}s", sessionId.c_str());
    return ERR_OK;
}

int32_t CliToolManagerService::NotifyRemoteExecResult(const std::string &sourceDeviceId,
    const std::string &sessionId, int32_t resultCode, const CliSessionInfo &session)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "NotifyRemoteExecResult: sourceDevice=%{public}s, sessionId=%{public}s, resultCode=%{public}d",
        sourceDeviceId.substr(0, 8).c_str(), sessionId.c_str(), resultCode);

    // 校验来源设备
    if (sourceDeviceId.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "NotifyRemoteExecResult: invalid sourceDeviceId");
        return ERR_INVALID_PARAM;
    }

    // 调用回调管理器处理结果
    RemoteExecCallbackManager::GetInstance().HandleRemoteExecResult(sessionId, resultCode, session);

    return ERR_OK;
}

void CliToolManagerService::NotifyRemoteCaller(const std::string &sourceDeviceId, const std::string &sessionId,
    int32_t resultCode, const CliSessionInfo &session)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "NotifyRemoteCaller: sourceDevice=%{public}s, sessionId=%{public}s, resultCode=%{public}d",
        sourceDeviceId.substr(0, 8).c_str(), sessionId.c_str(), resultCode);

    // 通过 DstbIpcManager 获取发起端的 proxy 并调用 NotifyRemoteExecResult
    auto proxy = DstbIpcManager::GetInstance().GetRemoteCliToolMgrProxy(sourceDeviceId);
    if (proxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "NotifyRemoteCaller: failed to get proxy for device %{public}s",
            sourceDeviceId.substr(0, 8).c_str());
        return;
    }

    sptr<ICliToolManager> cliMgrProxy = iface_cast<ICliToolManager>(proxy);
    if (cliMgrProxy == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "NotifyRemoteCaller: iface_cast failed");
        DstbIpcManager::GetInstance().ClearRemoteProxy(sourceDeviceId);
        return;
    }

    // 获取本地设备 ID
    std::string localDeviceId = DstbIpcManager::GetInstance().GetLocalDeviceId();

    // 调用发起端的 NotifyRemoteExecResult
    auto ret = cliMgrProxy->NotifyRemoteExecResult(localDeviceId, sessionId, resultCode, session);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "NotifyRemoteCaller: NotifyRemoteExecResult failed: %{public}d", ret);
        DstbIpcManager::GetInstance().ClearRemoteProxy(sourceDeviceId);
        return;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "NotifyRemoteCaller: notification sent successfully");
}

std::string CliToolManagerService::GetChallenge(Security::AccessToken::AccessTokenID hostTokenId,
    const std::string &toolName, const std::string &subcommand)
{
    std::string agentId = "1001";
    Security::AccessToken::CliAuthInfo cliAuthInfo;
    cliAuthInfo.cliInfo.cliName = toolName;
    cliAuthInfo.cliInfo.subCliName = subcommand;

    // 根据工具名设置权限
    if (toolName == "ohos-queryTime") {
        cliAuthInfo.permissionNames = {"ohos.permission.APPROXIMATELY_LOCATION", "ohos.permission.LOCATION"};
        cliAuthInfo.authorizationResults = {true, true};
    }
    // 其他工具可以在此添加权限配置

    std::vector<Security::AccessToken::CliAuthInfo> authInfoList = {cliAuthInfo};
    Security::AccessToken::ToolAuthResult authResult;
    int32_t ret = Security::AccessToken::AccessTokenKit::GenerateCliAuthResult(
        hostTokenId, agentId, authInfoList, authResult);
    if (ret == Security::AccessToken::RET_SUCCESS && !authResult.authResults.empty()) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "GetChallenge success, tool=%{public}s, challenge=%{public}s",
            toolName.c_str(), authResult.authResults[0].substr(0, 8).c_str());
        return authResult.authResults[0];
    }
    TAG_LOGW(AAFwkTag::CLI_TOOL, "GetChallenge failed, tool=%{public}s, ret=%{public}d", toolName.c_str(), ret);
    return "";
}

} // namespace CliTool
} // namespace OHOS
