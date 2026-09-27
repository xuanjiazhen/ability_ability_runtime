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

#ifndef OHOS_AGENT_RUNTIME_AGENT_MANAGER_CLIENT_H
#define OHOS_AGENT_RUNTIME_AGENT_MANAGER_CLIENT_H

#include <functional>
#include <vector>

#include "iagent_manager.h"

namespace OHOS {
namespace AgentRuntime {
using ClearProxyCallback = std::function<void(const wptr<IRemoteObject>&)>;

class AgentManagerClient final {
public:
    AgentManagerClient() = default;
    virtual ~AgentManagerClient() = default;
    static AgentManagerClient &GetInstance();

    /**
     * @brief Completes asynchronous service-loading and installs the remote agent manager proxy.
     */
    void OnLoadSystemAbilitySuccess(const sptr<IRemoteObject> &remoteObject);
    /**
     * @brief Wakes blocked callers when the agent manager service cannot be loaded.
     */
    void OnLoadSystemAbilityFail();

    int32_t GetAllAgentCards(std::vector<AgentCard> &cards);
    int32_t GetAgentCardsByBundleName(const std::string &bundleName, std::vector<AgentCard> &cards);
    int32_t GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId, AgentCard &card);
    int32_t GetCallerAgentCardByAgentId(const std::string &agentId, AgentCard &card);
    int32_t RegisterAgentCard(const AgentCard &card);
    int32_t UpdateAgentCard(const AgentCard &card);
    int32_t DeleteAgentCard(const std::string &bundleName, const std::string &agentId);

    int32_t ConnectAgentExtensionAbility(const AAFwk::Want &want,
        const sptr<AAFwk::IAbilityConnection> &connection);

    int32_t DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection);

    int32_t ConnectAgentExtensionAbilityForCli(const AAFwk::Want &want,
        const sptr<AAFwk::IAbilityConnection> &connection, const std::string &callerIdentity);

    int32_t DisconnectAgentExtensionAbilityForCli(const sptr<AAFwk::IAbilityConnection> &connection,
        const std::string &callerIdentity);

    /**
     * @brief Connects a service extension ability on behalf of the given caller token.
     */
    int32_t ConnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken, const AAFwk::Want &want,
        const sptr<AAFwk::IAbilityConnection> &connection);

    /**
     * @brief Disconnects a previously connected service extension ability for the given caller token.
     */
    int32_t DisconnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
        const sptr<AAFwk::IAbilityConnection> &connection);

    /**
     * @brief Releases the active LOW_CODE agent marker once the caller reports completion.
     */
    int32_t NotifyLowCodeAgentComplete(const std::string &agentId);

    int32_t GetAgentCardTypeForConnect(AAFwk::Want &want, int32_t &cardType);

    int32_t VerifyAgentConnectRequest(const AAFwk::Want &want,
        const sptr<AAFwk::IAbilityConnection> &connection, std::string &callerIdentity);

    int32_t VerifyAgentDisconnectRequests(const std::vector<AAFwk::Want> &wants,
        const sptr<AAFwk::IAbilityConnection> &connection, std::string &callerIdentity);

private:
    sptr<IAgentManager> GetAgentMgrProxy();
    // Death-notification entry: compare the cached proxy's remote with the died remote and clear
    // the cache under mutex_ in one critical section. A stale notification (old proxy died after a
    // new proxy was reloaded) must not clobber the fresh proxy.
    void ClearProxyIfMatch(const wptr<IRemoteObject> &remote);
    bool LoadAgentMgrService();
    void SetAgentMgr(const sptr<IRemoteObject> &remoteObject);
    sptr<IAgentManager> GetAgentMgr();
    // Registers the death notification on the given remote. Callers must hold registerMutex_:
    // it serializes concurrent loads and, together with the registeredRemote_ weak identity
    // check, also prevents a second recipient when different paths install the same live remote
    // one after another. Returns false when the recipient could not be registered (typically the
    // remote is already dead); callers must not populate the cache with such a remote, or the
    // cache could never self-heal. The binder call runs while registerMutex_ is held but the
    // death callback only takes mutex_, so no deadlock is possible.
    bool RegisterDeathRecipient(const sptr<IRemoteObject> &remoteObject);

    class AgentManagerServiceDeathRecipient : public IRemoteObject::DeathRecipient {
    public:
        explicit AgentManagerServiceDeathRecipient(const ClearProxyCallback &proxy) : proxy_(proxy) {}
        virtual ~AgentManagerServiceDeathRecipient() = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        ClearProxyCallback proxy_;
    };

private:
    std::mutex mutex_;
    // Serializes load + death-recipient registration so concurrent first callers register the
    // recipient exactly once per loaded remote. Never taken inside mutex_ holders.
    std::mutex registerMutex_;
    // The remote whose death recipient is currently registered. Guarded by registerMutex_ and
    // only trusted for dedup while it is still the cached remote: a dead remote re-delivered by
    // a late load callback on an empty cache is then rejected by the AddDeathRecipient failure
    // path instead of being mistaken for an already-registered live remote.
    wptr<IRemoteObject> registeredRemote_;
    sptr<IAgentManager> agentMgr_ = nullptr;
};
} // namespace AgentRuntime
} // namespace OHOS
#endif // OHOS_AGENT_RUNTIME_AGENT_MANAGER_CLIENT_H
