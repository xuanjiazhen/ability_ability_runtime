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

// 输出 result 事件
void emitResult(const std::string& status, const std::string& message, int repeat_count) {
    std::cout << "{\"event\": \"result\", \"data\": {"
              << "\"status\": \"" << status << "\", "
              << "\"message\": \"" << message << "\", "
              << "\"repeat_count\": " << repeat_count
              << "}}" << std::endl;
}

// 输出 error 事件
void emitError(const std::string& error) {
    std::cout << "{\"event\": \"error\", \"data\": {"
              << "\"error\": \"" << error << "\""
              << "}}" << std::endl;
}

// 输出 stdout 事件
void emitStdout(const std::string& message) {
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
            message = arg.substr(10); // --message= 后面的内容
        } else if (arg.find("--count=") == 0) {
            count = std::atoi(arg.substr(8).c_str());
            if (count < 1) count = 1;
            if (count > 10) count = 10;
        } else if (arg == "--verbose") {
            verbose = true;
        } else if (arg == "--help" || arg == "-h") {
            emitStdout("Usage: cli-simple [options]");
            emitStdout("Options:");
            emitStdout("  --message=<msg>  Set message to display (default: 'Hello from cli-simple')");
            emitStdout("  --count=<num>    Number of repetitions (1-10, default: 1)");
            emitStdout("  --verbose        Enable verbose output");
            emitStdout("  --help, -h       Show this help message");
            return 0;
        }
    }

    // 验证参数
    if (message.empty()) {
        emitError("Message cannot be empty");
        return 1;
    }

    // 详细输出模式
    if (verbose) {
        emitStdout("Processing with parameters:");
        emitStdout("  Message: " + message);
        emitStdout("  Count: " + std::to_string(count));
        emitStdout("  Verbose: enabled");
    }

    // 执行主逻辑：重复消息并拼接
    std::string result;
    for (int i = 0; i < count; ++i) {
        if (i > 0) {
            result += " ";
        }
        result += message;

        if (verbose) {
            emitStdout("Iteration " + std::to_string(i + 1) + "/" + std::to_string(count));
        }
    }

    // 输出最终结果
    emitResult("success", result, count);

    return 0;
}
