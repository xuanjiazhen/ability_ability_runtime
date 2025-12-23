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

#include "ability_manager_errors.h"
#include "match_tag_wrapper.h"
#include "ui_content.h"
#include "window.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const uint8_t CONDITION_LIST_SIZE = 2;
}

PageScalingManager::PageScalingManager() {}

void PageScalingManager::Initialize(std::string configJson, const sptr<Rosen::Window>& window)
{
    TAG_LOGW(AAFwkTag::ABILITY, "czx intialize mgr");
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return;
    }

    ParseWhitelistConfig(configJson, configs_);
    
    // Create and register UI content listener
    uiContentListener_ = new WindowUIContentImpl(shared_from_this(), configs_, window);
    window->RegisterWindowUIContentListener(uiContentListener_);
}

void PageScalingManager::WindowUIContentImpl::OnUIContentCreate(OHOS::Ace::UIContent* uiContent)
{
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "OnUIContentCreate: uiContent null");
        return;
    }
    
    auto owner = owner_.lock();
    if (!owner) {
        TAG_LOGE(AAFwkTag::ABILITY, "OnUIContentCreate: owner has been destroyed");
        return;
    }
    
    TAG_LOGI(AAFwkTag::ABILITY, "OnUIContentCreate: uiContent created, registering navigate change callback");
    
    // Register navigate change callback to the created UI content
    uiContent->RegisterNavigateChangeCallback(
        [owner, this](const OHOS::Ace::NavigateChangeInfo& from, const OHOS::Ace::NavigateChangeInfo& to) {
            if (!window_) {
                TAG_LOGE(AAFwkTag::ABILITY, "OnUIContentCreate: window not found");
                return;
            }
            owner->HandleNavigateChangeInternal(window_, to.name, config_);
        });
}

void PageScalingManager::ParseWhitelistConfig(const std::string &configJson, std::vector<PageScalingConfig> &configs)
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
    
    // 遍历数组，保存所有配置
    for (const auto& item : jsonObj) {
        if (!item.contains("page") || !item["page"].is_object()) {
            continue;
        }
        // 获取页面配置
        std::string configPageName = item["page"].value("pageName", "");
        std::string patternType = item["page"].value("patternType", "");
        
        // 创建新的配置对象
        PageScalingConfig config;
        config.page.pageName = configPageName;
        config.page.patternType = patternType;
        
        TAG_LOGI(AAFwkTag::ABILITY, "Found config for page %{public}s with pattern %{public}s", 
            configPageName.c_str(), patternType.c_str());
        
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
        
        // 保存配置到vector
        configs.push_back(config);
    }
    }
}

int PageScalingManager::NotifyPageChanged(char* targetPageName, uint32_t windowId)
{
    if (targetPageName == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "targetPage null");
        return AAFwk::INNER_ERR;
    }

    auto window = Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return AAFwk::INNER_ERR;
    }

    std::string targetPageNameStr(targetPageName);
    TAG_LOGI(AAFwkTag::ABILITY, "targetPageName: %{public}s", targetPageNameStr.c_str());

    HandleNavigateChangeInternal(window, targetPageNameStr, configs_);
    return ERR_OK;
}

void PageScalingManager::HandleNavigateChangeInternal(const sptr<Rosen::Window>& window,
    const std::string& targetPageName, const PageScalingConfig& config)
{
    if (ComparePage(config.page.pageName, targetPageName, config.page.patternType)) {
        ApplyConfig(window, config);
    } else {
        RestoreConfig(window);
    }
}

bool PageScalingManager::ConditionCheck(const std::string& condition, Rosen::DisplayOrientation orientation, Rosen::FoldDisplayMode foldMode)
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
        
        if (condList.size() != CONDITION_LIST_SIZE) {
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

    for (const auto& hideConfig : config.hideStatusBar) {
        TAG_LOGD(AAFwkTag::ABILITY, "hideStatusCofig value %{public}d", hideConfig.value);
        auto prop = window->GetSystemBarPropertyForPage(Rosen::WindowType::WINDOW_STATUS_BAR);
        prop.enable_ = hideConfig.value;
        window->SetSystemBarPropertyForPage(Rosen::WindowType::WINDOW_STATUS_BAR, prop);
        break;
    }

    for (const auto& videoShrinkConfig : config.videoShrink) {
        TAG_LOGD(AAFwkTag::ABILITY, "videoShrinkConfig value %{public}d", videoShrinkConfig.value);
        // TODO: Implement videoShrink logic based on value
        break;
    }
}

void PageScalingManager::RestoreConfig(const sptr<Rosen::Window>& window)
{
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window is null");
        return;
    }
    float defaultDensity = -1.0f;
    window->SetCustomDensity(defaultDensity);
    
    auto defaultProp = std::nullopt;
    window->SetSystemBarPropertyForPage(Rosen::WindowType::WINDOW_STATUS_BAR, defaultProp);

    
}

bool PageScalingManager::ComparePage(const std::string &whitelistPageName, const std::string &actualPageName, const std::string &patternType)
{
    if (patternType == "equals") {
        return actualPageName == whitelistPageName;
    }
    if (patternType == "prefix") {
        return actualPageName.find(whitelistPageName) == 0;
    }
    TAG_LOGE(AAFwkTag::ABILITY, "unknown patternType: %{public}s", patternType.c_str());
    return false;
}
} // namespace AppExecFwk
} // namespace OHOS