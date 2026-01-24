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

#include "ability_connection.h"
#include "ability_manager_errors.h"
#include "agent_extension_connection_constants.h"
#include "agent_manager_client.h"
#include "hilog_tag_wrapper.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr int64_t CONNECTING_TIMEOUT = 30000;
}

AgentConnectionInfo::AgentConnectionInfo(const std::string &agentId,
    const AAFwk::Operation &connectReceiver, const sptr<AbilityConnection> &abilityConnection)
    : agentId(agentId), connectReceiver(connectReceiver), abilityConnection(abilityConnection)
{}

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

// ErrCode AgentConnectionManager::DisconnectAgentExtensionAbility(const sptr<AbilityConnection> &connection)
// {
//     if (connection == nullptr) {
//         TAG_LOGE(AAFwkTag::SER_ROUTER, "null connection");
//         return ERR_INVALID_VALUE;
//     }

//     TAG_LOGD(AAFwkTag::SER_ROUTER, "connection: %{public}p", connection.GetRefPtr());

//     std::lock_guard<std::mutex> lock(connectionsLock_);
//     auto item = abilityConnections_.begin();

//     while (item != abilityConnections_.end()) {
//         if (item->first.abilityConnection != connection) {
//             item++;
//             continue;
//         }

//         TAG_LOGD(AAFwkTag::SER_ROUTER, "found connection, callback size: %{public}zu", item->second.size());
//         sptr<AbilityConnection> abilityConnection = item->first.abilityConnection;
//         std::vector<sptr<AbilityConnectCallback>> callbacks = item->second;
//         // Save connectReceiver info before erasing
//         const AAFwk::Operation connectReceiver = item->first.connectReceiver;
//         item = abilityConnections_.erase(item);

//         // Notify all callbacks and disconnect
//         for (auto &callback : callbacks) {
//             AppExecFwk::ElementName element;
//             element.SetBundleName(connectReceiver.GetBundleName());
//             element.SetModuleName(connectReceiver.GetModuleName());
//             element.SetAbilityName(connectReceiver.GetAbilityName());
//             callback->OnAbilityDisconnectDone(element, ERR_OK);
//             abilityConnection->RemoveConnectCallback(callback);
//         }

//         TAG_LOGI(AAFwkTag::SER_ROUTER, "disconnectAbility");
//         auto ret = AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(abilityConnection);
//         if (ret != ERR_OK) {
//             TAG_LOGE(AAFwkTag::SER_ROUTER, "disconnect err:%{public}d", ret);
//             return ret;
//         }
//         return ERR_OK;
//     }

//     TAG_LOGE(AAFwkTag::SER_ROUTER, "connection not found");
//     return AAFwk::CONNECTION_NOT_EXIST;
// }

// bool AgentConnectionManager::RemoveConnection(const sptr<AbilityConnection> &connection)
// {
//     std::lock_guard<std::mutex> lock(connectionsLock_);
//     TAG_LOGD(AAFwkTag::SER_ROUTER, "abilityConnectionsSize: %{public}zu", abilityConnections_.size());

//     bool isDisconnect = false;
//     auto iter = abilityConnections_.begin();
//     while (iter != abilityConnections_.end()) {
//         AgentConnectionInfo connectionInfo = iter->first;
//         if (connectionInfo.abilityConnection == connection) {
//             TAG_LOGD(AAFwkTag::SER_ROUTER, "Remove connection");
//             iter = abilityConnections_.erase(iter);
//             isDisconnect = true;
//         } else {
//             ++iter;
//         }
//     }
//     return isDisconnect;
// }

// void AgentConnectionManager::ReportConnectionLeakEvent(const int pid, const int tid)
// {
//     TAG_LOGD(AAFwkTag::SER_ROUTER, "pid:%{public}d, tid:%{public}d", pid, tid);
//     // TODO: Implement HiChecker reporting if needed
//     (void)pid;
//     (void)tid;
// }

// bool AgentConnectionManager::DisconnectNonexistentService(const AppExecFwk::ElementName &element,
//     const sptr<AbilityConnection> &connection)
// {
//     bool exit = false;
//     std::map<AgentConnectionInfo, std::vector<sptr<AbilityConnectCallback>>> abilityConnections;
//     {
//         std::lock_guard<std::mutex> lock(connectionsLock_);
//         abilityConnections = abilityConnections_;
//     }
//     TAG_LOGD(AAFwkTag::SER_ROUTER, "abilityConnectionsSize: %{public}zu", abilityConnections.size());

//     for (auto &&abilityConnection : abilityConnections) {
//         AgentConnectionInfo connectionInfo = abilityConnection.first;
//         if (connectionInfo.abilityConnection == connection &&
//             connectionInfo.connectReceiver.GetBundleName() == element.GetBundleName()) {
//             TAG_LOGD(AAFwkTag::SER_ROUTER, "find connection");
//             exit = true;
//             break;
//         }
//     }
//     if (!exit) {
//         TAG_LOGE(AAFwkTag::SER_ROUTER, "service need disconnect");
//         AAFwk::AbilityManagerClient::GetInstance()->DisconnectAbility(connection);
//         return true;
//     }
//     return false;
// }

bool AgentConnectionManager::IsConnectReceiverEqual(const AAFwk::Operation &connectReceiver,
    const AppExecFwk::ElementName &connectReceiverOther)
{
    return connectReceiver.GetBundleName() == connectReceiverOther.GetBundleName() &&
        connectReceiver.GetAbilityName() == connectReceiverOther.GetAbilityName();
}

void *AgentConnectionManager::GetAgentExtProxyPtr(const AAFwk::Want &want)
{
    sptr<IRemoteObject> agentExtProxySptr = want.GetRemoteObject(AGENTEXTENSIONHOSTPROXY_KEY);
    void *agentExtProxy = nullptr;
    if (agentExtProxySptr != nullptr) {
        agentExtProxy = agentExtProxySptr.GetRefPtr();
    }
    return agentExtProxy;
}

bool AgentConnectionManager::MatchConnection(const std::string &agentId, const AAFwk::Want &connectReceiver,
    const std::map<AgentConnectionInfo, std::vector<sptr<AbilityConnectCallback>>>::value_type &connection)
{
    // 1. Match by agentId
    if (agentId != connection.first.agentId) {
        return false;
    }

    // 2. Match by agentExtProxy
    void *agentExtProxy = GetAgentExtProxyPtr(connectReceiver);
    if (agentExtProxy != connection.first.agentExtProxy) {
        return false;
    }

    // 3. Match by connectReceiver (bundleName, moduleName, abilityName)
    const AAFwk::Operation &storedReceiver = connection.first.connectReceiver;

    if (connectReceiver.GetElement().GetBundleName() != storedReceiver.GetBundleName()) {
        return false;
    }
    if (connectReceiver.GetElement().GetModuleName() != storedReceiver.GetModuleName()) {
        return false;
    }
    if (connectReceiver.GetElement().GetAbilityName() != storedReceiver.GetAbilityName()) {
        return false;
    }

    return true;
}

ErrCode AgentConnectionManager::CreateConnection(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "called");
    sptr<AbilityConnection> abilityConnection = sptr<AbilityConnection>::MakeSptr();
    if (abilityConnection == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null abilityConnection");
        return AAFwk::ERR_INVALID_CALLER;
    }

    abilityConnection->AddConnectCallback(connectCallback);
    abilityConnection->SetConnectionState(CONNECTION_STATE_CONNECTING);

    // Extract agentId from Want
    std::string agentId = want.GetStringParam(AGENTID_KEY);

    // Create AgentConnectionInfo
    AAFwk::Operation connectReceiver = want.GetOperation();
    AgentConnectionInfo connectionInfo(agentId, connectReceiver, abilityConnection);
    connectionInfo.SetAgentExtProxyPtr(GetAgentExtProxyPtr(want));
    connectionInfo.RecordConnectingTime();

    {
        std::lock_guard<std::mutex> lock(connectionsLock_);
        std::vector<sptr<AbilityConnectCallback>> callbacks;
        callbacks.push_back(connectCallback);
        abilityConnections_[connectionInfo] = callbacks;
    }

    ErrCode ret = AgentManagerClient::GetInstance().ConnectAgentExtensionAbility(want, abilityConnection);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "error:%{public}d", ret);
        // Remove the failed connection from map
        std::lock_guard<std::mutex> lock(connectionsLock_);
        abilityConnections_.erase(connectionInfo);
    }
    return ret;
}

bool AgentConnectionManager::IsConnectingTimeout(const AgentConnectionInfo &info)
{
    if (info.connectingTime == 0) {
        return false;
    }
    if (!info.abilityConnection || info.abilityConnection->GetConnectionState() != CONNECTION_STATE_CONNECTING) {
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

ErrCode AgentConnectionManager::ConnectAbilityInner(const AAFwk::Want &want,
    const sptr<AbilityConnectCallback> &connectCallback)
{
    if (connectCallback == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null connectCallback");
        return AAFwk::ERR_INVALID_CALLER;
    }

    // Extract agentId from Want
    std::string agentId = want.GetStringParam(AGENTID_KEY);
    if (agentId.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "empty agentId");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "agentId: %{public}s, element: %{public}s/%{public}s/%{public}s",
        agentId.c_str(), want.GetElement().GetBundleName().c_str(),
        want.GetElement().GetModuleName().c_str(), want.GetElement().GetAbilityName().c_str());

    sptr<AbilityConnection> abilityConnection;

    {
        std::lock_guard<std::mutex> lock(connectionsLock_);

        // Search for existing connection
        auto connectionIter = abilityConnections_.end();
        for (auto iter = abilityConnections_.begin(); iter != abilityConnections_.end(); ++iter) {
            if (MatchConnection(agentId, want, *iter) && !IsConnectingTimeout(iter->first)) {
                connectionIter = iter;
                break;
            }
        }

        if (connectionIter != abilityConnections_.end()) {
            // Found existing connection
            std::vector<sptr<AbilityConnectCallback>> &callbacks = connectionIter->second;
            callbacks.push_back(connectCallback);
            abilityConnection = connectionIter->first.abilityConnection;

            if (!abilityConnection) {
                TAG_LOGE(AAFwkTag::SER_ROUTER, "AbilityConnection not exist");
                return AAFwk::ERR_INVALID_CALLER;
            }

            TAG_LOGI(AAFwkTag::SER_ROUTER, "abilityConnectionsSize: %{public}zu, ConnectionState: %{public}d",
                abilityConnections_.size(), abilityConnection->GetConnectionState());

            abilityConnection->AddConnectCallback(connectCallback);
            TAG_LOGD(AAFwkTag::SER_ROUTER, "abilityConnection exist, callbackSize:%{public}zu", callbacks.size());

            if (abilityConnection->GetConnectionState() == CONNECTION_STATE_CONNECTED) {
                AppExecFwk::ElementName element = want.GetElement();
                connectCallback->OnAbilityConnectDone(element, abilityConnection->GetRemoteObject(),
                    abilityConnection->GetResultCode());
                return ERR_OK;
            }
            if (abilityConnection->GetConnectionState() == CONNECTION_STATE_CONNECTING) {
                return ERR_OK;
            }
            TAG_LOGE(AAFwkTag::SER_ROUTER, "abilityConnection disconnected");
            abilityConnections_.erase(connectionIter);
        }
    }

    // Create connection outside of lock
    return CreateConnection(want, connectCallback);
}
} // namespace AgentRuntime
} // namespace OHOS
