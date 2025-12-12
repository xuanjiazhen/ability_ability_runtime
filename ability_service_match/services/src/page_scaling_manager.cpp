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
#include "setting_data_manager.h"
#include "nlohmann/json.hpp"
#include "hilog/log.h"

namespace OHOS::AbilityServiceMatch::PageScalingManager {

static const std::string TAG = "PageScalingManager";

// 实现单例模式
PageScalingManager &PageScalingManager::GetInstance() {
    static PageScalingManager instance;
    return instance;
}

// 构造函数
PageScalingManager::PageScalingManager() : isInitialized_(false) {
}

// 初始化
void PageScalingManager::Initialize(const std::string &key, const std::string &bundleName, const std::string &pageName) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (isInitialized_) {
        return;
    }

    // 初始化SettingDataManager
    SettingDataManager &settingManager = SettingDataManager::GetInstance();
    ErrCode ret = settingManager.Initialize();
    if (ret != ERR_OK) {
        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: Failed to initialize SettingDataManager, ret: %{public}d", TAG.c_str(), ret);
        return;
    }

    isInitialized_ = true;
    HILOG_INFO(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: Initialized successfully", TAG.c_str());
}

// 获取当前页面的缩放配置
PageScalingConfig PageScalingManager::GetPageScalingConfig(const std::string &key, const std::string &bundleName, const std::string &pageName) {
    PageScalingConfig config;
    if (!isInitialized_) {
        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: Not initialized", TAG.c_str());
        return config;
    }

    // 初始化SettingDataManager
    SettingDataManager &settingManager = SettingDataManager::GetInstance();
    
    // 从SettingData获取白名单配置
    std::string configJson;
    ErrCode ret = settingManager.GetStringValue(key, configJson);
    if (ret != ERR_OK) {
        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: Failed to get string value for key: %{public}s, ret: %{public}d", TAG.c_str(), key.c_str(), ret);
        return config;
    }

    // 解析白名单配置
    ParseWhitelistConfig(bundleName, pageName, configJson, config);

    return config;
}

// 注册页面导航变化回调
void PageScalingManager::RegisterNavigateChangeCallback(const std::function<void(const NavigateChangeInfo&, const NavigateChangeInfo&)>&& callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    navigateChangeCallback_ = std::move(callback);
}

// 三方框架通知页面变化
void PageScalingManager::NotifyPageChanged(char* targetPageName) {
    if (targetPageName == nullptr) {
        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: targetPageName is nullptr", TAG.c_str());
        return;
    }

    std::string targetPageNameStr(targetPageName);
    HILOG_INFO(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: NotifyPageChanged, targetPageName: %{public}s", TAG.c_str(), targetPageNameStr.c_str());

    // 获取当前应用的bundleName（这里需要根据实际情况获取）
    std::string bundleName = "";
    // TODO: 实现获取当前应用bundleName的逻辑

    // 定义白名单key列表
    std::vector<std::string> keys = {
        "page_level_config_dpi",
        "page_level_config_hideStatusBar",
        "page_level_config_videoShrink",
        "page_level_config_verticalDisplay"
    };

    // 获取目标页面的白名单信息
    for (const auto& key : keys) {
        PageScalingConfig config = GetPageScalingConfig(key, bundleName, targetPageNameStr);
        if (!config.bundleName.empty()) {
            // 调用注册的回调函数
            std::lock_guard<std::mutex> lock(mutex_);
            if (navigateChangeCallback_) {
                NavigateChangeInfo from;
                NavigateChangeInfo to;
                to.name = targetPageNameStr;
                navigateChangeCallback_(from, to);
            }
        }
    }
}

// 解析白名单配置
void PageScalingManager::ParseWhitelistConfig(const std::string &bundleName, const std::string &pageName, const std::string &configJson, PageScalingConfig &config) {
    if (configJson.empty()) {
        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: configJson is empty", TAG.c_str());
        return;
    }

    // 把configJson从string变成Array<Object>，使用非抛出版本
    nlohmann::json jsonArray;
    auto parseResult = nlohmann::json::parse(configJson, nullptr, false);
    if (parseResult.is_discarded()) {
        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: Failed to parse configJson, configJson: %{public}s", TAG.c_str(), configJson.c_str());
        return;
    }
    jsonArray = parseResult;

    // 检查是否为数组
    if (!jsonArray.is_array()) {
        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: configJson is not array, configJson: %{public}s", TAG.c_str(), configJson.c_str());
        return;
    }

    // 遍历jsonArray
    for (size_t i = 0; i < jsonArray.size(); ++i) {
        auto& jsonObj = jsonArray[i];
        if (!jsonObj.is_object()) {
            HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu] is not object", TAG.c_str(), i);
            continue;
        }

        // 比较bundleName pageName是否一致
        std::string configBundleName;
        if (jsonObj.contains("bundleName")) {
            if (jsonObj["bundleName"].is_string()) {
                configBundleName = jsonObj["bundleName"].get<std::string>();
            } else {
                HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].bundleName is not string", TAG.c_str(), i);
                continue;
            }
        } else {
            HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu] missing bundleName", TAG.c_str(), i);
            continue;
        }

        // 获取pageName和patternType
        std::string configPageName;
        std::string patternType = "exact"; // 默认精确匹配
        if (jsonObj.contains("page")) {
            if (jsonObj["page"].is_object()) {
                auto& pageJson = jsonObj["page"];
                if (pageJson.contains("pageName")) {
                    if (pageJson["pageName"].is_string()) {
                        configPageName = pageJson["pageName"].get<std::string>();
                    } else {
                        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].page.pageName is not string", TAG.c_str(), i);
                        continue;
                    }
                } else {
                    HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].page missing pageName", TAG.c_str(), i);
                    continue;
                }

                // 获取patternType
                if (pageJson.contains("patternType")) {
                    if (pageJson["patternType"].is_string()) {
                        patternType = pageJson["patternType"].get<std::string>();
                    } else {
                        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].page.patternType is not string", TAG.c_str(), i);
                    }
                }
            } else {
                HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].page is not object", TAG.c_str(), i);
                continue;
            }
        } else {
            HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu] missing page", TAG.c_str(), i);
            continue;
        }

        if (!IsMatch(configBundleName, bundleName, configPageName, pageName)) {
            continue;
        }

        HILOG_INFO(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: find current page, bundleName: %{public}s, pageName: %{public}s", TAG.c_str(), configBundleName.c_str(), configPageName.c_str());

        // 保存当前的condition和value到PageScalingConfig中
        config.bundleName = configBundleName;
        config.page.pageName = configPageName;
        config.page.patternType = patternType;
        
        // 解析dpi配置
        if (jsonObj.contains("dpi")) {
            if (jsonObj["dpi"].is_array()) {
                auto& dpiArray = jsonObj["dpi"];
                for (size_t j = 0; j < dpiArray.size(); ++j) {
                    auto& dpiJson = dpiArray[j];
                    if (!dpiJson.is_object()) {
                        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].dpi[%{public}zu] is not object", TAG.c_str(), i, j);
                        continue;
                    }

                    DpiConfig dpiConfig;
                    // 解析condition
                    if (dpiJson.contains("condition")) {
                        dpiConfig.condition = dpiJson.value("condition", "");
                    } else {
                        HILOG_WARN(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].dpi[%{public}zu] missing condition, using empty string", TAG.c_str(), i, j);
                        dpiConfig.condition = "";
                    }

                    // 解析value
                    if (dpiJson.contains("value")) {
                        if (dpiJson["value"].is_string()) {
                            dpiConfig.value = dpiJson["value"].get<std::string>();
                        } else {
                            HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].dpi[%{public}zu].value is not string", TAG.c_str(), i, j);
                            continue;
                        }
                    } else {
                        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].dpi[%{public}zu] missing value", TAG.c_str(), i, j);
                        continue;
                    }

                    config.dpi.push_back(dpiConfig);
                }
            } else {
                HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].dpi is not array", TAG.c_str(), i);
            }
        }
        
        // 解析hideStatusBar配置
        if (jsonObj.contains("hideStatusBar")) {
            if (jsonObj["hideStatusBar"].is_array()) {
                auto& hideStatusBarArray = jsonObj["hideStatusBar"];
                for (size_t j = 0; j < hideStatusBarArray.size(); ++j) {
                    auto& hideStatusBarJson = hideStatusBarArray[j];
                    if (!hideStatusBarJson.is_object()) {
                        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].hideStatusBar[%{public}zu] is not object", TAG.c_str(), i, j);
                        continue;
                    }

                    HideStatusBarConfig hideStatusBarConfig;
                    // 解析condition
                    if (hideStatusBarJson.contains("condition")) {
                        hideStatusBarConfig.condition = hideStatusBarJson.value("condition", "");
                    } else {
                        HILOG_WARN(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].hideStatusBar[%{public}zu] missing condition, using empty string", TAG.c_str(), i, j);
                        hideStatusBarConfig.condition = "";
                    }

                    // 解析value
                    if (hideStatusBarJson.contains("value")) {
                        if (hideStatusBarJson["value"].is_string()) {
                            hideStatusBarConfig.value = hideStatusBarJson["value"].get<std::string>();
                        } else {
                            HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].hideStatusBar[%{public}zu].value is not string", TAG.c_str(), i, j);
                            continue;
                        }
                    } else {
                        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].hideStatusBar[%{public}zu] missing value", TAG.c_str(), i, j);
                        continue;
                    }

                    config.hideStatusBar.push_back(hideStatusBarConfig);
                }
            } else {
                HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].hideStatusBar is not array", TAG.c_str(), i);
            }
        }
        
        // 解析videoShrink配置
        if (jsonObj.contains("videoShrink")) {
            if (jsonObj["videoShrink"].is_array()) {
                auto& videoShrinkArray = jsonObj["videoShrink"];
                for (size_t j = 0; j < videoShrinkArray.size(); ++j) {
                    auto& videoShrinkJson = videoShrinkArray[j];
                    if (!videoShrinkJson.is_object()) {
                        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].videoShrink[%{public}zu] is not object", TAG.c_str(), i, j);
                        continue;
                    }

                    VideoShrinkConfig videoShrinkConfig;
                    // 解析condition
                    if (videoShrinkJson.contains("condition")) {
                        if (videoShrinkJson["condition"].is_string()) {
                            videoShrinkConfig.condition = videoShrinkJson["condition"].get<std::string>();
                        } else {
                            HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].videoShrink[%{public}zu].condition is not string", TAG.c_str(), i, j);
                            continue;
                        }
                    } else {
                        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].videoShrink[%{public}zu] missing condition", TAG.c_str(), i, j);
                        continue;
                    }

                    // 解析value
                    if (videoShrinkJson.contains("value")) {
                        if (videoShrinkJson["value"].is_string()) {
                            videoShrinkConfig.value = videoShrinkJson["value"].get<std::string>();
                        } else {
                            HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].videoShrink[%{public}zu].value is not string", TAG.c_str(), i, j);
                            continue;
                        }
                    } else {
                        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].videoShrink[%{public}zu] missing value", TAG.c_str(), i, j);
                        continue;
                    }

                    config.videoShrink.push_back(videoShrinkConfig);
                }
            } else {
                HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].videoShrink is not array", TAG.c_str(), i);
            }
        }
        
        // 解析verticalDisplay配置
        if (jsonObj.contains("verticalDisplay")) {
            if (jsonObj["verticalDisplay"].is_array()) {
                auto& verticalDisplayArray = jsonObj["verticalDisplay"];
                for (size_t j = 0; j < verticalDisplayArray.size(); ++j) {
                    auto& verticalDisplayJson = verticalDisplayArray[j];
                    if (!verticalDisplayJson.is_object()) {
                        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].verticalDisplay[%{public}zu] is not object", TAG.c_str(), i, j);
                        continue;
                    }

                    VerticalDisplayConfig verticalDisplayConfig;
                    // 解析condition
                    if (verticalDisplayJson.contains("condition")) {
                        if (verticalDisplayJson["condition"].is_object()) {
                            auto& conditionJson = verticalDisplayJson["condition"];
                            verticalDisplayConfig.condition.orientation = conditionJson.value("orientation", "");
                        } else {
                            HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].verticalDisplay[%{public}zu].condition is not object", TAG.c_str(), i, j);
                            continue;
                        }
                    } else {
                        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].verticalDisplay[%{public}zu] missing condition", TAG.c_str(), i, j);
                        continue;
                    }

                    // 解析value
                    if (verticalDisplayJson.contains("value")) {
                        if (verticalDisplayJson["value"].is_string()) {
                            verticalDisplayConfig.value = verticalDisplayJson["value"].get<std::string>();
                        } else {
                            HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].verticalDisplay[%{public}zu].value is not string", TAG.c_str(), i, j);
                            continue;
                        }
                    } else {
                        HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].verticalDisplay[%{public}zu] missing value", TAG.c_str(), i, j);
                        continue;
                    }

                    config.verticalDisplay.push_back(verticalDisplayConfig);
                }
            } else {
                HILOG_ERROR(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: jsonArray[%{public}zu].verticalDisplay is not array", TAG.c_str(), i);
            }
        }
    }
}

// 比较bundleName和pageName是否匹配
bool PageScalingManager::IsMatch(const std::string &configBundleName, const std::string &targetBundleName,
    const std::string &configPageName, const std::string &targetPageName) {
    // 比较bundleName
    if (configBundleName != targetBundleName) {
        return false;
    }

    // 比较pageName
    if (configPageName != targetPageName) {
        return false;
    }

    return true;
}

// 处理页面导航变化
void PageScalingManager::HandleNavigateChange(const NavigateChangeInfo& from, const NavigateChangeInfo& to) {
    HILOG_INFO(HILOG_MODULE_ABILITY_RUNTIME, "%{public}s: HandleNavigateChange, from: %{public}s, to: %{public}s", TAG.c_str(), from.name.c_str(), to.name.c_str());

    // 获取目标页面的白名单信息
    NotifyPageChanged(const_cast<char*>(to.name.c_str()));
}

} // namespace OHOS::AbilityServiceMatch::PageScalingManager

// NDK接口实现
extern "C" {
    void OH_AbilityRuntime_ApplicationContextNotifyPageChanged(char* targetPageName) {
        OHOS::AbilityServiceMatch::PageScalingManager::GetInstance().NotifyPageChanged(targetPageName);
    }
}