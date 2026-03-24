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

int32_t CLIManagerService::QueryToolSummariesInternal(MessageParcel& data, MessageParcel& reply)
{
    std::vector<ToolSummary> summaries;
    int32_t ret = QueryToolSummaries(summaries);

    // 写入错误码
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(TAG, "Failed to write error code");
        return ERR_INVALID_DATA;
    }

    if (ret != ErrCode::OK) {
        return ERR_OK;
    }

    // 写入工具摘要数量
    if (!reply.WriteInt32(summaries.size())) {
        TAG_LOGE(TAG, "Failed to write summaries count");
        return ERR_INVALID_DATA;
    }

    // 写入每个工具摘要（只有 3 个字段）
    for (const auto& summary : summaries) {
        if (!reply.WriteString(summary.name)) {
            TAG_LOGE(TAG, "Failed to write summary name");
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteString(summary.version)) {
            TAG_LOGE(TAG, "Failed to write summary version");
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteString(summary.description)) {
            TAG_LOGE(TAG, "Failed to write summary description");
            return ERR_INVALID_DATA;
        }
    }

    TAG_LOGI(TAG, "QueryToolSummariesInternal: returned %{public}zu summaries", summaries.size());
    return ERR_OK;
}

int32_t CLIManagerService::GetToolInfoByNameInternal(MessageParcel& data, MessageParcel& reply)
{
    // 读取工具名称
    std::string name = data.ReadString();
    if (name.empty()) {
        TAG_LOGE(TAG, "Failed to read tool name or name is empty");
        reply.WriteInt32(ErrCode::ERR_INVALID_VALUE);
        return ERR_OK;
    }

    // 查询工具信息
    ToolInfo tool;
    int32_t ret = GetToolInfoByName(name, tool);

    // 写入错误码
    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(TAG, "Failed to write error code");
        return ERR_INVALID_DATA;
    }

    if (ret != ErrCode::OK) {
        TAG_LOGW(TAG, "GetToolInfoByNameInternal: tool '%{public}s' not found", name.c_str());
        return ERR_OK;
    }

    // 写入工具信息
    if (!WriteToolInfo(reply, tool)) {
        TAG_LOGE(TAG, "Failed to write tool info for '%{public}s'", name.c_str());
        return ERR_INVALID_DATA;
    }

    TAG_LOGI(TAG, "GetToolInfoByNameInternal: returned tool '%{public}s'", name.c_str());
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

    // 读取新的选项参数
    bool background = data.ReadBool();
    uint32_t yieldMs = data.ReadUint32();
    uint32_t timeoutMs = data.ReadUint32();
    std::string workingDir = data.ReadString();

    std::string envStr = data.ReadString();
    Json::Value env;
    if (!envStr.empty()) {
        reader.parse(envStr, env);
    }

    ExecOptions options;
    options.background = background;
    options.yieldMs = yieldMs;
    options.timeoutMs = timeoutMs;
    options.workingDir = workingDir;
    options.env = env;

    // 执行工具
    SessionInfo session;
    int32_t ret = ExecTool(name, arguments, options, session);

    // 写入错误码
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_DATA;
    }

    if (ret != ErrCode::OK) {
        return ERR_OK;
    }

    // 写入会话信息
    if (!reply.WriteString(session.sessionId)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteString(session.toolName)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteString(session.status)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteUint64(session.startTime)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteUint64(session.endTime)) {
        return ERR_INVALID_DATA;
    }

    // 写入执行结果（如果有）
    bool hasResult = (session.result != nullptr);
    if (!reply.WriteBool(hasResult)) {
        return ERR_INVALID_DATA;
    }

    if (hasResult && session.result != nullptr) {
        if (!reply.WriteString(session.result->taskId)) {
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteInt32(session.result->exitCode)) {
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteString(session.result->stdout)) {
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteString(session.result->stderr)) {
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteInt32(session.result->signal)) {
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteBool(session.result->timedOut)) {
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteUint64(session.result->executionTimeMs)) {
            return ERR_INVALID_DATA;
        }
    }

    return ERR_OK;
}

int32_t CLIManagerService::SubscribeSessionInternal(MessageParcel& data, MessageParcel& reply)
{
    // 读取参数
    std::string sessionId = data.ReadString();

    sptr<IRemoteObject> callbackObj = data.ReadRemoteObject();
    sptr<ICLICallback> callback = iface_cast<ICLICallback>(callbackObj);

    // 订阅会话事件
    int32_t ret = SubscribeSession(sessionId, callback);

    // 写入错误码
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_DATA;
    }

    return ERR_OK;
}

int32_t CLIManagerService::CancelSessionInternal(MessageParcel& data, MessageParcel& reply)
{
    std::string sessionId = data.ReadString();

    int32_t ret = CancelSession(sessionId);

    return reply.WriteInt32(ret) ? ERR_OK : ERR_INVALID_DATA;
}

int32_t CLIManagerService::QuerySessionStatusInternal(MessageParcel& data, MessageParcel& reply)
{
    std::string sessionId = data.ReadString();

    SessionInfo session;
    int32_t ret = QuerySessionStatus(sessionId, session);

    // 写入错误码
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_DATA;
    }

    if (ret != ErrCode::OK) {
        return ERR_OK;
    }

    // 写入会话信息
    if (!reply.WriteString(session.sessionId)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteString(session.toolName)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteString(session.status)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteUint64(session.startTime)) {
        return ERR_INVALID_DATA;
    }
    if (!reply.WriteUint64(session.endTime)) {
        return ERR_INVALID_DATA;
    }

    // 写入执行结果（如果有）
    bool hasResult = (session.result != nullptr);
    if (!reply.WriteBool(hasResult)) {
        return ERR_INVALID_DATA;
    }

    if (hasResult && session.result != nullptr) {
        if (!reply.WriteString(session.result->taskId)) {
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteInt32(session.result->exitCode)) {
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteString(session.result->stdout)) {
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteString(session.result->stderr)) {
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteInt32(session.result->signal)) {
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteBool(session.result->timedOut)) {
            return ERR_INVALID_DATA;
        }
        if (!reply.WriteUint64(session.result->executionTimeMs)) {
            return ERR_INVALID_DATA;
        }
    }

    return ERR_OK;
}

// ============================================================================
// 动态工具注册 RPC 处理
// ============================================================================

int32_t CLIManagerService::RegisterToolInternal(MessageParcel& data, MessageParcel& reply)
{
    // 读取工具信息
    ToolInfo tool;
    if (!ReadToolInfo(data, tool)) {
        HILOG_ERROR(TAG, "Failed to read tool info");
        return reply.WriteInt32(ErrCode::ERR_INVALID_ARGS) ? ERR_OK : ERR_INVALID_DATA;
    }

    // 读取持久化标志
    bool persist = data.ReadBool();

    // 权限检查
    if (!CheckRegisterPermission()) {
        HILOG_ERROR(TAG, "Permission denied for tool registration");
        return reply.WriteInt32(ErrCode::ERR_PERMISSION_DENIED) ? ERR_OK : ERR_INVALID_DATA;
    }

    // 注册工具
    int32_t ret = RegisterTool(tool, persist);

    return reply.WriteInt32(ret) ? ERR_OK : ERR_INVALID_DATA;
}

int32_t CLIManagerService::RegisterToolsInternal(MessageParcel& data, MessageParcel& reply)
{
    // 读取工具数量
    int32_t toolCount = data.ReadInt32();
    if (toolCount <= 0 || toolCount > 1000) {  // 限制批量注册数量
        HILOG_ERROR(TAG, "Invalid tool count: %{public}d", toolCount);
        return reply.WriteInt32(ErrCode::ERR_INVALID_ARGS) ? ERR_OK : ERR_INVALID_DATA;
    }

    // 读取工具列表
    std::vector<ToolInfo> tools;
    tools.reserve(toolCount);
    for (int32_t i = 0; i < toolCount; ++i) {
        ToolInfo tool;
        if (!ReadToolInfo(data, tool)) {
            HILOG_ERROR(TAG, "Failed to read tool info at index %{public}d", i);
            return reply.WriteInt32(ErrCode::ERR_INVALID_ARGS) ? ERR_OK : ERR_INVALID_DATA;
        }
        tools.push_back(tool);
    }

    // 读取持久化标志
    bool persist = data.ReadBool();

    // 权限检查
    if (!CheckRegisterPermission()) {
        HILOG_ERROR(TAG, "Permission denied for tools registration");
        return reply.WriteInt32(ErrCode::ERR_PERMISSION_DENIED) ? ERR_OK : ERR_INVALID_DATA;
    }

    // 批量注册
    int32_t ret = RegisterTools(tools, persist);

    return reply.WriteInt32(ret) ? ERR_OK : ERR_INVALID_DATA;
}

int32_t CLIManagerService::UpdateToolInternal(MessageParcel& data, MessageParcel& reply)
{
    // 读取工具信息
    ToolInfo tool;
    if (!ReadToolInfo(data, tool)) {
        HILOG_ERROR(TAG, "Failed to read tool info");
        return reply.WriteInt32(ErrCode::ERR_INVALID_ARGS) ? ERR_OK : ERR_INVALID_DATA;
    }

    // 读取持久化标志
    bool persist = data.ReadBool();

    // 权限检查
    if (!CheckRegisterPermission()) {
        HILOG_ERROR(TAG, "Permission denied for tool update");
        return reply.WriteInt32(ErrCode::ERR_PERMISSION_DENIED) ? ERR_OK : ERR_INVALID_DATA;
    }

    // 更新工具
    int32_t ret = UpdateTool(tool, persist);

    return reply.WriteInt32(ret) ? ERR_OK : ERR_INVALID_DATA;
}

int32_t CLIManagerService::UnregisterToolInternal(MessageParcel& data, MessageParcel& reply)
{
    // 读取工具名称
    std::string toolName = data.ReadString();
    if (toolName.empty()) {
        HILOG_ERROR(TAG, "Tool name is empty");
        return reply.WriteInt32(ErrCode::ERR_INVALID_ARGS) ? ERR_OK : ERR_INVALID_DATA;
    }

    // 读取持久化标志
    bool persist = data.ReadBool();

    // 权限检查
    if (!CheckRegisterPermission()) {
        HILOG_ERROR(TAG, "Permission denied for tool unregistration");
        return reply.WriteInt32(ErrCode::ERR_PERMISSION_DENIED) ? ERR_OK : ERR_INVALID_DATA;
    }

    // 注销工具
    int32_t ret = UnregisterTool(toolName, persist);

    return reply.WriteInt32(ret) ? ERR_OK : ERR_INVALID_DATA;
}

int32_t CLIManagerService::QueryToolInternal(MessageParcel& data, MessageParcel& reply)
{
    // 读取工具名称
    std::string toolName = data.ReadString();
    if (toolName.empty()) {
        HILOG_ERROR(TAG, "Tool name is empty");
        return reply.WriteInt32(ErrCode::ERR_INVALID_ARGS) ? ERR_OK : ERR_INVALID_DATA;
    }

    // 查询工具
    ToolInfo tool;
    int32_t ret = QueryTool(toolName, tool);

    // 写入错误码
    if (!reply.WriteInt32(ret)) {
        return ERR_INVALID_DATA;
    }

    if (ret != ErrCode::OK) {
        return ERR_OK;
    }

    // 写入工具信息
    if (!WriteToolInfo(reply, tool)) {
        HILOG_ERROR(TAG, "Failed to write tool info");
        return ERR_INVALID_DATA;
    }

    return ERR_OK;
}

int32_t CLIManagerService::ReloadRegistryInternal(MessageParcel& data, MessageParcel& reply)
{
    // 权限检查
    if (!CheckRegisterPermission()) {
        HILOG_ERROR(TAG, "Permission denied for registry reload");
        return reply.WriteInt32(ErrCode::ERR_PERMISSION_DENIED) ? ERR_OK : ERR_INVALID_DATA;
    }

    // 重新加载注册表
    int32_t ret = ReloadRegistry();

    return reply.WriteInt32(ret) ? ERR_OK : ERR_INVALID_DATA;
}

bool CLIManagerService::ReadToolInfo(MessageParcel& parcel, ToolInfo& tool)
{
    // 读取基本字段
    tool.name = parcel.ReadString();
    if (tool.name.empty()) {
        HILOG_ERROR(TAG, "Tool name is empty");
        return false;
    }

    tool.version = parcel.ReadString();
    tool.description = parcel.ReadString();
    tool.executablePath = parcel.ReadString();

    if (tool.executablePath.empty()) {
        HILOG_ERROR(TAG, "Executable path is empty");
        return false;
    }

    // 读取权限列表
    int32_t permCount = parcel.ReadInt32();
    if (permCount < 0 || permCount > 100) {
        HILOG_ERROR(TAG, "Invalid permission count: %{public}d", permCount);
        return false;
    }

    tool.permissions.clear();
    tool.permissions.reserve(permCount);
    for (int32_t i = 0; i < permCount; ++i) {
        std::string perm = parcel.ReadString();
        if (!perm.empty()) {
            tool.permissions.push_back(perm);
        }
    }

    // 读取 JSON Schema
    Json::Reader reader;
    std::string inputSchemaStr = parcel.ReadString();
    std::string outputSchemaStr = parcel.ReadString();

    if (!inputSchemaStr.empty()) {
        reader.parse(inputSchemaStr, tool.inputSchema);
    }
    if (!outputSchemaStr.empty()) {
        reader.parse(outputSchemaStr, tool.outputSchema);
    }

    // 读取超时时间
    tool.timeoutMs = parcel.ReadUint32();

    // 读取事件类型
    int32_t eventCount = parcel.ReadInt32();
    if (eventCount < 0 || eventCount > 100) {
        HILOG_ERROR(TAG, "Invalid event type count: %{public}d", eventCount);
        return false;
    }

    tool.eventTypes.clear();
    tool.eventTypes.reserve(eventCount);
    for (int32_t i = 0; i < eventCount; ++i) {
        std::string event = parcel.ReadString();
        if (!event.empty()) {
            tool.eventTypes.push_back(event);
        }
    }

    // 读取注册时间和启用状态
    tool.registeredTime = parcel.ReadUint64();
    tool.enabled = parcel.ReadBool();

    // 读取参数映射规则 (argMapping)
    std::string argMappingStr = parcel.ReadString();
    if (!argMappingStr.empty()) {
        if (!reader.parse(argMappingStr, tool.argMapping)) {
            HILOG_ERROR(TAG, "Failed to parse argMapping JSON: %{public}s", argMappingStr.c_str());
            return false;
        }
    }

    // 读取事件 Schema 定义 (eventSchemas)
    std::string eventSchemasStr = parcel.ReadString();
    if (!eventSchemasStr.empty()) {
        if (!reader.parse(eventSchemasStr, tool.eventSchemas)) {
            HILOG_ERROR(TAG, "Failed to parse eventSchemas JSON: %{public}s", eventSchemasStr.c_str());
            return false;
        }
    }

    // 读取子命令信息
    tool.hasSubcommands = parcel.ReadBool();

    if (tool.hasSubcommands) {
        // 读取子命令数量
        int32_t subcmdCount = parcel.ReadInt32();
        if (subcmdCount < 0 || subcmdCount > 100) {
            HILOG_ERROR(TAG, "Invalid subcommand count: %{public}d", subcmdCount);
            return false;
        }

        tool.subcommands.clear();
        for (int32_t i = 0; i < subcmdCount; ++i) {
            SubCommandInfo subcmd;

            // 读取子命令名称
            std::string name = parcel.ReadString();
            if (name.empty()) {
                HILOG_ERROR(TAG, "Subcommand name is empty");
                return false;
            }

            // 读取子命令描述
            subcmd.description = parcel.ReadString();

            // 读取子命令的 Schema
            std::string subInputSchemaStr = parcel.ReadString();
            std::string subOutputSchemaStr = parcel.ReadString();
            std::string subArgMappingStr = parcel.ReadString();
            std::string subEventSchemasStr = parcel.ReadString();

            if (!subInputSchemaStr.empty()) {
                if (!reader.parse(subInputSchemaStr, subcmd.inputSchema)) {
                    HILOG_ERROR(TAG, "Failed to parse subcommand inputSchema");
                    return false;
                }
            }
            if (!subOutputSchemaStr.empty()) {
                if (!reader.parse(subOutputSchemaStr, subcmd.outputSchema)) {
                    HILOG_ERROR(TAG, "Failed to parse subcommand outputSchema");
                    return false;
                }
            }
            if (!subArgMappingStr.empty()) {
                if (!reader.parse(subArgMappingStr, subcmd.argMapping)) {
                    HILOG_ERROR(TAG, "Failed to parse subcommand argMapping");
                    return false;
                }
            }
            if (!subEventSchemasStr.empty()) {
                if (!reader.parse(subEventSchemasStr, subcmd.eventSchemas)) {
                    HILOG_ERROR(TAG, "Failed to parse subcommand eventSchemas");
                    return false;
                }
            }

            // 读取子命令的事件类型
            int32_t subEventCount = parcel.ReadInt32();
            if (subEventCount < 0 || subEventCount > 100) {
                HILOG_ERROR(TAG, "Invalid subcommand event type count: %{public}d", subEventCount);
                return false;
            }

            subcmd.eventTypes.clear();
            subcmd.eventTypes.reserve(subEventCount);
            for (int32_t j = 0; j < subEventCount; ++j) {
                std::string event = parcel.ReadString();
                if (!event.empty()) {
                    subcmd.eventTypes.push_back(event);
                }
            }

            // 添加子命令到映射
            tool.subcommands[name] = subcmd;
        }

        HILOG_INFO(TAG, "Read %{public}d subcommands for tool %{public}s", subcmdCount, tool.name.c_str());
    }

    return true;
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

    // 写入参数映射规则 (argMapping)
    std::string argMappingStr = Json::writeString(builder, tool.argMapping);
    if (!parcel.WriteString(argMappingStr)) {
        HILOG_ERROR(TAG, "Failed to write argMapping");
        return false;
    }

    // 写入事件 Schema 定义 (eventSchemas)
    std::string eventSchemasStr = Json::writeString(builder, tool.eventSchemas);
    if (!parcel.WriteString(eventSchemasStr)) {
        HILOG_ERROR(TAG, "Failed to write eventSchemas");
        return false;
    }

    // 写入子命令信息
    if (!parcel.WriteBool(tool.hasSubcommands)) {
        HILOG_ERROR(TAG, "Failed to write hasSubcommands");
        return false;
    }

    if (tool.hasSubcommands) {
        // 写入子命令数量
        if (!parcel.WriteInt32(tool.subcommands.size())) {
            HILOG_ERROR(TAG, "Failed to write subcommand count");
            return false;
        }

        // 写入每个子命令
        for (const auto& [name, subcmd] : tool.subcommands) {
            if (!parcel.WriteString(name)) {
                HILOG_ERROR(TAG, "Failed to write subcommand name");
                return false;
            }

            if (!parcel.WriteString(subcmd.description)) {
                HILOG_ERROR(TAG, "Failed to write subcommand description");
                return false;
            }

            // 序列化子命令的 Schema
            std::string subInputSchemaStr = Json::writeString(builder, subcmd.inputSchema);
            std::string subOutputSchemaStr = Json::writeString(builder, subcmd.outputSchema);
            std::string subArgMappingStr = Json::writeString(builder, subcmd.argMapping);
            std::string subEventSchemasStr = Json::writeString(builder, subcmd.eventSchemas);

            if (!parcel.WriteString(subInputSchemaStr)) {
                HILOG_ERROR(TAG, "Failed to write subcommand inputSchema");
                return false;
            }
            if (!parcel.WriteString(subOutputSchemaStr)) {
                HILOG_ERROR(TAG, "Failed to write subcommand outputSchema");
                return false;
            }
            if (!parcel.WriteString(subArgMappingStr)) {
                HILOG_ERROR(TAG, "Failed to write subcommand argMapping");
                return false;
            }
            if (!parcel.WriteString(subEventSchemasStr)) {
                HILOG_ERROR(TAG, "Failed to write subcommand eventSchemas");
                return false;
            }

            // 序列化子命令的事件类型
            if (!parcel.WriteInt32(subcmd.eventTypes.size())) {
                HILOG_ERROR(TAG, "Failed to write subcommand eventTypes count");
                return false;
            }
            for (const auto& event : subcmd.eventTypes) {
                if (!parcel.WriteString(event)) {
                    HILOG_ERROR(TAG, "Failed to write subcommand eventType");
                    return false;
                }
            }
        }
    }

    return true;
}

} // namespace CliMgr
} // namespace OHOS
