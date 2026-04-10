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

#include "timeout_manager.h"
#include "hilog_tag_wrapper.h"
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

namespace OHOS {
namespace AbilityRuntime {

TimeoutManager::TimeoutManager()
{
}

TimeoutManager::~TimeoutManager()
{
    Stop();
}

void TimeoutManager::Start()
{
    if (running_.load()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "TimeoutManager already running");
        return;
    }

    running_.store(true);
    timeoutThread_ = std::thread(&TimeoutManager::TimeoutCheckThread, this);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "TimeoutManager started");
}

void TimeoutManager::Stop()
{
    if (!running_.load()) {
        return;
    }

    running_.store(false);

    // Wake up the waiting thread
    {
        std::lock_guard<std::mutex> lock(cvMutex_);
        cv_.notify_all();
    }

    if (timeoutThread_.joinable()) {
        timeoutThread_.join();
    }

    // Wait for all async timeout tasks to complete with timeout
    {
        std::lock_guard<std::mutex> lock(asyncMutex_);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Waiting for %{public}zu async timeout tasks to complete (timeout: %{public}u s)...",
            asyncResults_.size(), STOP_TIMEOUT_SEC);

        for (auto& future : asyncResults_) {
            if (future.valid()) {
                if (future.wait_for(std::chrono::seconds(STOP_TIMEOUT_SEC)) == std::future_status::timeout) {
                    TAG_LOGW(AAFwkTag::ABILITYMGR, "Async task did not complete within %{public}u seconds, forcing stop",
                        STOP_TIMEOUT_SEC);
                } else {
                    TAG_LOGI(AAFwkTag::ABILITYMGR, "Async task completed successfully");
                }
            }
        }
        asyncResults_.clear();
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "TimeoutManager stopped");
}

void TimeoutManager::SetTimeoutCallback(TimeoutCallback callback)
{
    timeoutCallback_ = std::move(callback);
}

uint32_t TimeoutManager::AddTimeoutTask(const std::string& sessionId,
                                       pid_t processId,
                                       std::shared_ptr<TimeoutContext> context,
                                       uint32_t timeoutMs)
{
    if (!context) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid context for session %{public}s", sessionId.c_str());
        return 0;
    }

    // If timeout is 0, no timeout monitoring
    if (timeoutMs == 0) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "No timeout for session %{public}s", sessionId.c_str());
        return 0;
    }

    TimeoutTask task;
    task.sessionId = sessionId;
    task.processId = processId;
    task.context = context;
    task.timeoutMs = timeoutMs;
    task.deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);

    {
        std::lock_guard<std::mutex> lock(tasksMutex_);
        timeoutTasks_[sessionId] = task;
    }

    // Wake up check thread
    {
        std::lock_guard<std::mutex> lock(cvMutex_);
        cv_.notify_one();
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Timeout task added: %{public}s, timeout=%{public}u ms",
        sessionId.c_str(), timeoutMs);

    return timeoutMs;
}

void TimeoutManager::CancelTimeoutTask(const std::string& sessionId)
{
    std::lock_guard<std::mutex> lock(tasksMutex_);
    timeoutTasks_.erase(sessionId);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Timeout task cancelled: %{public}s", sessionId.c_str());
}

bool TimeoutManager::IsTaskTimedOut(const std::string& sessionId)
{
    if (!sessionId.empty()) {
        // Check if context exists and is marked as timed out
        std::lock_guard<std::mutex> lock(tasksMutex_);
        auto it = timeoutTasks_.find(sessionId);
        if (it != timeoutTasks_.end() && it->second.context) {
            return it->second.context->isTimedOut.load();
        }
    }
    return false;
}

void TimeoutManager::TimeoutCheckThread()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Timeout check thread started");

    while (running_.load()) {
        std::vector<TimeoutTask> timedOutTasks;

        {
            std::lock_guard<std::mutex> lock(tasksMutex_);

            auto now = std::chrono::steady_clock::now();

            for (auto it = timeoutTasks_.begin(); it != timeoutTasks_.end();) {
                const auto& task = it->second;

                if (now >= task.deadline) {
                    // Task timed out
                    timedOutTasks.push_back(task);
                    it = timeoutTasks_.erase(it);
                } else {
                    ++it;
                }
            }
        }

        // Handle timed out tasks asynchronously
        if (!timedOutTasks.empty()) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Processing %{public}zu timed out tasks asynchronously",
                timedOutTasks.size());

            for (const auto& task : timedOutTasks) {
                // Check maximum future limit before launching
                {
                    std::lock_guard<std::mutex> lock(asyncMutex_);
                    if (asyncResults_.size() >= MAX_ASYNC_TASKS) {
                        TAG_LOGW(AAFwkTag::ABILITYMGR,
                            "Too many async tasks (%{public}zu, max: %{public}zu), skipping timeout for %{public}s",
                            asyncResults_.size(), MAX_ASYNC_TASKS, task.sessionId.c_str());
                        continue;
                    }
                }

                // Launch async task to handle timeout
                auto future = std::async(std::launch::async, [this, task]() {
                    HandleTimeout(task);
                });

                // Store future for cleanup on Stop()
                {
                    std::lock_guard<std::mutex> lock(asyncMutex_);
                    asyncResults_.push_back(std::move(future));
                }
            }
        }

        // Wait for next check or be woken up
        std::unique_lock<std::mutex> lock(cvMutex_);
        cv_.wait_for(lock, std::chrono::milliseconds(CHECK_INTERVAL_MS));
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Timeout check thread exited");
}

void TimeoutManager::HandleTimeout(const struct TimeoutTask& task)
{
    TAG_LOGW(AAFwkTag::ABILITYMGR, "Task timeout: %{public}s (pid=%{public}d, timeout=%{public}u ms)",
        task.sessionId.c_str(), task.processId, task.timeoutMs);

    // Mark task as timed out
    if (task.context) {
        task.context->isTimedOut.store(true);
        task.context->isCompleted.store(true);
    }

    // Terminate process
    if (task.processId > 0) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Terminating timed out process: pid=%{public}d", task.processId);
        TerminateProcess(task.processId);
    }

    // Notify callback
    if (timeoutCallback_) {
        timeoutCallback_(task.sessionId);
    }
}

void TimeoutManager::TerminateProcess(pid_t pid)
{
    if (pid <= 0) {
        return;
    }

    // Check if process exists
    if (kill(pid, 0) == -1 && errno == ESRCH) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Process %{public}d already terminated", pid);
        return;
    }

    // Send SIGTERM first (graceful shutdown)
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Sending SIGTERM to process %{public}d", pid);
    if (kill(pid, SIGTERM) == -1) {
        if (errno == ESRCH) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Process %{public}d already terminated", pid);
            return;
        } else if (errno == EPERM) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "SIGTERM permission denied for pid=%{public}d, possibly running in sandbox (claw_sandbox)", pid);
        } else {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to send SIGTERM to %{public}d: %{public}s",
                pid, strerror(errno));
        }
    }

    // Wait for grace period
    std::this_thread::sleep_for(std::chrono::milliseconds(GRACE_PERIOD_MS));

    // Check if process still exists
    if (kill(pid, 0) == -1 && errno == ESRCH) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Process %{public}d terminated gracefully", pid);
        return;
    }

    // Force kill with SIGKILL
    TAG_LOGW(AAFwkTag::ABILITYMGR, "Force killing process %{public}d with SIGKILL", pid);
    if (kill(pid, SIGKILL) == -1) {
        if (errno == ESRCH) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Process %{public}d already terminated", pid);
        } else if (errno == EPERM) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "SIGKILL permission denied for pid=%{public}d, possibly running in sandbox (claw_sandbox)", pid);
        } else {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to send SIGKILL to %{public}d: %{public}s",
                pid, strerror(errno));
        }
    }

    // Wait to ensure process is reaped
    int status;
    waitpid(pid, &status, WNOHANG);
}

} // namespace AbilityRuntime
} // namespace OHOS
