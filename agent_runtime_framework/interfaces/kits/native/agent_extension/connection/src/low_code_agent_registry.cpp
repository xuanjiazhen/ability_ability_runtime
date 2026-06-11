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

#include "low_code_agent_registry.h"

#include <mutex>
#include <set>

namespace OHOS {
namespace AgentRuntime {
namespace {
std::mutex g_lowCodeAgentMutex;
std::set<std::string> g_activeLowCodeAgents;
} // namespace

bool LowCodeAgentRegistry::MarkActive(const std::string &agentId)
{
    std::lock_guard<std::mutex> lock(g_lowCodeAgentMutex);
    return g_activeLowCodeAgents.emplace(agentId).second;
}

void LowCodeAgentRegistry::MarkComplete(const std::string &agentId)
{
    std::lock_guard<std::mutex> lock(g_lowCodeAgentMutex);
    g_activeLowCodeAgents.erase(agentId);
}
} // namespace AgentRuntime
} // namespace OHOS
