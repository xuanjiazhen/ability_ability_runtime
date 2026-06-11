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
#include "agent_service_connection.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
// Bridges legacy uid-only keys used by existing tests to the new caller identity model.
AgentCallerIdentity GetStandardCallerIdentity(const StandardAgentKey &key)
{
    return key.callerIdentity.IsValid() ? key.callerIdentity : AgentCallerIdentity(key.callerUid);
}
}

StandardAgentKeyBuilder &StandardAgentKeyBuilder::SetCallerIdentity(const AgentCallerIdentity &callerIdentity)
{
    callerIdentity_ = callerIdentity;
    return *this;
}

StandardAgentKeyBuilder &StandardAgentKeyBuilder::SetAgentId(const std::string &agentId)
{
    agentId_ = agentId;
    return *this;
}

StandardAgentKeyBuilder &StandardAgentKeyBuilder::SetCardType(AgentCardType cardType)
{
    cardType_ = cardType;
    return *this;
}

StandardAgentKeyBuilder &StandardAgentKeyBuilder::SetTargetUid(int32_t targetUid)
{
    targetUid_ = targetUid;
    return *this;
}

StandardAgentKeyBuilder &StandardAgentKeyBuilder::SetTargetInstanceKey(
    const std::string &targetInstanceKey)
{
    targetInstanceKey_ = targetInstanceKey;
    return *this;
}

StandardAgentKeyBuilder &StandardAgentKeyBuilder::SetWant(const AAFwk::Want &want)
{
    want_ = want;
    return *this;
}

StandardAgentKey StandardAgentKeyBuilder::Build() const
{
    StandardAgentKey key;
    key.callerIdentity = callerIdentity_;
    key.targetIdentity.type = cardType_;
    key.targetIdentity.targetUid = targetUid_;
    key.targetIdentity.targetInstanceKey = targetInstanceKey_;
    key.targetIdentity.agentId = agentId_;
    const auto &element = want_.GetElement();
    key.targetIdentity.bundleName = element.GetBundleName();
    key.targetIdentity.moduleName = element.GetModuleName();
    key.targetIdentity.abilityName = element.GetAbilityName();
    return key;
}

StandardAgentKey AgentManagerService::BuildStandardAgentKey(int32_t callerUid, const std::string &agentId,
    const AAFwk::Want &want) const
{
    return StandardAgentKeyBuilder()
        .SetCallerIdentity(AgentCallerIdentity(callerUid))
        .SetAgentId(agentId)
        .SetCardType(AgentCardType::APP)
        .SetWant(want)
        .Build();
}

bool AgentManagerService::IsStandardAgentKeyEqual(const StandardAgentKey &left, const StandardAgentKey &right) const
{
    return IsStandardAgentKeyMatched(left, right);
}

bool AgentManagerService::IsStandardAgentKeyMatched(
    const StandardAgentKey &storedKey, const StandardAgentKey &incomingKey) const
{
    return AgentStandardRecord(storedKey).Matches(incomingKey);
}

std::shared_ptr<AgentStandardRecord> AgentManagerService::FindStandardSessionLocked(
    const StandardAgentKey &key) const
{
    for (const auto &entry : standardSessions_) {
        if (entry != nullptr && entry->IsReusable() && entry->Matches(key)) {
            return entry;
        }
    }
    return nullptr;
}

AgentManagerService::StandardSessionIter AgentManagerService::FindStandardSessionByServiceRemoteLocked(
    const sptr<IRemoteObject> &serviceRemote)
{
    return std::find_if(standardSessions_.begin(), standardSessions_.end(),
        [&serviceRemote](const auto &item) {
            return item != nullptr && item->MatchesServiceRemote(serviceRemote);
        });
}

int32_t AgentManagerService::RegisterStandardSessionCallerLocked(
    const std::shared_ptr<AgentStandardRecord> &session,
    const sptr<AAFwk::IAbilityConnection> &connection, bool countTowardsCallerLimit,
    const sptr<AAFwk::IAbilityConnection> &serviceConnection)
{
    if (session == nullptr || connection == nullptr) {
        return ERR_INVALID_VALUE;
    }
    auto actualServiceConnection = serviceConnection != nullptr ? serviceConnection : session->serviceConnection;
    if (actualServiceConnection == nullptr) {
        return ERR_INVALID_VALUE;
    }
    auto callerRemote = GetConnectionIdentityRemote(connection);
    if (callerRemote == nullptr) {
        return ERR_INVALID_VALUE;
    }

    auto trackedIt = trackedConnections_.find(callerRemote);
    if (trackedIt == trackedConnections_.end()) {
        auto ret = TryRegisterConnectionLocked(connection, GetStandardCallerIdentity(session->key),
            actualServiceConnection, nullptr, countTowardsCallerLimit);
        if (ret != ERR_OK) {
            return ret;
        }
        trackedIt = trackedConnections_.find(callerRemote);
    } else if (GetStandardCallerIdentity(trackedIt->second.standardKey).IsValid() &&
        !IsStandardAgentKeyEqual(trackedIt->second.standardKey, session->key)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Connection already belongs to another standard session");
        return ERR_INVALID_VALUE;
    }

    if (trackedIt != trackedConnections_.end()) {
        trackedIt->second.standardKey = session->key;
        trackedIt->second.serviceConnection = actualServiceConnection;
    }
    session->AddCallerRemote(callerRemote);
    session->AddCallback(connection);
    session->AddServiceConnection(actualServiceConnection, connection);
    return ERR_OK;
}

bool AgentManagerService::TransferStandardCallerLimitWithinSessionLocked(
    const std::shared_ptr<AgentStandardRecord> &session, const sptr<IRemoteObject> &transferFrom,
    sptr<IRemoteObject> &transferTo)
{
    if (session == nullptr || transferFrom == nullptr) {
        return false;
    }
    auto currentIter = trackedConnections_.find(transferFrom);
    if (currentIter == trackedConnections_.end() || !currentIter->second.countTowardsCallerLimit) {
        return false;
    }
    for (const auto &candidateRemote : session->callerRemotes) {
        if (candidateRemote == nullptr || candidateRemote == transferFrom) {
            continue;
        }
        auto candidateIter = trackedConnections_.find(candidateRemote);
        if (candidateIter == trackedConnections_.end() ||
            !(candidateIter->second.callerIdentity == currentIter->second.callerIdentity) ||
            candidateIter->second.countTowardsCallerLimit) {
            continue;
        }
        candidateIter->second.countTowardsCallerLimit = true;
        currentIter->second.countTowardsCallerLimit = false;
        transferTo = candidateRemote;
        return true;
    }
    return false;
}

bool AgentManagerService::RemoveStandardSessionCallerLocked(const std::shared_ptr<AgentStandardRecord> &session,
    const sptr<IRemoteObject> &callerRemote, const sptr<AAFwk::IAbilityConnection> &serviceConnection)
{
    if (session == nullptr || callerRemote == nullptr) {
        return false;
    }
    auto trackedIt = trackedConnections_.find(callerRemote);
    sptr<AAFwk::IAbilityConnection> callerConnection =
        trackedIt != trackedConnections_.end() ? iface_cast<AAFwk::IAbilityConnection>(callerRemote) : nullptr;
    sptr<IRemoteObject> transferTo = nullptr;
    TransferStandardCallerLimitWithinSessionLocked(session, callerRemote, transferTo);
    session->RemoveCallback(callerConnection);
    session->RemoveCallerRemote(callerRemote);
    session->RemoveServiceConnection(serviceConnection);
    ReleaseTrackedConnectionByRemoteLocked(callerRemote);
    return true;
}

int32_t AgentManagerService::TryConnectExistingAgentStandardRecord(
    const StandardAgentKey &key, const sptr<AAFwk::IAbilityConnection> &connection, bool &isExistingSession)
{
    isExistingSession = false;
    AppExecFwk::ElementName cachedElement;
    sptr<IRemoteObject> cachedRemoteObject = nullptr;
    int32_t cachedResultCode = ERR_OK;
    bool notifyImmediately = false;
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto existing = FindStandardSessionLocked(key);
        if (existing == nullptr) {
            return ERR_OK;
        }

        isExistingSession = true;
        notifyImmediately = existing->state == StandardAgentState::ACTIVE && existing->cachedRemoteObject != nullptr;
        cachedElement = existing->cachedElement;
        cachedRemoteObject = existing->cachedRemoteObject;
        cachedResultCode = existing->cachedResultCode;
        auto ret = RegisterStandardSessionCallerLocked(existing, connection, false);
        if (ret != ERR_OK) {
            return ret;
        }
    }

    if (notifyImmediately && connection != nullptr) {
        connection->OnAbilityConnectDone(cachedElement, cachedRemoteObject, cachedResultCode);
        HandleConnectionDone(connection, cachedResultCode, false);
    }
    return ERR_OK;
}

int32_t AgentManagerService::CreateAgentStandardRecord(const AAFwk::Want &connectWant,
    const std::string &agentId, const StandardAgentKey &key,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    auto serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    if (serviceConnection == nullptr) {
        return ERR_INVALID_VALUE;
    }

    serviceConnection->SetStandardSessionMode();

    auto session = std::make_shared<AgentStandardRecord>(key);
    session->state = StandardAgentState::CONNECTING;
    session->serviceConnection = serviceConnection;

    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto ret = RegisterStandardSessionCallerLocked(session, connection, true);
        if (ret != ERR_OK) {
            return ret;
        }
        standardSessions_.emplace_back(session);
    }

    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ConnectAbilityWithExtensionType(
        connectWant, serviceConnection, nullptr, AAFwk::DEFAULT_INVAL_VALUE, AppExecFwk::ExtensionAbilityType::AGENT);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "ConnectAbilityWithExtensionType failed: %{public}d", ret);
        ReleaseTrackedConnection(connection);
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto sessionIt = FindStandardSessionByServiceRemoteLocked(serviceConnection->AsObject());
        if (sessionIt != standardSessions_.end()) {
            standardSessions_.erase(sessionIt);
        }
        return ret;
    }
    return ERR_OK;
}

void AgentManagerService::HandleStandardAgentConnectDone(
    const sptr<IRemoteObject> &serviceRemote,
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto sessionIt = FindStandardSessionByServiceRemoteLocked(serviceRemote);
        if (sessionIt == standardSessions_.end() || *sessionIt == nullptr) {
            return;
        }
        auto &session = *sessionIt;
        session->CacheConnectResult(element, remoteObject, resultCode);

        if (resultCode == ERR_OK && remoteObject != nullptr) {
            session->CollectConnectDoneCallbacks(serviceRemote, callbacks);
        } else {
            auto pendingIt = session->pendingCallbackByServiceRemote.find(serviceRemote);
            if (pendingIt != session->pendingCallbackByServiceRemote.end()) {
                callbacks.emplace_back(pendingIt->second);
                RemoveStandardSessionCallerLocked(session, pendingIt->second->AsObject(),
                    session->standardServiceConnections[serviceRemote]);
            } else {
                session->MarkConnectFailed();
                callbacks = session->pendingCallbacks;
                session->pendingCallbacks.clear();
                standardSessions_.erase(sessionIt);
            }
        }
    }

    for (const auto &cb : callbacks) {
        if (cb != nullptr) {
            cb->OnAbilityConnectDone(element, remoteObject, resultCode);
        }
        if (resultCode != ERR_OK) {
            ReleaseTrackedConnection(cb);
        }
    }
}

void AgentManagerService::HandleStandardAgentDisconnectDone(
    const sptr<IRemoteObject> &serviceRemote,
    const AppExecFwk::ElementName &element, int32_t resultCode)
{
    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    {
        std::lock_guard<std::mutex> lock(connectionLock_);
        auto sessionIt = FindStandardSessionByServiceRemoteLocked(serviceRemote);
        if (sessionIt == standardSessions_.end() || *sessionIt == nullptr) {
            return;
        }
        auto &session = *sessionIt;
        auto pendingDisconnectIt = session->pendingDisconnectServiceRemotes.find(serviceRemote);
        if (resultCode != ERR_OK) {
            bool wasBackingDisconnect = session->isDisconnecting;
            session->pendingDisconnectServiceRemotes.erase(serviceRemote);
            session->MarkDisconnectFailed();
            for (auto &trackedEntry : trackedConnections_) {
                auto &record = trackedEntry.second;
                if (record.serviceConnection == nullptr || record.serviceConnection->AsObject() != serviceRemote) {
                    continue;
                }
                record.isDisconnecting = false;
            }
            if (wasBackingDisconnect) {
                callerConnectionCounts_[GetStandardCallerIdentity(session->key)]++;
            }
            return;
        }
        if (pendingDisconnectIt != session->pendingDisconnectServiceRemotes.end() && !session->isDisconnecting) {
            for (auto trackedIt = trackedConnections_.begin(); trackedIt != trackedConnections_.end(); ++trackedIt) {
                const auto &record = trackedIt->second;
                if (record.serviceConnection == nullptr || record.serviceConnection->AsObject() != serviceRemote ||
                    !record.isDisconnecting) {
                    continue;
                }
                auto callback = iface_cast<AAFwk::IAbilityConnection>(trackedIt->first);
                if (callback != nullptr) {
                    callbacks.emplace_back(callback);
                }
                RemoveStandardSessionCallerLocked(session, trackedIt->first, record.serviceConnection);
                break;
            }
            if (session->callerRemotes.empty()) {
                session->MarkDisconnected();
                standardSessions_.erase(sessionIt);
            } else {
                session->MarkConnected();
            }
        } else {
            session->MarkDisconnected();
            session->CollectAllDisconnectCallbacks(callbacks);
            standardSessions_.erase(sessionIt);
        }
    }

    for (const auto &cb : callbacks) {
        if (cb != nullptr) {
            cb->OnAbilityDisconnectDone(element, resultCode);
            HandleConnectionDone(cb, resultCode, true);
        }
    }
}
}  // namespace AgentRuntime
}  // namespace OHOS
