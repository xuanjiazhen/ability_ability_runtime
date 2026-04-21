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

#include "aa_start_command.h"
#include <getopt.h>
#include <hilog_tag_wrapper.h>
#include <ability_manager_client.h>
#include <element_name.h>

namespace OHOS {
namespace AAFwk {

namespace {
    // 短选项定义
    const std::string SHORT_OPTIONS = "h:d:a:b:e:t:s:m:u:A:U:W";

    // 长选项定义
    constexpr struct option LONG_OPTIONS[] = {
        {"help", no_argument, nullptr, 'h'},
        {"device", required_argument, nullptr, 'd'},
        {"ability", required_argument, nullptr, 'a'},
        {"bundle", required_argument, nullptr, 'b'},
        {"setting", required_argument, nullptr, 's'},
        {"module", required_argument, nullptr, 'm'},
        {"action", required_argument, nullptr, 'A'},
        {"URI", required_argument, nullptr, 'U'},
        {"wait", no_argument, nullptr, 'W'},
        {"entity", required_argument, nullptr, 'e'},
        {"type", required_argument, nullptr, 't'},
        {"user", required_argument, nullptr, 'u'},
        {"pi", required_argument, nullptr, AAStartCommand::OPTION_PARAMETER_INTEGER},
        {"ps", required_argument, nullptr, AAStartCommand::OPTION_PARAMETER_STRING},
        {"pb", required_argument, nullptr, AAStartCommand::OPTION_PARAMETER_BOOL},
        {"psn", required_argument, nullptr, AAStartCommand::OPTION_PARAMETER_NULL_STRING},
        {"wl", required_argument, nullptr, AAStartCommand::OPTION_WINDOW_LEFT},
        {"wt", required_argument, nullptr, AAStartCommand::OPTION_WINDOW_TOP},
        {"wh", required_argument, nullptr, AAStartCommand::OPTION_WINDOW_HEIGHT},
        {"ww", required_argument, nullptr, AAStartCommand::OPTION_WINDOW_WIDTH},
        {nullptr, 0, nullptr, 0},
    };

    // 消息常量
    const std::string STRING_START_ABILITY_OK = "start ability successfully";
    const std::string STRING_START_ABILITY_NG = "error: failed to start ability";
    const std::string HELP_MSG_START =
        "usage: ohos-aa start <options>\n"
        "options list:\n"
        "  -h, --help                                                   list available commands\n"
        "  [-d <device-id>] [-a <ability-name> -b <bundle-name>] [-m <module-name>] \n"
        "  [-s <setting>] "
        "  [--ps <key> <string-value>] "
        "  [--pi <key> <integer-value>] "
        "  [--pb <key> <boolean-value>] "
        "  [--psn <key>] "
        "  [-A <action-name>] "
        "  [-U <URI>] "
        "  [-e <entity>] "
        "  [-t <mime-type>] "
        "  [-W] "
        "  [--wl <window-left>] "
        "  [--wt <window-top>] "
        "  [--wh <window-height>] "
        "  [--ww <window-width>] "
        "  [-u <user-id>] "
        "  start ability with an element name";
} // namespace

AAStartCommand::AAStartCommand() = default;

char **AAStartCommand::BuildArgv(const std::vector<std::string>& args, int& argc)
{
    argc = args.size() + 1;
    char **argv = new char *[argc + 1];
    argv[0] = const_cast<char*>("ohos-aa");
    for (size_t i = 0; i < args.size(); ++i) {
        argv[i + 1] = const_cast<char*>(args[i].c_str());
    }
    argv[argc] = nullptr;
    return argv;
}

bool AAStartCommand::HandleBasicOption(int option, char* optarg, ParseContext& context, int32_t& userId)
{
    switch (option) {
        case 'd':
            if (optarg != nullptr) {
                context.deviceId = optarg;
            }
            return true;

        case 'a':
            context.abilityName = optarg;
            return true;

        case 'b':
            context.bundleName = optarg;
            return true;

        case 'e':
            context.entities.push_back(optarg);
            return true;

        case 't':
            context.typeVal = optarg;
            return true;

        case 's':
            context.windowMode = optarg;
            return true;

        case 'm':
            context.moduleName = optarg;
            return true;

        case 'A':
            context.action = optarg;
            return true;

        case 'U':
            context.uri = optarg;
            return true;

        case 'u':
            if (optarg != nullptr) {
                userId = std::atoi(optarg);
            }
            return true;

        case 'W':
            waitFlag_ = true;
            return true;

        case 'h':
            return true;

        default:
            return false;
    }
}

bool AAStartCommand::HandleParameterOption(int option, int argc, char **argv, ParseContext& context)
{
    if (optind >= argc) {
        return false;
    }

    std::string key = optarg;
    std::string value = argv[optind];
    optind++;

    switch (option) {
        case OPTION_PARAMETER_INTEGER: {
            int intVal = std::atoi(value.c_str());
            context.parametersInteger[key] = intVal;
            return true;
        }

        case OPTION_PARAMETER_STRING:
            context.parametersString[key] = value;
            return true;

        case OPTION_PARAMETER_BOOL: {
            bool boolVal = (value == "true" || value == "1");
            context.parametersBool[key] = boolVal;
            return true;
        }

        case OPTION_PARAMETER_NULL_STRING:
            context.parametersString[key] = "";
            return true;

        default:
            return false;
    }
}

void AAStartCommand::HandleWindowOption(int option, char* optarg, ParseContext& context)
{
    if (optarg == nullptr) {
        return;
    }

    switch (option) {
        case OPTION_WINDOW_LEFT:
            context.windowLeft = std::atoi(optarg);
            context.hasWindowLeft = true;
            break;

        case OPTION_WINDOW_TOP:
            context.windowTop = std::atoi(optarg);
            context.hasWindowTop = true;
            break;

        case OPTION_WINDOW_HEIGHT:
            context.windowHeight = std::atoi(optarg);
            context.hasWindowHeight = true;
            break;

        case OPTION_WINDOW_WIDTH:
            context.windowWidth = std::atoi(optarg);
            context.hasWindowWidth = true;
            break;

        default:
            break;
    }
}

ErrCode AAStartCommand::Execute(const std::vector<std::string>& args)
{
    for (const auto& arg : args) {
        if (arg == "-h" || arg == "--help") {
            ShowHelp();
            return OHOS::ERR_OK;
        }
    }

    Want want;
    int32_t userId = -1;

    ErrCode result = MakeWantFromCmd(args, want, userId);
    if (result != OHOS::ERR_OK) {
        std::cerr << "Invalid parameters" << std::endl;
        return result;
    }

    if (waitFlag_) {
        result = StartAbilityWithWait(want, userId);
    } else {
        result = StartAbility(want, userId);
    }

    return result;
}

ErrCode AAStartCommand::ParseOptions(int argc, char **argv, ParseContext& context, int32_t& userId)
{
    optind = 1;
    while (true) {
        int option = getopt_long(argc, argv, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);

        if (option == -1) {
            break;
        }

        if (option == '?') {
            return OHOS::ERR_INVALID_VALUE;
        }

        // 处理基本选项
        if (HandleBasicOption(option, optarg, context, userId)) {
            continue;
        }

        // 处理参数选项（需要额外值）
        if (option >= OPTION_PARAMETER_INTEGER && option <= OPTION_PARAMETER_NULL_STRING) {
            if (!HandleParameterOption(option, argc, argv, context)) {
                return OHOS::ERR_INVALID_VALUE;
            }
            continue;
        }

        // 处理窗口选项
        HandleWindowOption(option, optarg, context);
    }

    return OHOS::ERR_OK;
}

void AAStartCommand::BuildWantObject(const ParseContext& context, Want& want)
{
    if (!context.abilityName.empty() || !context.bundleName.empty() ||
        !context.action.empty() || !context.uri.empty()) {
        AppExecFwk::ElementName element(
            context.deviceId, context.bundleName,
            context.abilityName, context.moduleName);
        want.SetElement(element);
    }

    if (!context.action.empty()) {
        want.SetAction(context.action);
    }

    if (!context.uri.empty()) {
        want.SetUri(context.uri);
    }

    if (!context.typeVal.empty()) {
        want.SetType(context.typeVal);
    }

    for (auto entity : context.entities) {
        want.AddEntity(entity);
    }

    SetParams(context.parametersInteger, want);
    SetParams(context.parametersString, want);
    SetParams(context.parametersBool, want);
}

void AAStartCommand::SetWindowProperties(const ParseContext& context, Want& want)
{
    if (context.hasWindowLeft) {
        want.SetParam(Want::PARAM_RESV_WINDOW_LEFT, context.windowLeft);
    }
    if (context.hasWindowTop) {
        want.SetParam(Want::PARAM_RESV_WINDOW_TOP, context.windowTop);
    }
    if (context.hasWindowHeight) {
        want.SetParam(Want::PARAM_RESV_WINDOW_HEIGHT, context.windowHeight);
    }
    if (context.hasWindowWidth) {
        want.SetParam(Want::PARAM_RESV_WINDOW_WIDTH, context.windowWidth);
    }
}

ErrCode AAStartCommand::MakeWantFromCmd(
    const std::vector<std::string>& args, Want& want, int32_t& userId)
{
    ParseContext context;
    int argc = 0;
    char **argv = BuildArgv(args, argc);

    ErrCode result = ParseOptions(argc, argv, context, userId);
    delete[] argv;

    if (result != OHOS::ERR_OK) {
        return result;
    }

    BuildWantObject(context, want);
    SetWindowProperties(context, want);

    return OHOS::ERR_OK;
}

ErrCode AAStartCommand::StartAbility(const Want& want, int32_t userId)
{
    auto abilityMs = AbilityManagerClient::GetInstance();
    ErrCode result = abilityMs->StartAbility(want, -1, userId);
    if (result == OHOS::ERR_OK) {
        EmitResult("success", STRING_START_ABILITY_OK);
    } else {
        std::string errorMsg = STRING_START_ABILITY_NG + ": " + GetMessageFromCode(result);
        std::cerr << errorMsg << std::endl;
    }

    return result;
}

ErrCode AAStartCommand::StartAbilityWithWait(const Want& want, int32_t userId)
{
    // 简化实现：直接调用 StartAbility
    // 完整实现需要 observer 模式
    return StartAbility(want, userId);
}

void AAStartCommand::SetParams(const ParametersInteger& pi, Want& want)
{
    for (auto it = pi.begin(); it != pi.end(); it++) {
        want.SetParam(it->first, it->second);
    }
}

void AAStartCommand::SetParams(const ParametersString& ps, Want& want)
{
    for (auto it = ps.begin(); it != ps.end(); it++) {
        want.SetParam(it->first, it->second);
    }
}

void AAStartCommand::SetParams(const ParametersBool& pb, Want& want)
{
    for (auto it = pb.begin(); it != pb.end(); it++) {
        want.SetParam(it->first, it->second);
    }
}

void AAStartCommand::EmitResult(const std::string& status, const std::string& message)
{
    std::cout << "{\"event\": \"result\", \"data\": {"
              << "\"status\": \"" << status << "\", "
              << "\"message\": \"" << message << "\""
              << "}}" << std::endl;
}

void AAStartCommand::EmitStdout(const std::string& message)
{
    std::cout << "{\"event\": \"stdout\", \"data\": {"
              << "\"message\": \"" << message << "\""
              << "}}" << std::endl;
}

void AAStartCommand::ShowHelp()
{
    EmitStdout(HELP_MSG_START);
}

std::string AAStartCommand::GetMessageFromCode(ErrCode code)
{
    switch (code) {
        case OHOS::ERR_OK:
            return "Success";
        case OHOS::ERR_INVALID_VALUE:
            return "Invalid parameter";
        case OHOS::ERR_NO_INIT:
            return "Not initialized";
        default:
            return "Error code: " + std::to_string(code);
    }
}

} // namespace AAFwk
} // namespace OHOS
