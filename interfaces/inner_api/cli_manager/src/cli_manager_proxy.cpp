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
#include "ipc_skeleton.h"
#include "message_parcel.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t ERROR_CODE = -1;
constexpr int32_t ERR_OK = 0;
}

CliManagerProxy::CliManagerProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<ICliManager>(impl)
{}

int32_t CliManagerProxy::QueryTools(std::vector<ToolInfo> &tools)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed");
        return ERROR_CODE;
    }

    int32_t result = SendTransactCmd(CliManagerCmd::QUERY_TOOLS, data, reply, option);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send transact cmd failed, result: %{public}d", result);
        return result;
    }

    // Read tools count
    int32_t toolCount = 0;
    if (!reply.ReadInt32(toolCount)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read tool count failed");
        return ERROR_CODE;
    }

    tools.clear();
    for (int32_t i = 0; i < toolCount; i++) {
        std::unique_ptr<ToolInfo> toolPtr(reply.ReadParcelable<ToolInfo>());
        if (!toolPtr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Read tool %{public}d failed", i);
            return ERROR_CODE;
        }
        tools.push_back(*toolPtr);
    }

    return ERR_OK;
}

int32_t CliManagerProxy::ExecTool(const std::string &name,
                                   const std::map<std::string, std::string> &args,
                                   const ExecOptions &options,
                                   ExecResult &result)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed");
        return ERROR_CODE;
    }

    if (!data.WriteString(name)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write tool name failed");
        return ERROR_CODE;
    }

    // Write args map
    if (!data.WriteUint32(static_cast<uint32_t>(args.size()))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write args size failed");
        return ERROR_CODE;
    }
    for (const auto &[key, value] : args) {
        if (!data.WriteString(key)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Write arg key failed");
            return ERROR_CODE;
        }
        if (!data.WriteString(value)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Write arg value failed");
            return ERROR_CODE;
        }
    }

    if (!data.WriteParcelable(&options)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write options failed");
        return ERROR_CODE;
    }

    int32_t ret = SendTransactCmd(CliManagerCmd::EXEC_TOOL, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send transact cmd failed, result: %{public}d", ret);
        return ret;
    }

    std::unique_ptr<ExecResult> resultPtr(reply.ReadParcelable<ExecResult>());
    if (!resultPtr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read result failed");
        return ERROR_CODE;
    }
    result = *resultPtr;

    return ERR_OK;
}

int32_t CliManagerProxy::SendTransactCmd(CliManagerCmd code, MessageParcel &data,
                                          MessageParcel &reply, MessageOption &option)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Remote is nullptr");
        return ERROR_CODE;
    }
    return remote->SendRequest(static_cast<uint32_t>(code), data, reply, option);
}

} // namespace AbilityRuntime
} // namespace OHOS
