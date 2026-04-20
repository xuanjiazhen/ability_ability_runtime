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

#include "cli_manager_service.h"

#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <functional>
#include <memory>
#include <new>
#include <nlohmann/json.hpp>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <unordered_map>
#include <vector>

#include "cli_manager_errors.h"
#include "cli_manager_scheduler_proxy.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "iremote_object.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t CLI_MANAGER_SA_ID = 8899;
constexpr int32_t ERR_OK = 0;
constexpr int32_t DEFAULT_TIMEOUT_SEC = 30 * 60;

int64_t GetCurrentTimeMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

// 时间戳日志宏
#define LOG_TIMESTAMP(tag, desc) \
    do { \
        auto nowMs = GetCurrentTimeMs(); \
        TAG_LOGI(AAFwkTag::ABILITYMGR, "[TIMESTAMP] [%{public}s] %{public}lld - %{public}s", \
            tag, static_cast<long long>(nowMs), desc); \
    } while(0)

class CallbackDeathRecipient final : public IRemoteObject::DeathRecipient {
public:
    using Handler = std::function<void(const wptr<IRemoteObject> &)>;

    explicit CallbackDeathRecipient(Handler handler) : handler_(std::move(handler))
    {}

    ~CallbackDeathRecipient() override = default;

    void OnRemoteDied(const wptr<IRemoteObject> &remote) override
    {
        if (handler_) {
            handler_(remote);
        }
    }

private:
    Handler handler_;
};

class EventDispatcher final {
public:
    struct SubscriberKey {
        int32_t callerPid = 0;
        std::string subscriptionId;

        bool operator==(const SubscriberKey &other) const
        {
            return callerPid == other.callerPid && subscriptionId == other.subscriptionId;
        }
    };

    struct SubscriberKeyHash {
        size_t operator()(const SubscriberKey &key) const
        {
            return std::hash<int32_t>()(key.callerPid) ^ (std::hash<std::string>()(key.subscriptionId) << 1);
        }
    };

    struct SchedulerState {
        sptr<ICliManagerScheduler> scheduler;
        sptr<IRemoteObject> remote;
        sptr<IRemoteObject::DeathRecipient> deathRecipient;
    };

    struct SubscriberState {
        SubscriberKey key;
        sptr<ICliManagerScheduler> scheduler;
    };

    static EventDispatcher &GetInstance()
    {
        static EventDispatcher instance;
        return instance;
    }

    int32_t RegisterScheduler(int32_t callerPid, const sptr<ICliManagerScheduler> &scheduler)
    {
        if (callerPid <= 0 || scheduler == nullptr || scheduler->AsObject() == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "RegisterScheduler failed: invalid callerPid=%{public}d or scheduler remote is null", callerPid);
            return ERR_CLI_INVALID_SCHEDULER_PARAMS;
        }

        auto remote = scheduler->AsObject();
        auto deathRecipient = sptr<IRemoteObject::DeathRecipient>(
            new (std::nothrow) CallbackDeathRecipient([callerPid](const wptr<IRemoteObject> &) {
                EventDispatcher::GetInstance().UnregisterScheduler(callerPid);
            }));
        if (deathRecipient == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "RegisterScheduler failed: alloc death recipient for pid %{public}d",
                callerPid);
            return ERR_CLI_MEMORY_ALLOC_FAILED;
        }
        if (!remote->AddDeathRecipient(deathRecipient)) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "AddDeathRecipient failed for scheduler pid %{public}d", callerPid);
        }

        sptr<IRemoteObject> oldRemote;
        sptr<IRemoteObject::DeathRecipient> oldDeathRecipient;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto schedulerIt = schedulers_.find(callerPid);
            if (schedulerIt != schedulers_.end()) {
                oldRemote = schedulerIt->second.remote;
                oldDeathRecipient = schedulerIt->second.deathRecipient;
                RemoveSubscribersForPidLocked(callerPid);
            }
            schedulers_[callerPid] = SchedulerState {scheduler, remote, deathRecipient};
        }

        if (oldRemote != nullptr && oldDeathRecipient != nullptr) {
            oldRemote->RemoveDeathRecipient(oldDeathRecipient);
        }
        return ERR_OK;
    }

    int32_t UnregisterScheduler(int32_t callerPid)
    {
        sptr<IRemoteObject> remote;
        sptr<IRemoteObject::DeathRecipient> deathRecipient;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = schedulers_.find(callerPid);
            if (it != schedulers_.end()) {
                remote = it->second.remote;
                deathRecipient = it->second.deathRecipient;
                schedulers_.erase(it);
            }
            RemoveSubscribersForPidLocked(callerPid);
        }
        if (remote != nullptr && deathRecipient != nullptr) {
            remote->RemoveDeathRecipient(deathRecipient);
        }
        return ERR_OK;
    }

    int32_t RegisterSubscriber(const std::string &sessionId, const std::string &subscriptionId, int32_t callerPid)
    {
        if (sessionId.empty() || subscriptionId.empty() || callerPid <= 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "RegisterSubscriber failed: invalid args sessionId='%{public}s', subscriptionId='%{public}s', "
                "callerPid=%{public}d", sessionId.c_str(), subscriptionId.c_str(), callerPid);
            return ERR_CLI_INVALID_SUBSCRIBER_PARAMS;
        }

        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto schedulerIt = schedulers_.find(callerPid);
            if (schedulerIt == schedulers_.end() || schedulerIt->second.scheduler == nullptr) {
                TAG_LOGE(AAFwkTag::ABILITYMGR,
                    "RegisterSubscriber failed: scheduler not found for pid %{public}d, sessionId='%{public}s', "
                    "subscriptionId='%{public}s'", callerPid, sessionId.c_str(), subscriptionId.c_str());
                return ERR_CLI_SCHEDULER_NOT_FOUND;
            }
            auto &subscribers = sessionSubscribers_[sessionId];
            SubscriberKey key {callerPid, subscriptionId};
            subscribers[key] = SubscriberState {key, schedulerIt->second.scheduler};
        }
        return ERR_OK;
    }

    int32_t UnregisterSubscriber(const std::string &sessionId, const std::string &subscriptionId, int32_t callerPid)
    {
        if (sessionId.empty() || subscriptionId.empty() || callerPid <= 0) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "UnregisterSubscriber failed: invalid args sessionId='%{public}s', subscriptionId='%{public}s', "
                "callerPid=%{public}d", sessionId.c_str(), subscriptionId.c_str(), callerPid);
            return ERR_CLI_INVALID_SUBSCRIBER_PARAMS;
        }

        std::lock_guard<std::mutex> lock(mutex_);
        auto sessionIt = sessionSubscribers_.find(sessionId);
        if (sessionIt == sessionSubscribers_.end()) {
            TAG_LOGW(AAFwkTag::ABILITYMGR,
                "UnregisterSubscriber skipped: no subscribers for sessionId='%{public}s'", sessionId.c_str());
            return ERR_OK;
        }

        sessionIt->second.erase(SubscriberKey {callerPid, subscriptionId});
        if (sessionIt->second.empty()) {
            sessionSubscribers_.erase(sessionIt);
        }
        return ERR_OK;
    }

    int32_t DispatchIOEvent(const std::string &sessionId, const std::string &eventType, const std::string &data)
    {
        ToolEvent event;
        event.type = eventType;
        event.eventData = data;
        event.exitCode = 0;
        event.timestamp = GetCurrentTimeMs();
        return DispatchEvent(sessionId, event);
    }

    int32_t DispatchErrorEvent(const std::string &sessionId, const std::string &error)
    {
        return DispatchIOEvent(sessionId, "error", error);
    }

    int32_t DispatchExitEvent(const std::string &sessionId, int32_t exitCode)
    {
        ToolEvent event;
        event.type = "exit";
        event.eventData = "";
        event.exitCode = exitCode;
        event.timestamp = GetCurrentTimeMs();
        return DispatchEvent(sessionId, event);
    }

    void ClearSessionSubscribers(const std::string &sessionId)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        sessionSubscribers_.erase(sessionId);
    }

    void ClearAll()
    {
        std::vector<std::pair<sptr<IRemoteObject>, sptr<IRemoteObject::DeathRecipient>>> cleanupList;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto &[pid, schedulerState] : schedulers_) {
                cleanupList.emplace_back(schedulerState.remote, schedulerState.deathRecipient);
            }
            schedulers_.clear();
            sessionSubscribers_.clear();
        }
        for (const auto &[remote, deathRecipient] : cleanupList) {
            if (remote != nullptr && deathRecipient != nullptr) {
                remote->RemoveDeathRecipient(deathRecipient);
            }
        }
    }

private:
    int32_t DispatchEvent(const std::string &sessionId, const ToolEvent &event)
    {
        LOG_TIMESTAMP("Service-DispatchEvent", "DispatchEvent START");
        std::vector<SubscriberState> subscribers;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto sessionIt = sessionSubscribers_.find(sessionId);
            if (sessionIt == sessionSubscribers_.end()) {
                TAG_LOGW(AAFwkTag::ABILITYMGR,
                    "DispatchEvent skipped: no subscribers for sessionId='%{public}s', eventType='%{public}s'",
                    sessionId.c_str(), event.type.c_str());
                return ERR_OK;
            }
            for (const auto &[key, state] : sessionIt->second) {
                subscribers.push_back(state);
            }
        }

        std::vector<SubscriberKey> failedSubscribers;
        for (const auto &subscriber : subscribers) {
            LOG_TIMESTAMP("Service-DispatchEvent", "Before ScheduleSessionEvent IPC call");
            if (subscriber.scheduler == nullptr ||
                subscriber.scheduler->ScheduleSessionEvent(sessionId, subscriber.key.subscriptionId, event) != ERR_OK) {
                TAG_LOGW(AAFwkTag::ABILITYMGR,
                    "DispatchEvent failed for pid %{public}d, sessionId='%{public}s', subscriptionId='%{public}s', "
                    "eventType='%{public}s'", subscriber.key.callerPid, sessionId.c_str(),
                    subscriber.key.subscriptionId.c_str(), event.type.c_str());
                failedSubscribers.push_back(subscriber.key);
            }
            LOG_TIMESTAMP("Service-DispatchEvent", "After ScheduleSessionEvent IPC call");
        }

        if (!failedSubscribers.empty()) {
            std::lock_guard<std::mutex> lock(mutex_);
            auto sessionIt = sessionSubscribers_.find(sessionId);
            if (sessionIt != sessionSubscribers_.end()) {
                for (const auto &key : failedSubscribers) {
                    sessionIt->second.erase(key);
                }
                if (sessionIt->second.empty()) {
                    sessionSubscribers_.erase(sessionIt);
                }
            }
        }
        LOG_TIMESTAMP("Service-DispatchEvent", "DispatchEvent END (Event dispatched to all subscribers)");
        return ERR_OK;
    }

    void RemoveSubscribersForPidLocked(int32_t callerPid)
    {
        for (auto sessionIt = sessionSubscribers_.begin(); sessionIt != sessionSubscribers_.end();) {
            auto &subscribers = sessionIt->second;
            for (auto it = subscribers.begin(); it != subscribers.end();) {
                if (it->first.callerPid == callerPid) {
                    it = subscribers.erase(it);
                } else {
                    ++it;
                }
            }
            if (subscribers.empty()) {
                sessionIt = sessionSubscribers_.erase(sessionIt);
            } else {
                ++sessionIt;
            }
        }
    }

    std::unordered_map<int32_t, SchedulerState> schedulers_;
    std::unordered_map<std::string, std::unordered_map<SubscriberKey, SubscriberState, SubscriberKeyHash>>
        sessionSubscribers_;
    std::mutex mutex_;
};
} // namespace

sptr<CliManagerService> CliManagerService::instance_ = nullptr;
std::mutex CliManagerService::instanceMutex_;
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(CliManagerService::GetInstance());

sptr<CliManagerService> CliManagerService::GetInstance()
{
    std::lock_guard<std::mutex> lock(instanceMutex_);
    if (instance_ != nullptr) {
        return instance_;
    }
    instance_ = new (std::nothrow) CliManagerService();
    return instance_;
}

CliManagerService::CliManagerService() : SystemAbility(CLI_MANAGER_SA_ID, true)
{}

CliManagerService::~CliManagerService() = default;

void CliManagerService::Init()
{
    cliToolManager_ = std::make_unique<CliToolManager>();
    timeoutManager_ = std::make_shared<TimeoutManager>();
    if (timeoutManager_ != nullptr) {
        timeoutManager_->Start();
    }
    processMonitor_.SetExitCallback([this](const std::string &sessionId, int status, bool timedOut) {
        HandleProcessExit(sessionId, status, timedOut);
    });
    processMonitor_.SetTimeoutCallback([](const std::string &sessionId) {
        EventDispatcher::GetInstance().DispatchErrorEvent(sessionId, "session timed out");
    });
    ioMonitor_.SetOutputCallback([](const std::string &sessionId, bool isStdout, const std::string &data) {
        LOG_TIMESTAMP("Service-OutputCallback", "Received data from IOMonitor");
        auto record = CliManagerService::GetInstance()->GetSessionRecord(sessionId);
        if (record != nullptr) {
            record->AppendOutput(isStdout, data);
        }
        LOG_TIMESTAMP("Service-OutputCallback", "Before DispatchIOEvent");
        EventDispatcher::GetInstance().DispatchIOEvent(sessionId, isStdout ? "stdout" : "stderr", data);
        LOG_TIMESTAMP("Service-OutputCallback", "After DispatchIOEvent (Service callback done)");
    });
    ioMonitor_.SetSessionClosedCallback([this](const std::string &sessionId, bool isStdout) {
        HandleOutputClosed(sessionId, isStdout);
    });
    ioMonitor_.SetSessionDrainedCallback([this](const std::string &sessionId) {
        HandleOutputDrained(sessionId);
    });
    ioMonitor_.Start();
    processMonitor_.Start();
}

void CliManagerService::OnStart()
{
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "OnStart skipped: service already running");
        return;
    }

    stopping_.store(false, std::memory_order_release);
    Init();

    if (!Publish(CliManagerService::GetInstance())) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnStart failed: Publish returned false");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
}

void CliManagerService::OnStop()
{
    state_ = ServiceRunningState::STATE_NOT_START;
    stopping_.store(true, std::memory_order_release);

    std::vector<pid_t> activePids;
    {
        std::lock_guard<std::mutex> lock(sessionsMutex_);
        for (const auto &[sessionId, record] : sessionRecords_) {
            if (record != nullptr && record->processId > 0) {
                activePids.push_back(record->processId);
            }
        }
        sessionRecords_.clear();
    }
    EventDispatcher::GetInstance().ClearAll();

    for (pid_t pid : activePids) {
        kill(pid, SIGKILL);
    }

    ioMonitor_.Stop();
    processMonitor_.Stop();
    if (timeoutManager_ != nullptr) {
        timeoutManager_->Stop();
    }
}

std::string CliManagerService::GenerateSessionId(const std::string &name, int64_t startTimeMs)
{
    uint64_t counter = sessionCounter_.fetch_add(1, std::memory_order_relaxed) + 1;
    return name + "-" + std::to_string(startTimeMs) + "-" + std::to_string(counter);
}

void CliManagerService::AddSessionRecord(const std::shared_ptr<SessionRecord> &record)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    sessionRecords_[record->sessionId] = record;
}

std::shared_ptr<SessionRecord> CliManagerService::GetSessionRecord(const std::string &sessionId)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    auto it = sessionRecords_.find(sessionId);
    if (it == sessionRecords_.end()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "GetSessionRecord failed: sessionId='%{public}s' not found", sessionId.c_str());
        return nullptr;
    }
    return it->second;
}

void CliManagerService::RemoveSessionRecord(const std::string &sessionId)
{
    std::lock_guard<std::mutex> lock(sessionsMutex_);
    sessionRecords_.erase(sessionId);
}

void CliManagerService::HandleProcessExit(const std::string &sessionId, int status, bool timedOut)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR,
            "HandleProcessExit skipped: sessionId='%{public}s' not found, status=%{public}d, timedOut=%{public}d",
            sessionId.c_str(), status, timedOut);
        return;
    }
    record->SetTerminalResult(status, timedOut);
    if (record->OutputDrained()) {
        FinalizeBackgroundSession(record);
    }
}

void CliManagerService::HandleOutputClosed(const std::string &sessionId, bool isStdout)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR,
            "HandleOutputClosed skipped: sessionId='%{public}s' not found, isStdout=%{public}d",
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

void CliManagerService::HandleOutputDrained(const std::string &sessionId)
{
    auto record = GetSessionRecord(sessionId);
    if (record != nullptr && record->HasProcessExited()) {
        FinalizeBackgroundSession(record);
    }
}

void CliManagerService::FinalizeBackgroundSession(const std::shared_ptr<SessionRecord> &record)
{
    if (record == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "FinalizeBackgroundSession skipped: record is null");
        return;
    }
    if (!record->BeginCleanup()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR,
            "FinalizeBackgroundSession skipped: cleanup already started for sessionId='%{public}s'",
            record->sessionId.c_str());
        return;
    }

    const int status = record->GetTerminalStatus();
    const int32_t exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;

    if (timeoutManager_ != nullptr) {
        timeoutManager_->CancelTimeoutTask(record->sessionId);
    }
    EventDispatcher::GetInstance().DispatchExitEvent(record->sessionId, exitCode);
    ioMonitor_.UnregisterSession(record->sessionId);
    processMonitor_.UnregisterSession(record->sessionId);
    EventDispatcher::GetInstance().ClearSessionSubscribers(record->sessionId);
    RemoveSessionRecord(record->sessionId);
}

int32_t CliManagerService::QueryTools(std::vector<ToolInfo> &tools)
{
    LOG_TIMESTAMP("Service", "QueryTools ENTRY");
    std::lock_guard<std::mutex> lock(mutex_);
    if (cliToolManager_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "QueryTools failed: cliToolManager_ is null");
        return ERR_CLI_MANAGER_NOT_INITIALIZED;
    }
    LOG_TIMESTAMP("Service", "QueryTools Before Manager call");
    int32_t ret = cliToolManager_->QueryTools(tools);
    LOG_TIMESTAMP("Service", "QueryTools After Manager call (Service Done)");
    return ret;
}

int32_t CliManagerService::ExecTool(const std::string &name,
    const std::vector<std::pair<std::string, std::string>> &args,
    const ExecOptions &options,
    const std::string &challenge,
    SessionInfo &session)
{
    LOG_TIMESTAMP("Service", "ExecTool ENTRY");
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (cliToolManager_ == nullptr || !cliToolManager_->ValidateTool(name)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "ExecTool failed: tool '%{public}s' not found or manager not initialized", name.c_str());
            return ERR_CLI_TOOL_NOT_FOUND;
        }
    }

    {
        std::lock_guard<std::mutex> lock(sessionsMutex_);
        if (sessionRecords_.size() >= MAX_SESSION_COUNT) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "ExecTool failed: too many sessions, current=%{public}zu, max=%{public}zu",
                sessionRecords_.size(), MAX_SESSION_COUNT);
            return ERR_CLI_TOO_MANY_SESSIONS;
        }
    }

    auto startTimePoint = std::chrono::system_clock::now();
    int64_t startTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        startTimePoint.time_since_epoch()).count();

    session.sessionId = GenerateSessionId(name, startTimeMs);
    session.toolName = name;
    session.status = "running";
    session.startTime = startTimeMs;
    session.endTime = 0;
    session.result = std::make_shared<ExecResult>();
    if (session.result == nullptr) {
        session.status = "failed";
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ExecTool failed: alloc ExecResult for tool '%{public}s'", name.c_str());
        return ERR_CLI_MEMORY_ALLOC_FAILED;
    }
    session.result->exitCode = -1;
    session.result->outputText = "";
    session.result->errorText = "";
    session.result->signalNumber = 0;
    session.result->timedOut = false;
    session.result->executionTime = 0;

    std::string sandboxConfig = GenerateSandBoxConfig(challenge);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecTool: options.timeout=%{public}d sec", options.timeout);
    int32_t timeoutMs = static_cast<int32_t>(ResolveTimeout(options, name));
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecTool: resolved timeoutMs=%{public}d ms (%{public}.2f sec)",
        timeoutMs, timeoutMs / 1000.0);
    if (timeoutMs == 0) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "ExecTool: timeoutMs is 0, NO timeout monitoring will be applied!");
    }

    int stdoutPipe[2];
    int stderrPipe[2];
    if (pipe(stdoutPipe) == -1) {
        session.status = "failed";
        session.result->errorText = "Failed to create stdout pipe: " + std::string(strerror(errno));
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ExecTool failed: create stdout pipe for tool '%{public}s': %{public}s",
            name.c_str(), strerror(errno));
        return ERR_CLI_CREATE_STDOUT_PIPE_FAILED;
    }
    if (pipe(stderrPipe) == -1) {
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        session.status = "failed";
        session.result->errorText = "Failed to create stderr pipe: " + std::string(strerror(errno));
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ExecTool failed: create stderr pipe for tool '%{public}s': %{public}s",
            name.c_str(), strerror(errno));
        return ERR_CLI_CREATE_STDERR_PIPE_FAILED;
    }

    LOG_TIMESTAMP("Service", "Before fork");
    pid_t pid = fork();
    if (pid == -1) {
        close(stdoutPipe[0]);
        close(stdoutPipe[1]);
        close(stderrPipe[0]);
        close(stderrPipe[1]);
        session.status = "failed";
        session.result->errorText = "Failed to fork: " + std::string(strerror(errno));
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ExecTool failed: fork for tool '%{public}s': %{public}s",
            name.c_str(), strerror(errno));
        return ERR_CLI_FORK_FAILED;
    }

    if (pid == 0) {
        LOG_TIMESTAMP("Service-Child", "After fork (child process)");
        if (prctl(PR_SET_PDEATHSIG, SIGTERM) != 0) {
            _exit(128);
        }
        if (getppid() == 1) {
            _exit(129);
        }
        close(stdoutPipe[0]);
        close(stderrPipe[0]);
        dup2(stdoutPipe[1], STDOUT_FILENO);
        dup2(stderrPipe[1], STDERR_FILENO);
        close(stdoutPipe[1]);
        close(stderrPipe[1]);

        std::vector<char*> execArgs;
        std::string clawSandbox = "/system/bin/claw_sandbox";
        std::string configPrompt = "--config";
        std::string cmdPrompt = "--cmd";
        std::string cmdline = name;
        execArgs.push_back(const_cast<char *>(clawSandbox.c_str()));
        execArgs.push_back(const_cast<char *>(configPrompt.c_str()));
        execArgs.push_back(const_cast<char *>(sandboxConfig.c_str()));
        execArgs.push_back(const_cast<char *>(cmdPrompt.c_str()));
        for (const auto &[key, value] : args) {
            cmdline += " " + key + " " + value;
        }
        execArgs.push_back(const_cast<char *>(cmdline.c_str()));
        execArgs.push_back(nullptr);
        LOG_TIMESTAMP("Service-Child", "Before execvp");
        execvp(execArgs[0], execArgs.data());
        _exit(127);
    }

    LOG_TIMESTAMP("Service-Parent", "After fork (parent process)");
    close(stdoutPipe[1]);
    close(stderrPipe[1]);

    std::shared_ptr<TimeoutContext> timeoutContext;
    if (timeoutManager_ != nullptr) {
        timeoutContext = std::make_shared<TimeoutContext>();
        if (timeoutContext != nullptr) {
            timeoutContext->sessionId = session.sessionId;
            timeoutContext->processId = pid;
            timeoutManager_->AddTimeoutTask(session.sessionId, pid, timeoutContext,
                static_cast<uint32_t>(timeoutMs));
        }
    }

    if (options.background) {
        auto record = std::make_shared<SessionRecord>();
        if (record == nullptr) {
            close(stdoutPipe[0]);
            close(stderrPipe[0]);
            kill(pid, SIGKILL);
            if (timeoutManager_ != nullptr) {
                timeoutManager_->CancelTimeoutTask(session.sessionId);
            }
            session.status = "failed";
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "ExecTool failed: alloc SessionRecord for background sessionId='%{public}s'", session.sessionId.c_str());
            return ERR_CLI_MEMORY_ALLOC_FAILED;
        }

        record->sessionId = session.sessionId;
        record->toolName = name;
        record->processId = pid;
        record->startTime = startTimeMs;
        record->timeoutMs = timeoutMs;
        record->SetState(SessionState::RUNNING);
        AddSessionRecord(record);

        if (ioMonitor_.RegisterSession(record->sessionId, stdoutPipe[0], stderrPipe[0]) != ERR_OK) {
            close(stdoutPipe[0]);
            close(stderrPipe[0]);
            kill(pid, SIGKILL);
            if (timeoutManager_ != nullptr) {
                timeoutManager_->CancelTimeoutTask(session.sessionId);
            }
            RemoveSessionRecord(record->sessionId);
            session.status = "failed";
            session.result->errorText = "Failed to register IO monitor";
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "ExecTool failed: ioMonitor registration for sessionId='%{public}s', pid=%{public}d",
                record->sessionId.c_str(), pid);
            return ERR_CLI_IO_MONITOR_REGISTER_FAILED;
        }
        if (processMonitor_.RegisterSession(pid, record->sessionId, startTimeMs, timeoutMs) != ERR_OK) {
            ioMonitor_.UnregisterSession(record->sessionId);
            kill(pid, SIGKILL);
            if (timeoutManager_ != nullptr) {
                timeoutManager_->CancelTimeoutTask(session.sessionId);
            }
            RemoveSessionRecord(record->sessionId);
            session.status = "failed";
            session.result->errorText = "Failed to register process monitor";
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "ExecTool failed: processMonitor registration for sessionId='%{public}s', pid=%{public}d",
                record->sessionId.c_str(), pid);
            return ERR_CLI_PROCESS_MONITOR_REGISTER_FAILED;
        }

        session.result = nullptr;
        LOG_TIMESTAMP("Service", "ExecTool END (background task started)");
        return ERR_OK;
    }

    std::string stdoutOutput;
    std::string stderrOutput;
    char buffer[4096];
    ssize_t bytesRead = 0;
    while ((bytesRead = read(stdoutPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        stdoutOutput += buffer;
    }
    while ((bytesRead = read(stderrPipe[0], buffer, sizeof(buffer) - 1)) > 0) {
        buffer[bytesRead] = '\0';
        stderrOutput += buffer;
    }
    close(stdoutPipe[0]);
    close(stderrPipe[0]);

    int status = 0;
    pid_t waitResult = waitpid(pid, &status, 0);
    if (timeoutManager_ != nullptr) {
        timeoutManager_->CancelTimeoutTask(session.sessionId);
    }
    if (waitResult == -1) {
        session.status = "failed";
        session.result->errorText = "waitpid failed: " + std::string(strerror(errno));
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ExecTool failed: waitpid for sessionId='%{public}s', pid=%{public}d: %{public}s",
            session.sessionId.c_str(), pid, strerror(errno));
        return ERR_CLI_WAITPID_FAILED;
    }

    auto endTimePoint = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTimePoint - startTimePoint);
    int64_t endTimeMs = std::chrono::duration_cast<std::chrono::milliseconds>(
        endTimePoint.time_since_epoch()).count();
    session.result->exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
    session.result->outputText = stdoutOutput;
    session.result->errorText = stderrOutput;
    session.result->signalNumber = WIFSIGNALED(status) ? WTERMSIG(status) : 0;
    session.result->timedOut = timeoutContext != nullptr && timeoutContext->isTimedOut.load();
    if (session.result->timedOut) {
        session.result->errorText = "Execution timed out";
    }
    session.result->executionTime = duration.count();
    session.status = (!session.result->timedOut && session.result->exitCode == 0) ? "completed" : "failed";
    session.endTime = endTimeMs;
    LOG_TIMESTAMP("Service", "ExecTool END (foreground task completed)");
    return ERR_OK;
}

int32_t CliManagerService::CancelSession(const std::string &sessionId)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CancelSession failed: sessionId='%{public}s' not found", sessionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }
    if (record->GetState() != SessionState::RUNNING && record->GetState() != SessionState::SPAWNING) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "CancelSession failed: sessionId='%{public}s' state=%{public}d is not cancellable",
            sessionId.c_str(), static_cast<int32_t>(record->GetState()));
        return ERR_CLI_SESSION_NOT_RUNNING;
    }
    record->SetState(SessionState::CANCELLING);
    if (record->processId > 0 && kill(record->processId, SIGTERM) != 0 && errno != ESRCH) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "CancelSession SIGTERM failed: %{public}s", strerror(errno));
    }
    return ERR_OK;
}

int32_t CliManagerService::RegisterScheduler(const sptr<ICliManagerScheduler> &scheduler)
{
    return EventDispatcher::GetInstance().RegisterScheduler(IPCSkeleton::GetCallingPid(), scheduler);
}

int32_t CliManagerService::UnregisterScheduler()
{
    return EventDispatcher::GetInstance().UnregisterScheduler(IPCSkeleton::GetCallingPid());
}

int32_t CliManagerService::SubscribeSession(const std::string &sessionId, const std::string &subscriptionId)
{
    if (sessionId.empty() || subscriptionId.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "SubscribeSession failed: invalid args sessionId='%{public}s', subscriptionId='%{public}s'",
            sessionId.c_str(), subscriptionId.c_str());
        return ERR_CLI_INVALID_SUBSCRIBER_PARAMS;
    }
    if (GetSessionRecord(sessionId) == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR,
            "SubscribeSession failed: sessionId='%{public}s' not found, subscriptionId='%{public}s'",
            sessionId.c_str(), subscriptionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }

    return EventDispatcher::GetInstance().RegisterSubscriber(sessionId, subscriptionId, IPCSkeleton::GetCallingPid());
}

int32_t CliManagerService::UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId)
{
    return EventDispatcher::GetInstance().UnregisterSubscriber(sessionId, subscriptionId, IPCSkeleton::GetCallingPid());
}

int32_t CliManagerService::QuerySessionStatus(const std::string &sessionId, SessionStatus &status)
{
    auto record = GetSessionRecord(sessionId);
    if (record == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "QuerySessionStatus failed: sessionId='%{public}s' not found", sessionId.c_str());
        return ERR_CLI_SESSION_NOT_FOUND;
    }

    status.sessionId = record->sessionId;
    status.toolName = record->toolName;
    if (!record->HasProcessExited() || !record->OutputDrained()) {
        status.status = "running";
        status.result = nullptr;
    } else {
        auto result = record->BuildExecResult();
        status.result = result;
        status.status = (result != nullptr && result->exitCode == 0 && !result->timedOut) ? "completed" : "failed";
    }
    status.startTime = record->startTime;
    status.endTime = record->GetEndTimeMs();
    return ERR_OK;
}

std::string CliManagerService::GenerateSandBoxConfig(const std::string &challenge)
{
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    uint64_t callingTokenId = IPCSkeleton::GetCallingFullTokenID();
    int32_t gid = callingUid;

    std::vector<int32_t> groupsVec;
    groupsVec.push_back(callingUid);

    nlohmann::json config;
    config["uid"] = callingUid;
    config["gid"] = gid;
    config["groups"] = groupsVec;
    config["callerTokenId"] = callingTokenId;
    config["challenge"] = challenge;
    config["bundleName"] = "com.huawei.hmos.vassistant";
    config["permissions"] = std::vector<std::string> {};
    return config.dump();
}

uint32_t CliManagerService::ResolveTimeout(const ExecOptions &options, const std::string &toolName)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ResolveTimeout: options.timeout=%{public}d, DEFAULT_TIMEOUT_SEC=%{public}d",
        options.timeout, DEFAULT_TIMEOUT_SEC);

    // Priority 1: Use options.timeout if set, capped at DEFAULT_TIMEOUT_SEC
    if (options.timeout > 0) {
        int32_t effectiveTimeout = (options.timeout > DEFAULT_TIMEOUT_SEC) ?
            DEFAULT_TIMEOUT_SEC : options.timeout;
        uint32_t result = static_cast<uint32_t>(effectiveTimeout) * 1000;
        if (options.timeout > DEFAULT_TIMEOUT_SEC) {
            TAG_LOGW(AAFwkTag::ABILITYMGR,
                "ResolveTimeout: options.timeout=%{public}d sec exceeds max, capped to %{public}d sec",
                options.timeout, DEFAULT_TIMEOUT_SEC);
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ResolveTimeout: using options timeout: %{public}u ms (%{public}d sec)",
            result, effectiveTimeout);
        return result;
    }

    // Priority 2: Use tool config timeout if available, capped at DEFAULT_TIMEOUT_SEC
    std::lock_guard<std::mutex> lock(mutex_);
    if (cliToolManager_ != nullptr && !toolName.empty()) {
        int32_t toolTimeout = 0;
        int32_t ret = cliToolManager_->GetToolTimeout(toolName, toolTimeout);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "ResolveTimeout: GetToolTimeout returned %{public}d, toolTimeout=%{public}d",
            ret, toolTimeout);
        if (ret == ERR_OK && toolTimeout > 0) {
            int32_t effectiveTimeout = (toolTimeout > DEFAULT_TIMEOUT_SEC) ?
                DEFAULT_TIMEOUT_SEC : toolTimeout;
            uint32_t result = static_cast<uint32_t>(effectiveTimeout) * 1000;
            if (toolTimeout > DEFAULT_TIMEOUT_SEC) {
                TAG_LOGW(AAFwkTag::ABILITYMGR,
                    "ResolveTimeout: tool timeout=%{public}d sec exceeds max, capped to %{public}d sec",
                    toolTimeout, DEFAULT_TIMEOUT_SEC);
            }
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ResolveTimeout: using tool timeout: %{public}u ms (%{public}d sec)",
                result, effectiveTimeout);
            return result;
        }
    }

    // Priority 3: Use default timeout
    uint32_t result = DEFAULT_TIMEOUT_SEC * 1000;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ResolveTimeout: using default timeout: %{public}u ms (%{public}d sec)",
        result, DEFAULT_TIMEOUT_SEC);
    return result;
}
} // namespace AbilityRuntime
} // namespace OHOS
