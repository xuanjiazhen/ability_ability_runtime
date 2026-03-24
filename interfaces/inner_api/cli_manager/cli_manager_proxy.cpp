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

    // 读取类型
    uint32_t typeValue;
    if (!reply.ReadUint32(typeValue)) {
        return false;
    }
    tool.type = static_cast<ToolType>(typeValue);

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

} // namespace CliMgr
} // namespace OHOS
