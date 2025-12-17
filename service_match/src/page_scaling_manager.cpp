/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <map>
#include <string>
#include <vector>
#include <mutex>
#include "match_tag_wrapper.h"
#include "page_data_manager.h"
#include "ui_content.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {

{
    std::vector<std::string> PAGE_CONFIG_KEYS = {
        "apps_page_level_config_dpi",
        "apps_page_level_config_hideStatusBar",
        "apps_page_level_config_videoShrink",
        "apps_page_level_config_verticalDisplay"
    };
}

PageScalingManager::PageScalingManager() {
    // 构造函数实现
}

void PageScalingManager::Initialize(std::weak_ptr<AbilityRuntime::AbilityContext> &context,sptr<Rosen::Window>& window, const std::string &key,
    const std::string &bundleName, const std::string &pageName)
{
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "window null");
        return;
    }

    auto& dataMgr = PageDataManager::GetInstance();
    if (!dataMgr.IsInitialized()) {
        ErrCode ret = dataMgr.Initialize(context);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to initialize page data manager, ret: %{public}d", ret);
            return;
        }
    }
    bundleName_ = bundleName;
    // 初始化config_的bundleName和pageName
    config_.bundleName = bundleName;
    config_.page.pageName = pageName;
    RetrivePageScalingConfig(dataMgr, window, bundleName_, pageName, config_);
    RegisterPageChangeListeners(window, key, config_);
}

void PageScalingManager::RetrivePageScalingConfig(PageDataManager &dataMgr, sptr<Rosen::Window>& window,
    const std::string &bundleName, const std::string &pageName, PageScalingConfig &config)
{
    for (auto configKey : PAGE_CONFIG_KEYS) {
        std::string configJson;
        ErrCode ret = dataMgr.GetStringValue(configKey, configJson);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get value key: %{public}s, ret: %{public}d", configKey.c_str(), ret);
            continue;
        }
        ParseWhitelistConfig(bundleName, pageName, configJson, config);
    }
}


void PageScalingManager::ParseWhitelistConfig(const std::string &bundleName, const std::string &pageName,
    const std::string &configJson, PageScalingConfig &config)
{
    try {
        nlohmann::json jsonArray = nlohmann::json::parse(configJson);
        
        for (auto& jsonObj : jsonArray) {
            std::string configBundleName = jsonObj.value("bundleName", "");
            std::string configPageName = jsonObj.value("page.pageName", "");
            if (configBundleName != bundleName) {
                continue;
            }
            if (configPageName != pageName) {
                continue;
            }
            TAG_LOGI(AAFwkTag::ABILITYMGR, "found page config");
            
            if (config.bundleName.empty()) {
                config.bundleName = configBundleName;
            }
            if (config.page.pageName.empty()) {
                config.page.pageName = configPageName;
            }
            if (config.page.patternType.empty()) {
                config.page.patternType = jsonObj.value("page.patternType", "");
            }
            
            if (jsonObj.contains("dpi") && jsonObj["dpi"].is_array()) {
                for (const auto& dpiJson : jsonObj["dpi"]) {
                    DpiConfig dpiConfig;
                    dpiConfig.condition = dpiJson.value("condition", "");
                    dpiConfig.value = dpiJson.value("value", "");
                    config.dpi.push_back(dpiConfig);
                }
            }
            
            if (jsonObj.contains("hideStatusBar") && jsonObj["hideStatusBar"].is_array()) {
                for (const auto& hideStatusBarJson : jsonObj["hideStatusBar"]) {
                    HideStatusBarConfig hideStatusBarConfig;
                    hideStatusBarConfig.condition = hideStatusBarJson.value("condition", "");
                    hideStatusBarConfig.value = hideStatusBarJson.value("value", "");
                    config.hideStatusBar.push_back(hideStatusBarConfig);
                }
            }
            
            if (jsonObj.contains("videoShrink") && jsonObj["videoShrink"].is_array()) {
                for (const auto& videoShrinkJson : jsonObj["videoShrink"]) {
                    VideoShrinkConfig videoShrinkConfig;
                    videoShrinkConfig.condition = videoShrinkJson["condition"];
                    videoShrinkConfig.value = videoShrinkJson.value("value", "");
                    config.videoShrink.push_back(videoShrinkConfig);
                }
            }
            
            if (jsonObj.contains("verticalDisplay") && jsonObj["verticalDisplay"].is_array()) {
                for (const auto& verticalDisplayJson : jsonObj["verticalDisplay"]) {
                    VerticalDisplayConfig verticalDisplayConfig;
                    if (verticalDisplayJson.contains("condition")) {
                        auto& conditionJson = verticalDisplayJson["condition"];
                        verticalDisplayConfig.condition.orientation = conditionJson.value("orientation", "");
                    }
                    verticalDisplayConfig.value = verticalDisplayJson.value("value", "");
                    config.verticalDisplay.push_back(verticalDisplayConfig);
                }
            }
        }
    } catch (const std::exception& e) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ParseWhitelistConfig failed, exception: %{public}s", e.what());
    }
}


void PageScalingManager::NotifyPageChanged(char* targetPageName, uint32_t windowId)
{
    if (targetPageName == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "targetPage null");
        return;
    }

    auto window = Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "window null");
        return;
    }

    std::string targetPageNameStr(targetPageName);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "NotifyPageChanged, targetPageName: %{public}s", targetPageNameStr.c_str());

    for (const auto& key : PAGE_CONFIG_KEYS) {
        HandleNavigateChangeInternal(window, key, targetPageNameStr, config_);
    }
}

void PageScalingManager::RegisterPageChangeListeners(const Rosen::Window& window, const std::string &key, const PageScalingConfig &config)
{
    if (!window || !window->GetUIContent()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "window or uicontent ull");
        return;
    }
    //TODO：注册页面监听回调？
    window->GetUIContent()->RegisterPageChangedCallback(
        [window, key, config](const OHOS::Ace::NavigateChangeInfo& from, const OHOS::Ace::NavigateChangeInfo& to) {
        // TODO：不同的key，要调用什么接口去设置? 
        // 只拿到window对象，能否获取到需要比较的信息？
        HandleNavigateChangeInternal(window, key, to.name, config);
    });
}

void PageScalingManager::HandleNavigateChangeInternal(const Rosen::Window& window, const std::string& key,
    const std::string& targetPageName, const PageScalingConfig& config)
{
    if (targetPageName == config.page.pageName && bundleName_ == config.page.bundleName) {
        ApplyConfig(window, config, key);
    }
}

void PageScalingManager::ApplyConfig(const Rosen::Window& window, const PageScalingConfig& config, const std::string& key)
{
    switch(key) {
        case "apps_page_level_config_dpi":
            for (const auto& dpiConfig : config.dpi) {
                if (dpiConfig.condition.empty()) {
                    bool applyToSubwindow = false;
                    float density = std::stof(dpiConfig.value);
                    window->SetCustomDensity(density, applyToSubwindow);
                    break;
                }
            }
            break;
        case "apps_page_level_config_hideStatusBar":
            for (const auto& hideConfig : config.hideStatusBar) {
                if (hideConfig.condition.empty()) {
                    auto prop = window->GetSystemBarPropertyForPage(Rosen::WidnowType::WINDOW_STATUS_BAR);
                    bool hideValue = (hideConfig.value == "true");
                    prop->enable_ = !hideValue;
                    window->SetSystemBarPropertyForPage(Rosen::WidnowType::WINDOW_STATUS_BAR, prop);
                    break;
                }
            }
            break;
        case "apps_page_level_config_videoShrink":
            for (const auto& videoConfig : config.videoShrink) {
            }
            break;
        case "apps_page_level_config_verticalDisplay":
            for (const auto& verticalConfig : config.verticalDisplay) {
            }
            break;
        default:
            TAG_LOGE(AAFwkTag::ABILITYMGR, "unknown key: %{public}s", key.c_str());
            break;
    }
}
} // namespace AbilityRuntime
} // namespace OHOS