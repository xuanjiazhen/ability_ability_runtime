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

#ifndef OHOS_ABILITY_RUNTIME_AGENT_ABILITY_UTIL_H
#define OHOS_ABILITY_RUNTIME_AGENT_ABILITY_UTIL_H

#include <cstddef>
#include <functional>
#include <string>
#include <vector>

#include "ability_info.h"
#include "extension_ability_info.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
namespace AgentKeyHash {
inline void Combine(std::size_t &seed, std::size_t value)
{
    seed ^= value + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}
} // namespace AgentKeyHash

struct ResolvedAgentExtKey {
    int32_t cardType = 0;
    int32_t userId = 0;
    int32_t appIndex = 0;
    std::string bundleName;
    std::string moduleName;
    std::string abilityName;
};

struct ResolvedAgentExtKeyEqual {
    bool operator()(const ResolvedAgentExtKey &left, const ResolvedAgentExtKey &right) const
    {
        return left.cardType == right.cardType && left.userId == right.userId && left.appIndex == right.appIndex &&
            left.bundleName == right.bundleName && left.moduleName == right.moduleName &&
            left.abilityName == right.abilityName;
    }
};

struct ResolvedAgentExtKeyHash {
    std::size_t operator()(const ResolvedAgentExtKey &key) const
    {
        std::size_t seed = std::hash<int32_t>()(key.cardType);
        AgentKeyHash::Combine(seed, std::hash<int32_t>()(key.userId));
        AgentKeyHash::Combine(seed, std::hash<int32_t>()(key.appIndex));
        AgentKeyHash::Combine(seed, std::hash<std::string>()(key.bundleName));
        AgentKeyHash::Combine(seed, std::hash<std::string>()(key.moduleName));
        AgentKeyHash::Combine(seed, std::hash<std::string>()(key.abilityName));
        return seed;
    }
};

struct StandardAgentInstanceKey {
    ResolvedAgentExtKey extKey;
    std::string agentId;
};

struct StandardAgentInstanceKeyEqual {
    bool operator()(const StandardAgentInstanceKey &left, const StandardAgentInstanceKey &right) const
    {
        return ResolvedAgentExtKeyEqual()(left.extKey, right.extKey) && left.agentId == right.agentId;
    }
};

struct StandardAgentInstanceKeyHash {
    std::size_t operator()(const StandardAgentInstanceKey &key) const
    {
        std::size_t seed = ResolvedAgentExtKeyHash()(key.extKey);
        AgentKeyHash::Combine(seed, std::hash<std::string>()(key.agentId));
        return seed;
    }
};

struct LowCodeHostKey {
    ResolvedAgentExtKey extKey;
};

struct LowCodeHostKeyEqual {
    bool operator()(const LowCodeHostKey &left, const LowCodeHostKey &right) const
    {
        return ResolvedAgentExtKeyEqual()(left.extKey, right.extKey);
    }
};

struct LowCodeHostKeyHash {
    std::size_t operator()(const LowCodeHostKey &key) const
    {
        return ResolvedAgentExtKeyHash()(key.extKey);
    }
};

struct LowCodeAgentKey {
    LowCodeHostKey hostKey;
    std::string agentId;
};

struct LowCodeAgentKeyEqual {
    bool operator()(const LowCodeAgentKey &left, const LowCodeAgentKey &right) const
    {
        return LowCodeHostKeyEqual()(left.hostKey, right.hostKey) && left.agentId == right.agentId;
    }
};

struct LowCodeAgentKeyHash {
    std::size_t operator()(const LowCodeAgentKey &key) const
    {
        std::size_t seed = LowCodeHostKeyHash()(key.hostKey);
        AgentKeyHash::Combine(seed, std::hash<std::string>()(key.agentId));
        return seed;
    }
};

class AgentAbilityUtil final {
public:
    static ResolvedAgentExtKey BuildResolvedAgentExtKey(int32_t cardType, int32_t userId, int32_t appIndex,
        const AppExecFwk::ExtensionAbilityInfo &extensionInfo)
    {
        ResolvedAgentExtKey key;
        key.cardType = cardType;
        key.userId = userId;
        key.appIndex = appIndex;
        key.bundleName = extensionInfo.bundleName;
        key.moduleName = extensionInfo.moduleName;
        key.abilityName = extensionInfo.name;
        return key;
    }
    static ResolvedAgentExtKey BuildResolvedAgentExtKey(int32_t cardType, int32_t userId, int32_t appIndex,
        const AppExecFwk::AbilityInfo &abilityInfo)
    {
        ResolvedAgentExtKey key;
        key.cardType = cardType;
        key.userId = userId;
        key.appIndex = appIndex;
        key.bundleName = abilityInfo.bundleName;
        key.moduleName = abilityInfo.moduleName;
        key.abilityName = abilityInfo.name;
        return key;
    }
    static bool IsAgentExtensionType(AppExecFwk::ExtensionAbilityType extensionType);
    static bool IsAgentExtensionAbilityInfo(const AppExecFwk::AbilityInfo &abilityInfo);
    static bool IsAgentExtensionInfo(const AppExecFwk::ExtensionAbilityInfo &extensionInfo);
    static bool IsAtomicServiceAgentExtensionInfo(const AppExecFwk::ExtensionAbilityInfo &extensionInfo);
    static bool HasAtomicServiceAgentExtensionInfo(const std::vector<AppExecFwk::ExtensionAbilityInfo> &extensionInfos);
    static bool HasAgentOnlyParams(const Want &want);
    static int32_t CheckAgentConnectEntry(const Want &want, AppExecFwk::ExtensionAbilityType extensionType);
    static int32_t CheckConnectAgentResolvedTarget(
        AppExecFwk::ExtensionAbilityType requestType, const AppExecFwk::AbilityInfo &abilityInfo);
    static int32_t RejectAgentResolvedTarget(const AppExecFwk::AbilityInfo &abilityInfo, const char *entryName);
    static bool WantResolvesToAgentExtension(const Want &want, int32_t userId);
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_AGENT_ABILITY_UTIL_H
