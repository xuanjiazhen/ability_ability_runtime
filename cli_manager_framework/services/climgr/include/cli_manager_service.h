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
#include "cli_manager_data.h"
#include "cli_manager_stub.h"
#include "cli_tool_manager.h"
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
     * @brief Wrap execution arguments with claw_sandbox
     * @param execArgs Original execution arguments (char* array with nullptr terminator)
     * @param challenge Challenge string for sandbox config generation
     * @return Wrapped execution arguments ready for execvp
     */
    std::vector<char*> WrapSandBoxExecArgs(const std::vector<char*>& execArgs, const std::string& challenge);

    DISALLOW_COPY_AND_MOVE(CliManagerService);

private:
    static sptr<CliManagerService> instance_;
    static std::mutex instanceMutex_;
    std::mutex mutex_;
    ServiceRunningState state_ = ServiceRunningState::STATE_NOT_START;
    std::unique_ptr<CliToolManager> cliToolManager_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_MANAGER_SERVICE_H
