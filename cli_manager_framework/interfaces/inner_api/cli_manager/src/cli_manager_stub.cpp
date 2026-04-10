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
        case CliManagerCmd::REGISTER_SCHEDULER:
            return RegisterSchedulerInner(data, reply);
        case CliManagerCmd::UNREGISTER_SCHEDULER:
            return UnregisterSchedulerInner(data, reply);
        case CliManagerCmd::SUBSCRIBE_SESSION:
            return SubscribeSessionInner(data, reply);
        case CliManagerCmd::UNSUBSCRIBE_SESSION:
            return UnsubscribeSessionInner(data, reply);
        case CliManagerCmd::CANCEL_SESSION:
            return CancelSessionInner(data, reply);
        case CliManagerCmd::QUERY_SESSION_STATUS:
            return QuerySessionStatusInner(data, reply);
        default:
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Unknown command code: %{public}u", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int32_t CliManagerStub::QueryToolsInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerStub QueryToolsInner called");
    std::vector<ToolInfo> tools;
    int32_t result = QueryTools(tools);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "QueryTools returned result=%{public}d, tools.size()=%{public}zu",
        result, tools.size());

    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "QueryTools failed: %{public}d", result);
        reply.WriteInt32(0);  // toolCount = 0
        reply.WriteInt32(result);  // error code
        return result;
    }

    // Write tool count
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Writing tool count: %{public}zu", tools.size());
    if (!reply.WriteInt32(static_cast<int32_t>(tools.size()))) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write tool count failed");
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }

    // Write each tool
    for (size_t i = 0; i < tools.size(); i++) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Writing tool[%{public}zu]: %{public}s", i, tools[i].name.c_str());
        if (!reply.WriteParcelable(&tools[i])) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Write tool[%{public}zu] failed", i);
            reply.WriteInt32(ERROR_CODE);
            return ERROR_CODE;
        }
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Writing final result code: ERR_OK");
    reply.WriteInt32(ERR_OK);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "QueryToolsInner completed successfully");
    return ERR_OK;
}

int32_t CliManagerStub::ExecToolInner(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerStub ExecToolInner called");

    // Read name
    std::string name;
    if (!data.ReadString(name)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read tool name failed");
        SessionInfo emptySession;
        reply.WriteParcelable(&emptySession);
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerStub: read name='%{public}s'", name.c_str());

    // Read args map
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerStub: reading args size");
    uint32_t argsSize = 0;
    if (!data.ReadUint32(argsSize)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read args size failed");
        SessionInfo emptySession;
        reply.WriteParcelable(&emptySession);
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerStub: argsSize=%{public}u", argsSize);

    std::vector<std::pair<std::string, std::string>> args;
    for (uint32_t i = 0; i < argsSize; i++) {
        std::string key;
        std::string value;
        if (!data.ReadString(key)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Read arg key failed");
            SessionInfo emptySession;
            reply.WriteParcelable(&emptySession);
            reply.WriteInt32(ERROR_CODE);
            return ERROR_CODE;
        }
        if (!data.ReadString(value)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Read arg value failed");
            SessionInfo emptySession;
            reply.WriteParcelable(&emptySession);
            reply.WriteInt32(ERROR_CODE);
            return ERROR_CODE;
        }
        args.push_back({key, value});
        TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerStub:   read arg[%{public}s]='%{public}s'",
            key.c_str(), value.c_str());
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerStub: total args read=%{public}zu", args.size());

    std::unique_ptr<ExecOptions> options(data.ReadParcelable<ExecOptions>());
    if (!options) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read options failed");
        SessionInfo emptySession;
        reply.WriteParcelable(&emptySession);
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerStub: options.background=%{public}d, yieldMs=%{public}d, timeout=%{public}d",
        options->background, options->yieldMs, options->timeout);

    // Read challenge
    std::string challenge;
    if (!data.ReadString(challenge)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Read challenge failed");
        SessionInfo emptySession;
        reply.WriteParcelable(&emptySession);
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerStub: read challenge='%{public}s'", challenge.c_str());

    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerStub: calling ExecTool(name='%{public}s', args.count=%{public}zu)",
        name.c_str(), args.size());
    SessionInfo session;
    int32_t ret = ExecTool(name, args, *options, challenge, session);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerStub: ExecTool returned ret=%{public}d, session.status='%{public}s', sessionId='%{public}s'",
        ret, session.status.c_str(), session.sessionId.c_str());

    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ExecTool failed: %{public}d", ret);
        SessionInfo emptySession;
        reply.WriteParcelable(&emptySession);
        reply.WriteInt32(ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerStub: writing session to reply");
    if (!reply.WriteParcelable(&session)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Write session failed");
        SessionInfo emptySession;
        reply.WriteParcelable(&emptySession);
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerStub: writing final result code ERR_OK");
    reply.WriteInt32(ERR_OK);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliManagerStub: ExecToolInner completed successfully");
    return ERR_OK;
}

int32_t CliManagerStub::CancelSessionInner(MessageParcel &data, MessageParcel &reply)
{
    std::string sessionId;
    if (!data.ReadString(sessionId)) {
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }

    int32_t ret = CancelSession(sessionId);
    reply.WriteInt32(ret);
    return ret;
}

int32_t CliManagerStub::RegisterSchedulerInner(MessageParcel &data, MessageParcel &reply)
{
    auto remote = data.ReadRemoteObject();
    auto scheduler = iface_cast<ICliManagerScheduler>(remote);
    if (scheduler == nullptr) {
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }

    int32_t ret = RegisterScheduler(scheduler);
    reply.WriteInt32(ret);
    return ret;
}

int32_t CliManagerStub::UnregisterSchedulerInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = UnregisterScheduler();
    reply.WriteInt32(ret);
    return ret;
}

int32_t CliManagerStub::SubscribeSessionInner(MessageParcel &data, MessageParcel &reply)
{
    std::string sessionId;
    std::string subscriptionId;
    if (!data.ReadString(sessionId) || !data.ReadString(subscriptionId)) {
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }

    int32_t ret = SubscribeSession(sessionId, subscriptionId);
    reply.WriteInt32(ret);
    return ret;
}

int32_t CliManagerStub::UnsubscribeSessionInner(MessageParcel &data, MessageParcel &reply)
{
    std::string sessionId;
    std::string subscriptionId;
    if (!data.ReadString(sessionId) || !data.ReadString(subscriptionId)) {
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }

    int32_t ret = UnsubscribeSession(sessionId, subscriptionId);
    reply.WriteInt32(ret);
    return ret;
}

int32_t CliManagerStub::QuerySessionStatusInner(MessageParcel &data, MessageParcel &reply)
{
    std::string sessionId;
    if (!data.ReadString(sessionId)) {
        reply.WriteInt32(ERROR_CODE);
        return ERROR_CODE;
    }

    SessionStatus status;
    int32_t ret = QuerySessionStatus(sessionId, status);
    reply.WriteInt32(ret);
    if (ret != ERR_OK) {
        return ret;
    }

    if (!reply.WriteParcelable(&status)) {
        return ERROR_CODE;
    }
    return ERR_OK;
}

} // namespace AbilityRuntime
} // namespace OHOS
