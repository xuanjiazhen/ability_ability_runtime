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
#include <sstream>
#include <cstring>
#include <ctime>

// Constants for magic numbers
namespace {
    constexpr int PROGRESS_MAX = 100;
    constexpr int MIN_ARGC = 2;
}

// 输出 progress 事件
void EmitProgress(int percentage, const std::string& status)
{
    std::cout << "{\"event\": \"progress\", \"data\": {"
              << "\"percentage\": " << percentage << ", "
              << "\"status\": \"" << status << "\""
              << "}}" << std::endl;
}

// 输出 result 事件
void EmitResult(const std::string& result)
{
    std::cout << "{\"event\": \"result\", \"data\": {"
              << "\"result\": \"" << result << "\""
              << "}}" << std::endl;
}

// 输出带字段的 result 事件（用于 version 命令）
void EmitResultWithFields(const std::string& version, const std::string& buildTime)
{
    std::cout << "{\"event\": \"result\", \"data\": {"
              << "\"version\": \"" << version << "\", "
              << "\"build_time\": \"" << buildTime << "\""
              << "}}" << std::endl;
}

// 输出 error 事件
void EmitError(const std::string& error)
{
    std::cout << "{\"event\": \"error\", \"data\": {"
              << "\"error\": \"" << error << "\""
              << "}}" << std::endl;
}

// run 子命令实现
int RunCommand(const std::vector<std::string>& args)
{
    EmitProgress(0, "starting");

    // 模拟执行：拼接所有参数
    std::string result = "执行完成";
    for (size_t i = 0; i < args.size(); i++) {
        if (i > 0 || !result.empty()) {
            result += " ";
        }
        result += args[i];

        // 模拟进度更新
        int progress = static_cast<int>((i + 1) * PROGRESS_MAX / (args.size() + 1));
        EmitProgress(progress, "running");
    }

    EmitProgress(PROGRESS_MAX, "completed");
    EmitResult(result);

    return 0;
}

// version 子命令实现
int VersionCommand()
{
    // 使用 std::string 代替 snprintf，避免使用不安全函数
    std::string version = "1.0.0";

    // 使用静态构建时间字符串（避免使用 __DATE__ 和 __TIME__）
    const char* buildTime = "2026-04-04 00:00:00";

    EmitResultWithFields(version, buildTime);

    return 0;
}

// 显示帮助信息
void ShowHelp()
{
    std::cout << "{\"event\": \"stdout\", \"data\": {"
              << "\"message\": \"Usage: cli-example <subcommand> [args]\\n"
              << "Subcommands:\\n"
              << "  run [args...]     Run the tool with arguments\\n"
              << "  version          Show version information\\n"
              << "  help             Show this help message\""
              << "}}" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc < MIN_ARGC) {
        EmitError("No subcommand specified");
        ShowHelp();
        return 1;
    }

    std::string subcommand = argv[1];

    // 解析子命令
    if (subcommand == "run") {
        // 收集参数（跳过 'run' 子命令）
        std::vector<std::string> args;
        for (int i = 2; i < argc; ++i) {
            args.push_back(argv[i]);
        }
        return RunCommand(args);

    } else if (subcommand == "version") {
        return VersionCommand();
    } else if (subcommand == "help" || subcommand == "--help" || subcommand == "-h") {
        ShowHelp();
        return 0;
    } else {
        std::string error = "Unknown subcommand: " + subcommand;
        EmitError(error);
        ShowHelp();
        return 1;
    }

    return 0;
}
