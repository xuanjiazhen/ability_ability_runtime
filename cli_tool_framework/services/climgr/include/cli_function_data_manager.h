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

#ifndef OHOS_ABILITY_RUNTIME_CLI_FUNCTION_DATA_MANAGER_H
#define OHOS_ABILITY_RUNTIME_CLI_FUNCTION_DATA_MANAGER_H

#include <mutex>
#include <string>
#include <vector>

#include "function_info.h"
#include "distributed_kv_data_manager.h"
#include "nocopyable.h"

namespace OHOS {
namespace CliTool {

class CliFunctionDataManager {
public:
    /**
     * @brief Get singleton instance
     * @return CliFunctionDataManager& Reference to singleton instance
     */
    static CliFunctionDataManager &GetInstance();

    /**
     * @brief Register a function to database
     * @param function FunctionInfo to register
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t RegisterFunction(const FunctionInfo &function);

    /**
     * @brief Get function by bundleName and functionName from KVStore
     * @param bundleName Bundle name
     * @param functionName Function name
     * @param function Output FunctionInfo
     * @return int32_t ERR_OK if found, error code otherwise
     */
    int32_t GetFunctionByName(const std::string &bundleName, const std::string &functionName, FunctionInfo &function);

private:
    CliFunctionDataManager();
    ~CliFunctionDataManager();
    DISALLOW_COPY_AND_MOVE(CliFunctionDataManager);

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
     * @brief Store a single function in KVStore (internal use, caller must hold lock)
     * @param function FunctionInfo to store
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t StoreFunction(const FunctionInfo &function);

    /**
     * @brief Restore KVStore if corrupted
     * @param status The status code from KVStore operation
     * @return DistributedKv::Status The final status after restoration
     */
    DistributedKv::Status RestoreKvStore(DistributedKv::Status status);

    /**
     * @brief Generate KVStore key from bundleName and functionName
     * @param bundleName Bundle name
     * @param functionName Function name
     * @return std::string Generated key string
     */
    static std::string GenerateFunctionKey(const std::string &bundleName, const std::string &functionName);

    DistributedKv::DistributedKvDataManager dataManager_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;

    // KVStore configuration constants
    static constexpr const char* KV_STORE_APP_ID = "cli_functions_db";
    static constexpr const char* KV_STORE_STORE_ID = "cli_functions_store";
    static constexpr const char* STORAGE_DIR = "/data/service/el1/public/database/aimgr/cli_function";
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_FUNCTION_DATA_MANAGER_H
