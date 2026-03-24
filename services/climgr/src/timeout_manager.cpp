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

#include "timeout_manager.h"
#include "process_manager.h"
#include "event_dispatcher.h"
#include "hilog_tag_wrapper.h"
#include <algorithm>

namespace OHOS {
namespace CliMgr {

TimeoutManager::TimeoutManager(ProcessManager* processMgr,
                               EventDispatcher* eventDispatcher)
    : processManager_(processMgr),
      eventDispatcher_(eventDispatcher)
{
}

TimeoutManager::~TimeoutManager()
{
    Stop();
}

bool TimeoutManager::Start()
{
    if (running_.load()) {
        TAG_LOGW(TAG, "TimeoutManager already running");
        return true;
    }

    running_.store(true);
    timeoutThread_ = std::thread(&TimeoutManager::TimeoutCheckThread, this);

    TAG_LOGI(TAG, "TimeoutManager started");
    return true;
}

void TimeoutManager::Stop()
{
    if (!running_.load()) {
        return;
    }

    running_.store(false);

    // 唤醒等待的线程
    {
        std::lock_guard<std::mutex> lock(cvMutex_);
        cv_.notify_all();
    }

    if (timeoutThread_.joinable()) {
        timeoutThread_.join();
    }

    TAG_LOGI(TAG, "TimeoutManager stopped");
}

void TimeoutManager::AddTimeoutTask(const std::string& taskId,
                                   std::shared_ptr<ExecutionContext> context,
                                   uint32_t timeoutMs)
{
    if (!context) {
        TAG_LOGE(TAG, "Invalid context for task %{public}s", taskId.c_str());
        return;
    }

    TimeoutTask task;
    task.taskId = taskId;
    task.timeoutMs = timeoutMs;
    task.deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeoutMs);
    task.context = context;

    {
        std::lock_guard<std::mutex> lock(tasksMutex_);
        timeoutTasks_[taskId] = task;
    }

    // 唤醒检查线程
    {
        std::lock_guard<std::mutex> lock(cvMutex_);
        cv_.notify_one();
    }

    TAG_LOGI(TAG, "Timeout task added: %{public}s, timeout=%{public}u ms",
             taskId.c_str(), timeoutMs);
}

void TimeoutManager::CancelTimeoutTask(const std::string& taskId)
{
    std::lock_guard<std::mutex> lock(tasksMutex_);
    timeoutTasks_.erase(taskId);

    TAG_LOGI(TAG, "Timeout task cancelled: %{public}s", taskId.c_str());
}

bool TimeoutManager::IsTaskTimedOut(const std::string& taskId)
{
    std::lock_guard<std::mutex> lock(tasksMutex_);
    auto it = timeoutTasks_.find(taskId);
    return it == timeoutTasks_.end();
}

void TimeoutManager::TimeoutCheckThread()
{
    TAG_LOGI(TAG, "Timeout check thread started");

    while (running_.load()) {
        std::vector<TimeoutTask> timedOutTasks;

        {
            std::lock_guard<std::mutex> lock(tasksMutex_);

            auto now = std::chrono::steady_clock::now();

            for (auto it = timeoutTasks_.begin(); it != timeoutTasks_.end();) {
                const auto& task = it->second;

                if (now >= task.deadline) {
                    // 任务超时
                    timedOutTasks.push_back(task);
                    it = timeoutTasks_.erase(it);
                } else {
                    ++it;
                }
            }
        }

        // 处理超时任务
        for (auto& task : timedOutTasks) {
            HandleTimeout(task);
        }

        // 等待下一次检查或被唤醒
        std::unique_lock<std::mutex> lock(cvMutex_);
        cv_.wait_for(lock, std::chrono::milliseconds(CHECK_INTERVAL_MS));
    }

    TAG_LOGI(TAG, "Timeout check thread exited");
}

void TimeoutManager::HandleTimeout(TimeoutTask& task)
{
    TAG_LOGW(TAG, "Task timeout: %{public}s", task.taskId.c_str());

    // 标记任务超时
    if (task.context) {
        task.context->isTimedOut();
        task.context->isCompleted.store(true);
    }

    // 终止进程
    if (task.context && task.context->processId > 0) {
        TAG_LOGI(TAG, "Terminating timed out process: pid=%{public}d",
                 task.context->processId);

        // 先发送 SIGTERM
        processManager_->TerminateProcess(task.context->processId, SIGTERM);

        // 等待 3 秒
        std::this_thread::sleep_for(std::chrono::seconds(3));

        // 如果进程仍在运行，发送 SIGKILL
        if (task.context->isRunning.load()) {
            TAG_LOGW(TAG, "Force killing process: pid=%{public}d",
                     task.context->processId);
            processManager_->TerminateProcess(task.context->processId, SIGKILL);
        }
    }

    // 通知超时错误
    NotifyTimeout(task.taskId);
}

void TimeoutManager::NotifyTimeout(const std::string& taskId)
{
    if (eventDispatcher_) {
        eventDispatcher_->DispatchErrorEvent(
            taskId,
            ErrCode::ERR_TIMEOUT,
            "Execution timed out"
        );
    }
}

} // namespace CliMgr
} // namespace OHOS
