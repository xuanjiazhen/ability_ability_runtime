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

// 在 OnStart 中初始化新组件
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
    processMonitor_ = nullptr;  // 将在 ProcessMonitor 中处理

    // 创建 Permission Checker
    permissionChecker_ = std::make_shared<PermissionChecker>();
    if (!permissionChecker_->Initialize()) {
        TAG_LOGE(TAG, "Failed to initialize PermissionChecker");
        return false;
    }

    TAG_LOGI(TAG, "All components initialized successfully");
    return true;
}

// RPC 处理方法
int32_t CLIManagerService::QueryToolsInternal(MessageParcel& data, MessageParcel& reply)
{
    std::vector<ToolInfo> tools;
    int32_t ret = QueryTools(tools);

    // 写入错误码
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_DATA;
    }

    if (ret != ErrCode::OK) {
        return ERR_OK;
    }

    // 写入工具数量
    if (!reply.WriteInt32(tools.size())) {
        return ERR_INVALID_DATA;
    }

    // 写入每个工具
    for (const auto& tool : tools) {
        if (!WriteToolInfo(reply, tool)) {
            TAG_LOGE(TAG, "Failed to write tool info");
            return ERR_INVALID_DATA;
        }
    }

    return ERR_OK;
}

int32_t CLIManagerService::QueryToolByNameInternal(MessageParcel& data, MessageParcel& reply)
{
    std::string name = data.ReadString();

    ToolInfo tool;
    int32_t ret = QueryToolByName(name, tool);

    // 写入错误码
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_DATA;
    }

    if (ret != ErrCode::OK) {
        return ERR_OK;
    }

    // 写入工具信息
    if (!WriteToolInfo(reply, tool)) {
        return ERR_INVALID_DATA;
    }

    return ERR_OK;
}

int32_t CLIManagerService::ExecToolInternal(MessageParcel& data, MessageParcel& reply)
{
    // 读取参数
    std::string name = data.ReadString();

    std::string argsStr = data.ReadString();
    Json::Value arguments;
    Json::Reader reader;
    reader.parse(argsStr, arguments);

    uint32_t mode = data.ReadUint32();
    uint32_t timeoutMs = data.ReadUint32();
    std::string workingDir = data.ReadString();

    ExecOptions options;
    options.mode = static_cast<ExecutionMode>(mode);
    options.timeoutMs = timeoutMs;
    options.workingDir = workingDir;

    // 执行工具
    ExecutionResult result;
    int32_t ret = ExecTool(name, arguments, options, result);

    // 写入错误码
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_DATA;
    }

    if (ret != ErrCode::OK) {
        return ERR_OK;
    }

    // 写入执行结果
    if (!reply.WriteString(result.taskId)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(result.exitCode)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteString(result.stdout)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteString(result.stderr)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(result.signal)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteBool(result.timedOut)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteUint64(result.executionTimeMs)) {
        return ERR_INVALID_DATA;
    }

    return ERR_OK;
}

int32_t CLIManagerService::ExecToolAsyncInternal(MessageParcel& data, MessageParcel& reply)
{
    // 读取参数
    std::string name = data.ReadString();

    std::string argsStr = data.ReadString();
    Json::Value arguments;
    Json::Reader reader;
    reader.parse(argsStr, arguments);

    sptr<IRemoteObject> callbackObj = data.ReadRemoteObject();
    sptr<ICLICallback> callback = iface_cast<ICLICallback>(callbackObj);

    uint32_t mode = data.ReadUint32();
    uint32_t timeoutMs = data.ReadUint32();
    std::string workingDir = data.ReadString();

    ExecOptions options;
    options.mode = static_cast<ExecutionMode>(mode);
    options.timeoutMs = timeoutMs;
    options.workingDir = workingDir;

    // 异步执行工具
    std::string taskId;
    int32_t ret = ExecToolAsync(name, arguments, callback, options, taskId);

    // 写入错误码
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_DATA;
    }

    if (ret != ErrCode::OK) {
        return ERR_OK;
    }

    // 写入任务 ID
    if (!reply.WriteString(taskId)) {
        return ERR_INVALID_DATA;
    }

    return ERR_OK;
}

int32_t CLIManagerService::CancelTaskInternal(MessageParcel& data, MessageParcel& reply)
{
    std::string taskId = data.ReadString();

    int32_t ret = CancelTask(taskId);

    return reply.WriteInt32(ret) ? ERR_OK : ERR_INVALID_DATA;
}

int32_t CLIManagerService::QueryTaskStatusInternal(MessageParcel& data, MessageParcel& reply)
{
    std::string taskId = data.ReadString();

    TaskStatus status;
    int32_t ret = QueryTaskStatus(taskId, status);

    // 写入错误码
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_DATA;
    }

    if (ret != ErrCode::OK) {
        return ERR_OK;
    }

    // 写入状态
    if (!reply.WriteString(status.taskId)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteString(status.toolName)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteString(status.status)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteUint64(status.startTime)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteUint64(status.endTime)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteInt32(status.exitCode)) {
        return ERR_INVALID_DATA;
    }

    return ERR_OK;
}

bool CLIManagerService::WriteToolInfo(MessageParcel& parcel, const ToolInfo& tool)
{
    // 写入基本字段
    if (!parcel.WriteString(tool.name)) {
        return false;
    }
    if (!parcel.WriteString(tool.version)) {
        return false;
    }
    if (!parcel.WriteString(tool.description)) {
        return false;
    }
    if (!parcel.WriteString(tool.executablePath)) {
        return false;
    }

    // 写入类型
    if (!parcel.WriteUint32(static_cast<uint32_t>(tool.type))) {
        return false;
    }

    // 写入权限列表
    if (!parcel.WriteInt32(tool.permissions.size())) {
        return false;
    }
    for (const auto& perm : tool.permissions) {
        if (!parcel.WriteString(perm)) {
            return false;
        }
    }

    // 写入 JSON Schema
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string inputSchemaStr = Json::writeString(builder, tool.inputSchema);
    std::string outputSchemaStr = Json::writeString(builder, tool.outputSchema);

    if (!parcel.WriteString(inputSchemaStr)) {
        return false;
    }
    if (!parcel.WriteString(outputSchemaStr)) {
        return false;
    }

    // 写入超时时间
    if (!parcel.WriteUint32(tool.timeoutMs)) {
        return false;
    }

    // 写入事件类型
    if (!parcel.WriteInt32(tool.eventTypes.size())) {
        return false;
    }
    for (const auto& event : tool.eventTypes) {
        if (!parcel.WriteString(event)) {
            return false;
        }
    }

    // 写入注册时间和启用状态
    if (!parcel.WriteUint64(tool.registeredTime)) {
        return false;
    }
    if (!parcel.WriteBool(tool.enabled)) {
        return false;
    }

    return true;
}

} // namespace CliMgr
} // namespace OHOS
