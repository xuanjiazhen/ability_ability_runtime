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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_CONNECTION_RECORD_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_CONNECTION_RECORD_H

#include "ability_connect_callback_stub.h"
#include "agent_host_connection.h"
#include "agent_host_connection_types.h"
#include "agent_standard_record.h"
#include "element_name.h"

namespace OHOS {
namespace AgentRuntime {
/**
 * One caller-visible connection owner. Several records can share one backing standard session or low-code host.
 */
struct TrackedConnectionRecord {
    int32_t callerUid = 0;
    AgentCallerIdentity callerIdentity;
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    sptr<IRemoteObject> callerRemote = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
    AgentHostKey hostKey;
    bool isLowCode = false;
    bool countTowardsCallerLimit = true;
    bool isDisconnecting = false;
    StandardAgentKey standardKey;
};

/**
 * Prepared disconnect data captured under lock and consumed after lock release when calling AMS.
 */
struct AgentDisconnectPlan {
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    sptr<AgentHostConnection> hostConnection = nullptr;
    sptr<AAFwk::IAbilityConnection> callerConnection = nullptr;
    sptr<IRemoteObject> callerRemote = nullptr;
    AppExecFwk::ElementName cachedElement;
    int32_t cachedResultCode = ERR_OK;
    AgentHostKey hostKey;
    bool hasHostKey = false;
    bool notifyCallerDisconnect = false;
    bool isStandard = false;
    int32_t standardCallerUid = 0;
    AgentCallerIdentity standardCallerIdentity;
    StandardAgentKey standardKey;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_CONNECTION_RECORD_H
