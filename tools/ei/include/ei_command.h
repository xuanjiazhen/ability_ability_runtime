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

#ifndef OHOS_ABILITY_RUNTIME_EXECUTE_INTENT_COMMAND_H
#define OHOS_ABILITY_RUNTIME_EXECUTE_INTENT_COMMAND_H

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

const std::string TOOL_NAME = "ei";

const std::string HELP_MSG = "usage: ei <command> <options>\n"
    "These are common ei commands list:\n"
    "  help                        list available commands\n"
    "  execute                     execute insight intent with result\n"
    "  list                        get all insight intent info\n";

const std::string HELP_MSG_EXECUTE =
    "usage: ei execute <options>\n"
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

const std::string HELP_MSG_LIST =
    "usage: ei list <options>\n"
    "options list:\n"
    "  -h, --help                                  list available commands\n"
    "  -f, --flag <flag>                           get type flag (required)\n"
    "                                              Values:\n"
    "                                                1 - GET_FULL_INSIGHT_INTENT\n"
    "                                                2 - GET_SUMMARY_INSIGHT_INTENT\n"
    "                                                4 - GET_ENTITY_INFO\n"
    "                                                5 - GET_FULL_INSIGHT_INTENT_ENTITY\n"
    "                                                6 - GET_SUMMARY_INSIGHT_INTENT_ENTITY\n"
    "  [-u, --user-id <id>]                       user id (optional)\n";

const int DEFAULT_INVAL_VALUE = -1;
}  // namespace

class ExecuteIntentShellCommand : public ShellCommand {
public:
    ExecuteIntentShellCommand(int argc, char* argv[]);
    ~ExecuteIntentShellCommand() override
    {}

    ErrCode CreateMessageMap() override;
    WantParams BuildWantParamsFromJson(const std::string& jsonStr);
    WantParams ParseJsonObjectToWantParams(const nlohmann::json& jsonObj);
    sptr<IArray> ParseJsonArrayToIArray(const nlohmann::json& jsonArr);
private:
    ErrCode CreateCommandMap() override;
    ErrCode init() override;

    ErrCode RunAsHelpCommand();
    ErrCode RunAsExecuteCommand();
    ErrCode RunAsListCommand();

    sptr<IAbilityManager> GetAbilityManagerService();
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_EXECUTE_INTENT_COMMAND_H
