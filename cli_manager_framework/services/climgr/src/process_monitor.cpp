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

#include "process_monitor.h"

#include <chrono>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ACTIVE_WAIT_MS = 50;
constexpr int32_t IDLE_WAIT_MS = 200;
}

ProcessMonitor::~ProcessMonitor()
{
    Stop();
}

int32_t ProcessMonitor::Start()
{
    if (running_.exchange(true)) {
        return ERR_OK;
    }
    monitorThread_ = std::thread(&ProcessMonitor::MonitorLoop, this);
    return ERR_OK;
}

void ProcessMonitor::Stop()
{
    if (!running_.exchange(false)) {
        return;
    }
    if (monitorThread_.joinable()) {
        monitorThread_.join();
    }
    std::lock_guard<std::mutex> lock(entriesMutex_);
    entries_.clear();
}

int32_t ProcessMonitor::RegisterSession(pid_t pid, const std::string &sessionId, int64_t startTimeMs, int32_t timeoutMs)
{
    std::lock_guard<std::mutex> lock(entriesMutex_);
    entries_[pid] = ProcessEntry {sessionId, startTimeMs, timeoutMs, false, false};
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ProcessMonitor: registered sessionId='%{public}s', pid=%{public}d, timeoutMs=%{public}d ms",
        sessionId.c_str(), pid, timeoutMs);
    if (timeoutMs <= 0) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "ProcessMonitor: timeoutMs=%{public}d, NO timeout monitoring will be applied!", timeoutMs);
    }
    return ERR_OK;
}

void ProcessMonitor::UnregisterSession(const std::string &sessionId)
{
    std::lock_guard<std::mutex> lock(entriesMutex_);
    for (auto it = entries_.begin(); it != entries_.end(); ++it) {
        if (it->second.sessionId == sessionId) {
            entries_.erase(it);
            return;
        }
    }
}

void ProcessMonitor::SetExitCallback(ExitCallback callback)
{
    exitCallback_ = std::move(callback);
}

void ProcessMonitor::SetTimeoutCallback(TimeoutCallback callback)
{
    timeoutCallback_ = std::move(callback);
}

void ProcessMonitor::MonitorLoop()
{
    while (running_.load(std::memory_order_acquire)) {
        std::vector<pid_t> pidsToCheck;
        {
            std::lock_guard<std::mutex> lock(entriesMutex_);
            for (const auto &[pid, entry] : entries_) {
                if (!entry.exitObserved) {
                    pidsToCheck.push_back(pid);
                }
            }
        }

        bool handledExit = false;
        for (pid_t pid : pidsToCheck) {
            int status = 0;
            pid_t waitResult = waitpid(pid, &status, WNOHANG);
            if (waitResult == pid) {
                HandleExit(pid, status);
                handledExit = true;
            } else if (waitResult == -1 && errno == ECHILD) {
                std::lock_guard<std::mutex> lock(entriesMutex_);
                auto it = entries_.find(pid);
                if (it != entries_.end()) {
                    it->second.exitObserved = true;
                }
            }
        }
        if (handledExit) {
            continue;
        }
        CheckTimeouts();

        size_t activeCount = 0;
        {
            std::lock_guard<std::mutex> lock(entriesMutex_);
            activeCount = entries_.size();
        }
        int32_t sleepMs = activeCount > 0 ? ACTIVE_WAIT_MS : IDLE_WAIT_MS;
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepMs));
    }
}

void ProcessMonitor::CheckTimeouts()
{
    const auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    std::vector<std::pair<pid_t, std::string>> expired;
    {
        std::lock_guard<std::mutex> lock(entriesMutex_);
        for (auto &[pid, entry] : entries_) {
            if (entry.timeoutMs <= 0) {
                TAG_LOGW(AAFwkTag::ABILITYMGR,
                    "ProcessMonitor: skipping timeout check for sessionId='%{public}s', pid=%{public}d, timeoutMs=%{public}d (<= 0)",
                    entry.sessionId.c_str(), pid, entry.timeoutMs);
                continue;
            }
            if (entry.timedOut) {
                TAG_LOGI(AAFwkTag::ABILITYMGR,
                    "ProcessMonitor: skipping timeout check for sessionId='%{public}s', pid=%{public}d (already timed out)",
                    entry.sessionId.c_str(), pid);
                continue;
            }
            int64_t elapsedMs = nowMs - entry.startTimeMs;
            if (elapsedMs >= entry.timeoutMs) {
                entry.timedOut = true;
                expired.emplace_back(pid, entry.sessionId);
                TAG_LOGW(AAFwkTag::ABILITYMGR,
                    "ProcessMonitor: TIMEOUT! sessionId='%{public}s', pid=%{public}d, elapsed=%{public}ld ms, timeout=%{public}d ms",
                    entry.sessionId.c_str(), pid, elapsedMs, entry.timeoutMs);
            }
        }
    }

    for (const auto &[pid, sessionId] : expired) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ProcessMonitor: killing timed out process pid=%{public}d, sessionId='%{public}s'",
            pid, sessionId.c_str());

        if (kill(pid, SIGKILL) != 0) {
            if (errno == ESRCH) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "ProcessMonitor: process %{public}d already terminated", pid);
            } else if (errno == EPERM) {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "ProcessMonitor: kill permission denied for pid=%{public}d, possibly running in sandbox (claw_sandbox)",
                    pid);
            } else {
                TAG_LOGW(AAFwkTag::ABILITYMGR, "ProcessMonitor: kill failed for pid=%{public}d, sessionId='%{public}s': %{public}s",
                    pid, sessionId.c_str(), strerror(errno));
            }
        } else {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ProcessMonitor: killed process pid=%{public}d successfully", pid);
        }

        if (timeoutCallback_) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ProcessMonitor: calling timeout callback for sessionId='%{public}s'",
                sessionId.c_str());
            timeoutCallback_(sessionId);
        }
    }
}

void ProcessMonitor::HandleExit(pid_t pid, int status)
{
    ProcessEntry entry;
    {
        std::lock_guard<std::mutex> lock(entriesMutex_);
        auto it = entries_.find(pid);
        if (it == entries_.end()) {
            return;
        }
        entry = it->second;
        entries_.erase(it);  // Remove entry when process exits
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ProcessMonitor: removed exited process pid=%{public}d, sessionId='%{public}s', timedOut=%{public}d",
            pid, entry.sessionId.c_str(), entry.timedOut);
    }
    if (exitCallback_) {
        exitCallback_(entry.sessionId, status, entry.timedOut);
    }
}

} // namespace AbilityRuntime
} // namespace OHOS
