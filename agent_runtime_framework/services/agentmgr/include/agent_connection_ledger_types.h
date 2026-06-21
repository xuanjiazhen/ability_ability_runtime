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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_CONNECTION_LEDGER_TYPES_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_CONNECTION_LEDGER_TYPES_H

#include <chrono>
#include <set>
#include <string>
#include <vector>

#include "agent_card.h"
#include "agent_host_connection_types.h"
#include "want.h"

namespace OHOS {
namespace AgentRuntime {
using AgentPreflightClock = std::chrono::steady_clock;
using AgentPreflightTimePoint = AgentPreflightClock::time_point;

struct TrackedConnectionRecord {
    int32_t callerUid = 0;
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    sptr<IRemoteObject> callerRemote = nullptr;
    sptr<IRemoteObject::DeathRecipient> deathRecipient = nullptr;
    AgentHostKey hostKey;
    std::string agentId;
    std::set<std::string> lowCodeAgentIds;
    std::string originalIdentity;
    long verificationNonce = 0;
    bool isLowCode = false;
    bool isDisconnecting = false;
    bool hasQuota = false;
};

struct AgentQuotaKey {
    AgentHostKey hostKey;
    std::string agentId;
    bool isLowCode = false;
};

bool operator<(const AgentQuotaKey &left, const AgentQuotaKey &right);

struct AgentDisconnectRequest {
    sptr<IRemoteObject> callerRemote = nullptr;
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    AgentHostKey hostKey;
    std::set<std::string> lowCodeAgentIds;
    bool isLowCode = false;
    bool alreadyDisconnecting = false;
};

struct LowCodeDisconnectTarget {
    std::set<std::string> agentIds;
    sptr<AgentHostConnection> hostConnection = nullptr;
};

struct LowCodeDisconnectRequest {
    sptr<AAFwk::IAbilityConnection> callerConnection = nullptr;
    std::vector<LowCodeDisconnectTarget> targets;
    AppExecFwk::ElementName element;
    bool notifyCallerDisconnect = false;
    bool alreadyDisconnecting = false;
};

struct LowCodeCompleteRequest {
    AgentHostKey hostKey;
    std::string agentId;
    sptr<AgentHostConnection> hostConnection = nullptr;
};

struct AgentCallerDeathRequest {
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    AgentHostKey hostKey;
    std::vector<LowCodeDisconnectTarget> lowCodeTargets;
};

struct AgentConnectPreflightRecord {
    int32_t callerUid = 0;
    AAFwk::Want connectWant;
    std::string agentId;
    AgentCard card;
    AgentPreflightTimePoint expiresAt;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_CONNECTION_LEDGER_TYPES_H
