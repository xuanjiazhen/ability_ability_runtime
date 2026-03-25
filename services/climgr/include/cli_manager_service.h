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
#include <singleton.h>
#include "cli_manager_data.h"
#include "cli_manager_stub.h"
#include "system_ability.h"
#include "event_handler.h"

namespace OHOS {
namespace AbilityRuntime {

/**
 * @class CliManagerService
 * CliManagerService is the system service for CLI tool management
 */
class CliManagerService : public SystemAbility, public CliManagerStub {
    DECLARE_DELAYED_SINGLETON(CliManagerService);
    DECLARE_SYSTEM_ABILITY(CliManagerService);

public:
    /**
     * @brief Constructor
     */
    CliManagerService(int32_t systemAbilityId, bool runOnCreate = false);

    /**
     * @brief Destructor
     */
    virtual ~CliManagerService() = default;

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
                     const std::map<std::string, std::string> &args,
                     const ExecOptions &options,
                     ExecResult &result) override;

private:
    bool Init();
    void LoadToolsFromConfig();

    std::mutex mutex_;
    bool serviceStarted_ = false;
    std::vector<ToolInfo> tools_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_MANAGER_SERVICE_H
