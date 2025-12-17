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


PageScalingManager &PageScalingManager::GetInstance() {
    static PageScalingManager instance;
    return instance;
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
    
    PageScalingConfig config;
    GetPageScalingConfig(context, window, key, bundleName, pageName, config);
    RegisterPageChangeListeners(key, config);
}

std::shared_ptr<DataShare::DataShareHelper> PageScalingManager::CreateDataShareHelper(
    const sptr<IRemoteObject> &token, std::shared_ptr<DataShare::DataShareHelper> &dataShareHelper)
{
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "token null");
        return ERR_INVALID_PARAM;
    }
    const auto [errCode, helper] =
        DataShare::DataShareHelper::Create(remoteObject_,TEST_URI);
    if (errCode != DataShare::E_OK) {
        LOGE("create data share helper failed: %{public}d", errCode);
        return nullptr;
    }
    return helper;
}

void PageScalingManager::GetPageScalingConfig(sptr<Rosen::Window>& window, const std::string &key, const std::string &bundleName,
    const std::string &pageName, PageScalingConfig &config)
{
    

    std::string configJson;
    ErrCode ret = dataShareHelper->GetStringValue(key, configJson);
   
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get value key: %{public}s, ret: %{public}d", key.c_str(), ret);
        return;
    }

    ParseWhitelistConfig(bundleName, pageName, configJson, config);
}


void PageScalingManager::ParseWhitelistConfig(const std::string &bundleName, const std::string &pageName,
    const std::string &configJson, PageScalingConfig &config)
{
    try {
        nlohmann::json jsonArray = nlohmann::json::parse(configJson);
        
        for (auto& jsonObj : jsonArray) {
            std::string configBundleName = jsonObj.value("bundleName", "");
            std::string configPageName = jsonObj.value("page.pageName", "");
            if (configPageName != targetPageName) {
                continue;
            }
            TAG_LOGI(AAFwkTag::ABILITYMGR, "found page config");
            
            config.bundleName = configBundleName;
            config.page.pageName = configPageName;
            config.page.patternType = jsonObj.value("page.patternType", "");
            
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


void PageScalingManager::NotifyPageChanged(char* targetPageName)
{
    if (targetPageName == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "targetPage null");
        return;
    }

    std::string targetPageNameStr(targetPageName);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "NotifyPageChanged, targetPageName: %{public}s", targetPageNameStr.c_str());

    std::vector<std::string> keys = {
        "apps_page_level_config_dpi",
        "apps_page_level_config_hideStatusBar",
        "apps_page_level_config_videoShrink",
        "apps_page_level_config_verticalDisplay"
    };

    for (const auto& key : keys) {
        PageScalingConfig config;
        GetPageScalingConfig(key, bundleName, targetPageNameStr, config);
        HandleNavigateChangeInternal(key, targetPageNameStr, config);
    }
}

void PageScalingManager::HandleNavigateChangeInternal(const std::string key,
    const std::string& targetPageName, const PageScalingConfig& config)
{
    if (targetPageName == config.page.pageName) {
        ApplyConfig(config, key);
    }
}

void PageScalingManager::ApplyConfig(const PageScalingConfig& config, const std::string& key)
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

void PageScalingManager::RegisterPageChangeListeners(const std::string &key, const PageScalingConfig &config)
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
        HandleNavigateChangeInternal(to.name, config);
    });
}

} // namespace AbilityRuntime
} // namespace OHOS