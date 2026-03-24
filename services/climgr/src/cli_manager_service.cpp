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
        case QUERY_TOOL_BY_NAME:
            return QueryToolByNameInternal(data, reply);
        case EXEC_TOOL:
            return ExecToolInternal(data, reply);
        case EXEC_TOOL_ASYNC:
            return ExecToolAsyncInternal(data, reply);
        case CANCEL_TASK:
            return CancelTaskInternal(data, reply);
        case QUERY_TASK_STATUS:
            return QueryTaskStatusInternal(data, reply);
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

int32_t CLIManagerService::QueryToolByName(const std::string& name, ToolInfo& tool)
{
    if (!running_.load()) {
        TAG_LOGE(TAG, "Service not ready");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    return registryManager_->QueryToolByName(name, tool);
}

int32_t CLIManagerService::ExecTool(
    const std::string& name,
    const Json::Value& arguments,
    const ExecOptions& options,
    ExecutionResult& result)
{
    if (!running_.load()) {
        TAG_LOGE(TAG, "Service not ready");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    // 查询工具
    ToolInfo tool;
    int32_t ret = registryManager_->QueryToolByName(name, tool);
    if (ret != ErrCode::OK) {
        TAG_LOGE(TAG, "Tool not found: %{public}s", name.c_str());
        return ret;
    }

    // 校验权限
    CallerInfo callerInfo;  // TODO: 从 IPC 上下文获取
    PermissionCheckResult permResult = permissionChecker_->CheckToolPermission(callerInfo, tool);
    if (!permResult.granted) {
        TAG_LOGE(TAG, "Permission denied: %{public}s", permResult.reason.c_str());
        return ErrCode::ERR_PERMISSION_DENIED;
    }

    // 创建执行上下文
    auto context = processManager_->CreateExecutionContext(name, ExecutionMode::SYNC);
    context->arguments = arguments;
    context->timeoutMs = options.timeoutMs > 0 ? options.timeoutMs : tool.timeoutMs;
    context->workingDir = options.workingDir;
    context->taskId = GenerateTaskId();

    // 启动进程
    if (!processManager_->SpawnProcess(tool, context)) {
        TAG_LOGE(TAG, "Failed to spawn process for tool %{public}s", name.c_str());
        return ErrCode::ERR_IO_ERROR;
    }

    // 等待进程完成
    int exitCode = processManager_->WaitForProcess(context, context->timeoutMs);

    // 收集结果
    result.taskId = context->taskId;
    result.exitCode = exitCode;
    result.stdout = context->stdoutBuffer;
    result.stderr = context->stderrBuffer;
    result.signal = context->signal;
    result.timedOut = (exitCode == 0 && context->isRunning.load());

    auto endTime = std::chrono::steady_clock::now();
    result.executionTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTime - context->startTime).count();

    // 清理
    RemoveTask(context->taskId);

    TAG_LOGI(TAG, "Tool executed: %{public}s, exitCode=%{public}d, time=%{public}llu ms",
             name.c_str(), exitCode, result.executionTimeMs);

    return ErrCode::OK;
}

int32_t CLIManagerService::ExecToolAsync(
    const std::string& name,
    const Json::Value& arguments,
    const sptr<ICLICallback>& callback,
    const ExecOptions& options,
    std::string& taskId)
{
    if (!running_.load()) {
        TAG_LOGE(TAG, "Service not ready");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    // 查询工具
    ToolInfo tool;
    int32_t ret = registryManager_->QueryToolByName(name, tool);
    if (ret != ErrCode::OK) {
        TAG_LOGE(TAG, "Tool not found: %{public}s", name.c_str());
        return ret;
    }

    // 校验权限
    CallerInfo callerInfo;  // TODO: 从 IPC 上下文获取
    PermissionCheckResult permResult = permissionChecker_->CheckToolPermission(callerInfo, tool);
    if (!permResult.granted) {
        TAG_LOGE(TAG, "Permission denied: %{public}s", permResult.reason.c_str());
        return ErrCode::ERR_PERMISSION_DENIED;
    }

    // 检查并发任务数限制
    {
        std::lock_guard<std::mutex> lock(tasksMutex_);
        if (tasks_.size() >= MAX_CONCURRENT_TASKS) {
            TAG_LOGE(TAG, "Too many concurrent tasks");
            return ErrCode::ERR_TOO_MANY_TASKS;
        }
    }

    // 检查应用进程数限制
    CallerInfo callerInfo;  // TODO: 从 IPC 上下文获取
    if (!resourceManager_->CanCreateTask(callerInfo.bundleName)) {
        TAG_LOGE(TAG, "Cannot create task for app: %{public}s", callerInfo.bundleName.c_str());
        return ErrCode::ERR_TOO_MANY_TASKS;
    }

    // 创建执行上下文
    auto context = processManager_->CreateExecutionContext(name, ExecutionMode::ASYNC);
    context->arguments = arguments;
    context->timeoutMs = options.timeoutMs > 0 ? options.timeoutMs : tool.timeoutMs;
    context->workingDir = options.workingDir;
    context->taskId = GenerateTaskId();
    context->callback = callback.GetRefPtr();

    // 注册回调
    eventDispatcher_->RegisterCallback(context->taskId, callback);

    // 启动进程
    if (!processManager_->SpawnProcess(tool, context)) {
        TAG_LOGE(TAG, "Failed to spawn process for tool %{public}s", name.c_str());
        eventDispatcher_->UnregisterCallback(context->taskId);
        return ErrCode::ERR_IO_ERROR;
    }

    // 注册到 ProcessMonitor
    processMonitor_->RegisterProcess(context->processId, context->taskId);

    // 注册到 ResourceManager
    resourceManager_->RegisterTask(context->taskId, context->processId, callerInfo.bundleName);

    // 注册到 TimeoutManager
    timeoutManager_->AddTimeoutTask(context->taskId, context, context->timeoutMs);

    // 添加到任务列表
    AddTask(context->taskId, context);

    taskId = context->taskId;

    TAG_LOGI(TAG, "Tool started asynchronously: %{public}s, taskId=%{public}s",
             name.c_str(), taskId.c_str());

    return ErrCode::OK;
}

int32_t CLIManagerService::CancelTask(const std::string& taskId)
{
    if (!running_.load()) {
        TAG_LOGE(TAG, "Service not ready");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    auto context = GetTask(taskId);
    if (!context) {
        TAG_LOGW(TAG, "Task not found: %{public}s", taskId.c_str());
        return ErrCode::ERR_TASK_NOT_FOUND;
    }

    // 取消超时监控
    timeoutManager_->CancelTimeoutTask(taskId);

    // 终止进程
    if (context->processId > 0) {
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
    eventDispatcher_->DispatchErrorEvent(taskId, ErrCode::ERR_TASK_CANCELLED,
                                         "Task cancelled by user");

    // 清理资源
    RemoveTask(taskId);

    // 注销进程监控
    if (context->processId > 0) {
        processMonitor_->UnregisterProcess(context->processId);
    }

    // 注销资源管理
    resourceManager_->UnregisterTask(taskId);

    TAG_LOGI(TAG, "Task cancelled: %{public}s", taskId.c_str());
    return ErrCode::OK;
}

int32_t CLIManagerService::QueryTaskStatus(const std::string& taskId, TaskStatus& status)
{
    if (!running_.load()) {
        TAG_LOGE(TAG, "Service not ready");
        return ErrCode::ERR_SERVICE_NOT_READY;
    }

    auto context = GetTask(taskId);
    if (!context) {
        TAG_LOGW(TAG, "Task not found: %{public}s", taskId.c_str());
        return ErrCode::ERR_TASK_NOT_FOUND;
    }

    status.taskId = taskId;
    status.toolName = context->toolName;

    if (context->isCompleted.load()) {
        if (context->signal != 0) {
            status.status = "failed";
        } else if (context->exitCode == 0) {
            status.status = "completed";
        } else {
            status.status = "failed";
        }
    } else if (context->isRunning.load()) {
        status.status = "running";
    } else {
        status.status = "cancelled";
    }

    status.startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        context->startTime.time_since_epoch()).count();

    if (context->isCompleted.load()) {
        status.endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            context->endTime.time_since_epoch()).count();
    }

    status.exitCode = context->exitCode;

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
