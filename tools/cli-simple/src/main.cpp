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
#include <cstdlib>

// Constants for magic numbers
namespace {
    constexpr int MESSAGE_PREFIX_LEN = 10;
    constexpr int COUNT_PREFIX_LEN = 8;
    constexpr int MAX_COUNT = 10;
    constexpr int MIN_COUNT = 1;
}

// 输出 result 事件
void EmitResult(const std::string& status, const std::string& message, int repeatCount)
{
    std::cout << "{\"event\": \"result\", \"data\": {"
              << "\"status\": \"" << status << "\", "
              << "\"message\": \"" << message << "\", "
              << "\"repeat_count\": " << repeatCount
              << "}}" << std::endl;
}

// 输出 error 事件
void EmitError(const std::string& error)
{
    std::cout << "{\"event\": \"error\", \"data\": {"
              << "\"error\": \"" << error << "\""
              << "}}" << std::endl;
}

// 输出 stdout 事件
void EmitStdout(const std::string& message)
{
    std::cout << "{\"event\": \"stdout\", \"data\": {"
              << "\"message\": \"" << message << "\""
              << "}}" << std::endl;
}

int main(int argc, char* argv[])
{
    std::string message = "Hello from cli-simple";
    int count = 1;
    bool verbose = false;

    // 解析命令行参数
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg.find("--message=") == 0) {
            message = arg.substr(MESSAGE_PREFIX_LEN);
        } else if (arg.find("--count=") == 0) {
            count = std::atoi(arg.substr(COUNT_PREFIX_LEN).c_str());
            if (count < MIN_COUNT) {
                count = MIN_COUNT;
            }
            if (count > MAX_COUNT) {
                count = MAX_COUNT;
            }
        } else if (arg == "--verbose") {
            verbose = true;
        } else if (arg == "--help" || arg == "-h") {
            EmitStdout("Usage: cli-simple [options]");
            EmitStdout("Options:");
            EmitStdout("  --message=<msg>  Set message to display (default: 'Hello from cli-simple')");
            EmitStdout("  --count=<num>    Number of repetitions (1-10, default: 1)");
            EmitStdout("  --verbose        Enable verbose output");
            EmitStdout("  --help, -h       Show this help message");
            return 0;
        }
    }

    // 验证参数
    if (message.empty()) {
        EmitError("Message cannot be empty");
        return 1;
    }

    // 详细输出模式
    if (verbose) {
        EmitStdout("Processing with parameters:");
        EmitStdout("  Message: " + message);
        EmitStdout("  Count: " + std::to_string(count));
        EmitStdout("  Verbose: enabled");
    }

    // 执行主逻辑：重复消息并拼接
    std::string result;
    for (int i = 0; i < count; ++i) {
        if (i > 0) {
            result += " ";
        }
        result += message;

        if (verbose) {
            EmitStdout("Iteration " + std::to_string(i + 1) + "/" + std::to_string(count));
        }
    }

    // 输出最终结果
    EmitResult("success", result, count);

    return 0;
}
