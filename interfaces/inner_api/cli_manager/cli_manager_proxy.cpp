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

#include "cli_manager_proxy.h"
#include "cli_callback.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace CliMgr {

CLIManagerProxy::CLIManagerProxy(const sptr<IRemoteObject>& object)
    : IRemoteProxy<ICLIManagerService>(object)
{
    TAG_LOGI("CLIManagerProxy", "CLIManagerProxy created");
}

int32_t CLIManagerProxy::QueryTools(std::vector<ToolInfo>& tools)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken()) {
        TAG_LOGE("CLIManagerProxy", "Failed to write interface token");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 发送请求
    int32_t ret = Remote()->SendRequest(QUERY_TOOLS, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE("CLIManagerProxy", "SendRequest failed: %{public}d", ret);
        return ErrCode::ERR_IPC_ERROR;
    }

    // 读取错误码
    int32_t errorCode;
    if (!reply.ReadInt32(errorCode)) {
        return ErrCode::ERR_IPC_ERROR;
    }

    if (errorCode != ErrCode::OK) {
        return errorCode;
    }

    // 读取工具数量
    int32_t toolCount;
    if (!reply.ReadInt32(toolCount)) {
        return ErrCode::ERR_IPC_ERROR;
    }

    // 读取每个工具
    tools.clear();
    for (int32_t i = 0; i < toolCount; ++i) {
        ToolInfo tool;
        if (!ReadToolInfo(reply, tool)) {
            TAG_LOGE("CLIManagerProxy", "Failed to read tool info at index %{public}d", i);
            return ErrCode::ERR_IPC_ERROR;
        }
        tools.push_back(tool);
    }

    TAG_LOGI("CLIManagerProxy", "QueryTools succeeded: %{public}d tools", toolCount);
    return ErrCode::OK;
}


int32_t CLIManagerProxy::ExecTool(
    const std::string& name,
    const Json::Value& arguments,
    const ExecOptions& options,
    SessionInfo& session)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken()) {
        TAG_LOGE("CLIManagerProxy", "Failed to write interface token");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入工具名称
    if (!data.WriteString(name)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write tool name");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入参数（JSON）
    if (!WriteJsonValue(data, arguments)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write arguments");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入选项
    if (!data.WriteBool(options.background)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write background");
        return ErrCode::ERR_IPC_ERROR;
    }
    if (!data.WriteUint32(options.yieldMs)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write yieldMs");
        return ErrCode::ERR_IPC_ERROR;
    }
    if (!data.WriteUint32(options.timeoutMs)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write timeout");
        return ErrCode::ERR_IPC_ERROR;
    }
    if (!data.WriteString(options.workingDir)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write working directory");
        return ErrCode::ERR_IPC_ERROR;
    }
    if (!WriteJsonValue(data, options.env)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write environment variables");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 发送请求（同步调用）
    option.SetFlags(MessageOption::TF_SYNC);
    int32_t ret = Remote()->SendRequest(EXEC_TOOL, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE("CLIManagerProxy", "SendRequest failed: %{public}d", ret);
        return ErrCode::ERR_IPC_ERROR;
    }

    // 读取错误码
    int32_t errorCode;
    if (!reply.ReadInt32(errorCode)) {
        return ErrCode::ERR_IPC_ERROR;
    }

    if (errorCode != ErrCode::OK) {
        return errorCode;
    }

    // 读取会话信息
    if (!ReadSessionInfo(reply, session)) {
        return ErrCode::ERR_IPC_ERROR;
    }

    TAG_LOGI("CLIManagerProxy", "ExecTool succeeded: %{public}s, sessionId=%{public}s, status=%{public}s",
             name.c_str(), session.sessionId.c_str(), session.status.c_str());
    return ErrCode::OK;
}

int32_t CLIManagerProxy::SubscribeSession(
    const std::string& sessionId,
    const sptr<ICLICallback>& callback)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken()) {
        TAG_LOGE("CLIManagerProxy", "Failed to write interface token");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入会话 ID
    if (!data.WriteString(sessionId)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write session ID");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入回调对象
    if (!data.WriteRemoteObject(callback->AsObject())) {
        TAG_LOGE("CLIManagerProxy", "Failed to write callback object");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 发送请求
    int32_t ret = Remote()->SendRequest(SUBSCRIBE_SESSION, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE("CLIManagerProxy", "SendRequest failed: %{public}d", ret);
        return ErrCode::ERR_IPC_ERROR;
    }

    // 读取错误码
    int32_t errorCode;
    if (!reply.ReadInt32(errorCode)) {
        return ErrCode::ERR_IPC_ERROR;
    }

    TAG_LOGI("CLIManagerProxy", "SubscribeSession: %{public}s, result=%{public}d",
             sessionId.c_str(), errorCode);
    return errorCode;
}

int32_t CLIManagerProxy::CancelSession(const std::string& sessionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken()) {
        TAG_LOGE("CLIManagerProxy", "Failed to write interface token");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入会话 ID
    if (!data.WriteString(sessionId)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write session ID");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 发送请求
    int32_t ret = Remote()->SendRequest(CANCEL_SESSION, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE("CLIManagerProxy", "SendRequest failed: %{public}d", ret);
        return ErrCode::ERR_IPC_ERROR;
    }

    // 读取错误码
    int32_t errorCode;
    if (!reply.ReadInt32(errorCode)) {
        return ErrCode::ERR_IPC_ERROR;
    }

    TAG_LOGI("CLIManagerProxy", "CancelSession: %{public}s, result=%{public}d",
             sessionId.c_str(), errorCode);
    return errorCode;
}

int32_t CLIManagerProxy::QuerySessionStatus(const std::string& sessionId, SessionInfo& status)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken()) {
        TAG_LOGE("CLIManagerProxy", "Failed to write interface token");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入会话 ID
    if (!data.WriteString(sessionId)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write session ID");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 发送请求
    int32_t ret = Remote()->SendRequest(QUERY_SESSION_STATUS, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE("CLIManagerProxy", "SendRequest failed: %{public}d", ret);
        return ErrCode::ERR_IPC_ERROR;
    }

    // 读取错误码
    int32_t errorCode;
    if (!reply.ReadInt32(errorCode)) {
        return ErrCode::ERR_IPC_ERROR;
    }

    if (errorCode != ErrCode::OK) {
        return errorCode;
    }

    // 读取会话信息
    if (!ReadSessionInfo(reply, status)) {
        return ErrCode::ERR_IPC_ERROR;
    }

    TAG_LOGI("CLIManagerProxy", "QuerySessionStatus succeeded: %{public}s", sessionId.c_str());
    return ErrCode::OK;
}

bool CLIManagerProxy::ReadToolInfo(MessageParcel& reply, ToolInfo& tool)
{
    // 读取基本字段
    if (!reply.ReadString(tool.name)) {
        return false;
    }
    if (!reply.ReadString(tool.version)) {
        return false;
    }
    if (!reply.ReadString(tool.description)) {
        return false;
    }
    if (!reply.ReadString(tool.executablePath)) {
        return false;
    }

    // 读取权限列表
    int32_t permCount;
    if (!reply.ReadInt32(permCount)) {
        return false;
    }
    tool.permissions.clear();
    for (int32_t i = 0; i < permCount; ++i) {
        std::string perm;
        if (!reply.ReadString(perm)) {
            return false;
        }
        tool.permissions.push_back(perm);
    }

    // 读取 JSON Schema
    std::string inputSchemaStr, outputSchemaStr;
    if (!reply.ReadString(inputSchemaStr)) {
        return false;
    }
    if (!reply.ReadString(outputSchemaStr)) {
        return false;
    }

    // 解析 JSON
    Json::Reader reader;
    reader.parse(inputSchemaStr, tool.inputSchema);
    reader.parse(outputSchemaStr, tool.outputSchema);

    // 读取超时时间
    if (!reply.ReadUint32(tool.timeoutMs)) {
        return false;
    }

    // 读取事件类型
    int32_t eventCount;
    if (!reply.ReadInt32(eventCount)) {
        return false;
    }
    tool.eventTypes.clear();
    for (int32_t i = 0; i < eventCount; ++i) {
        std::string event;
        if (!reply.ReadString(event)) {
            return false;
        }
        tool.eventTypes.push_back(event);
    }

    // 读取注册时间和启用状态
    if (!reply.ReadUint64(tool.registeredTime)) {
        return false;
    }
    if (!reply.ReadBool(tool.enabled)) {
        return false;
    }

    // 读取参数映射规则 (argMapping)
    std::string argMappingStr;
    if (!reply.ReadString(argMappingStr)) {
        return false;
    }
    if (!argMappingStr.empty()) {
        Json::Reader reader;
        if (!reader.parse(argMappingStr, tool.argMapping)) {
            TAG_LOGE("CLIManagerProxy", "Failed to parse argMapping JSON");
            return false;
        }
    }

    // 读取事件 Schema 定义 (eventSchemas)
    std::string eventSchemasStr;
    if (!reply.ReadString(eventSchemasStr)) {
        return false;
    }
    if (!eventSchemasStr.empty()) {
        Json::Reader reader;
        if (!reader.parse(eventSchemasStr, tool.eventSchemas)) {
            TAG_LOGE("CLIManagerProxy", "Failed to parse eventSchemas JSON");
            return false;
        }
    }

    // 读取子命令信息
    tool.hasSubcommands = reply.ReadBool();

    if (tool.hasSubcommands) {
        // 读取子命令数量
        int32_t subcmdCount;
        if (!reply.ReadInt32(subcmdCount)) {
            return false;
        }

        if (subcmdCount < 0 || subcmdCount > 100) {
            TAG_LOGE("CLIManagerProxy", "Invalid subcommand count: %{public}d", subcmdCount);
            return false;
        }

        tool.subcommands.clear();
        Json::Reader reader;
        for (int32_t i = 0; i < subcmdCount; ++i) {
            SubCommandInfo subcmd;

            // 读取子命令名称
            std::string name;
            if (!reply.ReadString(name)) {
                return false;
            }
            if (name.empty()) {
                TAG_LOGE("CLIManagerProxy", "Subcommand name is empty");
                return false;
            }

            // 读取子命令描述
            if (!reply.ReadString(subcmd.description)) {
                return false;
            }

            // 读取子命令的 Schema
            std::string subInputSchemaStr, subOutputSchemaStr;
            std::string subArgMappingStr, subEventSchemasStr;

            if (!reply.ReadString(subInputSchemaStr) ||
                !reply.ReadString(subOutputSchemaStr) ||
                !reply.ReadString(subArgMappingStr) ||
                !reply.ReadString(subEventSchemasStr)) {
                return false;
            }

            if (!subInputSchemaStr.empty()) {
                if (!reader.parse(subInputSchemaStr, subcmd.inputSchema)) {
                    TAG_LOGE("CLIManagerProxy", "Failed to parse subcommand inputSchema");
                    return false;
                }
            }
            if (!subOutputSchemaStr.empty()) {
                if (!reader.parse(subOutputSchemaStr, subcmd.outputSchema)) {
                    TAG_LOGE("CLIManagerProxy", "Failed to parse subcommand outputSchema");
                    return false;
                }
            }
            if (!subArgMappingStr.empty()) {
                if (!reader.parse(subArgMappingStr, subcmd.argMapping)) {
                    TAG_LOGE("CLIManagerProxy", "Failed to parse subcommand argMapping");
                    return false;
                }
            }
            if (!subEventSchemasStr.empty()) {
                if (!reader.parse(subEventSchemasStr, subcmd.eventSchemas)) {
                    TAG_LOGE("CLIManagerProxy", "Failed to parse subcommand eventSchemas");
                    return false;
                }
            }

            // 读取子命令的事件类型
            int32_t subEventCount;
            if (!reply.ReadInt32(subEventCount)) {
                return false;
            }

            if (subEventCount < 0 || subEventCount > 100) {
                TAG_LOGE("CLIManagerProxy", "Invalid subcommand event type count: %{public}d", subEventCount);
                return false;
            }

            subcmd.eventTypes.clear();
            for (int32_t j = 0; j < subEventCount; ++j) {
                std::string event;
                if (!reply.ReadString(event)) {
                    return false;
                }
                if (!event.empty()) {
                    subcmd.eventTypes.push_back(event);
                }
            }

            // 添加子命令到映射
            tool.subcommands[name] = subcmd;
        }

        TAG_LOGI("CLIManagerProxy", "Read %{public}d subcommands for tool %{public}s",
                 subcmdCount, tool.name.c_str());
    }

    return true;
}

bool CLIManagerProxy::WriteToolInfo(MessageParcel& parcel, const ToolInfo& tool)
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
        TAG_LOGE("CLIManagerProxy", "Failed to write argMapping");
        return false;
    }

    // 写入事件 Schema 定义 (eventSchemas)
    std::string eventSchemasStr = Json::writeString(builder, tool.eventSchemas);
    if (!parcel.WriteString(eventSchemasStr)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write eventSchemas");
        return false;
    }

    // 写入子命令信息
    if (!parcel.WriteBool(tool.hasSubcommands)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write hasSubcommands");
        return false;
    }

    if (tool.hasSubcommands) {
        // 写入子命令数量
        if (!parcel.WriteInt32(tool.subcommands.size())) {
            TAG_LOGE("CLIManagerProxy", "Failed to write subcommand count");
            return false;
        }

        // 写入每个子命令
        for (const auto& [name, subcmd] : tool.subcommands) {
            if (!parcel.WriteString(name)) {
                TAG_LOGE("CLIManagerProxy", "Failed to write subcommand name");
                return false;
            }

            if (!parcel.WriteString(subcmd.description)) {
                TAG_LOGE("CLIManagerProxy", "Failed to write subcommand description");
                return false;
            }

            // 序列化子命令的 Schema
            std::string subInputSchemaStr = Json::writeString(builder, subcmd.inputSchema);
            std::string subOutputSchemaStr = Json::writeString(builder, subcmd.outputSchema);
            std::string subArgMappingStr = Json::writeString(builder, subcmd.argMapping);
            std::string subEventSchemasStr = Json::writeString(builder, subcmd.eventSchemas);

            if (!parcel.WriteString(subInputSchemaStr)) {
                TAG_LOGE("CLIManagerProxy", "Failed to write subcommand inputSchema");
                return false;
            }
            if (!parcel.WriteString(subOutputSchemaStr)) {
                TAG_LOGE("CLIManagerProxy", "Failed to write subcommand outputSchema");
                return false;
            }
            if (!parcel.WriteString(subArgMappingStr)) {
                TAG_LOGE("CLIManagerProxy", "Failed to write subcommand argMapping");
                return false;
            }
            if (!parcel.WriteString(subEventSchemasStr)) {
                TAG_LOGE("CLIManagerProxy", "Failed to write subcommand eventSchemas");
                return false;
            }

            // 序列化子命令的事件类型
            if (!parcel.WriteInt32(subcmd.eventTypes.size())) {
                TAG_LOGE("CLIManagerProxy", "Failed to write subcommand eventTypes count");
                return false;
            }
            for (const auto& event : subcmd.eventTypes) {
                if (!parcel.WriteString(event)) {
                    TAG_LOGE("CLIManagerProxy", "Failed to write subcommand eventType");
                    return false;
                }
            }
        }
    }

    return true;
}

bool CLIManagerProxy::ReadExecutionResult(MessageParcel& reply, ExecutionResult& result)
{
    if (!reply.ReadString(result.taskId)) {
        return false;
    }
    if (!reply.ReadInt32(result.exitCode)) {
        return false;
    }
    if (!reply.ReadString(result.stdout)) {
        return false;
    }
    if (!reply.ReadString(result.stderr)) {
        return false;
    }
    if (!reply.ReadInt32(result.signal)) {
        return false;
    }
    if (!reply.ReadBool(result.timedOut)) {
        return false;
    }
    if (!reply.ReadUint64(result.executionTimeMs)) {
        return false;
    }

    return true;
}

bool CLIManagerProxy::WriteJsonValue(MessageParcel& parcel, const Json::Value& value)
{
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string jsonStr = Json::writeString(builder, value);

    return parcel.WriteString(jsonStr);
}

bool CLIManagerProxy::ReadJsonValue(MessageParcel& reply, Json::Value& value)
{
    std::string jsonStr;
    if (!reply.ReadString(jsonStr)) {
        return false;
    }

    Json::Reader reader;
    return reader.parse(jsonStr, value);
}

bool CLIManagerProxy::ReadSessionInfo(MessageParcel& reply, SessionInfo& session)
{
    // 清理旧的结果指针
    delete session.result;
    session.result = nullptr;

    // 读取基本字段
    if (!reply.ReadString(session.sessionId)) {
        return false;
    }
    if (!reply.ReadString(session.toolName)) {
        return false;
    }
    if (!reply.ReadString(session.status)) {
        return false;
    }
    if (!reply.ReadUint64(session.startTime)) {
        return false;
    }
    if (!reply.ReadUint64(session.endTime)) {
        return false;
    }

    // 读取执行结果（如果有的话）
    bool hasResult;
    if (!reply.ReadBool(hasResult)) {
        return false;
    }

    if (hasResult) {
        session.result = new ExecutionResult();
        if (!ReadExecutionResult(reply, *session.result)) {
            delete session.result;
            session.result = nullptr;
            return false;
        }
    }

    return true;
}

// ============================================================================
// 动态工具注册接口实现
// ============================================================================

int32_t CLIManagerProxy::RegisterTool(const ToolInfo& tool, bool persist)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken()) {
        TAG_LOGE("CLIManagerProxy", "Failed to write interface token");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入工具信息
    if (!WriteToolInfo(data, tool)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write tool info");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入持久化标志
    if (!data.WriteBool(persist)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write persist flag");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 发送请求
    int32_t ret = Remote()->SendRequest(REGISTER_TOOL, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE("CLIManagerProxy", "SendRequest failed: %{public}d", ret);
        return ErrCode::ERR_IPC_ERROR;
    }

    // 读取错误码
    int32_t errorCode;
    if (!reply.ReadInt32(errorCode)) {
        return ErrCode::ERR_IPC_ERROR;
    }

    return errorCode;
}

int32_t CLIManagerProxy::RegisterTools(const std::vector<ToolInfo>& tools, bool persist)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken()) {
        TAG_LOGE("CLIManagerProxy", "Failed to write interface token");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入工具数量
    if (!data.WriteInt32(tools.size())) {
        TAG_LOGE("CLIManagerProxy", "Failed to write tool count");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入每个工具
    for (const auto& tool : tools) {
        if (!WriteToolInfo(data, tool)) {
            TAG_LOGE("CLIManagerProxy", "Failed to write tool info");
            return ErrCode::ERR_IPC_ERROR;
        }
    }

    // 写入持久化标志
    if (!data.WriteBool(persist)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write persist flag");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 发送请求
    int32_t ret = Remote()->SendRequest(REGISTER_TOOLS, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE("CLIManagerProxy", "SendRequest failed: %{public}d", ret);
        return ErrCode::ERR_IPC_ERROR;
    }

    // 读取错误码
    int32_t errorCode;
    if (!reply.ReadInt32(errorCode)) {
        return ErrCode::ERR_IPC_ERROR;
    }

    return errorCode;
}

int32_t CLIManagerProxy::UpdateTool(const ToolInfo& tool, bool persist)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken()) {
        TAG_LOGE("CLIManagerProxy", "Failed to write interface token");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入工具信息
    if (!WriteToolInfo(data, tool)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write tool info");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入持久化标志
    if (!data.WriteBool(persist)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write persist flag");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 发送请求
    int32_t ret = Remote()->SendRequest(UPDATE_TOOL, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE("CLIManagerProxy", "SendRequest failed: %{public}d", ret);
        return ErrCode::ERR_IPC_ERROR;
    }

    // 读取错误码
    int32_t errorCode;
    if (!reply.ReadInt32(errorCode)) {
        return ErrCode::ERR_IPC_ERROR;
    }

    return errorCode;
}

int32_t CLIManagerProxy::UnregisterTool(const std::string& name, bool persist)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken()) {
        TAG_LOGE("CLIManagerProxy", "Failed to write interface token");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入工具名称
    if (!data.WriteString(name)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write tool name");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入持久化标志
    if (!data.WriteBool(persist)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write persist flag");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 发送请求
    int32_t ret = Remote()->SendRequest(UNREGISTER_TOOL, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE("CLIManagerProxy", "SendRequest failed: %{public}d", ret);
        return ErrCode::ERR_IPC_ERROR;
    }

    // 读取错误码
    int32_t errorCode;
    if (!reply.ReadInt32(errorCode)) {
        return ErrCode::ERR_IPC_ERROR;
    }

    return errorCode;
}

int32_t CLIManagerProxy::QueryTool(const std::string& toolName, ToolInfo& tool)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken()) {
        TAG_LOGE("CLIManagerProxy", "Failed to write interface token");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 写入工具名称
    if (!data.WriteString(toolName)) {
        TAG_LOGE("CLIManagerProxy", "Failed to write tool name");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 发送请求
    int32_t ret = Remote()->SendRequest(QUERY_TOOL, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE("CLIManagerProxy", "SendRequest failed: %{public}d", ret);
        return ErrCode::ERR_IPC_ERROR;
    }

    // 读取错误码
    int32_t errorCode;
    if (!reply.ReadInt32(errorCode)) {
        return ErrCode::ERR_IPC_ERROR;
    }

    if (errorCode != ErrCode::OK) {
        return errorCode;
    }

    // 读取工具信息
    if (!ReadToolInfo(reply, tool)) {
        TAG_LOGE("CLIManagerProxy", "Failed to read tool info");
        return ErrCode::ERR_IPC_ERROR;
    }

    return ErrCode::OK;
}

int32_t CLIManagerProxy::ReloadRegistry()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken()) {
        TAG_LOGE("CLIManagerProxy", "Failed to write interface token");
        return ErrCode::ERR_IPC_ERROR;
    }

    // 发送请求
    int32_t ret = Remote()->SendRequest(RELOAD_REGISTRY, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE("CLIManagerProxy", "SendRequest failed: %{public}d", ret);
        return ErrCode::ERR_IPC_ERROR;
    }

    // 读取错误码
    int32_t errorCode;
    if (!reply.ReadInt32(errorCode)) {
        return ErrCode::ERR_IPC_ERROR;
    }

    return errorCode;
}

} // namespace CliMgr
} // namespace OHOS
