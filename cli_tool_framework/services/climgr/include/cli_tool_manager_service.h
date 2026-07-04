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

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_SERVICE_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_SERVICE_H

#include <atomic>
#include <map>
#include <memory>
#include <signal.h>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "cli_tool_manager_stub.h"
#include "cli_tool_data_manager.h"
#include "exec_cmd_param.h"
#include "exec_options.h"
#include "ffrt.h"
#include "iremote_object.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include "io_monitor.h"
#include "process_manager.h"
#include "session_record.h"
#include "skill/skill_execute_callback_stub.h"

namespace OHOS {
namespace AppExecFwk {
class IApplicationStateObserver;
struct SkillExecuteResult;
}
namespace CliTool {
class SessionRecord;
class SkillCallbackAdapter;
class CliToolManagerService;

class CliToolManagerService : public SystemAbility,
                              public CliToolManagerStub {
    DECLARE_SYSTEM_ABILITY(CliToolManagerService);

    friend class SkillCallbackAdapter;

public:
    static sptr<CliToolManagerService> GetInstance();
    virtual ~CliToolManagerService() = default;

    /**
     * @brief Query all available tools
     */
    int32_t GetAllToolInfos(ToolsRawData &tools) override;

    /**
     * @brief Query tool summaries (lightweight for listing)
     */
    int32_t GetAllToolSummaries(std::vector<ToolSummary> &summaries) override;

    /**
     * @brief Get tool information by name
     */
    int32_t GetToolInfoByName(const std::string &name, ToolInfo &tool) override;

    /**
     * @brief Execute a CLI tool with key-value pairs (convenience method).
     * @param param The CLI tool param.
     * @param objectCallback The callback RemoteObject.
     * @return Returns ERR_OK on success, error code otherwise.
     */
    int32_t ExecTool(const ExecToolParam &param, const std::string &eventId,
        const sptr<ICliToolManagerScheduler> &scheduler) override;

    int32_t ExecCmd(const ExecCmdParam &param, const std::string &eventId,
        const sptr<ICliToolManagerScheduler> &scheduler, const std::string &subscriptionId) override;

    int32_t ClearSession(const std::string &sessionId) override;
    int32_t SubscribeSession(const std::string &sessionId, const std::string &subscriptionId,
        const sptr<ICliToolManagerScheduler> &scheduler) override;
    int32_t UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId) override;
    
    int32_t QuerySession(const std::string &sessionId, CliSessionInfo &session) override;
    int32_t SendMessage(const std::string &sessionId,
        const std::string &inputText, const std::string &eventId,
        const sptr<ICliToolManagerScheduler> &scheduler) override;

    /**
     * @brief Batch query command permissions (Inner API, for SA only).
     */
    int32_t BatchQueryPermissionBySubCommand(const std::vector<Command> &cmds,
        std::vector<CommandPermission> &cmdPermissions) override;

    /**
     * @brief Register a function
     * @param function FunctionInfo to register
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t RegisterFunction(const FunctionInfo &function) override;

    /**
     * @brief Batch register functions
     * @param functions Vector of FunctionInfo to register
     * @param successCount Output count of successfully registered functions
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t BatchRegisterFunctions(const FunctionsRawData &functions, int32_t &successCount) override;

    /**
     * @brief Get function information by bundleName and functionName
     * @param bundleName Bundle name
     * @param functionName Function name
     * @param function Output FunctionInfo
     * @return int32_t ERR_OK if found, error code otherwise
     */
    int32_t GetFunctionInfo(const std::string &functionNamespace, const std::string &functionName,
        FunctionInfo &function) override;

    /**
     * @brief Unregister a function
     * @param functionNamespace Namespace
     * @param functionName Function name
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t UnregisterFunction(const std::string &functionNamespace, const std::string &functionName) override;

    /**
     * @brief Batch unregister intentFunctions by namespace
     * @param functionNamespace Namespace to delete all functions from
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t UnregisterIntentFunctionsByNamespace(const std::string &functionNamespace) override;

    /**
     * @brief Get all functions via raw data (for large data transfer)
     * @param functions Output FunctionsRawData
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t GetAllFunctions(FunctionsRawData &functions) override;

protected:
    void OnStart() override;
    void OnStop() override;
    int32_t OnIdle(const SystemAbilityOnDemandReason &idlReason) override;

private:
    CliToolManagerService() : SystemAbility(CLI_TOOL_MGR_SERVICE_ID, false) {};

    // RAII wrapper for interface call counter
    class InterfaceCallCounter {
    public:
        explicit InterfaceCallCounter(std::atomic<int32_t>& counter) : counter_(counter)
        {
            counter_.fetch_add(1, std::memory_order_acq_rel);
        }
        ~InterfaceCallCounter()
        {
            counter_.fetch_sub(1, std::memory_order_acq_rel);
        }
        InterfaceCallCounter(const InterfaceCallCounter&) = delete;
        InterfaceCallCounter& operator=(const InterfaceCallCounter&) = delete;
    private:
        std::atomic<int32_t>& counter_;
    };

    enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

    /**
     * @brief Initialize service-side monitors and callbacks.
     *
     * Sets up child process exit monitoring, installs the SIGCHLD handler, creates the I/O monitor, and binds
     * callbacks back to this service instance.
     *
     * @return true if all monitors are ready, false otherwise.
     */
    bool Init();

    /**
     * @brief Schedule delayed unloading of the CLI manager system ability.
     *
     * The service is unloaded only when no IPC call is active, no session remains, and no observed caller process
     * is still tracked. Otherwise a new delayed check is scheduled.
     */
    void DelayUnloadTask();

    std::shared_ptr<SessionRecord> CreateSessionRecord(const ExecToolParam &param, const std::string &eventId);
    void AddSessionRecord(const std::shared_ptr<SessionRecord> &record);
    std::shared_ptr<SessionRecord> GetSessionRecord(const std::string &sessionId);
    std::vector<std::shared_ptr<SessionRecord>> GetSessionRecords();
    void RemoveSessionRecord(const std::string &sessionId);
    bool IsSessionOwner(const std::shared_ptr<SessionRecord> &record, const char *action) const;

    bool RegisterSessionWithMonitors(const std::shared_ptr<SessionRecord> &record, const ExecOptions &options);
    void UnregisterSessionWithMonitors(const std::string &sessionId);

    int32_t ValidateExecToolPermissions();
    int32_t ValidateSessionLimit();
    int32_t ValidateAndPrepareTool(const ExecToolParam &param, uint32_t tokenId,
        ToolInfo &toolInfo, std::string &sandboxConfig, std::string &bundleName, std::string& detail);

    int32_t ValidateAndPrepareCmd(const ExecCmdParam &param, uint32_t tokenId,
        std::string &sandboxConfig, std::string &bundleName);
    int32_t SetupAndStartSession(const ExecToolParam &param, const std::string &eventId,
        const ToolInfo &toolInfo, const std::string &sandboxConfig, const std::string &bundleName);

    int32_t TryDispatchSkillSession(const ExecToolParam &param,
        const std::string &eventId, const ToolInfo &toolInfo, bool &dispatched);
    int32_t SetupAndStartSkillSession(const ExecToolParam &param,
        const std::string &eventId, const ToolInfo &toolInfo);
    int32_t ValidateSkillTypeFromParam(ExecToolParam &param, int32_t &skillType);
    int32_t ValidateSkillType(const std::string &bundleName,
        const std::string &moduleName, const std::string &skillName, int32_t &skillType);
    void HandleSkillSessionComplete(const std::string &sessionId, int32_t callerPid, int32_t callerUid,
        const std::string &eventId, int32_t resultCode, const CliSessionInfo &session);
    void HandleSkillSessionTimeout(const std::string &sessionId);

    void HandleBackgroundSessionReply(const std::shared_ptr<SessionRecord> &record, const std::string &eventId);
    void HandleProcessTimeout(const std::string &sessionId);
    void HandleProcessYieldTimeout(const std::string &sessionId);
    void HandleOutputClosed(const std::string &sessionId, bool isStdout);
    void HandleOutputDrained(const std::string &sessionId);
    void FinalizeBackgroundSession(const std::shared_ptr<SessionRecord> &record);

    /**
     * @brief Cancel and clean a session whose caller process has died.
     *
     * The caller is expected to remove the record from sessionRecords_ before invoking this function. This helper
     * kills the CLI child process group when one exists, unregisters monitor state, and dispatches the session exit
     * event to remaining subscribers.
     *
     * @param record Session record owned by the dead caller process.
     */
    void CleanupSessionAfterCallerDied(const std::shared_ptr<SessionRecord> &record);

    /**
     * @brief Start the child-exit notification thread.
     *
     * Creates a nonblocking, close-on-exec pipe used by the SIGCHLD handler and starts a thread that polls the read
     * side. The actual waitpid work is done on that thread instead of inside the signal handler.
     *
     * @return true if the monitor is already running or starts successfully, false otherwise.
     */
    bool StartChildExitMonitor();

    /**
     * @brief Stop child-exit monitoring and close notification pipe descriptors.
     */
    void StopChildExitMonitor();

    /**
     * @brief Install the service SIGCHLD handler.
     *
     * The handler only writes a notification byte to the monitor pipe and then chains to the previously installed
     * handler, keeping signal-context work minimal.
     *
     * @return true if the handler is already installed or installs successfully, false otherwise.
     */
    bool InstallSigChldHandler();

    /**
     * @brief Restore the SIGCHLD handler that was active before this service installed its handler.
     */
    void RestoreSigChldHandler();

    /**
     * @brief Make a child-exit notification pipe descriptor nonblocking and close-on-exec.
     *
     * @param fd Pipe descriptor to update.
     * @return true if both descriptor flags are set successfully, false otherwise.
     */
    bool SetChildExitNotifyFdFlags(int32_t fd);

    /**
     * @brief Poll the child-exit notification pipe and reap exited CLI child processes.
     *
     * This is the entry point for childExitMonitorThread_. It drains pending SIGCHLD notifications and delegates the
     * nonblocking waitpid work to ProcessExitedChildren().
     */
    void ProcessChildExitNotifications();

    /**
     * @brief Try to reap all active CLI child processes for a received child-exit signal.
     *
     * @param sig Signal associated with the exit notification.
     */
    void ProcessExitedChildren(int32_t sig);

    /**
     * @brief Snapshot active CLI child process IDs from current session records.
     *
     * @return Process IDs that still have a live session record.
     */
    std::vector<pid_t> GetActiveChildPids();

    /**
     * @brief Reap a single CLI child process without blocking.
     *
     * @param pid Child process ID to wait for.
     * @param sig Signal associated with the wait attempt.
     * @return true when no further wait is needed; false when the child is still running.
     */
    bool TryReapChild(pid_t pid, int32_t sig);

    /**
     * @brief Retry reaping active child processes during service shutdown.
     *
     * @param pids Snapshot of child process IDs to reap.
     * @param sig Signal associated with the shutdown wait attempts.
     */
    void ReapActiveChildren(const std::vector<pid_t> &pids, int32_t sig);

    /**
     * @brief Check whether a newly created child process exited before monitor registration completed.
     *
     * @param record Session record that owns the child process.
     */
    void ReapPendingChildExit(const std::shared_ptr<SessionRecord> &record);

    DISALLOW_COPY_AND_MOVE(CliToolManagerService);

private:
    static sptr<CliToolManagerService> instance_;

    /**
     * @brief SIGCHLD handler that wakes the child-exit monitor thread.
     *
     * The handler is intentionally limited to signal-safe work and chains to the previous SIGCHLD handler when one
     * existed.
     */
    static void sigchld_handler(int32_t sig, siginfo_t *info, void *context);

    void PostExecToolTask(int64_t time, const std::string &sessionId, bool isTimeout);
    void WaitPid(pid_t pid, int32_t status, int32_t sig);
    void RegisterAppStateObserver(const std::string &bundleName, pid_t callerPid);
    void OnProcessDied(const std::string &bundleName, pid_t diedPid);
    void AddCallerPid(int64_t pid);
    void RemoveCallerPid(int64_t pid);
    int32_t GetCallerPidCount();

    std::atomic<bool> initialized_ = false;
    std::shared_ptr<IOMonitor> ioMonitor_ = nullptr;
    int32_t childExitNotifyReadFd_ = -1;  // Read end polled by childExitMonitorThread_.
    int32_t childExitNotifyWriteFd_ = -1; // Write end used by the SIGCHLD handler.
    std::atomic<bool> childExitMonitorRunning_ = false;
    std::thread childExitMonitorThread_;
    struct sigaction oldSigChldAction_ {}; // Handler restored when the service stops.
    bool sigChldActionInstalled_ = false;

    std::atomic<int32_t> interfaceCalledCount_ = 0;
    ffrt::mutex sessionsMutex_;
    std::unordered_map<std::string, std::shared_ptr<SessionRecord>> sessionRecords_;
    ffrt::mutex observerMutex_;
    std::unordered_map<std::string, sptr<AppExecFwk::IApplicationStateObserver>> bundleObservers_;
    ffrt::mutex callerPidsMutex_;
    std::unordered_set<int64_t> callerPids;
};

class SkillCallbackAdapter : public AAFwk::SkillExecuteCallbackStub {
public:
    SkillCallbackAdapter(const std::string &sessionId,
        int32_t callerPid, int32_t callerUid, const std::string &eventId,
        wptr<CliToolManagerService> service);
    void OnExecuteDone(const std::string &requestCode, int32_t resultCode,
        const AppExecFwk::SkillExecuteResult &result) override;

private:
    std::string sessionId_;
    int32_t callerPid_;
    int32_t callerUid_;
    std::string eventId_;
    wptr<CliToolManagerService> service_;
};
} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_SERVICE_H
