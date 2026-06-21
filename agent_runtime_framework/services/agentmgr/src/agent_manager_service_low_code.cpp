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
#include <vector>

#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "agent_manager_caller_identity.h"
#include "agent_extension_connection_constants.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "permission_constants.h"
#include "permission_verification.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr int32_t BASE_USER_RANGE = 200000;
}

int32_t AgentManagerService::DisconnectLowCodeTrackedConnection(
    const sptr<IRemoteObject> &callerRemote, const AgentHostKey &hostKey,
    const std::set<std::string> &agentIds)
{
    LowCodeDisconnectRequest request;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto ret = PrepareLowCodeDisconnectLocked(callerRemote, hostKey, agentIds, request);
        if (ret != ERR_OK || request.alreadyDisconnecting) {
            return ret;
        }
    }

    if (request.notifyCallerDisconnect && request.callerConnection != nullptr) {
        request.callerConnection->OnAbilityDisconnectDone(request.element, ERR_OK);
    }
    if (request.targets.empty()) {
        return ERR_OK;
    }
    int32_t result = ERR_OK;
    for (const auto &target : request.targets) {
        if (target.hostConnection == nullptr) {
            continue;
        }
        auto ret = RequestLowCodeHostDisconnect(hostKey, target.hostConnection, target.agentId);
        if (ret != ERR_OK && result == ERR_OK) {
            result = ret;
        }
    }
    if (result != ERR_OK) {
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto trackedIter = trackedConnections_.find(callerRemote);
        if (trackedIter != trackedConnections_.end()) {
            trackedIter->second.isDisconnecting = false;
        }
    }
    return result;
}

int32_t AgentManagerService::PrepareLowCodeDisconnectLocked(
    const sptr<IRemoteObject> &callerRemote, const AgentHostKey &hostKey, const std::set<std::string> &agentIds,
    LowCodeDisconnectRequest &request)
{
    auto trackedIter = trackedConnections_.find(callerRemote);
    if (trackedIter == trackedConnections_.end() || !trackedIter->second.isLowCode) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Low-code connection not tracked");
        return ERR_INVALID_VALUE;
    }
    if (trackedIter->second.isDisconnecting) {
        request.alreadyDisconnecting = true;
        return ERR_OK;
    }
    auto sessionIter = agentHostSessions_.find(hostKey);
    if (sessionIter == agentHostSessions_.end() || sessionIter->second == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Low-code host session missing");
        return ERR_INVALID_VALUE;
    }
    auto session = sessionIter->second;
    request.element = session->element;
    auto callerIter = session->callerConnections.find(callerRemote);
    if (callerIter != session->callerConnections.end()) {
        request.callerConnection = callerIter->second;
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER,
        "Low-code disconnect: ownedAgentCount=%{public}zu, activeAgentCount=%{public}zu",
        agentIds.size(), session->agents.size());
    std::set<std::string> targetAgentIds = agentIds;
    if (targetAgentIds.empty()) {
        for (const auto &agentEntry : session->agents) {
            if (agentEntry.second.callerRemote == callerRemote) {
                targetAgentIds.insert(agentEntry.first);
            }
        }
    }
    bool hasDisconnectingTarget = false;
    for (const auto &agentId : targetAgentIds) {
        auto agentIter = session->agents.find(agentId);
        if (agentIter == session->agents.end()) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "owned low-code agent not active: %{public}s", agentId.c_str());
            RemoveLowCodeAgentOwnershipLocked(callerRemote, agentId);
            continue;
        }
        if (agentIter->second.callerRemote != callerRemote) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code agent owner remote changed: %{public}s", agentId.c_str());
        }
        if (agentIter->second.isDisconnecting) {
            hasDisconnectingTarget = true;
            continue;
        }
        if (agentIter->second.hostConnection == nullptr) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code agent host connection missing: %{public}s", agentId.c_str());
            continue;
        }
        agentIter->second.isDisconnecting = true;
        request.targets.emplace_back(LowCodeDisconnectTarget { agentIter->first, agentIter->second.hostConnection });
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER,
        "Low-code disconnect removedAgentCount=%{public}zu, remainingAgentCount=%{public}zu",
        request.targets.size(), session->agents.size());
    if (!request.targets.empty()) {
        trackedIter->second.isDisconnecting = true;
        return ERR_OK;
    }
    if (hasDisconnectingTarget) {
        request.alreadyDisconnecting = true;
        return ERR_OK;
    }
    session->callerConnections.erase(callerRemote);
    ReleaseTrackedConnectionByRemoteLocked(callerRemote);
    request.notifyCallerDisconnect = true;
    TAG_LOGI(AAFwkTag::SER_ROUTER, "Low-code disconnect releases proxy only");
    return ERR_OK;
}

void AgentManagerService::RemoveLowCodeAgentLocked(AgentHostSession &session, const std::string &agentId,
    const LowCodeAgentRecord &record)
{
    ReleaseLowCodeAgentQuotaLocked(session, agentId, record.callerUid, BuildLowCodeQuotaKey(session.key));
    agentOwners_.erase(AgentOwnerKey { record.callerUid, agentId });
}

void AgentManagerService::AddLowCodeAgentOwnershipLocked(
    const sptr<IRemoteObject> &callerRemote, const std::string &agentId)
{
    auto trackedIter = trackedConnections_.find(callerRemote);
    if (trackedIter == trackedConnections_.end() || !trackedIter->second.isLowCode) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code owner connection missing");
        return;
    }
    trackedIter->second.lowCodeAgentIds.insert(agentId);
}

void AgentManagerService::RemoveLowCodeAgentOwnershipLocked(
    const sptr<IRemoteObject> &callerRemote, const std::string &agentId)
{
    auto trackedIter = trackedConnections_.find(callerRemote);
    if (trackedIter == trackedConnections_.end() || !trackedIter->second.isLowCode) {
        return;
    }
    trackedIter->second.lowCodeAgentIds.erase(agentId);
}

int32_t AgentManagerService::RequestLowCodeHostDisconnect(
    const AgentHostKey &hostKey, const sptr<AgentHostConnection> &hostConnection, const std::string &agentId)
{
    AgentManagerCallerIdentityScope identityScope;
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(hostConnection);
    if (ret == ERR_OK) {
        return ERR_OK;
    }
    std::lock_guard<std::mutex> lock(agentHostMutex_);
    auto sessionIter = agentHostSessions_.find(hostKey);
    if (sessionIter != agentHostSessions_.end() && sessionIter->second != nullptr) {
        auto &session = sessionIter->second;
        auto agentIter = session->agents.find(agentId);
        if (agentIter != session->agents.end()) {
            agentIter->second.isDisconnecting = false;
        }
    }
    TAG_LOGE(AAFwkTag::SER_ROUTER, "DisconnectAbility failed: %{public}d", ret);
    return ret;
}

int32_t AgentManagerService::NotifyLowCodeAgentComplete(const std::string &agentId)
{
    int32_t callingUid = 0;
    auto ret = ValidateNotifyLowCodeAgentCompleteRequest(agentId, callingUid);
    if (ret != ERR_OK) {
        return ret;
    }

    LowCodeCompleteRequest request;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        ret = PrepareLowCodeCompleteLocked(agentId, callingUid, request);
    }
    if (ret != ERR_OK || request.hostConnection == nullptr) {
        return ret;
    }
    return RequestLowCodeHostDisconnect(request.hostKey, request.hostConnection, request.agentId);
}

int32_t AgentManagerService::ValidateNotifyLowCodeAgentCompleteRequest(
    const std::string &agentId, int32_t &callingUid) const
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
    callingUid = IPCSkeleton::GetCallingUid();
    return ERR_OK;
}

int32_t AgentManagerService::PrepareLowCodeCompleteLocked(
    const std::string &agentId, int32_t callingUid, LowCodeCompleteRequest &request)
{
    AgentOwnerKey ownerKey { callingUid, agentId };
    auto ownerIter = agentOwners_.find(ownerKey);
    if (ownerIter == agentOwners_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code agent not found");
        return AAFwk::ERR_INVALID_AGENT_CARD_ID;
    }
    auto session = ownerIter->second;
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

    if (agentIter->second.isDisconnecting) {
        return ERR_OK;
    }
    agentIter->second.isDisconnecting = true;
    request.hostKey = session->key;
    request.agentId = agentId;
    request.hostConnection = agentIter->second.hostConnection;
    return ERR_OK;
}

bool AgentManagerService::HasOtherAgentForCallerLocked(const AgentHostSession &session,
    const std::string &agentId, const sptr<IRemoteObject> &callerRemote) const
{
    for (const auto &entry : session.agents) {
        if (entry.first != agentId && entry.second.callerRemote == callerRemote) {
            return true;
        }
    }
    return false;
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
        ret = AdmitExistingLowCodeAgent(agentId, plan);
        if (ret != ERR_OK) {
            CleanupLowCodeConnectPlan(plan, agentId);
            return ret;
        }
        if (plan.admittedNewAgent) {
            ret = RecordAdmittedLowCodeAgentLocked(plan, agentId, true);
            if (ret != ERR_OK) {
                CleanupLowCodeConnectPlan(plan, agentId);
                return ret;
            }
            return CompleteAgentHostConnect(want, agentId, plan);
        }
        if (plan.notifyExistingConnection) {
            NotifyExistingLowCodeConnection(plan, connection);
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
    plan.agentId = agentId;
    std::scoped_lock lock(connectionLock_, agentHostMutex_);

    auto callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection remote object is null");
        return ERR_INVALID_VALUE;
    }
    plan.callerRemote = callerRemote;
    plan.hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey, callerRemote, agentId);
    if (plan.hostConnection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Create low-code agent connection failed");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<AgentHostSession> session;
    auto ret = GetOrCreateLowCodeHostSessionLocked(hostKey, hostUid, plan, session);
    if (ret != ERR_OK) {
        return ret;
    }
    ret = RegisterLowCodeConnectionLocked(session, connection, agentId, plan);
    if (ret != ERR_OK) {
        return ret;
    }
    FillLowCodeConnectPlanCacheLocked(session, plan);
    return ERR_OK;
}

int32_t AgentManagerService::GetOrCreateLowCodeHostSessionLocked(const AgentHostKey &hostKey, int32_t hostUid,
    AgentConnectPlan &plan, std::shared_ptr<AgentHostSession> &session)
{
    auto sessionIter = agentHostSessions_.find(hostKey);
    if (sessionIter != agentHostSessions_.end()) {
        session = sessionIter->second;
        if (session == nullptr) {
            agentHostSessions_.erase(sessionIter);
        }
    }
    if (session != nullptr) {
        plan.reusedHostSession = true;
        return ERR_OK;
    }

    session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = hostUid;
    session->hostConnection = plan.hostConnection;
    if (session->hostConnection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Create host connection failed");
        return ERR_INVALID_VALUE;
    }
    agentHostSessions_[hostKey] = session;
    plan.needRealConnect = true;
    return ERR_OK;
}

int32_t AgentManagerService::RegisterLowCodeConnectionLocked(std::shared_ptr<AgentHostSession> &session,
    const sptr<AAFwk::IAbilityConnection> &connection, const std::string &agentId, AgentConnectPlan &plan)
{
    auto trackedIter = trackedConnections_.find(plan.callerRemote);
    if (trackedIter != trackedConnections_.end()) {
        if (!trackedIter->second.isLowCode || trackedIter->second.isDisconnecting ||
            !AgentHostKeyEqual()(trackedIter->second.hostKey, plan.hostKey)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection already tracked by incompatible agent connection");
            return ERR_INVALID_VALUE;
        }
        session->callerConnections[plan.callerRemote] = connection;
        plan.reusedCallerConnection = true;
        return ERR_OK;
    }

    auto ret = TryRegisterConnectionLocked(connection, plan.callerUid, plan.hostConnection, &plan.hostKey);
    if (ret != ERR_OK) {
        if (plan.needRealConnect) {
            agentHostSessions_.erase(plan.hostKey);
        }
        return ret;
    }
    plan.registeredTrackedConnection = true;
    session->callerConnections[plan.callerRemote] = connection;
    if (plan.reusedHostSession) {
        return ERR_OK;
    }
    return AdmitInitialLowCodeAgentLocked(session, agentId, plan);
}

int32_t AgentManagerService::AdmitInitialLowCodeAgentLocked(std::shared_ptr<AgentHostSession> &session,
    const std::string &agentId, AgentConnectPlan &plan)
{
    auto ret = AdmitLowCodeAgentLocked(plan.callerUid, BuildLowCodeQuotaKey(plan.hostKey), *session, agentId);
    if (ret != ERR_OK) {
        if (plan.needRealConnect) {
            agentHostSessions_.erase(plan.hostKey);
        }
        ReleaseTrackedConnectionByRemoteLocked(plan.callerRemote);
        return ret == AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE ? ERR_OK : ret;
    }
    session->agents[agentId] = LowCodeAgentRecord {
        plan.callerRemote, plan.callerUid, !session->isConnected, plan.hostConnection
    };
    AddLowCodeAgentOwnershipLocked(plan.callerRemote, agentId);
    agentOwners_[AgentOwnerKey { plan.callerUid, agentId }] = session;
    return ERR_OK;
}

void AgentManagerService::FillLowCodeConnectPlanCacheLocked(
    const std::shared_ptr<AgentHostSession> &session, AgentConnectPlan &plan)
{
    plan.notifyExistingConnection = session->isConnected && session->remoteObject != nullptr;
    if (!plan.notifyExistingConnection) {
        return;
    }
    plan.cachedElement = session->element;
    plan.cachedRemoteObject = session->remoteObject;
    plan.cachedResultCode = session->resultCode;
}

void AgentManagerService::NotifyExistingLowCodeConnection(const AgentConnectPlan &plan,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    if (plan.reusedCallerConnection) {
        return;
    }
    connection->OnAbilityConnectDone(plan.cachedElement, plan.cachedRemoteObject, plan.cachedResultCode);
}

int32_t AgentManagerService::AdmitExistingLowCodeAgent(const std::string &agentId, AgentConnectPlan &plan)
{
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
    session->agents[agentId] = LowCodeAgentRecord {
        plan.callerRemote, plan.callerUid, isPending, plan.hostConnection
    };
    AddLowCodeAgentOwnershipLocked(plan.callerRemote, agentId);
    agentOwners_[AgentOwnerKey { plan.callerUid, agentId }] = session;
    return ERR_OK;
}

void AgentManagerService::CleanupLowCodeConnectPlan(const AgentConnectPlan &plan, const std::string &agentId)
{
    std::scoped_lock lock(connectionLock_, agentHostMutex_);
    agentOwners_.erase(AgentOwnerKey { plan.callerUid, agentId });
    bool shouldReleaseTrackedConnection = plan.registeredTrackedConnection;
    auto sessionIter = agentHostSessions_.find(plan.hostKey);
    if (sessionIter != agentHostSessions_.end() && sessionIter->second != nullptr) {
        auto session = sessionIter->second;
        auto agentIter = session->agents.find(agentId);
        if (agentIter != session->agents.end()) {
            ReleaseLowCodeAgentQuotaLocked(*session, agentId, agentIter->second.callerUid,
                BuildLowCodeQuotaKey(plan.hostKey));
        }
        if (plan.callerRemote != nullptr) {
            RemoveLowCodeAgentOwnershipLocked(plan.callerRemote, agentId);
        }
        session->agents.erase(agentId);
        bool callerHasOtherAgent = HasOtherAgentForCallerLocked(*session, agentId, plan.callerRemote);
        if (plan.callerRemote != nullptr && !callerHasOtherAgent) {
            session->callerConnections.erase(plan.callerRemote);
        } else {
            shouldReleaseTrackedConnection = false;
        }
        if (session->callerConnections.empty() && session->agents.empty()) {
            agentHostSessions_.erase(sessionIter);
        }
    }
    if (shouldReleaseTrackedConnection && plan.callerRemote != nullptr) {
        ReleaseTrackedConnectionByRemoteLocked(plan.callerRemote);
    }
}

int32_t AgentManagerService::CompleteAgentHostConnect(const AAFwk::Want &want, const std::string &agentId,
    AgentConnectPlan &plan)
{
    AAFwk::Want connectWant = want;
    long verificationNonce = GenerateAgentVerificationNonce();
    connectWant.SetParam(AGENT_VERIFICATION_NONCE_KEY, verificationNonce);
    AgentManagerCallerIdentityScope identityScope;
    {
        std::scoped_lock lock(connectionLock_, agentHostMutex_);
        auto sessionIter = agentHostSessions_.find(plan.hostKey);
        if (sessionIter != agentHostSessions_.end() && sessionIter->second != nullptr) {
            auto &session = sessionIter->second;
            auto agentIter = session->agents.find(agentId);
            if (agentIter != session->agents.end()) {
                agentIter->second.originalIdentity = identityScope.GetOriginalIdentity();
                agentIter->second.verificationNonce = verificationNonce;
            }
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

void AgentManagerService::EraseAgentOwnersLocked(const AgentHostSession &session)
{
    for (const auto &agentEntry : session.agents) {
        agentOwners_.erase(AgentOwnerKey { agentEntry.second.callerUid, agentEntry.first });
    }
}

void AgentManagerService::ReleaseLowCodeHostQuotasLocked(const AgentHostSession &session)
{
    std::vector<int32_t> releasedCallerUids;
    auto quotaKey = BuildLowCodeQuotaKey(session.key);
    for (const auto &agentEntry : session.agents) {
        int32_t callerUid = agentEntry.second.callerUid;
        if (std::find(releasedCallerUids.begin(), releasedCallerUids.end(), callerUid) !=
            releasedCallerUids.end()) {
            continue;
        }
        releasedCallerUids.emplace_back(callerUid);
        ReleaseCallerQuotaLocked(callerUid, quotaKey);
    }
}

void AgentManagerService::HandleAgentHostConnectDone(const AgentHostKey &key,
    const sptr<IRemoteObject> &callerRemote, const std::string &agentId,
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    sptr<AAFwk::IAbilityConnection> callback;
    bool releaseConnectionOnFailure = false;
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
        if (resultCode == ERR_OK && remoteObject != nullptr) {
            session->remoteObject = remoteObject;
            session->resultCode = resultCode;
            session->isConnected = true;
        }
        auto callerIter = session->callerConnections.find(callerRemote);
        if (callerIter != session->callerConnections.end()) {
            callback = callerIter->second;
        }
        auto agentIter = session->agents.find(agentId);
        if (agentIter == session->agents.end()) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code connect done for unknown agent");
            return;
        }
        agentIter->second.isPending = false;
        if (resultCode != ERR_OK || remoteObject == nullptr) {
            auto record = agentIter->second;
            RemoveLowCodeAgentLocked(*session, agentId, record);
            RemoveLowCodeAgentOwnershipLocked(record.callerRemote, agentId);
            session->agents.erase(agentIter);
            releaseConnectionOnFailure = !HasOtherAgentForCallerLocked(*session, agentId, record.callerRemote);
            if (releaseConnectionOnFailure && record.callerRemote != nullptr) {
                session->callerConnections.erase(record.callerRemote);
                ReleaseTrackedConnectionByRemoteLocked(record.callerRemote);
            }
            if (session->agents.empty() && session->callerConnections.empty()) {
                agentHostSessions_.erase(sessionIter);
            }
        }
    }
    if (callback != nullptr) {
        callback->OnAbilityConnectDone(element, remoteObject, resultCode);
        if (releaseConnectionOnFailure) {
            HandleConnectionDone(callback, resultCode, false);
        }
    }
}

void AgentManagerService::HandleAgentHostDisconnectDone(const AgentHostKey &key,
    const sptr<IRemoteObject> &callerRemote, const std::string &agentId,
    const AppExecFwk::ElementName &element, int resultCode)
{
    sptr<AAFwk::IAbilityConnection> callback;
    bool releaseConnection = false;
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
        auto callerIter = session->callerConnections.find(callerRemote);
        if (callerIter != session->callerConnections.end()) {
            callback = callerIter->second;
        }
        auto agentIter = session->agents.find(agentId);
        if (agentIter == session->agents.end()) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code disconnect done for unknown agent");
            return;
        }
        auto record = agentIter->second;
        RemoveLowCodeAgentLocked(*session, agentId, record);
        RemoveLowCodeAgentOwnershipLocked(record.callerRemote, agentId);
        session->agents.erase(agentIter);
        releaseConnection = !HasOtherAgentForCallerLocked(*session, agentId, record.callerRemote);
        if (releaseConnection && record.callerRemote != nullptr) {
            session->callerConnections.erase(record.callerRemote);
            ReleaseTrackedConnectionByRemoteLocked(record.callerRemote);
        }
        if (session->agents.empty()) {
            agentHostSessions_.erase(sessionIter);
        }
    }
    if (callback != nullptr) {
        callback->OnAbilityDisconnectDone(element, resultCode);
        if (releaseConnection) {
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
        auto session = sessionIter->second;
        ReleaseLowCodeHostQuotasLocked(*session);
        EraseAgentOwnersLocked(*session);
        for (const auto &callerEntry : session->callerConnections) {
            ReleaseTrackedConnectionByRemoteLocked(callerEntry.first);
        }
    }
    agentHostSessions_.erase(sessionIter);
}
}  // namespace AgentRuntime
}  // namespace OHOS
