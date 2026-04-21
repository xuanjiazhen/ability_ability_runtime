// Copyright (c) 2025 Huawei Device Co., Ltd.
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "aa_start_command.h"

using namespace OHOS::AAFwk;

namespace {
constexpr int MIN_ARGC = 2;
constexpr int COMMAND_ARG_INDEX = 1;
constexpr int ARGS_START_INDEX = 2;

constexpr const char* CLI_AA_VERSION = "1.0.0";
constexpr const char* CLI_AA_HELP =
    "usage: ohos-aa35 <command> <options>\n"
    "These are common ohos-aa35 commands list:\n"
    "  help                        list available commands\n"
    "  start                       start ability with options\n"
    "  -v, --version               show version information\n";
}

void EmitVersion()
{
    std::cout << "{\"event\": \"result\", \"data\": {"
              << "\"version\": \"" << CLI_AA_VERSION << "\""
              << "}}" << std::endl;
}

void EmitStdout(const std::string& message)
{
    std::cout << "{\"event\": \"stdout\", \"data\": {" <<
                 "\"message\": \"" << message << "\"" <<
                 "}}" << std::endl;
}

void ShowHelp()
{
    EmitStdout(CLI_AA_HELP);
}

int main(int argc, char* argv[])
{
    if (argc < MIN_ARGC) {
        std::cerr << "No command specified" << std::endl;
        return 1;
    }

    std::string command = argv[COMMAND_ARG_INDEX];

    if (command == "-v" || command == "--version") {
        EmitVersion();
        return 0;
    }

    if (command == "help") {
        ShowHelp();
        return 0;
    }

    if (command != "start") {
        std::cerr << "Unknown command: " << command << std::endl;
        return 1;
    }

    // 创建 start 子命令对象
    auto startCommand = std::make_unique<AAStartCommand>();

    // 构建参数列表（跳过程序名和子命令名）
    std::vector<std::string> args;
    for (int i = ARGS_START_INDEX; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    OHOS::ErrCode result = startCommand->Execute(args);

    return (result == OHOS::ERR_OK) ? 0 : 1;
}
