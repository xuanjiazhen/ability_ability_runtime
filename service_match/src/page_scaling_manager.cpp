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

int PageScalingManager::Initialize(std::string configJson, const wptr<Rosen::Window>& window)
{
    TAG_LOGW(AAFwkTag::ABILITY, "czx intialize mgr");
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return ERR_NO_INIT;
    }

    auto ret = ParseWhitelistConfig(configJson, configs_);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "ParseWhitelistConfig failed, ret: %{public}d", ret);
        return ret;
    }
    sptr<Rosen::Window> windowPtr = window.promote();
    if (windowPtr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "windowPtr null");
        return ERR_NO_INIT;
    }
    uiContentListener_ = new WindowUIContentImpl(shared_from_this(), configs_, windowPtr);
    windowPtr->RegisterWindowUIContentListener(uiContentListener_);
    return ERR_OK;
}

int PageScalingManager::NotifyPageChanged(char* targetPageName, int32_t targetPageNameLen, int32_t windowId)
{
    if (targetPageName == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "targetPage null");
        return ERR_NO_INIT;
    }

    auto window = Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return ERR_NO_INIT;
    }

    std::string targetPageNameStr(targetPageName);
    TAG_LOGI(AAFwkTag::ABILITY, "targetPageName: %{public}s", targetPageNameStr.c_str());
    HandleNavigateChangeInternal(window, targetPageNameStr, targetPageNameLen, configs_);
    return ERR_OK;
}

void PageScalingManager::WindowUIContentImpl::OnUIContentCreate(OHOS::Ace::UIContent* uiContent)
{
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "uiContent null");
        return;
    }
    
    auto owner = owner_.lock();
    if (!owner) {
        TAG_LOGE(AAFwkTag::ABILITY, "owner null");
        return;
    }

    TAG_LOGI(AAFwkTag::ABILITY, "uiContent created, registering navigate change callback");
    
    uiContent->RegisterNavigateChangeCallback(
        [owner, window_, configs_, this](const OHOS::Ace::NavigateChangeInfo& from, const OHOS::Ace::NavigateChangeInfo& to) {
            if (!window_) {
                TAG_LOGE(AAFwkTag::ABILITY, "window not found");
                return;
            }
            owner->HandleNavigateChangeInternal(window_, to.name, to.name.length(), configs_);
        });
}

int PageScalingManager::ParseWhitelistConfig(const std::string &configJson, std::vector<PageScalingConfig> &configs)
{
    auto parseResult = nlohmann::json::parse(configJson, nullptr, false);
    if (parseResult.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITY, "ParseWhitelistConfig failed, configJson: %{public}s", configJson.c_str());
        return ERR_INVALID_VALUE;
    }
    
    nlohmann::json jsonObj = parseResult;
    if (!jsonObj.is_array()) {
        TAG_LOGE(AAFwkTag::ABILITY, "configJson is not an array, configJson: %{public}s", configJson.c_str());
        return ERR_INVALID_VALUE;
    }
    
    for (const auto& item : jsonObj) {
        if (!item.contains("page") || !item["page"].is_object()) {
            continue;
        }
        std::string configPageName = item["page"].value("pn", "");
        std::string patternType = item["page"].value("pt", "");
        PageScalingConfig config;
        config.page.pageName = configPageName;
        config.page.patternType = patternType;
        
        TAG_LOGI(AAFwkTag::ABILITY, "Found config for page %{public}s with pattern %{public}s", 
            configPageName.c_str(), patternType.c_str());
        
        if (item.contains("dpi") && item["dpi"].is_array()) {
            for (const auto& dpiItem : item["dpi"]) {
                Config dpiConfig;
                StoreConfig(dpiItem, dpiConfig);
                TAG_LOGI(AAFwkTag::ABILITY, "DPI condition: %{public}s, value: %{public}s", 
                    dpiConfig.condition.c_str(), dpiConfig.value.c_str());
                config.dpi.push_back(dpiConfig);
            }
        }
        if (item.contains("hsb") && item["hsb"].is_array()) {
            for (const auto& hideItem : item["hsb"]) {
                Config hideConfig;
                StoreConfig(hideItem, hideConfig);
                TAG_LOGI(AAFwkTag::ABILITY, "HideStatusBar value: %{public}s", hideConfig.value.c_str());
                config.hideStatusBar.push_back(hideConfig);
            }
        }
        if (item.contains("vsk") && item["vsk"].is_array()) {
            for (const auto& videoItem : item["vsk"]) {
                Config videoConfig;
                StoreConfig(videoItem, videoConfig);
                TAG_LOGI(AAFwkTag::ABILITY, "VideoShrink value: %{public}s", videoConfig.value.c_str());
                config.videoShrink.push_back(videoConfig);
            }
        }
        if (item.contains("vd") && item["vd"].is_array()) {
            for (const auto& verticalItem : item["vd"]) {
                Config verticalConfig;
                StoreConfig(verticalItem, verticalConfig);
                TAG_LOGI(AAFwkTag::ABILITY, "VerticalDisplay condition: %{public}s, value: %{public}s",
                    verticalConfig.condition.c_str(), verticalConfig.value.c_str());
                config.verticalDisplay.push_back(verticalConfig);
            }
        }
    }
    return ERR_OK;
}

void PageScalingManager::HandleNavigateChangeInternal(const sptr<Rosen::Window>& window,
    const std::string& targetPageName, int32_t targetPageNameLen, const std::vector<PageScalingConfig>& configs)
{
    for (const auto& config : configs) {
        if (ComparePage(config.page.pageName, targetPageName, config.page.patternType, targetPageNameLen)) {
            ApplyConfig(window, config);
            TAG_LOGD(AAFwkTag::ABILITY, "Apply config for page %{public}s", config.page.pageName.c_str());
            return;
        }
    }
    RestoreConfig(window);
    TAG_LOGD(AAFwkTag::ABILITY, "Restore config for page %{public}s", targetPageName.c_str());
}

void PageScalingManager::ApplyConfig(const sptr<Rosen::Window>& window, const PageScalingConfig& config)
{
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return;
    }
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(window->GetDisplayId());
    if (display == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "display null");
        return;
    }
    auto foldDisplayMode = Rosen::DisplayManager::GetInstance().GetFoldDisplayMode();
    auto orientation = display->GetDisplayInfo().orientation;
    
    for (const auto& dpiConfig : config.dpi) {
        const std::string& condition = dpiConfig.condition;
        float dpiValue = std::stof(dpiConfig.value);
        if (ConditionCheck(condition, orientation, foldDisplayMode)) {
            bool applyToSubWindow = false;
            window->SetCustomDensity(dpiValue, applyToSubWindow);
            TAG_LOGI(AAFwkTag::ABILITY, "SetCustomDensity %{public}f for condition %{public}s", 
                dpiValue, condition.c_str());
            break;
        }
    }
    for (const auto& verticalConfig : config.verticalDisplay) {
        const std::string& condition = verticalConfig.condition.orientation;
        const std::string& value = verticalConfig.value;
        if (ConditionCheck(condition, orientation, foldDisplayMode)) {
            // TODO: Implement verticalDisplay logic based on value
            TAG_LOGI(AAFwkTag::ABILITY, "verticalDisplay condition matched %{public}s, value: %{public}s", 
                condition.c_str(), value.c_str());
            break;
        }
    }
    for (const auto& hideConfig : config.hideStatusBar) {
        TAG_LOGD(AAFwkTag::ABILITY, "hideStatusCofig value %{public}s", hideConfig.value.c_str());
        auto prop = window->GetSystemBarPropertyForPage(Rosen::WindowType::WINDOW_STATUS_BAR);
        prop.enable_ = !(hideConfig.value == "true");
        window->SetSystemBarPropertyForPage(Rosen::WindowType::WINDOW_STATUS_BAR, prop);
        break;
    }
    for (const auto& videoShrinkConfig : config.videoShrink) {
        TAG_LOGD(AAFwkTag::ABILITY, "videoShrinkConfig value %{public}s", videoShrinkConfig.value.c_str());
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

void PageScalingManager::StoreConfig(const nlohmann::json& jsonItem, Config& config)
{
    if (jsonItem.contains("condition")) {
        config.condition = jsonItem.value("condition", "");
    } else {
        config.condition = "";
    }
    config.value = jsonItem.value("val", "");
}

bool PageScalingManager::ComparePage(const std::string &whitelistPageName, 
    const std::string &targetPageName, const std::string &patternType, int32_t targetPageNameLen)
{
    if (patternType == "equals") {
        if (whitelistPageName.length() != actualPageName.length()) {
            return false;
        }
        return actualPageName == whitelistPageName;
    } else if (patternType == "prefix") {
        size_t whitelistLen = whitelistPageName.length();
        if (targetPageNameLen >= whitelistLen) {
            return targetPageName.compare(0, whitelistLen, whitelistPageName) == 0;
        } else {
            return whitelistPageName.compare(0, targetPageNameLen, targetPageName) == 0;
        }
    }
    TAG_LOGE(AAFwkTag::ABILITY, "patternType invalid");
    return false;
}

bool PageScalingManager::ConditionCheck(const std::string& condition, Rosen::DisplayOrientation orientation, Rosen::FoldDisplayMode foldMode)
{
    // no condtion always true
    if (condition.empty()) {
        return true;
    }
    // combo condition
    if (condition.find(",") != std::string::npos) {
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
        
        if (cond1 == "A" && cond2 == "D") {
            return (foldMode == Rosen::FoldDisplayMode::FOLD_DISPLAY_MODE_MAIN && 
                   orientation == Rosen::DisplayOrientation::PORTRAIT);
        } else if  (cond1 == "A" && cond2 == "C") {
            return (foldMode == Rosen::FoldDisplayMode::FOLD_DISPLAY_MODE_MAIN && 
                   orientation == Rosen::DisplayOrientation::LANDSCAPE);
        } else if (cond1 == "B" && cond2 == "C") {
            return (foldMode == Rosen::FoldDisplayMode::FOLD_DISPLAY_MODE_FULL && 
                   orientation == Rosen::DisplayOrientation::LANDSCAPE);
        } else if (cond1 == "B" && cond2 == "D") {
            return (foldMode == Rosen::FoldDisplayMode::FOLD_DISPLAY_MODE_FULL && 
                   orientation == Rosen::DisplayOrientation::PORTRAIT);
        }
        return false;
    } else { // single condition
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
} // namespace AppExecFwk
} // namespace OHOS