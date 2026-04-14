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

// optind 是 getopt 的全局变量
extern int optind;

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
        "usage: cli-aa start <options>\n"
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

    // 构建 getopt 需要的参数
    int argc = args.size() + 1; // +1 for program name
    char** argv = new char*[argc + 1]; // +1 for null terminator

    // 第一个参数是程序名（模拟）
    argv[0] = const_cast<char*>("cli-aa");

    // 复制参数
    for (size_t i = 0; i < args.size(); ++i) {
        argv[i + 1] = const_cast<char*>(args[i].c_str());
    }
    argv[argc] = nullptr;

    // 设置全局变量供 getopt 使用
    optind = 1;

    // 解析参数
    ErrCode result = MakeWantFromCmd(args, want, userId);

    // 释放内存
    delete[] argv;

    if (result != OHOS::ERR_OK) {
        std::cerr << "Invalid parameters" << std::endl;
        return result;
    }

    // 启动 Ability
    if (waitFlag_) {
        result = StartAbilityWithWait(want, userId);
    } else {
        result = StartAbility(want, userId);
    }

    return result;
}

ErrCode AAStartCommand::MakeWantFromCmd(
    const std::vector<std::string>& args, Want& want, int32_t& userId)
{
    int result = OHOS::ERR_OK;
    int option = -1;
    int counter = 0;

    std::string deviceId = "";
    std::string bundleName = "";
    std::string abilityName = "";
    std::string moduleName;
    std::string windowMode;
    ParametersInteger parametersInteger;
    ParametersString parametersString;
    ParametersBool parametersBool;
    std::string uri;
    std::string action;
    std::vector<std::string> entities;
    std::string typeVal;
    int windowLeft = 0;
    bool hasWindowLeft = false;
    int windowTop = 0;
    bool hasWindowTop = false;
    int windowHeight = 0;
    bool hasWindowHeight = false;
    int windowWidth = 0;
    bool hasWindowWidth = false;

    // 构建 getopt 需要的参数
    int argc = args.size() + 1;
    char** argv = new char*[argc + 1];
    argv[0] = const_cast<char*>("cli-aa");
    for (size_t i = 0; i < args.size(); ++i) {
        argv[i + 1] = const_cast<char*>(args[i].c_str());
    }
    argv[argc] = nullptr;

    optind = 1;

    while (true) {
        counter++;
        option = getopt_long(argc, argv, SHORT_OPTIONS.c_str(), LONG_OPTIONS, nullptr);

        if (option == -1) {
            break;
        }

        if (option == '?') {
            delete[] argv;
            return OHOS::ERR_INVALID_VALUE;
        }

        switch (option) {
            case 'h':
                break;

            case 'd':
                if (optarg != nullptr) {
                    deviceId = optarg;
                }
                break;

            case 'a':
                abilityName = optarg;
                break;

            case 'b':
                bundleName = optarg;
                break;

            case 'e':
                entities.push_back(optarg);
                break;

            case 't':
                typeVal = optarg;
                break;

            case 's':
                windowMode = optarg;
                break;

            case 'm':
                moduleName = optarg;
                break;

            case 'A':
                action = optarg;
                break;

            case 'U':
                uri = optarg;
                break;

            case 'u':
                if (optarg != nullptr) {
                    userId = std::atoi(optarg);
                }
                break;

            case 'W':
                waitFlag_ = true;
                break;

            case OPTION_PARAMETER_INTEGER: {
                if (optind >= argc) {
                    delete[] argv;
                    return OHOS::ERR_INVALID_VALUE;
                }
                std::string key = optarg;
                std::string value = argv[optind];
                optind++;
                int intVal = std::atoi(value.c_str());
                parametersInteger[key] = intVal;
                break;
            }

            case OPTION_PARAMETER_STRING: {
                if (optind >= argc) {
                    delete[] argv;
                    return OHOS::ERR_INVALID_VALUE;
                }
                std::string key = optarg;
                std::string value = argv[optind];
                optind++;
                parametersString[key] = value;
                break;
            }

            case OPTION_PARAMETER_BOOL: {
                if (optind >= argc) {
                    delete[] argv;
                    return OHOS::ERR_INVALID_VALUE;
                }
                std::string key = optarg;
                std::string value = argv[optind];
                optind++;
                bool boolVal = (value == "true" || value == "1");
                parametersBool[key] = boolVal;
                break;
            }

            case OPTION_PARAMETER_NULL_STRING: {
                std::string key = optarg;
                parametersString[key] = "";
                break;
            }

            case OPTION_WINDOW_LEFT:
                if (optarg != nullptr) {
                    windowLeft = std::atoi(optarg);
                    hasWindowLeft = true;
                }
                break;

            case OPTION_WINDOW_TOP:
                if (optarg != nullptr) {
                    windowTop = std::atoi(optarg);
                    hasWindowTop = true;
                }
                break;

            case OPTION_WINDOW_HEIGHT:
                if (optarg != nullptr) {
                    windowHeight = std::atoi(optarg);
                    hasWindowHeight = true;
                }
                break;

            case OPTION_WINDOW_WIDTH:
                if (optarg != nullptr) {
                    windowWidth = std::atoi(optarg);
                    hasWindowWidth = true;
                }
                break;

            default:
                break;
        }
    }

    delete[] argv;

    // 构建 Want 对象
    if (!abilityName.empty() || !bundleName.empty() || !action.empty() || !uri.empty()) {
        AppExecFwk::ElementName element(deviceId, bundleName, abilityName, moduleName);
        want.SetElement(element);
    }

    if (!action.empty()) {
        want.SetAction(action);
    }

    if (!uri.empty()) {
        want.SetUri(uri);
    }

    if (!typeVal.empty()) {
        want.SetType(typeVal);
    }

    for (auto entity : entities) {
        want.AddEntity(entity);
    }

    // 设置参数
    SetParams(parametersInteger, want);
    SetParams(parametersString, want);
    SetParams(parametersBool, want);

    // 设置窗口属性
    if (hasWindowLeft) {
        want.SetParam(Want::PARAM_RESV_WINDOW_LEFT, windowLeft);
    }
    if (hasWindowTop) {
        want.SetParam(Want::PARAM_RESV_WINDOW_TOP, windowTop);
    }
    if (hasWindowHeight) {
        want.SetParam(Want::PARAM_RESV_WINDOW_HEIGHT, windowHeight);
    }
    if (hasWindowWidth) {
        want.SetParam(Want::PARAM_RESV_WINDOW_WIDTH, windowWidth);
    }

    return result;
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
