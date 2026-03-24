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

#include "process_monitor.h"
#include "event_dispatcher.h"
#include "hilog_tag_wrapper.h"
#include <sys/wait.h>
#include <errno.h>
#include <unistd.h>

namespace OHOS {
namespace CliMgr {

ProcessMonitor::ProcessMonitor(EventDispatcher* eventDispatcher)
    : eventDispatcher_(eventDispatcher)
{
}

ProcessMonitor::~ProcessMonitor()
{
    Stop();
}

bool ProcessMonitor::Start()
{
    if (running_.load()) {
        TAG_LOGW(TAG, "ProcessMonitor already running");
        return true;
    }

    running_.store(true);
    monitorThread_ = std::thread(&ProcessMonitor::MonitorThread, this);

    TAG_LOGI(TAG, "ProcessMonitor started");
    return true;
}

void ProcessMonitor::Stop()
{
    if (!running_.load()) {
        return;
    }

    running_.store(false);

    // 发送信号以唤醒 waitpid
    // 这里可以通过 kill(getpid(), SIGCHLD) 来唤醒

    if (monitorThread_.joinable()) {
        monitorThread_.join();
    }

    TAG_LOGI(TAG, "ProcessMonitor stopped");
}

void ProcessMonitor::RegisterProcess(pid_t pid, const std::string& taskId)
{
    std::lock_guard<std::mutex> lock(pidMutex_);
    pidToTaskId_[pid] = taskId;

    TAG_LOGI(TAG, "Process registered: pid=%{public}d, task=%{public}s",
             pid, taskId.c_str());
}

void ProcessMonitor::UnregisterProcess(pid_t pid)
{
    std::lock_guard<std::mutex> lock(pidMutex_);
    pidToTaskId_.erase(pid);

    TAG_LOGI(TAG, "Process unregistered: pid=%{public}d", pid);
}

std::string ProcessMonitor::GetTaskId(pid_t pid)
{
    std::lock_guard<std::mutex> lock(pidMutex_);
    auto it = pidToTaskId_.find(pid);
    if (it != pidToTaskId_.end()) {
        return it->second;
    }
    return "";
}

void ProcessMonitor::MonitorThread()
{
    TAG_LOGI(TAG, "ProcessMonitor thread started");

    while (running_.load()) {
        int status;
        pid_t pid = waitpid(-1, &status, WNOHANG);  // 非阻塞

        if (pid > 0) {
            // 有进程退出
            TAG_LOGI(TAG, "Process exited: pid=%{public}d", pid);
            HandleProcessExit(pid, status);
        } else if (pid < 0) {
            if (errno == ECHILD) {
                // 没有子进程
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            } else if (errno == EINTR) {
                // 被信号中断，继续
                continue;
            } else {
                TAG_LOGE(TAG, "waitpid error: %{public}d", errno);
                break;
            }
        }

        // 避免 CPU 占用
        if (pid == 0) {  // 没有进程退出
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }

    TAG_LOGI(TAG, "ProcessMonitor thread exited");
}

void ProcessMonitor::HandleProcessExit(pid_t pid, int status)
{
    // 获取对应的 taskId
    std::string taskId;
    {
        std::lock_guard<std::mutex> lock(pidMutex_);
        auto it = pidToTaskId_.find(pid);
        if (it == pidToTaskId_.end()) {
            TAG_LOGW(TAG, "Unknown process exited: pid=%{public}d", pid);
            return;
        }
        taskId = it->second;
        pidToTaskId_.erase(it);
    }

    // 计算退出码
    int exitCode = -1;
    if (WIFEXITED(status)) {
        exitCode = WEXITSTATUS(status);
        TAG_LOGI(TAG, "Process %{public}d exited normally: code=%{public}d", pid, exitCode);
    } else if (WIFSIGNALED(status)) {
        int signal = WTERMSIG(status);
        TAG_LOGW(TAG, "Process %{public}d terminated by signal: %{public}d", pid, signal);
        exitCode = -1;
    }

    // 分发退出事件
    if (eventDispatcher_) {
        eventDispatcher_->DispatchExitEvent(taskId, exitCode);
    }
}

bool ProcessMonitor::WaitForProcessExit(pid_t pid, int* status)
{
    // 阻塞等待特定进程退出
    pid_t ret = waitpid(pid, status, 0);

    if (ret < 0) {
        TAG_LOGE(TAG, "waitpid failed for pid=%{public}d: %{public}d", pid, errno);
        return false;
    }

    if (ret != pid) {
        TAG_LOGW(TAG, "waitpid returned unexpected pid: %{public}d (expected %{public}d)",
                 ret, pid);
        return false;
    }

    return true;
}

} // namespace CliMgr
} // namespace OHOS
