/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_MANAGER_H

#include <memory>
#include <mutex>
#include <vector>

#include "cli_manager_data.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

/**
 * @brief CliToolManager manages CLI tool registration, querying and validation
 * Similar to AppExitReasonHelper pattern
 */
class CliToolManager {
public:
    CliToolManager();
    ~CliToolManager();

    /**
     * @brief Check if tool list is loaded
     * @return bool true if loaded
     */
    bool IsToolListLoaded();

    /**
     * @brief Load tools from specified registry file
     * @param filePath Path to cli_tools.json
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t LoadToolsFromRegistry(const std::string &filePath);

    /**
     * @brief Query all available tools
     * @param tools Output vector of ToolInfo
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t QueryTools(std::vector<ToolInfo> &tools);

    /**
     * @brief Query tool by name
     * @param name Tool name
     * @param tool Output ToolInfo
     * @return int32_t ERR_OK if found, error code otherwise
     */
    int32_t QueryToolByName(const std::string &name, ToolInfo &tool);

    /**
     * @brief Query tools filtered by permission
     * @param permission Permission string
     * @param tools Output vector of ToolInfo
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t QueryToolsByPermission(const std::string &permission, std::vector<ToolInfo> &tools);

    /**
     * @brief Validate if tool exists and is properly configured
     * @param name Tool name
     * @return bool true if valid
     */
    bool ValidateTool(const std::string &name);

    /**
     * @brief Get tool timeout value
     * @param name Tool name
     * @param timeout Output timeout in milliseconds
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t GetToolTimeout(const std::string &name, int32_t &timeout);

private:
    static constexpr const char* DEFAULT_REGISTRY_PATH = "/etc/cli/cli_tools.json";

    std::mutex mutex_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_MANAGER_H

