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

#include "cli_function_data_manager.h"

#include <nlohmann/json.hpp>
#include <unistd.h>

#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"
#include "kvstore_transaction.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t CHECK_INTERVAL = 20000; // 20ms
constexpr int32_t MAX_TIMES = 2;           // 2 * 20ms = 40ms

constexpr const char* KV_STORE_APP_ID = "cli_functions_db";
constexpr const char* KV_STORE_STORE_ID = "cli_functions_store";
constexpr const char* STORAGE_DIR = "/data/service/el1/public/database/aimgr/cli_function";

const DistributedKv::AppId APP_ID { KV_STORE_APP_ID };
const DistributedKv::StoreId STORE_ID { KV_STORE_STORE_ID };
}

CliFunctionDataManager &CliFunctionDataManager::GetInstance()
{
    static CliFunctionDataManager manager;
    return manager;
}

CliFunctionDataManager::CliFunctionDataManager()
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "CliFunctionDataManager constructor called");
}

CliFunctionDataManager::~CliFunctionDataManager()
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "CliFunctionDataManager destructor called");
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(APP_ID, kvStorePtr_);
    }
}

DistributedKv::Status CliFunctionDataManager::GetKvStore()
{
    DistributedKv::Options options = { .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = STORAGE_DIR };
    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get KVStore: %{public}d", static_cast<int>(status));
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "KVStore initialized successfully");
    }
    return status;
}

bool CliFunctionDataManager::CheckKvStore()
{
    if (kvStorePtr_ != nullptr) {
        return true;
    }

    int32_t tryTimes = MAX_TIMES;
    while (tryTimes > 0) {
        DistributedKv::Status status = GetKvStore();
        if (status == DistributedKv::Status::SUCCESS && kvStorePtr_ != nullptr) {
            return true;
        }
        TAG_LOGW(AAFwkTag::CLI_TOOL, "CheckKvStore failed, try times: %{public}d", tryTimes);
        usleep(CHECK_INTERVAL);
        tryTimes--;
    }

    return kvStorePtr_ != nullptr;
}

int32_t CliFunctionDataManager::EnsureFunctionsInitialized()
{
    if (functionsInitialized_) {
        return ERR_OK;
    }

    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready for functions initialization");
        return ERR_NO_INIT;
    }

    functionsInitialized_ = true;
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Functions database initialized successfully");
    return ERR_OK;
}

int32_t CliFunctionDataManager::RegisterFunction(const FunctionInfo &function)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    TAG_LOGD(AAFwkTag::CLI_TOOL, "RegisterFunction called: %{public}s/%{public}s",
        function.functionNamespace.c_str(), function.functionName.c_str());

    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_NO_INIT;
    }

    // function.userId is caller-supplied (validated >= 0) and used as-is for both key and record
    std::string keyStr = GenerateFunctionKey(function.userId, function.functionNamespace, function.functionName);
    DistributedKv::Key key(keyStr);
    DistributedKv::Value value;
    DistributedKv::Status status = kvStorePtr_->Get(key, value);
    if (status == DistributedKv::Status::SUCCESS) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Function already exists, will overwrite: %{public}s/%{public}s",
            function.functionNamespace.c_str(), function.functionName.c_str());
    }

    int32_t ret = StoreFunctionNoLock(function);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to store function: %{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully registered function: %{public}s/%{public}s, userId: %{public}d",
        function.functionNamespace.c_str(), function.functionName.c_str(), function.userId);
    return ERR_OK;
}

int32_t CliFunctionDataManager::BatchRegisterFunctions(const std::vector<FunctionInfo> &functions,
    int32_t &successCount)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "BatchRegisterFunctions called: %{public}zu functions",
        functions.size());

    if (functions.empty()) {
        successCount = 0;
        return ERR_INVALID_PARAM;
    }

    successCount = 0;

    // Phase 1 (outside lock): Process and prepare entries; userId is caller-supplied (validated >= 0)
    std::vector<DistributedKv::Entry> entries;
    entries.reserve(functions.size());

    for (const auto &function : functions) {
        std::string keyStr = GenerateFunctionKey(function.userId, function.functionNamespace, function.functionName);
        DistributedKv::Entry entry;
        entry.key = DistributedKv::Key(keyStr);
        entry.value = DistributedKv::Value(function.ParseToJson().dump());
        entries.push_back(entry);
    }

    // Phase 2 (with lock): Batch insert with transaction
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);

    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_NO_INIT;
    }

    // Start transaction for batch put operation
    KvStoreTransaction transaction(kvStorePtr_);
    if (!transaction.IsStarted()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to start transaction for batch register");
        return ERR_KVSTORE_ERROR;
    }

    DistributedKv::Status status = kvStorePtr_->PutBatch(entries);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "PutBatch failed: %{public}d", static_cast<int>(status));
        transaction.Rollback();
        RestoreKvStore(status);
        return ERR_KVSTORE_ERROR;
    }

    if (transaction.Commit() != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to commit transaction for batch register");
        return ERR_KVSTORE_ERROR;
    }

    successCount = static_cast<int32_t>(entries.size());
    TAG_LOGI(AAFwkTag::CLI_TOOL, "BatchRegisterFunctions completed successfully: %{public}d functions", successCount);
    return ERR_OK;
}

int32_t CliFunctionDataManager::GetFunctionByName(int32_t userId, const std::string &functionNamespace,
    const std::string &functionName, FunctionInfo &function)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "GetFunctionByName called: %{public}s/%{public}s",
        functionNamespace.c_str(), functionName.c_str());

    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_NO_INIT;
    }
    // Point get: {userId}/{namespace}/{functionName} is the full record identity,
    // so the caller's query parameters address exactly one key
    std::string keyStr = GenerateFunctionKey(userId, functionNamespace, functionName);
    DistributedKv::Key key(keyStr);
    DistributedKv::Value value;
    DistributedKv::Status status = kvStorePtr_->Get(key, value);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetFunctionByName error: %{public}d", status);
        if (status == DistributedKv::Status::KEY_NOT_FOUND) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "function not found under userId: %{public}d", userId);
            return ERR_FUNCTION_NOT_EXIST;
        }
        RestoreKvStore(status);
        return ERR_KVSTORE_ERROR;
    }

    nlohmann::json j = nlohmann::json::parse(value.ToString(), nullptr, false);
    if (j.is_discarded()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to parse JSON for function: %{public}s", functionName.c_str());
        return ERR_JSON_PARSE_FAILED;
    }

    if (!FunctionInfo::ParseFromJson(j, function)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid function data for: %{public}s", functionName.c_str());
        return ERR_JSON_PARSE_FAILED;
    }

    return ERR_OK;
}

int32_t CliFunctionDataManager::StoreFunctionNoLock(const FunctionInfo &function)
{
    // Start transaction for single put operation
    KvStoreTransaction transaction(kvStorePtr_);
    if (!transaction.IsStarted()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to start transaction for storing function");
        return ERR_KVSTORE_ERROR;
    }

    std::string keyStr = GenerateFunctionKey(function.userId, function.functionNamespace, function.functionName);
    DistributedKv::Key key(keyStr);
    DistributedKv::Value value(function.ParseToJson().dump());
    DistributedKv::Status status = kvStorePtr_->Put(key, value);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to store function: %{public}s/%{public}s, status: %{public}d",
            function.functionNamespace.c_str(), function.functionName.c_str(), static_cast<int>(status));
        transaction.Rollback();
        RestoreKvStore(status);
        return ERR_KVSTORE_ERROR;
    }

    if (transaction.Commit() != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to commit transaction for storing function");
        return ERR_KVSTORE_ERROR;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Stored function: %{public}s/%{public}s",
        function.functionNamespace.c_str(), function.functionName.c_str());
    return ERR_OK;
}

std::string CliFunctionDataManager::GenerateFunctionKey(int32_t userId, const std::string &functionNamespace,
    const std::string &functionName)
{
    return std::to_string(userId) + "/" + functionNamespace + "/" + functionName;
}

std::string CliFunctionDataManager::GenerateUserPrefix(int32_t userId)
{
    return std::to_string(userId) + "/";
}

std::string CliFunctionDataManager::GenerateNamespacePrefix(int32_t userId, const std::string &functionNamespace)
{
    return GenerateUserPrefix(userId) + functionNamespace + "/";
}

std::string CliFunctionDataManager::ExtractNamespaceFromKey(const std::string &keyStr)
{
    // Key format: {userId}/{namespace}/{functionName}
    // Since namespace and name cannot contain '/', the namespace is the segment
    // between the first and second '/'
    size_t firstPos = keyStr.find('/');
    if (firstPos == std::string::npos) {
        return "";  // Invalid key format
    }
    size_t secondPos = keyStr.find('/', firstPos + 1);
    if (secondPos == std::string::npos) {
        return "";  // Invalid key format
    }
    return keyStr.substr(firstPos + 1, secondPos - firstPos - 1);
}

bool CliFunctionDataManager::KeyMatchesNamespace(const std::string &entryKey,
    const std::string &functionNamespace)
{
    std::string ns = ExtractNamespaceFromKey(entryKey);
    if (ns.empty()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Invalid key format: %{public}s", entryKey.c_str());
        return false;
    }
    return ns == functionNamespace;
}

bool CliFunctionDataManager::IsIntentFunction(const DistributedKv::Value &entryValue)
{
    nlohmann::json j = nlohmann::json::parse(entryValue.ToString(), nullptr, false);
    if (j.is_discarded()) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to parse entry value as JSON");
        return false;
    }
    FunctionInfo functionInfo;
    if (!FunctionInfo::ParseFromJson(j, functionInfo)) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to parse FunctionInfo from JSON");
        return false;
    }
    return functionInfo.functionType == FunctionType::INTENT_FUNCTION;
}

int32_t CliFunctionDataManager::GetExistingIntentFunctionsNoLock(int32_t userId,
    const std::string &functionNamespace, std::unordered_set<std::string> &existingKeys,
    DistributedKv::Status &kvStatus)
{
    kvStatus = DistributedKv::Status::SUCCESS;
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_NO_INIT;
    }
    existingKeys.clear();
    DistributedKv::Key prefixKey(GenerateNamespacePrefix(userId, functionNamespace));
    std::vector<DistributedKv::Entry> existingEntries;
    DistributedKv::Status status = kvStorePtr_->GetEntries(prefixKey, existingEntries);
    if (status == DistributedKv::Status::KEY_NOT_FOUND) {
        // Nothing registered under this namespace yet, which is not an error
        TAG_LOGI(AAFwkTag::CLI_TOOL, "No existing entries in namespace: %{public}s, userId: %{public}d",
            functionNamespace.c_str(), userId);
        return ERR_OK;
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get existing entries: %{public}d",
            static_cast<int>(status));
        // Restore is the caller's job: it owns the transaction and must roll it
        // back before any store restore
        kvStatus = status;
        return ERR_KVSTORE_ERROR;
    }

    for (const auto &entry : existingEntries) {
        if (IsIntentFunction(entry.value)) {
            existingKeys.insert(entry.key.ToString());
        }
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Found %{public}zu existing intent functions", existingKeys.size());
    return ERR_OK;
}

void CliFunctionDataManager::ProcessNewFunctions(const std::vector<FunctionInfo> &functions,
    std::unordered_set<std::string> &newKeys, std::vector<DistributedKv::Entry> &entries)
{
    newKeys.clear();
    entries.clear();
    entries.reserve(functions.size());

    for (const auto &function : functions) {
        if (function.functionType != FunctionType::INTENT_FUNCTION) {
            TAG_LOGD(AAFwkTag::CLI_TOOL,
                "Function is not an intent function: %{public}s/%{public}s, skipping",
                function.functionNamespace.c_str(), function.functionName.c_str());
            continue;
        }

        // userId is caller-supplied and used as-is for the key
        std::string keyStr = GenerateFunctionKey(function.userId, function.functionNamespace, function.functionName);
        newKeys.insert(keyStr);

        DistributedKv::Entry entry;
        entry.key = DistributedKv::Key(keyStr);
        entry.value = DistributedKv::Value(function.ParseToJson().dump());
        entries.push_back(entry);
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Processed %{public}zu new intent functions", entries.size());
}

void CliFunctionDataManager::CalculateObsoleteKeys(const std::unordered_set<std::string> &existingKeys,
    const std::unordered_set<std::string> &newKeys, std::vector<DistributedKv::Key> &keysToDelete)
{
    keysToDelete.clear();
    keysToDelete.reserve(existingKeys.size());

    for (const auto &oldKey : existingKeys) {
        if (newKeys.find(oldKey) == newKeys.end()) {
            keysToDelete.push_back(DistributedKv::Key(oldKey));
        }
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Calculated %{public}zu obsolete keys to delete", keysToDelete.size());
}

int32_t CliFunctionDataManager::UnregisterFunction(int32_t userId, const std::string &functionNamespace,
    const std::string &functionName)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    TAG_LOGD(AAFwkTag::CLI_TOOL, "UnregisterFunction called: %{public}s/%{public}s",
        functionNamespace.c_str(), functionName.c_str());

    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_NO_INIT;
    }

    // Start transaction for delete operation
    KvStoreTransaction transaction(kvStorePtr_);
    if (!transaction.IsStarted()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to start transaction for unregister");
        return ERR_KVSTORE_ERROR;
    }

    // Point delete: {userId}/{namespace}/{functionName} is the full record identity
    std::string keyStr = GenerateFunctionKey(userId, functionNamespace, functionName);
    DistributedKv::Key key(keyStr);
    DistributedKv::Status status = kvStorePtr_->Delete(key);
    if (status == DistributedKv::Status::KEY_NOT_FOUND) {
        // Idempotent delete: nothing stored under this user
        TAG_LOGW(AAFwkTag::CLI_TOOL, "function not found under userId: %{public}d", userId);
        return ERR_OK;
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to delete function: %{public}s/%{public}s, status: %{public}d",
            functionNamespace.c_str(), functionName.c_str(), static_cast<int>(status));
        transaction.Rollback();
        RestoreKvStore(status);
        return ERR_KVSTORE_ERROR;
    }

    if (transaction.Commit() != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to commit transaction for unregister");
        return ERR_KVSTORE_ERROR;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully unregistered function: %{public}s/%{public}s, userId: %{public}d",
        functionNamespace.c_str(), functionName.c_str(), userId);
    return ERR_OK;
}

void CliFunctionDataManager::RestoreKvStore(DistributedKv::Status status)
{
    if (status != DistributedKv::Status::DATA_CORRUPTED) {
        return;
    }
    TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore data corrupted, deleting and recreating");
    DistributedKv::Options options = { .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = STORAGE_DIR };
    dataManager_.DeleteKvStore(APP_ID, STORE_ID, options.baseDir);
    kvStorePtr_ = nullptr;  // Clear before GetSingleKvStore
    status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to recreate KVStore, status: %{public}d", static_cast<int>(status));
        kvStorePtr_ = nullptr;  // Ensure null on failure
        return;
    }
    TAG_LOGI(AAFwkTag::CLI_TOOL, "KVStore recreated successfully");
}

int32_t CliFunctionDataManager::DeleteIntentFunctionsByNamespaceNoLock(int32_t userId,
    const std::string &functionNamespace)
{
    std::vector<DistributedKv::Key> keysToDelete;

    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_NO_INIT;
    }

    // Start transaction before the query: reads use the transaction snapshot,
    // so the query and the batch delete commit as one atomic read-modify-write
    KvStoreTransaction transaction(kvStorePtr_);
    if (!transaction.IsStarted()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to start transaction for batch delete");
        return ERR_KVSTORE_ERROR;
    }

    // Get entries with the user's namespace prefix from the transaction snapshot
    std::vector<DistributedKv::Entry> allEntries;
    DistributedKv::Key prefixKey(GenerateNamespacePrefix(userId, functionNamespace));
    DistributedKv::Status status = kvStorePtr_->GetEntries(prefixKey, allEntries);
    if (status == DistributedKv::Status::KEY_NOT_FOUND) {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "No entries in namespace: %{public}s, userId: %{public}d",
            functionNamespace.c_str(), userId);
        return ERR_OK;
    }
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get entries: %{public}d", static_cast<int>(status));
        // Roll back before restoring: RestoreKvStore may close and replace
        // kvStorePtr_, after which a destructor-time rollback would target a defunct store
        transaction.Rollback();
        RestoreKvStore(status);
        return ERR_KVSTORE_ERROR;
    }

    // Filter intent function keys
    for (const auto &entry : allEntries) {
        if (IsIntentFunction(entry.value)) {
            keysToDelete.push_back(entry.key);
        }
    }

    // Batch delete
    if (!keysToDelete.empty()) {
        DistributedKv::Status deleteStatus = kvStorePtr_->DeleteBatch(keysToDelete);
        if (deleteStatus != DistributedKv::Status::SUCCESS) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "DeleteBatch failed: %{public}d", static_cast<int>(deleteStatus));
            transaction.Rollback();
            RestoreKvStore(deleteStatus);
            return ERR_KVSTORE_ERROR;
        }
    }

    if (transaction.Commit() != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to commit transaction for batch delete");
        return ERR_KVSTORE_ERROR;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Deleted %{public}zu intent functions in namespace: %{public}s",
        keysToDelete.size(), functionNamespace.c_str());
    return ERR_OK;
}

int32_t CliFunctionDataManager::UnregisterIntentFunctionsByNamespace(int32_t userId,
    const std::string &functionNamespace)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "UnregisterIntentFunctionsByNamespace called: %{public}s", functionNamespace.c_str());

    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);

    int32_t ret = DeleteIntentFunctionsByNamespaceNoLock(userId, functionNamespace);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "DeleteIntentFunctionsByNamespaceNoLock failed: %{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "UnregisterIntentFunctionsByNamespace completed: %{public}s",
        functionNamespace.c_str());
    return ERR_OK;
}

int32_t CliFunctionDataManager::GetAllFunctions(int32_t userId, std::vector<FunctionInfo> &functions)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllFunctions called, userId: %{public}d", userId);
    // Scope the query to the user's {userId}/ prefix so other users' records
    // never leave the store
    std::vector<DistributedKv::Entry> allEntries;
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
            return ERR_NO_INIT;
        }
        DistributedKv::Key prefixKey(GenerateUserPrefix(userId));
        DistributedKv::Status status = kvStorePtr_->GetEntries(prefixKey, allEntries);
        if (status == DistributedKv::Status::KEY_NOT_FOUND) {
            // No functions registered for this user yet, which is not an error
            functions.clear();
            return ERR_OK;
        }
        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to get entries: %{public}d", static_cast<int>(status));
            RestoreKvStore(status);
            return ERR_KVSTORE_ERROR;
        }
    }

    // Parse all entries to FunctionInfo
    functions.clear();
    for (const auto &entry : allEntries) {
        nlohmann::json j = nlohmann::json::parse(entry.value.ToString(), nullptr, false);
        if (j.is_discarded()) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to parse entry value as JSON: %{public}s",
                entry.key.ToString().c_str());
            continue;
        }

        FunctionInfo function;
        if (FunctionInfo::ParseFromJson(j, function)) {
            functions.push_back(function);
        } else {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "Invalid function data: %{public}s",
                entry.key.ToString().c_str());
        }
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllFunctions completed: %{public}zu functions, userId: %{public}d",
        functions.size(), userId);
    return ERR_OK;
}

int32_t CliFunctionDataManager::ResetNamespaceFunctions(int32_t userId, const std::string &functionNamespace,
    const std::vector<FunctionInfo> &functions, int32_t &successCount)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ResetNamespaceFunctions called: %{public}s, %{public}zu functions",
        functionNamespace.c_str(), functions.size());

    if (functionNamespace.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Invalid namespace");
        return ERR_INVALID_PARAM;
    }

    // The existing-key query is scoped by the userId argument, and every new entry's
    // key is built from its caller-supplied function.userId (the service layer validates
    // they match), keeping the read-modify-write within one user's records
    std::unordered_set<std::string> existingKeys;
    std::unordered_set<std::string> newKeys;
    std::vector<DistributedKv::Entry> entriesToAdd;
    std::vector<DistributedKv::Key> keysToDelete;

    ProcessNewFunctions(functions, newKeys, entriesToAdd);

    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);

    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_NO_INIT;
    }

    KvStoreTransaction transaction(kvStorePtr_);
    if (!transaction.IsStarted()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to start transaction for reset namespace");
        return ERR_KVSTORE_ERROR;
    }

    // Step 1: Get existing data from the transaction snapshot; the raw KVStore
    // status is propagated so the rollback below happens before any store restore
    DistributedKv::Status kvStatus = DistributedKv::Status::SUCCESS;
    int32_t ret = GetExistingIntentFunctionsNoLock(userId, functionNamespace, existingKeys, kvStatus);
    if (ret != ERR_OK) {
        // Roll back before restoring: RestoreKvStore may close and replace
        // kvStorePtr_, after which a destructor-time rollback would target a defunct store
        transaction.Rollback();
        RestoreKvStore(kvStatus);
        return ret;
    }

    // Step 2: Calculate obsolete keys
    CalculateObsoleteKeys(existingKeys, newKeys, keysToDelete);

    // Batch insert new functions
    if (!entriesToAdd.empty()) {
        DistributedKv::Status status = kvStorePtr_->PutBatch(entriesToAdd);
        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "PutBatch failed: %{public}d", static_cast<int>(status));
            transaction.Rollback();
            RestoreKvStore(status);
            return ERR_KVSTORE_ERROR;
        }
    }

    // Batch delete obsolete functions
    if (!keysToDelete.empty()) {
        DistributedKv::Status status = kvStorePtr_->DeleteBatch(keysToDelete);
        if (status != DistributedKv::Status::SUCCESS) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "DeleteBatch failed: %{public}d", static_cast<int>(status));
            transaction.Rollback();
            RestoreKvStore(status);
            return ERR_KVSTORE_ERROR;
        }
    }

    // Commit transaction atomically
    if (transaction.Commit() != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to commit transaction for reset namespace");
        return ERR_KVSTORE_ERROR;
    }

    successCount = static_cast<int32_t>(entriesToAdd.size());

    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "ResetNamespaceFunctions completed: %{public}s, added=%{public}d, deleted=%{public}zu, userId: %{public}d",
        functionNamespace.c_str(), successCount, keysToDelete.size(), userId);

    return ERR_OK;
}

} // namespace CliTool
} // namespace OHOS
