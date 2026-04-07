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
#include <chrono>
#include <thread>
#include <cstdlib>
#include <cerrno>
#include <climits>

// Constants for magic numbers
namespace {
    constexpr int PROGRESS_PERCENTAGE_MAX = 100;
    constexpr int DEFAULT_INTERVAL = 1;
    constexpr int MIN_DURATION = 1;
    constexpr int MIN_INTERVAL = 1;
    constexpr int DURATION_PREFIX_LEN = 11;
    constexpr int INTERVAL_PREFIX_LEN = 11;
    constexpr int HELP_ARGC = 2;
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
void EmitResult(const std::string& status, int duration, int actualDuration)
{
    std::cout << "{\"event\": \"result\", \"data\": {"
              << "\"status\": \"" << status << "\", "
              << "\"duration\": " << duration << ", "
              << "\"actual_duration\": " << actualDuration
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

// 显示帮助信息
void ShowHelp()
{
    EmitStdout("Usage: cli-timer [options]");
    EmitStdout("Options:");
    EmitStdout("  --duration=<sec>  Duration in seconds (required, minimum 1)");
    EmitStdout("  --interval=<sec>  Progress update interval in seconds (optional, default 1)");
    EmitStdout("  --progress        Enable progress events");
    EmitStdout("  --verbose         Enable verbose output");
    EmitStdout("  --help, -h        Show this help message");
}

// 解析命令行参数
bool ParseArguments(int argc, char* argv[], int& duration, int& interval, bool& showProgress, bool& verbose)
{
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg.find("--duration=") == 0) {
            std::string value = arg.substr(DURATION_PREFIX_LEN);
            char* end = nullptr;
            errno = 0;
            long val = std::strtol(value.c_str(), &end, 10);
            if (errno != 0 || end == value.c_str() || *end != '\0' || val < 0 || val > INT_MAX) {
                EmitError("Invalid duration value: " + value);
                return false;
            }
            duration = static_cast<int>(val);
        } else if (arg.find("--interval=") == 0) {
            std::string value = arg.substr(INTERVAL_PREFIX_LEN);
            char* end = nullptr;
            errno = 0;
            long val = std::strtol(value.c_str(), &end, 10);
            if (errno != 0 || end == value.c_str() || *end != '\0' || val < 0 || val > INT_MAX) {
                EmitError("Invalid interval value: " + value);
                return false;
            }
            interval = static_cast<int>(val);
        } else if (arg == "--progress") {
            showProgress = true;
        } else if (arg == "--verbose") {
            verbose = true;
        } else if (arg == "--help" || arg == "-h") {
            ShowHelp();
            return false;
        }
    }
    return true;
}

// 验证参数
bool ValidateArguments(int duration, int interval)
{
    if (duration < MIN_DURATION) {
        EmitError("Duration must be at least " + std::to_string(MIN_DURATION) + " second");
        return false;
    }

    if (interval < MIN_INTERVAL) {
        EmitError("Interval must be at least " + std::to_string(MIN_INTERVAL) + " second");
        return false;
    }

    if (interval > duration) {
        EmitError("Interval cannot be greater than duration");
        return false;
    }

    return true;
}

// 执行定时器
int ExecuteTimer(int duration, int interval, bool showProgress, bool verbose)
{
    if (verbose) {
        EmitStdout("Starting timer with duration: " + std::to_string(duration) +
                   "s, interval: " + std::to_string(interval) + "s");
    }

    auto startTime = std::chrono::steady_clock::now();
    int lastPercentage = -1;
    int elapsed = 0;

    if (showProgress) {
        EmitProgress(0, "starting");
    }

    while (elapsed < duration) {
        if (verbose) {
            EmitStdout("Elapsed: " + std::to_string(elapsed) + "s");
        }

        if (showProgress) {
            int percentage = static_cast<int>(
                (elapsed * PROGRESS_PERCENTAGE_MAX) / duration
            );

            if (percentage > lastPercentage) {
                if (percentage > 0) {
                    EmitProgress(percentage, "running");
                }
                lastPercentage = percentage;
            }
        }

        std::this_thread::sleep_for(std::chrono::seconds(interval));

        elapsed = static_cast<int>(
            std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::steady_clock::now() - startTime
            ).count()
        );
    }

    if (verbose) {
        EmitStdout("Elapsed: " + std::to_string(elapsed) + "s");
    }

    if (showProgress) {
        EmitProgress(PROGRESS_PERCENTAGE_MAX, "completed");
    }
    EmitResult("success", duration, elapsed);

    return 0;
}

int main(int argc, char* argv[])
{
    int duration = 0;
    int interval = DEFAULT_INTERVAL;
    bool showProgress = false;
    bool verbose = false;

    if (argc < HELP_ARGC) {
        EmitError("No arguments specified");
        ShowHelp();
        return 1;
    }

    if (!ParseArguments(argc, argv, duration, interval, showProgress, verbose)) {
        return 0;
    }

    if (duration == 0) {
        EmitError("Duration is required");
        ShowHelp();
        return 1;
    }

    if (!ValidateArguments(duration, interval)) {
        return 1;
    }

    return ExecuteTimer(duration, interval, showProgress, verbose);
}
