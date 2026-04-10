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

// 显示帮助信息
void ShowHelp()
{
    std::cout << "{\"event\": \"stdout\", \"data\": {"
              << "\"message\": \"Usage: cli-aa [options]\\n"
              << "Options:\\n"
              << "  -a, --ability <name>      Ability name\\n"
              << "  -b, --bundle <name>       Bundle name\\n"
              << "  -m, --module <name>       Module name\\n"
              << "  -A, --action <name>       Action name\\n"
              << "  -U, --uri <URI>           URI\\n"
              << "  -e, --entity <entity>     Entity\\n"
              << "  -t, --type <mime-type>    MIME type\\n"
              << "  -u, --user <id>           User ID\\n"
              << "  -W, --wait                Wait for ability to start\\n"
              << "  --ps <key> <string>       String parameter\\n"
              << "  --pi <key> <int>          Integer parameter\\n"
              << "  --pb <key> <bool>         Boolean parameter\\n"
              << "  --psn <key>               Null string parameter\\n"
              << "  --wl <left>               Window left\\n"
              << "  --wt <top>                Window top\\n"
              << "  --wh <height>             Window height\\n"
              << "  --ww <width>              Window width\\n"
              << "  -h, --help                Show this help message\""
              << "}}" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "{\"event\": \"error\", \"data\": {"
                  << "\"error\": \"No arguments provided. Use -h for help.\""
                  << "}}" << std::endl;
        return 1;
    }

    // 检查帮助参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" || arg == "--help") {
            ShowHelp();
            return 0;
        }
    }

    // 创建启动命令对象
    auto command = std::make_unique<AAStartCommand>();

    // 构建参数列表（跳过程序名）
    std::vector<std::string> args;
    for (int i = 1; i < argc; ++i) {
        args.push_back(argv[i]);
    }

    // 执行启动命令
    OHOS::ErrCode result = command->Execute(args);

    return (result == OHOS::ERR_OK) ? 0 : 1;
}
