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

#include "io_monitor.h"

#include <chrono>
#include <cstring>
#include <cerrno>
#include <fcntl.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t MAX_EVENTS = 16;
constexpr int32_t EPOLL_WAIT_MS = 100;

// 时间戳日志宏
#define LOG_TIMESTAMP(tag, desc) \
    do { \
        auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>( \
            std::chrono::system_clock::now().time_since_epoch()).count(); \
        TAG_LOGI(AAFwkTag::ABILITYMGR, "[TIMESTAMP] [%{public}s] %{public}lld - %{public}s", \
            tag, static_cast<long long>(nowMs), desc); \
    } while(0)
}

IOMonitor::~IOMonitor()
{
    Stop();
}

int32_t IOMonitor::Start()
{
    if (running_.exchange(true)) {
        return ERR_OK;
    }
    epollFd_ = epoll_create1(EPOLL_CLOEXEC);
    if (epollFd_ < 0) {
        running_.store(false, std::memory_order_release);
        TAG_LOGE(AAFwkTag::ABILITYMGR, "epoll_create1 failed: %{public}s", strerror(errno));
        return -1;
    }
    monitorThread_ = std::thread(&IOMonitor::MonitorLoop, this);
    return ERR_OK;
}

void IOMonitor::Stop()
{
    if (!running_.exchange(false)) {
        return;
    }
    if (epollFd_ >= 0) {
        close(epollFd_);
        epollFd_ = -1;
    }
    if (monitorThread_.joinable()) {
        monitorThread_.join();
    }

    std::lock_guard<std::mutex> lock(fdMutex_);
    for (const auto &[fd, info] : fdMap_) {
        close(fd);
    }
    fdMap_.clear();
}

int32_t IOMonitor::RegisterSession(const std::string &sessionId, int stdoutFd, int stderrFd)
{
    std::lock_guard<std::mutex> lock(fdMutex_);
    epoll_event event {};
    event.events = EPOLLIN | EPOLLHUP | EPOLLERR;

    if (stdoutFd >= 0) {
        int flags = fcntl(stdoutFd, F_GETFL, 0);
        if (flags >= 0) {
            fcntl(stdoutFd, F_SETFL, flags | O_NONBLOCK);
        }
        event.data.fd = stdoutFd;
        if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, stdoutFd, &event) != 0) {
            return -1;
        }
        fdMap_[stdoutFd] = FdInfo {sessionId, true};
    }

    if (stderrFd >= 0) {
        int flags = fcntl(stderrFd, F_GETFL, 0);
        if (flags >= 0) {
            fcntl(stderrFd, F_SETFL, flags | O_NONBLOCK);
        }
        event.data.fd = stderrFd;
        if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, stderrFd, &event) != 0) {
            if (stdoutFd >= 0) {
                epoll_ctl(epollFd_, EPOLL_CTL_DEL, stdoutFd, nullptr);
                fdMap_.erase(stdoutFd);
            }
            return -1;
        }
        fdMap_[stderrFd] = FdInfo {sessionId, false};
    }
    return ERR_OK;
}

void IOMonitor::UnregisterSession(const std::string &sessionId)
{
    std::vector<std::pair<int, FdInfo>> fdsToClose;
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        for (auto it = fdMap_.begin(); it != fdMap_.end();) {
            if (it->second.sessionId == sessionId) {
                fdsToClose.emplace_back(it->first, it->second);
                it = fdMap_.erase(it);
                continue;
            }
            ++it;
        }
    }

    for (const auto &[fd, info] : fdsToClose) {
        epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr);
        close(fd);
    }
}

void IOMonitor::SetOutputCallback(OutputCallback callback)
{
    outputCallback_ = std::move(callback);
}

void IOMonitor::SetSessionClosedCallback(SessionClosedCallback callback)
{
    sessionClosedCallback_ = std::move(callback);
}

void IOMonitor::SetSessionDrainedCallback(SessionDrainedCallback callback)
{
    sessionDrainedCallback_ = std::move(callback);
}

void IOMonitor::MonitorLoop()
{
    epoll_event events[MAX_EVENTS];
    while (running_.load(std::memory_order_acquire)) {
        int nfds = epoll_wait(epollFd_, events, MAX_EVENTS, EPOLL_WAIT_MS);
        if (nfds < 0) {
            if (errno == EINTR) {
                continue;
            }
            if (!running_.load(std::memory_order_acquire)) {
                break;
            }
            TAG_LOGW(AAFwkTag::ABILITYMGR, "epoll_wait failed: %{public}s", strerror(errno));
            continue;
        }

        for (int i = 0; i < nfds; ++i) {
            HandleReadableFd(events[i].data.fd);
        }
    }
}

void IOMonitor::HandleReadableFd(int fd)
{
    LOG_TIMESTAMP("IO-Monitor", "HandleReadableFd START");
    FdInfo info;
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        auto it = fdMap_.find(fd);
        if (it == fdMap_.end()) {
            return;
        }
        info = it->second;
    }

    char buffer[4096];
    while (true) {
        ssize_t bytesRead = read(fd, buffer, sizeof(buffer));
        if (bytesRead > 0) {
            LOG_TIMESTAMP("IO-Monitor", "Data received from fd, calling outputCallback");
            if (outputCallback_) {
                outputCallback_(info.sessionId, info.isStdout, std::string(buffer, bytesRead));
            }
            continue;
        }
        if (bytesRead == 0) {
            CloseFdLocked(fd, info, true);
            return;
        }
        if (errno == EINTR) {
            continue;
        }
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return;
        }
        CloseFdLocked(fd, info, true);
        return;
    }
}

void IOMonitor::CloseFdLocked(int fd, const FdInfo &info, bool notifyDrained)
{
    bool sessionDrained = false;
    {
        std::lock_guard<std::mutex> lock(fdMutex_);
        auto it = fdMap_.find(fd);
        if (it == fdMap_.end()) {
            return;
        }
        epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, nullptr);
        close(fd);
        fdMap_.erase(it);
        sessionDrained = true;
        for (const auto &[otherFd, otherInfo] : fdMap_) {
            if (otherInfo.sessionId == info.sessionId) {
                sessionDrained = false;
                break;
            }
        }
    }
    if (sessionClosedCallback_) {
        sessionClosedCallback_(info.sessionId, info.isStdout);
    }
    if (notifyDrained && sessionDrained && sessionDrainedCallback_) {
        sessionDrainedCallback_(info.sessionId);
    }
}

} // namespace AbilityRuntime
} // namespace OHOS
