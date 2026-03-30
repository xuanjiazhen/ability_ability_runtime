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
#include "workflow_connection_types.h"
#include "workflow_host_connection.h"

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

    int32_t GetAgentCardsByBundleName(const std::string &bundleName, std::vector<AgentCard> &cards) override;

    int32_t GetAgentCardByAgentId(const std::string &bundleName, const std::string &agentId, AgentCard &card) override;

    int32_t GetCallerAgentCardByAgentId(const std::string &agentId, AgentCard &card) override;

    int32_t RegisterAgentCard(const AgentCard &card) override;

    int32_t UpdateAgentCard(const AgentCard &card) override;

    int32_t DeleteAgentCard(const std::string &bundleName, const std::string &agentId) override;

    int32_t ConnectAgentExtensionAbility(const AAFwk::Want &want,
        const sptr<AAFwk::IAbilityConnection> &connection) override;

    int32_t DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection) override;

    int32_t NotifyLowCodeAgentComplete(const std::string &agentId) override;

private:
    friend class AgentServiceConnection;
    friend class WorkflowHostConnection;
    struct TrackedConnectionRecord {
        int32_t callerUid = 0;
        sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
        sptr<IRemoteObject> callerRemote = nullptr;
        sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
        bool isDisconnecting = false;
    };

    AgentManagerService();
    void Init();
    void RegisterBundleEventCallback();
    int32_t ValidateConnectAgentRequest(const sptr<AAFwk::IAbilityConnection> &connection, int32_t &callerUid);
    int32_t ResolveConnectAgentTarget(const AAFwk::Want &want, AAFwk::Want &connectWant,
        std::string &agentId, AgentCard &card, int32_t &callingUid) const;
    int32_t PrepareStandardAgentConnectWant(AAFwk::Want &connectWant, const AgentCard &card, int32_t callingUid) const;
    int32_t RegisterTrackedConnectionAndGetServiceConnection(const sptr<AAFwk::IAbilityConnection> &connection,
        int32_t callerUid, sptr<AAFwk::IAbilityConnection> &serviceConnection);
    bool HasReachedCallerConnectionLimitLocked(int32_t callerUid) const;
    size_t GetWorkflowAgentCountByUidLocked(int32_t callerUid) const;
    int32_t TryRegisterConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid);
    void ReleaseCallerConnectionCountByUidLocked(int32_t callerUid);
    void HandleConnectionDone(const sptr<AAFwk::IAbilityConnection> &connection, int32_t resultCode, bool isDisconnect);
    bool ReleaseCallerConnectionCountLocked(const sptr<IRemoteObject> &callerRemote);
    void ReleaseTrackedConnection(const sptr<AAFwk::IAbilityConnection> &connection);
    void HandleCallerConnectionDied(const wptr<IRemoteObject> &remote);
    int32_t ConnectWorkflowAgentExtensionAbility(const AAFwk::Want &want,
        const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid);
    int32_t PrepareWorkflowConnectPlan(const WorkflowHostKey &hostKey, const std::string &agentId,
        const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid, WorkflowConnectPlan &plan);
    void NotifyExistingWorkflowConnection(const WorkflowConnectPlan &plan, const std::string &agentId,
        const sptr<AAFwk::IAbilityConnection> &connection);
    void CleanupWorkflowConnectPlan(const WorkflowConnectPlan &plan, const std::string &agentId, int32_t callingUid);
    int32_t CompleteWorkflowHostConnect(const AAFwk::Want &want, const std::string &agentId,
        int32_t callingUid, WorkflowConnectPlan &plan);
    WorkflowHostKey BuildWorkflowHostKey(const AAFwk::Want &want, int32_t callingUid) const;
    bool NotifyAgentInvokedLocked(const WorkflowHostSession &session, const std::string &agentId);
    void EraseWorkflowOwnersLocked(const WorkflowHostSession &session);
    void HandleWorkflowConnectDone(const WorkflowHostKey &key, const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode);
    void HandleWorkflowDisconnectDone(const WorkflowHostKey &key, const AppExecFwk::ElementName &element,
        int resultCode);
    void ClearWorkflowSessionLocked(const WorkflowHostKey &key);
    DISALLOW_COPY_AND_MOVE(AgentManagerService);

private:
    static constexpr size_t MAX_CONNECTIONS_PER_CALLER = 5;
    static constexpr size_t MAX_WORKFLOW_AGENTS_PER_CALLER = 20;
    static constexpr size_t MAX_WORKFLOW_AGENTS_PER_HOST_SESSION = 10;
    static sptr<AgentManagerService> instance_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AgentEventHandler> eventHandler_;
    sptr<AgentBundleEventCallback> bundleEventCallback_;
    std::mutex connectionLock_;
    std::map<sptr<IRemoteObject>, TrackedConnectionRecord> trackedConnections_;
    std::map<int32_t, size_t> callerConnectionCounts_;
    std::mutex workflowMutex_;
    std::map<WorkflowHostKey, std::shared_ptr<WorkflowHostSession>> workflowHostSessions_;
    std::map<WorkflowAgentOwnerKey, std::shared_ptr<WorkflowHostSession>> workflowAgentOwners_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_MANAGER_SERVICE_H
