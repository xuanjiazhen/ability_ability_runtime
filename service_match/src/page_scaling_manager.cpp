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

void PageScalingManager::Initialize(std::weak_ptr<OHOS::AbilityRuntime::AbilityContext> &context, const sptr<Rosen::Window>& window,
    const std::string &key, const std::string &bundleName, const std::string &pageName)
{
    TAG_LOGW(AAFwkTag::ABILITY, "czx intialize mgr");
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return;
    }

    auto& dataMgr = PageDataManager::GetInstance();
    if (!dataMgr.IsInitialized()) {
        ErrCode ret = dataMgr.Initialize(context);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITY, "Failed to initialize page data manager, ret: %{public}d", ret);
            return;
        }
    }
    TAG_LOGW(AAFwkTag::ABILITY, "czx get datamgr");
    bundleName_ = bundleName;
    config_.bundleName = bundleName;
    config_.page.pageName = pageName;
    RetrivePageScalingConfig(dataMgr, window, bundleName_, pageName, config_);
    RegisterPageChangeListeners(window, key, config_);
}

void PageScalingManager::RetrivePageScalingConfig(PageDataManager &dataMgr, const sptr<Rosen::Window>& window,
    const std::string &bundleName, const std::string &pageName, PageScalingConfig &config)
{
    for (auto configKey : PAGE_CONFIG_KEYS) {
        std::string configJson;
        ErrCode ret = dataMgr.GetStringValue(configKey, configJson);
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITY, "Failed to get value key: %{public}s, ret: %{public}d", configKey.c_str(), ret);
            continue;
        }
        ParseWhitelistConfig(bundleName, pageName, configJson, config);
    }
}

void PageScalingManager::ParseWhitelistConfig(const std::string &bundleName, const std::string &pageName,
    const std::string &configJson, PageScalingConfig &config)
{
    // 解析JSON，使用非抛出版本
    auto parseResult = nlohmann::json::parse(configJson, nullptr, false);
    if (parseResult.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITY, "ParseWhitelistConfig failed, configJson: %{public}s", configJson.c_str());
        return;
    }
    
    nlohmann::json jsonObj = parseResult;

    // 检查是否为对象类型
    if (!jsonObj.is_object()) {
        TAG_LOGE(AAFwkTag::ABILITY, "configJson is not an object, configJson: %{public}s", configJson.c_str());
        return;
    }
    
    // 根据bundleName查找对应的配置数组
    if (!jsonObj.contains(bundleName)) {
        TAG_LOGI(AAFwkTag::ABILITY, "No config found for bundleName: %{public}s", bundleName.c_str());
        return;
    }
    
    // 获取bundleName对应的配置数组
    auto bundleConfigs = jsonObj[bundleName];
    if (!bundleConfigs.is_array()) {
        TAG_LOGE(AAFwkTag::ABILITY, "Bundle config is not an array, bundleName: %{public}s", bundleName.c_str());
        return;
    }
    
    // 遍历数组，查找匹配pageName的配置
    bool foundPage = false;
    for (const auto& item : bundleConfigs) {
        if (!item.contains("page") || !item["page"].is_object()) {
            continue;
        }
        
        std::string currentPageName = item["page"].value("pageName", "");
        if (currentPageName == pageName) {
            foundPage = true;
            
            // 设置基本配置信息
            config.bundleName = bundleName;
            config.page.pageName = currentPageName;
            config.page.patternType = item["page"].value("patternType", "");
            
            // 解析DPI配置
            if (item.contains("dpi") && item["dpi"].is_array()) {
                for (const auto& dpiItem : item["dpi"]) {
                    DpiConfig dpiConfig;
                    
                    // 解析condition
                    dpiConfig.condition = dpiItem.value("condition", "");
                    
                    // 解析value - 需要转换为float
                    std::string valueStr = dpiItem.value("value", "0");
                    dpiConfig.value = std::stof(valueStr);
                    config.dpi.push_back(dpiConfig);
                }
            }
            
            // 解析hideStatusBar配置
            if (item.contains("hideStatusBar") && item["hideStatusBar"].is_array()) {
                for (const auto& hideItem : item["hideStatusBar"]) {
                    HideStatusBarConfig hideConfig;
                    
                    // 解析condition
                    hideConfig.condition = hideItem.value("condition", "");
                    
                    // 解析value - 需要转换为bool
                    std::string valueStr = hideItem.value("value", "false");
                    hideConfig.value = (valueStr == "true" || valueStr == "1");
                    config.hideStatusBar.push_back(hideConfig);
                }
            }
            
            // 解析videoShrink配置
            if (item.contains("videoShrink") && item["videoShrink"].is_array()) {
                for (const auto& videoItem : item["videoShrink"]) {
                    VideoShrinkConfig videoConfig;
                    
                    // 解析condition
                    videoConfig.condition = videoItem.value("condition", "");
                    
                    // 解析value - 需要转换为bool
                    std::string valueStr = videoItem.value("value", "false");
                    videoConfig.value = (valueStr == "true" || valueStr == "1");
                    
                    config.videoShrink.push_back(videoConfig);
                }
            }
            
            // 解析verticalDisplay配置
            if (item.contains("verticalDisplay") && item["verticalDisplay"].is_array()) {
                for (const auto& verticalItem : item["verticalDisplay"]) {
                    VerticalDisplayConfig verticalConfig;
                    
                    // 解析condition - 这里是对象类型
                    if (verticalItem.contains("condition") && verticalItem["condition"].is_object()) {
                        verticalConfig.condition.orientation = 
                            verticalItem["condition"].value("orientation", "");
                    } else {
                        // 如果没有condition对象，尝试直接读取orientation
                        verticalConfig.condition.orientation = 
                            verticalItem.value("condition", ""); // fallback
                    }
                    
                    // 解析value
                    verticalConfig.value = verticalItem.value("value", "");
                    config.verticalDisplay.push_back(verticalConfig);
                }
            }
            
            break;
        }
    }
    
    if (!foundPage) {
        TAG_LOGI(AAFwkTag::ABILITY, "No config found for pageName: %{public}s in bundle: %{public}s", 
                    pageName.c_str(), bundleName.c_str());
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
    //TODO：注册页面监听回调？
    window->GetUIContent()->RegisterNavigateChangeCallback(
        [this, window, key, config](const OHOS::Ace::NavigateChangeInfo& from, const OHOS::Ace::NavigateChangeInfo& to) {
        // TODO：不同的key，要调用什么接口去设置? 
        // 只拿到window对象，能否获取到需要比较的信息？
        HandleNavigateChangeInternal(window, key, to.name, config);
    });
}

void PageScalingManager::HandleNavigateChangeInternal(const sptr<Rosen::Window>& window, const std::string& key,
    const std::string& targetPageName, const PageScalingConfig& config)
{
    if (targetPageName == config.page.pageName && bundleName_ == config.bundleName) {
        ApplyConfig(window, config, key);
    }
}

void PageScalingManager::ApplyConfig(const sptr<Rosen::Window>& window, const PageScalingConfig& config, const std::string& key)
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
} // namespace AppExecFwk
} // namespace OHOS