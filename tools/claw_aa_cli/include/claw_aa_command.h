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

#ifndef OHOS_ABILITY_RUNTIME_CLAW_AA_COMMAND_H
#define OHOS_ABILITY_RUNTIME_CLAW_AA_COMMAND_H

#include <getopt.h>
#include <map>
#include <regex>
#include <string>
#include <vector>

#include "ability_manager_interface.h"
#include "ability_start_setting.h"
#include "shell_command.h"

namespace OHOS {
namespace AAFwk {
namespace {
using ParametersInteger = std::map<std::string, int>;
using ParametersString = std::map<std::string, std::string>;
using ParametersBool = std::map<std::string, bool>;

const std::string TOOL_NAME = "ohos-aa";

const std::string HELP_MSG = "usage: ohos-aa <command> <options>\n"
    "These are common ohos-aa commands list:\n"
    "  help                        list available commands\n"
    "  version                     list version\n"
    "  start                       start ability with options\n"
    "  force-stop <bundle-name>    force stop the process with bundle name\n";

const std::string VERSION_MSG = "1.0.0\n";

const std::string HELP_MSG_START =
    "usage: ohos-aa start <options>\n"
    "options list:\n"
    "  [-d <device-id>] [-a <ability-name> -b <bundle-name>] [-m <module-name>] [-p <perf-cmd>] "
    "  [--ps <key> <string-value>] "
    "  [--pi <key> <integer-value>] "
    "  [--pb <key> <boolean-value>] "
    "  [--psn <key>] "
    "  [-A <action-name>] "
    "  [-U <URI>] "
    "  [-e <entity>] "
    "  [-t <mime-type>] "
    "  [-W] "
    "  start ability with an element name\n";

const std::string HELP_MSG_FORCE_STOP = "usage: ohos-aa force-stop <bundle-name>\n";

const std::string HELP_MSG_NO_BUNDLE_NAME_OPTION = "error: -b <bundle-name> is expected";

const std::string STRING_START_ABILITY_OK = "start ability successfully.";
const std::string STRING_START_ABILITY_NG = "error: failed to start ability.";

const std::string STRING_FORCE_STOP_OK = "force stop process successfully.";
const std::string STRING_FORCE_STOP_NG = "error: failed to force stop process.";

const int NUMBER_TWO = 2;
const int NUMBER_ONE = 1;

const std::string STRING_TEST_REGEX_INTEGER_NUMBERS = "^(0|[1-9][0-9]*|-[1-9][0-9]*)$";
const std::string STRING_REGEX_ALL_NUMBERS = "^(-)?([0-9]|[1-9][0-9]+)([\\.][0-9]+)?$";
const std::string STRING_IMPLICT_START_WITH_WAIT_NG = "The wait option does not support starting implict";
const std::string STRING_NON_UIABILITY_START_WITH_WAIT_NG = "The wait option does not support starting non-uiability";

constexpr int EXTRA_ARGUMENTS_FOR_KEY_VALUE_PAIR = 1;
constexpr int EXTRA_ARGUMENTS_FOR_NULL_STRING = 0;
constexpr int PARAM_LENGTH = 20;
constexpr int INDEX_OFFSET = 1;

enum OptionType {
    OPTION_PARAMETER_INTEGER = 1000,
    OPTION_PARAMETER_STRING,
    OPTION_PARAMETER_BOOL,
    OPTION_PARAMETER_NULL_STRING,
    OPTION_WINDOW_LEFT,
    OPTION_WINDOW_TOP,
    OPTION_WINDOW_HEIGHT,
    OPTION_WINDOW_WIDTH,
};

const std::string SHORT_OPTIONS = "hCd:a:b:Dp:m:e:s:t:u:w:A:U:ESNRcW";
const std::string SHORT_OPTIONS_DUMPSYS = "haielpdruc";

struct option LONG_OPTIONS[] = {
    {"help", no_argument, 0, 'h'},
    {"pi", required_argument, 0, OPTION_PARAMETER_INTEGER},
    {"ps", required_argument, 0, OPTION_PARAMETER_STRING},
    {"pb", required_argument, 0, OPTION_PARAMETER_BOOL},
    {"psn", required_argument, 0, OPTION_PARAMETER_NULL_STRING},
    {"wl", required_argument, 0, OPTION_WINDOW_LEFT},
    {"wt", required_argument, 0, OPTION_WINDOW_TOP},
    {"wh", required_argument, 0, OPTION_WINDOW_HEIGHT},
    {"ww", required_argument, 0, OPTION_WINDOW_WIDTH},
    {0, 0, 0, 0}
};
}

class ClawAaShellCommand : public ShellCommand {
public:
    ClawAaShellCommand(int argc, char* argv[]);
    ~ClawAaShellCommand() override
    {}

    ErrCode CreateMessageMap() override;
    void CheckStartAbilityResult(ErrCode& result);
    ErrCode CreateErrorInfoMap();
    AaToolErrorInfo GetErrorInfoFromCode(const int32_t code);

private:
    ErrCode init() override;
    ErrCode CreateCommandMap() override;
    
    ErrCode RunAsHelpCommand();
    ErrCode RunAsVersionCommand();
    ErrCode RunAsStartAbility();
    ErrCode RunAsForceStop();

    bool IsLongStartOption(const std::string &argv);
    bool IsShortStartOption(const std::string &argv);
    bool IsStartOption(const std::string &argv);
    bool CheckParameters(int target);
    ErrCode ParseParam(ParametersInteger& pi);
    ErrCode ParseParam(ParametersString& ps, bool isNull);
    ErrCode ParseParam(ParametersBool& pb);
    void SetParams(const ParametersInteger& pi, Want& want);
    void SetParams(const ParametersString& ps, Want& want);
    void SetParams(const ParametersBool& pb, Want& want);
    pid_t ConvertPid(std::string& inputPid);

    ErrCode MakeWantFromCmd(Want& want, int32_t& userId);
    ErrCode StartAbilityWithWait(Want& want, int32_t userId = DEFAULT_INVAL_VALUE);
    bool IsImplicitStartAction(const Want &want);
    bool MatchOrderString(const std::regex &regexScript, const std::string &orderCmd);
    bool CheckPerfCmdString(const char* optarg, const size_t paramLength, std::string &perfCmd);
    void FormatOutputForWithWait(const Want &want, const AbilityStartWithWaitObserverData& data);

    bool startAbilityWithWaitFlag_ = false;
    std::map<int32_t, AaToolErrorInfo> errorInfoMap_;
};
}  // namespace AAFwk
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CLAW_AA_COMMAND_H