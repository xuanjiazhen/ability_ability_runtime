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

#include "agent_connect_manager.h"

#include <algorithm>
#include <chrono>
#include <vector>

#include "ability_connect_callback_stub.h"
#include "ability_manager_errors.h"
#include "agent_extension_connection_constants.h"
#include "agent_service_connection.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AgentRuntime {
using namespace std::chrono;

namespace {
constexpr size_t MAX_AGENT_CONNECTIONS_PER_CALLER = 5;
constexpr size_t MAX_LOW_CODE_AGENTS_PER_HOST = 100;
constexpr size_t MAX_AGENT_CONNECT_PREFLIGHTS = 1024;
constexpr auto AGENT_CONNECT_PREFLIGHT_TIMEOUT = minutes(1);

bool IsConnectPreflightTargetMatched(const AAFwk::Want &want, const AgentConnectPreflightRecord &record)
{
    return want.GetStringParam(AGENTID_KEY) == record.agentId &&
        want.GetElement() == record.connectWant.GetElement() &&
        want.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0) ==
            record.connectWant.GetIntParam(AAFwk::Want::PARAM_APP_CLONE_INDEX_KEY, 0);
}

sptr<AgentHostConnection> FindHostConnectionForCaller(
    const AgentHostSession &session, const sptr<IRemoteObject> &callerRemote)
{
    for (const auto &agentEntry : session.agents) {
        if (agentEntry.second.callerRemote == callerRemote && agentEntry.second.hostConnection != nullptr) {
            return agentEntry.second.hostConnection;
        }
    }
    return nullptr;
}

void AddLowCodeDisconnectTarget(std::vector<LowCodeDisconnectTarget> &targets,
    const sptr<AgentHostConnection> &hostConnection, const std::string &agentId)
{
    if (hostConnection == nullptr || agentId.empty()) {
        return;
    }
    auto hostRemote = hostConnection->AsObject();
    auto iter = std::find_if(targets.begin(), targets.end(),
        [hostRemote](const LowCodeDisconnectTarget &target) {
            return target.hostConnection != nullptr && target.hostConnection->AsObject() == hostRemote;
        });
    if (iter == targets.end()) {
        targets.emplace_back(LowCodeDisconnectTarget { { agentId }, hostConnection });
        return;
    }
    iter->agentIds.insert(agentId);
}
}

AgentConnectManager &AgentConnectManager::GetInstance()
{
    static AgentConnectManager manager;
    return manager;
}

void AgentConnectManager::Clear()
{
    std::lock_guard<std::mutex> lock(stateLock_);
    trackedConnections_.clear();
    connectPreflights_.clear();
    connectPreflightCleanupScheduled_ = false;
    callerQuotas_.clear();
    agentHostSessions_.clear();
    agentOwners_.clear();
}

AgentConnectPreflightRegisterResult AgentConnectManager::RegisterConnectPreflight(
    const AgentConnectPreflightRegisterRequest &request, const GenerateNonceFunc &generateNonce)
{
    AgentConnectPreflightRegisterResult result;
    result.connectWant = request.connectWant;
    if (!generateNonce) {
        return result;
    }

    long nonce = generateNonce();
    auto now = AgentPreflightClock::now();
    std::lock_guard<std::mutex> lock(stateLock_);
    PruneExpiredConnectPreflightsLocked(now);
    while (connectPreflights_.find(nonce) != connectPreflights_.end()) {
        nonce = generateNonce();
    }
    result.connectWant.SetParam(AGENT_VERIFICATION_NONCE_KEY, nonce);

    AgentConnectPreflightRecord record;
    record.callerUid = request.callerUid;
    record.callerUserId = request.callerUserId;
    record.connectWant = result.connectWant;
    record.agentId = request.agentId;
    record.card = request.card;
    record.expiresAt = now + AGENT_CONNECT_PREFLIGHT_TIMEOUT;
    if (connectPreflights_.size() >= MAX_AGENT_CONNECT_PREFLIGHTS) {
        EraseOldestConnectPreflightLocked();
    }
    connectPreflights_[nonce] = record;
    result.nonce = nonce;
    result.needSchedule = MarkConnectPreflightCleanupLocked(record.expiresAt, result.cleanupAt);
    return result;
}

AgentConnectPreflightConsumeResult AgentConnectManager::TryConsumeConnectPreflight(
    const AgentConnectPreflightConsumeRequest &request)
{
    AgentConnectPreflightConsumeResult result;
    long nonce = request.want.GetLongParam(AGENT_VERIFICATION_NONCE_KEY, 0);
    if (nonce <= 0) {
        return result;
    }
    std::lock_guard<std::mutex> lock(stateLock_);
    PruneExpiredConnectPreflightsLocked(AgentPreflightClock::now());
    auto iter = connectPreflights_.find(nonce);
    if (iter == connectPreflights_.end()) {
        return result;
    }
    if (iter->second.callerUid != request.callerUid) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "connect preflight caller mismatch");
        connectPreflights_.erase(iter);
        return result;
    }
    if (iter->second.callerUserId != request.callerUserId) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "connect preflight user mismatch");
        connectPreflights_.erase(iter);
        return result;
    }
    if (!IsConnectPreflightTargetMatched(request.want, iter->second)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "connect preflight target mismatch");
        connectPreflights_.erase(iter);
        return result;
    }
    result.matched = true;
    result.connectWant = iter->second.connectWant;
    result.agentId = iter->second.agentId;
    result.card = iter->second.card;
    connectPreflights_.erase(iter);
    return result;
}

bool AgentConnectManager::CleanupExpiredConnectPreflights(
    AgentPreflightTimePoint scheduledAt, AgentPreflightTimePoint &nextAt)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    if (!connectPreflightCleanupScheduled_ || connectPreflightCleanupAt_ != scheduledAt) {
        return false;
    }
    connectPreflightCleanupScheduled_ = false;
    PruneExpiredConnectPreflightsLocked(AgentPreflightClock::now());
    if (connectPreflights_.empty()) {
        return false;
    }
    return MarkConnectPreflightCleanupLocked(GetEarliestConnectPreflightExpiryLocked(), nextAt);
}

void AgentConnectManager::PruneExpiredConnectPreflightsLocked(AgentPreflightTimePoint now)
{
    for (auto iter = connectPreflights_.begin(); iter != connectPreflights_.end();) {
        if (iter->second.expiresAt > now) {
            ++iter;
            continue;
        }
        iter = connectPreflights_.erase(iter);
    }
}

AgentPreflightTimePoint AgentConnectManager::GetEarliestConnectPreflightExpiryLocked() const
{
    auto earliest = std::min_element(connectPreflights_.begin(), connectPreflights_.end(),
        [](const auto &left, const auto &right) {
            return left.second.expiresAt < right.second.expiresAt;
        });
    return earliest == connectPreflights_.end() ? AgentPreflightTimePoint::max() : earliest->second.expiresAt;
}

void AgentConnectManager::EraseOldestConnectPreflightLocked()
{
    auto oldestExpiry = GetEarliestConnectPreflightExpiryLocked();
    auto oldest = std::find_if(connectPreflights_.begin(), connectPreflights_.end(),
        [oldestExpiry](const auto &item) {
            return item.second.expiresAt == oldestExpiry;
        });
    if (oldest != connectPreflights_.end()) {
        connectPreflights_.erase(oldest);
    }
}

bool AgentConnectManager::MarkConnectPreflightCleanupLocked(AgentPreflightTimePoint expiresAt,
    AgentPreflightTimePoint &cleanupAt)
{
    if (connectPreflightCleanupScheduled_ && connectPreflightCleanupAt_ <= expiresAt) {
        return false;
    }
    connectPreflightCleanupScheduled_ = true;
    connectPreflightCleanupAt_ = expiresAt;
    cleanupAt = expiresAt;
    return true;
}

int32_t AgentConnectManager::AdmitStandardAgentQuota(int32_t callerUid, const AgentQuotaKey &quotaKey)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    return AdmitStandardAgentConnectionLocked(callerUid, quotaKey);
}

void AgentConnectManager::ReleaseCallerQuota(int32_t callerUid, const AgentQuotaKey &quotaKey)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    ReleaseCallerQuotaLocked(callerUid, quotaKey);
}

int32_t AgentConnectManager::AdmitStandardAgentConnectionLocked(int32_t callerUid, const AgentQuotaKey &quotaKey)
{
    auto &callerQuota = callerQuotas_[callerUid];
    if (callerQuota.find(quotaKey) == callerQuota.end() &&
        callerQuota.size() >= MAX_AGENT_CONNECTIONS_PER_CALLER) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT caller quota reached");
        return AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED;
    }
    callerQuota[quotaKey]++;
    return ERR_OK;
}

int32_t AgentConnectManager::AdmitLowCodeAgentLocked(int32_t callerUid, const AgentQuotaKey &quotaKey,
    const AgentHostSession &session, const std::string &agentId)
{
    if (session.agents.find(agentId) != session.agents.end()) {
        return AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE;
    }
    if (session.agents.size() >= MAX_LOW_CODE_AGENTS_PER_HOST) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code AGENT host quota reached");
        return AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED;
    }
    bool callerAlreadyOwnsHost = std::any_of(session.agents.begin(), session.agents.end(),
        [callerUid](const auto &entry) {
            return entry.second.callerUid == callerUid;
        });
    if (callerAlreadyOwnsHost) {
        return ERR_OK;
    }
    auto &callerQuota = callerQuotas_[callerUid];
    if (callerQuota.find(quotaKey) == callerQuota.end() &&
        callerQuota.size() >= MAX_AGENT_CONNECTIONS_PER_CALLER) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT caller quota reached");
        return AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED;
    }
    callerQuota[quotaKey] = 1;
    return ERR_OK;
}

void AgentConnectManager::ReleaseCallerQuotaLocked(int32_t callerUid, const AgentQuotaKey &quotaKey)
{
    auto callerIter = callerQuotas_.find(callerUid);
    if (callerIter == callerQuotas_.end()) {
        return;
    }
    auto quotaIter = callerIter->second.find(quotaKey);
    if (quotaIter == callerIter->second.end()) {
        return;
    }
    if (quotaIter->second > 1) {
        quotaIter->second--;
        return;
    }
    callerIter->second.erase(quotaIter);
    if (callerIter->second.empty()) {
        callerQuotas_.erase(callerIter);
    }
}

void AgentConnectManager::ReleaseTrackedConnectionQuotaLocked(TrackedConnectionRecord &record)
{
    if (!record.hasQuota || record.isLowCode) {
        return;
    }
    AgentQuotaKey quotaKey;
    quotaKey.hostKey = record.hostKey;
    quotaKey.agentId = record.agentId;
    quotaKey.isLowCode = false;
    ReleaseCallerQuotaLocked(record.callerUid, quotaKey);
    record.hasQuota = false;
}

void AgentConnectManager::ReleaseLowCodeAgentQuotaLocked(AgentHostSession &session, const std::string &agentId,
    int32_t callerUid, const AgentQuotaKey &quotaKey)
{
    bool callerStillOwnsHost = false;
    for (const auto &entry : session.agents) {
        if (entry.first == agentId) {
            continue;
        }
        if (entry.second.callerUid == callerUid) {
            callerStillOwnsHost = true;
            break;
        }
    }
    if (!callerStillOwnsHost) {
        ReleaseCallerQuotaLocked(callerUid, quotaKey);
    }
}

void AgentConnectManager::ReleaseLowCodeHostQuotasLocked(const AgentHostSession &session)
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

AgentQuotaKey AgentConnectManager::BuildLowCodeQuotaKey(const AgentHostKey &hostKey) const
{
    AgentQuotaKey key;
    key.hostKey = hostKey;
    key.isLowCode = true;
    return key;
}

int32_t AgentConnectManager::RegisterTrackedConnectionAndGetServiceConnection(
    const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid,
    const CallerDeathHandler &deathHandler, sptr<AAFwk::IAbilityConnection> &serviceConnection)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    auto ret = TryRegisterConnectionLocked(connection, callerUid, deathHandler);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "register tracked connection failed: %{public}d", ret);
        return ret;
    }
    auto it = trackedConnections_.find(GetConnectionIdentityRemote(connection));
    if (it == trackedConnections_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "tracked connection missing after register");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    serviceConnection = it->second.serviceConnection;
    return ERR_OK;
}

int32_t AgentConnectManager::SetTrackedConnectionIdentityAndQuota(
    const sptr<AAFwk::IAbilityConnection> &connection, const std::string &agentId,
    const std::string &originalIdentity, const AgentQuotaKey &quotaKey, long verificationNonce)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    auto it = trackedConnections_.find(GetConnectionIdentityRemote(connection));
    if (it == trackedConnections_.end()) {
        return AAFwk::CONNECTION_NOT_EXIST;
    }
    it->second.agentId = agentId;
    it->second.hostKey = quotaKey.hostKey;
    it->second.originalIdentity = originalIdentity;
    it->second.verificationNonce = verificationNonce;
    it->second.hasQuota = true;
    return ERR_OK;
}

int32_t AgentConnectManager::PrepareAgentDisconnectRequest(
    const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid, AgentDisconnectRequest &request)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    auto it = FindTrackedConnectionLocked(connection, callerUid);
    if (it == trackedConnections_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection not tracked");
        return ERR_INVALID_VALUE;
    }
    request.callerRemote = it->first;
    if (it->second.isDisconnecting) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Connection is already disconnecting");
        request.alreadyDisconnecting = true;
        return ERR_OK;
    }
    if (it->second.isLowCode) {
        return PrepareLowCodeDisconnectRequestLocked(it, request);
    }
    it->second.isDisconnecting = true;
    request.serviceConnection = it->second.serviceConnection;
    return ERR_OK;
}

int32_t AgentConnectManager::PrepareLowCodeDisconnectRequestLocked(
    TrackedConnectionIter trackedIter, AgentDisconnectRequest &request)
{
    auto sessionIter = agentHostSessions_.find(trackedIter->second.hostKey);
    if (sessionIter == agentHostSessions_.end() || sessionIter->second == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Low-code host session missing");
        return ERR_INVALID_VALUE;
    }
    auto session = sessionIter->second;
    request.isLowCode = true;
    request.hostKey = trackedIter->second.hostKey;
    request.element = session->element;
    auto callerIter = session->callerConnections.find(trackedIter->first);
    if (callerIter != session->callerConnections.end()) {
        request.callerConnection = callerIter->second;
    }

    auto targetAgentIds = CollectLowCodeAgentIdsLocked(*session, trackedIter->first);
    bool hasDisconnectingTarget = false;
    for (const auto &agentId : targetAgentIds) {
        auto agentIter = session->agents.find(agentId);
        if (agentIter == session->agents.end() || agentIter->second.callerRemote != trackedIter->first) {
            continue;
        }
        if (agentIter->second.isDisconnecting) {
            hasDisconnectingTarget = true;
            continue;
        }
        if (agentIter->second.hostConnection == nullptr) {
            continue;
        }
        agentIter->second.isDisconnecting = true;
        AddLowCodeDisconnectTarget(request.lowCodeTargets, agentIter->second.hostConnection, agentIter->first);
    }
    if (!request.lowCodeTargets.empty()) {
        for (const auto &target : request.lowCodeTargets) {
            target.hostConnection->SetPendingDisconnectAgents(target.agentIds);
        }
        trackedIter->second.isDisconnecting = true;
        return ERR_OK;
    }
    if (hasDisconnectingTarget) {
        request.alreadyDisconnecting = true;
        return ERR_OK;
    }
    session->callerConnections.erase(trackedIter->first);
    ReleaseTrackedConnectionByRemoteLocked(trackedIter->first);
    request.notifyCallerDisconnect = true;
    return ERR_OK;
}

int32_t AgentConnectManager::PrepareServiceDisconnectRequest(
    const sptr<AAFwk::IAbilityConnection> &connection, AgentDisconnectRequest &request)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    auto remote = GetConnectionIdentityRemote(connection);
    auto it = trackedConnections_.find(remote);
    if (it == trackedConnections_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection not tracked");
        return ERR_INVALID_VALUE;
    }
    if (it->second.isDisconnecting) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "Connection is already disconnecting");
        request.alreadyDisconnecting = true;
        return ERR_OK;
    }
    it->second.isDisconnecting = true;
    request.callerRemote = remote;
    request.serviceConnection = it->second.serviceConnection;
    return ERR_OK;
}

void AgentConnectManager::RestoreStandardAgentDisconnectingState(const sptr<IRemoteObject> &callerRemote)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    auto it = trackedConnections_.find(callerRemote);
    if (it != trackedConnections_.end() && it->second.isDisconnecting) {
        it->second.isDisconnecting = false;
    }
}

void AgentConnectManager::RestoreConnectionDisconnectingState(const sptr<AAFwk::IAbilityConnection> &connection)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    auto it = trackedConnections_.find(GetConnectionIdentityRemote(connection));
    if (it != trackedConnections_.end() && it->second.isDisconnecting) {
        it->second.isDisconnecting = false;
    }
}

bool AgentConnectManager::IsAgentVerificationNonceMatched(const AAFwk::Want &want, long verificationNonce) const
{
    return verificationNonce > 0 && want.GetLongParam(AGENT_VERIFICATION_NONCE_KEY, 0) == verificationNonce;
}

int32_t AgentConnectManager::VerifyAgentConnectRequest(const sptr<IRemoteObject> &remote,
    const std::string &agentId, const AAFwk::Want &want, std::string &callerIdentity)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    auto ret = VerifyStandardAgentConnectRequestLocked(remote, agentId, want, callerIdentity);
    if (ret != AAFwk::CONNECTION_NOT_EXIST) {
        return ret;
    }
    ret = VerifyLowCodeAgentConnectRequestLocked(remote, agentId, want, callerIdentity);
    if (ret != AAFwk::CONNECTION_NOT_EXIST) {
        return ret;
    }
    TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT connect is not owned by AgentMgr");
    return AAFwk::CONNECTION_NOT_EXIST;
}

int32_t AgentConnectManager::VerifyStandardAgentConnectRequestLocked(const sptr<IRemoteObject> &remote,
    const std::string &agentId, const AAFwk::Want &want, std::string &callerIdentity) const
{
    for (const auto &entry : trackedConnections_) {
        const auto &record = entry.second;
        if (record.isLowCode || record.serviceConnection == nullptr ||
            record.serviceConnection->AsObject() != remote) {
            continue;
        }
        if (!agentId.empty() && !record.agentId.empty() && record.agentId != agentId) {
            return AAFwk::ERR_WRONG_INTERFACE_CALL;
        }
        if (!IsAgentVerificationNonceMatched(want, record.verificationNonce)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT connect nonce mismatch");
            return AAFwk::ERR_WRONG_INTERFACE_CALL;
        }
        if (record.originalIdentity.empty()) {
            return ERR_INVALID_VALUE;
        }
        callerIdentity = record.originalIdentity;
        return ERR_OK;
    }
    return AAFwk::CONNECTION_NOT_EXIST;
}

int32_t AgentConnectManager::VerifyLowCodeAgentConnectRequestLocked(const sptr<IRemoteObject> &remote,
    const std::string &agentId, const AAFwk::Want &want, std::string &callerIdentity) const
{
    for (const auto &entry : agentHostSessions_) {
        const auto &session = entry.second;
        if (session == nullptr) {
            continue;
        }
        for (const auto &agentEntry : session->agents) {
            const auto &record = agentEntry.second;
            if (record.hostConnection == nullptr || record.hostConnection->AsObject() != remote) {
                continue;
            }
            if (!agentId.empty() && agentEntry.first != agentId) {
                continue;
            }
            if (!IsAgentVerificationNonceMatched(want, record.verificationNonce)) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT host connect nonce mismatch");
                return AAFwk::ERR_WRONG_INTERFACE_CALL;
            }
            if (record.originalIdentity.empty()) {
                return ERR_INVALID_VALUE;
            }
            callerIdentity = record.originalIdentity;
            return ERR_OK;
        }
    }
    return AAFwk::CONNECTION_NOT_EXIST;
}

int32_t AgentConnectManager::VerifyAgentDisconnectRequests(const sptr<IRemoteObject> &remote,
    const std::vector<AAFwk::Want> &wants, std::string &callerIdentity)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    for (const auto &want : wants) {
        std::string verifiedCallerIdentity;
        auto ret = MatchStandardAgentDisconnectLocked(remote, want, verifiedCallerIdentity);
        if (ret == ERR_OK) {
            callerIdentity = verifiedCallerIdentity;
            return ERR_OK;
        }
    }
    if (MatchLowCodeAgentDisconnectLocked(remote, callerIdentity) == ERR_OK) {
        return ERR_OK;
    }
    TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT disconnect is not owned by AgentMgr");
    return AAFwk::CONNECTION_NOT_EXIST;
}

int32_t AgentConnectManager::MatchStandardAgentDisconnectLocked(const sptr<IRemoteObject> &remote,
    const AAFwk::Want &want, std::string &callerIdentity) const
{
    for (const auto &entry : trackedConnections_) {
        const auto &record = entry.second;
        if (record.isLowCode || record.serviceConnection == nullptr ||
            record.serviceConnection->AsObject() != remote) {
            continue;
        }
        if (record.originalIdentity.empty()) {
            return ERR_INVALID_VALUE;
        }
        if (!IsAgentVerificationNonceMatched(want, record.verificationNonce)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "AGENT disconnect nonce mismatch");
            return AAFwk::ERR_WRONG_INTERFACE_CALL;
        }
        callerIdentity = record.originalIdentity;
        return ERR_OK;
    }
    return AAFwk::CONNECTION_NOT_EXIST;
}

int32_t AgentConnectManager::MatchLowCodeAgentDisconnectLocked(
    const sptr<IRemoteObject> &remote, std::string &callerIdentity) const
{
    for (const auto &entry : agentHostSessions_) {
        const auto &session = entry.second;
        if (session == nullptr) {
            continue;
        }
        for (const auto &agentEntry : session->agents) {
            const auto &record = agentEntry.second;
            if (record.hostConnection == nullptr || record.hostConnection->AsObject() != remote) {
                continue;
            }
            if (!record.isDisconnecting || record.originalIdentity.empty()) {
                continue;
            }
            callerIdentity = record.originalIdentity;
            return ERR_OK;
        }
    }
    return AAFwk::CONNECTION_NOT_EXIST;
}

int32_t AgentConnectManager::TryRegisterConnectionLocked(const sptr<AAFwk::IAbilityConnection> &connection,
    int32_t callerUid, const CallerDeathHandler &deathHandler,
    const sptr<AAFwk::IAbilityConnection> &serviceConnection, const AgentHostKey *hostKey)
{
    auto callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection remote object is null");
        return ERR_INVALID_VALUE;
    }
    auto existing = trackedConnections_.find(callerRemote);
    if (existing != trackedConnections_.end()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection already tracked");
        return ERR_INVALID_VALUE;
    }

    auto actualServiceConnection = CreateServiceConnection(connection, serviceConnection);
    if (actualServiceConnection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Create service connection failed");
        return ERR_INVALID_VALUE;
    }

    TrackedConnectionRecord record;
    record.callerUid = callerUid;
    record.serviceConnection = actualServiceConnection;
    record.callerRemote = callerRemote;
    record.isLowCode = hostKey != nullptr;
    if (hostKey != nullptr) {
        record.hostKey = *hostKey;
    }
    AddCallerDeathRecipient(record, deathHandler);

    trackedConnections_.emplace(callerRemote, record);
    return ERR_OK;
}

AgentConnectManager::TrackedConnectionIter AgentConnectManager::FindTrackedConnectionLocked(
    const sptr<AAFwk::IAbilityConnection> &connection, int32_t callerUid)
{
    auto end = trackedConnections_.end();
    auto callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        return end;
    }
    auto it = trackedConnections_.find(callerRemote);
    if (it != end) {
        return it;
    }
    auto matched = end;
    for (auto iter = trackedConnections_.begin(); iter != end; ++iter) {
        if (iter->second.callerUid != callerUid || iter->second.isLowCode) {
            continue;
        }
        if (matched != end) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "Multiple tracked connections exist for callerUid: %{public}d", callerUid);
            return end;
        }
        matched = iter;
    }
    if (matched == end) {
        return end;
    }
    TAG_LOGW(AAFwkTag::SER_ROUTER, "Resolved tracked connection by callerUid fallback: %{public}d", callerUid);
    return matched;
}

sptr<AAFwk::IAbilityConnection> AgentConnectManager::CreateServiceConnection(
    const sptr<AAFwk::IAbilityConnection> &connection,
    const sptr<AAFwk::IAbilityConnection> &serviceConnection) const
{
    if (serviceConnection != nullptr) {
        return serviceConnection;
    }
    return sptr<AgentServiceConnection>::MakeSptr(connection);
}

void AgentConnectManager::AddCallerDeathRecipient(
    TrackedConnectionRecord &record, const CallerDeathHandler &deathHandler)
{
    if (record.callerRemote == nullptr) {
        return;
    }
    auto handler = [callerRemote = record.callerRemote, deathHandler](const wptr<IRemoteObject> &) {
        if (deathHandler) {
            deathHandler(callerRemote);
        }
    };
    record.deathRecipient = sptr<AAFwk::AbilityConnectCallbackRecipient>::MakeSptr(std::move(handler));
    if (record.deathRecipient != nullptr) {
        record.callerRemote->AddDeathRecipient(record.deathRecipient);
    }
}

void AgentConnectManager::ReleaseTrackedConnection(const sptr<AAFwk::IAbilityConnection> &connection)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    ReleaseTrackedConnectionByRemoteLocked(GetConnectionIdentityRemote(connection));
}

void AgentConnectManager::ReleaseTrackedConnectionByRemote(const sptr<IRemoteObject> &callerRemote)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    ReleaseTrackedConnectionByRemoteLocked(callerRemote);
}

void AgentConnectManager::ReleaseTrackedConnectionByRemoteLocked(const sptr<IRemoteObject> &callerRemote)
{
    auto it = trackedConnections_.find(callerRemote);
    if (it == trackedConnections_.end()) {
        return;
    }
    if (it->second.callerRemote != nullptr && it->second.deathRecipient != nullptr) {
        it->second.callerRemote->RemoveDeathRecipient(it->second.deathRecipient);
    }
    ReleaseTrackedConnectionQuotaLocked(it->second);
    it->second.verificationNonce = 0;
    trackedConnections_.erase(it);
}

bool AgentConnectManager::PrepareCallerDeathRequest(const sptr<IRemoteObject> &remote,
    AgentCallerDeathRequest &request)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    if (remote == nullptr) {
        return false;
    }
    auto it = trackedConnections_.find(remote);
    if (it == trackedConnections_.end()) {
        return false;
    }
    if (!it->second.isLowCode) {
        request.serviceConnection = it->second.serviceConnection;
        it->second.isDisconnecting = true;
        return true;
    }
    request.hostKey = it->second.hostKey;
    PrepareLowCodeCallerDeathLocked(remote, it->second.hostKey, request);
    ReleaseTrackedConnectionByRemoteLocked(remote);
    return !request.lowCodeTargets.empty();
}

void AgentConnectManager::PrepareLowCodeCallerDeathLocked(
    const sptr<IRemoteObject> &remote, const AgentHostKey &hostKey, AgentCallerDeathRequest &request)
{
    auto sessionIter = agentHostSessions_.find(hostKey);
    if (sessionIter == agentHostSessions_.end() || sessionIter->second == nullptr) {
        return;
    }
    auto session = sessionIter->second;
    std::set<std::string> ownedAgentIds = CollectLowCodeAgentIdsLocked(*session, remote);
    session->callerConnections.erase(remote);
    for (const auto &agentId : ownedAgentIds) {
        auto agentIter = session->agents.find(agentId);
        if (agentIter == session->agents.end() || agentIter->second.callerRemote != remote ||
            agentIter->second.isDisconnecting || agentIter->second.hostConnection == nullptr) {
            continue;
        }
        agentIter->second.isDisconnecting = true;
        AddLowCodeDisconnectTarget(request.lowCodeTargets, agentIter->second.hostConnection, agentIter->first);
    }
}

void AgentConnectManager::CleanupLowCodeCallerDeathTargets(
    const AgentHostKey &hostKey, const sptr<IRemoteObject> &remote, const std::set<std::string> &agentIds)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    auto sessionIter = agentHostSessions_.find(hostKey);
    if (sessionIter == agentHostSessions_.end() || sessionIter->second == nullptr) {
        return;
    }
    auto session = sessionIter->second;
    session->callerConnections.erase(remote);
    for (const auto &agentId : agentIds) {
        auto agentIter = session->agents.find(agentId);
        if (agentIter == session->agents.end() || agentIter->second.callerRemote != remote) {
            continue;
        }
        auto record = agentIter->second;
        RemoveLowCodeAgentLocked(*session, agentId, record);
        session->agents.erase(agentIter);
    }
    if (session->agents.empty() && session->callerConnections.empty()) {
        agentHostSessions_.erase(sessionIter);
    }
}

void AgentConnectManager::HandleConnectionDone(
    const sptr<AAFwk::IAbilityConnection> &connection, int32_t resultCode, bool isDisconnect)
{
    if (isDisconnect || resultCode != ERR_OK) {
        ReleaseTrackedConnection(connection);
    }
}

void AgentConnectManager::RestoreLowCodeDisconnectingState(
    const AgentHostKey &hostKey, const sptr<IRemoteObject> &callerRemote, const std::set<std::string> &agentIds)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    bool hasRemainingDisconnectingTarget = false;
    auto sessionIter = agentHostSessions_.find(hostKey);
    if (sessionIter != agentHostSessions_.end() && sessionIter->second != nullptr) {
        auto &session = sessionIter->second;
        for (const auto &agentId : agentIds) {
            auto agentIter = session->agents.find(agentId);
            if (agentIter != session->agents.end() &&
                (callerRemote == nullptr || agentIter->second.callerRemote == callerRemote)) {
                agentIter->second.isDisconnecting = false;
            }
        }
        for (const auto &agentEntry : session->agents) {
            if (agentEntry.second.callerRemote == callerRemote && agentEntry.second.isDisconnecting) {
                hasRemainingDisconnectingTarget = true;
                break;
            }
        }
    }
    auto trackedIter = trackedConnections_.find(callerRemote);
    if (trackedIter != trackedConnections_.end()) {
        trackedIter->second.isDisconnecting = hasRemainingDisconnectingTarget;
    }
}

int32_t AgentConnectManager::PrepareLowCodeComplete(
    const std::string &agentId, int32_t callingUid, LowCodeCompleteRequest &request)
{
    std::lock_guard<std::mutex> lock(stateLock_);
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
    auto record = agentIter->second;
    if (HasOtherAgentForCallerLocked(*session, agentId, record.callerRemote)) {
        RemoveLowCodeAgentLocked(*session, agentId, record);
        session->agents.erase(agentIter);
        return ERR_OK;
    }
    agentIter->second.isDisconnecting = true;
    request.hostKey = session->key;
    request.agentId = agentId;
    request.hostConnection = agentIter->second.hostConnection;
    return ERR_OK;
}

int32_t AgentConnectManager::PrepareLowCodeConnectPlan(
    const AgentConnectPlanRequest &request, AgentConnectPlan &plan)
{
    plan.hostKey = request.hostKey;
    plan.hostUid = request.hostUid;
    plan.callerUid = request.callerUid;
    plan.agentId = request.agentId;
    std::lock_guard<std::mutex> lock(stateLock_);

    auto callerRemote = GetConnectionIdentityRemote(request.connection);
    if (callerRemote == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection remote object is null");
        return ERR_INVALID_VALUE;
    }
    plan.callerRemote = callerRemote;
    plan.hostConnection = sptr<AgentHostConnection>::MakeSptr(request.hostKey, callerRemote, request.agentId);
    if (plan.hostConnection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Create low-code agent connection failed");
        return ERR_INVALID_VALUE;
    }

    std::shared_ptr<AgentHostSession> session;
    auto ret = GetOrCreateLowCodeHostSessionLocked(request.hostKey, request.hostUid, plan, session);
    if (ret != ERR_OK) {
        return ret;
    }
    ret = RegisterLowCodeConnectionLocked(session, request.connection, request.agentId, request.deathHandler, plan);
    if (ret != ERR_OK) {
        return ret;
    }
    FillLowCodeConnectPlanCacheLocked(session, plan);
    return ERR_OK;
}

int32_t AgentConnectManager::GetOrCreateLowCodeHostSessionLocked(const AgentHostKey &hostKey, int32_t hostUid,
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

int32_t AgentConnectManager::RegisterLowCodeConnectionLocked(std::shared_ptr<AgentHostSession> &session,
    const sptr<AAFwk::IAbilityConnection> &connection, const std::string &agentId,
    const CallerDeathHandler &deathHandler, AgentConnectPlan &plan)
{
    auto trackedIter = trackedConnections_.find(plan.callerRemote);
    if (trackedIter != trackedConnections_.end()) {
        if (!trackedIter->second.isLowCode || trackedIter->second.isDisconnecting ||
            !AgentHostKeyEqual()(trackedIter->second.hostKey, plan.hostKey)) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection already tracked by incompatible agent connection");
            return ERR_INVALID_VALUE;
        }
        auto hostConnection = FindHostConnectionForCaller(*session, plan.callerRemote);
        if (hostConnection == nullptr) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "low-code caller host connection missing");
            return ERR_INVALID_VALUE;
        }
        plan.hostConnection = hostConnection;
        session->callerConnections[plan.callerRemote] = connection;
        plan.reusedCallerConnection = true;
        auto ret = AdmitLowCodeAgentLocked(plan.callerUid, BuildLowCodeQuotaKey(plan.hostKey), *session, agentId);
        if (ret == AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE) {
            plan.admittedNewAgent = false;
            return ERR_OK;
        }
        if (ret != ERR_OK) {
            return ret;
        }
        plan.admittedNewAgent = true;
        session->agents[agentId] = LowCodeAgentRecord {
            plan.callerRemote, plan.callerUid, true, plan.hostConnection
        };
        agentOwners_[AgentOwnerKey { plan.callerUid, agentId }] = session;
        return ERR_OK;
    }

    auto ret = TryRegisterConnectionLocked(connection, plan.callerUid, deathHandler, plan.hostConnection,
        &plan.hostKey);
    if (ret != ERR_OK) {
        if (plan.needRealConnect) {
            agentHostSessions_.erase(plan.hostKey);
        }
        return ret;
    }
    plan.registeredTrackedConnection = true;
    session->callerConnections[plan.callerRemote] = connection;
    if (plan.reusedHostSession) {
        ret = AdmitLowCodeAgentLocked(plan.callerUid, BuildLowCodeQuotaKey(plan.hostKey), *session, agentId);
        if (ret == AAFwk::ERR_LOW_CODE_AGENT_ALREADY_ACTIVE) {
            plan.admittedNewAgent = false;
            return ERR_OK;
        }
        if (ret != ERR_OK) {
            session->callerConnections.erase(plan.callerRemote);
            ReleaseTrackedConnectionByRemoteLocked(plan.callerRemote);
            return ret;
        }
        plan.admittedNewAgent = true;
        session->agents[agentId] = LowCodeAgentRecord {
            plan.callerRemote, plan.callerUid, true, plan.hostConnection
        };
        agentOwners_[AgentOwnerKey { plan.callerUid, agentId }] = session;
        return ERR_OK;
    }
    return AdmitInitialLowCodeAgentLocked(session, agentId, plan);
}

int32_t AgentConnectManager::AdmitInitialLowCodeAgentLocked(std::shared_ptr<AgentHostSession> &session,
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
    agentOwners_[AgentOwnerKey { plan.callerUid, agentId }] = session;
    return ERR_OK;
}

void AgentConnectManager::FillLowCodeConnectPlanCacheLocked(
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

void AgentConnectManager::CleanupLowCodeConnectPlan(const AgentConnectPlan &plan, const std::string &agentId)
{
    std::lock_guard<std::mutex> lock(stateLock_);
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

int32_t AgentConnectManager::SetLowCodeConnectIdentity(const AgentHostKey &hostKey, const std::string &agentId,
    const std::string &originalIdentity, long verificationNonce)
{
    std::lock_guard<std::mutex> lock(stateLock_);
    auto sessionIter = agentHostSessions_.find(hostKey);
    if (sessionIter == agentHostSessions_.end() || sessionIter->second == nullptr) {
        return ERR_INVALID_VALUE;
    }
    auto agentIter = sessionIter->second->agents.find(agentId);
    if (agentIter == sessionIter->second->agents.end()) {
        return ERR_INVALID_VALUE;
    }
    agentIter->second.originalIdentity = originalIdentity;
    agentIter->second.verificationNonce = verificationNonce;
    return ERR_OK;
}

AgentHostConnectDoneResult AgentConnectManager::HandleAgentHostConnectDone(
    const AgentHostConnectDoneRequest &request)
{
    AgentHostConnectDoneResult result;
    std::lock_guard<std::mutex> lock(stateLock_);
    auto sessionIter = agentHostSessions_.find(request.hostKey);
    if (sessionIter == agentHostSessions_.end()) {
        return result;
    }
    auto session = sessionIter->second;
    if (session == nullptr) {
        agentHostSessions_.erase(sessionIter);
        return result;
    }
    session->element = request.element;
    if (request.resultCode == ERR_OK && request.remoteObject != nullptr) {
        session->remoteObject = request.remoteObject;
        session->resultCode = request.resultCode;
        session->isConnected = true;
    }
    auto callerIter = session->callerConnections.find(request.callerRemote);
    if (callerIter != session->callerConnections.end()) {
        result.callback = callerIter->second;
    }
    auto agentIter = session->agents.find(request.agentId);
    if (agentIter == session->agents.end()) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code connect done for unknown agent");
        return result;
    }
    agentIter->second.isPending = false;
    if (request.resultCode == ERR_OK && request.remoteObject != nullptr) {
        return result;
    }
    auto record = agentIter->second;
    RemoveLowCodeAgentLocked(*session, request.agentId, record);
    session->agents.erase(agentIter);
    result.releaseConnectionOnFailure = !HasOtherAgentForCallerLocked(*session, request.agentId, record.callerRemote);
    if (result.releaseConnectionOnFailure && record.callerRemote != nullptr) {
        session->callerConnections.erase(record.callerRemote);
        ReleaseTrackedConnectionByRemoteLocked(record.callerRemote);
    }
    if (session->agents.empty() && session->callerConnections.empty()) {
        agentHostSessions_.erase(sessionIter);
    }
    return result;
}

AgentHostDisconnectDoneResult AgentConnectManager::HandleAgentHostDisconnectDone(
    const AgentHostDisconnectDoneRequest &request)
{
    AgentHostDisconnectDoneResult result;
    std::lock_guard<std::mutex> lock(stateLock_);
    auto sessionIter = agentHostSessions_.find(request.hostKey);
    if (sessionIter == agentHostSessions_.end()) {
        return result;
    }
    auto session = sessionIter->second;
    if (session == nullptr) {
        agentHostSessions_.erase(sessionIter);
        return result;
    }
    auto callerIter = session->callerConnections.find(request.callerRemote);
    if (callerIter != session->callerConnections.end()) {
        result.callback = callerIter->second;
    }
    if (request.agentIds.empty()) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code disconnect done without agent");
        return result;
    }
    for (const auto &agentId : request.agentIds) {
        auto agentIter = session->agents.find(agentId);
        if (agentIter == session->agents.end()) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "low-code disconnect done for unknown agent");
            continue;
        }
        auto record = agentIter->second;
        RemoveLowCodeAgentLocked(*session, agentId, record);
        session->agents.erase(agentIter);
    }
    result.releaseConnection = !std::any_of(session->agents.begin(), session->agents.end(),
        [&request](const auto &agentEntry) {
            return agentEntry.second.callerRemote == request.callerRemote;
        });
    if (result.releaseConnection && request.callerRemote != nullptr) {
        session->callerConnections.erase(request.callerRemote);
        ReleaseTrackedConnectionByRemoteLocked(request.callerRemote);
    }
    if (session->agents.empty()) {
        agentHostSessions_.erase(sessionIter);
    }
    return result;
}

void AgentConnectManager::RemoveLowCodeAgentLocked(
    AgentHostSession &session, const std::string &agentId, const LowCodeAgentRecord &record)
{
    ReleaseLowCodeAgentQuotaLocked(session, agentId, record.callerUid, BuildLowCodeQuotaKey(session.key));
    agentOwners_.erase(AgentOwnerKey { record.callerUid, agentId });
}

std::set<std::string> AgentConnectManager::CollectLowCodeAgentIdsLocked(
    const AgentHostSession &session, const sptr<IRemoteObject> &callerRemote) const
{
    std::set<std::string> agentIds;
    for (const auto &agentEntry : session.agents) {
        if (agentEntry.second.callerRemote == callerRemote) {
            agentIds.insert(agentEntry.first);
        }
    }
    return agentIds;
}

bool AgentConnectManager::HasOtherAgentForCallerLocked(
    const AgentHostSession &session, const std::string &agentId, const sptr<IRemoteObject> &callerRemote) const
{
    for (const auto &entry : session.agents) {
        if (entry.first != agentId && entry.second.callerRemote == callerRemote) {
            return true;
        }
    }
    return false;
}

void AgentConnectManager::EraseAgentOwnersLocked(const AgentHostSession &session)
{
    for (const auto &agentEntry : session.agents) {
        agentOwners_.erase(AgentOwnerKey { agentEntry.second.callerUid, agentEntry.first });
    }
}

void AgentConnectManager::ClearAgentHostSessionLocked(const AgentHostKey &key)
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

sptr<IRemoteObject> AgentConnectManager::GetConnectionIdentityRemote(
    const sptr<AAFwk::IAbilityConnection> &connection) const
{
    if (connection == nullptr) {
        return nullptr;
    }
    return connection->AsObject();
}
}  // namespace AgentRuntime
}  // namespace OHOS
