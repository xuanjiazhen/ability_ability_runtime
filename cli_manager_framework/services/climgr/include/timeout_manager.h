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

#ifndef OHOS_ABILITY_RUNTIME_TIMEOUT_MANAGER_H
#define OHOS_ABILITY_RUNTIME_TIMEOUT_MANAGER_H

#include <string>
#include <unordered_map>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include <memory>
#include <functional>
#include "cli_manager_data.h"

namespace OHOS {
namespace AbilityRuntime {

/**
 * @brief Timeout callback function type
 * @param sessionId Session ID that timed out
 */
using TimeoutCallback = std::function<void(const std::string&)>;

/**
 * @brief Timeout context for tracking task execution
 */
struct TimeoutContext {
    std::string sessionId;
    pid_t processId;
    std::atomic<bool> isTimedOut{false};
    std::atomic<bool> isCompleted{false};
    std::atomic<bool> isRunning{false};

    TimeoutContext() : processId(-1) {}
};

/**
 * @brief Timeout manager for CLI tool execution
 *
 * Monitors task execution time and terminates process on timeout.
 * Supports both synchronous and asynchronous execution modes.
 */
class TimeoutManager {
public:
    TimeoutManager();
    ~TimeoutManager();

    /**
     * @brief Start the timeout manager background thread
     */
    void Start();

    /**
     * @brief Stop the timeout manager and cleanup resources
     */
    void Stop();

    /**
     * @brief Set timeout callback
     * @param callback Function to call when timeout occurs
     */
    void SetTimeoutCallback(TimeoutCallback callback);

    /**
     * @brief Add timeout monitoring task
     * @param sessionId Session ID
     * @param processId Process ID to monitor
     * @param context Timeout context
     * @param timeoutMs Timeout in milliseconds
     * @return Actual timeout in milliseconds
     */
    uint32_t AddTimeoutTask(const std::string& sessionId,
                           pid_t processId,
                           std::shared_ptr<TimeoutContext> context,
                           uint32_t timeoutMs);

    /**
     * @brief Cancel timeout monitoring for a task
     * @param sessionId Session ID
     */
    void CancelTimeoutTask(const std::string& sessionId);

    /**
     * @brief Check if task has timed out
     * @param sessionId Session ID
     * @return true if timed out, false otherwise
     */
    bool IsTaskTimedOut(const std::string& sessionId);

private:
    /**
     * @brief Background thread for timeout checking
     */
    void TimeoutCheckThread();

    /**
     * @brief Handle timeout for a specific task
     * @param task Timeout task
     */
    void HandleTimeout(const struct TimeoutTask& task);

    /**
     * @brief Terminate process gracefully then forcefully
     * @param pid Process ID
     */
    void TerminateProcess(pid_t pid);

private:
    struct TimeoutTask {
        std::string sessionId;
        pid_t processId;
        std::shared_ptr<TimeoutContext> context;
        std::chrono::time_point<std::chrono::steady_clock> deadline;
        uint32_t timeoutMs;
    };

    std::unordered_map<std::string, TimeoutTask> timeoutTasks_;
    std::mutex tasksMutex_;

    std::atomic<bool> running_{false};
    std::thread timeoutThread_;
    std::condition_variable cv_;
    std::mutex cvMutex_;

    TimeoutCallback timeoutCallback_;

    static constexpr int32_t DEFAULT_TIMEOUT_SEC = 30 * 60;      // 30 minutes (default)
    static constexpr uint32_t CHECK_INTERVAL_MS = 100;      // 100ms check interval
    static constexpr uint32_t GRACE_PERIOD_MS = 3000;       // 3 seconds grace before SIGKILL
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_TIMEOUT_MANAGER_H
