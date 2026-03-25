/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "app_utils.h"
#include <new>
#include <unordered_set>
#include "json_utils.h"
#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"
#include "parameter.h"
#include "parameters.h"
#ifdef SUPPORT_GRAPHICS
#include "scene_board_judgement.h"
#endif // SUPPORT_GRAPHICS


namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* BUNDLE_NAME_LAUNCHER = "com.ohos.launcher";
constexpr const char* BUNDLE_NAME_SCENEBOARD = "com.ohos.sceneboard";
constexpr const char* LAUNCHER_ABILITY_NAME = "com.ohos.launcher.MainAbility";
constexpr const char* SCENEBOARD_ABILITY_NAME = "com.ohos.sceneboard.MainAbility";
constexpr const char* INHERIT_WINDOW_SPLIT_SCREEN_MODE = "persist.sys.abilityms.inherit_window_split_screen_mode";
constexpr const char* SUPPORT_ANCO_APP = "persist.sys.abilityms.support_anco_app";
constexpr const char* TIMEOUT_UNIT_TIME_RATIO = "persist.sys.abilityms.timeout_unit_time_ratio";
constexpr const char* SELECTOR_DIALOG_POSSION = "persist.sys.abilityms.selector_dialog_possion";
constexpr const char* START_SPECIFIED_PROCESS = "persist.sys.abilityms.start_specified_process";
constexpr const char* USE_MULTI_RENDER_PROCESS = "persist.sys.abilityms.use_multi_render_process";
constexpr const char* LIMIT_MAXIMUM_OF_RENDER_PROCESS = "persist.sys.abilityms.limit_maximum_of_render_process";
constexpr const char* GRANT_PERSIST_URI_PERMISSION = "persist.sys.abilityms.grant_persist_uri_permission";
constexpr const char* GRANT_TEMPORARY_URI_PERMISSION = "persist.sys.abilityms.grant_temporary_uri_permission";
constexpr const char* START_OPTIONS_WITH_ANIMATION = "persist.sys.abilityms.start_options_with_animation";
constexpr const char* SUPPORT_START_ABILITIES = "persist.sys.abilityms.enable_start_abilities";
constexpr const char* MULTI_PROCESS_MODEL = "persist.sys.abilityms.multi_process_model";
constexpr const char* PARAM_ANCO_APP_IDENTIFIER = "persist.hmos_fusion_mgr.anco_identifier";
constexpr const char* ALLOW_CHILD_PROCESS_IN_MULTI_PROCESS_FEATURE_APP =
    "const.sys.abilityms.allow_child_process_in_multi_process_feature_app";
constexpr const char* MAX_MULTI_PROCESS_FEATURE_CHILD_PROCESS =
    "const.sys.abilityms.max_multi_process_feature_child_process";
constexpr const char* START_OPTIONS_WITH_PROCESS_OPTION = "persist.sys.abilityms.start_options_with_process_option";
constexpr const char* MOVE_UI_ABILITY_TO_BACKGROUND_API_ENABLE =
    "persist.sys.abilityms.move_ui_ability_to_background_api_enable";
constexpr const char* CONFIG_PATH = "/etc/ability_runtime/resident_process_in_extreme_memory.json";
constexpr const char* RESIDENT_PROCESS_IN_EXTREME_MEMORY = "residentProcessInExtremeMemory";
constexpr const char* PROCESS_PROHIBITED_FROM_RESTARTING = "processProhibitedFromRestarting";
constexpr const char* BUNDLE_NAME = "bundleName";
constexpr const char* REQUIRE_BIGMEMORY_APP = "requireBigMemoryApp";
constexpr const char* ABILITY_NAME = "abilityName";
constexpr const char* KEY_IDENTIFIER = "identifier";
constexpr const char* ALLOW_NATIVE_CHILD_PROCESS_APPS_CONFIG_PATH =
    "/etc/ability_runtime/allow_native_child_process_apps.json";
constexpr const char* KEY_ALLOW_NATIVE_CHILD_PROCESS_APPS = "allowNativeChildProcessApps";
constexpr const char* LAUNCH_EMBEDED_UI_ABILITY = "const.abilityms.launch_embeded_ui_ability";
const std::string SUPPROT_NATIVE_CHILD_PROCESS = "persist.sys.abilityms.start_native_child_process";
const std::string LIMIT_MAXIMUM_EXTENSIONS_OF_PER_PROCESS =
    "const.sys.abilityms.limit_maximum_extensions_of_per_process";
const std::string LIMIT_MAXIMUM_EXTENSIONS_OF_PER_DEVICE =
    "const.sys.abilityms.limit_maximum_extensions_of_per_device";
const std::string CACHE_EXTENSION_TYPES = "const.sys.abilityms.cache_extension";
constexpr const char* START_ABILITY_WITHOUT_CALLERTOKEN = "/system/etc/start_ability_without_caller_token.json";
constexpr const char* START_ABILITY_WITHOUT_CALLERTOKEN_PATH =
    "/etc/ability_runtime/start_ability_without_caller_token.json";
constexpr const char* START_ABILITY_WITHOUT_CALLERTOKEN_TITLE = "startAbilityWithoutCallerToken";
constexpr const char* BROKER_DELEGATE_BUNDLE_NAME = "const.sys.abilityms.broker_delegate_bundle_name";
constexpr const char* COLLABORATOR_BROKER_UID = "const.sys.abilityms.collaborator_broker_uid";
constexpr const char* COLLABORATOR_BROKER_RESERVE_UID = "const.sys.abilityms.collaborator_broker_reserve_uid";
constexpr const char* MAX_CHILD_PROCESS = "const.max_native_child_process";
constexpr const char* SUPPORT_MULTI_INSTANCE = "const.abilityms.support_multi_instance";
constexpr const char* MIGRATE_CLIENT_BUNDLE_NAME = "const.sys.abilityms.migrate_client_bundle_name";
constexpr const char* CONNECT_SUPPORT_CROSS_USER = "const.abilityms.connect_support_cross_user";
constexpr const char* SUPPORT_APP_SERVICE_EXTENSION = "const.abilityms.support_app_service";
constexpr const char* PRODUCT_PRELOAD_APPLICATION_SETTING_ENABLED = "const.product.preload_application.setting.enabled";
constexpr const char* FORBID_START = "persist.sys.abilityms.forbid_start";
constexpr const char* ALLOW_DEBUG_PERMISSION = "persist.sys.abilityms.allow_debug_permission";
constexpr const char* START_ABILITY_IN_CURRENT_PROCESS = "persist.sys.abilityms.start_ability_in_current_process";
constexpr const char* RESTART_APP_WITH_WINDOW = "persist.sys.abilityms.restart_app_with_window";
constexpr const char* PRODUCT_APPBOOT_SETTING_ENABLED = "const.product.appboot.setting.enabled";
// Support prepare terminate
constexpr int32_t PREPARE_TERMINATE_ENABLE_SIZE = 6;
constexpr const char* PREPARE_TERMINATE_ENABLE_PARAMETER = "persist.sys.prepare_terminate";
constexpr const char* CACHE_ABILITY_BY_LIST_ENABLE = "persist.sys.abilityms.cache_ability_enable";
constexpr const char* CACHE_ABILITY_LIST_PATH = "etc/ability/abilityms_cache_ability.json";
constexpr const char* CACHE_PROCESS_NAME = "cache_list";
constexpr const char* RESIDENT_WHITE_LIST_PATH = "etc/ability_runtime/resident_process.json";
constexpr const char* NORMAL_RESIDENT_APPS = "normal_resident_apps";
constexpr const char* ON_NEW_PROCESS_ENABLE_LIST_PATH = "etc/ability_runtime/on_new_process_enable_list.json";
constexpr const char* ON_NEW_PROCESS_ENABLE_LIST = "onNewProcessEnableList";
constexpr const char* HYBRIDSPAWN_UNIFIED = "persist.appspawn.hybridspawn.unified";
}

AppUtils::~AppUtils() {}

AppUtils::AppUtils()
{
#ifdef SUPPORT_GRAPHICS
    if (Rosen::SceneBoardJudgement::IsSceneBoardEnabled()) {
        isSceneBoard_ = true;
    }
#endif // SUPPORT_GRAPHICS
#ifdef ABILITY_RUNTIME_FORBID_START_ENABLED
    if (WatchParameter(FORBID_START, AppUtils::ForbidStartCallback, nullptr) != 0) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "want param failed: %{public}s", FORBID_START);
        isForbidStart_.isLoaded = false;
    }
#endif
}

template<typename T>
void AppUtils::ResetOnceCacheForTest(T &cache, const T &defaultValue, std::once_flag &flag)
{
    cache = defaultValue;
    flag.~once_flag();
    new (&flag) std::once_flag();
}

template<typename T>
void AppUtils::SetOnceCacheForTest(T &cache, const T &value, std::once_flag &flag)
{
    ResetOnceCacheForTest(cache, value, flag);
    std::call_once(flag, [&cache, value]() {
        cache = value;
    });
}

AppUtils &AppUtils::GetInstance()
{
    static AppUtils utils;
    return utils;
}

void AppUtils::ForbidStartCallback(const char *key, const char *, void *)
{
    if (strcmp(key, FORBID_START) == 0) {
        std::lock_guard lock(GetInstance().isForbidStartMutex_);
        GetInstance().isForbidStart_.value = system::GetBoolParameter(FORBID_START, false);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "param %{public}s changed: %{public}d", FORBID_START,
            GetInstance().isForbidStart_.value);
    }
}

bool AppUtils::IsLauncher(const std::string &bundleName) const
{
    if (isSceneBoard_) {
        return bundleName == BUNDLE_NAME_SCENEBOARD;
    }

    return bundleName == BUNDLE_NAME_LAUNCHER;
}

bool AppUtils::IsLauncherAbility(const std::string &abilityName) const
{
    if (isSceneBoard_) {
        return abilityName == SCENEBOARD_ABILITY_NAME;
    }

    return abilityName == LAUNCHER_ABILITY_NAME;
}

bool AppUtils::IsInheritWindowSplitScreenMode()
{
    std::call_once(isInheritWindowSplitScreenModeOnce, [this]() {
        isInheritWindowSplitScreenMode_ = system::GetBoolParameter(INHERIT_WINDOW_SPLIT_SCREEN_MODE, true);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isInheritWindowSplitScreenMode_);
    return isInheritWindowSplitScreenMode_;
}

bool AppUtils::IsSupportAncoApp()
{
    std::call_once(isSupportAncoAppOnce, [this]() {
        isSupportAncoApp_ = system::GetBoolParameter(SUPPORT_ANCO_APP, false);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isSupportAncoApp_);
    return isSupportAncoApp_;
}

int32_t AppUtils::GetTimeoutUnitTimeRatio()
{
    std::call_once(timeoutUnitTimeRatioOnce, [this]() {
        timeoutUnitTimeRatio_ = system::GetIntParameter<int32_t>(TIMEOUT_UNIT_TIME_RATIO, 1);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", timeoutUnitTimeRatio_);
    return timeoutUnitTimeRatio_;
}

bool AppUtils::IsSelectorDialogDefaultPossion()
{
    std::call_once(isSelectorDialogDefaultPossionOnce, [this]() {
        isSelectorDialogDefaultPossion_ = system::GetBoolParameter(SELECTOR_DIALOG_POSSION, true);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isSelectorDialogDefaultPossion_);
    return isSelectorDialogDefaultPossion_;
}

bool AppUtils::IsStartSpecifiedProcess()
{
    std::call_once(isStartSpecifiedProcessOnce, [this]() {
        isStartSpecifiedProcess_ = system::GetBoolParameter(START_SPECIFIED_PROCESS, false);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isStartSpecifiedProcess_);
    return isStartSpecifiedProcess_;
}

bool AppUtils::IsUseMultiRenderProcess()
{
    std::lock_guard lock(isUseMultiRenderProcessMutex_);
    if (!isUseMultiRenderProcess_.isLoaded) {
        isUseMultiRenderProcess_.value = system::GetBoolParameter(USE_MULTI_RENDER_PROCESS, true);
        isUseMultiRenderProcess_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isUseMultiRenderProcess_.value);
    return isUseMultiRenderProcess_.value;
}

bool AppUtils::IsLimitMaximumOfRenderProcess()
{
    std::call_once(isLimitMaximumOfRenderProcessOnce, [this]() {
        isLimitMaximumOfRenderProcess_ = system::GetBoolParameter(LIMIT_MAXIMUM_OF_RENDER_PROCESS, true);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isLimitMaximumOfRenderProcess_);
    return isLimitMaximumOfRenderProcess_;
}

bool AppUtils::IsGrantPersistUriPermission()
{
    std::lock_guard lock(isGrantPersistUriPermissionMutex_);
    if (!isGrantPersistUriPermission_.isLoaded) {
        isGrantPersistUriPermission_.value = system::GetBoolParameter(GRANT_PERSIST_URI_PERMISSION, false);
        isGrantPersistUriPermission_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isGrantPersistUriPermission_.value);
    return isGrantPersistUriPermission_.value;
}

bool AppUtils::IsSupportGrantUriPermission()
{
    std::lock_guard lock(isGrantTempUriPermissionMutex_);
    if (!isGrantTempUriPermission_.isLoaded) {
        isGrantTempUriPermission_.value = system::GetBoolParameter(GRANT_TEMPORARY_URI_PERMISSION, true);
        isGrantTempUriPermission_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isGrantTempUriPermission_.value);
    return isGrantTempUriPermission_.value;
}

bool AppUtils::IsStartOptionsWithAnimation()
{
    std::call_once(isStartOptionsWithAnimationOnce, [this]() {
        isStartOptionsWithAnimation_ = system::GetBoolParameter(START_OPTIONS_WITH_ANIMATION, false);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isStartOptionsWithAnimation_);
    return isStartOptionsWithAnimation_;
}

bool AppUtils::IsSupportStartAbilities()
{
    std::lock_guard lock(isSupportStartAbilitiesMutex_);
    if (!isSupportStartAbilities_.isLoaded) {
        isSupportStartAbilities_.value = system::GetBoolParameter(SUPPORT_START_ABILITIES, false);
        isSupportStartAbilities_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isSupportStartAbilities_.value);
    return isSupportStartAbilities_.value;
}

bool AppUtils::IsMultiProcessModel()
{
    std::call_once(isMultiProcessModelOnce, [this]() {
        isMultiProcessModel_ = system::GetBoolParameter(MULTI_PROCESS_MODEL, false);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isMultiProcessModel_);
    return isMultiProcessModel_;
}

bool AppUtils::AllowChildProcessInMultiProcessFeatureApp()
{
    std::call_once(allowChildProcessInMultiProcessFeatureAppOnce, [this]() {
        allowChildProcessInMultiProcessFeatureApp_ =
            system::GetBoolParameter(ALLOW_CHILD_PROCESS_IN_MULTI_PROCESS_FEATURE_APP, false);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "AllowChildProcessInMultiProcessFeatureApp:%{public}d",
        allowChildProcessInMultiProcessFeatureApp_);
    return allowChildProcessInMultiProcessFeatureApp_;
}

bool AppUtils::IsStartOptionsWithProcessOptions()
{
    std::lock_guard lock(isStartOptionsWithProcessOptionsMutex_);
    if (!isStartOptionsWithProcessOptions_.isLoaded) {
        isStartOptionsWithProcessOptions_.value = system::GetBoolParameter(START_OPTIONS_WITH_PROCESS_OPTION, false);
        isStartOptionsWithProcessOptions_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isStartOptionsWithProcessOptions_.value);
    return isStartOptionsWithProcessOptions_.value;
}

bool AppUtils::EnableMoveUIAbilityToBackgroundApi()
{
    std::call_once(enableMoveUIAbilityToBackgroundApiOnce, [this]() {
        enableMoveUIAbilityToBackgroundApi_ =
            system::GetBoolParameter(MOVE_UI_ABILITY_TO_BACKGROUND_API_ENABLE, true);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", enableMoveUIAbilityToBackgroundApi_);
    return enableMoveUIAbilityToBackgroundApi_;
}

bool AppUtils::IsLaunchEmbededUIAbility()
{
    std::lock_guard lock(isLaunchEmbededUIAbilityMutex_);
    if (!isLaunchEmbededUIAbility_.isLoaded) {
        isLaunchEmbededUIAbility_.value = system::GetBoolParameter(LAUNCH_EMBEDED_UI_ABILITY, false);
        isLaunchEmbededUIAbility_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isLaunchEmbededUIAbility_.value);
    return isLaunchEmbededUIAbility_.value;
}

bool AppUtils::IsSupportNativeChildProcess()
{
    std::call_once(isSupportNativeChildProcessOnce, [this]() {
        isSupportNativeChildProcess_ = system::GetBoolParameter(SUPPROT_NATIVE_CHILD_PROCESS, false);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isSupportNativeChildProcess_);
    return isSupportNativeChildProcess_;
}

bool AppUtils::IsAllowResidentInExtremeMemory(const std::string& bundleName, const std::string& abilityName)
{
    std::lock_guard lock(residentProcessInExtremeMemoryMutex_);
    if (!residentProcessInExtremeMemory_.isLoaded) {
        LoadResidentProcessInExtremeMemory();
        residentProcessInExtremeMemory_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isSupportNativeChildProcess_);
    for (auto &element : residentProcessInExtremeMemory_.value) {
        if (bundleName == element.first &&
            (abilityName == "" || abilityName == element.second)) {
            return true;
        }
    }
    return false;
}

bool AppUtils::IsBigMemoryUnrelatedKeepAliveProc(const std::string &bundleName)
{
    std::lock_guard lock(processProhibitedFromRestartingMutex_);
    if (!processProhibitedFromRestarting_.isLoaded) {
        LoadProcessProhibitedFromRestarting();
        processProhibitedFromRestarting_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "loadJson about processProhibitedFromRestarting %{public}d",
        processProhibitedFromRestarting_.isLoaded);
    for (auto &element : processProhibitedFromRestarting_.value) {
        if (bundleName == element) {
            return true;
        }
    }
    return false;
}

bool AppUtils::IsRequireBigMemoryProcess(const std::string &bundleName)
{
    std::lock_guard lock(requireBigMemoryAppMutex_);
    if (!requireBigMemoryApp_.isLoaded) {
        LoadRequireBigMemoryApp();
        requireBigMemoryApp_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "loadJson about requireBigMemoryApp %{public}d",
        requireBigMemoryApp_.isLoaded);
    for (auto &element : requireBigMemoryApp_.value) {
        if (bundleName == element) {
            return true;
        }
    }
    return false;
}

void AppUtils::LoadProcessProhibitedFromRestarting()
{
    nlohmann::json object;
    if (!JsonUtils::GetInstance().LoadConfiguration(CONFIG_PATH, object)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "process prohibited invalid");
        return;
    }
    if (!object.contains(PROCESS_PROHIBITED_FROM_RESTARTING) ||
        !object.at(PROCESS_PROHIBITED_FROM_RESTARTING).is_array()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "process prohibited invalid.");
        return;
    }

    for (auto &item : object.at(PROCESS_PROHIBITED_FROM_RESTARTING).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.contains(BUNDLE_NAME) || !jsonObject.at(BUNDLE_NAME).is_string()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "load bundleName failed");
            return;
        }
        std::string bundleName = jsonObject.at(BUNDLE_NAME).get<std::string>();
        processProhibitedFromRestarting_.value.emplace_back(bundleName);
    }
}

void AppUtils::LoadRequireBigMemoryApp()
{
    nlohmann::json object;
    if (!JsonUtils::GetInstance().LoadConfiguration(CONFIG_PATH, object)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "process prohibited invalid");
        return;
    }
    if (!object.contains(REQUIRE_BIGMEMORY_APP) ||
        !object.at(REQUIRE_BIGMEMORY_APP).is_array()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "process prohibited invalid.");
        return;
    }

    for (auto &item : object.at(REQUIRE_BIGMEMORY_APP).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.contains(BUNDLE_NAME) || !jsonObject.at(BUNDLE_NAME).is_string()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "load bundleName failed");
            return;
        }
        std::string bundleName = jsonObject.at(BUNDLE_NAME).get<std::string>();
        requireBigMemoryApp_.value.emplace_back(bundleName);
    }
}

void AppUtils::LoadResidentProcessInExtremeMemory()
{
    nlohmann::json object;
    if (!JsonUtils::GetInstance().LoadConfiguration(CONFIG_PATH, object)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "resident process failed");
        return;
    }
    if (!object.contains(RESIDENT_PROCESS_IN_EXTREME_MEMORY) ||
        !object.at(RESIDENT_PROCESS_IN_EXTREME_MEMORY).is_array()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "resident process invalid");
        return;
    }

    for (auto &item : object.at(RESIDENT_PROCESS_IN_EXTREME_MEMORY).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.contains(BUNDLE_NAME) || !jsonObject.at(BUNDLE_NAME).is_string()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "load bundleName failed");
            return;
        }
        if (!jsonObject.contains(ABILITY_NAME) || !jsonObject.at(ABILITY_NAME).is_string()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "load abilityName failed");
            return;
        }
        std::string bundleName = jsonObject.at(BUNDLE_NAME).get<std::string>();
        std::string abilityName = jsonObject.at(ABILITY_NAME).get<std::string>();
        residentProcessInExtremeMemory_.value.emplace_back(std::make_pair(bundleName, abilityName));
    }
}

bool AppUtils::IsAllowNativeChildProcess(const std::string &appIdentifier)
{
    TAG_LOGD(AAFwkTag::DEFAULT, "appId:%{private}s", appIdentifier.c_str());
    std::lock_guard lock(allowStartNativeProcessAppsMutex_);
    if (!allowStartNativeProcessApps_.isLoaded) {
        LoadAllowNativeChildProcessApps();
        allowStartNativeProcessApps_.isLoaded = true;
    }
    auto &apps = allowStartNativeProcessApps_.value;
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}zu", apps.size());
    return std::find(apps.begin(), apps.end(), appIdentifier) != apps.end();
}

void AppUtils::LoadAllowNativeChildProcessApps()
{
    nlohmann::json object;
    if (!JsonUtils::GetInstance().LoadConfiguration(ALLOW_NATIVE_CHILD_PROCESS_APPS_CONFIG_PATH, object)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "load child process config failed");
        return;
    }
    if (!object.contains(KEY_ALLOW_NATIVE_CHILD_PROCESS_APPS) ||
        !object.at(KEY_ALLOW_NATIVE_CHILD_PROCESS_APPS).is_array()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get key invalid");
        return;
    }

    for (auto &item : object.at(KEY_ALLOW_NATIVE_CHILD_PROCESS_APPS).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.contains(KEY_IDENTIFIER) || !jsonObject.at(KEY_IDENTIFIER).is_string()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "load identifier failed");
            return;
        }
        std::string identifier = jsonObject.at(KEY_IDENTIFIER).get<std::string>();
        allowStartNativeProcessApps_.value.emplace_back(identifier);
    }
}

int32_t AppUtils::GetLimitMaximumExtensionsPerProc()
{
    std::call_once(limitMaximumExtensionsPerProcOnce, [this]() {
        limitMaximumExtensionsPerProc_ =
            system::GetIntParameter<int32_t>(LIMIT_MAXIMUM_EXTENSIONS_OF_PER_PROCESS, DEFAULT_MAX_EXT_PER_PROC);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "limitMaximumExtensionsPerProc: %{public}d", limitMaximumExtensionsPerProc_);
    return limitMaximumExtensionsPerProc_;
}

int32_t AppUtils::GetLimitMaximumExtensionsPerDevice()
{
    std::call_once(limitMaximumExtensionsPerDeviceOnce, [this]() {
        limitMaximumExtensionsPerDevice_ =
            system::GetIntParameter<int32_t>(LIMIT_MAXIMUM_EXTENSIONS_OF_PER_DEVICE, DEFAULT_MAX_EXT_PER_DEV);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "limitMaximumExtensionsPerDevice: %{public}d", limitMaximumExtensionsPerDevice_);
    return limitMaximumExtensionsPerDevice_;
}

std::string AppUtils::GetCacheExtensionTypeList()
{
    std::string cacheExtAbilityTypeList = system::GetParameter(CACHE_EXTENSION_TYPES, "260");
    TAG_LOGD(AAFwkTag::DEFAULT, "cacheExtAbilityTypeList is %{public}s", cacheExtAbilityTypeList.c_str());
    return cacheExtAbilityTypeList;
}

std::string AppUtils::GetAncoAppIdentifiers()
{
    std::string identifiers = system::GetParameter(PARAM_ANCO_APP_IDENTIFIER, "");
    return identifiers;
}

bool AppUtils::IsAllowStartAbilityWithoutCallerToken(const std::string& bundleName, const std::string& abilityName)
{
    std::lock_guard lock(startAbilityWithoutCallerTokenMutex_);
    if (!startAbilityWithoutCallerToken_.isLoaded) {
        LoadStartAbilityWithoutCallerToken();
        startAbilityWithoutCallerToken_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "isLoaded: %{public}d", startAbilityWithoutCallerToken_.isLoaded);
    for (auto &element : startAbilityWithoutCallerToken_.value) {
        if (bundleName == element.first && abilityName == element.second) {
            TAG_LOGI(AAFwkTag::DEFAULT, "call");
            return true;
        }
    }
    return false;
}

void AppUtils::LoadStartAbilityWithoutCallerToken()
{
    nlohmann::json object;
    if (!JsonUtils::GetInstance().LoadConfiguration(
        START_ABILITY_WITHOUT_CALLERTOKEN_PATH, object, START_ABILITY_WITHOUT_CALLERTOKEN)) {
        TAG_LOGE(AAFwkTag::DEFAULT, "token list failed");
        return;
    }
    if (!object.contains(START_ABILITY_WITHOUT_CALLERTOKEN_TITLE) ||
        !object.at(START_ABILITY_WITHOUT_CALLERTOKEN_TITLE).is_array()) {
        TAG_LOGE(AAFwkTag::DEFAULT, "token config invalid");
        return;
    }

    for (auto &item : object.at(START_ABILITY_WITHOUT_CALLERTOKEN_TITLE).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.contains(BUNDLE_NAME) || !jsonObject.at(BUNDLE_NAME).is_string()) {
            TAG_LOGE(AAFwkTag::DEFAULT, "load bundleName failed");
            return;
        }
        if (!jsonObject.contains(ABILITY_NAME) || !jsonObject.at(ABILITY_NAME).is_string()) {
            TAG_LOGE(AAFwkTag::DEFAULT, "load abilityName failed");
            return;
        }
        std::string bundleName = jsonObject.at(BUNDLE_NAME).get<std::string>();
        std::string abilityName = jsonObject.at(ABILITY_NAME).get<std::string>();
        startAbilityWithoutCallerToken_.value.emplace_back(std::make_pair(bundleName, abilityName));
    }
}

std::string AppUtils::GetBrokerDelegateBundleName()
{
    std::call_once(brokerDelegateBundleNameOnce, [this]() {
        brokerDelegateBundleName_ = system::GetParameter(BROKER_DELEGATE_BUNDLE_NAME, "");
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "shellAssistantBundleName_ is %{public}s", brokerDelegateBundleName_.c_str());
    return brokerDelegateBundleName_;
}

int32_t AppUtils::GetCollaboratorBrokerUID()
{
    std::call_once(collaboratorBrokerUidOnce, [this]() {
        collaboratorBrokerUid_ = system::GetIntParameter(COLLABORATOR_BROKER_UID, DEFAULT_INVALID_VALUE);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "collaboratorBrokerUid_ is %{public}d", collaboratorBrokerUid_);
    return collaboratorBrokerUid_;
}

int32_t AppUtils::GetCollaboratorBrokerReserveUID()
{
    std::call_once(collaboratorBrokerReserveUidOnce, [this]() {
        collaboratorBrokerReserveUid_ = system::GetIntParameter(COLLABORATOR_BROKER_RESERVE_UID,
            DEFAULT_INVALID_VALUE);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "collaboratorBrokerReserveUid_ is %{public}d", collaboratorBrokerReserveUid_);
    return collaboratorBrokerReserveUid_;
}

int32_t AppUtils::MaxChildProcess()
{
    std::lock_guard lock(maxChildProcessMutex_);
    if (!maxChildProcess_.isLoaded) {
        maxChildProcess_.value =
            system::GetIntParameter<int32_t>(MAX_CHILD_PROCESS, DEFAULT_MAX_CHILD_PROCESS);
        maxChildProcess_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "MaxChildProcess: %{public}d", maxChildProcess_.value);
    return maxChildProcess_.value;
}

int32_t AppUtils::MaxMultiProcessFeatureChildProcess()
{
    std::lock_guard lock(maxMultiProcessFeatureChildProcessMutex_);
    if (!maxMultiProcessFeatureChildProcess_.isLoaded) {
        maxMultiProcessFeatureChildProcess_.value =
            system::GetIntParameter<int32_t>(MAX_MULTI_PROCESS_FEATURE_CHILD_PROCESS, DEFAULT_MAX_CHILD_PROCESS);
            maxMultiProcessFeatureChildProcess_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "maxMultiProcessFeatureChildProcess: %{public}d",
        maxMultiProcessFeatureChildProcess_.value);
    return maxMultiProcessFeatureChildProcess_.value;
}

bool AppUtils::IsSupportMultiInstance()
{
    std::call_once(isSupportMultiInstanceOnce, [this]() {
        isSupportMultiInstance_ = system::GetBoolParameter(SUPPORT_MULTI_INSTANCE, false);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isSupportMultiInstance_);
    return isSupportMultiInstance_;
}

std::string AppUtils::GetMigrateClientBundleName()
{
    std::lock_guard lock(migrateClientBundleNameMutex_);
    if (!migrateClientBundleName_.isLoaded) {
        migrateClientBundleName_.value = system::GetParameter(MIGRATE_CLIENT_BUNDLE_NAME, "");
        migrateClientBundleName_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "migrateClientBundleName_ is %{public}s", migrateClientBundleName_.value.c_str());
    return migrateClientBundleName_.value;
}

bool AppUtils::IsConnectSupportCrossUser()
{
    std::call_once(isConnectSupportCrossUserOnce, [this]() {
        isConnectSupportCrossUser_ = system::GetBoolParameter(CONNECT_SUPPORT_CROSS_USER, false);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isConnectSupportCrossUser_);
    return isConnectSupportCrossUser_;
}

bool AppUtils::IsPrepareTerminateEnabled()
{
    char value[PREPARE_TERMINATE_ENABLE_SIZE] = "false";
    int retSysParam = GetParameter(PREPARE_TERMINATE_ENABLE_PARAMETER, "false", value, PREPARE_TERMINATE_ENABLE_SIZE);
    TAG_LOGI(AAFwkTag::DEFAULT,
        "isPrepareTerminateEnable, %{public}s value=%{public}s", PREPARE_TERMINATE_ENABLE_PARAMETER, value);
    if (retSysParam > 0 && !std::strcmp(value, "true")) {
        return true;
    }
    return false;
}

bool AppUtils::IsSystemReasonMessage(const std::string &reasonMessage)
{
    const std::unordered_set<std::string> systemReasonMessagesSet = {
        "ReasonMessage_SystemShare",
        "ReasonMessage_DesktopShortcut",
        "ReasonMessage_Notification",
        "ReasonMessage_DesktopIcon",
    };
    return systemReasonMessagesSet.find(reasonMessage) != systemReasonMessagesSet.end();
}

bool AppUtils::IsCacheAbilityEnabled()
{
    return system::GetBoolParameter(CACHE_ABILITY_BY_LIST_ENABLE, false);
}

void AppUtils::LoadCacheAbilityList()
{
    nlohmann::json object;
    if (!JsonUtils::GetInstance().LoadConfiguration(CACHE_ABILITY_LIST_PATH, object)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "load cache_ability file failed");
        return;
    }
    if (!object.contains(CACHE_PROCESS_NAME) ||
        !object.at(CACHE_PROCESS_NAME).is_array()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "cache_ability file invalid");
        return;
    }

    for (auto &item : object.at(CACHE_PROCESS_NAME).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.contains(BUNDLE_NAME) || !jsonObject.at(BUNDLE_NAME).is_string()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "load cache_ability bundleName failed");
            return;
        }
        if (!jsonObject.contains(ABILITY_NAME) || !jsonObject.at(ABILITY_NAME).is_string()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "load cache_ability abilityName failed");
            return;
        }
        std::string bundleName = jsonObject.at(BUNDLE_NAME).get<std::string>();
        std::string abilityName = jsonObject.at(ABILITY_NAME).get<std::string>();
        cacheAbilityList_.value.emplace_back(std::make_pair(bundleName, abilityName));
    }
}

bool AppUtils::IsCacheExtensionAbilityByList(const std::string& bundleName, const std::string& abilityName)
{
    if (!cacheAbilityList_.isLoaded) {
        std::lock_guard lock(cacheAbilityListMutex_);
        if (!cacheAbilityList_.isLoaded) {
            LoadCacheAbilityList();
            cacheAbilityList_.isLoaded = true;
        }
    }

    if (cacheAbilityList_.value.empty() || !IsCacheAbilityEnabled()) {
        return false;
    }

    for (auto &element : cacheAbilityList_.value) {
        if (bundleName == element.first && abilityName == element.second) {
            TAG_LOGI(AAFwkTag::DEFAULT, "cache_ability: %{public}s, %{public}s",
                bundleName.c_str(), abilityName.c_str());
            return true;
        }
    }
    return false;
}

void AppUtils::LoadResidentWhiteList()
{
    nlohmann::json object;
    if (!JsonUtils::GetInstance().LoadConfiguration(RESIDENT_WHITE_LIST_PATH, object)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "load resident white list file failed");
        return;
    }
    if (!object.contains(NORMAL_RESIDENT_APPS) ||
        !object.at(NORMAL_RESIDENT_APPS).is_array()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "resident white list file invalid");
        return;
    }

    for (auto &item : object.at(NORMAL_RESIDENT_APPS).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.is_string()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "load resident white bundleName failed");
            return;
        }
        residentWhiteList_.value.emplace_back(jsonObject.get<std::string>());
    }
}

const std::vector<std::string>& AppUtils::GetResidentWhiteList()
{
    std::lock_guard lock(residentWhiteListMutex_);
    if (!residentWhiteList_.isLoaded) {
        LoadResidentWhiteList();
        residentWhiteList_.isLoaded = true;
    }
    return residentWhiteList_.value;
}

bool AppUtils::InResidentWhiteList(const std::string &bundleName)
{
    std::lock_guard lock(residentWhiteListMutex_);
    if (!residentWhiteList_.isLoaded) {
        LoadResidentWhiteList();
        residentWhiteList_.isLoaded = true;
    }
    for (const auto &item: residentWhiteList_.value) {
        if (bundleName == item) {
            return true;
        }
    }
    return false;
}

void AppUtils::LoadOnNewProcessEnableList()
{
    nlohmann::json object;
    if (!JsonUtils::GetInstance().LoadConfiguration(ON_NEW_PROCESS_ENABLE_LIST_PATH, object)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "load onNewProcessEnableList file failed");
        return;
    }
    if (!object.contains(ON_NEW_PROCESS_ENABLE_LIST) ||
        !object.at(ON_NEW_PROCESS_ENABLE_LIST).is_array()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "onNewProcessEnableList file invalid");
        return;
    }
    for (auto &item : object.at(ON_NEW_PROCESS_ENABLE_LIST).items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.is_string()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "load onNewProcessEnableList bundleName failed");
            return;
        }
        onNewProcessEnableList_.value.emplace_back(jsonObject.get<std::string>());
    }
}

bool AppUtils::InOnNewProcessEnableList(const std::string &bundleName)
{
    std::lock_guard lock(onNewProcessEnableListMutex_);
    if (!onNewProcessEnableList_.isLoaded) {
        LoadOnNewProcessEnableList();
        onNewProcessEnableList_.isLoaded = true;
    }
    for (const auto &item: onNewProcessEnableList_.value) {
        if (bundleName == item) {
            return true;
        }
    }
    return false;
}

bool AppUtils::IsSupportAppServiceExtension()
{
    std::call_once(isSupportAppServiceExtensionOnce, [this]() {
        isSupportAppServiceExtension_ = system::GetBoolParameter(SUPPORT_APP_SERVICE_EXTENSION, false);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isSupportAppServiceExtension_);
    return isSupportAppServiceExtension_;
}

bool AppUtils::IsPreloadApplicationEnabled()
{
    std::call_once(isPreloadApplicationEnabledOnce, [this]() {
        isPreloadApplicationEnabled_ = system::GetBoolParameter(PRODUCT_PRELOAD_APPLICATION_SETTING_ENABLED, false);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "called %{public}d", isPreloadApplicationEnabled_);
    return isPreloadApplicationEnabled_;
}

bool AppUtils::IsForbidStart()
{
#ifndef ABILITY_RUNTIME_FORBID_START_ENABLED
    return false;
#else
    std::lock_guard lock(isForbidStartMutex_);
    if (!isForbidStart_.isLoaded) {
        isForbidStart_.value = system::GetBoolParameter(FORBID_START, false);
        isForbidStart_.isLoaded = true;
    }
    return isForbidStart_.value;
#endif
}

bool AppUtils::IsSupportRestartAppWithWindow()
{
    std::call_once(isSupportRestartAppWithWindowOnce, [this]() {
        isSupportRestartAppWithWindow_ = system::GetBoolParameter(RESTART_APP_WITH_WINDOW, false);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "restartApp: %{public}d", isSupportRestartAppWithWindow_);
    return isSupportRestartAppWithWindow_;
}

bool AppUtils::IsSupportAllowDebugPermission()
{
    std::call_once(isSupportAllowDebugPermissionOnce, [this]() {
        isSupportAllowDebugPermission_ = system::GetBoolParameter(ALLOW_DEBUG_PERMISSION, false);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "restartApp: %{public}d", isSupportAllowDebugPermission_);
    return isSupportAllowDebugPermission_;
}

bool AppUtils::IsStartUIAbilityInCurrentProcess()
{
    std::lock_guard lock(isStartUIAbilityInCurrentProcessMutex_);
    if (!isStartUIAbilityInCurrentProcess_.isLoaded) {
        isStartUIAbilityInCurrentProcess_.value = system::GetBoolParameter(START_ABILITY_IN_CURRENT_PROCESS, false);
        isStartUIAbilityInCurrentProcess_.isLoaded = true;
    }
    TAG_LOGD(AAFwkTag::DEFAULT, "startAbilityInCurrentProcess: %{public}d", isStartUIAbilityInCurrentProcess_.value);
    return isStartUIAbilityInCurrentProcess_.value;
}

bool AppUtils::IsProductAppbootSettingEnabled()
{
    std::call_once(isProductAppbootSettingEnabledOnce, [this]() {
        isProductAppbootSettingEnabled_ = system::GetBoolParameter(PRODUCT_APPBOOT_SETTING_ENABLED, false);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "ProductAppbootSettingEnabled: %{public}d", isProductAppbootSettingEnabled_);
    return isProductAppbootSettingEnabled_;
}

void AppUtils::LoadAppTransferList()
{
    nlohmann::json object;
    if (!JsonUtils::GetInstance().LoadConfiguration("/system/etc/ability_runtime/app_transfer_list.json", object)) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "LoadAppTransferList failed");
        return;
    }
    if (!object.contains("appTransferList") ||
        !object.at("appTransferList").is_array()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "appTransferList file invalid");
        return;
    }
    for (auto &item : object.at("appTransferList").items()) {
        const nlohmann::json& jsonObject = item.value();
        if (!jsonObject.is_string()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "appTransferList bundleName failed");
            return;
        }
        appTransferList_.value.emplace_back(jsonObject.get<std::string>());
    }
}

bool AppUtils::InAppTransferList(const std::string &bundleName)
{
    std::lock_guard lock(appTransferListMutex_);
    if (!appTransferList_.isLoaded) {
        LoadAppTransferList();
        appTransferList_.isLoaded = true;
    }
    for (const auto &item: appTransferList_.value) {
        if (bundleName == item) {
            return true;
        }
    }
    return false;
}

bool AppUtils::IsHybridSpawnUnified()
{
    std::call_once(isHybridSpawnUnifiedOnce, [this]() {
        isHybridSpawnUnified_ = system::GetBoolParameter(HYBRIDSPAWN_UNIFIED, false);
    });
    TAG_LOGD(AAFwkTag::DEFAULT, "IsHybridSpawnUnified: %{public}d", isHybridSpawnUnified_);
    return isHybridSpawnUnified_;
}

void AppUtils::ResetSupportNativeChildProcessCacheForTest()
{
    ResetOnceCacheForTest(isSupportNativeChildProcess_, false, isSupportNativeChildProcessOnce);
}

void AppUtils::SetSupportNativeChildProcessCacheForTest(bool value)
{
    SetOnceCacheForTest(isSupportNativeChildProcess_, value, isSupportNativeChildProcessOnce);
}

void AppUtils::ResetSupportAppServiceExtensionCacheForTest()
{
    ResetOnceCacheForTest(isSupportAppServiceExtension_, false, isSupportAppServiceExtensionOnce);
}

void AppUtils::SetSupportAppServiceExtensionCacheForTest(bool value)
{
    SetOnceCacheForTest(isSupportAppServiceExtension_, value, isSupportAppServiceExtensionOnce);
}

void AppUtils::ResetPreloadApplicationEnabledCacheForTest()
{
    ResetOnceCacheForTest(isPreloadApplicationEnabled_, false, isPreloadApplicationEnabledOnce);
}

void AppUtils::SetPreloadApplicationEnabledCacheForTest(bool value)
{
    SetOnceCacheForTest(isPreloadApplicationEnabled_, value, isPreloadApplicationEnabledOnce);
}

void AppUtils::ResetSupportRestartAppWithWindowCacheForTest()
{
    ResetOnceCacheForTest(isSupportRestartAppWithWindow_, false, isSupportRestartAppWithWindowOnce);
}

void AppUtils::SetSupportRestartAppWithWindowCacheForTest(bool value)
{
    SetOnceCacheForTest(isSupportRestartAppWithWindow_, value, isSupportRestartAppWithWindowOnce);
}

void AppUtils::ResetSupportAllowDebugPermissionCacheForTest()
{
    ResetOnceCacheForTest(isSupportAllowDebugPermission_, false, isSupportAllowDebugPermissionOnce);
}

void AppUtils::SetSupportAllowDebugPermissionCacheForTest(bool value)
{
    SetOnceCacheForTest(isSupportAllowDebugPermission_, value, isSupportAllowDebugPermissionOnce);
}

void AppUtils::ResetStartSpecifiedProcessCacheForTest()
{
    ResetOnceCacheForTest(isStartSpecifiedProcess_, false, isStartSpecifiedProcessOnce);
}

void AppUtils::SetStartSpecifiedProcessCacheForTest(bool value)
{
    SetOnceCacheForTest(isStartSpecifiedProcess_, value, isStartSpecifiedProcessOnce);
}

void AppUtils::ResetMultiProcessModelCacheForTest()
{
    ResetOnceCacheForTest(isMultiProcessModel_, false, isMultiProcessModelOnce);
}

void AppUtils::SetMultiProcessModelCacheForTest(bool value)
{
    SetOnceCacheForTest(isMultiProcessModel_, value, isMultiProcessModelOnce);
}

void AppUtils::ResetAllowChildProcessInMultiProcessFeatureAppCacheForTest()
{
    ResetOnceCacheForTest(allowChildProcessInMultiProcessFeatureApp_, false,
        allowChildProcessInMultiProcessFeatureAppOnce);
}

void AppUtils::SetAllowChildProcessInMultiProcessFeatureAppCacheForTest(bool value)
{
    SetOnceCacheForTest(allowChildProcessInMultiProcessFeatureApp_, value,
        allowChildProcessInMultiProcessFeatureAppOnce);
}

void AppUtils::ResetStartOptionsWithAnimationCacheForTest()
{
    ResetOnceCacheForTest(isStartOptionsWithAnimation_, false, isStartOptionsWithAnimationOnce);
}

void AppUtils::SetStartOptionsWithAnimationCacheForTest(bool value)
{
    SetOnceCacheForTest(isStartOptionsWithAnimation_, value, isStartOptionsWithAnimationOnce);
}
}  // namespace AAFwk
}  // namespace OHOS
