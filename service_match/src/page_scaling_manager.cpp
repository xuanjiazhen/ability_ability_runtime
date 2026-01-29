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

#include "ability_business_error.h"
#include "display_manager.h"
#include "display_info.h"
#include "match_log_wrapper.h"
#include "ui_content.h"


extern "C" int32_t Initialize(const std::string& configJson, const OHOS::wptr<OHOS::Rosen::Window>& window)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    if (configJson.empty()) {
        TAG_LOGE(AAFwkTag::ABILITY, "config null");
        return OHOS::ERR_NO_INIT;
    }
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return OHOS::ERR_NO_INIT;
    }
    auto& mgr = OHOS::AppExecFwk::PageScalingManager::GetInstance();
    auto ret = mgr.ParseWhitelistConfig(configJson, mgr.GetConfigs());
    if (ret != OHOS::ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "parseconfig failed, ret: %{public}d", ret);
        return ret;
    }
    OHOS::sptr<OHOS::Rosen::Window> windowPtr = window.promote();
    if (windowPtr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "windowPtr null");
        return OHOS::ERR_NO_INIT;
    }
    auto uiContentListener_ = OHOS::sptr<OHOS::Rosen::IUIContentCreateListener>(
        new OHOS::AppExecFwk::PageScalingManager::WindowUIContentImpl(
            mgr.GetConfigs(), windowPtr));
    windowPtr->RegisterUIContentCreateListener(uiContentListener_);
    return OHOS::ERR_OK;
}

extern "C" int32_t NotifyPageChanged(const char* targetPageName,
    int32_t targetPageNameLen, int32_t windowId)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    if (targetPageName == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "targetPage null");
        return OHOS::ERR_NO_INIT;
    }
    TAG_LOGI(AAFwkTag::ABILITY, "windowId: %{public}d, targetPageNameLen: %{public}d",
        windowId, targetPageNameLen);
    auto window = OHOS::Rosen::Window::GetWindowWithId(windowId);
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return OHOS::ERR_NO_INIT;
    }
    std::string targetPageNameStr(targetPageName);
    auto& mgr = OHOS::AppExecFwk::PageScalingManager::GetInstance();
    mgr.HandleNavigateChangeInternal(window, "", 0, targetPageNameStr, targetPageNameLen, mgr.GetConfigs());
    return OHOS::ERR_OK;
}

namespace OHOS {
namespace AppExecFwk {
namespace {
static constexpr const char* DPI = "dpi";
static constexpr const char* HIDE_STATUS_BAR = "hsb";
static constexpr const char* VIDEO_SHRINK = "vsk";
static constexpr const char* VERTICAL_DISPLAY = "vd";
static constexpr const char* PAGE_NAME = "pn";
static constexpr const char* PATTERN_TYPE = "pt";
static constexpr const char* INTERNAL = "fullmode";
static constexpr const char* EXTERNAL = "foldmode";
static constexpr const char* HORIZONTAL = "landscape";
static constexpr const char* VERTICAL = "portrait";
static constexpr const char* STAY = "stay";
}

PageScalingManager& PageScalingManager::GetInstance()
{
    static PageScalingManager instance;
    return instance;
}

void PageScalingManager::WindowUIContentImpl::OnUIContentCreate(std::weak_ptr<Ace::UIContent> uiContent)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    auto uiContentPtr = uiContent.lock();
    if (uiContentPtr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "uiContentPtr null");
        return;
    }
    auto window = window_;
    auto configs = configs_;
    uiContentPtr->RegisterNavigateChangeCallback(
            [window, configs, this]
                (const OHOS::Ace::NavigateChangeInfo& from, const OHOS::Ace::NavigateChangeInfo& to) {
            TAG_LOGI(AAFwkTag::ABILITY, "recv navigate callback");
            if (!window) {
                TAG_LOGE(AAFwkTag::ABILITY, "window null");
                return;
            }
            auto& mgr = OHOS::AppExecFwk::PageScalingManager::GetInstance();
            mgr.HandleNavigateChangeInternal(window, from.name, from.name.length(), to.name, to.name.length(), configs);
        });
}

std::vector<PageScalingConfig>& PageScalingManager::GetConfigs()
{
    return configs_;
}

void PageScalingManager::ParseConfigArray(const nlohmann::json& item,
    const std::string& field, std::vector<Config>& dest)
{
    if (!item.contains(field) || !item[field].is_array()) {
        return;
    }

    for (const auto& subItem : item[field]) {
        Config config;
        config.condition = subItem.value("condition", "");
        config.value = subItem.value("val", "");

        dest.push_back(config);

        if (field == DPI) {
            TAG_LOGI(AAFwkTag::ABILITY, "DPI condition: %{public}s, value: %{public}s",
                config.condition.c_str(), config.value.c_str());
        } else if (field == HIDE_STATUS_BAR) {
            TAG_LOGI(AAFwkTag::ABILITY, "HideStatusBar condition: %{public}s, value: %{public}s",
                config.condition.c_str(), config.value.c_str());
        } else if (field == VIDEO_SHRINK) {
            TAG_LOGI(AAFwkTag::ABILITY, "VideoShrink condition: %{public}s, value: %{public}s",
                config.condition.c_str(), config.value.c_str());
        } else if (field == VERTICAL_DISPLAY) {
            TAG_LOGI(AAFwkTag::ABILITY, "VerticalDisplay condition: %{public}s, value: %{public}s",
                config.condition.c_str(), config.value.c_str());
        }
    }
}

int32_t PageScalingManager::ParseWhitelistConfig(
    const std::string &configJson, std::vector<PageScalingConfig> &configs)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    auto parseResult = nlohmann::json::parse(configJson, nullptr, false);
    if (parseResult.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITY, "ParseConfig failed");
        return ERR_INVALID_VALUE;
    }
    if (!parseResult.is_array()) {
        TAG_LOGE(AAFwkTag::ABILITY, "configJson not array");
        return ERR_INVALID_VALUE;
    }
    for (const auto& item : parseResult) {
        if (!item.contains("page") || !item["page"].is_object()) {
            continue;
        }
        PageScalingConfig config;
        if (item["page"].contains(PAGE_NAME) && item["page"][PAGE_NAME].is_string()) {
            config.page.pageName = item["page"][PAGE_NAME];
        } else {
            config.page.pageName = "";
        }
        if (item["page"].contains(PATTERN_TYPE) && item["page"][PATTERN_TYPE].is_string()) {
            config.page.patternType = item["page"][PATTERN_TYPE];
        } else {
            config.page.patternType = "";
        }
        TAG_LOGI(AAFwkTag::ABILITY, "Found config for page %{public}s with pattern %{public}s",
            config.page.pageName.c_str(), config.page.patternType.c_str());
        ParseConfigArray(item, DPI, config.dpi);
        ParseConfigArray(item, HIDE_STATUS_BAR, config.hideStatusBar);
        ParseConfigArray(item, VIDEO_SHRINK, config.videoShrink);
        ParseConfigArray(item, VERTICAL_DISPLAY, config.verticalDisplay);
        configs.push_back(config);
    }
    return ERR_OK;
}

void PageScalingManager::HandleNavigateChangeInternal(const sptr<Rosen::Window>& window,
    const std::string& fromPageName, int32_t fromPageNameLen, const std::string& toPageName,
    int32_t toPageNameLen, const std::vector<PageScalingConfig> &configs)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return;
    }
    auto display = Rosen::DisplayManager::GetInstance().GetDisplayById(window->GetDisplayId());
    if (display == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "display null");
        return;
    }
    if (display->GetDisplayInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "displayInfo null");
        return;
    }
    auto foldDisplayMode = Rosen::DisplayManager::GetInstance().GetFoldDisplayMode();
    auto orientation = display->GetDisplayInfo()->GetDisplayOrientation();
    TAG_LOGD(AAFwkTag::ABILITY, "foldDisplayMode %{public}d, orientation %{public}d",
            foldDisplayMode, orientation);
    for (const auto& config : configs) {
        ProcessDpiConfig(config.dpi, "dpi", foldDisplayMode, window, toPage, toPageNameLen, configPage);
        ProcessHideStatusBarConfig(config.hideStatusBar, "hsb", window, toPage, toPageNameLen, configPage);
        ProcessVideoShrinkConfig(config.videoShrink, "vsk", window, toPage, toPageNameLen, configPage);
        ProcessVerticalDisplayConfig(config.verticalDisplay, "vd", orientation, foldDisplayMode, window,
            fromPage, fromPageNameLen, toPage, toPageNameLen, configPage);
    }
}


void PageScalingManager::ProcessDpiConfig(const std::vector<Config>& configs, const std::string& tag,
    Rosen::FoldDisplayMode foldDisplayMode, const sptr<Rosen::Window>& window,
    const std::string& toPage, int32_t toPageNameLen, const Page& configPage)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    if (configs.empty()) {
        TAG_LOGD(AAFwkTag::ABILITY, "empty");
        return;
    }
    if (!ComparePage(configPage.pageName, toPage, configPage.patternType, toPageNameLen)) {
        TAG_LOGD(AAFwkTag::ABILITY, "toPage not targetPage");
        RestoreDPI(window);
        return;
    } 
    for (const auto& configItem : configs) {
        const std::string& condition = configItem.condition;
        const std::string& value = configItem.value;
        bool isMatch = DpiConditionCheck(condition, foldDisplayMode);
        TAG_LOGI(AAFwkTag::ABILITY, "%{public}s isMatch %{public}d", tag.c_str(), isMatch);
        if (isMatch) {
            float dpiValue = std::stof(value);
            bool applyToSubwindow = false;
            TAG_LOGI(AAFwkTag::ABILITY, "set dpi %{public}f for condition %{public}s",
                dpiValue, condition.c_str());
            auto retDPI = window->SetCustomDensity(dpiValue, applyToSubwindow);
            TAG_LOGD(AAFwkTag::ABILITY, "ret %{public}d", retDPI);
            return;
        }
    }
    RestoreDPI(window);
}

void PageScalingManager::ProcessVerticalDisplayConfig(
    const std::vector<Config>& configs, const std::string& tag,
    Rosen::DisplayOrientation orientation, Rosen::FoldDisplayMode foldDisplayMode,
    const sptr<Rosen::Window>& window, const std::string& fromPage,
    int32_t fromPageNameLen, const std::string& toPage, int32_t toPageNameLen,
    const Page& configPage)
{
    if (configs.empty()) {
        return;
    }
    
    bool hasStayCondition = false;
    
    for (const auto& configItem : configs) {
        const std::string condition = configItem.condition;
        const std::string& value = configItem.value;
        
        // 1. 先识别是否为 Stay 配置（无论设备状态如何）
        if (IsStayCondition(condition)) {
            hasStayCondition = true;
            
            // 2. 再检查设备状态
            bool isConditionMatch = ConditionCheck(condition, orientation, foldDisplayMode);
            if (!isConditionMatch) {
                TAG_LOGE(AAFwkTag::ABILITY, "Condition %{public}s not match ", condition.c_str());
                continue; // 如果条件不匹配，跳过此配置项
            }
            
            bool isEnterTargetPage = ComparePage(configPage.pageName, toPage, 
                configPage.patternType, toPageNameLen) && 
                !ComparePage(configPage.pageName, fromPage, 
                configPage.patternType, fromPageNameLen);
                
            bool isExitTargetPage = ComparePage(configPage.pageName, fromPage, 
                configPage.patternType, fromPageNameLen) && 
                !ComparePage(configPage.pageName, toPage, 
                configPage.patternType, toPageNameLen);
            
            // 3. Stay 逻辑：退出时无需检查设备状态，进入时需检查
            if (isEnterTargetPage) {
                TAG_LOGI(AAFwkTag::ABILITY, "Setting compatible mode for stay condition: entering target page %{public}s, value: %{public}d",
                    configPage.pageName.c_str(), valueInt);
                int32_t valueInt = std::stoi(value);
                window->UpdateCompatibleStyleMode(
                    static_cast<Rosen::CompatibleStyleMode>(valueInt));
                return;
                // 进入但设备不匹配：不设置，继续检查其他配置
            } else if (isExitTargetPage) {
                // 离开页面：无论当前设备状态如何，都恢复
                TAG_LOGI(AAFwkTag::ABILITY, "Restoring vertical display for stay condition: exiting target page %{public}s",
                    configPage.pageName.c_str());
                RestoreVerticalDisplay(window);
                return;
            }
            // 不是进入也不是离开（页面内跳转）：不操作，继续检查其他配置
            
        } else {
            // 4. 非 Stay 逻辑保持原有行为（仅在设备匹配时处理）
            if (!ConditionCheck(condition, orientation, foldDisplayMode)) {
                continue;
            }
            
            if (!ComparePage(configPage.pageName, toPage, 
                configPage.patternType, toPageNameLen)) {
                RestoreVerticalDisplay(window);
                return;
            }
            
            int32_t valueInt = std::stoi(value);
            window->UpdateCompatibleStyleMode(
                static_cast<Rosen::CompatibleStyleMode>(valueInt));
            return;
        }
    }
    
    if (!hasStayCondition) {
        RestoreVerticalDisplay(window);
    }
}
void PageScalingManager::ProcessHideStatusBarConfig(const std::vector<Config>& configs,
    const std::string& tag, const sptr<Rosen::Window>& window,
    const std::string& toPage, int32_t toPageNameLen, const std::string& targetPage)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    if (configs.empty()) {
        TAG_LOGD(AAFwkTag::ABILITY, "empty");
        return;
    }
    if (!ComparePage(configPage.pageName, toPage, configPage.patternType, toPageNameLen)) {
        TAG_LOGD(AAFwkTag::ABILITY, "toPage not targetPage");
        RestoreHideStatusBar(window);
        return;
    }
    for (const auto& configItem : configs) {
        const std::string& condition = configItem.condition;
        const std::string& value = configItem.value;
        bool isMatch = EmptyConditionCheck(condition);
        TAG_LOGI(AAFwkTag::ABILITY, "%{public}s isMatch %{public}d", tag.c_str(), isMatch);
        if (isMatch) {
            auto prop = window->GetSystemBarPropertyByType(Rosen::WindowType::WINDOW_TYPE_STATUS_BAR);
            prop.enable_ = (value == "1") ? 0 : 1;
            TAG_LOGI(AAFwkTag::ABILITY, "set hsb %{public}d for %{public}s",
                prop.enable_, condition.c_str());
            auto retHSB = window->SetSystemBarPropertyForPage(Rosen::WindowType::WINDOW_TYPE_STATUS_BAR, prop);
            TAG_LOGD(AAFwkTag::ABILITY, "ret %{public}d", retHSB);
            return;
        }
    }
    RestoreHideStatusBar(window);
}

void PageScalingManager::ProcessVideoShrinkConfig(const std::vector<Config>& configs,
    const std::string& tag, const sptr<Rosen::Window>& window,
    const std::string& toPage, int32_t toPageNameLen, const std::string& targetPage)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    if (configs.empty()) {
        TAG_LOGD(AAFwkTag::ABILITY, "empty");
        return;
    }
    if (!ComparePage(toPage, targetPage, configPage.patternType, toPageNameLen)) {
        TAG_LOGD(AAFwkTag::ABILITY, "toPage not targetPage");
        RestoreVideoShrink(window);
        return;
    }
    for (const auto& configItem : configs) {
        const std::string& condition = configItem.condition;
        const std::string& value = configItem.value;
        bool isMatch = EmptyConditionCheck(condition);
        TAG_LOGI(AAFwkTag::ABILITY, "%{public}s isMatch %{public}d", tag.c_str(), isMatch);
        if (isMatch) {
            auto uiContent = window->GetUIContent();
            if (uiContent == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITY, "uiContent null");
                return;
            }
            auto enable = (value == "1") ? 1 : 0;
            TAG_LOGI(AAFwkTag::ABILITY, "set vsk %{public}d for %{public}s",
                enable, condition.c_str());
            uiContent->SetXComponentDisplayConstraintEnabled(enable);
            return;
        }
    }
    RestoreVideoShrink(window);
}

void PageScalingManager::RestoreDPI(const sptr<Rosen::Window>& window)
{
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return;
    }
    float defaultDensity = -1.0f;
    auto retDPI = window->SetCustomDensity(defaultDensity, false);
    TAG_LOGD(AAFwkTag::ABILITY, "restore dpi ret %{public}d", retDPI);
}

void PageScalingManager::RestoreHideStatusBar(const sptr<Rosen::Window>& window)
{
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return;
    }
    auto defaultProp = std::nullopt;
    auto retHSB = window->SetSystemBarPropertyForPage(Rosen::WindowType::WINDOW_TYPE_STATUS_BAR, defaultProp);
    TAG_LOGD(AAFwkTag::ABILITY, "restore hsb ret %{public}d", retHSB);
}

void PageScalingManager::RestoreVerticalDisplay(const sptr<Rosen::Window>& window)
{
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return;
    }
    auto retVD = window->UpdateCompatibleStyleMode(Rosen::CompatibleStyleMode::INVALID_VALUE);
    TAG_LOGD(AAFwkTag::ABILITY, "restore vd ret %{public}d", retVD);
}

void PageScalingManager::RestoreVideoShrink(const sptr<Rosen::Window>& window)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    if (window == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "window null");
        return;
    }
    auto uiContent = window->GetUIContent();
    if (uiContent == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "uiContent null");
        return;
    }
    uiContent->SetXComponentDisplayConstraintEnabled(false);
}

bool PageScalingManager::ComparePage(const std::string &whitelistPageName,
    const std::string &targetPageName, const std::string &patternType, int32_t targetPageNameLen)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    size_t whitelistLen = whitelistPageName.length();
    if (patternType == "eq") {
        if (whitelistLen != static_cast<size_t>(targetPageNameLen)) {
            TAG_LOGE(AAFwkTag::ABILITY, "len not equal, targetPage %{public}d, config %{public}zu",
                targetPageNameLen, whitelistLen);
            return false;
        }
        return targetPageName == whitelistPageName;
    } else if (patternType == "prefix") {
        if (static_cast<size_t>(targetPageNameLen) >= whitelistLen) {
            return targetPageName.compare(0, whitelistLen, whitelistPageName) == 0;
        } else {
            return whitelistPageName.compare(0, targetPageNameLen, targetPageName) == 0;
        }
    }
    TAG_LOGE(AAFwkTag::ABILITY, "patternType invalid");
    return false;
}

bool PageScalingManager::EmptyConditionCheck(const std::string& condition)
{
    if (condition.empty()) {
        TAG_LOGD(AAFwkTag::ABILITY, "no condition");
        return true;
    }
    return false;
}

bool PageScalingManager::DpiConditionCheck(const std::string& condition,
    Rosen::FoldDisplayMode foldMode)
{
    if (condition.empty()) {
        return true; // 空条件默认匹配
    }
    if (condition == INTERNAL) {
        return (foldMode == Rosen::FoldDisplayMode::FULL);
    } else if (condition == EXTERNAL) {
        return (foldMode == Rosen::FoldDisplayMode::MAIN);
    }
    TAG_LOGE(AAFwkTag::ABILITY, "invalid condition: %{public}s", condition.c_str());
    return false;
}

bool PageScalingManager::ConditionCheck(const std::string& condition,
    Rosen::DisplayOrientation orientation, Rosen::FoldDisplayMode foldMode)
{
    TAG_LOGD(AAFwkTag::ABILITY, "call");
    // always set when no condition
    if (condition.empty()) {
        TAG_LOGI(AAFwkTag::ABILITY, "no condition");
        return true;
    }

    if (condition.find(",") != std::string::npos) {
        std::vector<std::string> condList = ParseCondition(condition);
        return CheckComboCondition(condList, orientation, foldMode);
    } else {
        return CheckSingleCondition(condition, orientation, foldMode);
    }
}

bool PageScalingManager::CheckSingleCondition(const std::string& condition,
    Rosen::DisplayOrientation orientation, Rosen::FoldDisplayMode foldMode)
{
    if (condition == INTERNAL) {
        return (foldMode == Rosen::FoldDisplayMode::FULL);
    } else if (condition == EXTERNAL) {
        return (foldMode == Rosen::FoldDisplayMode::MAIN);
    } else if (condition == HORIZONTAL) {
        return (orientation == Rosen::DisplayOrientation::LANDSCAPE ||
                orientation == Rosen::DisplayOrientation::LANDSCAPE_INVERTED);
    } else if (condition == VERTICAL) {
        return (orientation == Rosen::DisplayOrientation::PORTRAIT ||
                orientation == Rosen::DisplayOrientation::PORTRAIT_INVERTED);
    } else if (condition == STAY) {
        // STAY条件本身不直接依赖设备状态，所以总是返回true
        // 实际的处理逻辑在ProcessVerticalDisplayConfig中进行
        return true;
    }
    TAG_LOGE(AAFwkTag::ABILITY, "invalid condition");
    return false;
}

bool PageScalingManager::CheckComboCondition(const std::vector<std::string>& condList,
    Rosen::DisplayOrientation orientation, Rosen::FoldDisplayMode foldMode)
{
    for (const auto& condition : condList) {
        if (!CheckSingleCondition(condition, orientation, foldMode)) {
            return false;
        }
    }
    return true;
}


std::vector<std::string> PageScalingManager::ParseCondition(const std::string& condition)
{
    std::vector<std::string> condList;
    size_t pos = 0;
    std::string token;
    std::string conditionCopy = condition;

    while ((pos = conditionCopy.find(",")) != std::string::npos) {
        token = conditionCopy.substr(0, pos);
        condList.push_back(token);
        conditionCopy.erase(0, pos + 1);
    }
    condList.push_back(conditionCopy);
    return condList;
}


bool PageScalingManager::IsStayCondition(const std::string& condition)
{
    if (condition.empty()) {
        return false;
    }
    if (condition.find(",") != std::string::npos) {
        std::vector<std::string> condList = ParseCondition(condition);
        for (const auto& cond : condList) {
            if (cond == STAY) {
                return true;
            }
        }
    } else {
        return condition == STAY;
    }
    return false;
}
} // namespace AppExecFwk
} // namespace OHOS