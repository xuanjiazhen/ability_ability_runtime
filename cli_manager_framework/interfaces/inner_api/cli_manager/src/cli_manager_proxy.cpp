/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy QueryTools called");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed");
        return ERROR_CODE;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Sending transact command QUERY_TOOLS");
    int32_t result = SendTransactCmd(CliManagerCmd::QUERY_TOOLS, data, reply, option);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send transact cmd failed, result: %{public}d", result);
        return result;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Transact command succeeded, reading reply");

    // Read tools count
    int32_t toolCount = 0;
    if (!reply.ReadInt32(toolCount)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read tool count failed");
        return ERROR_CODE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Read tool count: %{public}d", toolCount);

    tools.clear();
    for (int32_t i = 0; i < toolCount; i++) {
        std::unique_ptr<ToolInfo> toolPtr(reply.ReadParcelable<ToolInfo>());
        if (!toolPtr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Read tool %{public}d failed", i);
            return ERROR_CODE;
        }
        tools.push_back(*toolPtr);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Read tool[%{public}d]: name=%{public}s, version=%{public}s",
            i, toolPtr->name.c_str(), toolPtr->version.c_str());
    }

    // Read final result code
    int32_t finalResult = ERROR_CODE;
    if (!reply.ReadInt32(finalResult)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read final result failed");
        return ERROR_CODE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Read final result code: %{public}d", finalResult);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "QueryTools completed successfully, returning %{public}d tools", toolCount);
    return finalResult;
}

int32_t CliManagerProxy::ExecTool(const std::string &name,
                                   const std::vector<std::pair<std::string, std::string>> &args,
                                   const ExecOptions &options,
                                   const std::string &challenge,
                                   SessionInfo &session)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy ExecTool called");
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy: received name='%{public}s'", name.c_str());
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy: received args.count=%{public}zu", args.size());
    for (const auto &[key, value] : args) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy:   arg[%{public}s]='%{public}s'",
            key.c_str(), value.c_str());
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy: options.background=%{public}d, yieldMs=%{public}d, timeout=%{public}d",
        options.background, options.yieldMs, options.timeout);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy: received challenge='%{public}s'", challenge.c_str());

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write interface token failed");
        return ERROR_CODE;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy: writing name to parcel");
    if (!data.WriteString(name)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write tool name failed");
        return ERROR_CODE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy: name written successfully");

    // Write args map
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy: writing %{public}zu args to parcel", args.size());
    if (!data.WriteUint32(static_cast<uint32_t>(args.size()))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write args size failed");
        return ERROR_CODE;
    }
    for (const auto &[key, value] : args) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy:   writing arg[%{public}s]='%{public}s'",
            key.c_str(), value.c_str());
        if (!data.WriteString(key)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Write arg key failed");
            return ERROR_CODE;
        }
        if (!data.WriteString(value)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Write arg value failed");
            return ERROR_CODE;
        }
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy: all args written successfully");

    if (!data.WriteParcelable(&options)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write options failed");
        return ERROR_CODE;
    }

    // Write challenge
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy: writing challenge to parcel");
    if (!data.WriteString(challenge)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write challenge failed");
        return ERROR_CODE;
    }

    int32_t ret = SendTransactCmd(CliManagerCmd::EXEC_TOOL, data, reply, option);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Send transact cmd failed, result: %{public}d", ret);
        return ret;
    }

    std::unique_ptr<SessionInfo> sessionPtr(reply.ReadParcelable<SessionInfo>());
    if (!sessionPtr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read session failed");
        return ERROR_CODE;
    }
    session = *sessionPtr;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy: read session from reply: status='%{public}s', sessionId='%{public}s'",
        session.status.c_str(), session.sessionId.c_str());

    // Read final result code
    int32_t finalResult = ERROR_CODE;
    if (!reply.ReadInt32(finalResult)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read final result failed");
        return ERROR_CODE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy: read final result code=%{public}d", finalResult);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerProxy: ExecTool succeeded, returning");
    return finalResult;
}

int32_t CliManagerProxy::RegisterScheduler(const sptr<ICliManagerScheduler> &scheduler)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor()) || scheduler == nullptr || scheduler->AsObject() == nullptr ||
        !data.WriteRemoteObject(scheduler->AsObject())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "RegisterScheduler write request failed");
        return ERROR_CODE;
    }

    int32_t ret = SendTransactCmd(CliManagerCmd::REGISTER_SCHEDULER, data, reply, option);
    if (ret != ERR_OK) {
        return ret;
    }

    int32_t finalResult = ERROR_CODE;
    if (!reply.ReadInt32(finalResult)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "RegisterScheduler read result failed");
        return ERROR_CODE;
    }
    return finalResult;
}

int32_t CliManagerProxy::UnregisterScheduler()
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UnregisterScheduler write request failed");
        return ERROR_CODE;
    }

    int32_t ret = SendTransactCmd(CliManagerCmd::UNREGISTER_SCHEDULER, data, reply, option);
    if (ret != ERR_OK) {
        return ret;
    }

    int32_t finalResult = ERROR_CODE;
    if (!reply.ReadInt32(finalResult)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UnregisterScheduler read result failed");
        return ERROR_CODE;
    }
    return finalResult;
}

int32_t CliManagerProxy::SubscribeSession(const std::string &sessionId, const std::string &subscriptionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor()) || !data.WriteString(sessionId) || !data.WriteString(subscriptionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SubscribeSession write request failed");
        return ERROR_CODE;
    }

    int32_t ret = SendTransactCmd(CliManagerCmd::SUBSCRIBE_SESSION, data, reply, option);
    if (ret != ERR_OK) {
        return ret;
    }

    int32_t finalResult = ERROR_CODE;
    if (!reply.ReadInt32(finalResult)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SubscribeSession read result failed");
        return ERROR_CODE;
    }
    return finalResult;
}

int32_t CliManagerProxy::UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor()) || !data.WriteString(sessionId) ||
        !data.WriteString(subscriptionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UnsubscribeSession write request failed");
        return ERROR_CODE;
    }

    int32_t ret = SendTransactCmd(CliManagerCmd::UNSUBSCRIBE_SESSION, data, reply, option);
    if (ret != ERR_OK) {
        return ret;
    }

    int32_t finalResult = ERROR_CODE;
    if (!reply.ReadInt32(finalResult)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UnsubscribeSession read result failed");
        return ERROR_CODE;
    }
    return finalResult;
}

int32_t CliManagerProxy::CancelSession(const std::string &sessionId)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor()) || !data.WriteString(sessionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CancelSession write request failed");
        return ERROR_CODE;
    }

    int32_t ret = SendTransactCmd(CliManagerCmd::CANCEL_SESSION, data, reply, option);
    if (ret != ERR_OK) {
        return ret;
    }

    int32_t finalResult = ERROR_CODE;
    if (!reply.ReadInt32(finalResult)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CancelSession read result failed");
        return ERROR_CODE;
    }
    return finalResult;
}

int32_t CliManagerProxy::QuerySessionStatus(const std::string &sessionId, SessionStatus &status)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(GetDescriptor()) || !data.WriteString(sessionId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "QuerySessionStatus write request failed");
        return ERROR_CODE;
    }

    int32_t ret = SendTransactCmd(CliManagerCmd::QUERY_SESSION_STATUS, data, reply, option);
    if (ret != ERR_OK) {
        return ret;
    }

    int32_t finalResult = ERROR_CODE;
    if (!reply.ReadInt32(finalResult)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "QuerySessionStatus read result failed");
        return ERROR_CODE;
    }
    if (finalResult != ERR_OK) {
        return finalResult;
    }

    std::unique_ptr<SessionStatus> statusPtr(reply.ReadParcelable<SessionStatus>());
    if (!statusPtr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read session status failed");
        return ERROR_CODE;
    }
    status = *statusPtr;
    return finalResult;
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
