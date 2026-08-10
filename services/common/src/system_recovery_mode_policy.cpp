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

#include "system_recovery_mode_policy.h"

#include <array>
#include <string_view>
#include <unordered_set>

#include "json_utils.h"
#include "parameters.h"
#include "system_recovery_mode_constants.h"

namespace OHOS {
namespace AAFwk {
namespace {

// 产品白名单缺失或格式非法时使用的最小安全白名单，避免恢复界面无法启动。
constexpr std::array<std::string_view, 5> RECOVERY_ESSENTIAL_BUNDLES = {
    "com.ohos.settings",
    "com.ohos.systemui",
    "com.ohos.sceneboard",
    "com.ohos.launcher",
    "com.ohos.systemdialog",
};

std::unordered_set<std::string> GetBuiltInRecoveryEssentialBundles()
{
    std::unordered_set<std::string> bundles;
    for (const auto bundleName : RECOVERY_ESSENTIAL_BUNDLES) {
        bundles.emplace(bundleName);
    }
    return bundles;
}

const std::unordered_set<std::string> &GetRecoveryEssentialBundles()
{
    static const std::unordered_set<std::string> bundles = [] {
        auto builtInBundles = GetBuiltInRecoveryEssentialBundles();
        nlohmann::json config;
        if (!JsonUtils::GetInstance().LoadConfiguration(SYSTEM_RECOVERY_APP_WHITELIST_CONFIG, config,
            SYSTEM_RECOVERY_APP_WHITELIST_DEFAULT_CONFIG)) {
            return builtInBundles;
        }
        if (!config.contains(SYSTEM_RECOVERY_APP_WHITELIST_KEY) ||
            !config.at(SYSTEM_RECOVERY_APP_WHITELIST_KEY).is_array()) {
            return builtInBundles;
        }

        std::unordered_set<std::string> configuredBundles;
        for (const auto &item : config.at(SYSTEM_RECOVERY_APP_WHITELIST_KEY)) {
            if (!item.is_string()) {
                return builtInBundles;
            }
            auto bundleName = item.get<std::string>();
            if (bundleName.empty()) {
                return builtInBundles;
            }
            configuredBundles.emplace(bundleName);
        }
        return configuredBundles.empty() ? builtInBundles : configuredBundles;
    }();
    return bundles;
}

} // namespace

SystemRecoveryModePolicy &SystemRecoveryModePolicy::GetInstance()
{
    static SystemRecoveryModePolicy policy;
    return policy;
}

bool SystemRecoveryModePolicy::IsRecoveryMode() const
{
    return system::GetParameter(SYSTEM_RECOVERY_MODE_PARAMETER, "") == SYSTEM_RECOVERY_MODE_VALUE;
}

bool SystemRecoveryModePolicy::IsRecoveryEssentialBundle(const std::string &bundleName) const
{
    const auto &essentialBundles = GetRecoveryEssentialBundles();
    return essentialBundles.find(bundleName) != essentialBundles.end();
}

bool SystemRecoveryModePolicy::ShouldBlockBundle(const std::string &bundleName) const
{
    if (!IsRecoveryMode()) {
        return false;
    }

    // 已进入恢复模式但目标为空时按未命中白名单处理。
    return bundleName.empty() || !IsRecoveryEssentialBundle(bundleName);
}

} // namespace AAFwk
} // namespace OHOS
