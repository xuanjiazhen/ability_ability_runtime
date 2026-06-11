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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_HOST_CONNECTION_TYPES_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_HOST_CONNECTION_TYPES_H

#include <memory>
#include <map>
#include <string>

#include "agent_host_connection.h"
#include "agent_low_code_record.h"

namespace OHOS {
namespace AgentRuntime {
/**
 * @struct AgentOwnerKey
 * @brief Maps a logical low-code agent owner to the shared host session that currently serves it.
 */
struct AgentOwnerKey {
    AgentCallerIdentity callerIdentity;
    std::string agentId;

    AgentOwnerKey() = default;
    AgentOwnerKey(const AgentCallerIdentity &callerIdentityValue, const std::string &agentIdValue)
        : callerIdentity(callerIdentityValue), agentId(agentIdValue)
    {}
    AgentOwnerKey(int32_t callerUid, const std::string &agentIdValue)
        : callerIdentity(callerUid), agentId(agentIdValue)
    {}
};

using AgentHostSession = AgentLowCodeRecord;

struct AgentHostSessionRecord {
    AgentHostKey key;
    std::shared_ptr<AgentHostSession> session = nullptr;
};

struct AgentOwnerRecord {
    AgentOwnerKey key;
    std::shared_ptr<AgentHostSession> session = nullptr;
};

/**
 * @struct AgentConnectPlan
 * @brief Carries all intermediate state needed to either reuse or create a low-code host connection.
 */
struct AgentConnectPlan {
    AgentHostKey hostKey;
    int32_t hostUid = 0;
    AgentCallerIdentity callerIdentity;
    sptr<AgentHostConnection> hostConnection;
    sptr<IRemoteObject> callerRemote;
    AppExecFwk::ElementName cachedElement;
    sptr<IRemoteObject> cachedRemoteObject;
    int32_t cachedResultCode = ERR_OK;
    bool notifyExistingConnection = false;
    bool needRealConnect = false;
    bool registeredTrackedConnection = false;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_HOST_CONNECTION_TYPES_H
