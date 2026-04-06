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

#ifndef OHOS_ABILITY_RUNTIME_CLI_MANAGER_SERVICE_H
#define OHOS_ABILITY_RUNTIME_CLI_MANAGER_SERVICE_H

#include <memory>
#include <mutex>
#include <utility>
#include <unordered_map>
#include "cli_manager_data.h"
#include "cli_manager_stub.h"
#include "cli_tool_manager.h"
#include "timeout_manager.h"
#include "system_ability.h"

namespace OHOS {
namespace AbilityRuntime {

enum class ServiceRunningState { STATE_NOT_START, STATE_RUNNING };

/**
 * @class CliManagerService
 * CliManagerService is the system service for CLI tool management
 */
class CliManagerService : public SystemAbility,
                          public CliManagerStub,
                          public std::enable_shared_from_this<CliManagerService> {
    DECLEAR_SYSTEM_ABILITY(CliManagerService);

public:
    /**
     * @brief Get singleton instance
     */
    static sptr<CliManagerService> GetInstance();

    /**
     * @brief Constructor
     */
    CliManagerService();

    /**
     * @brief Destructor
     */
    ~CliManagerService();

    /**
     * @brief System ability OnStart
     */
    void OnStart() override;

    /**
     * @brief System ability OnStop
     */
    void OnStop() override;

    /**
     * @brief Query all available tools
     */
    int32_t QueryTools(std::vector<ToolInfo> &tools) override;

    /**
     * @brief Execute tool
     */
    int32_t ExecTool(const std::string &name,
                     const std::vector<std::pair<std::string, std::string>> &args,
                     const ExecOptions &options,
                     const std::string &challenge,
                     SessionInfo &session) override;

private:
    void Init();

    /**
     * @brief Generate sandbox configuration JSON
     * @param challenge Challenge string
     * @return JSON string with sandbox configuration
     */
    std::string GenerateSandBoxConfig(const std::string& challenge);

    /**
     * @brief Create session record object
     * @param sessionId Session ID
     * @param toolName Tool name
     * @param challenge Challenge string
     * @return Shared pointer to session record
     */
    std::shared_ptr<SessionRecord> CreateSessionRecord(
        const std::string& sessionId,
        const std::string& toolName,
        const std::string& challenge);

    /**
     * @brief Generate unique session ID using nanosecond time
     * @return Unique session ID string
     */
    std::string GenerateSessionId();

    /**
     * @brief Build SessionInfo from SessionRecord
     * @param record Session record
     * @return Session info for client
     */
    SessionInfo BuildSessionInfo(const std::shared_ptr<SessionRecord>& record);

    /**
     * @brief Resolve timeout with priority
     * @param options Exec options
     * @param toolName Tool name
     * @return Timeout in milliseconds
     */
    uint32_t ResolveTimeout(const ExecOptions& options, const std::string& toolName);

    DISALLOW_COPY_AND_MOVE(CliManagerService);

private:
    static constexpr size_t MAX_SESSION_COUNT = 64;  // Maximum concurrent sessions

private:
    static sptr<CliManagerService> instance_;
    static std::mutex instanceMutex_;
    std::mutex mutex_;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
    std::shared_ptr<CliToolManager> cliToolManager_;
    std::shared_ptr<TimeoutManager> timeoutManager_;

    // Session management: sessionId -> SessionRecord
    std::unordered_map<std::string, std::shared_ptr<SessionRecord>> sessionRecords_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_MANAGER_SERVICE_H
