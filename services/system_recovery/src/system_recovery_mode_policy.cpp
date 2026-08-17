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

struct RecoveryAppConfig {
    // 配置异常时不整体放行系统应用，避免扩大恢复模式的可启动范围。
    bool allowSystemApps = false;
    std::unordered_set<std::string> bundleNames;
};

RecoveryAppConfig GetBuiltInRecoveryAppConfig()
{
    // 内置集合只作为配置缺失或损坏时的启动兜底，不覆盖有效的产品配置。
    RecoveryAppConfig config;
    for (const auto bundleName : RECOVERY_ESSENTIAL_BUNDLES) {
        config.bundleNames.emplace(bundleName);
    }
    return config;
}

const RecoveryAppConfig &GetRecoveryAppConfig()
{
    // 每个进程只解析一次配置，避免在高频启动检查中重复访问文件系统。
    static const RecoveryAppConfig recoveryConfig = [] {
        auto builtInConfig = GetBuiltInRecoveryAppConfig();
        nlohmann::json jsonConfig;
        // 优先读取产品定制路径，失败时回退系统默认路径和内置集合。
        if (!JsonUtils::GetInstance().LoadConfiguration(SYSTEM_RECOVERY_APP_WHITELIST_CONFIG, jsonConfig,
            SYSTEM_RECOVERY_APP_WHITELIST_DEFAULT_CONFIG)) {
            return builtInConfig;
        }
        // 系统应用开关和 Bundle 数组类型必须正确，格式异常时整体回退到最小集合。
        if (!jsonConfig.contains(SYSTEM_RECOVERY_ALLOW_SYSTEM_APPS_KEY) ||
            !jsonConfig.at(SYSTEM_RECOVERY_ALLOW_SYSTEM_APPS_KEY).is_boolean() ||
            !jsonConfig.contains(SYSTEM_RECOVERY_APP_WHITELIST_KEY) ||
            !jsonConfig.at(SYSTEM_RECOVERY_APP_WHITELIST_KEY).is_array()) {
            return builtInConfig;
        }

        RecoveryAppConfig configuredAppConfig;
        configuredAppConfig.allowSystemApps = jsonConfig.at(SYSTEM_RECOVERY_ALLOW_SYSTEM_APPS_KEY).get<bool>();
        for (const auto &item : jsonConfig.at(SYSTEM_RECOVERY_APP_WHITELIST_KEY)) {
            // 任意非法条目都会使整份配置失效，避免只加载半份白名单。
            if (!item.is_string()) {
                return builtInConfig;
            }
            auto bundleName = item.get<std::string>();
            if (bundleName.empty()) {
                return builtInConfig;
            }
            configuredAppConfig.bundleNames.emplace(bundleName);
        }
        // Bundle 白名单不能为空，避免产品误配置后连恢复界面也无法启动。
        return configuredAppConfig.bundleNames.empty() ? builtInConfig : configuredAppConfig;
    }();
    return recoveryConfig;
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
    const auto &bundleNames = GetRecoveryAppConfig().bundleNames;
    return bundleNames.find(bundleName) != bundleNames.end();
}

bool SystemRecoveryModePolicy::ShouldBlockBundle(const std::string &bundleName, bool isSystemApp) const
{
    if (!IsEnabled()) {
        return false;
    }
    // 只有产品配置显式开启时才整体放行系统应用，不能仅依赖安装位置推断。
    if (isSystemApp && GetRecoveryAppConfig().allowSystemApps) {
        return false;
    }
    return bundleName.empty() || !IsRecoveryEssentialBundle(bundleName);
}

} // namespace AAFwk
} // namespace OHOS
