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

#include "agent_connection_manager.h"

#include <algorithm>
#include <optional>
#include <unistd.h>

#include "ability_manager_errors.h"
#include "agent_extension_connection_constants.h"
#include "agent_manager_client.h"
#include "connection_manager.h"
#include "hilog_tag_wrapper.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr int64_t CONNECTING_TIMEOUT = 30000;
constexpr int32_t DIED = -1;

AppExecFwk::ElementName BuildElementName(const AAFwk::Operation &operation)
{
    AppExecFwk::ElementName element;
    element.SetBundleName(operation.GetBundleName());
    element.SetModuleName(operation.GetModuleName());
    element.SetAbilityName(operation.GetAbilityName());
    return element;
}
}

AgentConnectionInfo::AgentConnectionInfo(const std::string &agentId,
    const AAFwk::Operation &connectReceiver, const sptr<AgentConnection> &agentConnection)
    : agentId(agentId), connectReceiver(connectReceiver), agentConnection(agentConnection)
{}

void AgentConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER,
        "OnAbilityConnectDone, bundleName:%{public}s, abilityName:%{public}s, resultCode:%{public}d",
        element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);
    std::vector<sptr<AbilityConnectCallback>> callbacks;
    {
        std::lock_guard<std::mutex> lock(agentMutex_);
        callbacks = GetCallbackList();
        if (callbacks.empty()) {
            TAG_LOGW(AAFwkTag::SER_ROUTER, "empty callbackList");
            return;
        }

        SetRemoteObject(remoteObject);
        SetResultCode(resultCode);
        SetConnectionState(CONNECTION_STATE_CONNECTED);
    }
    sptr<AgentConnection> connection(this);
    if (AgentConnectionManager::GetInstance().DisconnectNonexistentService(element, connection)) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "No need onConnect callback");
        return;
    }

    auto item = callbacks.begin();
    while (item != callbacks.end()) {
        (*item)->OnAbilityConnectDone(element, remoteObject, resultCode);
        item++;
    }
}

void AgentConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER,
        "OnAbilityDisconnectDone, bundleName:%{public}s, abilityName:%{public}s, resultCode:%{public}d",
        element.GetBundleName().c_str(), element.GetAbilityName().c_str(), resultCode);
    std::vector<sptr<AbilityConnectCallback>> callbacks;
    {
        std::lock_guard<std::mutex> lock(agentMutex_);
        SetConnectionState(CONNECTION_STATE_DISCONNECTED);
        callbacks = GetCallbackList();
        if (callbacks.empty()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "empty callbackList");
            return;
        }
    }

    // if resultCode < 0 that means the service is dead
    if (resultCode == DIED) {
        sptr<AgentConnection> connection(this);
        bool ret = AgentConnectionManager::GetInstance().RemoveConnection(connection);
        if (ret) {
            ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
            TAG_LOGI(AAFwkTag::SER_ROUTER, "not disconnected");
        }
        resultCode = DIED + 1;
    }

    for (auto &callback : callbacks) {
        callback->OnAbilityDisconnectDone(element, resultCode);
    }
    SetRemoteObject(nullptr);
}

AgentConnectionManager &AgentConnectionManager::GetInstance()
{
    static AgentConnectionManager agentConnectionManager;
    return agentConnectionManager;
}

ErrCode AgentConnectionManager::ConnectAgentExtensionAbility(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback)
{
    return ConnectAbilityInner(want, connectCallback);
}

ErrCode AgentConnectionManager::DisconnectAgentExtensionAbility(const sptr<AbilityConnectCallback> &connectCallback)
{
    if (connectCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null connectCallback");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "DisconnectAgentExtensionAbility called");
    std::optional<AgentConnectionRecord> disconnectRecord;
    {
        std::lock_guard<std::mutex> lock(connectionsLock_);
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Connection size:%{public}zu", agentConnections_.size());
        auto item = FindConnectionByCallbackLocked(connectCallback);
        if (item == agentConnections_.end()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "connection not found");
            return AAFwk::CONNECTION_NOT_EXIST;
        }

        TAG_LOGD(AAFwkTag::SER_ROUTER, "callback size: %{public}zu", item->second.size());
        sptr<AgentConnection> agentConnection = item->first.agentConnection;
        auto element = BuildElementName(item->first.connectReceiver);
        if (item->second.size() > 1) {
            auto callbackIter = std::find(item->second.begin(), item->second.end(), connectCallback);
            item->second.erase(callbackIter);
            connectCallback->OnAbilityDisconnectDone(element, ERR_OK);
            if (agentConnection != nullptr) {
                agentConnection->RemoveConnectCallback(connectCallback);
            }
            TAG_LOGD(AAFwkTag::SER_ROUTER, "callbacks not empty, no need disconnectAbility");
            return ERR_OK;
        }

        disconnectRecord = *item;
        agentConnections_.erase(item);
    }

    TAG_LOGI(AAFwkTag::SER_ROUTER, "no callback left, disconnectAbility");
    return DisconnectConnectionRecord(*disconnectRecord);
}

ErrCode AgentConnectionManager::DisconnectAgentExtensionAbilityByProxy(
    const sptr<AbilityConnectCallback> &connectCallback)
{
    if (connectCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null connectCallback");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }

    std::optional<AgentConnectionRecord> disconnectRecord;
    {
        std::lock_guard<std::mutex> lock(connectionsLock_);
        auto item = FindConnectionByCallbackLocked(connectCallback);
        if (item == agentConnections_.end()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "connection not found");
            return AAFwk::CONNECTION_NOT_EXIST;
        }
        disconnectRecord = *item;
        agentConnections_.erase(item);
    }

    TAG_LOGI(AAFwkTag::SER_ROUTER, "disconnect agent proxy owner");
    return DisconnectConnectionRecord(*disconnectRecord);
}

ErrCode AgentConnectionManager::ConnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
    const AAFwk::Want &want, const sptr<AAFwk::IAbilityConnection> &connection)
{
    if (callerToken == nullptr || connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null callerToken or connection");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    return AgentManagerClient::GetInstance().ConnectServiceExtensionAbility(callerToken, want, connection);
}

ErrCode AgentConnectionManager::DisconnectServiceExtensionAbility(const sptr<IRemoteObject> &callerToken,
    const sptr<AAFwk::IAbilityConnection> &connection)
{
    if (callerToken == nullptr || connection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null callerToken or connection");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    return AgentManagerClient::GetInstance().DisconnectServiceExtensionAbility(callerToken, connection);
}

bool AgentConnectionManager::RemoveConnection(const sptr<AgentConnection> &connection)
{
    std::lock_guard<std::mutex> lock(connectionsLock_);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "agentConnectionsSize: %{public}zu", agentConnections_.size());
    bool isDisconnect = false;
    for (auto iter = agentConnections_.begin(); iter != agentConnections_.end();) {
        if (iter->first.agentConnection == connection) {
            TAG_LOGD(AAFwkTag::SER_ROUTER, "Remove connection");
            iter = agentConnections_.erase(iter);
            isDisconnect = true;
        } else {
            ++iter;
        }
    }
    return isDisconnect;
}

bool AgentConnectionManager::DisconnectNonexistentService(const AppExecFwk::ElementName &element,
    const sptr<AgentConnection> &connection)
{
    bool exist = false;
    std::lock_guard<std::mutex> lock(connectionsLock_);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "agentConnectionsSize: %{public}zu", agentConnections_.size());
    for (const auto &agentConnection : agentConnections_) {
        if (agentConnection.first.agentConnection == connection &&
            agentConnection.first.connectReceiver.GetBundleName() == element.GetBundleName()) {
            TAG_LOGD(AAFwkTag::SER_ROUTER, "find connection");
            exist = true;
            break;
        }
    }
    if (!exist) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "service need disconnect");
        AgentManagerClient::GetInstance().DisconnectAgentExtensionAbility(connection);
        return true;
    }
    return false;
}

bool AgentConnectionManager::IsConnectReceiverEqual(const AAFwk::Operation &connectReceiver,
    const AppExecFwk::ElementName &connectReceiverOther)
{
    return connectReceiver.GetBundleName() == connectReceiverOther.GetBundleName() &&
        connectReceiver.GetAbilityName() == connectReceiverOther.GetAbilityName();
}

void *AgentConnectionManager::GetAgentExtProxyPtr(const AAFwk::Want &want)
{
    sptr<IRemoteObject> agentExtProxySptr = want.GetRemoteObject(AGENTEXTENSIONHOSTPROXY_KEY);
    return agentExtProxySptr != nullptr ? agentExtProxySptr.GetRefPtr() : nullptr;
}

bool AgentConnectionManager::MatchConnection(const std::string &agentId, const AAFwk::Want &connectReceiver,
    const sptr<AbilityConnectCallback> &connectCallback, const AgentConnectionRecord &connection)
{
    if (agentId != connection.first.agentId) {
        return false;
    }
    const AAFwk::Operation &storedReceiver = connection.first.connectReceiver;
    const AppExecFwk::ElementName &element = connectReceiver.GetElement();
    if (element.GetBundleName() != storedReceiver.GetBundleName() ||
        element.GetModuleName() != storedReceiver.GetModuleName() ||
        element.GetAbilityName() != storedReceiver.GetAbilityName()) {
        return false;
    }
    if (std::find(connection.second.begin(), connection.second.end(), connectCallback) == connection.second.end()) {
        return false;
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "MatchConnection hit, reuse existing agent connection");
    return true;
}

ErrCode AgentConnectionManager::CreateConnection(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback)
{
    sptr<AgentConnection> agentConnection = sptr<AgentConnection>::MakeSptr();
    if (agentConnection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null agentConnection");
        return AAFwk::ERR_INVALID_CALLER;
    }
    agentConnection->AddConnectCallback(connectCallback);
    agentConnection->SetConnectionState(CONNECTION_STATE_CONNECTING);

    // Extract agentId from Want
    std::string agentId = want.GetStringParam(AGENTID_KEY);
    // Create AgentConnectionInfo
    AAFwk::Operation connectReceiver = want.GetOperation();
    AgentConnectionInfo connectionInfo(agentId, connectReceiver, agentConnection);
    connectionInfo.SetAgentExtProxyPtr(GetAgentExtProxyPtr(want));
    connectionInfo.RecordConnectingTime();

    {
        std::lock_guard<std::mutex> lock(connectionsLock_);
        agentConnections_.emplace_back(connectionInfo, AgentConnectionCallbacks { connectCallback });
    }

    ErrCode ret = AgentManagerClient::GetInstance().ConnectAgentExtensionAbility(want, agentConnection);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "error:%{public}d", ret);
        std::lock_guard<std::mutex> lock(connectionsLock_);
        for (auto iter = agentConnections_.begin(); iter != agentConnections_.end(); ++iter) {
            if (iter->first.agentConnection == agentConnection) {
                agentConnections_.erase(iter);
                break;
            }
        }
    }
    return ret;
}

bool AgentConnectionManager::IsConnectingTimeout(const AgentConnectionInfo &info)
{
    if (info.connectingTime == 0) {
        return false;
    }
    if (!info.agentConnection || info.agentConnection->GetConnectionState() != CONNECTION_STATE_CONNECTING) {
        return false;
    }
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now).count() - info.connectingTime;
    if (duration >= CONNECTING_TIMEOUT) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "connecting timeout, no reuse");
        return true;
    }
    return false;
}

ErrCode AgentConnectionManager::ValidateConnectAbilityInnerInput(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback, std::string &agentId) const
{
    if (connectCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null connectCallback");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    agentId = want.GetStringParam(AGENTID_KEY);
    if (agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty agentId");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    if (want.GetElement().GetBundleName().empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty bundleName");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    if (want.GetElement().GetAbilityName().empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty abilityName");
        return AAFwk::INVALID_PARAMETERS_ERR;
    }
    return ERR_OK;
}

AgentConnectionManager::AgentConnectionRecords::iterator AgentConnectionManager::FindConnectionByCallbackLocked(
    const sptr<AbilityConnectCallback> &connectCallback)
{
    return std::find_if(agentConnections_.begin(), agentConnections_.end(),
        [&connectCallback](const AgentConnectionRecord &record) {
            return std::find(record.second.begin(), record.second.end(), connectCallback) != record.second.end();
        });
}

ErrCode AgentConnectionManager::DisconnectConnectionRecord(const AgentConnectionRecord &connectionRecord)
{
    auto ret = AgentManagerClient::GetInstance().DisconnectAgentExtensionAbility(
        connectionRecord.first.agentConnection);
    if (ret == ERR_OK) {
        return ERR_OK;
    }

    TAG_LOGE(AAFwkTag::SER_ROUTER, "disconnect err:%{public}d", ret);
    RestoreConnectionRecord(connectionRecord);
    return ret;
}

void AgentConnectionManager::RestoreConnectionRecord(const AgentConnectionRecord &connectionRecord)
{
    std::lock_guard<std::mutex> lock(connectionsLock_);
    agentConnections_.push_back(connectionRecord);
}

AgentConnectionManager::AgentConnectionRecords::iterator AgentConnectionManager::FindReusableConnectionLocked(
    const std::string &agentId, const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback)
{
    for (auto iter = agentConnections_.begin(); iter != agentConnections_.end(); ++iter) {
        if (MatchConnection(agentId, want, connectCallback, *iter) && !IsConnectingTimeout(iter->first)) {
            return iter;
        }
    }
    return agentConnections_.end();
}

bool AgentConnectionManager::TryReuseConnectionLocked(AgentConnectionRecords::iterator connectionIter,
    const AAFwk::Want &want, const sptr<AbilityConnectCallback> &connectCallback, ErrCode &result)
{
    if (connectionIter == agentConnections_.end()) {
        return false;
    }
    auto &callbacks = connectionIter->second;
    callbacks.push_back(connectCallback);
    sptr<AgentConnection> agentConnection = connectionIter->first.agentConnection;
    if (!agentConnection) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "AgentConnection not exist");
        result = AAFwk::ERR_INVALID_CALLER;
        return true;
    }
    agentConnection->AddConnectCallback(connectCallback);
    if (agentConnection->GetConnectionState() == CONNECTION_STATE_CONNECTED) {
        AppExecFwk::ElementName element = want.GetElement();
        connectCallback->OnAbilityConnectDone(element, agentConnection->GetRemoteObject(),
            agentConnection->GetResultCode());
        result = ERR_OK;
        return true;
    }
    if (agentConnection->GetConnectionState() == CONNECTION_STATE_CONNECTING) {
        result = ERR_OK;
        return true;
    }
    TAG_LOGE(AAFwkTag::SER_ROUTER, "agentConnection disconnected");
    agentConnections_.erase(connectionIter);
    return false;
}

ErrCode AgentConnectionManager::ConnectAbilityInner(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback)
{
    std::string agentId;
    auto ret = ValidateConnectAbilityInnerInput(want, connectCallback, agentId);
    if (ret != ERR_OK) {
        return ret;
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "agentId: %{public}s, element: %{public}s/%{public}s/%{public}s",
        agentId.c_str(), want.GetElement().GetBundleName().c_str(),
        want.GetElement().GetModuleName().c_str(), want.GetElement().GetAbilityName().c_str());

    {
        std::lock_guard<std::mutex> lock(connectionsLock_);
        ErrCode reuseResult = ERR_OK;
        auto connectionIter = FindReusableConnectionLocked(agentId, want, connectCallback);
        if (TryReuseConnectionLocked(connectionIter, want, connectCallback, reuseResult)) {
            return reuseResult;
        }
    }
    return CreateConnection(want, connectCallback);
}
} // namespace AgentRuntime
} // namespace OHOS
