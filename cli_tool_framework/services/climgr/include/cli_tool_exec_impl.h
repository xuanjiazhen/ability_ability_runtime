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

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_EXEC_IMPL_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_EXEC_IMPL_H

#include <string>
#include <vector>
#include <memory>

#include "cli_tool_types.h"

namespace OHOS {
namespace CliTool {

/**
 * @class CliToolExecutor
 * @brief Handles execution of CLI tools.
 *
 * This class provides methods to execute CLI tools with proper
 * resource management, timeout handling, and output capture.
 */
class CliToolExecutor {
public:
    CliToolExecutor() = default;
    ~CliToolExecutor() = default;

    /**
     * @brief Execute a CLI tool.
     *
     * @param cliName The CLI tool name.
     * @param args Command line arguments.
     * @param options Execution options.
     * @param session Output session information.
     * @return Returns 0 on success, error code otherwise.
     */
    int32_t Execute(const std::string &cliName,
        const std::vector<std::string> &args,
        const ExecOptions &options,
        SessionInfo &session);

private:
    /**
     * @brief Build command line arguments.
     *
     * @param cliName The CLI tool name.
     * @param args The arguments.
     * @return Returns the complete command line.
     */
    std::string BuildCommandLine(const std::string &cliName,
        const std::vector<std::string> &args);

    /**
     * @brief Validate the CLI tool name.
     *
     * @param cliName The CLI tool name.
     * @return Returns true if valid, false otherwise.
     */
    bool ValidateCliTool(const std::string &cliName);

    /**
     * @brief Check if the caller has permission to execute the tool.
     *
     * @param cliName The CLI tool name.
     * @return Returns true if permitted, false otherwise.
     */
    bool CheckPermission(const std::string &cliName);

    /**
     * @brief Generate a unique session ID.
     *
     * @return Returns the session ID.
     */
    std::string GenerateSessionId();

    DISALLOW_COPY_AND_MOVE(CliToolExecutor);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_EXEC_IMPL_H
