/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_SESSION_RECORD_H
#define OHOS_ABILITY_RUNTIME_SESSION_RECORD_H

#include <atomic>
#include <memory>
#include <mutex>
#include <string>
#include <chrono>
#include <sys/types.h>
#include <sys/wait.h>

#include "cli_manager_data.h"

namespace OHOS {
namespace AbilityRuntime {

enum class SessionState {
    SPAWNING = 0,
    RUNNING,
    CANCELLING,
};

class SessionRecord {
public:
    SessionRecord() = default;
    ~SessionRecord() = default;

    std::string sessionId;
    std::string toolName;
    pid_t processId = -1;
    int64_t startTime = 0;
    int32_t timeoutMs = 0;

    void SetState(SessionState state)
    {
        state_.store(state, std::memory_order_release);
    }

    SessionState GetState() const
    {
        return state_.load(std::memory_order_acquire);
    }

    void SetTerminalResult(int status, bool timedOut)
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        terminalStatus_ = status;
        timedOut_ = timedOut;
        endTimeMs_ = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count();
        processExited_.store(true, std::memory_order_release);
    }

    int GetTerminalStatus() const
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        return terminalStatus_;
    }

    bool TimedOut() const
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        return timedOut_;
    }

    int64_t GetEndTimeMs() const
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        return endTimeMs_;
    }

    bool HasProcessExited() const
    {
        return processExited_.load(std::memory_order_acquire);
    }

    void MarkStdoutClosed()
    {
        stdoutClosed_.store(true, std::memory_order_release);
    }

    void MarkStderrClosed()
    {
        stderrClosed_.store(true, std::memory_order_release);
    }

    bool OutputDrained() const
    {
        return stdoutClosed_.load(std::memory_order_acquire) &&
            stderrClosed_.load(std::memory_order_acquire);
    }

    bool BeginCleanup()
    {
        bool expected = false;
        return cleanupStarted_.compare_exchange_strong(expected, true, std::memory_order_acq_rel);
    }

    void AppendOutput(bool isStdout, const std::string &data)
    {
        std::lock_guard<std::mutex> lock(resultMutex_);
        if (isStdout) {
            stdoutText_ += data;
            TrimBufferedOutput(stdoutText_);
            return;
        }
        stderrText_ += data;
        TrimBufferedOutput(stderrText_);
    }

    std::shared_ptr<ExecResult> BuildExecResult() const
    {
        if (!HasProcessExited() || !OutputDrained()) {
            return nullptr;
        }

        auto result = std::make_shared<ExecResult>();
        if (result == nullptr) {
            return nullptr;
        }

        std::lock_guard<std::mutex> lock(resultMutex_);
        result->exitCode = WIFEXITED(terminalStatus_) ? WEXITSTATUS(terminalStatus_) : -1;
        result->outputText = stdoutText_;
        result->errorText = stderrText_;
        result->signalNumber = WIFSIGNALED(terminalStatus_) ? WTERMSIG(terminalStatus_) : 0;
        result->timedOut = timedOut_;
        result->executionTime = (endTimeMs_ > startTime) ? (endTimeMs_ - startTime) : 0;
        return result;
    }

private:
    void TrimBufferedOutput(std::string &buffer)
    {
        if (buffer.size() <= MAX_BUFFERED_OUTPUT_BYTES) {
            return;
        }
        buffer.erase(0, buffer.size() - MAX_BUFFERED_OUTPUT_BYTES);
    }

    std::atomic<SessionState> state_ {SessionState::SPAWNING};
    std::atomic<bool> processExited_ {false};
    std::atomic<bool> stdoutClosed_ {false};
    std::atomic<bool> stderrClosed_ {false};
    std::atomic<bool> cleanupStarted_ {false};
    mutable std::mutex resultMutex_;
    int terminalStatus_ = 0;
    bool timedOut_ = false;
    int64_t endTimeMs_ = 0;
    std::string stdoutText_;
    std::string stderrText_;

    static constexpr size_t MAX_BUFFERED_OUTPUT_BYTES = 64 * 1024;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SESSION_RECORD_H
