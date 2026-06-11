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

#include "agent_low_code_record.h"

#include <algorithm>

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "agent_receiver_proxy.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
AgentTargetInfo BuildTarget(const AgentHostKey &key)
{
    return AgentTargetInfo { key.userId, key.bundleName, key.moduleName, key.abilityName };
}
}

AgentLowCodeRecord::AgentLowCodeRecord() : AgentBaseRecord(AgentTargetInfo {})
{}

AgentLowCodeRecord::AgentLowCodeRecord(const AgentHostKey &keyValue)
    : AgentBaseRecord(BuildTarget(keyValue)), key(keyValue)
{}

bool AgentLowCodeRecord::Matches(const AgentHostKey &keyValue) const
{
    return MatchesTarget(BuildTarget(keyValue)) && key.targetUid == keyValue.targetUid &&
        key.appIndex == keyValue.appIndex && key.targetInstanceKey == keyValue.targetInstanceKey;
}

bool AgentLowCodeRecord::HasAgent(const std::string &agentId) const
{
    return agents.find(agentId) != agents.end();
}

bool AgentLowCodeRecord::HasCapacity(size_t maxAgents) const
{
    return agents.size() < maxAgents;
}

bool AgentLowCodeRecord::HasOtherAgentsForCaller(const std::string &agentId,
    const sptr<IRemoteObject> &callerRemote, const sptr<IRemoteObject> &hostProxyRemote) const
{
    for (const auto &entry : agents) {
        if (entry.first == agentId) {
            continue;
        }
        if (entry.second.callerRemote == callerRemote && entry.second.hostProxyRemote == hostProxyRemote) {
            return true;
        }
    }
    return false;
}

bool AgentLowCodeRecord::HasHostProxyConnection(const sptr<IRemoteObject> &hostProxyRemote) const
{
    return hostProxyConnections.find(hostProxyRemote) != hostProxyConnections.end();
}

bool AgentLowCodeRecord::IsReusable() const
{
    return !isDisconnecting && (state == LowCodeHostState::CONNECTING || state == LowCodeHostState::ACTIVE);
}

void AgentLowCodeRecord::MarkConnected()
{
    state = LowCodeHostState::ACTIVE;
    isConnected = true;
    isDisconnecting = false;
}

void AgentLowCodeRecord::MarkConnectFailed()
{
    state = LowCodeHostState::FAILED;
    isConnected = false;
    isDisconnecting = false;
}

void AgentLowCodeRecord::MarkDisconnectRequested()
{
    state = LowCodeHostState::DISCONNECT_REQUESTED;
}

void AgentLowCodeRecord::MarkBackingDisconnecting()
{
    state = LowCodeHostState::DISCONNECTING_BACKING;
    isDisconnecting = true;
}

void AgentLowCodeRecord::MarkDisconnectFailed()
{
    state = LowCodeHostState::ACTIVE;
    isConnected = true;
    isDisconnecting = false;
}

void AgentLowCodeRecord::MarkDisconnected()
{
    state = LowCodeHostState::DISCONNECTED;
    isConnected = false;
    isDisconnecting = false;
}

int32_t AgentLowCodeRecord::CreateHostConnection(const AgentHostKey &hostKey,
    const sptr<IRemoteObject> &hostProxyRemote)
{
    AgentHostKey proxyKey = hostKey;
    proxyKey.hostProxyRemote = hostProxyRemote;
    auto newHostConnection = sptr<AgentHostConnection>::MakeSptr(proxyKey);
    if (newHostConnection == nullptr) {
        return ERR_INVALID_VALUE;
    }
    AddHostConnection(hostProxyRemote, newHostConnection);
    if (hostConnection == nullptr) {
        hostConnection = newHostConnection;
    }
    return ERR_OK;
}

void AgentLowCodeRecord::AddHostConnection(const sptr<IRemoteObject> &hostProxyRemote,
    const sptr<AgentHostConnection> &hostConnectionValue)
{
    if (hostProxyRemote != nullptr && hostConnectionValue != nullptr) {
        hostProxyConnections[hostProxyRemote] = hostConnectionValue;
    }
}

void AgentLowCodeRecord::AddAgent(const std::string &agentId, const sptr<IRemoteObject> &callerRemote,
    const sptr<AAFwk::IAbilityConnection> &connection, const sptr<IRemoteObject> &hostProxyRemote,
    bool isPending)
{
    callerConnections[callerRemote] = connection;
    agents[agentId] = LowCodeAgentRecord { callerRemote, isPending, hostProxyRemote };
}

void AgentLowCodeRecord::RemoveAgent(const std::string &agentId)
{
    agents.erase(agentId);
}

void AgentLowCodeRecord::RemoveAgentsByHostProxy(const sptr<IRemoteObject> &hostProxyRemote,
    std::vector<std::pair<std::string, sptr<IRemoteObject>>> &removedAgents)
{
    for (auto agentIter = agents.begin(); agentIter != agents.end();) {
        if (agentIter->second.hostProxyRemote != hostProxyRemote) {
            ++agentIter;
            continue;
        }
        removedAgents.emplace_back(agentIter->first, agentIter->second.callerRemote);
        agentIter = agents.erase(agentIter);
    }
}

void AgentLowCodeRecord::RemoveAgentsByCaller(const sptr<IRemoteObject> &callerRemote,
    std::vector<std::string> &removedAgentIds, sptr<IRemoteObject> &hostProxyRemote)
{
    for (auto agentIter = agents.begin(); agentIter != agents.end();) {
        if (agentIter->second.callerRemote != callerRemote) {
            ++agentIter;
            continue;
        }
        hostProxyRemote = agentIter->second.hostProxyRemote;
        removedAgentIds.emplace_back(agentIter->first);
        agentIter = agents.erase(agentIter);
    }
}

void AgentLowCodeRecord::RemoveCaller(const sptr<IRemoteObject> &callerRemote,
    const sptr<IRemoteObject> &hostProxyRemote, bool callerStillOwnsAgent)
{
    if (callerStillOwnsAgent || callerRemote == nullptr) {
        return;
    }
    PrepareDisconnectCallback(callerRemote, hostProxyRemote);
    callerConnections.erase(callerRemote);
}

sptr<AgentHostConnection> AgentLowCodeRecord::GetHostConnection(const sptr<IRemoteObject> &hostProxyRemote) const
{
    auto hostConnectionIt = hostProxyConnections.find(hostProxyRemote);
    return hostConnectionIt != hostProxyConnections.end() ? hostConnectionIt->second : hostConnection;
}

void AgentLowCodeRecord::CacheConnectDone(const AppExecFwk::ElementName &elementValue,
    const sptr<IRemoteObject> &remoteObjectValue, int32_t resultCodeValue)
{
    if (resultCodeValue != ERR_OK || remoteObjectValue == nullptr) {
        if (!isConnected) {
            MarkConnectFailed();
        }
        return;
    }
    element = elementValue;
    cachedRemoteObject_ = remoteObjectValue;
    resultCode = resultCodeValue;
    MarkConnected();
}

void AgentLowCodeRecord::CollectPendingCallbacks(const sptr<IRemoteObject> &hostProxyRemote,
    std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks, std::vector<std::string> &pendingAgentIds)
{
    for (auto &[pendingAgentId, agentRecord] : agents) {
        bool matchesProxy = hostProxyRemote == nullptr || agentRecord.hostProxyRemote == hostProxyRemote;
        if (!agentRecord.isPending || !matchesProxy) {
            continue;
        }
        auto callbackIt = callerConnections.find(agentRecord.callerRemote);
        bool hasCallback = callbackIt != callerConnections.end() && callbackIt->second != nullptr;
        if (hasCallback && std::find(callbacks.begin(), callbacks.end(), callbackIt->second) == callbacks.end()) {
            callbacks.emplace_back(callbackIt->second);
        }
        pendingAgentIds.emplace_back(pendingAgentId);
        agentRecord.isPending = false;
    }
}

void AgentLowCodeRecord::CleanupFailedConnect(const sptr<IRemoteObject> &hostProxyRemote,
    const std::vector<std::string> &pendingAgentIds)
{
    for (const auto &agentId : pendingAgentIds) {
        auto agentIter = agents.find(agentId);
        if (agentIter != agents.end()) {
            hostProxyConnections.erase(agentIter->second.hostProxyRemote);
        }
        agents.erase(agentId);
    }
    if (hostProxyRemote != nullptr) {
        hostProxyConnections.erase(hostProxyRemote);
    }
}

void AgentLowCodeRecord::PrepareDisconnectCallback(const sptr<IRemoteObject> &callerRemote,
    const sptr<IRemoteObject> &hostProxyRemote)
{
    auto callerConnectionIt = callerConnections.find(callerRemote);
    if (callerConnectionIt != callerConnections.end()) {
        disconnectingCallbacks[hostProxyRemote] = callerConnectionIt->second;
    }
}

void AgentLowCodeRecord::CollectDisconnectCallback(const sptr<IRemoteObject> &hostProxyRemote,
    std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks)
{
    if (hostProxyRemote == nullptr) {
        for (const auto &disconnecting : disconnectingCallbacks) {
            if (disconnecting.second != nullptr) {
                callbacks.emplace_back(disconnecting.second);
            }
        }
        disconnectingCallbacks.clear();
        hostProxyConnections.clear();
        return;
    }
    auto disconnectingIt = disconnectingCallbacks.find(hostProxyRemote);
    if (disconnectingIt != disconnectingCallbacks.end()) {
        if (disconnectingIt->second != nullptr) {
            callbacks.emplace_back(disconnectingIt->second);
        }
        disconnectingCallbacks.erase(disconnectingIt);
    }
    hostProxyConnections.erase(hostProxyRemote);
}

bool AgentLowCodeRecord::NotifyAgentInvoked(const std::string &agentId) const
{
    return NotifyAgentInvoked(cachedRemoteObject_, agentId);
}

bool AgentLowCodeRecord::NotifyAgentInvoked(const sptr<IRemoteObject> &remoteObject, const std::string &agentId)
{
    if (remoteObject == nullptr) {
        return false;
    }
    auto receiver = iface_cast<IAgentReceiver>(remoteObject);
    if (receiver == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "agent receiver null");
        return false;
    }
    auto ret = receiver->AgentInvoked(agentId);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "AgentInvoked failed:%{public}d", ret);
        return false;
    }
    return true;
}

const sptr<IRemoteObject> &AgentLowCodeRecord::GetCachedRemoteObject() const
{
    return cachedRemoteObject_;
}

bool AgentLowCodeRecord::HasDeadCachedRemoteObject() const
{
    return cachedRemoteObject_ != nullptr && cachedRemoteObject_->IsObjectDead();
}

bool AgentLowCodeRecord::IsEmptyForRemoval() const
{
    return agents.empty() && disconnectingCallbacks.empty();
}
}  // namespace AgentRuntime
}  // namespace OHOS
