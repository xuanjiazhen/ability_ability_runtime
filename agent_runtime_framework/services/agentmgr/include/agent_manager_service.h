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

#include <list>
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
#include "agent_host_connection.h"
#include "agent_host_connection_types.h"
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
    friend class AgentHostConnection;

    /**
     * @struct StandardAgentKey
     * @brief Identity key for deduplicating non-low-code agent connections by caller uid and target.
     */
    struct StandardAgentKey {
        int32_t callerUid = 0;
        std::string agentId;
        std::string bundleName;
        std::string moduleName;
        std::string abilityName;
        sptr<IRemoteObject> hostProxyRemote = nullptr;
    };

    enum class StandardAgentState : int32_t {
        CONNECTING,
        CONNECTED,
    };

    /**
     * @struct StandardAgentSession
     * @brief Collapsed session for standard agent connections sharing the same caller uid and target.
     */
    struct StandardAgentSession {
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

    /**
     * @struct TrackedConnectionRecord
     * @brief Stores one caller-visible connection together with the service-side wrapper used to talk to AMS.
     */
    struct TrackedConnectionRecord {
        int32_t callerUid = 0;
        sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
        sptr<IRemoteObject> callerRemote = nullptr;
        sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
        AgentHostKey hostKey;
        bool isLowCode = false;
        bool countTowardsCallerLimit = true;
        bool isDisconnecting = false;
        StandardAgentKey standardKey;
    };

    using TrackedConnectionIter = std::map<sptr<IRemoteObject>, TrackedConnectionRecord>::iterator;
    using StandardSessionIter = std::list<std::shared_ptr<StandardAgentSession>>::iterator;
    using AgentHostSessionIter = std::list<std::shared_ptr<AgentHostSession>>::iterator;
    using AgentOwnerIter = std::list<AgentOwnerRecord>::iterator;

    struct AgentDisconnectPlan {
        sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
        std::vector<sptr<AAFwk::IAbilityConnection>> serviceConnections;
        sptr<AgentHostConnection> hostConnection = nullptr;
        sptr<IRemoteObject> callerRemote = nullptr;
        AgentHostKey hostKey;
        bool hasHostKey = false;
        bool lowCodeQuotaReleased = false;
        bool isStandard = false;
        bool standardQuotaReleased = false;
        int32_t standardCallerUid = 0;
        StandardAgentKey standardKey;
        sptr<IRemoteObject> lowCodeQuotaTransferFrom = nullptr;
        sptr<IRemoteObject> lowCodeQuotaTransferTo = nullptr;
        sptr<IRemoteObject> standardQuotaTransferFrom = nullptr;
        sptr<IRemoteObject> standardQuotaTransferTo = nullptr;
        sptr<AAFwk::IAbilityConnection> lowCodeCallerConnection = nullptr;
        std::map<std::string, LowCodeAgentRecord> lowCodeRemovedAgents;
        std::set<sptr<IRemoteObject>> lowCodeRemovedHostProxies;
    };

    struct LowCodeCompleteRollbackInfo {
        AgentHostKey hostKey;
        AgentOwnerKey ownerKey;
        LowCodeAgentRecord agentRecord;
        sptr<IRemoteObject> callerRemote = nullptr;
        sptr<IRemoteObject> hostProxyRemote = nullptr;
        sptr<AAFwk::IAbilityConnection> callerConnection = nullptr;
        sptr<IRemoteObject> quotaTransferFrom = nullptr;
        sptr<IRemoteObject> quotaTransferTo = nullptr;
        bool needRollback = false;
    };

    AgentManagerService();
    void Init();
    void RegisterBundleEventCallback();
    /**
     * @brief Validates caller permission and foreground state before classifying the agent connect request.
     */
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
    int32_t ConnectStandardAgentExtensionAbility(const AAFwk::Want &connectWant, const std::string &agentId,
        const AgentCard &card, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid);
    int32_t ConnectExistingStandardSession(const AAFwk::Want &connectWant,
        const std::shared_ptr<StandardAgentSession> &session,
        const sptr<AAFwk::IAbilityConnection> &connection, bool &handled);
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
        int32_t callerUid, bool countTowardsCallerLimit, sptr<AAFwk::IAbilityConnection> &serviceConnection);
    /**
     * @brief Checks whether the caller has consumed all available agent connection slots.
     */
    bool HasReachedCallerConnectionLimitLocked(int32_t callerUid) const;
    /**
     * @brief Adds one tracked connection record, optionally binding it to a shared low-code host session.
     */
    int32_t TryRegisterConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid,
        const sptr<AAFwk::IAbilityConnection> &serviceConnection = nullptr, const AgentHostKey *hostKey = nullptr,
        bool countTowardsCallerLimit = true);
    int32_t CheckExistingTrackedConnectionLocked(const sptr<IRemoteObject> &callerRemote,
        const AgentHostKey *hostKey) const;
    sptr<AAFwk::IAbilityConnection> EnsureServiceConnection(const sptr<AAFwk::IAbilityConnection> &connection,
        const sptr<AAFwk::IAbilityConnection> &serviceConnection) const;
    /**
     * @brief Returns the binder object used as the identity of one caller-visible connection request.
     */
    sptr<IRemoteObject> GetConnectionIdentityRemote(const sptr<AAFwk::IAbilityConnection> &connection) const;
    /**
     * @brief Finds the tracked record that matches the caller callback object or the fallback caller uid path.
     */
    TrackedConnectionIter FindTrackedConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection,
        int32_t callerUid);
    /**
     * @brief Builds the disconnect plan and removes bookkeeping that must be restored on AMS failure.
     */
    int32_t PrepareAgentDisconnectLocked(const sptr<AAFwk::IAbilityConnection> &connection,
        AgentDisconnectPlan &plan);
    int32_t ExecuteAgentDisconnectPlan(const AgentDisconnectPlan &plan);
    /**
     * @brief Prepares disconnect state for a low-code caller in a shared host session.
     */
    int32_t PrepareLowCodeDisconnectLocked(TrackedConnectionIter it, AgentDisconnectPlan &plan);
    bool RemoveLowCodeAgentsForCallerLocked(const std::shared_ptr<AgentHostSession> &session,
        const sptr<IRemoteObject> &callerRemote, AgentDisconnectPlan &plan,
        std::set<sptr<IRemoteObject>> &removedHostProxies);
    int32_t PrepareLowCodeCallerQuotaForDisconnectLocked(TrackedConnectionIter it,
        const std::shared_ptr<AgentHostSession> &session, AgentDisconnectPlan &plan);
    void SelectLowCodeDisconnectConnectionLocked(const std::shared_ptr<AgentHostSession> &session,
        const std::set<sptr<IRemoteObject>> &removedHostProxies, AgentDisconnectPlan &plan);
    /**
     * @brief Prepares disconnect state for a standard collapsed agent session.
     */
    int32_t PrepareStandardDisconnectLocked(TrackedConnectionIter it, AgentDisconnectPlan &plan);
    int32_t PrepareMatchedStandardDisconnectLocked(TrackedConnectionIter it,
        const std::shared_ptr<StandardAgentSession> &session, AgentDisconnectPlan &plan);
    void CollectStandardDisconnectConnectionsLocked(const std::shared_ptr<StandardAgentSession> &session,
        AgentDisconnectPlan &plan);
    void ArmStandardDisconnectLocked(const std::shared_ptr<StandardAgentSession> &session,
        const AgentDisconnectPlan &plan);
    /**
     * @brief Restores connection bookkeeping when the real disconnect request cannot be issued.
     */
    void RollbackAgentDisconnectLocked(const AgentDisconnectPlan &plan);
    /**
     * @brief Restores low-code host session and caller quota state after a failed host disconnect request.
     */
    void RollbackLowCodeAgentDisconnectLocked(const AgentDisconnectPlan &plan);
    /**
     * @brief Restores standard agent session and caller quota state after a failed service disconnect request.
     */
    void RollbackStandardAgentDisconnectLocked(const AgentDisconnectPlan &plan);
    /**
     * @brief Restores a quota transfer by making the original owner count again.
     */
    void RollbackTransferredCallerLimitLocked(const sptr<IRemoteObject> &transferFrom,
        const sptr<IRemoteObject> &transferTo);
    /**
     * @brief Clears the caller-visible disconnecting flag and restores simple tracked quota state.
     */
    void RollbackTrackedAgentDisconnectLocked(const AgentDisconnectPlan &plan);
    /**
     * @brief Releases one caller quota slot by caller uid when no binder identity is available.
     */
    void ReleaseCallerConnectionCountByUidLocked(int32_t callerUid);
    /**
     * @brief Forwards connect/disconnect completion from the service-side wrapper back to the original caller.
     */
    void HandleConnectionDone(const sptr<AAFwk::IAbilityConnection> &connection, int32_t resultCode, bool isDisconnect);
    void AddDeathRecipientLocked(TrackedConnectionRecord &record);
    /**
     * @brief Releases the quota slot associated with a tracked caller binder.
     */
    bool ReleaseCallerConnectionCountLocked(const sptr<IRemoteObject> &callerRemote);
    /**
     * @brief Removes one tracked connection and releases any related quota state.
     */
    void ReleaseTrackedConnection(const sptr<AAFwk::IAbilityConnection> &connection);
    /**
     * @brief Removes one tracked connection by binder identity while the connection lock is held.
     */
    void ReleaseTrackedConnectionByRemoteLocked(const sptr<IRemoteObject> &callerRemote);
    /**
     * @brief Moves a low-code quota slot to another caller in the same shared host session when possible.
     */
    void TransferLowCodeCallerLimitLocked(const std::shared_ptr<AgentHostSession> &session,
        const sptr<IRemoteObject> &callerRemote);
    /**
     * @brief Compares low-code host keys for one caller and target AgentExt.
     */
    bool IsLowCodeHostQuotaKeyMatched(const AgentHostKey &left, const AgentHostKey &right) const;
    /**
     * @brief Compares low-code host keys for shared host-session identity.
     */
    bool IsLowCodeHostKeyMatched(const AgentHostKey &left, const AgentHostKey &right) const;
    /**
     * @brief Finds the shared low-code host session that exactly matches the given key.
     */
    AgentHostSessionIter FindAgentHostSessionLocked(const AgentHostKey &key);
    /**
     * @brief Compares low-code agent owner keys without relying on ordered-key operators.
     */
    bool IsAgentOwnerKeyEqual(const AgentOwnerKey &left, const AgentOwnerKey &right) const;
    /**
     * @brief Finds the reverse owner index for a low-code agent.
     */
    AgentOwnerIter FindAgentOwnerLocked(const AgentOwnerKey &key);
    /**
     * @brief Creates or updates the reverse owner index for a low-code agent.
     */
    void SetAgentOwnerLocked(const AgentOwnerKey &key, const std::shared_ptr<AgentHostSession> &session);
    /**
     * @brief Removes the reverse owner index for a low-code agent.
     */
    void EraseAgentOwnerLocked(const AgentOwnerKey &key);
    /**
     * @brief Checks whether a caller already holds quota for the same low-code host target.
     */
    bool HasLowCodeHostQuotaLocked(const AgentHostKey &hostKey, int32_t callerUid) const;
    /**
     * @brief Transfers quota between low-code host sessions that share the same quota identity.
     */
    bool TransferLowCodeCallerLimitAcrossSessionsLocked(const AgentHostKey &hostKey, int32_t callerUid,
        const sptr<IRemoteObject> &transferFrom, sptr<IRemoteObject> &transferTo);
    /**
     * @brief Handles death-recipient cleanup from a weak caller binder reference.
     */
    void HandleCallerConnectionDied(const wptr<IRemoteObject> &remote);
    /**
     * @brief Handles death-recipient cleanup from a resolved caller binder.
     */
    void HandleCallerConnectionDied(const sptr<IRemoteObject> &remote);
    /**
     * @brief Cleans a dead standard connection and returns the real service connection to disconnect.
     */
    void CleanupDeadStandardConnectionLocked(TrackedConnectionIter it, const sptr<IRemoteObject> &remote,
        std::vector<sptr<AAFwk::IAbilityConnection>> &serviceConnections);
    void RemoveDeadStandardCallerFromSessionLocked(const std::shared_ptr<StandardAgentSession> &session,
        const sptr<IRemoteObject> &remote, const sptr<AAFwk::IAbilityConnection> &deadServiceConnection);
    void TransferStandardQuotaAfterCallerDiedLocked(const StandardAgentKey &stdKey,
        const sptr<IRemoteObject> &remote);
    /**
     * @brief Cleans a dead low-code connection and returns the host connection to disconnect when needed.
     */
    void CleanupDeadLowCodeConnectionLocked(TrackedConnectionIter it, const sptr<IRemoteObject> &remote,
        std::vector<sptr<AgentHostConnection>> &hostConnections);
    void CollectDeadLowCodeHostConnectionsLocked(const std::shared_ptr<AgentHostSession> &session,
        const std::set<sptr<IRemoteObject>> &removedHostProxies,
        std::vector<sptr<AgentHostConnection>> &hostConnections);
    /**
     * @brief Issues AMS disconnect for a real service connection after caller death cleanup.
     */
    void DisconnectDeadServiceConnections(const std::vector<sptr<AAFwk::IAbilityConnection>> &serviceConnections);
    /**
     * @brief Issues AMS disconnect for a real low-code host connection after caller death cleanup.
     */
    void DisconnectDeadHostConnections(const std::vector<sptr<AgentHostConnection>> &hostConnections);
    /**
     * @brief Connect path for LOW_CODE agents, which reuse one shared host session per target host.
     */
    int32_t ConnectLowCodeAgentExtensionAbility(const AAFwk::Want &want,
        const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid,
        int32_t hostUid);
    /**
     * @brief Builds the shared-host connect plan and records the caller into the chosen host session.
     */
    int32_t PrepareLowCodeConnectPlan(const AgentHostKey &hostKey, int32_t hostUid, const std::string &agentId,
        const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid, AgentConnectPlan &plan);
    int32_t GetOrCreateLowCodeHostSessionLocked(const AgentHostKey &hostKey, int32_t hostUid,
        const sptr<IRemoteObject> &hostProxyRemote, std::shared_ptr<AgentHostSession> &session,
        AgentConnectPlan &plan);
    int32_t RegisterLowCodePlanLocked(const AgentHostKey &hostKey, const std::string &agentId,
        const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid,
        const sptr<IRemoteObject> &callerRemote, const sptr<IRemoteObject> &hostProxyRemote,
        const std::shared_ptr<AgentHostSession> &session, AgentConnectPlan &plan);
    /**
     * @brief Delivers an already-established shared host connection to a new low-code caller.
     */
    void NotifyExistingLowCodeConnection(const AgentConnectPlan &plan, const std::string &agentId,
        const sptr<AAFwk::IAbilityConnection> &connection);
    /**
     * @brief Rolls back host-session bookkeeping after a low-code connect attempt fails.
     */
    void CleanupLowCodeConnectPlan(const AgentConnectPlan &plan, const std::string &agentId);
    /**
     * @brief Performs the real AMS connect for a newly created low-code shared host session.
     */
    int32_t CompleteAgentHostConnect(const AAFwk::Want &want, const std::string &agentId,
        AgentConnectPlan &plan);
    /**
     * @brief Extracts the host-session identity from the target element and caller user context.
     */
    AgentHostKey BuildAgentHostKey(const AAFwk::Want &want, int32_t callingUid) const;
    /**
     * @brief Builds the identity key for standard agent session deduplication.
     */
    StandardAgentKey BuildStandardAgentKey(int32_t callerUid, const std::string &agentId,
        const AAFwk::Want &want) const;
    /**
     * @brief Compares standard keys for exact identity after both keys are normalized.
     */
    bool IsStandardAgentKeyEqual(const StandardAgentKey &left, const StandardAgentKey &right) const;
    /**
     * @brief Matches standard keys for session lookup while tolerating a missing module name.
     */
    bool IsStandardAgentKeyMatched(const StandardAgentKey &storedKey, const StandardAgentKey &incomingKey) const;
    /**
     * @brief Compares standard keys for caller quota grouping, excluding per-callback host binder identity.
     */
    bool IsStandardAgentQuotaKeyMatched(const StandardAgentKey &storedKey, const StandardAgentKey &incomingKey) const;
    /**
     * @brief Finds an existing standard session matching the given key.
     */
    std::shared_ptr<StandardAgentSession> FindStandardSessionLocked(const StandardAgentKey &key) const;
    /**
     * @brief Finds a standard session by the service-side wrapper binder used for AMS callbacks.
     */
    StandardSessionIter FindStandardSessionByServiceRemoteLocked(const sptr<IRemoteObject> &serviceRemote);
    /**
     * @brief Registers a caller callback against an existing standard session without consuming another quota slot.
     */
    int32_t RegisterStandardSessionCallerLocked(const std::shared_ptr<StandardAgentSession> &session,
        const sptr<AAFwk::IAbilityConnection> &connection,
        const sptr<AAFwk::IAbilityConnection> &serviceConnection, bool countTowardsCallerLimit,
        bool forcePending, bool &isNewCaller);
    int32_t EnsureStandardTrackedCallerLocked(const std::shared_ptr<StandardAgentSession> &session,
        const sptr<AAFwk::IAbilityConnection> &connection,
        const sptr<AAFwk::IAbilityConnection> &serviceConnection, bool countTowardsCallerLimit,
        sptr<IRemoteObject> &callerRemote);
    void AddStandardSessionCallbackLocked(const std::shared_ptr<StandardAgentSession> &session,
        const sptr<AAFwk::IAbilityConnection> &connection,
        const sptr<AAFwk::IAbilityConnection> &serviceConnection, bool forcePending);
    /**
     * @brief Removes one newly registered standard caller after its real connect request fails immediately.
     */
    void RollbackStandardSessionCallerLocked(const std::shared_ptr<StandardAgentSession> &session,
        const sptr<AAFwk::IAbilityConnection> &connection);
    /**
     * @brief Checks whether the caller already holds quota for the same standard agent target.
     */
    bool HasStandardAgentQuotaLocked(const StandardAgentKey &key) const;
    /**
     * @brief Transfers a standard-agent quota slot to another logical connection when possible.
     */
    bool TransferStandardCallerLimitLocked(const StandardAgentKey &disconnectingKey,
        sptr<IRemoteObject> &transferFrom, sptr<IRemoteObject> &transferTo);
    /**
     * @brief Creates a new collapsed standard session, registers one quota, and issues the real AMS connect.
     */
    int32_t CreateStandardAgentSession(const AAFwk::Want &connectWant, const std::string &agentId,
        const StandardAgentKey &key, const sptr<AAFwk::IAbilityConnection> &connection);
    /**
     * @brief Handles AMS connect completion for a standard collapsed session.
     */
    void HandleStandardAgentConnectDone(const sptr<IRemoteObject> &serviceRemote,
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode);
    bool HandlePendingStandardConnectDoneLocked(const sptr<IRemoteObject> &serviceRemote,
        const sptr<IRemoteObject> &remoteObject, int32_t resultCode,
        const StandardSessionIter &sessionIt, std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks,
        std::vector<sptr<AAFwk::IAbilityConnection>> &failedCallbacks);
    void HandleLegacyStandardConnectDoneLocked(const sptr<IRemoteObject> &remoteObject, int32_t resultCode,
        const StandardSessionIter &sessionIt, std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks,
        std::vector<sptr<AAFwk::IAbilityConnection>> &failedCallbacks);
    /**
     * @brief Handles AMS disconnect completion for a standard collapsed session.
     */
    void HandleStandardAgentDisconnectDone(const sptr<IRemoteObject> &serviceRemote,
        const AppExecFwk::ElementName &element, int32_t resultCode);
    int32_t RemoveCompletedLowCodeAgentLocked(const std::string &agentId,
        sptr<AgentHostConnection> &hostConnection, LowCodeCompleteRollbackInfo &rollbackInfo);
    bool LowCodeCallerStillOwnsAgentLocked(const std::shared_ptr<AgentHostSession> &session,
        const std::string &agentId, const sptr<IRemoteObject> &callerRemote) const;
    void RemoveCompletedLowCodeCallerLocked(const std::shared_ptr<AgentHostSession> &session,
        const sptr<IRemoteObject> &callerRemote, const sptr<IRemoteObject> &hostProxyRemote,
        LowCodeCompleteRollbackInfo &rollbackInfo);
    void RollbackLowCodeAgentCompleteLocked(const std::string &agentId,
        const LowCodeCompleteRollbackInfo &rollbackInfo);
    /**
     * @brief Marks the target low-code agent as invoked so future callers can detect duplicate activation.
     */
    bool NotifyAgentInvokedLocked(const AgentHostSession &session, const std::string &agentId);
    /**
     * @brief Clears reverse owner indexes for every low-code agent served by one shared host session.
     */
    void EraseAgentOwnersLocked(const AgentHostSession &session);
    /**
     * @brief Handles AMS completion for the shared low-code host connect.
     */
    void HandleAgentHostConnectDone(const AgentHostKey &key, const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode);
    void CollectPendingLowCodeConnectCallbacksLocked(const std::shared_ptr<AgentHostSession> &session,
        const sptr<IRemoteObject> &hostProxyRemote, std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks,
        std::vector<std::string> &pendingAgentIds);
    void CleanupFailedLowCodeHostConnectLocked(const std::shared_ptr<AgentHostSession> &session,
        const AgentHostKey &key, const sptr<IRemoteObject> &hostProxyRemote,
        const std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks,
        const std::vector<std::string> &pendingAgentIds);
    /**
     * @brief Handles AMS completion for the shared low-code host disconnect.
     */
    void HandleAgentHostDisconnectDone(const AgentHostKey &key, const AppExecFwk::ElementName &element,
        int resultCode);
    /**
     * @brief Drops one shared host session and its reverse indexes after disconnect or rollback.
     */
    void ClearAgentHostSessionLocked(const AgentHostKey &key);
    DISALLOW_COPY_AND_MOVE(AgentManagerService);

private:
    static constexpr size_t MAX_CONNECTIONS_PER_CALLER = 5;
    static constexpr size_t MAX_AGENTS_PER_HOST_SESSION = 100;
    static sptr<AgentManagerService> instance_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AgentEventHandler> eventHandler_;
    sptr<AgentBundleEventCallback> bundleEventCallback_;
    std::mutex connectionLock_;
    std::map<sptr<IRemoteObject>, TrackedConnectionRecord> trackedConnections_;
    std::map<int32_t, size_t> callerConnectionCounts_;
    std::mutex agentHostMutex_;
    std::list<std::shared_ptr<AgentHostSession>> agentHostSessions_;
    std::list<AgentOwnerRecord> agentOwners_;
    std::list<std::shared_ptr<StandardAgentSession>> standardSessions_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_MANAGER_SERVICE_H
