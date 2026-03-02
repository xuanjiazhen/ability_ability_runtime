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

#include "agent_module_checker.h"

#include <cstring>
#include <string>
#include <vector>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AgentRuntime {

namespace {
// API blocklist for agent - APIs that are not allowed to be used
constexpr const char *API_BLOCK_LIST[] = {
    "windowStage.createSubWindow",
    "windowStage.createSubWindowWithOptions",
    "window.createSubWindow",
};

bool MatchBlockPattern(const std::string& apiPath, const std::string& pattern)
{
    // Exact match
    if (apiPath == pattern) {
        return true;
    }
    // Prefix match with wildcard (e.g., "windowStage.createSubWindow.*")
    if (pattern.find('*') != std::string::npos) {
        std::string prefix = pattern.substr(0, pattern.find('*') - 1);
        if (apiPath.compare(0, prefix.size(), prefix) == 0) {
            return true;
        }
    }
    // Check if apiPath starts with pattern + "."
    if (apiPath.size() > pattern.size() &&
        apiPath.compare(0, pattern.size(), pattern) == 0 &&
        apiPath[pattern.size()] == '.') {
        return true;
    }
    return false;
}

bool CheckApiBlockList(const std::string& apiPath)
{
    for (const auto& pattern : API_BLOCK_LIST) {
        if (MatchBlockPattern(apiPath, pattern)) {
            TAG_LOGE(AAFwkTag::AGENT, "API is blocked: %{public}s, pattern: %{public}s",
                apiPath.c_str(), pattern);
            return true;
        }
    }
    return false;
}
} // namespace

bool AgentModuleChecker::CheckModuleLoadable(const char *moduleName,
    std::unique_ptr<ApiAllowListChecker> &apiAllowListChecker, bool isAppModule)
{
    TAG_LOGD(AAFwkTag::AGENT, "Check module loadable: %{public}s, isAppModule: %{public}d",
        moduleName, isAppModule);
    // For agent, we set up API blocklist checker
    if (apiAllowListChecker == nullptr) {
        apiAllowListChecker = std::make_unique<ApiAllowListChecker>([](const std::string& apiPath) {
            // Return true if API is allowed (not in blocklist)
            return !CheckApiBlockList(apiPath);
        });
    }
    return true;
}

bool AgentModuleChecker::DiskCheckOnly()
{
    return false;
}

} // namespace AgentRuntime
} // namespace OHOS
