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

#include "hilog/log.h"
#include <string>
#include <vector>
#include <mutex>
#include "setting_data_manager.h"
#include "ui_content.h"

namespace OHOS {
namespace AbilityRuntime {

static const std::string TAG = "PageScalingManager";

// 实现单例模式
PageScalingManager &PageScalingManager::GetInstance() {
    static PageScalingManager instance;
    return instance;
}

// 初始化
void PageScalingManager::Initialize(sptr<Rosen::Window>& window, const std::string &key, const std::string &bundleName, const std::string &pageName) {
    if (window == nullptr) {
        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: window is null", TAG.c_str());
        return;
    }
    
    // 调用getPageScalingConfig获取配置
    PageScalingConfig config;
    GetPageScalingConfig(window, key, bundleName, pageName, config);
    
    
    // 注册页面监听回调
    RegisterPageChangeListeners(key, config);
}

// 获取当前页面的缩放配置
void PageScalingManager::GetPageScalingConfig(const std::string &key, const std::string &bundleName, const std::string &pageName, PageScalingConfig &config) {
    
    // 从SettingData中获取白名单信息
    SettingDataManager &settingManager = SettingDataManager::GetInstance();
    
    // 检查SettingDataManager是否已初始化
    if (!settingManager.IsInitialized()) {
        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: SettingDataManager is not initialized", TAG.c_str());
        return;
    }

    // 从SettingData获取白名单配置
    std::string configJson;
    ErrCode ret = settingManager.GetStringValue(key, configJson);
    if (ret != ERR_OK) {
        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: Failed to get string value for key: %{public}s, ret: %{public}d", TAG.c_str(), key.c_str(), ret);
        return;
    }

    // 解析白名单配置
    ParseWhitelistConfig(bundleName, pageName, configJson, config);
}


// 解析白名单配置
void PageScalingManager::ParseWhitelistConfig(const std::string &bundleName, const std::string &pageName, const std::string &configJson, PageScalingConfig &config) {
    try {
        // 把configJson从string变成Array<Object>
        nlohmann::json jsonArray = nlohmann::json::parse(configJson);
        
        // 遍历jsonArray
        for (auto& jsonObj : jsonArray) {
            // 比较bundleName pageName是否一致
            std::string configBundleName = jsonObj.value("bundleName", "");
            std::string configPageName = jsonObj.value("page.pageName", "");
            if (configPageName != targetPageName) {
                continue;
            }
            HILOG_INFO(HILOG_MODULE_ABILITY_RUNTIME, "found current page");
            // 保存当前的condition和value到PageScalingConfig中
            config.bundleName = configBundleName;
            config.page.pageName = configPageName;
            config.page.patternType = jsonObj.value("page.patternType", "");
            
            // 解析dpi配置
            if (jsonObj.contains("dpi") && jsonObj["dpi"].is_array()) {
                for (const auto& dpiJson : jsonObj["dpi"]) {
                    DpiConfig dpiConfig;
                    dpiConfig.condition = dpiJson.value("condition", "");
                    dpiConfig.value = dpiJson.value("value", "");
                    config.dpi.push_back(dpiConfig);
                }
            }
            
            // 解析hideStatusBar配置
            if (jsonObj.contains("hideStatusBar") && jsonObj["hideStatusBar"].is_array()) {
                for (const auto& hideStatusBarJson : jsonObj["hideStatusBar"]) {
                    HideStatusBarConfig hideStatusBarConfig;
                    hideStatusBarConfig.condition = hideStatusBarJson.value("condition", "");
                    hideStatusBarConfig.value = hideStatusBarJson.value("value", "");
                    config.hideStatusBar.push_back(hideStatusBarConfig);
                }
            }
            
            // 解析videoShrink配置
            if (jsonObj.contains("videoShrink") && jsonObj["videoShrink"].is_array()) {
                for (const auto& videoShrinkJson : jsonObj["videoShrink"]) {
                    VideoShrinkConfig videoShrinkConfig;
                    videoShrinkConfig.condition = videoShrinkJson["condition"];
                    videoShrinkConfig.value = videoShrinkJson.value("value", "");
                    config.videoShrink.push_back(videoShrinkConfig);
                }
            }
            
            // 解析verticalDisplay配置
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
        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: ParseWhitelistConfig failed, exception: %{public}s", TAG.c_str(), e.what());
    }
}



// 三方框架通知页面变化
void PageScalingManager::NotifyPageChanged(char* targetPageName) {
    if (targetPageName == nullptr) {
        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: targetPageName is nullptr", TAG.c_str());
        return;
    }

    std::string targetPageNameStr(targetPageName);
    HILOG_INFO(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: NotifyPageChanged, targetPageName: %{public}s", TAG.c_str(), targetPageNameStr.c_str());

    // 定义白名单key列表
    std::vector<std::string> keys = {
        "page_level_config_dpi",
        "page_level_config_hideStatusBar",
        "page_level_config_videoShrink",
        "page_level_config_verticalDisplay"
    };

    // 获取目标页面的白名单信息
    for (const auto& key : keys) {
        PageScalingConfig config;
        GetPageScalingConfig(key, bundleName, targetPageNameStr, config);
        // 处理页面变化
        HandleNavigateChangeInternal(key, targetPageNameStr, config);
    }
}

void PageScalingManager::HandleNavigateChangeInternal(const std::string key, const std::string& targetPageName, const PageScalingConfig& config) {
    if (targetPageName == config.page.pageName) {
        // 处理页面变化
        ApplyConfig(config, key);
    }
}

void PageScalingManager::ApplyConfig(const PageScalingConfig& config, const std::string& key) {
    switch(key) {
        case "page_level_config_dpi":
            // 应用DPI配置  
            for (const auto& dpiConfig : config.dpi) {
                // 这里可以根据条件判断是否应用配置
                if (dpiConfig.condition.empty()) {
                    bool applyToSubwindow = false;
                    float density = std::stof(dpiConfig.value);
                    window->SetCustomDensity(density, applyToSubwindow);
                    break;
                }
            }
            break;
        case "page_level_config_hideStatusBar":
            // 应用隐藏状态栏配置
            for (const auto& hideConfig : config.hideStatusBar) {
                if (hideConfig.condition.empty()) {
                    std::optional<Rosen::SystemBarProperty> prop;
                    prop->enable_ = hideConfig.value;
                    window->SetSystemBarPropertyForPage(Rosen::WidnowType::WINDOW_STATUS_BAR, prop);
                    break;
                }
            }
            break;
        case "page_level_config_videoShrink":
            // 应用视频压缩配置
            for (const auto& videoConfig : config.videoShrink) {

            }
            break;
        case "page_level_config_verticalDisplay":
            // 应用垂直显示配置
            for (const auto& verticalConfig : config.verticalDisplay) {
                if (verticalConfig.condition.orientation == targetPageName.displayInfo.orientation) {
                    // set VerticalDisplay ?? 
                }
            }
            break;
        default:
            HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: ApplyConfig failed, unknown key: %{public}s", TAG.c_str(), key.c_str());
            break;
    }
    
}

// 根据key 和获取到的配置，调用对应的设置接口 
void PageScalingManager::RegisterPageChangeListeners(const std::string &key, const PageScalingConfig &config) {
    if (!window || !window->GetUIContent()) {
        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "Window or UIContent is null");
        return;
    }
    //TODO：注册页面监听回调？
    window->GetUIContent()->RegisterPageChangedCallback(
        [window, key, config](const OHOS::Ace::NavigateChangeInfo& from, const OHOS::Ace::NavigateChangeInfo& to) {
        // TODO：不同的key，要调用什么接口去设置? 
        // 只拿到window对象，能否获取到需要比较的信息？
        HandleNavigateChangeInternal(to.name, config);
    });
}

} // namespace AbilityRuntime
} // namespace OHOS