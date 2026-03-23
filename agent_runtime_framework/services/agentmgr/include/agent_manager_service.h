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

    int32_t RegisterAgentCard(const AgentCard &card) override;

    int32_t UpdateAgentCard(const AgentCard &card) override;

    int32_t DeleteAgentCard(const std::string &bundleName, const std::string &agentId) override;

    int32_t ConnectAgentExtensionAbility(const AAFwk::Want &want,
        const sptr<AAFwk::IAbilityConnection> &connection) override;

    int32_t DisconnectAgentExtensionAbility(const sptr<AAFwk::IAbilityConnection> &connection) override;

    int32_t NotifyWorkflowAgentComplete(const std::string &agentId) override;

private:
    struct WorkflowHostKey {
        int32_t callingUid = 0;
        std::string bundleName;
        std::string moduleName;
        std::string abilityName;

        bool operator<(const WorkflowHostKey &that) const
        {
            if (callingUid != that.callingUid) {
                return callingUid < that.callingUid;
            }
            if (bundleName != that.bundleName) {
                return bundleName < that.bundleName;
            }
            if (moduleName != that.moduleName) {
                return moduleName < that.moduleName;
            }
            return abilityName < that.abilityName;
        }
    };

    struct WorkflowAgentOwnerKey {
        int32_t callingUid = 0;
        std::string agentId;

        bool operator<(const WorkflowAgentOwnerKey &that) const
        {
            if (callingUid != that.callingUid) {
                return callingUid < that.callingUid;
            }
            return agentId < that.agentId;
        }
    };

    class WorkflowHostConnection : public AAFwk::AbilityConnectionStub {
    public:
        explicit WorkflowHostConnection(const WorkflowHostKey &key);
        ~WorkflowHostConnection() override = default;

        void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
            const sptr<IRemoteObject> &remoteObject, int resultCode) override;
        void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

    private:
        WorkflowHostKey key_;
    };

    struct WorkflowHostSession {
        WorkflowHostKey key;
        sptr<WorkflowHostConnection> hostConnection;
        std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
        std::set<std::string> activeAgentIds;
        std::set<std::string> pendingAgentIds;
        AppExecFwk::ElementName element;
        sptr<IRemoteObject> remoteObject;
        int32_t resultCode = ERR_OK;
        bool isConnected = false;
        bool isDisconnecting = false;
    };

    AgentManagerService();
    void Init();
    void RegisterBundleEventCallback();
    int32_t ConnectWorkflowAgentExtensionAbility(const AAFwk::Want &want,
        const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid);
    WorkflowHostKey BuildWorkflowHostKey(const AAFwk::Want &want, int32_t callingUid) const;
    bool NotifyWorkflowInvokedLocked(const WorkflowHostSession &session, const std::string &agentId);
    void EraseWorkflowOwnersLocked(const WorkflowHostSession &session);
    void HandleWorkflowConnectDone(const WorkflowHostKey &key, const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int resultCode);
    void HandleWorkflowDisconnectDone(const WorkflowHostKey &key, const AppExecFwk::ElementName &element,
        int resultCode);
    void ClearWorkflowSessionLocked(const WorkflowHostKey &key);
    DISALLOW_COPY_AND_MOVE(AgentManagerService);

private:
    static sptr<AgentManagerService> instance_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<AgentEventHandler> eventHandler_;
    sptr<AgentBundleEventCallback> bundleEventCallback_;
    std::mutex workflowMutex_;
    std::map<WorkflowHostKey, std::shared_ptr<WorkflowHostSession>> workflowHostSessions_;
    std::map<WorkflowAgentOwnerKey, std::shared_ptr<WorkflowHostSession>> workflowAgentOwners_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_MANAGER_SERVICE_H
