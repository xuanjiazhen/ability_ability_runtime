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

#include "agent_manager_service.h"

#include <algorithm>

#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "agent_ams_caller_identity.h"
#include "agent_extension_connection_constants.h"
#include "agent_internal_constants.h"
#include "agent_receiver_proxy.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "permission_constants.h"
#include "permission_verification.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr int32_t BASE_USER_RANGE = 200000;
}

int32_t AgentManagerService::DisconnectLowCodeTrackedConnection(const sptr<IRemoteObject> &callerRemote,
    const AgentHostKey &hostKey)
{
    std::shared_ptr<AgentHostSession> session;
    std::vector<std::string> agentIds;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto trackedIter = trackedConnections_.find(callerRemote);
        if (trackedIter == trackedConnections_.end() || !trackedIter->second.isLowCode) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Low-code connection not tracked");
            return ERR_INVALID_VALUE;
        }
        if (trackedIter->second.isDisconnecting) {
            return ERR_OK;
        }
        auto sessionIter = agentHostSessions_.find(hostKey);
        if (sessionIter == agentHostSessions_.end() || sessionIter->second == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Low-code host session missing");
            return ERR_INVALID_VALUE;
        }
        session = sessionIter->second;
        if (session->isDisconnecting) {
            return ERR_OK;
        }
        trackedIter->second.isDisconnecting = true;
        for (const auto &agentEntry : session->agents) {
            if (agentEntry.second.callerRemote == callerRemote) {
                agentIds.emplace_back(agentEntry.first);
            }
        }
    }

    sptr<AAFwk::IAbilityConnection> callerConnection = nullptr;
    sptr<AgentHostConnection> hostConnection = nullptr;
    AppExecFwk::ElementName element;
    bool notifyCallerDisconnect = false;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto sessionIter = agentHostSessions_.find(hostKey);
        if (sessionIter == agentHostSessions_.end() || sessionIter->second == nullptr) {
            ReleaseTrackedConnectionByRemoteLocked(callerRemote);
            return ERR_OK;
        }
        session = sessionIter->second;
        element = session->element;
        auto callerIter = session->callerConnections.find(callerRemote);
        if (callerIter != session->callerConnections.end()) {
            callerConnection = callerIter->second;
        }
        for (const auto &agentId : agentIds) {
            auto agentIter = session->agents.find(agentId);
            if (agentIter == session->agents.end() || agentIter->second.callerRemote != callerRemote) {
                continue;
            }
            ReleaseLowCodeAgentQuotaLocked(*session, agentId, agentIter->second.callerUid,
                BuildLowCodeQuotaKey(session->key));
            session->agents.erase(agentIter);
            agentOwners_.erase(AgentOwnerKey { agentIter->second.callerUid, agentId });
        }
        ReleaseTrackedConnectionByRemoteLocked(callerRemote);
        if (session->agents.empty()) {
            if (!session->isDisconnecting) {
                session->isDisconnecting = true;
                hostConnection = session->hostConnection;
            }
        } else {
            session->callerConnections.erase(callerRemote);
            notifyCallerDisconnect = true;
        }
    }

    if (notifyCallerDisconnect && callerConnection != nullptr) {
        callerConnection->OnAbilityDisconnectDone(element, ERR_OK);
    }
    if (hostConnection == nullptr) {
        return ERR_OK;
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(hostConnection);
    if (ret != ERR_OK) {
        std::lock_guard<std::mutex> lock(agentHostMutex_);
        auto sessionIter = agentHostSessions_.find(hostKey);
        if (sessionIter != agentHostSessions_.end() && sessionIter->second != nullptr) {
            sessionIter->second->isDisconnecting = false;
        }
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

int32_t AgentManagerService::NotifyLowCodeAgentComplete(const std::string &agentId)
{
    if (!AAFwk::PermissionVerification::GetInstance()->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "caller no system-app, can not use system-api");
        return AAFwk::ERR_NOT_SYSTEM_APP;
    }
    if (!AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
        AAFwk::PermissionConstants::PERMISSION_CONNECT_AGENT)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Permission verification failed");
        return ERR_PERMISSION_DENIED;
    }
    if (agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty agentId");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    std::shared_ptr<AgentHostSession> session;
    sptr<AgentHostConnection> hostConnection = nullptr;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        AgentOwnerKey ownerKey { IPCSkeleton::GetCallingUid(), agentId };
        auto ownerIter = agentOwners_.find(ownerKey);
        if (ownerIter == agentOwners_.end()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code agent not found");
            return AAFwk::ERR_INVALID_AGENT_CARD_ID;
        }
        session = ownerIter->second;
        if (session == nullptr) {
            agentOwners_.erase(ownerIter);
            return AAFwk::CONNECTION_NOT_EXIST;
        }
        auto agentIter = session->agents.find(agentId);
        if (agentIter == session->agents.end()) {
            agentOwners_.erase(ownerIter);
            TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code agent bookkeeping missing");
            return AAFwk::CONNECTION_NOT_EXIST;
        }
    }

    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        AgentOwnerKey ownerKey { IPCSkeleton::GetCallingUid(), agentId };
        auto ownerIter = agentOwners_.find(ownerKey);
        if (ownerIter == agentOwners_.end() || ownerIter->second != session) {
            return ERR_OK;
        }
        auto agentIter = session->agents.find(agentId);
        if (agentIter == session->agents.end()) {
            agentOwners_.erase(ownerIter);
            return ERR_OK;
        }
        auto callerRemote = agentIter->second.callerRemote;
        auto callerUid = agentIter->second.callerUid;
        bool callerStillOwnsAgent = false;
        for (const auto &entry : session->agents) {
            if (entry.first == agentId) {
                continue;
            }
            if (entry.second.callerRemote == callerRemote) {
                callerStillOwnsAgent = true;
                break;
            }
        }
        ReleaseLowCodeAgentQuotaLocked(*session, agentId, callerUid, BuildLowCodeQuotaKey(session->key));
        session->agents.erase(agentId);
        agentOwners_.erase(ownerIter);
        if (!callerStillOwnsAgent && callerRemote != nullptr) {
            session->callerConnections.erase(callerRemote);
            ReleaseTrackedConnectionByRemoteLocked(callerRemote);
        }
        if (!session->agents.empty() || session->isDisconnecting) {
            return ERR_OK;
        }
        session->isDisconnecting = true;
        hostConnection = session->hostConnection;
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(hostConnection);
    if (ret != ERR_OK) {
        std::lock_guard<std::mutex> lock(agentHostMutex_);
        auto sessionIter = std::find_if(agentHostSessions_.begin(), agentHostSessions_.end(),
            [hostConnection](const auto &item) {
                return item.second != nullptr && item.second->hostConnection == hostConnection;
            });
        if (sessionIter != agentHostSessions_.end()) {
            sessionIter->second->isDisconnecting = false;
        }
        TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

int32_t AgentManagerService::ConnectLowCodeAgentExtensionAbility(const AAFwk::Want &want,
    const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid,
    int32_t hostUid)
{
    AgentConnectPlan plan;
    auto ret = PrepareLowCodeConnectPlan(BuildAgentHostKey(want, callingUid), hostUid, agentId, connection,
        callingUid, plan);
    if (ret != ERR_OK) {
        return ret;
    }

    if (plan.reusedHostSession) {
        ret = AdmitExistingLowCodeAgent(want, agentId, plan);
        if (ret != ERR_OK) {
            CleanupLowCodeConnectPlan(plan, agentId);
            return ret;
        }
        if (plan.admittedNewAgent) {
            ret = RecordAdmittedLowCodeAgentLocked(plan, agentId, !plan.notifyExistingConnection);
            if (ret != ERR_OK) {
                CleanupLowCodeConnectPlan(plan, agentId);
                return ret;
            }
        }
        if (plan.notifyExistingConnection) {
            NotifyExistingLowCodeConnection(plan, agentId, connection);
        }
        return ERR_OK;
    }

    if (!plan.needRealConnect) {
        return ERR_OK;
    }
    return CompleteAgentHostConnect(want, agentId, plan);
}

int32_t AgentManagerService::PrepareLowCodeConnectPlan(const AgentHostKey &hostKey, int32_t hostUid,
    const std::string &agentId, const sptr<AAFwk::IAbilityConnection> &connection, int32_t callingUid,
    AgentConnectPlan &plan)
{
    plan.hostKey = hostKey;
    plan.hostUid = hostUid;
    plan.callerUid = callingUid;
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    AgentOwnerKey ownerKey { callingUid, agentId };

    auto callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection remote object is null");
        return ERR_INVALID_VALUE;
    }
    plan.callerRemote = callerRemote;

    auto sessionIter = agentHostSessions_.find(hostKey);
    std::shared_ptr<AgentHostSession> session;
    if (sessionIter != agentHostSessions_.end()) {
        session = sessionIter->second;
        if (session == nullptr) {
            agentHostSessions_.erase(sessionIter);
        }
    }
    if (session == nullptr) {
        session = std::make_shared<AgentHostSession>();
        session->key = hostKey;
        session->hostUid = hostUid;
        session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
        if (session->hostConnection == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Create host connection failed");
            return ERR_INVALID_VALUE;
        }
        agentHostSessions_[hostKey] = session;
        plan.needRealConnect = true;
    } else {
        if (session->isDisconnecting) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code host session is disconnecting");
            return ERR_INVALID_VALUE;
        }
        plan.reusedHostSession = true;
    }

    auto ret = TryRegisterConnectionLocked(connection, callingUid, session->hostConnection, &hostKey);
    if (ret != ERR_OK) {
        if (plan.needRealConnect) {
            agentHostSessions_.erase(hostKey);
        }
        return ret;
    }
    plan.registeredTrackedConnection = true;

    session->callerConnections[callerRemote] = connection;
    if (!plan.reusedHostSession) {
        auto quotaRet = AdmitLowCodeAgentLocked(callingUid, BuildLowCodeQuotaKey(hostKey), *session, agentId);
        if (quotaRet != ERR_OK) {
            if (plan.needRealConnect) {
                agentHostSessions_.erase(hostKey);
            }
            ReleaseTrackedConnectionByRemoteLocked(callerRemote);
            return quotaRet == AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE ? ERR_OK : quotaRet;
        }
        session->agents[agentId] = LowCodeAgentRecord { callerRemote, callingUid, !session->isConnected };
        agentOwners_[ownerKey] = session;
    }
    plan.hostConnection = session->hostConnection;
    plan.notifyExistingConnection = session->isConnected && session->remoteObject != nullptr;
    if (plan.notifyExistingConnection) {
        plan.cachedElement = session->element;
        plan.cachedRemoteObject = session->remoteObject;
        plan.cachedResultCode = session->resultCode;
    }
    return ERR_OK;
}

void AgentManagerService::NotifyExistingLowCodeConnection(const AgentConnectPlan &plan, const std::string &agentId,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    AgentHostSession callbackSession;
    callbackSession.remoteObject = plan.cachedRemoteObject;
    if (plan.admittedNewAgent) {
        NotifyAgentInvokedLocked(callbackSession, agentId);
    }
    connection->OnAbilityConnectDone(plan.cachedElement, plan.cachedRemoteObject, plan.cachedResultCode);
}

int32_t AgentManagerService::AdmitExistingLowCodeAgent(const AAFwk::Want &want, const std::string &agentId,
    AgentConnectPlan &plan)
{
    (void)want;
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    auto sessionIter = agentHostSessions_.find(plan.hostKey);
    if (sessionIter == agentHostSessions_.end() || sessionIter->second == nullptr) {
        return ERR_INVALID_VALUE;
    }
    auto ret = AdmitLowCodeAgentLocked(plan.callerUid, BuildLowCodeQuotaKey(plan.hostKey), *sessionIter->second,
        agentId);
    if (ret == AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE) {
        plan.admittedNewAgent = false;
        return ERR_OK;
    }
    if (ret != ERR_OK) {
        return ret;
    }
    plan.admittedNewAgent = true;
    return ERR_OK;
}

int32_t AgentManagerService::RecordAdmittedLowCodeAgentLocked(const AgentConnectPlan &plan,
    const std::string &agentId, bool isPending)
{
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    auto sessionIter = agentHostSessions_.find(plan.hostKey);
    if (sessionIter == agentHostSessions_.end() || sessionIter->second == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code host session missing after AMS admission");
        return ERR_INVALID_VALUE;
    }
    auto session = sessionIter->second;
    if (session->agents.find(agentId) != session->agents.end()) {
        return ERR_OK;
    }
    session->agents[agentId] = LowCodeAgentRecord { plan.callerRemote, plan.callerUid, isPending };
    agentOwners_[AgentOwnerKey { plan.callerUid, agentId }] = session;
    return ERR_OK;
}

void AgentManagerService::CleanupLowCodeConnectPlan(const AgentConnectPlan &plan, const std::string &agentId)
{
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    agentOwners_.erase(AgentOwnerKey { plan.callerUid, agentId });
    auto sessionIter = agentHostSessions_.find(plan.hostKey);
    if (sessionIter != agentHostSessions_.end() && sessionIter->second != nullptr) {
        auto session = sessionIter->second;
        auto agentIter = session->agents.find(agentId);
        if (agentIter != session->agents.end()) {
            ReleaseLowCodeAgentQuotaLocked(*session, agentId, agentIter->second.callerUid,
                BuildLowCodeQuotaKey(plan.hostKey));
        }
        session->agents.erase(agentId);
        if (plan.callerRemote != nullptr) {
            session->callerConnections.erase(plan.callerRemote);
        }
        if (session->callerConnections.empty() && session->agents.empty()) {
            agentHostSessions_.erase(sessionIter);
        }
    }
    if (plan.registeredTrackedConnection && plan.callerRemote != nullptr) {
        ReleaseTrackedConnectionByRemoteLocked(plan.callerRemote);
    }
}

int32_t AgentManagerService::CompleteAgentHostConnect(const AAFwk::Want &want, const std::string &agentId,
    AgentConnectPlan &plan)
{
    AAFwk::Want connectWant = want;
    AgentAmsCallerIdentityScope identityScope;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto sessionIter = agentHostSessions_.find(plan.hostKey);
        if (sessionIter != agentHostSessions_.end() && sessionIter->second != nullptr) {
            sessionIter->second->originalIdentity = identityScope.GetOriginalIdentity();
        }
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        connectWant, plan.hostConnection, nullptr, AAFwk::DEFAULT_INVAL_VALUE,
        AppExecFwk::ExtensionAbilityType::AGENT);
    if (ret != ERR_OK) {
        CleanupLowCodeConnectPlan(plan, agentId);
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

AgentHostKey AgentManagerService::BuildAgentHostKey(const AAFwk::Want &want, int32_t callingUid) const
{
    AgentHostKey key;
    key.userId = callingUid / BASE_USER_RANGE;
    key.appIndex = want.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0);
    key.bundleName = want.GetElement().GetBundleName();
    key.moduleName = want.GetElement().GetModuleName();
    key.abilityName = want.GetElement().GetAbilityName();
    return key;
}

bool AgentManagerService::NotifyAgentInvokedLocked(const AgentHostSession &session, const std::string &agentId)
{
    if (session.remoteObject == nullptr) {
        return false;
    }
    auto receiver = iface_cast<IAgentReceiver>(session.remoteObject);
    if (receiver == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agent receiver null");
        return false;
    }
    receiver->AgentInvoked(agentId);
    return true;
}

void AgentManagerService::EraseAgentOwnersLocked(const AgentHostSession &session)
{
    for (const auto &agentEntry : session.agents) {
        agentOwners_.erase(AgentOwnerKey { agentEntry.second.callerUid, agentEntry.first });
    }
}

void AgentManagerService::HandleAgentHostConnectDone(const AgentHostKey &key, const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    std::vector<std::string> pendingAgentIds;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto sessionIter = agentHostSessions_.find(key);
        if (sessionIter == agentHostSessions_.end()) {
            return;
        }
        auto session = sessionIter->second;
        if (session == nullptr) {
            agentHostSessions_.erase(sessionIter);
            return;
        }
        session->element = element;
        session->remoteObject = remoteObject;
        session->resultCode = resultCode;
        session->isConnected = (resultCode == ERR_OK && remoteObject != nullptr);
        for (const auto &callbackEntry : session->callerConnections) {
            callbacks.emplace_back(callbackEntry.second);
        }
        for (auto &[pendingAgentId, agentRecord] : session->agents) {
            if (!agentRecord.isPending) {
                continue;
            }
            pendingAgentIds.emplace_back(pendingAgentId);
            agentRecord.isPending = false;
        }
        if (!session->isConnected) {
            ClearAgentHostSessionLocked(key);
        }
    }
    AgentHostSession callbackSession;
    callbackSession.remoteObject = remoteObject;
    if (resultCode == ERR_OK && remoteObject != nullptr) {
        for (const auto &agentId : pendingAgentIds) {
            NotifyAgentInvokedLocked(callbackSession, agentId);
        }
    }
    for (const auto &callback : callbacks) {
        if (callback != nullptr) {
            callback->OnAbilityConnectDone(element, remoteObject, resultCode);
            HandleConnectionDone(callback, resultCode, false);
        }
    }
}

void AgentManagerService::HandleAgentHostDisconnectDone(const AgentHostKey &key,
    const AppExecFwk::ElementName &element, int resultCode)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto sessionIter = agentHostSessions_.find(key);
        if (sessionIter == agentHostSessions_.end()) {
            return;
        }
        if (sessionIter->second == nullptr) {
            agentHostSessions_.erase(sessionIter);
            return;
        }
        for (const auto &callbackEntry : sessionIter->second->callerConnections) {
            callbacks.emplace_back(callbackEntry.second);
        }
        ClearAgentHostSessionLocked(key);
    }
    for (const auto &callback : callbacks) {
        if (callback != nullptr) {
            callback->OnAbilityDisconnectDone(element, resultCode);
            HandleConnectionDone(callback, resultCode, true);
        }
    }
}

void AgentManagerService::ClearAgentHostSessionLocked(const AgentHostKey &key)
{
    auto sessionIter = agentHostSessions_.find(key);
    if (sessionIter == agentHostSessions_.end()) {
        return;
    }
    if (sessionIter->second != nullptr) {
        EraseAgentOwnersLocked(*sessionIter->second);
    }
    agentHostSessions_.erase(sessionIter);
}
}  // namespace AgentRuntime
}  // namespace OHOS
