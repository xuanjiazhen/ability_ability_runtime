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
     * @brief Batch register functions to database
     * @param functions Vector of FunctionInfo to register
     * @param successCount Output count of successfully registered functions
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t BatchRegisterFunctions(const std::vector<FunctionInfo> &functions, int32_t &successCount);

    /**
     * @brief Get function by namespace and functionName from KVStore
     * @param functionNamespace Namespace
     * @param functionName Function name
     * @param function Output FunctionInfo
     * @return int32_t ERR_OK if found, error code otherwise
     */
    int32_t GetFunctionByName(const std::string &functionNamespace, const std::string &functionName,
        FunctionInfo &function);

    /**
     * @brief Unregister a function from database
     * @param functionNamespace Namespace
     * @param functionName Function name
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t UnregisterFunction(const std::string &functionNamespace, const std::string &functionName);

    /**
     * @brief Batch unregister intentFunctions by namespace
     * @param functionNamespace Namespace to delete all functions from
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t UnregisterIntentFunctionsByNamespace(const std::string &functionNamespace);

    /**
     * @brief Reset all functions by namespace (delete all existing and add new ones)
     * @param functionNamespace Namespace to reset functions for
     * @param functions New function list to replace existing ones
     * @param successCount Output count of successfully reset functions
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t ResetNamespaceFunctions(const std::string &functionNamespace,
        const std::vector<FunctionInfo> &functions, int32_t &successCount);

    /**
     * @brief Get all functions from database
     * @param functions Output vector of FunctionInfo
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t GetAllFunctions(std::vector<FunctionInfo> &functions);

    /**
     * @brief Ensure functions database is initialized (lazy initialization)
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t EnsureFunctionsInitialized();

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
     * @brief Delete all intent functions for a namespace (internal use)
     * @param functionNamespace Namespace to delete functions from
     * @param deletedCount Output count of deleted functions
     * @return int32_t ERR_OK on success, error code otherwise
     */
    int32_t DeleteIntentFunctionsByNamespaceInternal(const std::string &functionNamespace, int32_t &deletedCount);

    /**
     * @brief Restore KVStore if corrupted
     * @param status The status code from KVStore operation
     */
    void RestoreKvStore(DistributedKv::Status status);

    /**
     * @brief Generate KVStore key from namespace and functionName
     * @param functionNamespace Namespace
     * @param functionName Function name
     * @return std::string Generated key string
     */
    static std::string GenerateFunctionKey(const std::string &functionNamespace, const std::string &functionName);

    /**
     * @brief Extract namespace from KVStore key string
     * @param keyStr Key string in format {namespace}/{functionName}
     * @return std::string Extracted namespace, empty string if invalid format
     */
    static std::string ExtractNamespaceFromKey(const std::string &keyStr);

    /**
     * @brief Check if a KVStore entry key matches the given namespace
     * @param entryKey The KVStore entry key string
     * @param functionNamespace The namespace to match against
     * @return bool true if the entry's namespace matches
     */
    static bool KeyMatchesNamespace(const std::string &entryKey, const std::string &functionNamespace);

    /**
     * @brief Check if a KVStore entry value is an INTENT_FUNCTION
     * @param entryValue The KVStore entry value (JSON string)
     * @return bool true if the entry is an INTENT_FUNCTION
     */
    static bool IsIntentFunction(const DistributedKv::Value &entryValue);

    DistributedKv::DistributedKvDataManager dataManager_;
    std::shared_ptr<DistributedKv::SingleKvStore> kvStorePtr_;
    mutable std::mutex kvStorePtrMutex_;
    std::atomic<bool> functionsInitialized_ = false;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_FUNCTION_DATA_MANAGER_H
