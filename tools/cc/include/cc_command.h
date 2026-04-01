/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CC_COMMAND_H
#define OHOS_ABILITY_RUNTIME_CC_COMMAND_H

#include <regex>

#include "ability_manager_interface.h"
#include "shell_command.h"
#include "cc_param_parser.h"

namespace OHOS {
namespace AAFwk {
namespace {
using ParametersInteger = std::map<std::string, int>;
using ParametersString = std::map<std::string, std::string>;
using ParametersBool = std::map<std::string, bool>;

const std::string TOOL_NAME = "claw_cc";

const std::string HELP_MSG =
    "usage: claw_cc <command> <options>\n"
    "These are common claw_cc commands list:\n"
    "  help                        list available commands\n"
    "  execute-intent              execute insight intent with result\n"
    "  get-intent                  get all insight intent info\n";

const std::string HELP_MSG_EXECUTE_INTENT =
    "usage: claw_cc execute-intent <options>\n"
    "options list:\n"
    "  -h, --help                                  list available commands\n"
    "  -b, --bundle <bundle-name>                  bundle name (required)\n"
    "  -m, --module <module-name>                  module name (required)\n"
    "  -a, --ability <ability-name>                ability name (optional, auto-resolved if omitted)\n"
    "  -i, --intent <intent-name>                  insight intent name (required)\n"
    "  -e, --execute-mode <mode>                   execute mode (optional, auto-resolved if only one mode)\n"
    "  -p, --param <json>                          intent parameters in JSON format (optional)\n"
    "                                              Example: '{\"key1\":\"value1\",\"key2\":123}'\n"
    "Note: This command waits for execution result and returns it.\n";

const std::string HELP_MSG_GET_INTENT =
    "usage: claw_cc get-intent <options>\n"
    "options list:\n"
    "  -h, --help                                  list available commands\n"
    "  -e, --execute-mode <mode>                   query mode (required)\n"
    "                                              Values:\n"
    "                                                1 - query all intents (GetAllInsightIntentInfo)\n"
    "                                                2 - query by bundle name (GetInsightIntentInfoByBundleName)\n"
    "                                                3 - query by intent name (GetInsightIntentInfoByIntentName)\n"
    "  -f, --flag <flag>                           get type flag (required)\n"
    "                                              Values:\n"
    "                                                1 - GET_FULL_INSIGHT_INTENT\n"
    "                                                2 - GET_SUMMARY_INSIGHT_INTENT\n"
    "                                                4 - GET_ENTITY_INFO\n"
    "                                                5 - GET_FULL_INSIGHT_INTENT_ENTITY\n"
    "                                                6 - GET_SUMMARY_INSIGHT_INTENT_ENTITY\n"
    "  [-u, --user-id <id>]                        user id (optional, default 100)\n"
    "  [-b, --bundle <bundle-name>]                bundle name (required when -e is 2 or 3)\n"
    "  [-m, --module <module-name>]                module name (required when -e is 3)\n"
    "  [-i, --intent <intent-name>]                intent name (required when -e is 3)\n"
    "Examples:\n"
    "  claw_cc get-intent -e 1 -f 1\n"
    "  claw_cc get-intent -e 2 -f 1 -b com.example.bundle\n"
    "  claw_cc get-intent -e 3 -f 1 -b com.example.bundle -m entry -i MyIntent\n";
}  // namespace

class InsightIntentShellCommand : public ShellCommand {
public:
    InsightIntentShellCommand(int argc, char* argv[]);
    ~InsightIntentShellCommand() override = default;

    ErrCode CreateMessageMap() override;

private:
    ErrCode CreateCommandMap() override;
    ErrCode init() override;

    ErrCode RunAsHelpCommand();
    ErrCode RunAsExecuteIntentCommand();
    ErrCode RunAsGetIntentCommand();

    ErrCode ParseExecuteIntentOptions(std::string &bundleName, std::string &moduleName,
        std::string &abilityName, std::string &insightIntentName,
        std::string &intentParamJson, int32_t &executeMode);
    ErrCode HandleExecuteIntentOption(int option, std::string &bundleName,
        std::string &moduleName, std::string &abilityName,
        std::string &insightIntentName, std::string &intentParamJson,
        int32_t &executeMode);
    ErrCode CheckRequiredExecuteParams(const std::string &bundleName,
        const std::string &moduleName,
        const std::string &insightIntentName);
    ErrCode ValidateGetIntentParams(int32_t flag, int32_t executeMode,
        int32_t &userId);
    ErrCode ResolveAbilityName(const std::string &bundleName,
        const std::string &moduleName, const std::string &insightIntentName,
        std::string &abilityName);
    ErrCode ResolveIntentInfo(const std::string &bundleName,
        const std::string &moduleName, const std::string &insightIntentName,
        std::string &abilityName, int32_t &executeMode);
    ErrCode ResolveAbilityNameFromQuery(
        const AbilityRuntime::InsightIntentInfoForQuery &queryInfo,
        std::string &abilityName);
    ErrCode ResolveExecuteModeFromQuery(
        const AbilityRuntime::InsightIntentInfoForQuery &queryInfo,
        int32_t &executeMode);
    std::vector<int32_t> CollectSupportedModes(
        const AbilityRuntime::InsightIntentInfoForQuery &queryInfo);
    ErrCode ValidateModeInSupportedList(const std::vector<int32_t> &modes,
        int32_t executeMode);
    ErrCode AutoResolveExecuteMode(const std::vector<int32_t> &modes,
        int32_t &executeMode);
    void AppendSupportedModes(const std::vector<int32_t> &modes);
    ErrCode ExecuteIntentWithParam(const InsightIntentExecuteParam &param);
    void AppendExecuteResult(const InsightIntentExecuteResult &executeResult);

    ErrCode ParseGetIntentOptions(int32_t &flag, int32_t &executeMode,
        int32_t &userId, std::string &bundleName,
        std::string &moduleName, std::string &intentName);
    ErrCode HandleGetIntentOption(int option, int32_t &flag,
        int32_t &executeMode, int32_t &userId, std::string &bundleName,
        std::string &moduleName, std::string &intentName);
    ErrCode DispatchGetIntentMode(int32_t executeMode, int32_t flag,
        int32_t userId, const std::string &bundleName,
        const std::string &moduleName, const std::string &intentName);
    ErrCode RunGetIntentModeAll(int32_t flag, int32_t userId);
    ErrCode RunGetIntentModeByBundle(int32_t flag, int32_t userId,
        const std::string &bundleName);
    ErrCode RunGetIntentModeByIntent(int32_t flag, int32_t userId,
        const std::string &bundleName, const std::string &moduleName,
        const std::string &intentName);

    ErrCode ParseIntOption(const char *arg, int32_t &value);
    void AppendIntentInfoList(
        const std::vector<AbilityRuntime::InsightIntentInfoForQuery> &infos);
    void AppendSingleIntentInfo(
        const AbilityRuntime::InsightIntentInfoForQuery &info);
    void AppendIntentBasicInfo(
        const AbilityRuntime::InsightIntentInfoForQuery &info);
    void AppendIntentDetailInfo(
        const AbilityRuntime::InsightIntentInfoForQuery &info);
    void AppendIntentPageInfo(
        const AbilityRuntime::InsightIntentInfoForQuery &info);
    void AppendIntentEntryInfo(
        const AbilityRuntime::InsightIntentInfoForQuery &info);
    void AppendIntentFormInfo(
        const AbilityRuntime::InsightIntentInfoForQuery &info);
    void AppendIntentAbilityInfo(
        const AbilityRuntime::InsightIntentInfoForQuery &info);
    void AppendExtensionInfo(
        const AbilityRuntime::InsightIntentInfoForQuery &info);
    void AppendIntentEntityList(
        const AbilityRuntime::InsightIntentInfoForQuery &info);
    void AppendStringVector(const std::string &label,
        const std::vector<std::string> &vec);

    ErrCode ValidateExecuteIntentParams(const std::string &bundleName,
        const std::string &moduleName, int32_t executeMode);
    ErrCode ValidateExecuteMode(int32_t executeMode);
    ErrCode ValidateModuleNameInResults(
        const std::vector<AbilityRuntime::InsightIntentInfoForQuery> &bundleInfos,
        const std::string &moduleName);
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CC_COMMAND_H
