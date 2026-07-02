/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "shell_command_executor.h"

#include <chrono>
#include <cinttypes>
#include <iostream>
#include <sstream>

#include "hilog_tag_wrapper.h"
#include "shell_command_config_loader.h"

using namespace std::chrono_literals;
namespace OHOS {
namespace AAFwk {
ShellCommandExecutor::ShellCommandExecutor(const std::string& cmd, const int64_t timeoutSec)
    : timeoutSec_(timeoutSec), cmd_(cmd)
{
    handler_ = std::make_shared<AppExecFwk::EventHandler>(AppExecFwk::EventRunner::Create());
}

ShellCommandResult ShellCommandExecutor::WaitWorkDone()
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "enter");

    if (!DoWork()) {
        TAG_LOGI(AAFwkTag::AA_TOOL, "Failed cmd: \"%{public}s\"", cmd_.data());
        return cmdResult_;
    }

    std::unique_lock<std::mutex> workLock(mtxWork_);

    auto condition = [this]() { return isDone_; };
    if (timeoutSec_ <= 0) {
        cvWork_.wait(workLock, condition);
    } else if (!cvWork_.wait_for(workLock, timeoutSec_ * 1s, condition)) {
        TAG_LOGW(AAFwkTag::AA_TOOL, "cmd timed out! cmd : \"%{public}s\", timeoutSec : %{public}" PRId64,
            cmd_.data(), timeoutSec_);
        std::cout << "Warning! Command execution timed out! cmd : " << cmd_ << ", timeoutSec : " << timeoutSec_
            << std::endl;

        ShellCommandResult realResult;
        realResult.exitCode = -1;
        {
            std::lock_guard<std::mutex> copyLock(mtxCopy_);
            realResult.stdResult = cmdResult_.stdResult;
        }
        return realResult;
    }

    TAG_LOGI(AAFwkTag::AA_TOOL, "cmd complete, cmd : \"%{public}s\", exitCode : %{public}d",
        cmd_.data(), cmdResult_.exitCode);
    return cmdResult_;
}

bool ShellCommandExecutor::DoWork()
{
    TAG_LOGD(AAFwkTag::AA_TOOL, "enter");

    if (cmd_.empty()) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Invalid cmd_");
        return false;
    }

    if (!handler_) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Invalid handler_");
        return false;
    }
    
    if (!CheckCommand()) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Invalid command");
        return false;
    }

    auto self(shared_from_this());
    handler_->PostTask([this, self]() {
        TAG_LOGI(AAFwkTag::AA_TOOL, "DoWork task begin, cmd: \"%{public}s\"", cmd_.data());

        FILE* file = popen(cmd_.c_str(), "r");
        if (!file) {
            TAG_LOGE(AAFwkTag::AA_TOOL, "popen failed, cmd: \"%{public}s\"", cmd_.data());

            {
                std::unique_lock<std::mutex> workLock(mtxWork_);
                isDone_ = true;
            }
            cvWork_.notify_one();
            TAG_LOGI(AAFwkTag::AA_TOOL, "DoWork task end, cmd: \"%{public}s\"", cmd_.data());
            return;
        }

        char commandResult[1024] = { '\0' };
        while ((fgets(commandResult, sizeof(commandResult), file)) != nullptr) {
            {
                std::lock_guard<std::mutex> copyLock(mtxCopy_);
                cmdResult_.stdResult.append(commandResult);
            }
            std::cout << commandResult;
        }

        cmdResult_.exitCode = pclose(file);
        file = nullptr;

        {
            std::unique_lock<std::mutex> workLock(mtxWork_);
            isDone_ = true;
        }
        cvWork_.notify_one();
        TAG_LOGI(AAFwkTag::AA_TOOL, "DoWork task end, cmd: \"%{public}s\"", cmd_.data());
    });

    return true;
}

bool ShellCommandExecutor::CheckCommand()
{
    if (HasDangerousMetaChars(cmd_)) {
        TAG_LOGE(AAFwkTag::AA_TOOL, "Command contains dangerous meta chars");
        return false;
    }
    auto subCommands = SplitCommandByOperators(cmd_);
    for (const auto& subCmd : subCommands) {
        std::istringstream iss(subCmd);
        std::string firstWord;
        iss >> firstWord;
        if (firstWord.empty()) {
            continue;
        }
        if (ShellCommandConfigLoader::commands_.find(firstWord) ==
            ShellCommandConfigLoader::commands_.end()) {
            TAG_LOGE(AAFwkTag::AA_TOOL, "Invalid sub-command: %{public}s", firstWord.c_str());
            return false;
        }
    }
    return true;
}

bool ShellCommandExecutor::HasDangerousMetaChars(const std::string& cmd)
{
    bool inSingleQuote = false;
    bool inDoubleQuote = false;
    for (size_t i = 0; i < cmd.size(); ++i) {
        char c = cmd[i];
        if (c == '\'' && !inDoubleQuote) {
            inSingleQuote = !inSingleQuote;
        } else if (c == '"' && !inSingleQuote) {
            inDoubleQuote = !inDoubleQuote;
        } else if (!inSingleQuote) {
            if (c == '`') {
                return true;
            }
            if (c == '$' && i + 1 < cmd.size() && cmd[i + 1] == '(') {
                return true;
            }
        }
    }
    return false;
}

std::vector<std::string> ShellCommandExecutor::SplitCommandByOperators(const std::string& cmd)
{
    std::vector<std::string> subCommands;
    std::string remaining = cmd;
    static const std::vector<std::pair<std::string, size_t>> seps = {
        {"||", 2}, {"&&", 2}, {"|", 1}, {";", 1}, {"\n", 1}, {"&", 1}
    };
    while (!remaining.empty()) {
        size_t minPos = remaining.size();
        size_t sepLen = 1;
        bool inSingleQuote = false;
        bool inDoubleQuote = false;
        for (size_t i = 0; i < remaining.size(); ++i) {
            char c = remaining[i];
            if (c == '\'' && !inDoubleQuote) {
                inSingleQuote = !inSingleQuote;
            } else if (c == '"' && !inSingleQuote) {
                inDoubleQuote = !inDoubleQuote;
            } else if (!inSingleQuote && !inDoubleQuote && i < minPos) {
                for (const auto& [sep, len] : seps) {
                    if (remaining.compare(i, len, sep) == 0) {
                        minPos = i;
                        sepLen = len;
                        break;
                    }
                }
            }
        }
        std::string part = remaining.substr(0, minPos);
        if (!part.empty()) {
            subCommands.push_back(part);
        }
        remaining = (minPos < remaining.size()) ? remaining.substr(minPos + sepLen) : "";
    }
    return subCommands;
}
}  // namespace AAFwk
}  // namespace OHOS
