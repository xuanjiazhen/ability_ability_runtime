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

#include "nlohmann/json.hpp"
#include "refbase.h"
#include "window.h"
#include "nocopyable.h"

namespace OHOS {
namespace AppExecFwk {

typedef struct {
    std::string condition;
    std::string value;
} Config;
typedef struct {
    std::string pageName;
    std::string patternType;
} Page;
typedef struct {
    Page page;
    std::vector<Config> dpi;
    std::vector<Config> hideStatusBar;
    std::vector<Config> videoShrink;
    std::vector<Config> verticalDisplay;
} PageScalingConfig;

class PageScalingManager final : public NoCopyable {
public:
    static PageScalingManager& GetInstance();
    int32_t ParseWhitelistConfig(const std::string &configJson, std::vector<PageScalingConfig> &configs);
    void ParseConfigArray(const nlohmann::json& item, const std::string& field, std::vector<Config>& dest);
    void HandleNavigateChangeInternal(const sptr<Rosen::Window>& window, const std::string& fromPageName,
    int32_t fromPageNameLen, const std::string& toPageName, int32_t toPageNameLen,
    const std::vector<PageScalingConfig>& configs);
    bool ConditionCheck(const std::string& condition, Rosen::DisplayOrientation orientation,
        Rosen::FoldDisplayMode foldMode);
    bool ComparePage(const std::string &whitelistPageName, const std::string &targetPageName,
        const std::string &patternType, int32_t targetPageNameLen);
    std::vector<PageScalingConfig>& GetConfigs();

    class WindowUIContentImpl : public Rosen::IUIContentCreateListener {
    public:
        WindowUIContentImpl(const std::vector<PageScalingConfig>& configs,
            const sptr<Rosen::Window>& window)
            : configs_(configs), window_(window) {}
        virtual ~WindowUIContentImpl() override = default;
        
        void OnUIContentCreate(std::weak_ptr<Ace::UIContent> uiContent) override;

    private:
        std::vector<PageScalingConfig> configs_;
        sptr<Rosen::Window> window_;
    };

private:
    PageScalingManager() = default;
    ~PageScalingManager() = default;

    void ProcessDpiConfig(const std::vector<Config>& configs, const std::string& tag,
        Rosen::FoldDisplayMode foldDisplayMode, const sptr<Rosen::Window>& window,
        const std::string& toPage, int32_t toPageNameLen, const Page& configPage);
    void ProcessVerticalDisplayConfig(const std::vector<Config>& configs,
        const std::string& tag, Rosen::DisplayOrientation orientation,
        Rosen::FoldDisplayMode foldDisplayMode, const sptr<Rosen::Window>& window,
        const std::string& fromPage, int32_t fromPageNameLen,
        const std::string& toPage, int32_t toPageNameLen, const Page& configPage)
    void ProcessHideStatusBarConfig(const std::vector<Config>& configs,
        const std::string& tag, const sptr<Rosen::Window>& window,
        const std::string& toPage, int32_t toPageNameLen, const Page& configPage);
    void ProcessVideoShrinkConfig(const std::vector<Config>& configs,
        const std::string& tag, const sptr<Rosen::Window>& window,
        const std::string& toPage, int32_t toPageNameLen, const Page& configPage);
    std::vector<std::string> ParseCondition(const std::string& condition);

    void RestoreDPI(const sptr<Rosen::Window>& window);
    void RestoreHideStatusBar(const sptr<Rosen::Window>& window);
    void RestoreVerticalDisplay(const sptr<Rosen::Window>& window);
    void RestoreVideoShrink(const sptr<Rosen::Window>& window);

    bool EmptyConditionCheck(const std::string& condition);
    bool DpiConditionCheck(const std::string& condition, Rosen::FoldDisplayMode foldMode);
    bool CheckComboCondition(const std::vector<std::string>& condList,
        Rosen::DisplayOrientation orientation, Rosen::FoldDisplayMode foldMode);
    bool CheckSingleCondition(const std::string& condition,
    Rosen::DisplayOrientation orientation, Rosen::FoldDisplayMode foldMode);
bool IsStayCondition(const std::string& condition);
std::vector<PageScalingConfig> configs_;
    sptr<Rosen::IUIContentCreateListener> uiContentListener_ = nullptr;
};

} // namespace AppExecFwk
} // namespace OHOS


extern "C" int32_t Initialize(const std::string& configJson, const OHOS::wptr<OHOS::Rosen::Window>& window);

extern "C" int32_t NotifyPageChanged(const char* targetPageName,
    int32_t targetPageNameLength, int32_t windowId);


#endif // OHOS_ABILITY_RUNTIME_PAGE_SCALING_MANAGER_H