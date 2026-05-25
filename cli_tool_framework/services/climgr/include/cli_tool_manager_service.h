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
#include <string>
#include <unordered_map>
#include <vector>

#include "cli_tool_manager_stub.h"
#include "cli_tool_data_manager.h"
#include "ffrt.h"
#include "iremote_object.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include "io_monitor.h"
#include "process_manager.h"
#include "session_record.h"
#include "skill/skill_execute_callback_stub.h"
#include "dstb_types.h"

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
     * @brief Register a CLI tool
     */
    int32_t RegisterTool(const ToolInfo &tool) override;

    /**
     * @brief Execute a CLI tool with key-value pairs (convenience method).
     * @param param The CLI tool param.
     * @param objectCallback The callback RemoteObject.
     * @return Returns ERR_OK on success, error code otherwise.
     */
    int32_t ExecTool(const ExecToolParam &param, const std::string &eventId,
        const sptr<ICliToolManagerScheduler> &scheduler) override;

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

    // ============== 分布式接口 ==============
    /**
     * @brief Get available devices for distributed CLI tool
     * @param devices Output vector of DstbDeviceInfo
     * @return ERR_OK on success
     */
    int32_t GetAvailableDevices(std::vector<DstbDeviceInfo> &devices) override;

    /**
     * @brief Get tool summaries from a remote device
     * @param targetDeviceId Target device network ID
     * @param summaries Output vector of ToolSummary
     * @return ERR_OK on success
     */
    int32_t GetRemoteToolSummaries(const std::string &targetDeviceId,
        std::vector<ToolSummary> &summaries) override;

    /**
     * @brief Get tool info by name from a remote device
     * @param targetDeviceId Target device network ID
     * @param name Tool name
     * @param tool Output ToolInfo
     * @return ERR_OK on success
     */
    int32_t GetRemoteToolInfoByName(const std::string &targetDeviceId,
        const std::string &name, ToolInfo &tool) override;

    /**
     * @brief Handle remote request: get all tool summaries
     * @param callerInfo Remote caller information
     * @param summaries Output vector of ToolSummary
     * @return ERR_OK on success
     */
    int32_t GetAllToolSummariesFromRemote(const DstbCallerInfo &callerInfo,
        std::vector<ToolSummary> &summaries) override;

    /**
     * @brief Handle remote request: get tool info by name
     * @param callerInfo Remote caller information
     * @param name Tool name
     * @param tool Output ToolInfo
     * @return ERR_OK on success
     */
    int32_t GetToolInfoByNameFromRemote(const DstbCallerInfo &callerInfo,
        const std::string &name, ToolInfo &tool) override;

    /**
     * @brief Execute tool on a remote device
     * @param targetDeviceId Target device network ID
     * @param param Tool execution parameters
     * @param eventId Event ID for callback
     * @param scheduler Callback scheduler
     * @return ERR_OK on success
     */
    int32_t ExecToolRemote(const std::string &targetDeviceId,
        const ExecToolParam &param, const std::string &eventId,
        const sptr<ICliToolManagerScheduler> &scheduler) override;

    /**
     * @brief Handle remote request: execute tool
     * @param callerInfo Remote caller information
     * @param param Tool execution parameters
     * @param eventId Event ID for callback
     * @param sessionId Output session ID
     * @return ERR_OK on success
     */
    int32_t ExecToolFromRemote(const DstbCallerInfo &callerInfo,
        const ExecToolParam &param, const std::string &eventId,
        std::string &sessionId) override;

    /**
     * @brief Handle remote exec result notification (called by remote device)
     * @param sourceDeviceId The device that initiated the exec
     * @param sessionId Session ID
     * @param resultCode Execution result code
     * @param session Session info
     * @return ERR_OK on success
     */
    int32_t NotifyRemoteExecResult(const std::string &sourceDeviceId, const std::string &sessionId,
        int32_t resultCode, const CliSessionInfo &session) override;

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

    void Init();
    void DelayUnloadTask();

    std::shared_ptr<SessionRecord> CreateSessionRecord(const ExecToolParam &param, const std::string &eventId);
    void AddSessionRecord(const std::shared_ptr<SessionRecord> &record);
    std::shared_ptr<SessionRecord> GetSessionRecord(const std::string &sessionId);
    std::vector<std::shared_ptr<SessionRecord>> GetSessionRecords();
    void RemoveSessionRecord(const std::string &sessionId);
    bool IsSessionOwner(const std::shared_ptr<SessionRecord> &record, const char *action) const;

    bool RegisterSessionWithMonitors(const std::shared_ptr<SessionRecord> &record, const ExecToolParam &param);
    void UnregisterSessionWithMonitors(const std::string &sessionId);

    int32_t ValidateExecToolPermissions();
    int32_t ValidateSessionLimit();
    int32_t ValidateAndPrepareTool(const ExecToolParam &param, uint32_t tokenId,
        ToolInfo &toolInfo, std::string &sandboxConfig, std::string &bundleName);
    int32_t SetupAndStartSession(const ExecToolParam &param, const std::string &eventId,
        const ToolInfo &toolInfo, const std::string &sandboxConfig, const std::string &bundleName);

    int32_t TryDispatchSkillSession(const ExecToolParam &param,
        const std::string &eventId, const ToolInfo &toolInfo, bool &dispatched);
    int32_t SetupAndStartSkillSession(const ExecToolParam &param,
        const std::string &eventId, const ToolInfo &toolInfo);
    int32_t ValidateSkillTypeFromParam(const ExecToolParam &param, int32_t &skillType);
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

    // ============== 分布式相关私有方法 ==============
    void InitDistributed();
    void UnInitDistributed();
    bool CheckDstbCallerPermission(const DstbCallerInfo &callerInfo);
    void NotifyRemoteCaller(const std::string &sourceDeviceId, const std::string &sessionId,
        int32_t resultCode, const CliSessionInfo &session);

    DISALLOW_COPY_AND_MOVE(CliToolManagerService);

private:
    static sptr<CliToolManagerService> instance_;

    static void sigchld_handler(int32_t sig);

    void PostExecToolTask(int64_t time, const std::string &sessionId, bool isTimeout);
    void WaitPid(pid_t pid, int32_t status, int32_t sig);
    void RegisterAppStateObserver(const std::string &bundleName, pid_t callerPid);
    void OnProcessDied(const std::string &bundleName, pid_t diedPid);

    bool initialized_ = false;
    std::shared_ptr<IOMonitor> ioMonitor_ = nullptr;

    std::atomic<int32_t> interfaceCalledCount_ = 0;
    ffrt::mutex sessionsMutex_;
    std::unordered_map<std::string, std::shared_ptr<SessionRecord>> sessionRecords_;
    ffrt::mutex observerMutex_;
    std::unordered_map<std::string, sptr<AppExecFwk::IApplicationStateObserver>> bundleObservers_;
};

class SkillCallbackAdapter : public AAFwk::SkillExecuteCallbackStub {
public:
    SkillCallbackAdapter(const std::string &sessionId,
        int32_t callerPid, int32_t callerUid, const std::string &eventId);
    void OnExecuteDone(const std::string &requestCode, int32_t resultCode,
        const AppExecFwk::SkillExecuteResult &result) override;

private:
    std::string sessionId_;
    int32_t callerPid_;
    int32_t callerUid_;
    std::string eventId_;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_SERVICE_H
