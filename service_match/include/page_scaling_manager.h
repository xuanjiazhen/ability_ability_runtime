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

#ifndef FOUNDATION_ABILITY_RUNTIME_SERVICE_MATCH_PAGE_SCALING_MANAGER_H
#define FOUNDATION_ABILITY_RUNTIME_SERVICE_MATCH_PAGE_SCALING_MANAGER_H

#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include "nlohmann/json.hpp"
#include "sensor_data_manager.h"
#include "ui_content.h"
#include "window.h"

namespace OHOS {
namespace AbilityRuntime {

typedef struct {
    std::string orientation;
} Condition;

typedef struct {
    Condition condition;
    std::string value;
} DpiConfig;

typedef struct {
    Condition condition;
    std::string value;
} HideStatusBarConfig;

typedef struct {
    Condition condition;
    std::string value;
} VideoShrinkConfig;

typedef struct {
    Condition condition;
    std::string value;
} VerticalDisplayConfig;

typedef struct {
    std::string bundleName;
    struct {
        std::string pageName;
        std::string patternType;
    } page;
    std::vector<DpiConfig> dpi;
    std::vector<HideStatusBarConfig> hideStatusBar;
    std::vector<VideoShrinkConfig> videoShrink;
    std::vector<VerticalDisplayConfig> verticalDisplay;
} PageScalingConfig;

class PageScalingManager {
public:

    void Initialize(sptr<Rosen::Window>& window, const std::string &key, const std::string &bundleName, const std::string &pageName);

    void RetrivePageScalingConfig(PageDataManager &dataMgr, sptr<Rosen::Window>& window, const std::string &bundleName,
        const std::string &pageName, PageScalingConfig &config);

    void NotifyPageChanged(char* targetPageName);

private:
    PageScalingManager();
    ~PageScalingManager() = default;

    void ParseWhitelistConfig(const std::string &bundleName, const std::string &pageName, const std::string &configJson, PageScalingConfig &config);

    void RegisterPageChangeListeners(const std::string &key, const PageScalingConfig &config);

    void HandleNavigateChangeInternal(const std::string key, const std::string& targetPageName, const PageScalingConfig& config);

    void ApplyConfig(const PageScalingConfig& config, const std::string& key);

    // sptr<Rosen::Window> window_; // 窗口对象
    std::string bundleName_; // 应用包名
    PageScalingConfig config_; // 页面缩放配置
    // std::string targetPageName_; // 目标页面名称
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PAGE_SCALING_MANAGER_H