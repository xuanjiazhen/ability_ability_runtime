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

#ifndef OHOS_AGENT_RUNTIME_AGENT_CONNECTION_MANAGER_H
#define OHOS_AGENT_RUNTIME_AGENT_CONNECTION_MANAGER_H

#include <chrono>
#include <map>
#include <mutex>
#include <vector>

#include "ability_connect_callback.h"
#include "ability_connection.h"
#include "element_name.h"
#include "errors.h"
#include "operation.h"
#include "want.h"

namespace OHOS {
namespace AgentRuntime {
class AgentConnection;

struct AgentConnectionInfo {
    std::string agentId;
    AAFwk::Operation connectReceiver;
    sptr<AgentConnection> agentConnection;
    void *agentExtProxy = nullptr;
    int64_t connectingTime = 0;

    AgentConnectionInfo(const std::string &agentId, const AAFwk::Operation &connectReceiver,
        const sptr<AgentConnection> &agentConnection);

    void SetAgentExtProxyPtr(void *proxyPtr)
    {
        agentExtProxy = proxyPtr;
    }

    void RecordConnectingTime()
    {
        auto now = std::chrono::steady_clock::now().time_since_epoch();
        connectingTime = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
    }

    inline bool operator<(const AgentConnectionInfo &that) const
    {
        if (agentId < that.agentId) {
            return true;
        }
        if (agentExtProxy < that.agentExtProxy) {
            return true;
        }
        if (connectingTime < that.connectingTime) {
            return true;
        }
        if (connectReceiver.GetBundleName() < that.connectReceiver.GetBundleName()) {
            return true;
        }
        if (connectReceiver.GetBundleName() == that.connectReceiver.GetBundleName() &&
            connectReceiver.GetModuleName() < that.connectReceiver.GetModuleName()) {
            return true;
        }
        if (connectReceiver.GetBundleName() == that.connectReceiver.GetBundleName() &&
            connectReceiver.GetModuleName() == that.connectReceiver.GetModuleName() &&
            connectReceiver.GetAbilityName() < that.connectReceiver.GetAbilityName()) {
            return true;
        }
        if (connectReceiver.GetBundleName() == that.connectReceiver.GetBundleName() &&
            connectReceiver.GetModuleName() == that.connectReceiver.GetModuleName() &&
            connectReceiver.GetAbilityName() == that.connectReceiver.GetAbilityName() &&
            !(connectReceiver == that.connectReceiver)) {
            return true;
        }
        return false;
    }
};

class AgentConnection : public AbilityRuntime::AbilityConnection {
public:
    AgentConnection() = default;
    ~AgentConnection() override = default;

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;

private:
    std::mutex agentMutex_;
};

class AgentConnectionManager {
public:
    ~AgentConnectionManager() = default;
    AgentConnectionManager(const AgentConnectionManager &) = delete;
    AgentConnectionManager &operator=(const AgentConnectionManager &) = delete;

    static AgentConnectionManager &GetInstance();

    ErrCode ConnectAgentExtensionAbility(const AAFwk::Want &want,
        const sptr<AbilityRuntime::AbilityConnectCallback> &connectCallback);
    ErrCode DisconnectAgentExtensionAbility(const sptr<AbilityRuntime::AbilityConnectCallback> &connection);

    bool RemoveConnection(const sptr<AgentConnection> &connection);
    bool DisconnectNonexistentService(const AppExecFwk::ElementName &element,
        const sptr<AgentConnection> &connection);

private:
    AgentConnectionManager() = default;

    bool IsConnectReceiverEqual(const AAFwk::Operation &connectReceiver,
        const AppExecFwk::ElementName &connectReceiverOther);
    void *GetAgentExtProxyPtr(const AAFwk::Want &want);
    bool MatchConnection(const std::string &agentId, const AAFwk::Want &connectReceiver,
        const std::map<AgentConnectionInfo,
        std::vector<sptr<AbilityRuntime::AbilityConnectCallback>>>::value_type &connection);
    ErrCode CreateConnection(const AAFwk::Want &want,
        const sptr<AbilityRuntime::AbilityConnectCallback> &connectCallback);
    bool IsConnectingTimeout(const AgentConnectionInfo &info);
    ErrCode ConnectAbilityInner(const AAFwk::Want &want,
        const sptr<AbilityRuntime::AbilityConnectCallback> &connectCallback);

private:
    std::mutex connectionsLock_;
    std::map<AgentConnectionInfo, std::vector<sptr<AbilityRuntime::AbilityConnectCallback>>> agentConnections_;
};
} // namespace AgentRuntime
} // namespace OHOS

#endif // OHOS_AGENT_RUNTIME_AGENT_CONNECTION_MANAGER_H
