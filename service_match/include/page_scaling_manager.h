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
#include <memory>

#include "ability_context.h"
#include "nlohmann/json.hpp"
#include "page_data_manager.h"
#include "window.h"

namespace OHOS {
namespace AppExecFwk {

typedef struct {
    std::string condition;
    float value;
} DpiConfig;

typedef struct {
    bool value;
} HideStatusBarConfig;

typedef struct {
    bool value;
} VideoShrinkConfig;

typedef struct {
    std::string condition;
    std::string value;
} VerticalDisplayConfig;

typedef struct {
    std::string condition;
    std::string value;
} Config;

typedef struct {
    struct {
        std::string pageName;
        std::string patternType;
    } page;
    std::vector<Config> dpi;
    std::vector<Config> hideStatusBar;
    std::vector<Config> videoShrink;
    std::vector<Config> verticalDisplay;
} PageScalingConfig;

class PageScalingManager : public std::enable_shared_from_this<PageScalingManager> {
public:

    int Initialize(std::string configJson, const sptr<Rosen::Window>& window);

    int NotifyPageChanged(char* targetPageName, int32_t targetPageNameLen, int32_t windowId);

private:
    PageScalingManager();
    ~PageScalingManager() = default;

    int ParseWhitelistConfig(const std::string &configJson, std::vector<PageScalingConfig> &configs);

    void HandleNavigateChangeInternal(const sptr<Rosen::Window>& window,
        const std::string& targetPageName, const std::vector<PageScalingConfig>& configs);

    void ApplyConfig(const sptr<Rosen::Window>& window, const std::vector<PageScalingConfig>& configs);
    
    void RestoreConfig(const sptr<Rosen::Window>& window);
    
    void StoreConfig(const nlohmann::json& jsonItem, Config& config);
    
    bool ConditionCheck(const std::string& condition, Rosen::DisplayOrientation orientation, Rosen::FoldDisplayMode foldMode);

    bool ComparePage(const std::string &pageName, const std::string &targetPageName, std::string &patternType);

    class WindowUIContentImpl : public Rosen::IWindowUIContentListener {
    public:
        WindowUIContentImpl(const std::shared_ptr<PageScalingManager>& owner, const std::vector<PageScalingConfig>& configs, const sptr<Rosen::Window>& window)
            : owner_(owner), configs_(configs), window_(window) {}
        virtual ~WindowUIContentImpl() override = default;
        
        void OnUIContentCreate(OHOS::Ace::UIContent* uiContent) override;
        
    private:
        std::weak_ptr<PageScalingManager> owner_;
        std::vector<PageScalingConfig> configs_;
        sptr<Rosen::Window> window_;
    };

    std::vector<PageScalingConfig> configs_;
    sptr<WindowUIContentImpl> uiContentListener_ = nullptr;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_PAGE_SCALING_MANAGER_H