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
#include "agent_connection_record.h"
#include "agent_standard_record.h"
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
 * Carries the single BundleMgr resolution result used by all agent connect branches.
 */
struct AgentResolvedTarget {
    AAFwk::Want connectWant;
    std::string agentId;
    AgentCard card;
    int32_t callingUid = 0;
    int32_t userId = 0;
    int32_t targetUid = 0;
    int32_t targetAppIndex = 0;
    std::string targetInstanceKey;
};

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

    using TrackedConnectionIter = std::map<sptr<IRemoteObject>, TrackedConnectionRecord>::iterator;
    using StandardSessionIter = std::list<std::shared_ptr<AgentStandardRecord>>::iterator;
    using AgentHostSessionIter = std::list<AgentHostSessionRecord>::iterator;
    using AgentOwnerIter = std::list<AgentOwnerRecord>::iterator;

    AgentManagerService();
    void Init();
    void RegisterBundleEventCallback();
    /**
     * Reads the current IPC caller's uid and exact RunningProcessInfo.instanceKey.
     */
    AgentCallerIdentity GetCurrentCallerIdentity() const;
    /**
     * Validates an agent connect request and returns the caller identity used for quota and session ownership.
     */
    int32_t ValidateConnectAgentRequest(const sptr<AAFwk::IAbilityConnection> &connection,
        AgentCallerIdentity &callerIdentity);
    /**
     * Compatibility overload for existing private tests and uid-only call paths.
     */
    int32_t ValidateConnectAgentRequest(const sptr<AAFwk::IAbilityConnection> &connection, int32_t &callerUid);
    /**
     * Resolves the AgentCard target once and builds the Want consumed by all connect branches.
     */
    int32_t ResolveConnectAgentTarget(const AAFwk::Want &want, AgentResolvedTarget &target) const;
    /**
     * Performs the single BundleMgr preliminary check for an agent connect request.
     */
    int32_t ResolveAgentExtensionAbility(AgentResolvedTarget &target) const;
    /**
     * Connects or reuses a resolved standard agent session after AgentCard lookup has completed.
     */
    int32_t ConnectStandardAgentExtensionAbility(AgentResolvedTarget &target,
        const sptr<AAFwk::IAbilityConnection> &connection, const AgentCallerIdentity &callerIdentity);
    /**
     * Validates service-extension connect input for the caller-token based connect path.
     */
    int32_t ValidateConnectServiceRequest(const sptr<IRemoteObject> &callerToken,
        const sptr<AAFwk::IAbilityConnection> &connection) const;
    /**
     * Resolves the target service extension and builds the Want sent to AMS.
     */
    int32_t PrepareServiceConnectWant(const AAFwk::Want &want, AAFwk::Want &connectWant) const;
    /**
     * Registers one caller-visible connection and returns the service-side wrapper sent to AMS.
     */
    int32_t RegisterTrackedConnectionAndGetServiceConnection(const sptr<AAFwk::IAbilityConnection> &connection,
        const AgentCallerIdentity &callerIdentity, bool countTowardsCallerLimit,
        sptr<AAFwk::IAbilityConnection> &serviceConnection);
    /**
     * Compatibility overload; new agent paths should pass AgentCallerIdentity.
     */
    int32_t RegisterTrackedConnectionAndGetServiceConnection(const sptr<AAFwk::IAbilityConnection> &connection,
        int32_t callerUid, bool countTowardsCallerLimit, sptr<AAFwk::IAbilityConnection> &serviceConnection);
    /**
     * Checks the per-caller-instance quota, not just per uid.
     */
    bool HasReachedCallerConnectionLimitLocked(const AgentCallerIdentity &callerIdentity) const;
    /**
     * Installs one tracked record, optionally binding it to a low-code host and caller quota slot.
     */
    int32_t TryRegisterConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection,
        const AgentCallerIdentity &callerIdentity,
        const sptr<AAFwk::IAbilityConnection> &serviceConnection = nullptr, const AgentHostKey *hostKey = nullptr,
        bool countTowardsCallerLimit = true);
    int32_t TryRegisterConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid,
        const sptr<AAFwk::IAbilityConnection> &serviceConnection = nullptr, const AgentHostKey *hostKey = nullptr,
        bool countTowardsCallerLimit = true);
    /**
     * Checks duplicate connection state and the caller-instance quota before creating a tracked record.
     */
    int32_t ValidateTrackedConnectionRegistrationLocked(const sptr<IRemoteObject> &callerRemote,
        const AgentCallerIdentity &callerIdentity, bool countTowardsCallerLimit, size_t &currentCount) const;
    /**
     * Reuses an existing service wrapper when the caller already supplied one, otherwise creates a wrapper.
     */
    sptr<AAFwk::IAbilityConnection> GetOrCreateServiceConnection(
        const sptr<AAFwk::IAbilityConnection> &connection,
        const sptr<AAFwk::IAbilityConnection> &serviceConnection) const;
    /**
     * Builds the common tracking record used by standard and low-code connection ownership.
     */
    TrackedConnectionRecord BuildTrackedConnectionRecord(const sptr<IRemoteObject> &callerRemote,
        const AgentCallerIdentity &callerIdentity, const sptr<AAFwk::IAbilityConnection> &serviceConnection,
        const AgentHostKey *hostKey, bool countTowardsCallerLimit);
    /**
     * Subscribes to caller process death so ownership can be cleaned without tearing shared sessions early.
     */
    void AddCallerDeathRecipient(TrackedConnectionRecord &record);
    /**
     * Returns the remote object that uniquely represents one caller callback/proxy owner.
     */
    sptr<IRemoteObject> GetConnectionIdentityRemote(const sptr<AAFwk::IAbilityConnection> &connection) const;
    /**
     * Finds the tracked record for the exact caller identity behind the supplied callback/proxy.
     */
    TrackedConnectionIter FindTrackedConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection,
        const AgentCallerIdentity &callerIdentity);
    /**
     * Compatibility fallback; it represents the default instance for uid-only callers.
     */
    TrackedConnectionIter FindTrackedConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection,
        int32_t callerUid);
    int32_t PrepareAgentDisconnectLocked(const sptr<AAFwk::IAbilityConnection> &connection,
        AgentDisconnectPlan &plan);
    /**
     * Builds a disconnect plan for one low-code owner and decides whether the shared host must be disconnected.
     */
    int32_t PrepareLowCodeDisconnectLocked(TrackedConnectionIter it, AgentDisconnectPlan &plan);
    /**
     * Builds a disconnect plan for one standard owner and keeps the backing session when other owners survive.
     */
    int32_t PrepareStandardDisconnectLocked(TrackedConnectionIter it, AgentDisconnectPlan &plan);
    /**
     * Finds a standard backing session with the exact caller/target/callback identity.
     */
    StandardSessionIter FindStandardSessionIterLocked(const StandardAgentKey &key);
    /**
     * Marks one standard owner for disconnect while preserving shared backing state as needed.
     */
    int32_t PrepareStandardSessionDisconnectLocked(TrackedConnectionIter it,
        const std::shared_ptr<AgentStandardRecord> &session, AgentDisconnectPlan &plan);
    /**
     * Restores connection state when AMS rejects a disconnect requested by a prepared plan.
     */
    void RollbackAgentDisconnectLocked(const AgentDisconnectPlan &plan);
    /**
     * Releases one quota slot for the exact caller identity that reserved it.
     */
    void ReleaseCallerConnectionCountByIdentityLocked(const AgentCallerIdentity &callerIdentity);
    void HandleConnectionDone(const sptr<AAFwk::IAbilityConnection> &connection, int32_t resultCode,
        bool isDisconnect);
    bool ReleaseCallerConnectionCountLocked(const sptr<IRemoteObject> &callerRemote);
    void ReleaseTrackedConnection(const sptr<AAFwk::IAbilityConnection> &connection);
    /**
     * Removes tracking for one owner and releases its quota slot unless it is already disconnecting.
     */
    void ReleaseTrackedConnectionByRemoteLocked(const sptr<IRemoteObject> &callerRemote);
    /**
     * Moves a low-code quota slot to another same-identity owner before the selected owner is removed.
     */
    void TransferLowCodeCallerLimitLocked(const std::shared_ptr<AgentHostSession> &session,
        const sptr<IRemoteObject> &callerRemote);
    /**
     * Checks whether this low-code host already has an owner for the exact caller identity.
     */
    bool HasLowCodeCallerIdentityLocked(const std::shared_ptr<AgentHostSession> &session,
        const AgentCallerIdentity &callerIdentity) const;
    bool IsLowCodeHostKeyMatched(const AgentHostKey &left, const AgentHostKey &right) const;
    bool IsAgentOwnerKeyEqual(const AgentOwnerKey &left, const AgentOwnerKey &right) const;
    AgentHostSessionIter FindAgentHostSessionLocked(const AgentHostKey &key);
    AgentOwnerIter FindAgentOwnerLocked(const AgentOwnerKey &key);
    void SetAgentOwnerLocked(const AgentOwnerKey &key, const std::shared_ptr<AgentHostSession> &session);
    void EraseAgentOwnerLocked(const AgentOwnerKey &key);
    void ClearLowCodeHostSessionLocked(AgentHostSessionIter sessionIter);
    void HandleCallerConnectionDied(const wptr<IRemoteObject> &remote);
    void HandleCallerConnectionDied(const sptr<IRemoteObject> &remote);
    /**
     * Cleans one dead standard owner; the shared backing service is disconnected only for the last owner.
     */
    void CleanupDeadStandardConnectionLocked(TrackedConnectionIter it, const sptr<IRemoteObject> &remote,
        sptr<AAFwk::IAbilityConnection> &serviceConnection);
    /**
     * Cleans a dead standard owner after its shared backing session has been found.
     */
    void CleanupDeadStandardSessionLocked(StandardSessionIter sessionIt, TrackedConnectionIter it,
        const sptr<IRemoteObject> &remote, sptr<AAFwk::IAbilityConnection> &serviceConnection);
    /**
     * Cleans one dead low-code owner and disconnects the host only after the last AgentId owner is gone.
     */
    void CleanupDeadLowCodeConnectionLocked(TrackedConnectionIter it, const sptr<IRemoteObject> &remote,
        sptr<AgentHostConnection> &hostConnection);
    void DisconnectDeadServiceConnection(const sptr<AAFwk::IAbilityConnection> &serviceConnection);
    void DisconnectDeadHostConnection(const sptr<AgentHostConnection> &hostConnection);
    int32_t ValidateNotifyLowCodeAgentCompleteRequest(const std::string &agentId) const;
    /**
     * Removes one completed low-code AgentId and prepares host disconnect for its own connect path.
     */
    int32_t PrepareNotifyLowCodeAgentCompleteLocked(const std::string &agentId,
        sptr<AgentHostConnection> &hostConnection, sptr<AAFwk::IAbilityConnection> &callerConnection,
        AppExecFwk::ElementName &element);
    /**
     * Finds the low-code owner/session pair being completed and copies its per-AgentId state.
     */
    int32_t ResolveNotifyLowCodeAgentLocked(const std::string &agentId, AgentOwnerIter &ownerIter,
        std::shared_ptr<AgentHostSession> &session, LowCodeAgentRecord &agentRecord);
    /**
     * Prepares the caller-side disconnect callback when no target-side disconnect will follow.
     */
    void PrepareLowCodeCompleteCallerCallbackLocked(const std::shared_ptr<AgentHostSession> &session,
        const LowCodeAgentRecord &agentRecord, bool callerStillOwnsAgent, bool shouldDisconnectHostProxy,
        sptr<AAFwk::IAbilityConnection> &callerConnection, AppExecFwk::ElementName &element);
    /**
     * Releases the completed low-code caller owner and transfers its quota when possible.
     */
    void ReleaseLowCodeCompleteCallerLocked(const std::shared_ptr<AgentHostSession> &session,
        const LowCodeAgentRecord &agentRecord, bool callerStillOwnsAgent, bool shouldDisconnectHostProxy);
    /**
     * Decides whether completion should disconnect this low-code owner's target-side connection.
     */
    int32_t PrepareLowCodeCompleteBackingDisconnectLocked(const std::shared_ptr<AgentHostSession> &session,
        const LowCodeAgentRecord &agentRecord, bool callerStillOwnsAgent, bool shouldDisconnectHostProxy,
        sptr<AgentHostConnection> &hostConnection);
    void RollbackNotifyLowCodeAgentCompleteDisconnect(const sptr<AgentHostConnection> &hostConnection);
    /**
     * Connects or reuses the shared low-code host session while preserving per-AgentId ownership.
     */
    int32_t ConnectLowCodeAgentExtensionAbility(const AgentResolvedTarget &target,
        const sptr<AAFwk::IAbilityConnection> &connection, const AgentCallerIdentity &callerIdentity);
    /**
     * Creates the low-code connect plan, rejects duplicate active AgentIds, and records the caller owner.
     */
    int32_t PrepareLowCodeConnectPlan(const AgentHostKey &hostKey, int32_t hostUid, const std::string &agentId,
        const sptr<AAFwk::IAbilityConnection> &connection, const AgentCallerIdentity &callerIdentity,
        AgentConnectPlan &plan);
    /**
     * Validates caller/proxy identity before a low-code owner can be attached to a host session.
     */
    int32_t ValidateLowCodeConnectPlanLocked(const sptr<AAFwk::IAbilityConnection> &connection,
        const AgentHostKey &hostKey, AgentConnectPlan &plan, sptr<IRemoteObject> &hostProxyRemote);
    /**
     * Finds a reusable low-code host session or creates one when this is the first owner.
     */
    int32_t GetOrCreateLowCodeSessionLocked(const AgentHostKey &hostKey, int32_t hostUid,
        const sptr<IRemoteObject> &hostProxyRemote, AgentConnectPlan &plan,
        std::shared_ptr<AgentHostSession> &session);
    /**
     * Ensures clone/instance-specific host proxy bookkeeping exists before registering an owner.
     */
    int32_t EnsureLowCodeHostProxyConnectionLocked(const AgentHostKey &hostKey,
        const sptr<IRemoteObject> &hostProxyRemote, AgentConnectPlan &plan,
        const std::shared_ptr<AgentHostSession> &session);
    /**
     * Rolls back host-session bookkeeping when tracking registration fails during low-code connect.
     */
    void RollbackLowCodeConnectRegistrationLocked(const AgentHostKey &hostKey,
        const sptr<IRemoteObject> &hostProxyRemote, const std::shared_ptr<AgentHostSession> &session,
        const AgentConnectPlan &plan);
    /**
     * Records one low-code AgentId owner and caches connect completion when the host is already connected.
     */
    void RegisterLowCodeAgentLocked(const std::string &agentId,
        const sptr<AAFwk::IAbilityConnection> &connection, const AgentCallerIdentity &callerIdentity,
        const sptr<IRemoteObject> &hostProxyRemote, const std::shared_ptr<AgentHostSession> &session,
        AgentConnectPlan &plan);
    /**
     * Delivers an already-established shared host connection to a newly registered low-code owner.
     */
    void NotifyExistingLowCodeConnection(const AgentConnectPlan &plan, const std::string &agentId,
        const sptr<AAFwk::IAbilityConnection> &connection);
    /**
     * Removes low-code owner state after AMS rejects the real host connect.
     */
    void CleanupLowCodeConnectPlan(const AgentConnectPlan &plan, const std::string &agentId);
    /**
     * Performs the real AMS connect for a low-code host session that is not connected yet.
     */
    int32_t CompleteAgentHostConnect(const AAFwk::Want &want, const std::string &agentId,
        AgentConnectPlan &plan);
    /**
     * Extracts the low-code host-session identity from target element and caller user context.
     */
    AgentHostKey BuildAgentHostKey(const AgentResolvedTarget &target) const;
    /**
     * Compatibility overload for legacy uid-only unit tests.
     */
    StandardAgentKey BuildStandardAgentKey(int32_t callerUid, const std::string &agentId,
        const AAFwk::Want &want) const;
    bool IsStandardAgentKeyEqual(const StandardAgentKey &left, const StandardAgentKey &right) const;
    bool IsStandardAgentKeyMatched(const StandardAgentKey &storedKey, const StandardAgentKey &incomingKey) const;
    std::shared_ptr<AgentStandardRecord> FindStandardSessionLocked(const StandardAgentKey &key) const;
    StandardSessionIter FindStandardSessionByServiceRemoteLocked(const sptr<IRemoteObject> &serviceRemote);
    /**
     * Adds one standard caller owner to a collapsed backing session.
     */
    int32_t RegisterStandardSessionCallerLocked(const std::shared_ptr<AgentStandardRecord> &session,
        const sptr<AAFwk::IAbilityConnection> &connection, bool countTowardsCallerLimit,
        const sptr<AAFwk::IAbilityConnection> &serviceConnection = nullptr);
    /**
     * Reuses a connected/connecting standard backing session when target identity matches.
     */
    int32_t TryConnectExistingAgentStandardRecord(const AAFwk::Want &connectWant, const StandardAgentKey &key,
        const sptr<AAFwk::IAbilityConnection> &connection, bool &isExistingSession);
    /**
     * Moves a standard quota slot to another same-identity owner before removing the selected owner.
     */
    bool TransferStandardCallerLimitWithinSessionLocked(const std::shared_ptr<AgentStandardRecord> &session,
        const sptr<IRemoteObject> &transferFrom, sptr<IRemoteObject> &transferTo);
    /**
     * Removes one standard owner and keeps the backing session state for surviving owners.
     */
    bool RemoveStandardSessionCallerLocked(const std::shared_ptr<AgentStandardRecord> &session,
        const sptr<IRemoteObject> &callerRemote, const sptr<AAFwk::IAbilityConnection> &serviceConnection);
    /**
     * Creates the first backing session for a standard AgentId/target/caller-instance identity.
     */
    int32_t CreateAgentStandardRecord(const AAFwk::Want &connectWant, const std::string &agentId,
        const StandardAgentKey &key, const sptr<AAFwk::IAbilityConnection> &connection);
    void HandleStandardAgentConnectDone(const sptr<IRemoteObject> &serviceRemote,
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode);
    void HandleStandardAgentDisconnectDone(const sptr<IRemoteObject> &serviceRemote,
        const AppExecFwk::ElementName &element, int32_t resultCode);
    void EraseAgentOwnersLocked(const AgentHostSession &session);
    void HandleAgentHostConnectDone(const AgentHostKey &key, const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode);
    bool PrepareAgentHostConnectDoneLocked(const AgentHostKey &key, const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode,
        std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks, std::vector<std::string> &pendingAgentIds);
    void NotifyLowCodeConnectDoneCallbacks(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode,
        const std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks,
        const std::vector<std::string> &pendingAgentIds);
    void HandleAgentHostDisconnectDone(const AgentHostKey &key, const AppExecFwk::ElementName &element,
        int resultCode);
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
    std::map<AgentCallerIdentity, size_t> callerConnectionCounts_;
    std::mutex agentHostMutex_;
    std::list<AgentHostSessionRecord> agentHostSessions_;
    std::list<AgentOwnerRecord> agentOwners_;
    std::list<std::shared_ptr<AgentStandardRecord>> standardSessions_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_MANAGER_SERVICE_H
