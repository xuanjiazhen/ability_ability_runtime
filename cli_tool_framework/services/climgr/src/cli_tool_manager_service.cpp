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

#include "cli_tool_manager_service.h"

#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "ability_manager_client.h"
#include "accesstoken_kit.h"
#include "app_mgr_client.h"
#include "ccm_util.h"
#include "cli_error_code.h"
#include "cli_event_report.h"
#include "cli_tool_app_state_observer.h"
#include "cli_function_data_manager.h"
#include "event_dispatcher.h"
#include "function_info.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "permission_query_util.h"
#include "permission_util.h"
#include "process_manager.h"
#include "session_record.h"
#include "tokenid_kit.h"
#include "tool_util.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr const char* PERMISSION_EXEC_CLI_TOOL = "ohos.permission.EXEC_CLI_TOOL";
constexpr const char* PERMISSION_QUERY_CLI_TOOL = "ohos.permission.QUERY_CLI_TOOL";
constexpr const char* PERMISSION_REGISTER_CLI_TOOL = "ohos.permission.REGISTER_CLI_TOOL";
constexpr const char* PERMISSION_ACCESS_FUNCTION = "ohos.permission.ACCESS_FUNCTION";

constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t COEFFICIENT = 1000;
constexpr int32_t QUERY_SUCCESS = 0;
constexpr int32_t QUERY_COMMAND_NOT_EXIST = 1;
constexpr int32_t QUERY_DB_ERROR = 2;
constexpr int32_t MAX_QUERY_CMDS_SIZE = 100;
constexpr int32_t ACTIVE_TIME = 30 * 1000; // 30s
constexpr int32_t SKILL_TYPE_INDEPENDENT = -1;
constexpr int32_t CHILD_EXIT_POLL_TIMEOUT_MS = 1000;
constexpr int32_t CHILD_EXIT_REAP_RETRY_COUNT = 50;
constexpr int32_t CHILD_EXIT_REAP_RETRY_INTERVAL_US = 10 * 1000;
constexpr int32_t CALLER_DIED_EXIT_STATUS = SIGKILL;
volatile sig_atomic_t g_childExitNotifyWriteFd = -1;
volatile sig_atomic_t g_hasPreviousSigChldAction = 0;
struct sigaction g_previousSigChldAction = {};
} // namespace

std::mutex g_mutex;
sptr<CliToolManagerService> CliToolManagerService::instance_ = nullptr;
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(CliToolManagerService::GetInstance().GetRefPtr());

sptr<CliToolManagerService> CliToolManagerService::GetInstance()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (instance_ != nullptr) {
        return instance_;
    }
    instance_ = new (std::nothrow) CliToolManagerService();
    return instance_;
}

void CliToolManagerService::HandleProcessTimeout(const std::string &sessionId)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleProcessTimeout skipped: sessionId=%{public}s not found", sessionId.c_str());
        return;
    }

    if (record->sessionType == SessionType::SKILL) {
        HandleSkillSessionTimeout(sessionId);
        return;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "HandleProcessTimeout: sessionId=%{public}s", sessionId.c_str());
    record->SetTimeout(true);
    record->SetState(SessionState::CANCELLING);

    // Set end time for timeout reporting
    record->SetTerminalResult(0, 0);

    // Report timeout event
    int64_t durationMs = record->GetEndTimeMs() - record->startTime;
    std::string bundleName;
    auto tokenId = static_cast<AccessToken::AccessTokenID>(IPCSkeleton::GetCallingTokenID());
    AppExecFwk::BundleInfo bundleInfo;
    if (ToolUtil::GetBundleInfoByTokenId(tokenId, bundleInfo)) {
        bundleName = bundleInfo.name;
    }
    ReportCliTimeout(bundleName, record->toolName, std::to_string(durationMs));

    auto oldBackground = record->SetBackground(true);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "HandleProcessTimeout: sessionId=%{public}s, background=%{public}d",
        sessionId.c_str(), oldBackground);
    if (!oldBackground) {
        CliSessionInfo session;
        record->BuildSessionInfo(session);
        EventDispatcher::GetInstance().DispatchExecToolReplyEvent(
            record->callerPid, record->callerUid, record->eventId, ERR_OK, session);
    }

    EventDispatcher::GetInstance().DispatchErrorEvent(sessionId, "session timed out");
    ProcessManager::GetInstance().Killpg(record->processId);
}

void CliToolManagerService::HandleProcessYieldTimeout(const std::string &sessionId)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleProcessYieldTimeout skipped: sessionId=%{public}s not found", sessionId.c_str());
        return;
    }
    auto oldBackground = record->SetBackground(true);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "HandleProcessYieldTimeout: sessionId=%{public}s, background=%{public}d",
        sessionId.c_str(), oldBackground);
    if (!oldBackground) {
        CliSessionInfo session;
        record->BuildSessionInfo(session);
        EventDispatcher::GetInstance().DispatchExecToolReplyEvent(
            record->callerPid, record->callerUid, record->eventId, ERR_OK, session);
    }
}

void CliToolManagerService::HandleOutputClosed(const std::string &sessionId, bool isStdout)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleOutputClosed skipped: sessionId=%{public}s not found, isStdout=%{public}d",
            sessionId.c_str(), isStdout);
        return;
    }
    if (isStdout) {
        record->MarkStdoutClosed();
    } else {
        record->MarkStderrClosed();
    }
    if (record->HasProcessExited() && record->OutputDrained()) {
        FinalizeBackgroundSession(record);
    }
}

void CliToolManagerService::HandleOutputDrained(const std::string &sessionId)
{
    auto record = GetSessionRecord(sessionId);
    if (record != nullptr && record->HasProcessExited()) {
        FinalizeBackgroundSession(record);
    }
}

void CliToolManagerService::FinalizeBackgroundSession(const std::shared_ptr<SessionRecord> &record)
{
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "FinalizeBackgroundSession skipped: record is null");
        return;
    }
    const auto &sessionId = record->sessionId;
    if (!record->BeginCleanup()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "FinalizeBackgroundSession skipped: cleanup already started for sessionId=%{public}s",
            sessionId.c_str());
        return;
    }

    auto oldBackground = record->SetBackground(true);
    if (!oldBackground) {
        CliSessionInfo session;
        record->BuildSessionInfo(session);
        EventDispatcher::GetInstance().DispatchExecToolReplyEvent(
            record->callerPid, record->callerUid, record->eventId, ERR_OK, session);
    }
    if (ioMonitor_) {
        ioMonitor_->UnregisterSession(sessionId);
    }

    const int32_t status = record->GetTerminalStatus();
    EventDispatcher::GetInstance().DispatchExitEvent(sessionId, status);
    EventDispatcher::GetInstance().ClearSessionSubscribers(sessionId);
    RemoveSessionRecord(sessionId);
}

void CliToolManagerService::CleanupSessionAfterCallerDied(const std::shared_ptr<SessionRecord> &record)
{
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "CleanupSessionAfterCallerDied skipped: record is null");
        return;
    }

    const auto sessionId = record->sessionId;
    if (!record->BeginCleanup()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "CleanupSessionAfterCallerDied skipped: cleanup already started for sessionId=%{public}s",
            sessionId.c_str());
        return;
    }

    record->SetState(SessionState::CANCELLING);
    record->SetTerminalResult(CALLER_DIED_EXIT_STATUS, SIGCHLD);
    if (record->processId > 0) {
        ProcessManager::GetInstance().Killpg(record->processId);
    }
    UnregisterSessionWithMonitors(sessionId);
    EventDispatcher::GetInstance().DispatchExitEvent(sessionId, record->GetTerminalStatus());
    EventDispatcher::GetInstance().ClearSessionSubscribers(sessionId);
}

bool CliToolManagerService::SetChildExitNotifyFdFlags(int32_t fd)
{
    int32_t flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0 || fcntl(fd, F_SETFL, flags | O_NONBLOCK) < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "set child exit notify fd nonblock failed: %{public}s", strerror(errno));
        return false;
    }
    flags = fcntl(fd, F_GETFD, 0);
    if (flags < 0 || fcntl(fd, F_SETFD, flags | FD_CLOEXEC) < 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "set child exit notify fd cloexec failed: %{public}s", strerror(errno));
        return false;
    }
    return true;
}

bool CliToolManagerService::StartChildExitMonitor()
{
    if (childExitNotifyReadFd_ >= 0 && childExitNotifyWriteFd_ >= 0) {
        return true;
    }

    int32_t pipeFds[2] = {-1, -1};
    if (pipe(pipeFds) != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "create child exit notify pipe failed: %{public}s", strerror(errno));
        return false;
    }

    for (int32_t fd : pipeFds) {
        if (!SetChildExitNotifyFdFlags(fd)) {
            close(pipeFds[0]);
            close(pipeFds[1]);
            return false;
        }
    }

    childExitNotifyReadFd_ = pipeFds[0];
    childExitNotifyWriteFd_ = pipeFds[1];
    childExitMonitorRunning_.store(true, std::memory_order_release);
    g_childExitNotifyWriteFd = childExitNotifyWriteFd_;
    childExitMonitorThread_ = std::thread([this]() {
        ProcessChildExitNotifications();
    });
    return true;
}

bool CliToolManagerService::InstallSigChldHandler()
{
    if (sigChldActionInstalled_) {
        return true;
    }

    struct sigaction sa = {};
    sa.sa_sigaction = CliToolManagerService::sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    if (sigaction(SIGCHLD, &sa, &oldSigChldAction_) != 0) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "install SIGCHLD handler failed: %{public}s", strerror(errno));
        return false;
    }
    g_previousSigChldAction = oldSigChldAction_;
    g_hasPreviousSigChldAction = 1;
    sigChldActionInstalled_ = true;
    return true;
}

void CliToolManagerService::RestoreSigChldHandler()
{
    if (!sigChldActionInstalled_) {
        return;
    }

    g_hasPreviousSigChldAction = 0;
    if (sigaction(SIGCHLD, &oldSigChldAction_, nullptr) != 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "restore SIGCHLD handler failed: %{public}s", strerror(errno));
    }
    sigChldActionInstalled_ = false;
}

void CliToolManagerService::StopChildExitMonitor()
{
    if (childExitNotifyReadFd_ < 0 && childExitNotifyWriteFd_ < 0) {
        RestoreSigChldHandler();
        return;
    }

    RestoreSigChldHandler();
    childExitMonitorRunning_.store(false, std::memory_order_release);
    if (childExitNotifyWriteFd_ >= 0) {
        uint8_t notify = 0;
        (void)write(childExitNotifyWriteFd_, &notify, sizeof(notify));
    }
    if (childExitMonitorThread_.joinable()) {
        childExitMonitorThread_.join();
    }

    g_childExitNotifyWriteFd = -1;
    if (childExitNotifyReadFd_ >= 0) {
        close(childExitNotifyReadFd_);
        childExitNotifyReadFd_ = -1;
    }
    if (childExitNotifyWriteFd_ >= 0) {
        close(childExitNotifyWriteFd_);
        childExitNotifyWriteFd_ = -1;
    }
}

void CliToolManagerService::ProcessChildExitNotifications()
{
    while (childExitMonitorRunning_.load(std::memory_order_acquire)) {
        pollfd childExitFd {};
        childExitFd.fd = childExitNotifyReadFd_;
        childExitFd.events = POLLIN;
        int32_t pollRet = poll(&childExitFd, 1, CHILD_EXIT_POLL_TIMEOUT_MS);
        if (pollRet < 0) {
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
            while (read(childExitNotifyReadFd_, buffer, sizeof(buffer)) > 0) {}
            ProcessExitedChildren(SIGCHLD);
        }
        if ((childExitFd.revents & (POLLERR | POLLHUP | POLLNVAL)) != 0) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "child exit notify fd closed, revents=%{public}d", childExitFd.revents);
            break;
        }
    }
    ProcessExitedChildren(SIGCHLD);
}

void CliToolManagerService::ProcessExitedChildren(int32_t sig)
{
    auto pids = GetActiveChildPids();
    for (pid_t pid : pids) {
        (void)TryReapChild(pid, sig);
    }
}

std::vector<pid_t> CliToolManagerService::GetActiveChildPids()
{
    std::vector<pid_t> activePids;
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    activePids.reserve(sessionRecords_.size());
    for (const auto &[sessionId, record] : sessionRecords_) {
        if (record != nullptr && record->processId > 0) {
            activePids.emplace_back(record->processId);
        }
    }
    return activePids;
}

bool CliToolManagerService::TryReapChild(pid_t pid, int32_t sig)
{
    if (pid <= 0) {
        return true;
    }

    int32_t status = 0;
    pid_t waitRet = 0;
    do {
        waitRet = waitpid(pid, &status, WNOHANG);
    } while (waitRet < 0 && errno == EINTR);

    if (waitRet == pid) {
        WaitPid(pid, status, sig);
        ProcessManager::GetInstance().Killpg(pid);
        return true;
    }
    if (waitRet == 0) {
        return false;
    }
    if (errno != ECHILD) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "wait owned child failed, pid=%{public}d error=%{public}s",
            pid, strerror(errno));
    }
    return true;
}

void CliToolManagerService::ReapActiveChildren(const std::vector<pid_t> &pids, int32_t sig)
{
    std::vector<pid_t> remaining = pids;
    for (int32_t retry = 0; retry < CHILD_EXIT_REAP_RETRY_COUNT && !remaining.empty(); ++retry) {
        for (auto iter = remaining.begin(); iter != remaining.end();) {
            if (TryReapChild(*iter, sig)) {
                iter = remaining.erase(iter);
                continue;
            }
            ++iter;
        }
        if (!remaining.empty()) {
            usleep(CHILD_EXIT_REAP_RETRY_INTERVAL_US);
        }
    }
    for (pid_t pid : remaining) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "child not reaped before climgr stop, pid=%{public}d", pid);
    }
}

bool CliToolManagerService::Init()
{
    if (initialized_.load(std::memory_order_acquire)) {
        return true;
    }

    if (!StartChildExitMonitor()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "start child exit monitor failed");
        return false;
    }

    if (!InstallSigChldHandler()) {
        StopChildExitMonitor();
        return false;
    }

    ioMonitor_ = IOMonitor::Create();
    if (ioMonitor_ == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "create io monitor failed");
        StopChildExitMonitor();
        return false;
    }
    wptr<CliToolManagerService> weakSelf = this;
    ioMonitor_->SetOutputCallback([weakSelf](const std::string &sessionId, bool isStdout,
        const std::string &data) {
        auto service = weakSelf.promote();
        if (service == nullptr || !service->initialized_.load(std::memory_order_acquire)) {
            return;
        }
        auto record = service->GetSessionRecord(sessionId);
        if (record != nullptr) {
            record->AppendOutput(isStdout, data);
        }
        EventDispatcher::GetInstance().DispatchIOEvent(sessionId, isStdout ? "stdout" : "stderr", data);
    });
    ioMonitor_->SetInputReplyCallback([weakSelf](const std::string &sessionId,
        const std::string &eventId, bool result) {
        auto service = weakSelf.promote();
        if (service == nullptr || !service->initialized_.load(std::memory_order_acquire)) {
            return;
        }
        auto record = service->GetSessionRecord(sessionId);
        if (record == nullptr) {
            TAG_LOGI(AAFwkTag::CLI_TOOL,
                "Failed ioMonitor Input: session not found %{public}s: %{public}d", eventId.c_str(), result);
            return;
        }

        EventDispatcher::GetInstance().DispatchInputReplyEvent(record->callerPid, record->callerUid, eventId,
            result ? ERR_OK : ERR_CLI_SEND_MESSAGE);
    });
    ioMonitor_->SetSessionClosedCallback([weakSelf](const std::string &sessionId, bool isStdout) {
        auto service = weakSelf.promote();
        if (service != nullptr && service->initialized_.load(std::memory_order_acquire)) {
            service->HandleOutputClosed(sessionId, isStdout);
        }
    });
    ioMonitor_->SetSessionDrainedCallback([weakSelf](const std::string &sessionId) {
        auto service = weakSelf.promote();
        if (service != nullptr && service->initialized_.load(std::memory_order_acquire)) {
            service->HandleOutputDrained(sessionId);
        }
    });
    if (!ioMonitor_->Start()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "start io monitor failed");
        ioMonitor_ = nullptr;
        StopChildExitMonitor();
        return false;
    }
    initialized_.store(true, std::memory_order_release);
    return true;
}

void CliToolManagerService::OnStart()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "climgr start");

    if (!Init()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "climgr init failed");
        return;
    }

    // Publish the service
    auto cliService = CliToolManagerService::GetInstance();
    if (cliService == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "cli service is nullptr");
        return;
    }

    int32_t ret = CliToolDataManager::GetInstance().EnsureToolsLoaded();
    if (ret != 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to load tools: %{public}d", ret);
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully loaded tools");
    }

    ret = CliFunctionDataManager::GetInstance().EnsureFunctionsInitialized();
    if (ret != 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to initialize functions database: %{public}d", ret);
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully initialized functions database");
    }

    if (!Publish(cliService)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Publish failed");
        return;
    }
    DelayUnloadTask();
    TAG_LOGI(AAFwkTag::CLI_TOOL, "climgr start success");
}

void CliToolManagerService::OnStop()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "climgr stop");
    initialized_.store(false, std::memory_order_release);

    // Collect active PIDs before clearing sessions.
    std::vector<pid_t> activePids = GetActiveChildPids();

    // Clear event dispatcher
    EventDispatcher::GetInstance().ClearAll();

    // Kill all active processes
    auto &processManager = ProcessManager::GetInstance();
    for (pid_t pid : activePids) {
        processManager.Killpg(pid);
    }
    ReapActiveChildren(activePids, SIGCHLD);

    if (ioMonitor_ != nullptr) {
        ioMonitor_->Stop();
        ioMonitor_ = nullptr;
    }
    StopChildExitMonitor();
    {
        std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
        sessionRecords_.clear();
    }
}

int32_t CliToolManagerService::OnIdle(const SystemAbilityOnDemandReason &idlReason)
{
    int32_t sessionSize = 0;
    {
        std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
        sessionSize = static_cast<int32_t>(sessionRecords_.size());
    }
    int32_t calledCount = interfaceCalledCount_.load();
    int32_t callerPidSize = GetCallerPidCount();
    if (calledCount != 0 || sessionSize != 0 || callerPidSize != 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "service busy, calledCount=%{public}d, sessionSize=%{public}d",
            calledCount, sessionSize);
        if (!CancelIdle()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "Fail to cancel idle");
        }
        return -1;
    }
    return 0;
}

void CliToolManagerService::AddSessionRecord(const std::shared_ptr<SessionRecord> &record)
{
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    sessionRecords_[record->sessionId] = record;
}

std::shared_ptr<SessionRecord> CliToolManagerService::GetSessionRecord(const std::string &sessionId)
{
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    auto it = sessionRecords_.find(sessionId);
    if (it == sessionRecords_.end()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "GetSessionRecord failed: sessionId=%{public}s not found", sessionId.c_str());
        return nullptr;
    }
    if (it->second == nullptr) {
        sessionRecords_.erase(it); // for leak
        return nullptr;
    }
    return it->second;
}

std::vector<std::shared_ptr<SessionRecord>> CliToolManagerService::GetSessionRecords()
{
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    std::vector<std::shared_ptr<SessionRecord>> records;
    records.reserve(sessionRecords_.size());
    for (const auto &[sessionId, record] : sessionRecords_) {
        if (record != nullptr) {
            records.emplace_back(record);
        }
    }
    return records;
}

void CliToolManagerService::RemoveSessionRecord(const std::string &sessionId)
{
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    sessionRecords_.erase(sessionId);
}

bool CliToolManagerService::IsSessionOwner(const std::shared_ptr<SessionRecord> &record, const char *action) const
{
    if (record == nullptr) {
        return false;
    }
    auto callingPid = IPCSkeleton::GetCallingPid();
    if (record->callerPid != callingPid) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "%{public}s failed: caller pid=%{public}d is not session owner pid=%{public}d, sessionId=%{public}s",
            action, callingPid, record->callerPid, record->sessionId.c_str());
        return false;
    }
    return true;
}

void CliToolManagerService::DelayUnloadTask()
{
    wptr<CliToolManagerService> weakSelf = this;
    auto task = [weakSelf]() {
        auto service = weakSelf.promote();
        if (service == nullptr || !service->initialized_.load(std::memory_order_acquire)) {
            return;
        }
        int32_t sessionSize = 0;
        {
            std::lock_guard<ffrt::mutex> guard(service->sessionsMutex_);
            sessionSize = static_cast<int32_t>(service->sessionRecords_.size());
        }
        int32_t calledCount = service->interfaceCalledCount_.load();
        int32_t callerPidSize = service->GetCallerPidCount();
        if (calledCount == 0 && sessionSize == 0 && callerPidSize == 0) {
            TAG_LOGI(AAFwkTag::CLI_TOOL, "UnloadSA start");
            sptr<ISystemAbilityManager> saManager =
                OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
            if (saManager == nullptr) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "null saManager");
                return;
            }
            int32_t result = saManager->UnloadSystemAbility(CLI_TOOL_MGR_SERVICE_ID);
            if (result != ERR_OK) {
                TAG_LOGE(AAFwkTag::CLI_TOOL, "UnloadSystemAbility ret: %{public}d", result);
                return;
            }
            TAG_LOGI(AAFwkTag::CLI_TOOL, "UnloadSA success");
        } else {
            TAG_LOGI(AAFwkTag::CLI_TOOL, "Service still busy (calledCount=%{public}d, sessionSize=%{public}d), "
                "reschedule delay unload task", calledCount, sessionSize);
            service->DelayUnloadTask();
        }
    };
    ffrt::submit(std::move(task), ffrt::task_attr().delay(ACTIVE_TIME * COEFFICIENT));
}

bool CliToolManagerService::RegisterSessionWithMonitors(const std::shared_ptr<SessionRecord> &record,
    const ExecOptions &options)
{
    if (ioMonitor_ == nullptr || !ioMonitor_->RegisterSession(
        record->sessionId, record->stdoutPipe[0], record->stderrPipe[0], record->stdinPipe[1])) {
        close(record->stdoutPipe[0]);
        close(record->stderrPipe[0]);
        close(record->stdinPipe[1]);
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ioMonitor registration failed: sessionId=%{public}s, pid=%{public}d",
            record->sessionId.c_str(), record->processId);
        return false;
    }

    if (!options.background && options.yieldMs != 0) {
        PostExecToolTask(options.yieldMs, record->sessionId, false);
    }
    if (record->timeoutMs != 0) {
        PostExecToolTask(record->timeoutMs, record->sessionId, true);
    }
    return true;
}

void CliToolManagerService::UnregisterSessionWithMonitors(const std::string &sessionId)
{
    if (ioMonitor_) {
        ioMonitor_->UnregisterSession(sessionId);
    }
}

int32_t CliToolManagerService::GetAllToolInfos(ToolsRawData &tools)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllToolInfos called");
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolInfos: Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_QUERY_CLI_TOOL)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolInfos: Permission denied");
        return ERR_PERMISSION_DENIED;
    }

    return CliToolDataManager::GetInstance().GetAllToolsRawData(tools);
}

int32_t CliToolManagerService::GetAllToolSummaries(std::vector<ToolSummary> &summaries)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllToolSummaries called");
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolSummaries: Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_QUERY_CLI_TOOL)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllToolSummaries: Permission denied");
        return ERR_PERMISSION_DENIED;
    }

    return CliToolDataManager::GetInstance().QueryToolSummaries(summaries);
}

int32_t CliToolManagerService::GetToolInfoByName(const std::string &name, ToolInfo &tool)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetToolInfoByName called, name='%{public}s'", name.c_str());
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetToolInfoByName: Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_QUERY_CLI_TOOL)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetToolInfoByName: Permission denied");
        return ERR_PERMISSION_DENIED;
    }

    return CliToolDataManager::GetInstance().GetToolByName(name, tool);
}

int32_t CliToolManagerService::RegisterFunction(const FunctionInfo &function)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "RegisterFunction called: %{public}s/%{public}s",
        function.functionNamespace.c_str(), function.functionName.c_str());

    InterfaceCallCounter counter(interfaceCalledCount_);

    auto callingUid = IPCSkeleton::GetCallingUid();
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    if (callingUid != FOUNDATION_UID ||
        Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken) !=
        Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "RegisterFunction: Permission denied, uid=%{public}d", callingUid);
        return ERR_PERMISSION_DENIED;
    }

    if (!FunctionInfo::Validate(function)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "RegisterFunction: FunctionInfo validation failed");
        return ERR_INVALID_PARAM;
    }

    int32_t ret = CliFunctionDataManager::GetInstance().RegisterFunction(function);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "RegisterFunction: Failed to register function, ret=%{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully registered function: %{public}s/%{public}s",
        function.functionNamespace.c_str(), function.functionName.c_str());
    return ERR_OK;
}

int32_t CliToolManagerService::BatchRegisterFunctions(const FunctionsRawData &functions,
    int32_t &successCount)
{
    successCount = 0;
    TAG_LOGD(AAFwkTag::CLI_TOOL, "BatchRegisterFunctions called: %{public}u bytes",
        functions.size);

    InterfaceCallCounter counter(interfaceCalledCount_);

    auto callingUid = IPCSkeleton::GetCallingUid();
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    if (callingUid != FOUNDATION_UID ||
        Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken) !=
        Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "BatchRegisterFunctions: Permission denied, uid=%{public}d, tokenType=%{public}d",
            callingUid, static_cast<int32_t>(
                Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken)));
        return ERR_PERMISSION_DENIED;
    }

    std::vector<FunctionInfo> functionList;
    FunctionsRawData::ToFunctionInfoVec(functions, functionList);
    if (functionList.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "BatchRegisterFunctions: functions vector is empty");
        return ERR_INVALID_PARAM;
    }

    std::vector<FunctionInfo> validFunctions;
    validFunctions.reserve(functionList.size());
    for (const auto &function : functionList) {
        if (!FunctionInfo::Validate(function)) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "Invalid function info, will skip: %{public}s/%{public}s",
                function.functionNamespace.c_str(), function.functionName.c_str());
        } else {
            validFunctions.push_back(function);
        }
    }

    if (validFunctions.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "No valid functions to register");
        successCount = 0;
        return ERR_INVALID_PARAM;
    }

    int32_t ret = CliFunctionDataManager::GetInstance().BatchRegisterFunctions(validFunctions, successCount);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "BatchRegisterFunctions: Failed, ret=%{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully batch registered functions: success=%{public}d/%{public}zu",
        successCount, validFunctions.size());
    return ERR_OK;
}

int32_t CliToolManagerService::GetFunctionInfo(const std::string &functionNamespace,
    const std::string &functionName, FunctionInfo &function)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "GetFunctionInfo called: %{public}s/%{public}s",
        functionNamespace.c_str(), functionName.c_str());
    InterfaceCallCounter counter(interfaceCalledCount_);

    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    bool isSystemApp = AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
    bool isSA = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken) ==
        Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    if (!isSystemApp && !isSA) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetFunctionInfo: Not system app nor SA");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_ACCESS_FUNCTION)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetFunctionInfo: Permission denied");
        return ERR_PERMISSION_DENIED;
    }

    int32_t ret = CliFunctionDataManager::GetInstance().GetFunctionByName(functionNamespace, functionName, function);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetFunctionInfo: Failed to get function, ret=%{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully got function: %{public}s/%{public}s",
        functionNamespace.c_str(), functionName.c_str());
    return ERR_OK;
}

int32_t CliToolManagerService::UnregisterFunction(const std::string &functionNamespace,
    const std::string &functionName)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "UnregisterFunction called: %{public}s/%{public}s",
        functionNamespace.c_str(), functionName.c_str());
    InterfaceCallCounter counter(interfaceCalledCount_);

    auto callingUid = IPCSkeleton::GetCallingUid();
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    if (callingUid != FOUNDATION_UID ||
        Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken) !=
        Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "UnregisterFunction: Permission denied, uid=%{public}d", callingUid);
        return ERR_PERMISSION_DENIED;
    }

    int32_t ret = CliFunctionDataManager::GetInstance().UnregisterFunction(functionNamespace, functionName);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "UnregisterFunction: Failed to unregister function, ret=%{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully unregistered function: %{public}s/%{public}s",
        functionNamespace.c_str(), functionName.c_str());
    return ERR_OK;
}

int32_t CliToolManagerService::UnregisterIntentFunctionsByNamespace(const std::string &functionNamespace)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "UnregisterIntentFunctionsByNamespace called: %{public}s", functionNamespace.c_str());
    InterfaceCallCounter counter(interfaceCalledCount_);

    auto callingUid = IPCSkeleton::GetCallingUid();
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    if (callingUid != FOUNDATION_UID ||
        Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken) !=
        Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "UnregisterIntentFunctionsByNamespace: Permission denied, uid=%{public}d",
            callingUid);
        return ERR_PERMISSION_DENIED;
    }

    int32_t ret = CliFunctionDataManager::GetInstance().UnregisterIntentFunctionsByNamespace(functionNamespace);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "UnregisterIntentFunctionsByNamespace: Failed, ret=%{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully unregistered intentFunctions from namespace: %{public}s",
        functionNamespace.c_str());
    return ERR_OK;
}

int32_t CliToolManagerService::GetAllFunctions(FunctionsRawData &functions)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "GetAllFunctions called");
    InterfaceCallCounter counter(interfaceCalledCount_);

    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    bool isSystemApp = AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId);
    bool isSA = Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken) ==
        Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE;
    if (!isSystemApp && !isSA) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllFunctions: Not system app nor SA");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_ACCESS_FUNCTION)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllFunctions: Permission denied");
        return ERR_PERMISSION_DENIED;
    }

    std::vector<FunctionInfo> functionList;
    int32_t ret = CliFunctionDataManager::GetInstance().GetAllFunctions(functionList);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetAllFunctions: Failed to get functions, ret=%{public}d", ret);
        return ret;
    }

    FunctionsRawData::FromFunctionInfoVec(functionList, functions);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully got all functions (raw): %{public}zu", functionList.size());
    return ERR_OK;
}

int32_t CliToolManagerService::ValidateExecToolPermissions()
{
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_EXEC_CLI_TOOL)) {
        return ERR_PERMISSION_DENIED;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::ValidateSessionLimit()
{
    auto cliQuantity = CcmUtil::GetInstance().GetCliConcurrencyLimit();
    std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
    if (static_cast<int32_t>(sessionRecords_.size()) >= cliQuantity) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Session limit exceeded: %{public}d", cliQuantity);
        return ERR_SESSION_LIMIT_EXCEEDED;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::ValidateAndPrepareTool(const ExecToolParam &param, uint32_t tokenId,
    ToolInfo &toolInfo, std::string &sandboxConfig, std::string &bundleName, std::string& detail)
{
    if (CliToolDataManager::GetInstance().GetToolByName(param.toolName, toolInfo) != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Tool not found");
        return ERR_TOOL_NOT_EXIST;
    }

    auto checkPramRet = ToolUtil::ValidateProperties(toolInfo, param, tokenId, detail);
    if (checkPramRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Input schema validation failed");
        return checkPramRet;
    }

    if (!ToolUtil::GenerateSandboxConfig(param, tokenId, sandboxConfig, bundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "caller is not hap");
        return ERR_NOT_HAP;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::ValidateAndPrepareCmd(const ExecCmdParam &param, uint32_t tokenId,
    std::string &sandboxConfig, std::string &bundleName)
{
    std::string detail;
    auto checkPramRet = ToolUtil::ValidateExecOptionsProperties(param.options, detail);
    if (checkPramRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Input execoptions validation failed");
        return checkPramRet;
    }

    if (!ToolUtil::GenerateCmdSandboxConfig(param, tokenId, sandboxConfig, bundleName)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "caller is not hap");
        return ERR_NOT_HAP;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::SetupAndStartSession(const ExecToolParam &param, const std::string &eventId,
    const ToolInfo &toolInfo, const std::string &sandboxConfig, const std::string &bundleName)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "Dispatch to CLI path, toolName=%{public}s eventId=%{public}s", param.toolName.c_str(), eventId.c_str());

    std::shared_ptr<SessionRecord> record = CreateSessionRecord(param, eventId);
    if (record == nullptr) {
        return ERR_NO_INIT;
    }

    auto fatherSessionRecords = GetSessionRecords();
    auto createRet = ProcessManager::GetInstance().CreateChildProcess(
        param, sandboxConfig, toolInfo, record, fatherSessionRecords);
    if (createRet != ERR_OK) {
        return createRet;
    }
    AddSessionRecord(record);

    if (!RegisterSessionWithMonitors(record, param.options)) {
        ProcessManager::GetInstance().Killpg(record->processId);
        RemoveSessionRecord(record->sessionId);
        return ERR_NO_INIT;
    }

    if (!bundleName.empty()) {
        RegisterAppStateObserver(bundleName, record->callerPid);
    }

    if (param.options.background) {
        HandleBackgroundSessionReply(record, eventId);
    }

    return ERR_OK;
}

void CliToolManagerService::HandleBackgroundSessionReply(
    const std::shared_ptr<SessionRecord> &record, const std::string &eventId)
{
    CliSessionInfo session;
    record->BuildSessionInfo(session);
    EventDispatcher::GetInstance().DispatchExecToolReplyEvent(
        record->callerPid, record->callerUid, eventId, ERR_OK, session);
}

int32_t CliToolManagerService::TryDispatchSkillSession(const ExecToolParam &param,
    const std::string &eventId, const ToolInfo &toolInfo, bool &dispatched)
{
    dispatched = false;
    if (!ToolUtil::IsSkillTool(param.toolName)) {
        return ERR_OK;
    }

    ExecToolParam skillParam = param;
    int32_t skillType = 0;
    auto skillRet = ValidateSkillTypeFromParam(skillParam, skillType);
    if (skillRet != ERR_OK) {
        return skillRet;
    }
    if (skillType == SKILL_TYPE_INDEPENDENT) {
        TAG_LOGI(AAFwkTag::CLI_TOOL,
            "Independent skill, fallback to CLI path, toolName=%{public}s", param.toolName.c_str());
        return ERR_OK;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "Dispatch to skill path, toolName=%{public}s eventId=%{public}s",
        param.toolName.c_str(), eventId.c_str());
    dispatched = true;
    int32_t ret = SetupAndStartSkillSession(skillParam, eventId, toolInfo);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "Skill dispatch failed, toolName=%{public}s ret=%{public}d", param.toolName.c_str(), ret);
    }
    return ret;
}

int32_t CliToolManagerService::ExecTool(const ExecToolParam &param, const std::string &eventId,
    const sptr<ICliToolManagerScheduler> &scheduler)
{
    InterfaceCallCounter counter(interfaceCalledCount_);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecTool called: toolName=%{public}s, subcommand=%{public}s",
        param.toolName.c_str(), param.subcommand.c_str());

    std::string bundleName;
    auto tokenId = IPCSkeleton::GetCallingTokenID();

    // Get bundle name for event reporting
    AppExecFwk::BundleInfo bundleInfo;
    if (ToolUtil::GetBundleInfoByTokenId(tokenId, bundleInfo)) {
        bundleName = bundleInfo.name;
    }

    if (auto ret = ValidateExecToolPermissions(); ret != ERR_OK) {
        ReportCliExecuteFailed(bundleName, param.toolName, GetFailureReason(ret));
        return ret;
    }
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (!EventDispatcher::GetInstance().SetScheduler(callerPid, callerUid, scheduler)) {
        ReportCliExecuteFailed(bundleName, param.toolName, GetFailureReason(ERR_NO_INIT));
        return ERR_NO_INIT;
    }

    ToolInfo toolInfo;
    bool dispatched = false;
    auto skillRet = TryDispatchSkillSession(param, eventId, toolInfo, dispatched);
    if (skillRet != ERR_OK) {
        ReportCliExecuteFailed(bundleName, param.toolName, GetFailureReason(skillRet));
        return skillRet;
    }
    if (dispatched) {
        return ERR_OK;
    }

    if (auto ret = ValidateSessionLimit(); ret != ERR_OK) {
        ReportCliExecuteFailed(bundleName, param.toolName, GetFailureReason(ret));
        return ret;
    }

    std::string sandboxConfig;
    std::string detail;
    if (auto ret = ValidateAndPrepareTool(param, tokenId, toolInfo, sandboxConfig, bundleName, detail); ret != ERR_OK) {
        ReportCliExecuteFailed(bundleName, param.toolName, GetFailureReason(ret), detail);
        return ret;
    }

    auto ret = SetupAndStartSession(param, eventId, toolInfo, sandboxConfig, bundleName);
    if (ret != ERR_OK) {
        ReportCliExecuteFailed(bundleName, param.toolName, GetFailureReason(ret));
    }
    return ret;
}

int32_t CliToolManagerService::ExecCmd(const ExecCmdParam &param, const std::string &eventId,
    const sptr<ICliToolManagerScheduler> &scheduler, const std::string &subscriptionId)
{
    InterfaceCallCounter counter(interfaceCalledCount_);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecCmd called: cmd=%{public}s", param.cmd.c_str());
    if (auto ret = ValidateExecToolPermissions(); ret != ERR_OK) {
        return ret;
    }
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (!EventDispatcher::GetInstance().SetScheduler(callerPid, callerUid, scheduler)) {
        return ERR_NO_INIT;
    }
    if (auto ret = ValidateSessionLimit(); ret != ERR_OK) {
        return ret;
    }
    auto tokenId = IPCSkeleton::GetCallingTokenID();
    std::string sandboxConfig;
    std::string bundleName;
    if (auto ret = ValidateAndPrepareCmd(param, tokenId, sandboxConfig, bundleName); ret != ERR_OK) {
        return ret;
    }
    // Create session record for shell command
    auto record = std::make_shared<SessionRecord>();
    record->callerPid = callerPid;
    record->callerUid = callerUid;
    record->sessionId = ToolUtil::GenerateCliSessionId("shell", record);
    record->toolName = "shell";
    record->timeoutMs = param.options.timeout * COEFFICIENT;
    record->SetState(SessionState::RUNNING);
    record->SetBackground(param.options.background);
    record->eventId = eventId;
    auto fatherSessionRecords = GetSessionRecords();
    AddSessionRecord(record);
    auto subscribeRet = SubscribeSession(record->sessionId, subscriptionId, scheduler);
    if (subscribeRet != ERR_OK) {
        RemoveSessionRecord(record->sessionId);
        return subscribeRet;
    }
    auto createRet = ProcessManager::GetInstance().CreateShellProcess(param,
        sandboxConfig, record, fatherSessionRecords);
    if (createRet != ERR_OK) {
        RemoveSessionRecord(record->sessionId);
        return createRet;
    }
    if (!RegisterSessionWithMonitors(record, param.options)) {
        ProcessManager::GetInstance().Killpg(record->processId);
        RemoveSessionRecord(record->sessionId);
        return ERR_NO_INIT;
    }
    if (!bundleName.empty()) {
        RegisterAppStateObserver(bundleName, record->callerPid);
    }
    if (param.options.background) {
        HandleBackgroundSessionReply(record, eventId);
    }
    return ERR_OK;
}

void CliToolManagerService::PostExecToolTask(int64_t time, const std::string &sessionId, bool isTimeout)
{
    wptr<CliToolManagerService> weakSelf = this;
    auto timeoutTask = [weakSelf, sessionId, isTimeout]() {
        auto service = weakSelf.promote();
        if (service == nullptr || !service->initialized_.load(std::memory_order_acquire)) {
            return;
        }
        if (isTimeout) {
            service->HandleProcessTimeout(sessionId);
        } else {
            service->HandleProcessYieldTimeout(sessionId);
        }
    };
    ffrt::submit(std::move(timeoutTask), ffrt::task_attr().delay(time * COEFFICIENT));
}

void CliToolManagerService::WaitPid(pid_t pid, int32_t status, int32_t sig)
{
    std::shared_ptr<SessionRecord> record = nullptr;
    {
        std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
        for (auto iter = sessionRecords_.begin(); iter != sessionRecords_.end();) {
            if (iter->second == nullptr) {
                std::string sessionId = iter->first;
                iter = sessionRecords_.erase(iter);
                TAG_LOGW(AAFwkTag::CLI_TOOL, "delete leak sessionId:%{public}s", sessionId.c_str());
                continue;
            }
            if (pid == iter->second->processId) {
                record = iter->second;
                break;
            }
            ++iter;
        }
    }
    AccessToken::AccessTokenKit::DeleteToolTokenByPid(pid);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "WaitPid delete tool pid:%{public}d", pid);

    if (record) {
        // Get bundle name for event reporting
        std::string bundleName;
        auto tokenId = static_cast<AccessToken::AccessTokenID>(IPCSkeleton::GetCallingTokenID());
        AppExecFwk::BundleInfo bundleInfo;
        if (ToolUtil::GetBundleInfoByTokenId(tokenId, bundleInfo)) {
            bundleName = bundleInfo.name;
        }

        TAG_LOGI(AAFwkTag::CLI_TOOL, "WaitPid: found record for pid=%{public}d, toolName=%{public}s",
            pid, record->toolName.c_str());

        int32_t termSignal = 0;
        if (WIFSIGNALED(status)) {
            termSignal = WTERMSIG(status);
            TAG_LOGI(AAFwkTag::CLI_TOOL, "WaitPid: process killed by signal=%{public}d", termSignal);
            if (termSignal != 0 && termSignal != SIGTERM && termSignal != SIGHUP && termSignal != SIGCHLD) {
                ReportCliSignal(record->toolName, std::to_string(termSignal));
            }
        }

        record->SetTerminalResult(status, sig);
        if (record->OutputDrained()) {
            FinalizeBackgroundSession(record);
        }
    } else {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "WaitPid: no record found for pid=%{public}d", pid);
    }
}

void CliToolManagerService::sigchld_handler(int32_t sig, siginfo_t *info, void *context)
{
    int32_t savedErrno = errno;
    int32_t notifyFd = static_cast<int32_t>(g_childExitNotifyWriteFd);
    if (notifyFd >= 0) {
        uint8_t notify = static_cast<uint8_t>(sig);
        (void)write(notifyFd, &notify, sizeof(notify));
    }
    if (g_hasPreviousSigChldAction != 0) {
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

void CliToolManagerService::OnProcessDied(const std::string &bundleName, pid_t diedPid)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "OnProcessDied called: bundleName=%{public}s, diedPid=%{public}d",
        bundleName.c_str(), diedPid);
    RemoveCallerPid(diedPid);
    {
        std::lock_guard<ffrt::mutex> guard(observerMutex_);
        bundleObservers_.erase(bundleName);
    }
    std::vector<std::shared_ptr<SessionRecord>> cleanupRecords;
    // Iterate through sessionRecords_ to find matching SessionRecord by callerPid
    {
        std::lock_guard<ffrt::mutex> guard(sessionsMutex_);
        for (auto iter = sessionRecords_.begin(); iter != sessionRecords_.end();) {
            auto sessionRecord = iter->second;
            if (sessionRecord == nullptr) {
                std::string sessionId = iter->first;
                iter = sessionRecords_.erase(iter);
                TAG_LOGW(AAFwkTag::CLI_TOOL, "delete leak sessionId:%{public}s", sessionId.c_str());
                continue;
            }

            // Check if this session's callerPid matches the diedPid
            if (sessionRecord->GetCallerPid() != diedPid) {
                ++iter;
                continue;
            }

            cleanupRecords.emplace_back(sessionRecord);
            iter = sessionRecords_.erase(iter);
        }
    }

    for (const auto &record : cleanupRecords) {
        CleanupSessionAfterCallerDied(record);
    }
}

void CliToolManagerService::RegisterAppStateObserver(const std::string &bundleName, pid_t callerPid)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "RegisterAppStateObserver called: bundleName=%{public}s, callerPid=%{public}d",
        bundleName.c_str(), callerPid);
    // Check if observer already exists for this bundle
    std::lock_guard<ffrt::mutex> guard(observerMutex_);
    if (bundleObservers_.find(bundleName) != bundleObservers_.end()) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Observer already registered for bundleName=%{public}s", bundleName.c_str());
        return;
    }

    // Create observer with callback to OnProcessDied
    wptr<CliToolManagerService> weakSelf = this;
    auto callback = [weakSelf](const std::string &bundleName, pid_t diedPid) {
        auto service = weakSelf.promote();
        if (service == nullptr || !service->initialized_.load(std::memory_order_acquire)) {
            return;
        }
        service->OnProcessDied(bundleName, diedPid);
    };

    sptr<CliToolAppStateObserver> observer = new CliToolAppStateObserver(bundleName, callback);
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to create observer for bundleName=%{public}s", bundleName.c_str());
        return;
    }

    // Register observer through AppMgrClient
    AppExecFwk::AppMgrClient appMgrClient;
    auto ret = appMgrClient.ConnectAppMgrService();
    if (ret != AppExecFwk::AppMgrResultCode::RESULT_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to connect to AppMgrService");
        return;
    }

    std::vector<std::string> bundleNameList = { bundleName };
    auto registerRet = appMgrClient.RegisterApplicationStateObserver(observer, bundleNameList);
    if (registerRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to register observer for bundleName=%{public}s, ret=%{public}d",
            bundleName.c_str(), registerRet);
        return;
    }

    // Store observer
    bundleObservers_[bundleName] = observer;
    AddCallerPid(callerPid);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully registered observer for bundleName=%{public}s", bundleName.c_str());
}

std::shared_ptr<SessionRecord> CliToolManagerService::CreateSessionRecord(const ExecToolParam &param,
    const std::string &eventId)
{
    auto record = std::make_shared<SessionRecord>();
    record->callerPid = IPCSkeleton::GetCallingPid();
    record->callerUid = IPCSkeleton::GetCallingUid();
    record->sessionId = ToolUtil::GenerateCliSessionId(param.toolName, record);
    record->toolName = param.toolName;
    record->timeoutMs = param.options.timeout * COEFFICIENT;
    record->SetState(SessionState::RUNNING);
    record->SetBackground(param.options.background);
    record->eventId = eventId;
    return record;
}

int32_t CliToolManagerService::ClearSession(const std::string &sessionId)
{
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_EXEC_CLI_TOOL)) {
        return ERR_PERMISSION_DENIED;
    }

    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ClearSession failed: sessionId=%{public}s not found", sessionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    if (!IsSessionOwner(record, "ClearSession")) {
        return ERR_PERMISSION_DENIED;
    }
    if (record->GetState() != SessionState::RUNNING && record->GetState() != SessionState::SPAWNING) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "ClearSession failed: sessionId=%{public}s state=%{public}d is not cancellable",
            sessionId.c_str(), static_cast<int32_t>(record->GetState()));
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ClearSession: sessionId=%{public}s, pid=%{public}d",
        sessionId.c_str(), record->processId);

    if (record->sessionType == SessionType::SKILL) {
        // Skill sessions don't have a child process, mark as cancelling and clean up
        record->SetState(SessionState::CANCELLING);
        EventDispatcher::GetInstance().DispatchExitEvent(sessionId, 0);
        EventDispatcher::GetInstance().ClearSessionSubscribers(sessionId);
        RemoveSessionRecord(sessionId);
        return ERR_OK;
    }

    if (!ProcessManager::GetInstance().Killpg(record->processId)) {
        return ERR_NOT_KILL;
    }
    record->SetState(SessionState::CANCELLING);
    return ERR_OK;
}

int32_t CliToolManagerService::SubscribeSession(const std::string &sessionId, const std::string &subscriptionId,
    const sptr<ICliToolManagerScheduler> &scheduler)
{
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_EXEC_CLI_TOOL)) {
        return ERR_PERMISSION_DENIED;
    }

    if (sessionId.empty() || subscriptionId.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "SubscribeSession failed: invalid args sessionId=%{public}s, subscriptionId=%{public}s",
            sessionId.c_str(), subscriptionId.c_str());
        return ERR_INVALID_PARAM;
    }
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "SubscribeSession failed: sessionId=%{public}s not found, subscriptionId=%{public}s",
            sessionId.c_str(), subscriptionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    if (!IsSessionOwner(record, "SubscribeSession")) {
        return ERR_PERMISSION_DENIED;
    }
    CliSessionInfo session;
    record->BuildSessionInfo(session);
    if (session.status != "running") {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "SubscribeSession failed: sessionId=%{public}s status=%{public}s is not subscribable",
            sessionId.c_str(), session.status.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (!EventDispatcher::GetInstance().SetScheduler(callerPid, callerUid, scheduler)) {
        return ERR_NO_INIT;
    }
    if (!EventDispatcher::GetInstance().RegisterSubscriber(
        sessionId, subscriptionId, callerPid, callerUid)) {
        return ERR_NO_INIT;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId)
{
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_EXEC_CLI_TOOL)) {
        return ERR_PERMISSION_DENIED;
    }

    if (!EventDispatcher::GetInstance().UnregisterSubscriber(
        sessionId, subscriptionId, IPCSkeleton::GetCallingPid(), IPCSkeleton::GetCallingUid())) {
        return ERR_NO_INIT;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::QuerySession(const std::string &sessionId, CliSessionInfo &session)
{
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_EXEC_CLI_TOOL)) {
        return ERR_PERMISSION_DENIED;
    }

    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "QuerySession failed: sessionId=%{public}s not found", sessionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    if (!IsSessionOwner(record, "QuerySession")) {
        return ERR_PERMISSION_DENIED;
    }
    record->BuildSessionInfo(session);
    return ERR_OK;
}

int32_t CliToolManagerService::SendMessage(const std::string &sessionId,
    const std::string &inputText, const std::string &eventId,
    const sptr<ICliToolManagerScheduler> &scheduler)
{
    InterfaceCallCounter counter(interfaceCalledCount_);
    auto fullTokenId = IPCSkeleton::GetCallingFullTokenID();
    if (!AccessToken::TokenIdKit::IsSystemAppByFullTokenID(fullTokenId)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Not system app");
        return ERR_NOT_SYSTEM_APP;
    }

    auto tokenId = IPCSkeleton::GetCallingTokenID();
    if (!PermissionUtil::VerifyAccessToken(tokenId, PERMISSION_EXEC_CLI_TOOL)) {
        return ERR_PERMISSION_DENIED;
    }

    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "SendMessage failed: sessionId=%{public}s not found", sessionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    if (!IsSessionOwner(record, "SendMessage")) {
        return ERR_PERMISSION_DENIED;
    }

    if (ioMonitor_ == nullptr) {
        return ERR_CLI_SEND_MESSAGE;
    }

    // Check session state
    if (record->GetState() != SessionState::RUNNING) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "SendMessage failed: sessionId=%{public}s is not running", record->sessionId.c_str());
        return ERR_CLI_SEND_MESSAGE;
    }

    // Skill sessions don't support stdin
    if (record->sessionType == SessionType::SKILL) {
        TAG_LOGE(AAFwkTag::CLI_TOOL,
            "SendMessage failed: sessionId=%{public}s is a skill session (no stdin)", sessionId.c_str());
        return ERR_CLI_SEND_MESSAGE;
    }

    int32_t callerPid = IPCSkeleton::GetCallingPid();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    if (!EventDispatcher::GetInstance().SetScheduler(callerPid, callerUid, scheduler)) {
        return ERR_NO_INIT;
    }
    ioMonitor_->SendMessage(sessionId, inputText, eventId);
    return ERR_OK;
}

int32_t CliToolManagerService::BatchQueryPermissionBySubCommand(
    const std::vector<Command> &cmds,
    std::vector<CommandPermission> &cmdPermissions)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "BatchQueryPermissionBySubCommand begin, cnt=%{public}zu", cmds.size());
    InterfaceCallCounter counter(interfaceCalledCount_);
    if (cmds.empty() || cmds.size() >= MAX_QUERY_CMDS_SIZE) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "cmds is empty or reach limit");
        return ERR_INVALID_PARAM;
    }

    auto callerToken = IPCSkeleton::GetCallingTokenID();
    if (Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken) !=
        Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Caller is not SA");
        return ERR_NOT_SA_CALLER;
    }

    if (!PermissionUtil::VerifyAccessToken(callerToken, PERMISSION_QUERY_CLI_TOOL)) {
        return ERR_PERMISSION_DENIED;
    }

    return PermissionQueryUtil::BatchQueryPermissions(cmds, cmdPermissions);
}

SkillCallbackAdapter::SkillCallbackAdapter(const std::string &sessionId,
    int32_t callerPid, int32_t callerUid, const std::string &eventId,
    wptr<CliToolManagerService> service)
    : sessionId_(sessionId), callerPid_(callerPid), callerUid_(callerUid), eventId_(eventId), service_(service)
{}

void SkillCallbackAdapter::OnExecuteDone(const std::string &requestCode, int32_t resultCode,
    const AppExecFwk::SkillExecuteResult &result)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "SkillCallbackAdapter::OnExecuteDone sessionId:%{public}s code:%{public}d",
        sessionId_.c_str(), resultCode);

    auto service = service_.promote();
    if (service == nullptr || !service->initialized_.load(std::memory_order_acquire)) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "service expired for sessionId:%{public}s", sessionId_.c_str());
        return;
    }

    auto record = service->GetSessionRecord(sessionId_);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "OnExecuteDone skipped: sessionId:%{public}s already cleaned", sessionId_.c_str());
        return;
    }

    std::string outputText;
    if (result.result != nullptr) {
        outputText = result.result->ToString();
    }
    record->SetSkillResult(resultCode, outputText);
    record->SetState(resultCode == ERR_OK ? SessionState::COMPLETED : SessionState::FAILED);

    auto session = ToolUtil::BuildSkillSessionInfo(sessionId_, resultCode, result);
    service->HandleSkillSessionComplete(sessionId_, callerPid_, callerUid_, eventId_, resultCode, session);
}

int32_t CliToolManagerService::ValidateSkillTypeFromParam(ExecToolParam &param, int32_t &skillType)
{
    ToolUtil::NormalizeSkillParamKeys(param.args);
    auto bundleName = param.args.GetStringParam("bundleName");
    auto moduleName = param.args.GetStringParam("moduleName");
    auto skillName = param.args.GetStringParam("skillName");
    if (skillName.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "skillName is required in args");
        return ERR_INVALID_VALUE;
    }
    return ValidateSkillType(bundleName, moduleName, skillName, skillType);
}

int32_t CliToolManagerService::ValidateSkillType(const std::string &bundleName,
    const std::string &moduleName, const std::string &skillName, int32_t &skillType)
{
    auto queryRet = AAFwk::AbilityManagerClient::GetInstance()->QuerySkillType(
        bundleName, moduleName, skillName, skillType);
    if (queryRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "querySkillType failed:%{public}d", queryRet);
        return queryRet;
    }
    return ERR_OK;
}

int32_t CliToolManagerService::SetupAndStartSkillSession(const ExecToolParam &param,
    const std::string &eventId, const ToolInfo &toolInfo)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "SetupAndStartSkillSession: toolName=%{public}s",
        param.toolName.c_str());

    auto args = param.args;
    auto bundleName = args.GetStringParam("bundleName");
    auto moduleName = args.GetStringParam("moduleName");
    auto skillName = args.GetStringParam("skillName");
    auto scriptPath = args.GetStringParam("scriptPath");
    auto funcName = args.GetStringParam("functionName");

    ToolUtil::ExpandArgsJsonString(args);
    auto skillArgs = ToolUtil::FilterSkillArgs(args);

    auto record = CreateSessionRecord(param, eventId);
    if (record == nullptr) {
        return ERR_NO_INIT;
    }
    record->sessionType = SessionType::SKILL;
    AddSessionRecord(record);

    auto callerTokenId = IPCSkeleton::GetCallingTokenID();
    wptr<CliToolManagerService> weakSelf = this;
    auto adaptor = sptr<SkillCallbackAdapter>::MakeSptr(
        record->sessionId, record->callerPid, record->callerUid, eventId, weakSelf);

    AppExecFwk::SkillExecuteRequest skillRequest;
    skillRequest.callerTokenId = callerTokenId;
    skillRequest.bundleName = bundleName;
    skillRequest.moduleName = moduleName;
    skillRequest.skillName = skillName;
    skillRequest.scriptPath = scriptPath;
    skillRequest.functionName = funcName;
    skillRequest.skillArgs = skillArgs;

    TAG_LOGD(AAFwkTag::CLI_TOOL, "execSkill before ExecuteInAppSkillWithTokenId");
    int32_t ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInAppSkillWithTokenId(
        skillRequest, adaptor);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "ExecuteInAppSkillWithTokenId failed:%{public}d", ret);
        RemoveSessionRecord(record->sessionId);
        return ret;
    }

    if (param.options.background) {
        HandleBackgroundSessionReply(record, eventId);
    }

    return ERR_OK;
}

void CliToolManagerService::HandleSkillSessionComplete(const std::string &sessionId,
    int32_t callerPid, int32_t callerUid, const std::string &eventId, int32_t resultCode,
    const CliSessionInfo &session)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleSkillSessionComplete skipped: sessionId:%{public}s not found", sessionId.c_str());
        return;
    }
    if (!record->BeginCleanup()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleSkillSessionComplete skipped: already cleaning sessionId:%{public}s", sessionId.c_str());
        return;
    }

    auto oldBackground = record->SetBackground(true);
    if (!oldBackground) {
        EventDispatcher::GetInstance().DispatchExecToolReplyEvent(callerPid, callerUid, eventId, ERR_OK, session);
    }

    EventDispatcher::GetInstance().DispatchExitEvent(sessionId, 0);
    EventDispatcher::GetInstance().ClearSessionSubscribers(sessionId);
    RemoveSessionRecord(sessionId);
}

void CliToolManagerService::HandleSkillSessionTimeout(const std::string &sessionId)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL,
            "HandleSkillSessionTimeout skipped: sessionId:%{public}s not found", sessionId.c_str());
        return;
    }

    record->SetTimeout(true);
    record->SetState(SessionState::FAILED);

    auto oldBackground = record->SetBackground(true);
    if (!oldBackground) {
        CliSessionInfo session;
        record->BuildSessionInfo(session);
        EventDispatcher::GetInstance().DispatchExecToolReplyEvent(
            record->callerPid, record->callerUid, record->eventId, ERR_OK, session);
    }

    EventDispatcher::GetInstance().DispatchErrorEvent(sessionId, "session timed out");
    EventDispatcher::GetInstance().DispatchExitEvent(sessionId, 0);
    EventDispatcher::GetInstance().ClearSessionSubscribers(sessionId);
    RemoveSessionRecord(sessionId);
}

void CliToolManagerService::RemoveCallerPid(int64_t pid)
{
    std::lock_guard<ffrt::mutex> guard(callerPidsMutex_);
    callerPids.erase(pid);
}

void CliToolManagerService::AddCallerPid(int64_t pid)
{
    std::lock_guard<ffrt::mutex> guard(callerPidsMutex_);
    callerPids.insert(pid);
}

int32_t CliToolManagerService::GetCallerPidCount()
{
    std::lock_guard<ffrt::mutex> guard(callerPidsMutex_);
    return static_cast<int32_t>(callerPids.size());
}
} // namespace CliTool
} // namespace OHOS
