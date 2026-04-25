/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

#include "cli_tool_manager_stub.h"
#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace CliTool {

// Command codes for IPC methods
constexpr uint32_t COMMAND_GET_ALL_TOOL_SUMMARIES = 1;
constexpr uint32_t COMMAND_GET_TOOL_INFO_BY_NAME = 2;
constexpr uint32_t COMMAND_GET_ALL_TOOL_INFOS = 3;
constexpr uint32_t COMMAND_REGISTER_TOOL = 4;
constexpr uint32_t COMMAND_EXEC_TOOL = 5;
constexpr uint32_t COMMAND_BATCH_QUERY_PERMISSION_BY_SUBCOMMAND = 6;

int32_t CliToolManagerStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
    MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "CliToolManagerStub::OnRemoteRequest called, code=%{public}u", code);

    std::u16string descriptor = CliToolManagerStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Descriptor check failed");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case COMMAND_GET_ALL_TOOL_SUMMARIES:
            return GetAllToolSummariesInner(data, reply);
        case COMMAND_GET_TOOL_INFO_BY_NAME:
            return GetToolInfoByNameInner(data, reply);
        case COMMAND_GET_ALL_TOOL_INFOS:
            return GetAllToolInfosInner(data, reply);
        case COMMAND_REGISTER_TOOL:
            return RegisterToolInner(data, reply);
        case COMMAND_EXEC_TOOL:
            return ExecToolInner(data, reply);
        case COMMAND_BATCH_QUERY_PERMISSION_BY_SUBCOMMAND:
            return BatchQueryPermissionBySubCommandInner(data, reply);
        default:
            TAG_LOGW(AAFwkTag::CLI_TOOL, "Unknown command code: %{public}u", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t CliToolManagerStub::GetAllToolSummariesInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "GetAllToolSummariesInner called");

    std::vector<ToolSummary> summaries;
    int32_t ret = GetAllToolSummaries(summaries);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolSummaries failed: %{public}d", ret);
        return ret;
    }

    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write result code");
        return ERR_FLATTEN_OBJECT;
    }

    // Write ToolSummary vector
    if (!reply.WriteUInt32(summaries.size())) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write summaries size");
        return ERR_FLATTEN_OBJECT;
    }

    for (const auto &summary : summaries) {
        if (!summary.Marshalling(reply)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to marshal ToolSummary");
            return ERR_FLATTEN_OBJECT;
        }
    }

    return ERR_OK;
}

int32_t CliToolManagerStub::GetToolInfoByNameInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "GetToolInfoByNameInner called");

    std::string name;
    if (!data.ReadString(name)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read tool name");
        return ERR_INVALID_READ;
    }

    ToolInfo tool;
    int32_t ret = GetToolInfoByName(name, tool);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetToolInfoByName failed: %{public}d", ret);
        return ret;
    }

    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write result code");
        return ERR_FLATTEN_OBJECT;
    }

    if (!tool.Marshalling(reply)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to marshal ToolInfo");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_OK;
}

int32_t CliToolManagerStub::GetAllToolInfosInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "GetAllToolInfosInner called");

    std::vector<ToolInfo> tools;
    int32_t ret = GetAllToolInfos(tools);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolInfos failed: %{public}d", ret);
        return ret;
    }

    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write result code");
        return ERR_FLATTEN_OBJECT;
    }

    // Write ToolInfo vector
    if (!reply.WriteUInt32(tools.size())) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write tools size");
        return ERR_FLATTEN_OBJECT;
    }

    for (const auto &tool : tools) {
        if (!tool.Marshalling(reply)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to marshal ToolInfo");
            return ERR_FLATTEN_OBJECT;
        }
    }

    return ERR_OK;
}

int32_t CliToolManagerStub::RegisterToolInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "RegisterToolInner called");

    // Unmarshal ToolInfo
    ToolInfo *toolPtr = ToolInfo::Unmarshalling(data);
    if (toolPtr == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to unmarshal ToolInfo");
        return ERR_INVALID_READ;
    }
    std::unique_ptr<ToolInfo> tool(toolPtr);

    int32_t ret = RegisterTool(*tool);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "RegisterTool failed: %{public}d", ret);
        return ret;
    }

    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write result code");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_OK;
}

int32_t CliToolManagerStub::ExecToolInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "ExecToolInner called");

    // Unmarshal ExecToolParam
    ExecToolParam *paramPtr = ExecToolParam::Unmarshalling(data);
    if (paramPtr == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to unmarshal ExecToolParam");
        return ERR_INVALID_READ;
    }
    std::unique_ptr<ExecToolParam> param(paramPtr);

    // Read argument map
    std::map<std::string, std::string> args;
    uint32_t argsSize = data.ReadUint32();
    for (uint32_t i = 0; i < argsSize; ++i) {
        std::string key, value;
        if (!data.ReadString(key) || !data.ReadString(value)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to read argument");
            return ERR_INVALID_READ;
        }
        args[key] = value;
    }

    CliSessionInfo session;
    int32_t ret = ExecTool(*param, args, session);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ExecTool failed: %{public}d", ret);
        return ret;
    }

    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write result code");
        return ERR_FLATTEN_OBJECT;
    }

    if (!session.Marshalling(reply)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to marshal CliSessionInfo");
        return ERR_FLATTEN_OBJECT;
    }

    return ERR_OK;
}

int32_t CliToolManagerStub::BatchQueryPermissionBySubCommandInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "BatchQueryPermissionBySubCommandInner called");

    // Read Command vector
    std::vector<Command> cmds;
    uint32_t cmdsSize = data.ReadUint32();
    for (uint32_t i = 0; i < cmdsSize; ++i) {
        Command *cmdPtr = Command::Unmarshalling(data);
        if (cmdPtr == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to unmarshal Command");
            return ERR_INVALID_READ;
        }
        cmds.push_back(*cmdPtr);
        delete cmdPtr;
    }

    std::vector<CommandPermission> cmdPermissions;
    int32_t ret = BatchQueryPermissionBySubCommand(cmds, cmdPermissions);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "BatchQueryPermissionBySubCommand failed: %{public}d", ret);
        return ret;
    }

    if (!reply.WriteInt32(ret)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write result code");
        return ERR_FLATTEN_OBJECT;
    }

    // Write CommandPermission vector
    if (!reply.WriteUInt32(cmdPermissions.size())) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write cmdPermissions size");
        return ERR_FLATTEN_OBJECT;
    }

    for (const auto &cmdPerm : cmdPermissions) {
        if (!cmdPerm.Marshalling(reply)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to marshal CommandPermission");
            return ERR_FLATTEN_OBJECT;
        }
    }

    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
