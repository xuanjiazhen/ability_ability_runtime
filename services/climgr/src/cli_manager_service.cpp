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
#include "registry_manager.h"
#include "process_manager.h"
#include "process_monitor.h"
#include "timeout_manager.h"
#include "resource_manager.h"
#include "command_sanitizer.h"
#include "event_dispatcher.h"
#include "permission_checker.h"
#include "io_monitor.h"
#include "schema_validator.h"
#include "hilog_tag_wrapper.h"
#include <algorithm>

namespace OHOS {
namespace CliMgr {

// SystemAbility 注册
const bool CLI_MANAGER_SERVICE_REGISTERED = []() {
    auto service = new CLIManagerService(SA_CLIMGR_ID, false);
    SystemAbility::MakeAndRegisterAbility(service);
    return true;
}();

CLIManagerService::CLIManagerService(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate)
{
    TAG_LOGI(TAG, "CLIManagerService constructed, SA ID: %{public}d", systemAbilityId);
}

CLIManagerService::~CLIManagerService()
{
    TAG_LOGI(TAG, "CLIManagerService destroyed");
}

void CLIManagerService::OnStart()
{
    TAG_LOGI(TAG, "CLIManagerService OnStart");

    // 初始化组件
    if (!InitComponents()) {
        TAG_LOGE(TAG, "Failed to initialize components");
        return;
    }

    // 启动 I/O Monitor
    if (ioMonitor_ && !ioMonitor_->Start()) {
        TAG_LOGE(TAG, "Failed to start I/O Monitor");
        return;
    }

    // ProcessMonitor 和 TimeoutManager 已经在 InitComponents 中启动

    // 标记服务为就绪状态
    running_.store(true);

    // 发布服务
    bool res = Publish(this);
    if (!res) {
        TAG_LOGE(TAG, "Failed to publish service");
        return;
    }

    TAG_LOGI(TAG, "CLIManagerService started and published successfully");
}

void CLIManagerService::OnStop()
{
    TAG_LOGI(TAG, "CLIManagerService OnStop");

    running_.store(false);

    // 停止所有监控组件
    if (ioMonitor_) {
        ioMonitor_->Stop();
    }

    if (processMonitor_) {
        processMonitor_->Stop();
    }

    if (timeoutManager_) {
        timeoutManager_->Stop();
    }

    // 清理所有任务
    std::lock_guard<std::mutex> lock(tasksMutex_);
    for (auto& [taskId, context] : tasks_) {
        if (context->processId > 0) {
            processManager_->TerminateProcess(context->processId, SIGKILL);
        }
    }
    tasks_.clear();

    TAG_LOGI(TAG, "CLIManagerService stopped");
}

void CLIManagerService::OnAddSystemAbility(int32_t systemAbilityId,
                                          const std::string& deviceId)
{
    TAG_LOGI(TAG, "System ability added: %{public}d", systemAbilityId);
}

void CLIManagerService::OnRemoveSystemAbility(int32_t systemAbilityId,
                                             const std::string& deviceId)
{
    TAG_LOGI(TAG, "System ability removed: %{public}d", systemAbilityId);
}

bool CLIManagerService::InitComponents()
{
    // 创建 Registry Manager
    registryManager_ = std::make_shared<RegistryManager>();
    if (!registryManager_->Initialize()) {
        TAG_LOGE(TAG, "Failed to initialize RegistryManager");
        return false;
    }

    // 创建 Event Dispatcher
    eventDispatcher_ = std::make_shared<EventDispatcher>();

    // Create I/O Monitor
    ioMonitor_ = std::make_shared<IOMonitor>(eventDispatcher_.get());

    // 创建 Process Manager
    processManager_ = std::make_shared<ProcessManager>(ioMonitor_.get());

    // 创建 Process Monitor
    processMonitor_ = std::make_shared<ProcessMonitor>(eventDispatcher_.get());
    if (!processMonitor_->Start()) {
        TAG_LOGE(TAG, "Failed to start ProcessMonitor");
        return false;
    }

    // 创建 Timeout Manager
    timeoutManager_ = std::make_shared<TimeoutManager>(processManager_.get(), eventDispatcher_.get());
    if (!timeoutManager_->Start()) {
        TAG_LOGE(TAG, "Failed to start TimeoutManager");
        return false;
    }

    // 创建 Resource Manager
    resourceManager_ = std::make_shared<ResourceManager>();
    if (!resourceManager_->Initialize()) {
        TAG_LOGE(TAG, "Failed to initialize ResourceManager");
        return false;
    }

    // 设置进程退出回调
    processManager_->SetProcessExitCallback(
        [this](const std::string& taskId, int pid, int status) {
            // 处理进程退出
            auto context = GetTask(taskId);
            if (context) {
                int exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
                eventDispatcher_->DispatchExitEvent(taskId, exitCode);
                RemoveTask(taskId);

                // 清理资源管理器
                resourceManager_->UnregisterTask(taskId);
            }
        }
    );

    // 创建 Permission Checker
    permissionChecker_ = std::make_shared<PermissionChecker>();
    if (!permissionChecker_->Initialize()) {
        TAG_LOGE(TAG, "Failed to initialize PermissionChecker");
        return false;
    }

    TAG_LOGI(TAG, "All components initialized successfully");
    return true;
}

int32_t CLIManagerService::OnRemoteRequest(uint32_t code,
                                          MessageParcel& data,
                                          MessageParcel& reply,
                                          MessageOption& option)
{
    TAG_LOGD(TAG, "OnRemoteRequest: code=%{public}u", code);

    // 验证接口 Token
    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TAG_LOGE(TAG, "Invalid interface token");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case QUERY_TOOLS:
            return QueryToolsInternal(data, reply);
        case EXEC_TOOL:
            return ExecToolInternal(data, reply);
        case SUBSCRIBE_SESSION:
            return SubscribeSessionInternal(data, reply);
        case CANCEL_SESSION:
            return CancelSessionInternal(data, reply);
        case QUERY_SESSION_STATUS:
            return QuerySessionStatusInternal(data, reply);
        default:
            TAG_LOGW(TAG, "Unknown code: %{public}u", code);
            return ERR_UNKNOWN_TRANSACTION;
    }

    return ERR_OK;
}

int32_t CLIManagerService::QueryTools(std::vector<ToolInfo>& tools)
{
    if (!running_.load()) {
        TAG_LOGE(TAG, "Service not ready");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    return registryManager_->QueryTools(tools);
}

int32_t CLIManagerService::ExecTool(
    const std::string& name,
    const Json::Value& arguments,
    const ExecOptions& options,
    SessionInfo& session)
{
    if (!running_.load()) {
        TAG_LOGE(TAG, "Service not ready");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    // 1. 查询工具信息
    ToolInfo tool;
    int32_t ret = registryManager_->QueryToolByName(name, tool);
    if (ret != ErrCode::OK) {
        TAG_LOGE(TAG, "Tool not found: %{public}s", name.c_str());
        return ret;
    }

    // 2. 校验权限
    CallerInfo callerInfo;  // TODO: 从 IPC 上下文获取
    PermissionCheckResult permResult = permissionChecker_->CheckToolPermission(callerInfo, tool);
    if (!permResult.granted) {
        TAG_LOGE(TAG, "Permission denied: %{public}s", permResult.reason.c_str());
        return ErrCode::ERR_PERMISSION_DENIED;
    }

    // 3. 验证参数
    if (!tool.inputSchema.isNull()) {
        ValidationResult validationResult = SchemaValidator::Validate(arguments, tool.inputSchema);
        if (!validationResult.valid) {
            TAG_LOGE(TAG, "Parameter validation failed for tool %{public}s:", name.c_str());
            for (const auto& error : validationResult.errors) {
                TAG_LOGE(TAG, "  - %{public}s", error.c_str());
            }
            return ErrCode::ERR_INVALID_ARGS;
        }

        // 输出警告信息
        for (const auto& warning : validationResult.warnings) {
            TAG_LOGW(TAG, "Parameter validation warning: %{public}s", warning.c_str());
        }
    }

    // 5. 检查并发任务数限制
    {
        std::lock_guard<std::mutex> lock(tasksMutex_);
        if (tasks_.size() >= MAX_CONCURRENT_TASKS) {
            TAG_LOGE(TAG, "Too many concurrent tasks");
            return ErrCode::ERR_TOO_MANY_TASKS;
        }
    }

    // 6. 检查应用进程数限制
    if (!resourceManager_->CanCreateTask(callerInfo.bundleName)) {
        TAG_LOGE(TAG, "Cannot create task for app: %{public}s", callerInfo.bundleName.c_str());
        return ErrCode::ERR_TOO_MANY_TASKS;
    }

    // 7. 创建执行上下文
    auto context = processManager_->CreateExecutionContext(name, ExecutionMode::ASYNC);
    context->arguments = arguments;
    context->timeoutMs = options.timeoutMs > 0 ? options.timeoutMs : tool.timeoutMs;
    context->workingDir = options.workingDir;
    context->taskId = GenerateTaskId();

    // 8. 启动进程
    if (!processManager_->SpawnProcess(tool, context)) {
        TAG_LOGE(TAG, "Failed to spawn process for tool %{public}s", name.c_str());
        return ErrCode::ERR_IO_ERROR;
    }

    // 9. 注册到 I/O Monitor
    ioMonitor_->RegisterTask(context);

    // 10. 注册到各管理器
    processMonitor_->RegisterProcess(context->processId, context->taskId);
    resourceManager_->RegisterTask(context->taskId, context->processId, callerInfo.bundleName);
    timeoutManager_->AddTimeoutTask(context->taskId, context, context->timeoutMs);
    AddTask(context->taskId, context);

    // 11. 填充会话信息
    session.sessionId = context->taskId;
    session.toolName = name;
    session.status = "running";
    session.startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        context->startTime.time_since_epoch()).count();
    session.endTime = 0;
    session.result = nullptr;

    TAG_LOGI(TAG, "Tool started: %{public}s, sessionId=%{public}s, background=%{public}d",
             name.c_str(), session.sessionId.c_str(), options.background);

    // 12. 如果 background=false，等待 yieldMs 时间
    if (!options.background) {
        TAG_LOGI(TAG, "Waiting for tool completion in foreground: %{public}s, yieldMs=%{public}u",
                 name.c_str(), options.yieldMs);

        // 等待进程完成或超时
        int exitCode = processManager_->WaitForProcess(context, options.yieldMs);

        if (exitCode >= 0) {
            // 进程已完成
            session.status = (exitCode == 0) ? "completed" : "failed";
            session.endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count();

            // 创建执行结果
            session.result = new (std::nothrow) ExecutionResult();
            if (session.result != nullptr) {
                session.result->taskId = context->taskId;
                session.result->exitCode = exitCode;
                session.result->stdout = context->stdoutBuffer;
                session.result->stderr = context->stderrBuffer;
                session.result->signal = context->signal;
                session.result->timedOut = false;

                auto endTime = std::chrono::steady_clock::now();
                session.result->executionTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    endTime - context->startTime).count();
            }

            // 清理资源
            timeoutManager_->CancelTimeoutTask(context->taskId);
            RemoveTask(context->taskId);
            processMonitor_->UnregisterProcess(context->processId);
            resourceManager_->UnregisterTask(context->taskId);
            ioMonitor_->UnregisterTask(context->taskId);

            TAG_LOGI(TAG, "Tool completed in foreground: %{public}s, exitCode=%{public}d, time=%{public}llu ms",
                     name.c_str(), exitCode,
                     session.result ? session.result->executionTimeMs : 0ULL);
        } else {
            // 超时，进程仍在运行，已自动转入后台
            TAG_LOGI(TAG, "Tool moved to background after yieldMs timeout: %{public}s",
                     name.c_str());
        }
    } else {
        // background=true，立即返回
        TAG_LOGI(TAG, "Tool running in background: %{public}s", name.c_str());
    }

    return ErrCode::OK;
}

int32_t CLIManagerService::SubscribeSession(
    const std::string& sessionId,
    const sptr<ICLICallback>& callback)
{
    if (!running_.load()) {
        TAG_LOGE(TAG, "Service not ready");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    // 检查任务是否存在
    auto context = GetTask(sessionId);
    if (!context) {
        TAG_LOGW(TAG, "Session not found: %{public}s", sessionId.c_str());
        return ErrCode::ERR_TASK_NOT_FOUND;
    }

    // 检查回调是否有效
    if (!callback) {
        TAG_LOGE(TAG, "Invalid callback for session: %{public}s", sessionId.c_str());
        return ErrCode::ERR_INVALID_ARGS;
    }

    // 注册回调到事件分发器
    eventDispatcher_->RegisterCallback(sessionId, callback);

    TAG_LOGI(TAG, "Callback registered for session: %{public}s", sessionId.c_str());
    return ErrCode::OK;
}

int32_t CLIManagerService::CancelSession(const std::string& sessionId)
{
    if (!running_.load()) {
        TAG_LOGE(TAG, "Service not ready");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    auto context = GetTask(sessionId);
    if (!context) {
        TAG_LOGW(TAG, "Session not found: %{public}s", sessionId.c_str());
        return ErrCode::ERR_TASK_NOT_FOUND;
    }

    TAG_LOGI(TAG, "Cancelling session: %{public}s", sessionId.c_str());

    // 取消超时监控
    timeoutManager_->CancelTimeoutTask(sessionId);

    // 注销事件回调
    eventDispatcher_->UnregisterCallback(sessionId);

    // 终止进程
    if (context->processId > 0) {
        TAG_LOGI(TAG, "Terminating process: pid=%{public}d", context->processId);

        processManager_->TerminateProcess(context->processId, SIGTERM);

        // 等待 3 秒
        std::this_thread::sleep_for(std::chrono::seconds(3));

        // 如果仍在运行，强制终止
        if (context->isRunning.load()) {
            TAG_LOGW(TAG, "Force killing process: pid=%{public}d",
                     context->processId);
            processManager_->TerminateProcess(context->processId, SIGKILL);
        }
    }

    // 发送取消事件
    eventDispatcher_->DispatchErrorEvent(sessionId, ErrCode::ERR_TASK_CANCELLED,
                                         "Session cancelled by user");

    // 清理资源
    RemoveTask(sessionId);

    // 注销进程监控
    if (context->processId > 0) {
        processMonitor_->UnregisterProcess(context->processId);
    }

    // 注销资源管理
    resourceManager_->UnregisterTask(sessionId);

    // 注销 I/O 监控
    ioMonitor_->UnregisterTask(sessionId);

    TAG_LOGI(TAG, "Session cancelled: %{public}s", sessionId.c_str());
    return ErrCode::OK;
}

int32_t CLIManagerService::QuerySessionStatus(const std::string& sessionId, SessionInfo& session)
{
    if (!running_.load()) {
        TAG_LOGE(TAG, "Service not ready");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    auto context = GetTask(sessionId);
    if (!context) {
        TAG_LOGW(TAG, "Session not found: %{public}s", sessionId.c_str());
        return ErrCode::ERR_TASK_NOT_FOUND;
    }

    // 清理旧的结果指针
    delete session.result;
    session.result = nullptr;

    // 填充会话信息
    session.sessionId = sessionId;
    session.toolName = context->toolName;
    session.startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        context->startTime.time_since_epoch()).count();

    // 确定状态
    if (context->isCompleted.load()) {
        if (context->signal != 0) {
            session.status = "failed";
        } else if (context->exitCode == 0) {
            session.status = "completed";
        } else {
            session.status = "failed";
        }

        // 设置结束时间
        session.endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            context->endTime.time_since_epoch()).count();

        // 如果已完成，创建执行结果
        session.result = new (std::nothrow) ExecutionResult();
        if (session.result != nullptr) {
            session.result->taskId = sessionId;
            session.result->exitCode = context->exitCode;
            session.result->stdout = context->stdoutBuffer;
            session.result->stderr = context->stderrBuffer;
            session.result->signal = context->signal;
            session.result->timedOut = false;

            auto endTime = context->endTime;
            if (context->endTime == context->startTime) {
                endTime = std::chrono::steady_clock::now();
            }
            session.result->executionTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                endTime - context->startTime).count();
        }
    } else if (context->isRunning.load()) {
        session.status = "running";
        session.endTime = 0;
    } else {
        session.status = "cancelled";
        session.endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
    }

    TAG_LOGD(TAG, "Session status queried: %{public}s, status=%{public}s",
             sessionId.c_str(), session.status.c_str());

    return ErrCode::OK;
}

std::shared_ptr<RegistryManager> CLIManagerService::GetRegistryManager() const
{
    return registryManager_;
}

std::shared_ptr<ProcessManager> CLIManagerService::GetProcessManager() const
{
    return processManager_;
}

std::shared_ptr<EventDispatcher> CLIManagerService::GetEventDispatcher() const
{
    return eventDispatcher_;
}

std::shared_ptr<PermissionChecker> CLIManagerService::GetPermissionChecker() const
{
    return permissionChecker_;
}

std::string CLIManagerService::GenerateTaskId()
{
    uint64_t id = taskIdCounter_.fetch_add(1);
    auto now = std::chrono::system_clock::now();
    uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()).count();

    char buffer[64];
    snprintf(buffer, sizeof(buffer), "task_%llu_%llu", timestamp, id);
    return std::string(buffer);
}

void CLIManagerService::AddTask(const std::string& taskId,
                               std::shared_ptr<ExecutionContext> context)
{
    std::lock_guard<std::mutex> lock(tasksMutex_);
    tasks_[taskId] = context;
}

void CLIManagerService::RemoveTask(const std::string& taskId)
{
    std::lock_guard<std::mutex> lock(tasksMutex_);
    tasks_.erase(taskId);
}

std::shared_ptr<ExecutionContext> CLIManagerService::GetTask(const std::string& taskId)
{
    std::lock_guard<std::mutex> lock(tasksMutex_);
    auto it = tasks_.find(taskId);
    if (it != tasks_.end()) {
        return it->second;
    }
    return nullptr;
}

} // namespace CliMgr
} // namespace OHOS
