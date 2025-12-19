/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include <mutex>

#include "ability_context.h"
#include "nlohmann/json.hpp"
#include "page_data_manager.h"
#include "window.h"

namespace OHOS {
namespace AppExecFwk {

typedef struct {
    std::string orientation;
} Condition;

typedef struct {
    std::string condition;
    float value;
} DpiConfig;

typedef struct {
    std::string condition;
    bool value;
} HideStatusBarConfig;

typedef struct {
    std::string condition;
    bool value;
} VideoShrinkConfig;

typedef struct {
    Condition condition;
    std::string value;
} VerticalDisplayConfig;

typedef struct {
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

    void Initialize(std::string configJson, const sptr<Rosen::Window>& window,
        const std::string &pageName);

    void RetrivePageScalingConfig(std::string configJson, const sptr<Rosen::Window>& window,
        const std::string &pageName, PageScalingConfig &config);

    void NotifyPageChanged(char* targetPageName, uint32_t windowId);

private:
    PageScalingManager();
    ~PageScalingManager() = default;

    void ParseWhitelistConfig(const std::string &pageName, const std::string &configJson, PageScalingConfig &config);

    void RegisterPageChangeListeners(const sptr<Rosen::Window>& window, const PageScalingConfig &config);

    void HandleNavigateChangeInternal(const sptr<Rosen::Window>& window,
        const std::string& targetPageName, const PageScalingConfig& config);

    void ApplyConfig(const sptr<Rosen::Window>& window, const PageScalingConfig& config);

    PageScalingConfig config_; // 页面缩放配置
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PAGE_SCALING_MANAGER_H