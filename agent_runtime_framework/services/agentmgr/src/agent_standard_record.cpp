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

#include "agent_standard_record.h"

#include <algorithm>

namespace OHOS {
namespace AgentRuntime {
namespace {
AgentTargetInfo BuildTarget(const StandardAgentKey &key)
{
    if (!key.targetIdentity.bundleName.empty() || !key.targetIdentity.abilityName.empty()) {
        return AgentTargetInfo { 0, key.targetIdentity.bundleName, key.targetIdentity.moduleName,
            key.targetIdentity.abilityName };
    }
    return AgentTargetInfo { 0, key.bundleName, key.moduleName, key.abilityName };
}

AgentCallerIdentity GetCallerIdentity(const StandardAgentKey &key)
{
    return key.callerIdentity.IsValid() ? key.callerIdentity : AgentCallerIdentity(key.callerUid);
}

AgentTargetIdentity GetTargetIdentity(const StandardAgentKey &key)
{
    if (!key.targetIdentity.bundleName.empty() || !key.targetIdentity.abilityName.empty()) {
        return key.targetIdentity;
    }
    AgentTargetIdentity identity;
    identity.agentId = key.agentId;
    identity.bundleName = key.bundleName;
    identity.moduleName = key.moduleName;
    identity.abilityName = key.abilityName;
    return identity;
}
}

AgentStandardRecord::AgentStandardRecord() : AgentBaseRecord(AgentTargetInfo {})
{}

AgentStandardRecord::AgentStandardRecord(const StandardAgentKey &keyValue)
    : AgentBaseRecord(BuildTarget(keyValue)), key(keyValue)
{}

bool AgentStandardRecord::Matches(const StandardAgentKey &keyValue) const
{
    return GetCallerIdentity(key) == GetCallerIdentity(keyValue) &&
        GetTargetIdentity(key) == GetTargetIdentity(keyValue) && MatchesTarget(BuildTarget(keyValue));
}

bool AgentStandardRecord::MatchesServiceRemote(const sptr<IRemoteObject> &serviceRemote) const
{
    if (serviceRemote == nullptr) {
        return false;
    }
    if (serviceConnection != nullptr && serviceConnection->AsObject() == serviceRemote) {
        return true;
    }
    return standardServiceConnections.find(serviceRemote) != standardServiceConnections.end();
}

bool AgentStandardRecord::IsReusable() const
{
    return !isDisconnecting && (state == StandardAgentState::CONNECTING || state == StandardAgentState::ACTIVE);
}

void AgentStandardRecord::MarkConnected()
{
    state = StandardAgentState::ACTIVE;
    isDisconnecting = false;
}

void AgentStandardRecord::MarkConnectFailed()
{
    state = StandardAgentState::FAILED;
    isDisconnecting = false;
}

void AgentStandardRecord::MarkDisconnectRequested()
{
    state = StandardAgentState::DISCONNECT_REQUESTED;
}

void AgentStandardRecord::MarkBackingDisconnecting()
{
    state = StandardAgentState::DISCONNECTING_BACKING;
    isDisconnecting = true;
}

void AgentStandardRecord::MarkDisconnectFailed()
{
    state = StandardAgentState::ACTIVE;
    isDisconnecting = false;
}

void AgentStandardRecord::MarkDisconnected()
{
    state = StandardAgentState::DISCONNECTED;
    isDisconnecting = false;
}

void AgentStandardRecord::AddCallerRemote(const sptr<IRemoteObject> &callerRemote)
{
    if (callerRemote == nullptr) {
        return;
    }
    if (std::find(callerRemotes.begin(), callerRemotes.end(), callerRemote) == callerRemotes.end()) {
        callerRemotes.push_back(callerRemote);
    }
}

void AgentStandardRecord::AddCallback(const sptr<AAFwk::IAbilityConnection> &connection)
{
    auto &callbacks = state == StandardAgentState::CONNECTED ? connectedCallbacks : pendingCallbacks;
    if (connection != nullptr && std::find(callbacks.begin(), callbacks.end(), connection) == callbacks.end()) {
        callbacks.push_back(connection);
    }
}

void AgentStandardRecord::AddServiceConnection(const sptr<AAFwk::IAbilityConnection> &serviceConnectionValue,
    const sptr<AAFwk::IAbilityConnection> &callback)
{
    if (serviceConnectionValue == nullptr) {
        return;
    }
    standardServiceConnections[serviceConnectionValue->AsObject()] = serviceConnectionValue;
    if (state == StandardAgentState::CONNECTING || serviceConnectionValue != serviceConnection) {
        pendingCallbackByServiceRemote[serviceConnectionValue->AsObject()] = callback;
    }
}

void AgentStandardRecord::CacheConnectResult(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    cachedElement = element;
    cachedRemoteObject = remoteObject;
    cachedResultCode = resultCode;
}

void AgentStandardRecord::CollectConnectDoneCallbacks(const sptr<IRemoteObject> &serviceRemote,
    std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks)
{
    MarkConnected();
    auto pendingIt = pendingCallbackByServiceRemote.find(serviceRemote);
    if (pendingIt != pendingCallbackByServiceRemote.end()) {
        callbacks.emplace_back(pendingIt->second);
        RemoveCallback(pendingIt->second);
        connectedCallbacks.emplace_back(pendingIt->second);
        pendingCallbackByServiceRemote.erase(pendingIt);
        return;
    }
    callbacks = pendingCallbacks;
    pendingCallbacks.clear();
    connectedCallbacks.insert(connectedCallbacks.end(), callbacks.begin(), callbacks.end());
}

void AgentStandardRecord::CollectConnectFailedCallbacks(const sptr<IRemoteObject> &serviceRemote,
    std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks)
{
    auto pendingIt = pendingCallbackByServiceRemote.find(serviceRemote);
    if (pendingIt != pendingCallbackByServiceRemote.end()) {
        callbacks.emplace_back(pendingIt->second);
        return;
    }
    callbacks = pendingCallbacks;
    pendingCallbacks.clear();
}

void AgentStandardRecord::RemoveServiceConnection(const sptr<AAFwk::IAbilityConnection> &serviceConnectionValue)
{
    if (serviceConnectionValue == nullptr) {
        return;
    }
    auto serviceRemote = serviceConnectionValue->AsObject();
    pendingCallbackByServiceRemote.erase(serviceRemote);
    pendingDisconnectServiceRemotes.erase(serviceRemote);
    standardServiceConnections.erase(serviceRemote);
    if (serviceConnection == serviceConnectionValue) {
        serviceConnection = standardServiceConnections.empty() ? nullptr : standardServiceConnections.begin()->second;
    }
}

void AgentStandardRecord::RemoveCallerRemote(const sptr<IRemoteObject> &callerRemote)
{
    callerRemotes.erase(std::remove(callerRemotes.begin(), callerRemotes.end(), callerRemote), callerRemotes.end());
}

void AgentStandardRecord::RemoveCallback(const sptr<AAFwk::IAbilityConnection> &callback)
{
    pendingCallbacks.erase(std::remove(pendingCallbacks.begin(), pendingCallbacks.end(), callback),
        pendingCallbacks.end());
    connectedCallbacks.erase(std::remove(connectedCallbacks.begin(), connectedCallbacks.end(), callback),
        connectedCallbacks.end());
}

void AgentStandardRecord::CollectAllDisconnectCallbacks(std::vector<sptr<AAFwk::IAbilityConnection>> &callbacks)
{
    callbacks = pendingCallbacks;
    pendingCallbacks.clear();
    callbacks.insert(callbacks.end(), connectedCallbacks.begin(), connectedCallbacks.end());
    connectedCallbacks.clear();
}
}  // namespace AgentRuntime
}  // namespace OHOS
