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

// 配置缺失或格式错误时只保留恢复框架的最小系统集合，采用 fail-closed 语义。
constexpr std::array<std::string_view, 6> RECOVERY_ESSENTIAL_BUNDLES = {
    "com.ohos.settings",
    "com.ohos.settingsdata",
    "com.ohos.systemui",
    "com.ohos.sceneboard",
    "com.ohos.launcher",
    "com.ohos.systemdialog",
};

std::unordered_set<std::string> GetBuiltInRecoveryEssentialBundles()
{
    // 内置集合只作为配置缺失或损坏时的启动兜底，不覆盖有效的产品配置。
    std::unordered_set<std::string> bundles;
    for (const auto bundleName : RECOVERY_ESSENTIAL_BUNDLES) {
        bundles.emplace(bundleName);
    }
    return bundles;
}

const std::unordered_set<std::string> &GetRecoveryEssentialBundleSet()
{
    // 每个进程只解析一次配置，避免在高频启动检查中重复访问文件系统。
    static const std::unordered_set<std::string> bundles = [] {
        auto builtInBundles = GetBuiltInRecoveryEssentialBundles();
        nlohmann::json config;
        // 优先读取产品定制路径，失败时回退系统默认路径和内置集合。
        if (!JsonUtils::GetInstance().LoadConfiguration(SYSTEM_RECOVERY_APP_WHITELIST_CONFIG, config,
            SYSTEM_RECOVERY_APP_WHITELIST_DEFAULT_CONFIG)) {
            return builtInBundles;
        }
        // 配置根节点必须是 Bundle 名称数组，格式异常时保持可启动的最小集合。
        if (!config.contains(SYSTEM_RECOVERY_APP_WHITELIST_KEY) ||
            !config.at(SYSTEM_RECOVERY_APP_WHITELIST_KEY).is_array()) {
            return builtInBundles;
        }

        std::unordered_set<std::string> configuredBundles;
        for (const auto &item : config.at(SYSTEM_RECOVERY_APP_WHITELIST_KEY)) {
            // 任意非法条目都会使整份配置失效，避免只加载半份白名单。
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

// 模式参数在 init 的 ReadConfig 阶段已经可用，并在本次启动周期内保持不变。
SystemRecoveryModePolicy::SystemRecoveryModePolicy()
    : enabled_(system::GetParameter(SYSTEM_RECOVERY_MODE_PARAMETER, "") == SYSTEM_RECOVERY_MODE_VALUE)
{
}

bool SystemRecoveryModePolicy::IsEnabled() const
{
    return enabled_;
}

bool SystemRecoveryModePolicy::IsRecoveryEssentialBundle(const std::string &bundleName) const
{
    const auto &essentialBundles = GetRecoveryEssentialBundleSet();
    return essentialBundles.find(bundleName) != essentialBundles.end();
}

bool SystemRecoveryModePolicy::ShouldBlockBundle(const std::string &bundleName) const
{
    if (!IsEnabled()) {
        return false;
    }
    return bundleName.empty() || !IsRecoveryEssentialBundle(bundleName);
}

} // namespace AAFwk
} // namespace OHOS
