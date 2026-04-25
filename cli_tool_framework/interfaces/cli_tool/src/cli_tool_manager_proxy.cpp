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

#include "cli_tool_manager_proxy.h"
#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "parcel.h"

namespace OHOS {
namespace CliTool {

CliToolManagerProxy::CliToolManagerProxy(const sptr<IRemoteObject> &object)
    : IRemoteProxy<ICliToolManager>(object)
{}

int32_t CliToolManagerProxy::GetAllToolSummaries(std::vector<ToolSummary> &summaries)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "GetAllToolSummaries called");

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write interface token");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    int32_t ret = SendRequest(COMMAND_GET_ALL_TOOL_SUMMARIES, data, reply);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "SendRequest failed: %{public}d", ret);
        return ret;
    }

    ret = reply.ReadInt32();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolSummaries failed: %{public}d", ret);
        return ret;
    }

    // Read ToolSummary vector
    uint32_t summariesSize = reply.ReadUint32();
    for (uint32_t i = 0; i < summariesSize; ++i) {
        ToolSummary *summaryPtr = ToolSummary::Unmarshalling(reply);
        if (summaryPtr == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to unmarshal ToolSummary");
            return ERR_INVALID_READ;
        }
        summaries.push_back(*summaryPtr);
        delete summaryPtr;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllToolSummaries succeeded, count=%{public}zu", summaries.size());
    return ERR_OK;
}

int32_t CliToolManagerProxy::GetToolInfoByName(const std::string &name, ToolInfo &tool)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "GetToolInfoByName called, name=%{public}s", name.c_str());

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write interface token");
        return ERR_FLATTEN_OBJECT;
    }

    if (!data.WriteString(name)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write tool name");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    int32_t ret = SendRequest(COMMAND_GET_TOOL_INFO_BY_NAME, data, reply);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "SendRequest failed: %{public}d", ret);
        return ret;
    }

    ret = reply.ReadInt32();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetToolInfoByName failed: %{public}d", ret);
        return ret;
    }

    ToolInfo *toolPtr = ToolInfo::Unmarshalling(reply);
    if (toolPtr == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to unmarshal ToolInfo");
        return ERR_INVALID_READ;
    }
    tool = *toolPtr;
    delete toolPtr;

    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetToolInfoByName succeeded");
    return ERR_OK;
}

int32_t CliToolManagerProxy::GetAllToolInfos(std::vector<ToolInfo> &tools)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "GetAllToolInfos called");

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write interface token");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    int32_t ret = SendRequest(COMMAND_GET_ALL_TOOL_INFOS, data, reply);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "SendRequest failed: %{public}d", ret);
        return ret;
    }

    ret = reply.ReadInt32();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolInfos failed: %{public}d", ret);
        return ret;
    }

    // Read ToolInfo vector
    uint32_t toolsSize = reply.ReadUint32();
    for (uint32_t i = 0; i < toolsSize; ++i) {
        ToolInfo *toolPtr = ToolInfo::Unmarshalling(reply);
        if (toolPtr == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to unmarshal ToolInfo");
            return ERR_INVALID_READ;
        }
        tools.push_back(*toolPtr);
        delete toolPtr;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllToolInfos succeeded, count=%{public}zu", tools.size());
    return ERR_OK;
}

int32_t CliToolManagerProxy::RegisterTool(const ToolInfo &tool)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "RegisterTool called, name=%{public}s", tool.name.c_str());

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write interface token");
        return ERR_FLATTEN_OBJECT;
    }

    if (!tool.Marshalling(data)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to marshal ToolInfo");
        return ERR_FLATTEN_OBJECT;
    }

    MessageParcel reply;
    int32_t ret = SendRequest(COMMAND_REGISTER_TOOL, data, reply);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "SendRequest failed: %{public}d", ret);
        return ret;
    }

    ret = reply.ReadInt32();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "RegisterTool failed: %{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "RegisterTool succeeded");
    return ERR_OK;
}

int32_t CliToolManagerProxy::ExecTool(const ExecToolParam &param, const std::map<std::string, std::string> &args,
    CliSessionInfo &session)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "ExecTool called, toolName=%{public}s", param.toolName.c_str());

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write interface token");
        return ERR_FLATTEN_OBJECT;
    }

    if (!param.Marshalling(data)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to marshal ExecToolParam");
        return ERR_FLATTEN_OBJECT;
    }

    // Write argument map
    if (!data.WriteUInt32(args.size())) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write args size");
        return ERR_FLATTEN_OBJECT;
    }
    for (const auto &pair : args) {
        if (!data.WriteString(pair.first) || !data.WriteString(pair.second)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write argument");
            return ERR_FLATTEN_OBJECT;
        }
    }

    MessageParcel reply;
    int32_t ret = SendRequest(COMMAND_EXEC_TOOL, data, reply);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "SendRequest failed: %{public}d", ret);
        return ret;
    }

    ret = reply.ReadInt32();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ExecTool failed: %{public}d", ret);
        return ret;
    }

    CliSessionInfo *sessionPtr = CliSessionInfo::Unmarshalling(reply);
    if (sessionPtr == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to unmarshal CliSessionInfo");
        return ERR_INVALID_READ;
    }
    session = *sessionPtr;
    delete sessionPtr;

    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecTool succeeded, sessionId=%{public}s", session.sessionId.c_str());
    return ERR_OK;
}

int32_t CliToolManagerProxy::BatchQueryPermissionBySubCommand(const std::vector<Command> &cmds,
    std::vector<CommandPermission> &cmdPermissions)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "BatchQueryPermissionBySubCommand called, count=%{public}zu", cmds.size());

    MessageParcel data;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write interface token");
        return ERR_FLATTEN_OBJECT;
    }

    // Write Command vector
    if (!data.WriteUInt32(cmds.size())) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to write cmds size");
        return ERR_FLATTEN_OBJECT;
    }
    for (const auto &cmd : cmds) {
        if (!cmd.Marshalling(data)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to marshal Command");
            return ERR_FLATTEN_OBJECT;
        }
    }

    MessageParcel reply;
    int32_t ret = SendRequest(COMMAND_BATCH_QUERY_PERMISSION_BY_SUBCOMMAND, data, reply);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "SendRequest failed: %{public}d", ret);
        return ret;
    }

    ret = reply.ReadInt32();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "BatchQueryPermissionBySubCommand failed: %{public}d", ret);
        return ret;
    }

    // Read CommandPermission vector
    uint32_t cmdPermissionsSize = reply.ReadUint32();
    for (uint32_t i = 0; i < cmdPermissionsSize; ++i) {
        CommandPermission *cmdPermPtr = CommandPermission::Unmarshalling(reply);
        if (cmdPermPtr == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to unmarshal CommandPermission");
            return ERR_INVALID_READ;
        }
        cmdPermissions.push_back(*cmdPermPtr);
        delete cmdPermPtr;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "BatchQueryPermissionBySubCommand succeeded, total=%{public}zu", cmdPermissions.size());
    return ERR_OK;
}

int32_t CliToolManagerProxy::SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply)
{
    MessageOption option(MessageOption::TF_SYNC);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Remote object is null");
        return ERR_NULL_OBJECT;
    }

    int32_t ret = remote->SendRequest(code, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "SendRequest failed, code=%{public}u, ret=%{public}d", code, ret);
        return ret;
    }

    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
