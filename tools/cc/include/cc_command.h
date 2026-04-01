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
#include <nlohmann/json.hpp>
#include "array_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
using ParametersInteger = std::map<std::string, int>;
using ParametersString = std::map<std::string, std::string>;
using ParametersBool = std::map<std::string, bool>;

const std::string TOOL_NAME = "cc";

const std::string HELP_MSG =
    "usage: cc <command> <options>\n"
    "These are common cc commands list:\n"
    "  help                        list available commands\n"
    "  execute-intent              execute insight intent with result\n"
    "  get-intent                  get all insight intent info\n";

const std::string HELP_MSG_EXECUTE_INTENT =
    "usage: cc execute-intent <options>\n"
    "options list:\n"
    "  -h, --help                                  list available commands\n"
    "  -b, --bundle <bundle-name>                  bundle name (required)\n"
    "  -m, --module <module-name>                  module name (required)\n"
    "  -a, --ability <ability-name>                ability name (required)\n"
    "  -i, --intent <intent-name>                  insight intent name (required)\n"
    "  -e, --execute-mode <mode>                   execute mode (required)\n"
    "  -p, --param <json>                          intent parameters in JSON format (optional)\n"
    "                                              Example: '{\"key1\":\"value1\",\"key2\":123}'\n"
    "Note: This command waits for execution result and returns it.\n";

const std::string HELP_MSG_GET_INTENT =
    "usage: cc get-intent <options>\n"
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
    "  cc get-intent -e 1 -f 1\n"
    "  cc get-intent -e 2 -f 1 -b com.example.bundle\n"
    "  cc get-intent -e 3 -f 1 -b com.example.bundle -m entry -i MyIntent\n";
}  // namespace

class InsightIntentShellCommand : public ShellCommand {
public:
    InsightIntentShellCommand(int argc, char* argv[]);
    ~InsightIntentShellCommand() override = default;

    ErrCode CreateMessageMap() override;
    WantParams BuildWantParamsFromJson(const std::string& jsonStr);
    WantParams ParseJsonObjectToWantParams(const nlohmann::json& jsonObj);
    sptr<IArray> ParseJsonArrayToIArray(const nlohmann::json& jsonArr);

private:
    ErrCode CreateCommandMap() override;
    ErrCode init() override;

    ErrCode RunAsHelpCommand();
    ErrCode RunAsExecuteIntentCommand();
    ErrCode RunAsGetIntentCommand();
    ErrCode RunGetIntentModeAll(int32_t flag, int32_t userId);
    ErrCode RunGetIntentModeByBundle(int32_t flag, int32_t userId, const std::string &bundleName);
    ErrCode RunGetIntentModeByIntent(int32_t flag, int32_t userId,
        const std::string &bundleName, const std::string &moduleName, const std::string &intentName);
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CC_COMMAND_H
