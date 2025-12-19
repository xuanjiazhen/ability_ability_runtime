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
        bool isMatch = comparePage(pageName, configPageName, patternType)
        
        if (!isMatch) {
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
                
                // 解析condition
                dpiConfig.condition = dpiItem.value("condition", "");
                
                // 解析value - 需要转换为float
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
                
                // 解析condition
                hideConfig.condition = hideItem.value("condition", "");
                std::string valueStr = hideItem.value("value", "false");
                hideConfig.value = !(valueStr == "true");
                TAG_LOGI(AAFwkTag::ABILITY, "HideStatusBar condition: %{public}s, value: %{public}d", 
                    hideConfig.condition.c_str(), hideConfig.value);
                config.hideStatusBar.push_back(hideConfig);
            }
        }
        
        // 解析videoShrink配置
        if (item.contains("videoShrink") && item["videoShrink"].is_array()) {
            for (const auto& videoItem : item["videoShrink"]) {
                VideoShrinkConfig videoConfig;
                
                videoConfig.condition = videoItem.value("condition", "");
                std::string valueStr = videoItem.value("value", "false");
                videoConfig.value = (valueStr == "true");
                TAG_LOGI(AAFwkTag::ABILITY, "VideoShrink condition: %{public}s, value: %{public}d", 
                    videoConfig.condition.c_str(), videoConfig.value);
                config.videoShrink.push_back(videoConfig);
            }
        }
        
        // 解析verticalDisplay配置
        if (item.contains("verticalDisplay") && item["verticalDisplay"].is_array()) {
            for (const auto& verticalItem : item["verticalDisplay"]) {
                VerticalDisplayConfig verticalConfig;
                
                // 解析condition
                if (verticalItem.contains("condition") && verticalItem["condition"].is_object()) {
                    verticalConfig.condition.orientation = 
                        verticalItem["condition"].value("orientation", "");
                } else {
                    verticalConfig.condition.orientation = 
                        verticalItem.value("condition", ""); 
                }
                
                // 解析value
                verticalConfig.value = verticalItem.value("value", "");
                TAG_LOGI(AAFwkTag::ABILITY, "VerticalDisplay condition: %{public}s, value: %{public}s", 
                    verticalConfig.condition.orientation.c_str(), verticalConfig.value.c_str());
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

void PageScalingManager::RegisterPageChangeListeners(const sptr<Rosen::Window>& window, const std::string &key, const PageScalingConfig &config)
{
    if (!window || !window->GetUIContent()) {
        TAG_LOGE(AAFwkTag::ABILITY, "window or uicontent ull");
        return;
    }
    window->GetUIContent()->RegisterNavigateChangeCallback(
        [this, window, key, config](const OHOS::Ace::NavigateChangeInfo& from, const OHOS::Ace::NavigateChangeInfo& to) {
        HandleNavigateChangeInternal(window, key, to.name, config);
    });
}

void PageScalingManager::HandleNavigateChangeInternal(const sptr<Rosen::Window>& window,
    const std::string& targetPageName, const PageScalingConfig& config)
{
    if (targetPageName == config.page.pageName) {
        ApplyConfig(window, config);
    }
}

void PageScalingManager::ApplyConfig(const sptr<Rosen::Window>& window, const PageScalingConfig& config)
{
    if (key == "apps_page_level_config_dpi") {
        for (const auto& dpiConfig : config.dpi) {
            if (dpiConfig.condition.empty()) {
                bool applyToSubwindow = false;
                float density = std::stof(dpiConfig.value);
                window->SetCustomDensity(density, applyToSubwindow);
                break;
            }
        }
    }
    else if (key == "apps_page_level_config_hideStatusBar") {
        for (const auto& hideConfig : config.hideStatusBar) {
            if (hideConfig.condition.empty()) {
                auto prop = window->GetSystemBarPropertyByType(Rosen::WindowType::WINDOW_TYPE_STATUS_BAR);
                bool hideValue = (hideConfig.value == true);
                prop.enable_ = !hideValue;
                window->SetSystemBarPropertyForPage(Rosen::WindowType::WINDOW_TYPE_STATUS_BAR, prop);
                break;
            }
        }
    }
    else if (key == "apps_page_level_config_videoShrink") {}
    else if (key == "apps_page_level_config_verticalDisplay") {}
    else {
        TAG_LOGE(AAFwkTag::ABILITY, "unknown key: %{public}s", key.c_str());
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