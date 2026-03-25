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

#include "cli_manager_stub.h"
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t ERROR_CODE = -1;
constexpr int32_t ERR_OK = 0;
}

CliManagerStub::CliManagerStub()
{
    // Constructor
}

int32_t CliManagerStub::OnRemoteRequest(uint32_t code, MessageParcel &data,
                                         MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "CliManagerStub::OnRemoteRequest, code: %{public}u", code);

    if (data.ReadInterfaceToken() != GetDescriptor()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Remote request interface token check failed");
        return ERROR_CODE;
    }

    CliManagerCmd cmd = static_cast<CliManagerCmd>(code);
    switch (cmd) {
        case CliManagerCmd::QUERY_TOOLS:
            return QueryToolsInner(data, reply);
        case CliManagerCmd::EXEC_TOOL:
            return ExecToolInner(data, reply);
        default:
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Unknown command code: %{public}u", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t CliManagerStub::QueryToolsInner(MessageParcel &data, MessageParcel &reply)
{
    std::vector<ToolInfo> tools;
    int32_t result = QueryTools(tools);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "QueryTools failed: %{public}d", result);
        reply.WriteInt32(result);
        return result;
    }

    // Write tool count
    if (!reply.WriteInt32(static_cast<int32_t>(tools.size()))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write tool count failed");
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }

    // Write each tool
    for (const auto &tool : tools) {
        if (!reply.WriteParcelable(&tool)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Write tool failed");
            reply.WriteInt32(ERROR_CODE);
            return ERROR_CODE;
        }
    }

    reply.WriteInt32(ERR_OK);
    return ERR_OK;
}

int32_t CliManagerStub::ExecToolInner(MessageParcel &data, MessageParcel &reply)
{
    std::string name;
    if (!data.ReadString(name)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read tool name failed");
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }

    // Read args map
    uint32_t argsSize = 0;
    if (!data.ReadUInt32(argsSize)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read args size failed");
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }

    std::map<std::string, std::string> args;
    for (uint32_t i = 0; i < argsSize; i++) {
        std::string key, value;
        if (!data.ReadString(key)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Read arg key failed");
            reply.WriteInt32(ERROR_CODE);
            return ERROR_CODE;
        }
        if (!data.ReadString(value)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Read arg value failed");
            reply.WriteInt32(ERROR_CODE);
            return ERROR_CODE;
        }
        args[key] = value;
    }

    std::unique_ptr<ExecOptions> options(data.ReadParcelable<ExecOptions>());
    if (!options) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read options failed");
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }

    ExecResult result;
    int32_t ret = ExecTool(name, args, *options, result);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ExecTool failed: %{public}d", ret);
        reply.WriteInt32(ret);
        return ret;
    }

    if (!reply.WriteParcelable(&result)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write result failed");
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }

    reply.WriteInt32(ERR_OK);
    return ERR_OK;
}

} // namespace AbilityRuntime
} // namespace OHOS
