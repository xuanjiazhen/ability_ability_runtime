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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_MANAGER_SERVICE_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_MANAGER_SERVICE_H

#include <memory>
#include <map>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "ability_connect_callback_stub.h"
#include "agent_bundle_event_callback.h"
#include "agent_event_handler.h"
#include "agent_manager_stub.h"
#include "system_ability.h"
#include "task_handler_wrap.h"
#include "element_name.h"

namespace OHOS {
namespace AgentRuntime {
/**
 * @class AgentManagerService
 * AgentManagerService provides a facility for managing agent life cycle.
 */
class AgentManagerService : public SystemAbility,
                            public AgentManagerStub,
                            public std::enable_shared_from_this<AgentManagerService> {
DECLEAR_SYSTEM_ABILITY(AgentManagerService)

public:
    static sptr<AgentManagerService> GetInstance();
    ~AgentManagerService();
    void OnStart() noexcept override;
    void OnStop() noexcept override;

    virtual void OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId) noexcept override;

    virtual void OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId) noexcept override;

    int32_t GetAllAgentCards(AgentCardsRawData &cards) override;

    int32_t GetAgentCardsByBundleName(const std::string &bundleName, AgentCardsRawData &cards) override;

    int32_t GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId, AgentCard &card) override;

    int32_t GetCallerAgentCardByAgentId(const std::string &agentId, AgentCard &card) override;

    int32_t RegisterAgentCard(const AgentCard &card) override;

    int32_t UpdateAgentCard(const AgentCard &card) override;

    int32_t DeleteAgentCard(const std::string &bundleName, const std::string &agentId) override;

    int32_t ConnectAgentExtensionAbility(const AAFwk::Want &want,
        const sptr<AAFwk::IAbilityConnection> &connection) override;

    int32_t DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection) override;

    int32_t ConnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken, const AAFwk::Want &want,
        const sptr<AAFwk::IAbilityConnection> &connection) override;

    int32_t DisconnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
        const sptr<AAFwk::IAbilityConnection> &connection) override;

    int32_t NotifyLowCodeAgentComplete(const std::string &agentId) override;

private:
    friend class AgentServiceConnection;

    /**
     * @struct AgentCallerIdentity
     * @brief Caller identity for quota and session ownership. Clones share uid but differ by instanceKey.
     */
    struct AgentCallerIdentity {
        int32_t uid = 0;
        std::string instanceKey;

        AgentCallerIdentity() = default;
        AgentCallerIdentity(int32_t uidValue) : uid(uidValue) {}
        AgentCallerIdentity(int32_t uidValue, const std::string &instanceKeyValue)
            : uid(uidValue), instanceKey(instanceKeyValue) {}

        bool operator==(const AgentCallerIdentity &other) const
        {
            return uid == other.uid && instanceKey == other.instanceKey;
        }
    };

    /**
     * @struct TrackedConnectionRecord
     * @brief Stores one caller-visible connection together with the service-side wrapper used to talk to AMS.
     */
    struct TrackedConnectionRecord {
        AgentCallerIdentity callerIdentity;
        int32_t callerUid = 0;
        sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
        sptr<IRemoteObject> callerRemote = nullptr;
        sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
        bool isLowCode = false;
        std::string lowCodeOwnerKey;
        bool countTowardsCallerLimit = true;
        bool isDisconnecting = false;
    };

    struct LowCodeConnectionRecord {
        AgentCallerIdentity callerIdentity;
        sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
        sptr<IRemoteObject> callerRemote = nullptr;
        sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
        bool isDisconnecting = false;
    };

    using TrackedConnectionIter = std::map<sptr<IRemoteObject>, TrackedConnectionRecord>::iterator;
    using LowCodeConnectionIter = std::map<std::string, LowCodeConnectionRecord>::iterator;

    struct AgentDisconnectRecord {
        sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
        sptr<IRemoteObject> callerRemote = nullptr;
        std::string lowCodeOwnerKey;
    };

    struct AgentDisconnectPlan {
        std::vector<AgentDisconnectRecord> records;
    };

    AgentManagerService();
    void Init();
    void RegisterBundleEventCallback();
    /**
     * @brief Builds the current IPC caller identity, preserving empty instanceKey when AppMgr has none.
     */
    AgentCallerIdentity GetCurrentCallerIdentity() const;
    AgentCallerIdentity GetEffectiveCallerIdentity(const TrackedConnectionRecord &record) const;
    std::string BuildCallerIdentityStorageKey(const AgentCallerIdentity &callerIdentity) const;
    std::string BuildLowCodeOwnerStorageKey(int32_t hostUid, const std::string &agentId) const;
    /**
     * @brief Validates caller permission and foreground state before classifying the agent connect request.
     */
    int32_t ValidateConnectAgentRequest(const sptr<AAFwk::IAbilityConnection> &connection,
        AgentCallerIdentity &callerIdentity);
    int32_t ValidateConnectAgentRequest(const sptr<AAFwk::IAbilityConnection> &connection, int32_t &callerUid);
    /**
     * @brief Resolves the target AgentCard and builds the connect Want consumed by later connect paths.
     */
    int32_t ResolveConnectAgentTarget(const AAFwk::Want &want, AAFwk::Want &connectWant,
        std::string &agentId, AgentCard &card, int32_t &callingUid) const;
    /**
     * @brief Adds low-level connect metadata required for standard APP agent extension connects.
     */
    int32_t PrepareStandardAgentConnectWant(AAFwk::Want &connectWant, const AgentCard &card, int32_t callingUid) const;
    /**
     * @brief Resolves the host uid that owns the low-code agent target.
     */
    int32_t ResolveLowCodeHostInfo(const AAFwk::Want &want, int32_t userId, int32_t &hostUid) const;
    /**
     * @brief Validates service-extension connect input for the caller-token based connect path.
     */
    int32_t ValidateConnectServiceRequest(const sptr<IRemoteObject> &callerToken,
        const sptr<AAFwk::IAbilityConnection> &connection) const;
    /**
     * @brief Resolves the target service extension and builds the connect Want used by AMS.
     */
    int32_t PrepareServiceConnectWant(const AAFwk::Want &want, AAFwk::Want &connectWant) const;
    /**
     * @brief Installs a tracked wrapper connection and returns the service-side callback object sent to AMS.
     */
    int32_t RegisterTrackedConnectionAndGetServiceConnection(const sptr<AAFwk::IAbilityConnection> &connection,
        const AgentCallerIdentity &callerIdentity, bool countTowardsCallerLimit,
        sptr<AAFwk::IAbilityConnection> &serviceConnection);
    /**
     * @brief Registers one low-code agent connection without consuming the caller's standard-agent quota.
     */
    int32_t RegisterLowCodeConnectionAndGetServiceConnection(const sptr<AAFwk::IAbilityConnection> &connection,
        const AgentCallerIdentity &callerIdentity, const std::string &ownerKey,
        sptr<AAFwk::IAbilityConnection> &serviceConnection);
    bool HasReachedCallerConnectionLimitLocked(const AgentCallerIdentity &callerIdentity) const;
    /**
     * @brief Adds one tracked connection record and optionally records low-code ownership.
     */
    int32_t TryRegisterConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection,
        const AgentCallerIdentity &callerIdentity, const sptr<AAFwk::IAbilityConnection> &serviceConnection = nullptr,
        bool countTowardsCallerLimit = true, bool isLowCode = false, const std::string &lowCodeOwnerKey = "");
    sptr<IRemoteObject> GetConnectionIdentityRemote(const sptr<AAFwk::IAbilityConnection> &connection) const;
    /**
     * @brief Finds the tracked record that matches the callback object or the fallback caller identity path.
     */
    TrackedConnectionIter FindTrackedConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection,
        const AgentCallerIdentity &callerIdentity);
    int32_t PrepareAgentDisconnectLocked(const sptr<AAFwk::IAbilityConnection> &connection,
        AgentDisconnectPlan &plan);
    int32_t PrepareLowCodeProxyDisconnectLocked(const sptr<IRemoteObject> &callerRemote, AgentDisconnectPlan &plan);
    int32_t PrepareStandardDisconnectLocked(TrackedConnectionIter it, AgentDisconnectPlan &plan);
    void RollbackAgentDisconnectLocked(const AgentDisconnectPlan &plan);
    void ReleaseCallerConnectionCountLocked(const AgentCallerIdentity &callerIdentity);
    /**
     * @brief Forwards connect/disconnect completion from the service-side wrapper back to the original caller.
     */
    void HandleConnectionDone(const sptr<AAFwk::IAbilityConnection> &connection, int32_t resultCode, bool isDisconnect);
    void HandleConnectionDone(const sptr<AAFwk::IAbilityConnection> &connection,
        const std::string &lowCodeOwnerKey, int32_t resultCode, bool isDisconnect);
    bool ReleaseCallerConnectionCountLocked(const sptr<IRemoteObject> &callerRemote);
    void ReleaseTrackedConnection(const sptr<AAFwk::IAbilityConnection> &connection);
    void ReleaseTrackedConnectionByRemoteLocked(const sptr<IRemoteObject> &callerRemote);
    void ReleaseLowCodeConnectionByOwnerKeyLocked(const std::string &ownerKey);
    void ReleaseLowCodeConnectionsByRemoteLocked(const sptr<IRemoteObject> &callerRemote,
        std::vector<sptr<AAFwk::IAbilityConnection>> &serviceConnections);
    void HandleCallerConnectionDied(const wptr<IRemoteObject> &remote);
    void HandleCallerConnectionDied(const sptr<IRemoteObject> &remote);
    void CleanupDeadStandardConnectionLocked(TrackedConnectionIter it, const sptr<IRemoteObject> &remote,
        sptr<AAFwk::IAbilityConnection> &serviceConnection);
    void DisconnectDeadServiceConnection(const sptr<AAFwk::IAbilityConnection> &serviceConnection);
    /**
     * @brief Connect path for LOW_CODE agents, delegated to AMS as one connection per logical agentId.
     */
    int32_t ConnectLowCodeAgentExtensionAbility(const AAFwk::Want &want,
        const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection,
        const AgentCallerIdentity &callerIdentity, int32_t hostUid);
    /**
     * @brief Finds the active low-code connection that should be completed by the target host.
     */
    int32_t PrepareLowCodeCompleteLocked(const std::string &ownerKey, AgentDisconnectPlan &plan);
    DISALLOW_COPY_AND_MOVE(AgentManagerService);

private:
    static constexpr size_t MAX_CONNECTIONS_PER_CALLER = 5;
    static sptr<AgentManagerService> instance_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AgentEventHandler> eventHandler_;
    sptr<AgentBundleEventCallback> bundleEventCallback_;
    std::mutex connectionLock_;
    std::map<sptr<IRemoteObject>, TrackedConnectionRecord> trackedConnections_;
    std::map<std::string, size_t> callerConnectionCounts_;
    std::map<std::string, LowCodeConnectionRecord> lowCodeAgentConnections_;
    std::map<sptr<IRemoteObject>, std::set<std::string>> lowCodeOwnersByRemote_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_MANAGER_SERVICE_H
