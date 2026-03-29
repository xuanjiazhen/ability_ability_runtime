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

#ifndef OHOS_ABILITY_RUNTIME_ICLI_MANAGER_H
#define OHOS_ABILITY_RUNTIME_ICLI_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <utility>
#include "cli_manager_data.h"
#include "iremote_broker.h"

namespace OHOS {
namespace AbilityRuntime {

/**
 * @enum CliManagerCmd
 * CLI Manager IPC command codes
 */
enum class CliManagerCmd {
    // IPC command IDs
    QUERY_TOOLS = 0,
    EXEC_TOOL,
    CMD_MAX
};

/**
 * @class ICliManager
 * ICliManager is used to manage CLI tools execution and query
 */
class ICliManager : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.abilityruntime.climanager");

    /**
     * @brief Query all available tools
     * @param tools Output vector of tool information
     * @return Returns ERR_OK on success, others on failure
     */
    virtual int32_t QueryTools(std::vector<ToolInfo> &tools) = 0;

    /**
     * @brief Execute tool
     * @param name Tool name
     * @param args Arguments as key-value pairs (preserves insertion order)
     * @param options Execution options
     * @param result Output execution result
     * @return Returns ERR_OK on success, others on failure
     */
    virtual int32_t ExecTool(const std::string &name,
                             const std::vector<std::pair<std::string, std::string>> &args,
                             const ExecOptions &options,
                             ExecResult &result) = 0;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_ICLI_MANAGER_H
