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

#ifndef OHOS_ABILITY_RUNTIME_APP_STARTUP_PROFILE_PARSER_H
#define OHOS_ABILITY_RUNTIME_APP_STARTUP_PROFILE_PARSER_H

#include <string>

#include "app_startup_task.h"
#include "preload_so_startup_task.h"
#include "bundle_info.h"

namespace OHOS {
namespace AbilityRuntime {
struct ModuleStartupConfigInfo {
    std::string name_;
    std::string startupConfig_;
    std::string hapPath_;
    AppExecFwk::ModuleType moduleType_ = AppExecFwk::ModuleType::UNKNOWN;
    bool esModule_;

    ModuleStartupConfigInfo(std::string name, std::string startupConfig, std::string hapPath,
        const AppExecFwk::ModuleType& moduleType, bool esModule);
};

struct StartupTaskInfo {
    std::string name;
    std::string srcEntry;
    std::string ohmUrl;
    std::string moduleName;
    std::string hapPath;
    std::vector<std::string> dependencies;
    bool excludeFromAutoStart = false;
    bool callCreateOnMainThread = true;
    bool waitOnMainThread = true;
    bool esModule = true;
};

using AppStartupTasks = std::map<std::string, std::shared_ptr<AppStartupTask>>;

class AppStartupProfileParser {
public:
    static int32_t GetStartupConfigString(const ModuleStartupConfigInfo& info, std::string& config);

    static bool AnalyzeStartupConfig(const ModuleStartupConfigInfo& info, const std::string& startupConfig,
        AppStartupTasks& preloadSoStartupTasks,
        std::vector<StartupTaskInfo>& pendingStartupTaskInfos, std::string& pendingConfigEntry);

private:
    static bool AnalyzeAppStartupTask(const ModuleStartupConfigInfo& info, nlohmann::json &startupConfigJson,
        std::vector<StartupTaskInfo>& pendingStartupTaskInfos);
    static bool AnalyzePreloadSoStartupTask(const ModuleStartupConfigInfo& info, nlohmann::json &startupConfigJson,
        std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks);
    static bool AnalyzeAppStartupTaskInner(const ModuleStartupConfigInfo& info,
        const nlohmann::json &startupTaskJson,
        std::vector<StartupTaskInfo>& pendingStartupTaskInfos);
    static bool AnalyzePreloadSoStartupTaskInner(const ModuleStartupConfigInfo& info,
        const nlohmann::json &preloadStartupTaskJson,
        std::map<std::string, std::shared_ptr<AppStartupTask>>& preloadSoStartupTasks);
    static void SetOptionalParameters(const nlohmann::json& module, AppExecFwk::ModuleType moduleType,
        StartupTaskInfo& startupTaskInfo);
    static void SetOptionalParameters(const nlohmann::json &module, AppExecFwk::ModuleType moduleType,
        std::shared_ptr<PreloadSoStartupTask> &task);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_APP_STARTUP_PROFILE_PARSER_H
