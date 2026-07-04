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

#include "child_exit_monitor.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cli_tool_app_state_observer.h"
#include "cli_tool_manager_service.h"
#include "hilog_tag_wrapper.h"
#include "process_manager.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t CHILD_EXIT_POLL_TIMEOUT_MS = 1000;
constexpr int32_t CHILD_EXIT_REAP_RETRY_COUNT = 50;
constexpr int32_t CHILD_EXIT_REAP_RETRY_INTERVAL_US = 10 * 1000;
volatile sig_atomic_t g_childExitNotifyWriteFd = -1;
volatile sig_atomic_t g_hasPreviousSigChldAction = 0;
struct sigaction g_previousSigChldAction = {};
} // namespace

bool ChildExitMonitor::SetNotifyFdFlags(int32_t fd)
{
    // fcntl(F_GETFL/F_SETFL) reads and updates file status flags. O_NONBLOCK keeps reads and writes on the
    // notification pipe from blocking service shutdown or the SIGCHLD handler.
    int32_t flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "set child exit notify fd nonblock failed: %{public}s", strerror(errno));
        return false;
    }
    // fcntl(F_GETFD/F_SETFD) reads and updates descriptor flags. FD_CLOEXEC makes spawned tool processes close the
    // monitor pipe automatically during exec, so they cannot keep the service pipe alive accidentally.
    flags = fcntl(fd, F_GETFD, 0);
    if (flags < 0 || fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "set child exit notify fd cloexec failed: %{public}s", strerror(errno));
        return false;
    }
    return true;
}

bool ChildExitMonitor::Start(CliToolManagerService &service)
{
    service_ = &service;
    if (notifyReadFd_ >= 0 && notifyWriteFd_ >= 0) {
        if (!InstallSigChldHandler()) {
            Stop();
            return false;
        }
        return true;
    }

    // The SIGCHLD handler cannot lock, log, or call waitpid safely, so it only writes to this pipe. The monitor
    // thread drains the pipe and performs the real child cleanup in normal thread context.
    int32_t pipeFds[2] = {-1, -1};
    // pipe() returns two file descriptors: pipeFds[0] is read by the monitor thread, pipeFds[1] is written by the
    // signal handler to wake that thread.
    if (pipe(pipeFds) != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "create child exit notify pipe failed: %{public}s", strerror(errno));
        service_ = nullptr;
        return false;
    }

    for (int32_t fd : pipeFds) {
        if (!SetNotifyFdFlags(fd)) {
            close(pipeFds[0]);
            close(pipeFds[1]);
            service_ = nullptr;
            return false;
        }
    }

    notifyReadFd_ = pipeFds[0];
    notifyWriteFd_ = pipeFds[1];
    running_.store(true, std::memory_order_release);
    // Publish the write fd only after both pipe ends are configured.
    g_childExitNotifyWriteFd = notifyWriteFd_;
    monitorThread_ = std::thread([this]() {
        ProcessNotifications();
    });

    if (!InstallSigChldHandler()) {
        Stop();
        return false;
    }
    return true;
}

bool ChildExitMonitor::InstallSigChldHandler()
{
    if (sigChldActionInstalled_) {
        return true;
    }

    struct sigaction sa = {};
    sa.sa_sigaction = ChildExitMonitor::SigChldHandler;
    // sigemptyset() starts with no blocked signals while the SIGCHLD handler runs.
    sigemptyset(&sa.sa_mask);
    // SA_SIGINFO selects sa_sigaction(sig, info, context); SA_RESTART asks libc to retry interrupted syscalls where
    // possible after the handler returns.
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    // sigaction() installs this process-wide SIGCHLD handler and returns the previous action in oldSigChldAction_.
    if (sigaction(SIGCHLD, &sa, &oldSigChldAction_) != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "install SIGCHLD handler failed: %{public}s", strerror(errno));
        return false;
    }
    g_previousSigChldAction = oldSigChldAction_;
    g_hasPreviousSigChldAction = 1;
    sigChldActionInstalled_ = true;
    return true;
}

void ChildExitMonitor::RestoreSigChldHandler()
{
    if (!sigChldActionInstalled_) {
        return;
    }

    g_hasPreviousSigChldAction = 0;
    // Passing the saved action back to sigaction() restores the handler that was active before climgr started.
    if (sigaction(SIGCHLD, &oldSigChldAction_, nullptr) != 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "restore SIGCHLD handler failed: %{public}s", strerror(errno));
    }
    sigChldActionInstalled_ = false;
}

void ChildExitMonitor::Stop()
{
    if (notifyReadFd_ < 0 && notifyWriteFd_ < 0) {
        RestoreSigChldHandler();
        service_ = nullptr;
        return;
    }

    RestoreSigChldHandler();
    running_.store(false, std::memory_order_release);
    if (notifyWriteFd_ >= 0) {
        // Wake poll() so the monitor thread can observe running_ and exit promptly.
        uint8_t notify = 0;
        // write() is used here because it is also safe in the SIGCHLD path; failures are harmless during shutdown.
        (void)write(notifyWriteFd_, &notify, sizeof(notify));
    }
    if (monitorThread_.joinable()) {
        monitorThread_.join();
    }

    g_childExitNotifyWriteFd = -1;
    if (notifyReadFd_ >= 0) {
        // close() invalidates the kernel descriptor; reset the stored fd so Stop() is idempotent.
        close(notifyReadFd_);
        notifyReadFd_ = -1;
    }
    if (notifyWriteFd_ >= 0) {
        close(notifyWriteFd_);
        notifyWriteFd_ = -1;
    }
    service_ = nullptr;
}

void ChildExitMonitor::ProcessNotifications()
{
    while (running_.load(std::memory_order_acquire)) {
        pollfd childExitFd {};
        childExitFd.fd = notifyReadFd_;
        childExitFd.events = POLLIN;
        // poll() sleeps until the pipe has data, an error occurs, or the timeout expires. Timeout lets the thread
        // periodically observe running_ even if no signal arrives.
        int32_t pollRet = poll(&childExitFd, 1, CHILD_EXIT_POLL_TIMEOUT_MS);
        if (pollRet < 0) {
            // EINTR means another signal interrupted poll(); retry because no pipe state was consumed.
            if (errno == EINTR) {
                continue;
            }
            TAG_LOGW(AAFwkTag::CLI_TOOL, "child exit poll failed: %{public}s", strerror(errno));
            continue;
        }
        if (pollRet == 0) {
            continue;
        }
        if ((childExitFd.revents & POLLIN) != 0) {
            uint8_t buffer[64];
            // SIGCHLD notifications can be coalesced; one wake means "rescan all active child pids".
            // read() drains all pending wake bytes. It stops when the nonblocking pipe is empty.
            while (read(notifyReadFd_, buffer, sizeof(buffer)) > 0) {}
            ProcessExitedChildren(SIGCHLD);
        }
        if ((childExitFd.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "child exit notify fd closed, revents=%{public}d", childExitFd.revents);
            break;
        }
    }
    ProcessExitedChildren(SIGCHLD);
}

void ChildExitMonitor::ProcessExitedChildren(int32_t sig)
{
    if (service_ == nullptr) {
        return;
    }
    ProcessExitedChildren(*service_, sig);
}

void ChildExitMonitor::ProcessExitedChildren(CliToolManagerService &service, int32_t sig)
{
    // Snapshot pids first so waitpid/WaitPid never run while sessionsMutex_ is held.
    auto pids = service.GetActiveChildPids();
    for (pid_t pid : pids) {
        (void)TryReapChild(service, pid, sig);
    }
}

bool ChildExitMonitor::TryReapChild(CliToolManagerService &service, pid_t pid, int32_t sig)
{
    if (pid <= 0) {
        return true;
    }

    int32_t status = 0;
    pid_t waitRet = 0;
    // Wait only for pids owned by active CLI sessions. waitpid(-1) could reap children owned by other code in the
    // same service process. WNOHANG makes waitpid() nonblocking: it returns 0 when the child has not exited yet.
    do {
        waitRet = waitpid(pid, &status, WNOHANG);
    } while (waitRet < 0 && errno == EINTR);

    if (waitRet == pid) {
        service.WaitPid(pid, status, sig);
        ProcessManager::GetInstance().Killpg(pid);
        return true;
    }
    if (waitRet == 0) {
        return false;
    }
    // ECHILD means the pid is not waitable anymore, for example it was already reaped through another path.
    if (errno != ECHILD) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "wait owned child failed, pid=%{public}d error=%{public}s",
            pid, strerror(errno));
    }
    return true;
}

void ChildExitMonitor::ReapActiveChildren(CliToolManagerService &service, const std::vector<pid_t> &pids,
    int32_t sig)
{
    std::vector<pid_t> remaining = pids;
    // OnStop kills process groups first; retry briefly because SIGCHLD/status delivery can lag behind Killpg().
    for (int32_t retry = 0; retry < CHILD_EXIT_REAP_RETRY_COUNT && !remaining.empty(); ++retry) {
        for (auto iter = remaining.begin(); iter != remaining.end();) {
            if (TryReapChild(service, *iter, sig)) {
                iter = remaining.erase(iter);
                continue;
            }
            ++iter;
        }
        if (!remaining.empty()) {
            // usleep() gives killed children a short window to report exit before the next nonblocking waitpid().
            usleep(CHILD_EXIT_REAP_RETRY_INTERVAL_US);
        }
    }
    for (pid_t pid : remaining) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "child not reaped before climgr stop, pid=%{public}d", pid);
    }
}

void ChildExitMonitor::ReapPendingChildExit(CliToolManagerService &service, pid_t pid, int32_t sig)
{
    if (pid <= 0) {
        return;
    }
    // A short-lived tool can exit before its pid is registered with the I/O monitor, so probe once immediately.
    (void)TryReapChild(service, pid, sig);
}

void ChildExitMonitor::SigChldHandler(int32_t sig, siginfo_t *info, void *context)
{
    // Preserve errno because signal handlers run asynchronously and must not change the interrupted code's errno.
    int32_t savedErrno = errno;
    int32_t notifyFd = static_cast<int32_t>(g_childExitNotifyWriteFd);
    if (notifyFd >= 0) {
        uint8_t notify = static_cast<uint8_t>(sig);
        // write() is async-signal-safe. A single byte is enough to wake the monitor thread.
        (void)write(notifyFd, &notify, sizeof(notify));
    }
    if (g_hasPreviousSigChldAction != 0) {
        // Chain to the handler that existed before climgr installed its handler so process-level behavior is kept.
        if ((g_previousSigChldAction.sa_flags & SA_SIGINFO) != 0 &&
            g_previousSigChldAction.sa_sigaction != nullptr) {
            g_previousSigChldAction.sa_sigaction(sig, info, context);
        } else if (g_previousSigChldAction.sa_handler != SIG_DFL &&
            g_previousSigChldAction.sa_handler != SIG_IGN &&
            g_previousSigChldAction.sa_handler != nullptr) {
            g_previousSigChldAction.sa_handler(sig);
        }
    }
    errno = savedErrno;
}
} // namespace CliTool
} // namespace OHOS
