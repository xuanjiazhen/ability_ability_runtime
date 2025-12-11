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

#ifndef OHOS_ABILITY_RUNTIME_PAGE_SCALING_MANAGER_H
#define OHOS_ABILITY_RUNTIME_PAGE_SCALING_MANAGER_H

#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include "nlohmann/json.hpp"
#include "setting_data_manager.h"

// 窗口模块相关头文件（示例，实际需要根据具体的窗口模块接口进行调整）
// #include "window_manager.h"

namespace OHOS {
namespace AbilityRuntime {

// 通用条件结构体
typedef struct {
    std::string orientation;
} Condition;

// DPI配置结构体
typedef struct {
    Condition condition;
    std::string value;
} DpiConfig;

// 状态栏隐藏配置结构体
typedef struct {
    Condition condition;
    std::string value;
} HideStatusBarConfig;

// 视频缩放配置结构体
typedef struct {
    Condition condition;
    std::string value;
} VideoShrinkConfig;

// 垂直显示配置结构体
typedef struct {
    Condition condition;
    std::string value;
} VerticalDisplayConfig;

// 页面缩放配置结构体
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

// 页面缩放管理类
class PageScalingManager {
public:
    // 获取单例实例
    static PageScalingManager &GetInstance();

    // 初始化
    void Initialize(const std::string &key, const std::string &bundleName, const std::string &pageName);

    // 获取当前页面的缩放配置
    PageScalingConfig GetPageScalingConfig(const std::string &key, const std::string &bundleName, const std::string &pageName);

    // 三方框架通知页面变化
    void NotifyPageChanged(char* targetPageName);

private:
    PageScalingManager();
    ~PageScalingManager() = default;

    // 解析白名单配置
    void ParseWhitelistConfig(const std::string &bundleName, const std::string &pageName, const std::string &configJson, PageScalingConfig &config);

    // 处理页面导航变化
    void HandleNavigateChange(const NavigateChangeInfo& from, const NavigateChangeInfo& to);

};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PAGE_SCALING_MANAGER_H