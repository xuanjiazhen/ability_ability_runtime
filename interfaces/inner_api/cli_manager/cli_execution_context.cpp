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

#include "cli_execution_context.h"
#include <unistd.h>
#include <cstring>

namespace OHOS {
namespace CliMgr {

ExecutionContext::~ExecutionContext()
{
    CloseAllPipes();
}

ExecutionContext::ExecutionContext(ExecutionContext&& other) noexcept
    : taskId(std::move(other.taskId)),
      toolName(std::move(other.toolName)),
      mode(other.mode),
      processId(other.processId),
      startTime(other.startTime),
      endTime(other.endTime),
      caller(std::move(other.caller)),
      stdoutBuffer(std::move(other.stdoutBuffer)),
      stderrBuffer(std::move(other.stderrBuffer)),
      arguments(std::move(other.arguments)),
      timeoutMs(other.timeoutMs),
      workingDir(std::move(other.workingDir)),
      callback(other.callback),
      exitCode(other.exitCode),
      signal(other.signal)
{
    // 复制管道文件描述符
    stdoutPipe[0] = other.stdoutPipe[0];
    stdoutPipe[1] = other.stdoutPipe[1];
    stderrPipe[0] = other.stderrPipe[0];
    stderrPipe[1] = other.stderrPipe[1];
    stdinPipe[0] = other.stdinPipe[0];
    stdinPipe[1] = other.stdinPipe[1];

    // 复制状态标志
    isRunning.store(other.isRunning.load());
    isCompleted.store(other.isCompleted.load());

    // 清空源对象的文件描述符
    other.stdoutPipe[0] = -1;
    other.stdoutPipe[1] = -1;
    other.stderrPipe[0] = -1;
    other.stderrPipe[1] = -1;
    other.stdinPipe[0] = -1;
    other.stdinPipe[1] = -1;
}

ExecutionContext& ExecutionContext::operator=(ExecutionContext&& other) noexcept
{
    if (this != &other) {
        // 关闭现有管道
        CloseAllPipes();

        // 移动数据
        taskId = std::move(other.taskId);
        toolName = std::move(other.toolName);
        mode = other.mode;
        processId = other.processId;
        startTime = other.startTime;
        endTime = other.endTime;
        caller = std::move(other.caller);
        stdoutBuffer = std::move(other.stdoutBuffer);
        stderrBuffer = std::move(other.stderrBuffer);
        arguments = std::move(other.arguments);
        timeoutMs = other.timeoutMs;
        workingDir = std::move(other.workingDir);
        callback = other.callback;
        exitCode = other.exitCode;
        signal = other.signal;

        // 复制管道文件描述符
        stdoutPipe[0] = other.stdoutPipe[0];
        stdoutPipe[1] = other.stdoutPipe[1];
        stderrPipe[0] = other.stderrPipe[0];
        stderrPipe[1] = other.stderrPipe[1];
        stdinPipe[0] = other.stdinPipe[0];
        stdinPipe[1] = other.stdinPipe[1];

        // 复制状态标志
        isRunning.store(other.isRunning.load());
        isCompleted.store(other.isCompleted.load());

        // 清空源对象的文件描述符
        other.stdoutPipe[0] = -1;
        other.stdoutPipe[1] = -1;
        other.stderrPipe[0] = -1;
        other.stderrPipe[1] = -1;
        other.stdinPipe[0] = -1;
        other.stdinPipe[1] = -1;
    }

    return *this;
}

void ExecutionContext::CloseAllPipes()
{
    ClosePipe(stdoutPipe);
    ClosePipe(stderrPipe);
    ClosePipe(stdinPipe);
}

void ExecutionContext::CloseWriteEnds()
{
    if (stdoutPipe[1] >= 0) {
        close(stdoutPipe[1]);
        stdoutPipe[1] = -1;
    }
    if (stderrPipe[1] >= 0) {
        close(stderrPipe[1]);
        stderrPipe[1] = -1;
    }
    if (stdinPipe[1] >= 0) {
        close(stdinPipe[1]);
        stdinPipe[1] = -1;
    }
}

bool ExecutionContext::IsTimedOut() const
{
    if (timeoutMs == 0) {
        return false;
    }

    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();
    return elapsed >= static_cast<int64_t>(timeoutMs);
}

void ExecutionContext::ClosePipe(int pipe[2])
{
    if (pipe[0] >= 0) {
        close(pipe[0]);
        pipe[0] = -1;
    }
    if (pipe[1] >= 0) {
        close(pipe[1]);
        pipe[1] = -1;
    }
}

} // namespace CliMgr
} // namespace OHOS
