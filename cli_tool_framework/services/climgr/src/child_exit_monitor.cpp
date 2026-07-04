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
#include <utility>

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

std::shared_ptr<ChildExitMonitor> ChildExitMonitor::Create()
{
    return std::make_shared<ChildExitMonitor>();
}

ChildExitMonitor::~ChildExitMonitor()
{
    Stop();
}

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

bool ChildExitMonitor::Start(ActivePidProvider activePidProvider, ReapedChildHandler reapedChildHandler)
{
    if (!activePidProvider || !reapedChildHandler) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "start child exit monitor with invalid callbacks");
        return false;
    }
    if (state_ != nullptr && state_->notifyReadFd >= 0 && state_->notifyWriteFd >= 0) {
        return InstallSigChldHandler();
    }

    // The service supplies weak-pointer callbacks, so the thread state does not retain or dereference a
    // CliToolManagerService object after the service lifecycle has moved on.
    auto state = std::make_shared<MonitorState>();
    state->activePidProvider = std::move(activePidProvider);
    state->reapedChildHandler = std::move(reapedChildHandler);

    // The SIGCHLD handler cannot lock, log, or call waitpid safely, so it only writes to this pipe. The monitor
    // thread drains the pipe and performs the real child cleanup in normal thread context.
    int32_t pipeFds[2] = {-1, -1};
    // pipe() returns two file descriptors: pipeFds[0] is read by the monitor thread, pipeFds[1] is written by the
    // signal handler to wake that thread.
    if (pipe(pipeFds) != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "create child exit notify pipe failed: %{public}s", strerror(errno));
        return false;
    }

    for (int32_t fd : pipeFds) {
        if (!SetNotifyFdFlags(fd)) {
            close(pipeFds[0]);
            close(pipeFds[1]);
            return false;
        }
    }

    state->notifyReadFd = pipeFds[0];
    state->notifyWriteFd = pipeFds[1];
    state->running.store(true, std::memory_order_release);
    state_ = state;
    // Publish the write fd only after both pipe ends are configured.
    g_childExitNotifyWriteFd = state->notifyWriteFd;
    // The thread captures only its independent state. It must not capture this or a shared_ptr<ChildExitMonitor>.
    monitorThread_ = std::thread([state]() {
        ProcessNotifications(state);
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
    auto state = state_;
    if (state == nullptr || (state->notifyReadFd < 0 && state->notifyWriteFd < 0)) {
        RestoreSigChldHandler();
        state_ = nullptr;
        return;
    }

    RestoreSigChldHandler();
    state->running.store(false, std::memory_order_release);
    if (state->notifyWriteFd >= 0) {
        // Wake poll() so the monitor thread can observe running and exit promptly.
        uint8_t notify = 0;
        // write() is used here because it is also safe in the SIGCHLD path; failures are harmless during shutdown.
        (void)write(state->notifyWriteFd, &notify, sizeof(notify));
    }
    if (monitorThread_.joinable()) {
        monitorThread_.join();
    }

    g_childExitNotifyWriteFd = -1;
    if (state->notifyReadFd >= 0) {
        // close() invalidates the kernel descriptor; reset the stored fd so Stop() is idempotent.
        close(state->notifyReadFd);
        state->notifyReadFd = -1;
    }
    if (state->notifyWriteFd >= 0) {
        close(state->notifyWriteFd);
        state->notifyWriteFd = -1;
    }
    state->activePidProvider = nullptr;
    state->reapedChildHandler = nullptr;
    state_ = nullptr;
}

void ChildExitMonitor::ProcessNotifications(const std::shared_ptr<MonitorState> &state)
{
    while (state != nullptr && state->running.load(std::memory_order_acquire)) {
        pollfd childExitFd {};
        childExitFd.fd = state->notifyReadFd;
        childExitFd.events = POLLIN;
        // poll() sleeps until the pipe has data, an error occurs, or the timeout expires. Timeout lets the thread
        // periodically observe running even if no signal arrives.
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
            while (read(state->notifyReadFd, buffer, sizeof(buffer)) > 0) {}
            ProcessExitedChildren(state, SIGCHLD);
        }
        if ((childExitFd.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "child exit notify fd closed, revents=%{public}d", childExitFd.revents);
            break;
        }
    }
    ProcessExitedChildren(state, SIGCHLD);
}

void ChildExitMonitor::ProcessExitedChildren(int32_t sig)
{
    ProcessExitedChildren(state_, sig);
}

void ChildExitMonitor::ProcessExitedChildren(const std::shared_ptr<MonitorState> &state, int32_t sig)
{
    if (state == nullptr || !state->activePidProvider) {
        return;
    }
    // Snapshot pids first through the owner callback so waitpid and session finalization never run while the
    // service's session map mutex is held.
    auto pids = state->activePidProvider();
    for (pid_t pid : pids) {
        (void)TryReapChild(state, pid, sig);
    }
}

bool ChildExitMonitor::TryReapChild(const std::shared_ptr<MonitorState> &state, pid_t pid, int32_t sig)
{
    if (state == nullptr || !state->reapedChildHandler) {
        return true;
    }
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
        // The handler belongs to CliToolManagerService and performs session-state updates in service context.
        state->reapedChildHandler(pid, status, sig);
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

void ChildExitMonitor::ReapActiveChildren(const std::vector<pid_t> &pids, int32_t sig)
{
    auto state = state_;
    std::vector<pid_t> remaining = pids;
    // OnStop kills process groups first; retry briefly because SIGCHLD/status delivery can lag behind Killpg().
    for (int32_t retry = 0; retry < CHILD_EXIT_REAP_RETRY_COUNT && !remaining.empty(); ++retry) {
        for (auto iter = remaining.begin(); iter != remaining.end();) {
            if (TryReapChild(state, *iter, sig)) {
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

void ChildExitMonitor::ReapPendingChildExit(pid_t pid, int32_t sig)
{
    if (pid <= 0) {
        return;
    }
    // A short-lived tool can exit before its pid is registered with the I/O monitor, so probe once immediately.
    (void)TryReapChild(state_, pid, sig);
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
