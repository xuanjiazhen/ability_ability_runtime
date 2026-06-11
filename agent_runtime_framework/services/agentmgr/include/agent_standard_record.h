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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_STANDARD_RECORD_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_STANDARD_RECORD_H

#include <map>
#include <set>
#include <string>
#include <vector>

#include "ability_connect_callback_stub.h"
#include "agent_base_record.h"
#include "element_name.h"
#include "want.h"

namespace OHOS {
namespace AgentRuntime {
/**
 * Standard agent session identity. The canonical identity is callerIdentity + targetIdentity + agentId.
 */
struct StandardAgentKey {
    AgentCallerIdentity callerIdentity;
    AgentTargetIdentity targetIdentity;
    int32_t callerUid = 0;
    std::string agentId;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
};

/**
 * Builds StandardAgentKey incrementally so callers do not need to pass every identity field positionally.
 */
class StandardAgentKeyBuilder {
public:
    /**
     * Sets the caller uid and caller instanceKey used for ownership and quota.
     */
    StandardAgentKeyBuilder &SetCallerIdentity(const AgentCallerIdentity &callerIdentity);
    /**
     * Sets the AgentId. Non-low-code targets use it as part of the target identity.
     */
    StandardAgentKeyBuilder &SetAgentId(const std::string &agentId);
    /**
     * Sets whether the target is standard or low-code.
     */
    StandardAgentKeyBuilder &SetCardType(AgentCardType cardType);
    /**
     * Sets the resolved target uid.
     */
    StandardAgentKeyBuilder &SetTargetUid(int32_t targetUid);
    /**
     * Sets the target process instanceKey exactly as reported by RunningProcessInfo.
     */
    StandardAgentKeyBuilder &SetTargetInstanceKey(const std::string &targetInstanceKey);
    /**
     * Sets the target element fields extracted from Want.
     */
    StandardAgentKeyBuilder &SetWant(const AAFwk::Want &want);
    /**
     * Produces a fully populated key, including compatibility fields used by legacy tests.
     */
    StandardAgentKey Build() const;

private:
    AgentCallerIdentity callerIdentity_;
    std::string agentId_;
    AgentCardType cardType_ = static_cast<AgentCardType>(0);
    int32_t targetUid_ = 0;
    std::string targetInstanceKey_;
    AAFwk::Want want_;
};

enum class StandardAgentState : int32_t {
    CONNECTING,
    ACTIVE,
    CONNECTED = ACTIVE,
    DISCONNECT_REQUESTED,
    DISCONNECTING_BACKING,
    DISCONNECTED,
    FAILED,
};

class AgentStandardRecord : public AgentBaseRecord {
public:
    AgentStandardRecord();
    explicit AgentStandardRecord(const StandardAgentKey &key);

    /**
     * Checks whether an incoming standard connect belongs to this collapsed backing session.
     */
    bool Matches(const StandardAgentKey &key) const;
    /**
     * Checks whether an AMS callback remote belongs to this backing service connection.
     */
    bool MatchesServiceRemote(const sptr<IRemoteObject> &serviceRemote) const;
    /**
     * Returns true when new owners may reuse this record.
     */
    bool IsReusable() const;
    /**
     * Marks the backing service connected and reusable for later owners.
     */
    void MarkConnected();
    /**
     * Marks connect failure and prevents further reuse.
     */
    void MarkConnectFailed();
    /**
     * Marks one owner disconnect request while the backing service may still be shared.
     */
    void MarkDisconnectRequested();
    /**
     * Marks that the last owner requested backing service disconnect.
     */
    void MarkBackingDisconnecting();
    /**
     * Restores reusable state when AMS rejects a backing disconnect.
     */
    void MarkDisconnectFailed();
    /**
     * Marks terminal disconnected state.
     */
    void MarkDisconnected();
    /**
     * Records one caller owner remote in this standard session.
     */
    void AddCallerRemote(const sptr<IRemoteObject> &callerRemote);
    /**
     * Records one caller callback waiting for connect completion.
     */
    void AddCallback(const sptr<AAFwk::IAbilityConnection> &connection);
    /**
     * Maps the service-side wrapper remote back to the original caller callback.
     */
    void AddServiceConnection(const sptr<AAFwk::IAbilityConnection> &serviceConnection,
        const sptr<AAFwk::IAbilityConnection> &callback);
    /**
     * Caches successful connect data for owners that attach after the backing service is connected.
     */
    void CacheConnectResult(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int32_t resultCode);
    /**
     * Collects callbacks that should receive a successful connect result for one service remote.
     */
    void CollectConnectDoneCallbacks(const sptr<IRemoteObject> &serviceRemote,
        std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks);
    /**
     * Collects callbacks that should receive a connect failure for one service remote.
     */
    void CollectConnectFailedCallbacks(const sptr<IRemoteObject> &serviceRemote,
        std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks);
    /**
     * Removes one service-side wrapper after the corresponding owner disconnects or fails.
     */
    void RemoveServiceConnection(const sptr<AAFwk::IAbilityConnection> &serviceConnection);
    /**
     * Removes one caller owner remote from this session.
     */
    void RemoveCallerRemote(const sptr<IRemoteObject> &callerRemote);
    /**
     * Removes one callback from pending and connected callback lists.
     */
    void RemoveCallback(const sptr<AAFwk::IAbilityConnection> &callback);
    /**
     * Collects all callbacks that should receive backing disconnect completion.
     */
    void CollectAllDisconnectCallbacks(std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks);

    StandardAgentKey key;
    StandardAgentState state = StandardAgentState::CONNECTING;
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    std::vector<sptr<AAFwk::IAbilityConnection>> pendingCallbacks;
    std::vector<sptr<AAFwk::IAbilityConnection>> connectedCallbacks;
    std::map<sptr<IRemoteObject>, sptr<AAFwk::IAbilityConnection>> standardServiceConnections;
    std::map<sptr<IRemoteObject>, sptr<AAFwk::IAbilityConnection>> pendingCallbackByServiceRemote;
    std::set<sptr<IRemoteObject>> pendingDisconnectServiceRemotes;
    std::vector<sptr<IRemoteObject>> callerRemotes;
    AppExecFwk::ElementName cachedElement;
    sptr<IRemoteObject> cachedRemoteObject = nullptr;
    int32_t cachedResultCode = ERR_OK;
    bool isDisconnecting = false;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_STANDARD_RECORD_H
