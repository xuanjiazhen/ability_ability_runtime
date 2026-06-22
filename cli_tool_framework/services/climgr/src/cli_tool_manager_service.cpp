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

#include <sys/wait.h>

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "app_mgr_client.h"
#include "ccm_util.h"
#include "cli_error_code.h"
#include "cli_tool_app_state_observer.h"
#include "cli_function_data_manager.h"
#include "event_dispatcher.h"
#include "function_info.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "permission_query_util.h"
#include "permission_util.h"
#include "process_manager.h"
#include "session_record.h"
#include "tokenid_kit.h"
#include "tool_util.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr const char* PERMISSION_EXEC_CLI_TOOL = "ohos.permission.EXEC_CLI_TOOL";
constexpr const char* PERMISSION_QUERY_CLI_TOOL = "ohos.permission.QUERY_CLI_TOOL";
constexpr const char* PERMISSION_REGISTER_CLI_TOOL = "ohos.permission.REGISTER_CLI_TOOL";

constexpr int32_t FOUNDATION_UID = 5523;
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

// 获取 CLI tool manager 服务的单例实例。
sptr<CliToolManagerService> CliToolManagerService::GetInstance()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (instance_ != nullptr) {
        return instance_;
    }
    instance_ = new (std::nothrow) CliToolManagerService();
    return instance_;
}

// 处理会话超时，标记超时状态并终止 CLI 进程或 skill 会话。
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
    record->SetTimeout(true);
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
    ProcessManager::GetInstance().Killpg(record->processId);
}

// 在 yield 超时后将前台会话切换为后台会话。
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

// 标记 stdout 或 stderr 已关闭，并在输出全部读完后完成会话清理。
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

// 在子进程退出且输出读完后完成会话收尾。
void CliToolManagerService::HandleOutputDrained(const std::string &sessionId)
{
    auto record = GetSessionRecord(sessionId);
    if (record != nullptr && record->HasProcessExited()) {
        FinalizeBackgroundSession(record);
    }
}

// 发送后台会话的最终回复和退出事件，清理订阅者并移除会话。
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
            record->callerPid, record->callerUid, record->eventId, ERR_OK, session);
    }
    if (ioMonitor_) {
        ioMonitor_->UnregisterSession(sessionId);
    }

    const int32_t status = record->GetTerminalStatus();
    EventDispatcher::GetInstance().DispatchExitEvent(sessionId, status);
    EventDispatcher::GetInstance().ClearSessionSubscribers(sessionId);
    RemoveSessionRecord(sessionId);
}

// 初始化 SIGCHLD 处理逻辑和 CLI 会话的 I/O 监听回调。
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
        ioMonitor_->SetSessionClosedCallback([](const std::string &sessionId, bool isStdout) {
            auto service = CliToolManagerService::GetInstance();
            if (service != nullptr) {
                service->HandleOutputClosed(sessionId, isStdout);
            }
        });
        ioMonitor_->SetSessionDrainedCallback([](const std::string &sessionId) {
            auto service = CliToolManagerService::GetInstance();
            if (service != nullptr) {
                service->HandleOutputDrained(sessionId);
            }
        });
        ioMonitor_->Start();
    }
    initialized_ = true;
}

// 启动 SystemAbility，加载工具和 function 数据，发布 climgr，并安排延迟卸载。
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
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to load tools: %{public}d", ret);
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully loaded tools");
    }

    ret = CliFunctionDataManager::GetInstance().EnsureFunctionsInitialized();
    if (ret != 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to initialize functions database: %{public}d", ret);
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully initialized functions database");
    }

    if (!Publish(cliService)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Publish failed");
        return;
    }
    DelayUnloadTask();
    TAG_LOGI(AAFwkTag::CLI_TOOL, "climgr start success");
}

// 停止 climgr，清理所有会话状态，终止运行中的 CLI 进程，并停止 I/O 监听。
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
        processManager.Killpg(pid);
    }

    if (ioMonitor_ != nullptr) {
        ioMonitor_->Stop();
    }
}

// 仅在没有接口调用、会话和被观察 caller pid 时允许按需卸载。
int32_t CliToolManagerService::OnIdle(const SystemAbilityOnDemandReason &idlReason)
{
    int32_t sessionSize = 0;
    {
        std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
        sessionSize = static_cast<int32_t>(sessionRecords_.size());
    }
    int32_t calledCount = interfaceCalledCount_.load();
    int32_t callerPidSize = GetCallerPidCount();
    if (calledCount != 0 || sessionSize != 0 || callerPidSize != 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "service busy, calledCount=%{public}d, sessionSize=%{public}d",
            calledCount, sessionSize);
        if (!CancelIdle()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "Fail to cancel idle");
        }
        return -1;
    }
    return 0;
}

// 按 sessionId 保存新的会话记录。
void CliToolManagerService::AddSessionRecord(const std::shared_ptr<SessionRecord> &record)
{
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    sessionRecords_[record->sessionId] = record;
}

// 查找会话记录，并清理空记录。
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

// 返回当前所有有效会话记录的快照。
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

// 从活跃会话表中移除指定会话记录。
void CliToolManagerService::RemoveSessionRecord(const std::string &sessionId)
{
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    sessionRecords_.erase(sessionId);
}

// 检查当前调用进程是否拥有目标会话。
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

// 安排 climgr 延迟卸载；服务仍忙时重新安排卸载任务。
void CliToolManagerService::DelayUnloadTask()
{
    auto task = []() {
        int32_t sessionSize = 0;
        {
            std::lock_guard<ffrt::mutex> guard(CliToolManagerService::GetInstance()->sessionsMutex_);
            sessionSize = static_cast<int32_t>(CliToolManagerService::GetInstance()->sessionRecords_.size());
        }
        int32_t calledCount = CliToolManagerService::GetInstance()->interfaceCalledCount_.load();
        int32_t callerPidSize = CliToolManagerService::GetInstance()->GetCallerPidCount();
        if (calledCount == 0 && sessionSize == 0 && callerPidSize == 0) {
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

// 将会话注册到 I/O 监听，并投递 yield 超时和执行超时定时任务。
bool CliToolManagerService::RegisterSessionWithMonitors(const std::shared_ptr<SessionRecord> &record,
    const ExecOptions &options)
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

    if (options.background == false && options.yieldMs != 0) {
        PostExecToolTask(options.yieldMs, record->sessionId, false);
    }
    if (record->timeoutMs != 0) {
        PostExecToolTask(record->timeoutMs, record->sessionId, true);
    }
    return true;
}

// 从 I/O 监听中取消注册指定会话。
void CliToolManagerService::UnregisterSessionWithMonitors(const std::string &sessionId)
{
    if (ioMonitor_) {
        ioMonitor_->UnregisterSession(sessionId);
    }
}

// 在完成 system app 和查询权限校验后查询完整 CLI 工具信息。
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

// 在完成 system app 和查询权限校验后查询轻量级 CLI 工具摘要。
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

// 在完成 system app 和查询权限校验后按名称查询单个 CLI 工具定义。
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

// 拒绝通过当前公开服务入口动态注册 CLI 工具。
int32_t CliToolManagerService::RegisterTool(const ToolInfo &tool)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "RegisterTool called, tool name='%{public}s'", tool.name.c_str());
    return ERR_PERMISSION_DENIED;
}

// 校验通过后为 foundation 进程注册 function 定义。
int32_t CliToolManagerService::RegisterFunction(const FunctionInfo &function)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "RegisterFunction called: %{public}s/%{public}s",
        function.funcNamespace.c_str(), function.functionName.c_str());

    InterfaceCallCounter counter(interfaceCalledCount_);

    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "RegisterFunction: Permission denied, callingUid=%{public}d", callingUid);
        return ERR_PERMISSION_DENIED;
    }

    if (!FunctionInfo::Validate(function)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "RegisterFunction: FunctionInfo validation failed");
        return ERR_INVALID_PARAM;
    }

    int32_t ret = CliFunctionDataManager::GetInstance().RegisterFunction(function);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "RegisterFunction: Failed to register function, ret=%{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully registered function: %{public}s/%{public}s",
        function.funcNamespace.c_str(), function.functionName.c_str());
    return ERR_OK;
}

// 为 system app 调用方按命名空间和名称查询单个已注册 function。
int32_t CliToolManagerService::GetFunctionInfo(const std::string &funcNamespace,
    const std::string &functionName, FunctionInfo &function)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetFunctionInfo called: %{public}s/%{public}s",
        funcNamespace.c_str(), functionName.c_str());
    InterfaceCallCounter counter(interfaceCalledCount_);

    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllFunctions: Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    int32_t ret = CliFunctionDataManager::GetInstance().GetFunctionByName(funcNamespace, functionName, function);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetFunctionInfo: Failed to get function, ret=%{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully got function: %{public}s/%{public}s",
        funcNamespace.c_str(), functionName.c_str());
    return ERR_OK;
}

// 为 foundation 进程反注册指定 function 定义。
int32_t CliToolManagerService::UnregisterFunction(const std::string &funcNamespace,
    const std::string &functionName)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "UnregisterFunction called: %{public}s/%{public}s",
        funcNamespace.c_str(), functionName.c_str());
    InterfaceCallCounter counter(interfaceCalledCount_);

    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "UnregisterFunction: Permission denied, callingUid=%{public}d", callingUid);
        return ERR_PERMISSION_DENIED;
    }

    int32_t ret = CliFunctionDataManager::GetInstance().UnregisterFunction(funcNamespace, functionName);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "UnregisterFunction: Failed to unregister function, ret=%{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully unregistered function: %{public}s/%{public}s",
        funcNamespace.c_str(), functionName.c_str());
    return ERR_OK;
}

// 为 foundation 进程反注册指定命名空间下的所有 intent function。
int32_t CliToolManagerService::UnregisterIntentFunctionsByNamespace(const std::string &funcNamespace)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "UnregisterIntentFunctionsByNamespace called: %{public}s", funcNamespace.c_str());
    InterfaceCallCounter counter(interfaceCalledCount_);

    auto callingUid = IPCSkeleton::GetCallingUid();
    if (callingUid != FOUNDATION_UID) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "UnregisterIntentFunctionsByNamespace: Permission denied, callingUid=%{public}d",
            callingUid);
        return ERR_PERMISSION_DENIED;
    }

    int32_t ret = CliFunctionDataManager::GetInstance().UnregisterIntentFunctionsByNamespace(funcNamespace);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "UnregisterIntentFunctionsByNamespace: Failed, ret=%{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully unregistered intentFunctions from namespace: %{public}s",
        funcNamespace.c_str());
    return ERR_OK;
}

// 为 system app 调用方查询全部已注册 function。
int32_t CliToolManagerService::GetAllFunctions(std::vector<FunctionInfo> &functions)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllFunctions called");
    InterfaceCallCounter counter(interfaceCalledCount_);

    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllFunctions: Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    int32_t ret = CliFunctionDataManager::GetInstance().GetAllFunctions(functions);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllFunctions: Failed to get functions, ret=%{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully got all functions: %{public}zu", functions.size());
    return ERR_OK;
}

// 校验调用方是否为 system app 且具备 EXEC_CLI_TOOL 权限。
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

// 检查继续创建会话是否会超过配置的并发数量限制。
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

// 加载并校验已配置的 CLI 工具，然后生成对应沙箱配置。
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

// 校验 shell 命令执行参数，并为 execCmd 生成沙箱配置。
int32_t CliToolManagerService::ValidateAndPrepareCmd(const ExecCmdParam &param, uint32_t tokenId,
    std::string &sandboxConfig, std::string &bundleName)
{
    auto checkPramRet = ToolUtil::ValidateExecOptionsProperties(const_cast<ExecOptions &>(param.options));
    if (checkPramRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Input execoptions validation failed");
        return checkPramRet;
    }

    if (!ToolUtil::GenerateCmdSandboxConfig(param, tokenId, sandboxConfig, bundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "caller is not hap");
        return ERR_NOT_HAP;
    }
    return ERR_OK;
}

// 创建并启动普通 CLI 子进程会话。
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

    if (RegisterSessionWithMonitors(record, param.options) == false) {
        ProcessManager::GetInstance().Killpg(record->processId);
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

// 对继续在后台运行的会话立即发送成功回复。
void CliToolManagerService::HandleBackgroundSessionReply(
    const std::shared_ptr<SessionRecord> &record, const std::string &eventId)
{
    CliSessionInfo session;
    record->BuildSessionInfo(session);
    EventDispatcher::GetInstance().DispatchExecToolReplyEvent(
        record->callerPid, record->callerUid, eventId, ERR_OK, session);
}

// 将非 independent 的 arkTSScript 工具分发到 skill 执行路径。
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

// 执行已配置的 CLI 工具，或分发 arkTSScript skill 会话。
int32_t CliToolManagerService::ExecTool(const ExecToolParam &param, const std::string &eventId,
    const sptr<ICliToolManagerScheduler> &scheduler)
{
    // 记录当前正在执行的服务接口调用次数，避免服务空闲卸载时误判。
    InterfaceCallCounter counter(interfaceCalledCount_);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecTool called: toolName=%{public}s, subcommand=%{public}s",
        param.toolName.c_str(), param.subcommand.c_str());

    // 校验调用方是否为 system app，并且是否具有 EXEC_CLI_TOOL 权限。
    if (auto ret = ValidateExecToolPermissions(); ret != ERR_OK) {
        return ret;
    }

    // 保存调用方的 scheduler，后续服务端通过它把执行结果回调给客户端。
    // 调用方的 pid + uid 不是它生成的，而是服务端在收到 Binder IPC 请求时，通过 IPC 框架拿到的：
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (!EventDispatcher::GetInstance().SetScheduler(callerPid, callerUid, scheduler)) {
        return ERR_NO_INIT;
    }

    // arkTSScript 类型的工具可能走 AbilityManager 的 skill 执行路径。
    ToolInfo toolInfo;
    bool dispatched = false;
    auto skillRet = TryDispatchSkillSession(param, eventId, toolInfo, dispatched);
    if (skillRet != ERR_OK) {
        return skillRet;
    }
    // 如果已经分发到 skill 路径，这里直接返回；后续结果由 skill 回调处理。
    if (dispatched) {
        return ERR_OK;
    }

    // 普通 CLI 执行路径需要先检查当前会话数量是否超过并发限制。
    if (auto ret = ValidateSessionLimit(); ret != ERR_OK) {
        return ret;
    }

    // 查询工具配置、校验输入参数，并生成 claw_sandbox 所需的沙箱配置。
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    std::string sandboxConfig;
    std::string bundleName;
    if (auto ret = ValidateAndPrepareTool(param, tokenId, toolInfo, sandboxConfig, bundleName); ret != ERR_OK) {
        return ret;
    }

    // 创建 session、fork 子进程执行工具，并注册 I/O 监听和超时任务。
    return SetupAndStartSession(param, eventId, toolInfo, sandboxConfig, bundleName);
}

// 在沙箱会话中执行 shell 命令，并为调用方订阅会话事件。
int32_t CliToolManagerService::ExecCmd(const ExecCmdParam &param, const std::string &eventId,
    const sptr<ICliToolManagerScheduler> &scheduler, const std::string &subscriptionId)
{
    InterfaceCallCounter counter(interfaceCalledCount_);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecCmd called: cmd=%{public}s", param.cmd.c_str());
    if (auto ret = ValidateExecToolPermissions(); ret != ERR_OK) {
        return ret;
    }
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (!EventDispatcher::GetInstance().SetScheduler(callerPid, callerUid, scheduler)) {
        return ERR_NO_INIT;
    }
    if (auto ret = ValidateSessionLimit(); ret != ERR_OK) {
        return ret;
    }
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    std::string sandboxConfig;
    std::string bundleName;
    if (auto ret = ValidateAndPrepareCmd(param, tokenId, sandboxConfig, bundleName); ret != ERR_OK) {
        return ret;
    }
    // Create session record for shell command
    auto record = std::make_shared<SessionRecord>();
    record->callerPid = callerPid;
    record->callerUid = callerUid;
    record->sessionId = ToolUtil::GenerateCliSessionId("shell", record);
    record->toolName = "shell";
    record->timeoutMs = param.options.timeout * COEFFICIENT;
    record->SetState(SessionState::RUNNING);
    record->SetBackground(param.options.background);
    record->eventId = eventId;
    auto fatherSessionRecords = GetSessionRecords();
    AddSessionRecord(record);
    auto subscribeRet = SubscribeSession(record->sessionId, subscriptionId, scheduler);
    if (subscribeRet != ERR_OK) {
        RemoveSessionRecord(record->sessionId);
        return subscribeRet;
    }
    auto createRet = ProcessManager::GetInstance().CreateShellProcess(param,
        sandboxConfig, record, fatherSessionRecords);
    if (createRet != ERR_OK) {
        RemoveSessionRecord(record->sessionId);
        return createRet;
    }
    if (RegisterSessionWithMonitors(record, param.options) == false) {
        ProcessManager::GetInstance().Killpg(record->processId);
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

// 投递延迟任务，用于处理 yield 超时或硬执行超时。
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

// 记录子进程退出状态，并在输出读完后完成会话收尾。
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

// 处理 SIGCHLD 信号，回收已退出子进程并更新对应会话。
void CliToolManagerService::sigchld_handler(int32_t sig)
{
    int32_t status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        auto instance = CliToolManagerService::GetInstance();
        if (instance != nullptr) {
            instance->WaitPid(pid, status, sig);
            ProcessManager::GetInstance().Killpg(pid);
        }
    }
}

// 在调用方应用进程死亡时清理相关会话和进程观察者。
void CliToolManagerService::OnProcessDied(const std::string &bundleName, pid_t diedPid)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "OnProcessDied called: bundleName=%{public}s, diedPid=%{public}d",
        bundleName.c_str(), diedPid);
    RemoveCallerPid(diedPid);
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
            ProcessManager::GetInstance().Killpg(sessionRecord->processId);
        }

        // Clean up session
        iter = sessionRecords_.erase(iter);
    }
}

// 注册应用状态观察者，以便调用方进程死亡时清理其会话。
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
    AddCallerPid(callerPid);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully registered observer for bundleName=%{public}s", bundleName.c_str());
}

// 根据 execTool 参数和调用方身份创建初始化后的会话记录。
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

// 取消调用方拥有的运行中会话，并发送清理相关事件。
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

    if (!ProcessManager::GetInstance().Killpg(record->processId)) {
        return ERR_NOT_KILL;
    }
    record->SetState(SessionState::CANCELLING);
    return ERR_OK;
}

// 为调用方拥有的运行中会话订阅事件。
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

// 移除调用方的指定会话事件订阅。
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

// 构建调用方拥有的会话的当前状态信息。
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

// 向调用方拥有的运行中非 skill 会话发送 stdin 文本。
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

// 为 native system 调用方批量查询子命令所需权限。
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

// 保存会话和调用方上下文，以便 skill 执行回调能路由回 climgr。
SkillCallbackAdapter::SkillCallbackAdapter(const std::string &sessionId,
    int32_t callerPid, int32_t callerUid, const std::string &eventId)
    : sessionId_(sessionId), callerPid_(callerPid), callerUid_(callerUid), eventId_(eventId)
{}

// 接收 skill 执行完成回调，并将结果转给服务进行会话清理。
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

// 从 execTool 参数中提取 skill 参数，并校验目标 skill 类型。
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

// 在分发 skill 会话前通过 AbilityManager 查询并确认目标 skill 类型。
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

// 创建 skill 会话记录，构造 skill 请求，并通过 AbilityManager 启动执行。
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

// 发送 skill 最终结果事件，并移除已完成的 skill 会话。
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

// 标记 skill 会话超时，分发超时事件，并移除会话。
void CliToolManagerService::HandleSkillSessionTimeout(const std::string &sessionId)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleSkillSessionTimeout skipped: sessionId:%{public}s not found", sessionId.c_str());
        return;
    }

    record->SetTimeout(true);
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

// 从活跃调用方跟踪集合中移除指定 caller pid。
void CliToolManagerService::RemoveCallerPid(int64_t pid)
{
    std::lock_guard<ffrt::mutex> guard(callerPidsMutex_);
    callerPids.erase(pid);
}

// 将指定 caller pid 加入活跃调用方跟踪集合。
void CliToolManagerService::AddCallerPid(int64_t pid)
{
    std::lock_guard<ffrt::mutex> guard(callerPidsMutex_);
    callerPids.insert(pid);
}

// 返回当前服务正在跟踪的 caller pid 数量。
int32_t CliToolManagerService::GetCallerPidCount()
{
    std::lock_guard<ffrt::mutex> guard(callerPidsMutex_);
    return static_cast<int32_t>(callerPids.size());
}
} // namespace CliTool
} // namespace OHOS
