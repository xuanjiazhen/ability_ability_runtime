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

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_H

#include <mutex>
#include <string>
#include <vector>

#include "cli_manager_data.h"
#include "distributed_kv_data_manager.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {

class CliToolDataManager : public DelayedSingleton<CliToolDataManager> {
public:
    CliToolDataManager();
    virtual ~CliToolDataManager();

    /**
     * @brief Load tools from JSON file and store in KVStore
     * @param filePath Path to cli_tools.json
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t LoadToolsFromFile(const std::string &filePath);

    /**
     * @brief Get all tools from KVStore
     * @param tools Output vector of ToolInfo
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t GetAllTools(std::vector<ToolInfo> &tools);

    /**
     * @brief Get tool by name from KVStore
     * @param name Tool name
     * @param tool Output ToolInfo
     * @return int32_t ERR_OK if found, error code otherwise
     */
    int32_t GetToolByName(const std::string &name, ToolInfo &tool);

    /**
     * @brief Get tools by permission filter
     * @param permission Permission string
     * @param tools Output vector of ToolInfo
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t GetToolsByPermission(const std::string &permission, std::vector<ToolInfo> &tools);

    /**
     * @brief Check if tool exists
     * @param name Tool name
     * @return bool true if exists
     */
    bool ToolExists(const std::string &name);

private:
    /**
     * @brief Get or create KVStore
     * @return DistributedKv::Status
     */
    DistributedKv::Status GetKvStore();

    /**
     * @brief Check if KVStore is available
     * @return bool true if ready
     */
    bool CheckKvStore();

    /**
     * @brief Parse JSON file to ToolInfo list
     * @param filePath Path to JSON file
     * @param tools Output vector of ToolInfo
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t ParseJsonFile(const std::string &filePath, std::vector<ToolInfo> &tools);

    /**
     * @brief Convert ToolInfo to JSON string
     * @param tool Input ToolInfo
     * @return std::string JSON string
     */
    std::string ToolInfoToJson(const ToolInfo &tool);

    /**
     * @brief Convert JSON string to ToolInfo
     * @param jsonStr Input JSON string
     * @param tool Output ToolInfo
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t JsonToToolInfo(const std::string &jsonStr, ToolInfo &tool);

    /**
     * @brief Convert vector to JSON array string
     * @param tools Input ToolInfo vector
     * @return std::string JSON array string
     */
    std::string ToolsToJsonArray(const std::vector<ToolInfo> &tools);

    /**
     * @brief Convert JSON array string to ToolInfo vector
     * @param jsonStr Input JSON array string
     * @param tools Output ToolInfo vector
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t JsonArrayToTools(const std::string &jsonStr, std::vector<ToolInfo> &tools);

    /**
     * @brief Store tool in KVStore
     * @param tool ToolInfo to store
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t StoreTool(const ToolInfo &tool);

    static constexpr const char* DEFAULT_REGISTRY_PATH = "/etc/cli/cli_tools.json";
    static constexpr const char* KV_STORE_APP_ID = "cli_tools_storage";
    static constexpr const char* KV_STORE_STORE_ID = "cli_tools_store";
    static constexpr const char* TOOLS_LIST_KEY = "all_tools_list";
    static constexpr const char* CLI_TOOLS_STORAGE_DIR = "/data/service/el1/public/database/cli_manager";

    const DistributedKv::AppId appId_ { KV_STORE_APP_ID };
    const DistributedKv::StoreId storeId_ { KV_STORE_STORE_ID };
    DistributedKv::DistributedKvDataManager dataManager_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_DATA_MANAGER_H
