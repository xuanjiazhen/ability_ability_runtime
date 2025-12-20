/*
 * Copyright (c) 2025  Huawei Device Co., Ltd.
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

#include "page_scaling_manager.h"

#include <string>
#include <vector>
#include <mutex>

#include "ability_context.h"
#include "match_tag_wrapper.h"
#include "page_data_manager.h"
#include "ui_content.h"
#include "window.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
std::vector<std::string> PAGE_CONFIG_KEYS = {
    "apps_page_level_config_dpi",
    "apps_page_level_config_hideStatusBar",
    "apps_page_level_config_videoShrink",
    "apps_page_level_config_verticalDisplay"
};
}

PageScalingManager::PageScalingManager() {}

void PageScalingManager::Initialize(std::string configJson, const sptr<Rosen::Window>& window,
    const std::string &pageName)
{
    TAG_LOGW(AAFwkTag::ABILITY, "czx intialize mgr");
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return;
    }

    config_.page.pageName = pageName;
    ParseWhitelistConfig(pageName, configJson, config);
    RegisterPageChangeListeners(window, config_);
}


void PageScalingManager::ParseWhitelistConfig(const std::string &pageName,
    const std::string &configJson, PageScalingConfig &config)
{
    auto parseResult = nlohmann::json::parse(configJson, nullptr, false);
    if (parseResult.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITY, "ParseWhitelistConfig failed, configJson: %{public}s", configJson.c_str());
        return;
    }
    
    nlohmann::json jsonObj = parseResult;
    if (!jsonObj.is_array()) {
        TAG_LOGE(AAFwkTag::ABILITY, "configJson is not an array, configJson: %{public}s", configJson.c_str());
        return;
    }
    
    // 遍历数组，查找匹配pageName的配置
    for (const auto& item : jsonObj) {
        if (!item.contains("page") || !item["page"].is_object()) {
            continue;
        }
        // 获取页面配置
        std::string configPageName = item["page"].value("pageName", "");
        std::string patternType = item["page"].value("patternType", "");
        // 根据patternType判断是否匹配
        if (!comparePage(pageName, configPageName, patternType)) {
            continue;
        }
        
        TAG_LOGI(AAFwkTag::ABILITY, "Found matching config");
        
        // 设置基本配置信息
        config.page.pageName = configPageName;
        config.page.patternType = patternType;
        
        // 解析DPI配置
        if (item.contains("dpi") && item["dpi"].is_array()) {
            for (const auto& dpiItem : item["dpi"]) {
                DpiConfig dpiConfig;
                dpiConfig.condition = dpiItem.value("condition", "");
                std::string valueStr = dpiItem.value("value", "0");
                dpiConfig.value = std::stof(valueStr);
                TAG_LOGI(AAFwkTag::ABILITY, "DPI condition: %{public}s, value: %{public}f", 
                    dpiConfig.condition.c_str(), dpiConfig.value);
                config.dpi.push_back(dpiConfig);
            }
        }
        
        // 解析hideStatusBar配置
        if (item.contains("hideStatusBar") && item["hideStatusBar"].is_array()) {
            for (const auto& hideItem : item["hideStatusBar"]) {
                HideStatusBarConfig hideConfig;
                std::string valueStr = hideItem.value("value", "false");
                hideConfig.value = !(valueStr == "true");
                TAG_LOGI(AAFwkTag::ABILITY, "HideStatusBar value: %{public}d", hideConfig.value);
                config.hideStatusBar.push_back(hideConfig);
            }
        }
        
        // 解析videoShrink配置
        if (item.contains("videoShrink") && item["videoShrink"].is_array()) {
            for (const auto& videoItem : item["videoShrink"]) {
                VideoShrinkConfig videoConfig;
                std::string valueStr = videoItem.value("value", "false");
                videoConfig.value = (valueStr == "true");
                TAG_LOGI(AAFwkTag::ABILITY, "VideoShrink value: %{public}d", videoConfig.value);
                config.videoShrink.push_back(videoConfig);
            }
        }
        
        // 解析verticalDisplay配置
        if (item.contains("verticalDisplay") && item["verticalDisplay"].is_array()) {
            for (const auto& verticalItem : item["verticalDisplay"]) {
                VerticalDisplayConfig verticalConfig;
                verticalConfig.condition = verticalItem.value("condition", "");
                verticalConfig.value = verticalItem.value("value", "");
                TAG_LOGI(AAFwkTag::ABILITY, "VerticalDisplay condition: %{public}s, value: %{public}s", 
                    verticalConfig.condition.c_str(), verticalConfig.value.c_str());
                config.verticalDisplay.push_back(verticalConfig);
            }
        }
    }
}

void PageScalingManager::NotifyPageChanged(char* targetPageName, uint32_t windowId)
{
    if (targetPageName == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "targetPage null");
        return;
    }

    auto window = Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return;
    }

    std::string targetPageNameStr(targetPageName);
    TAG_LOGI(AAFwkTag::ABILITY, "targetPageName: %{public}s", targetPageNameStr.c_str());

    for (const auto& key : PAGE_CONFIG_KEYS) {
        HandleNavigateChangeInternal(window, key, targetPageNameStr, config_);
    }
}

void PageScalingManager::RegisterPageChangeListeners(const sptr<Rosen::Window>& window, const PageScalingConfig &config)
{
    if (!window || !window->GetUIContent()) {
        TAG_LOGE(AAFwkTag::ABILITY, "window or uicontent ull");
        return;
    }
    window->GetUIContent()->RegisterNavigateChangeCallback(
        [this, window, config](const OHOS::Ace::NavigateChangeInfo& from, const OHOS::Ace::NavigateChangeInfo& to) {
        HandleNavigateChangeInternal(window, to.name, config);
    });
}

void PageScalingManager::HandleNavigateChangeInternal(const sptr<Rosen::Window>& window,
    const std::string& targetPageName, const PageScalingConfig& config)
{
    if (targetPageName == config.page.pageName) {
        ApplyConfig(window, config);
    }
}

bool PageScalingManager::ConditionCheck(const std::string& condition, Rosen::DisplayOrientation orientation, int32_t foldMode)
{
    // Check if it's a combo condition
    if (condition.find(",") != std::string::npos) {
        // Split condition by comma
        std::vector<std::string> condList;
        size_t pos = 0;
        std::string token;
        std::string conditionCopy = condition;
        while ((pos = conditionCopy.find(",")) != std::string::npos) {
            token = conditionCopy.substr(0, pos);
            condList.push_back(token);
            conditionCopy.erase(0, pos + 1);
        }
        condList.push_back(conditionCopy);
        
        if (condList.size() != 2) {
            return false;
        }
        
        const std::string& cond1 = condList[0];
        const std::string& cond2 = condList[1];
        
        // Check all possible combo conditions
        if (cond1 == "A" && cond2 == "D") {
            return (foldMode == Rosen::FoldDisplayMode::FOLD_DISPLAY_MODE_MAIN && 
                   orientation == Rosen::DisplayOrientation::PORTRAIT);
        } else if (cond1 == "B" && cond2 == "C") {
            return (foldMode == Rosen::FoldDisplayMode::FOLD_DISPLAY_MODE_FULL && 
                   orientation == Rosen::DisplayOrientation::LANDSCAPE);
        } else if (cond1 == "B" && cond2 == "D") {
            return (foldMode == Rosen::FoldDisplayMode::FOLD_DISPLAY_MODE_FULL && 
                   orientation == Rosen::DisplayOrientation::PORTRAIT);
        }
        return false;
    } else {
        // Single condition check
        if (condition == "A") {
            return (foldMode == Rosen::FoldDisplayMode::FOLD_DISPLAY_MODE_MAIN);
        } else if (condition == "B") {
            return (foldMode == Rosen::FoldDisplayMode::FOLD_DISPLAY_MODE_FULL);
        } else if (condition == "C") {
            return (orientation == Rosen::DisplayOrientation::LANDSCAPE);
        } else if (condition == "D") {
            return (orientation == Rosen::DisplayOrientation::PORTRAIT);
        }
        return false;
    }
}

void PageScalingManager::ApplyConfig(const sptr<Rosen::Window>& window, const PageScalingConfig& config)
{
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "ApplyConfig: window is null");
        return;
    }

    // Get display information
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(window->GetDisplayId());
    if (display == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "ApplyConfig: display is null");
        return;
    }
    
    auto foldDisplayMode = display->GetFoldDisplayMode();
    auto displayInfo = display->GetDisplayInfo();
    auto orientation = displayInfo.orientation;
    
    // 1. Handle DPI configuration
    for (const auto& dpiConfig : config.dpi) {
        const std::string& condition = dpiConfig.condition;
        float dpiValue = dpiConfig.value;
        
        if (ConditionCheck(condition, orientation, foldDisplayMode)) {
            window->SetCustomDensity(dpiValue);
            TAG_LOGI(AAFwkTag::ABILITY, "ApplyConfig: SetCustomDensity %{public}f for condition %{public}s", 
                dpiValue, condition.c_str());
            break;
        }
    }
    
    // 2. Handle verticalDisplay configuration
    for (const auto& verticalConfig : config.verticalDisplay) {
        const std::string& condition = verticalConfig.condition.orientation;
        const std::string& value = verticalConfig.value;
        
        if (ConditionCheck(condition, orientation, foldDisplayMode)) {
            // Apply verticalDisplay configuration
            // TODO: Implement verticalDisplay logic based on value
            TAG_LOGI(AAFwkTag::ABILITY, "ApplyConfig: verticalDisplay condition matched %{public}s, value: %{public}s", 
                condition.c_str(), value.c_str());
            break;
        }
    }
}

bool PageScalingManager::comparePage(const std::string &pageName, const std::string &targetPageName, std::string &patternType)
{
    if (patternType == "equals") {
        return pageName == targetPageName;
    }
    if (patternType == "prefix") {
        return pageName.find(targetPageName) == 0;
    }
    TAG_LOGE(AAFwkTag::ABILITY, "unknown patternType");
    return false;
}
} // namespace AppExecFwk
} // namespace OHOS