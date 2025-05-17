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

#include "app_startup_profile_parser.h"
#include "event_report.h"
#include "extractor.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* PROFILE_FILE_PREFIX = "$profile:";
constexpr const char* PROFILE_PATH = "resources/base/profile/";
constexpr const char* JSON_SUFFIX = ".json";
constexpr const char* STARTUP_TASKS = "startupTasks";
constexpr const char* PRELOAD_STARTUP_TASKS = "appPreloadHintStartupTasks";
constexpr const char* NAME = "name";
constexpr const char* SRC_ENTRY = "srcEntry";
constexpr const char* DEPENDENCIES = "dependencies";
constexpr const char* EXCLUDE_FROM_AUTO_START = "excludeFromAutoStart";
constexpr const char* RUN_ON_THREAD = "runOnThread";
constexpr const char* WAIT_ON_MAIN_THREAD = "waitOnMainThread";
constexpr const char* CONFIG_ENTRY = "configEntry";
constexpr const char* TASK_POOL = "taskPool";
constexpr const char* TASK_POOL_LOWER = "taskpool";
constexpr const char* OHMURL = "ohmurl";
} // namespace

int32_t AppStartupProfileParser::GetStartupConfigString(const ModuleStartupConfigInfo& info, std::string& config)
{
    TAG_LOGD(AAFwkTag::STARTUP, "start");
    std::string appStartup = info.startupConfig_;
    if (appStartup.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "appStartup invalid");
        return ERR_STARTUP_CONFIG_NOT_FOUND;
    }
    AAFwk::EventInfo eventInfo;
    size_t pos = appStartup.rfind(PROFILE_FILE_PREFIX);
    if ((pos == std::string::npos) || (pos == appStartup.length() - strlen(PROFILE_FILE_PREFIX))) {
        TAG_LOGE(AAFwkTag::STARTUP, "appStartup %{public}s is invalid", appStartup.c_str());
        return ERR_STARTUP_CONFIG_PATH_ERROR;
    }
    std::string profileName = appStartup.substr(pos + strlen(PROFILE_FILE_PREFIX));
    std::string hapPath = info.hapPath_;
    std::unique_ptr<uint8_t[]> startupConfig = nullptr;
    size_t len = 0;
    std::string profilePath = PROFILE_PATH + profileName + JSON_SUFFIX;
    std::string loadPath = AbilityBase::ExtractorUtil::GetLoadFilePath(hapPath);
    bool newCreate = false;
    std::shared_ptr<AbilityBase::Extractor> extractor =
        AbilityBase::ExtractorUtil::GetExtractor(loadPath, newCreate);
    if (!extractor->ExtractToBufByName(profilePath, startupConfig, len)) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to get startup config, profilePath: %{private}s, hapPath: %{private}s",
            profilePath.c_str(), hapPath.c_str());
        eventInfo.errCode = ERR_STARTUP_CONFIG_PATH_ERROR;
        eventInfo.errReason = "failed to get startup";
        AAFwk::EventReport::SendLaunchFrameworkEvent(
            AAFwk::EventName::STARTUP_TASK_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_STARTUP_CONFIG_PATH_ERROR;
    }
    std::string configData(startupConfig.get(), startupConfig.get() + len);
    nlohmann::json profileJson = nlohmann::json::parse(configData, nullptr, false);
    if (profileJson.is_discarded()) {
        TAG_LOGE(AAFwkTag::STARTUP, "bad profile file");
        eventInfo.errCode = ERR_STARTUP_CONFIG_PARSE_ERROR;
        eventInfo.errReason = "bad profile file";
        AAFwk::EventReport::SendLaunchFrameworkEvent(
            AAFwk::EventName::STARTUP_TASK_ERROR, HiSysEventType::FAULT, eventInfo);
        return ERR_STARTUP_CONFIG_PARSE_ERROR;
    }
    config = profileJson.dump();
    return ERR_OK;
}

bool AppStartupProfileParser::AnalyzeStartupConfig(const ModuleStartupConfigInfo& info,
    const std::string& startupConfig,
    std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks,
    std::vector<StartupTaskInfo>& pendingStartupTaskInfos, std::string& pendingConfigEntry)
{
    if (startupConfig.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "startupConfig invalid");
        return false;
    }

    nlohmann::json startupConfigJson = nlohmann::json::parse(startupConfig, nullptr, false);
    if (startupConfigJson.is_discarded()) {
        TAG_LOGE(AAFwkTag::STARTUP, "Failed to parse json string");
        return false;
    }

    if (info.moduleType_ == AppExecFwk::ModuleType::ENTRY || info.moduleType_ == AppExecFwk::ModuleType::FEATURE) {
        if (!(startupConfigJson.contains(CONFIG_ENTRY) && startupConfigJson[CONFIG_ENTRY].is_string())) {
            TAG_LOGE(AAFwkTag::STARTUP, "no config entry.");
            return false;
        }
        pendingConfigEntry = startupConfigJson.at(CONFIG_ENTRY).get<std::string>();
        if (pendingConfigEntry.empty()) {
            TAG_LOGE(AAFwkTag::STARTUP, "startup config empty.");
            return false;
        }
    }

    if (!AnalyzeAppStartupTask(info, startupConfigJson, pendingStartupTaskInfos)) {
        return false;
    }
    if (!AnalyzePreloadSoStartupTask(info, startupConfigJson, preloadSoStartupTasks)) {
        return false;
    }
    return true;
}

bool AppStartupProfileParser::AnalyzeAppStartupTask(const ModuleStartupConfigInfo& info,
    nlohmann::json& startupConfigJson,
    std::vector<StartupTaskInfo>& pendingStartupTaskInfos)
{
    if (startupConfigJson.contains(STARTUP_TASKS) && startupConfigJson[STARTUP_TASKS].is_array()) {
        for (const auto& module : startupConfigJson.at(STARTUP_TASKS).get<nlohmann::json>()) {
            if (!module.contains(SRC_ENTRY) || !module[SRC_ENTRY].is_string() ||
                !module.contains(NAME) || !module[NAME].is_string()) {
                TAG_LOGE(AAFwkTag::STARTUP, "Invalid module data");
                return false;
            }
            int32_t result = AnalyzeAppStartupTaskInner(info, module, pendingStartupTaskInfos);
            if (!result) {
                return false;
            }
        }
        return true;
    }
    return true;
}

bool AppStartupProfileParser::AnalyzePreloadSoStartupTask(const ModuleStartupConfigInfo& info,
    nlohmann::json& startupConfigJson,
    std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks)
{
    if (startupConfigJson.contains(PRELOAD_STARTUP_TASKS) && startupConfigJson[PRELOAD_STARTUP_TASKS].is_array()) {
        for (const auto& module : startupConfigJson.at(PRELOAD_STARTUP_TASKS).get<nlohmann::json>()) {
            if (!module.contains(SRC_ENTRY) || !module[SRC_ENTRY].is_string() ||
                !module.contains(NAME) || !module[NAME].is_string()) {
                TAG_LOGE(AAFwkTag::STARTUP, "Invalid module data");
                return false;
            }
            int32_t result = AnalyzePreloadSoStartupTaskInner(info, module, preloadSoStartupTasks);
            if (!result) {
                return false;
            }
        }
        return true;
    }
    return true;
}

bool AppStartupProfileParser::AnalyzeAppStartupTaskInner(const ModuleStartupConfigInfo& info,
    const nlohmann::json& startupTaskJson,
    std::vector<StartupTaskInfo>& pendingStartupTaskInfos)
{
    if (!startupTaskJson.contains(SRC_ENTRY) || !startupTaskJson[SRC_ENTRY].is_string() ||
        !startupTaskJson.contains(NAME) || !startupTaskJson[NAME].is_string()) {
        TAG_LOGE(AAFwkTag::STARTUP, "Invalid startupTaskJson data");
        return false;
    }
    StartupTaskInfo startupTaskInfo;
    startupTaskInfo.moduleName = info.name_;
    startupTaskInfo.hapPath = info.hapPath_;
    startupTaskInfo.esModule = info.esModule_;

    startupTaskInfo.name = startupTaskJson.at(NAME).get<std::string>();
    startupTaskInfo.srcEntry = startupTaskJson.at(SRC_ENTRY).get<std::string>();
    if (startupTaskInfo.name.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "startup task name is empty");
        return false;
    }
    if (startupTaskInfo.srcEntry.empty()) {
        TAG_LOGE(AAFwkTag::STARTUP, "startup task %{public}s no srcEntry", startupTaskInfo.name.c_str());
        return false;
    }
    SetOptionalParameters(startupTaskJson, info.moduleType_, startupTaskInfo);
    pendingStartupTaskInfos.emplace_back(startupTaskInfo);
    return true;
}

bool AppStartupProfileParser::AnalyzePreloadSoStartupTaskInner(const ModuleStartupConfigInfo& info,
    const nlohmann::json& preloadStartupTaskJson,
    std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks)
{
    if (!preloadStartupTaskJson.contains(NAME) || !preloadStartupTaskJson[NAME].is_string() ||
        !preloadStartupTaskJson.contains(OHMURL) || !preloadStartupTaskJson[OHMURL].is_string()) {
        TAG_LOGE(AAFwkTag::STARTUP, "Invalid startupTaskJson data");
        return false;
    }

    std::string name = preloadStartupTaskJson.at(NAME).get<std::string>();
    std::string ohmUrl = preloadStartupTaskJson.at(OHMURL).get<std::string>();
    std::string path = info.name_;
    auto task = std::make_shared<PreloadSoStartupTask>(name, ohmUrl, path);
    if (task == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "task is null");
        return false;
    }

    SetOptionalParameters(preloadStartupTaskJson, info.moduleType_, task);
    preloadSoStartupTasks.emplace(name, task);
    return true;
}

void AppStartupProfileParser::SetOptionalParameters(const nlohmann::json& module, AppExecFwk::ModuleType moduleType,
    StartupTaskInfo& startupTaskInfo)
{
    if (module.contains(DEPENDENCIES) && module[DEPENDENCIES].is_array()) {
        for (const auto& dependency : module.at(DEPENDENCIES)) {
            if (dependency.is_string()) {
                startupTaskInfo.dependencies.push_back(dependency.get<std::string>());
            }
        }
    }

    if (module.contains(RUN_ON_THREAD) && module[RUN_ON_THREAD].is_string()) {
        std::string profileName = module.at(RUN_ON_THREAD).get<std::string>();
        if (profileName == TASK_POOL || profileName == TASK_POOL_LOWER) {
            startupTaskInfo.callCreateOnMainThread = false;
        } else {
            startupTaskInfo.callCreateOnMainThread = true;
        }
    }

    if (module.contains(WAIT_ON_MAIN_THREAD) && module[WAIT_ON_MAIN_THREAD].is_boolean()) {
        startupTaskInfo.waitOnMainThread = module.at(WAIT_ON_MAIN_THREAD).get<bool>();
    } else {
        startupTaskInfo.waitOnMainThread = true;
    }

    if (module.contains(OHMURL) && module[OHMURL].is_string()) {
        startupTaskInfo.ohmUrl = module.at(OHMURL).get<std::string>();
    }

    if (moduleType != AppExecFwk::ModuleType::ENTRY && moduleType != AppExecFwk::ModuleType::FEATURE) {
        startupTaskInfo.excludeFromAutoStart = true;
        return;
    }
    if (module.contains(EXCLUDE_FROM_AUTO_START) && module[EXCLUDE_FROM_AUTO_START].is_boolean()) {
        startupTaskInfo.excludeFromAutoStart = module.at(EXCLUDE_FROM_AUTO_START).get<bool>();
    } else {
        startupTaskInfo.excludeFromAutoStart = false;
    }
}

void AppStartupProfileParser::SetOptionalParameters(const nlohmann::json& module, AppExecFwk::ModuleType moduleType,
    std::shared_ptr<PreloadSoStartupTask>& task)
{
    if (module.contains(DEPENDENCIES) && module[DEPENDENCIES].is_array()) {
        std::vector<std::string> dependencies;
        for (const auto& dependency : module.at(DEPENDENCIES)) {
            if (dependency.is_string()) {
                dependencies.push_back(dependency.get<std::string>());
            }
        }
        task->SetDependencies(dependencies);
    }

    if (moduleType != AppExecFwk::ModuleType::ENTRY && moduleType != AppExecFwk::ModuleType::FEATURE) {
        task->SetIsExcludeFromAutoStart(true);
        return;
    }
    if (module.contains(EXCLUDE_FROM_AUTO_START) && module[EXCLUDE_FROM_AUTO_START].is_boolean()) {
        task->SetIsExcludeFromAutoStart(module.at(EXCLUDE_FROM_AUTO_START).get<bool>());
    } else {
        task->SetIsExcludeFromAutoStart(false);
    }
}
} // namespace AbilityRuntime
} // namespace OHOS
