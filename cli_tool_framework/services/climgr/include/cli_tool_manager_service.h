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

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cli_tool_manager_stub.h"
#include "cli_tool_data_manager.h"
#include "iremote_object.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include "io_monitor.h"
#include "process_manager.h"
#include "process_monitor.h"
#include "session_record.h"
#include "timeout_manager.h"

namespace OHOS {
namespace CliTool {
class CliToolManagerService : public SystemAbility,
                        public CliToolManagerStub,
                        public std::enable_shared_from_this<CliToolManagerService> {
    DECLARE_SYSTEM_ABILITY(CliToolManagerService);

public:
    static sptr<CliToolManagerService> GetInstance();
    virtual ~CliToolManagerService() = default;

    /**
     * @brief Query all available tools
     */
    int32_t GetAllToolInfos(std::vector<ToolInfo> &tools) override;

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

    int32_t RegisterScheduler(const sptr<IRemoteObject> &scheduler) override;

    int32_t UnregisterScheduler() override;

    /**
     * @brief Execute a CLI tool.
     * Implements the ICliToolManager interface method.
     */
    int32_t ExecTool(const ExecToolParam &param,
                     const std::map<std::string, std::string> &args,
                     const std::string &eventId) override;

    int32_t ClearSession(const std::string &sessionId) override;
    int32_t SubscribeSession(const std::string &sessionId, const std::string &subscriptionId) override;
    int32_t UnsubscribeSession(const std::string &sessionId, const std::string &subscriptionId) override;
    
    int32_t QuerySession(const std::string &sessionId, CliSessionInfo &session) override;
    int32_t SendMessage(const std::string &sessionId,
                        const std::string &inputText,
                        const std::string &eventId) override;

protected:
    void OnStart() override;
    void OnStop() override;

private:
    CliToolManagerService() : SystemAbility(CLI_TOOL_MGR_SERVICE_ID, false) {};

    enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

    void Init();
    int32_t PreExecuteValidation(const ExecToolParam &param);

    void AddSessionRecord(const std::shared_ptr<SessionRecord> &record);
    std::shared_ptr<SessionRecord> GetSessionRecord(const std::string &sessionId);
    void RemoveSessionRecord(const std::string &sessionId);

    bool RegisterSessionWithMonitors(const std::shared_ptr<SessionRecord> &record,
                                     bool background,
                                     int32_t yieldMs);
    void UnregisterSessionWithMonitors(const std::string &sessionId);

    void HandleProcessTimeout(const std::string &sessionId);
    void HandleProcessYieldTimeout(const std::string &sessionId);
    void HandleProcessExit(const std::string &sessionId, int status);
    void HandleOutputClosed(const std::string &sessionId, bool isStdout);
    void HandleOutputDrained(const std::string &sessionId);
    void FinalizeBackgroundSession(const std::shared_ptr<SessionRecord> &record);

    DISALLOW_COPY_AND_MOVE(CliToolManagerService);

private:
    bool initialized_ = false;

    std::mutex sessionsMutex_;
    std::unordered_map<std::string, std::shared_ptr<SessionRecord>> sessionRecords_;

    std::shared_ptr<TimeoutManager> timeoutManager_ = nullptr;
    std::shared_ptr<IOMonitor> ioMonitor_ = nullptr;
    ProcessMonitor processMonitor_;

    static sptr<CliToolManagerService> instance_;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_MGR_SERVICE_H
