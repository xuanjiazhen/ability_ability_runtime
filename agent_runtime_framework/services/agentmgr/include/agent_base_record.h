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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_BASE_RECORD_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_BASE_RECORD_H

#include <cstdint>
#include <string>

namespace OHOS {
namespace AgentRuntime {
enum class AgentCardType : int32_t;

/**
 * Identifies one caller instance for ownership and quota.
 * instanceKey is kept exactly as AppMgr reports it; an empty value means the default app instance.
 */
struct AgentCallerIdentity {
    int32_t uid = 0;
    std::string instanceKey;

    AgentCallerIdentity() = default;
    AgentCallerIdentity(int32_t uidValue, const std::string &instanceKeyValue = "")
        : uid(uidValue), instanceKey(instanceKeyValue)
    {}

    bool operator<(const AgentCallerIdentity &other) const
    {
        if (uid != other.uid) {
            return uid < other.uid;
        }
        return instanceKey < other.instanceKey;
    }

    bool operator==(const AgentCallerIdentity &other) const
    {
        return uid == other.uid && instanceKey == other.instanceKey;
    }

    bool IsValid() const
    {
        return uid != 0;
    }
};

/**
 * Identifies the resolved target side used to decide whether a backing agent session can be shared.
 * Low-code targets keep agentId as logical ownership; standard targets include agentId in the target identity.
 */
struct AgentTargetIdentity {
    AgentCardType type = static_cast<AgentCardType>(0);
    int32_t targetUid = 0;
    std::string targetInstanceKey;
    std::string agentId;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;

    bool operator==(const AgentTargetIdentity &other) const
    {
        return type == other.type && targetUid == other.targetUid &&
            targetInstanceKey == other.targetInstanceKey && agentId == other.agentId &&
            bundleName == other.bundleName && abilityName == other.abilityName &&
            (moduleName.empty() || other.moduleName.empty() || moduleName == other.moduleName);
    }
};

struct AgentTargetInfo {
    int32_t userId = 0;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
};

class AgentBaseRecord {
public:
    explicit AgentBaseRecord(const AgentTargetInfo &target);
    virtual ~AgentBaseRecord() = default;

    const AgentTargetInfo &GetTarget() const;
    bool MatchesTarget(const AgentTargetInfo &target) const;

    static bool IsSameTarget(const AgentTargetInfo &left, const AgentTargetInfo &right);

private:
    AgentTargetInfo target_;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_AGENT_BASE_RECORD_H
