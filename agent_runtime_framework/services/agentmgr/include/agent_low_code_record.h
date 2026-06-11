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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_LOW_CODE_RECORD_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_LOW_CODE_RECORD_H

#include <map>
#include <string>
#include <vector>

#include "ability_connect_callback_stub.h"
#include "agent_base_record.h"
#include "agent_host_connection.h"
#include "element_name.h"

namespace OHOS {
namespace AgentRuntime {
struct LowCodeAgentRecord {
    sptr<IRemoteObject> callerRemote;
    bool isPending = false;
    sptr<IRemoteObject> hostProxyRemote;
};

enum class LowCodeHostState : int32_t {
    CONNECTING,
    ACTIVE,
    DISCONNECT_REQUESTED,
    DISCONNECTING_BACKING,
    DISCONNECTED,
    FAILED,
};

class AgentLowCodeRecord : public AgentBaseRecord {
public:
    AgentLowCodeRecord();
    explicit AgentLowCodeRecord(const AgentHostKey &keyValue);

    bool Matches(const AgentHostKey &keyValue) const;
    bool HasAgent(const std::string &agentId) const;
    bool HasCapacity(size_t maxAgents) const;
    bool HasOtherAgentsForCaller(const std::string &agentId,
        const sptr<IRemoteObject> &callerRemote, const sptr<IRemoteObject> &hostProxyRemote) const;
    bool HasHostProxyConnection(const sptr<IRemoteObject> &hostProxyRemote) const;
    bool IsReusable() const;
    void MarkConnected();
    void MarkConnectFailed();
    void MarkDisconnectRequested();
    void MarkBackingDisconnecting();
    void MarkDisconnectFailed();
    void MarkDisconnected();
    int32_t CreateHostConnection(const AgentHostKey &hostKey, const sptr<IRemoteObject> &hostProxyRemote);
    void AddHostConnection(const sptr<IRemoteObject> &hostProxyRemote,
        const sptr<AgentHostConnection> &hostConnectionValue);
    void AddAgent(const std::string &agentId, const sptr<IRemoteObject> &callerRemote,
        const sptr<AAFwk::IAbilityConnection> &connection, const sptr<IRemoteObject> &hostProxyRemote,
        bool isPending);
    void RemoveAgent(const std::string &agentId);
    void RemoveAgentsByCaller(const sptr<IRemoteObject> &callerRemote,
        std::vector<std::string> &removedAgentIds, sptr<IRemoteObject> &hostProxyRemote);
    void RemoveCaller(const sptr<IRemoteObject> &callerRemote, const sptr<IRemoteObject> &hostProxyRemote,
        bool callerStillOwnsAgent);
    sptr<AgentHostConnection> GetHostConnection(const sptr<IRemoteObject> &hostProxyRemote) const;
    void CacheConnectDone(const AppExecFwk::ElementName &elementValue,
        const sptr<IRemoteObject> &remoteObjectValue, int32_t resultCodeValue);
    void CollectPendingCallbacks(const sptr<IRemoteObject> &hostProxyRemote,
        std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks, std::vector<std::string> &pendingAgentIds);
    void CleanupFailedConnect(const sptr<IRemoteObject> &hostProxyRemote,
        const std::vector<std::string> &pendingAgentIds);
    void PrepareDisconnectCallback(const sptr<IRemoteObject> &callerRemote,
        const sptr<IRemoteObject> &hostProxyRemote);
    void CollectDisconnectCallback(const sptr<IRemoteObject> &hostProxyRemote,
        std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks);
    bool NotifyAgentInvoked(const std::string &agentId) const;
    static bool NotifyAgentInvoked(const sptr<IRemoteObject> &remoteObject, const std::string &agentId);
    const sptr<IRemoteObject> &GetCachedRemoteObject() const;
    bool IsEmptyForRemoval() const;

    AgentHostKey key;
    int32_t hostUid = 0;
    sptr<AgentHostConnection> hostConnection;
    std::map<sptr<IRemoteObject>, sptr<AAFwk::IAbilityConnection>> callerConnections;
    std::map<sptr<IRemoteObject>, sptr<AgentHostConnection>> hostProxyConnections;
    std::map<sptr<IRemoteObject>, sptr<AAFwk::IAbilityConnection>> disconnectingCallbacks;
    std::map<std::string, LowCodeAgentRecord> agents;
    AppExecFwk::ElementName element;
    int32_t resultCode = ERR_OK;
    LowCodeHostState state = LowCodeHostState::CONNECTING;
    bool isConnected = false;
    bool isDisconnecting = false;

private:
    sptr<IRemoteObject> cachedRemoteObject_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_LOW_CODE_RECORD_H
