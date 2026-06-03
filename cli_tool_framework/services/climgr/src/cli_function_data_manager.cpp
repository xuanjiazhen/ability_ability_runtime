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

namespace OHOS {
namespace CliTool {

namespace {
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms

const DistributedKv::AppId APP_ID { CliFunctionDataManager::KV_STORE_APP_ID };
const DistributedKv::StoreId STORE_ID { CliFunctionDataManager::KV_STORE_STORE_ID };
}

CliFunctionDataManager &CliFunctionDataManager::GetInstance()
{
    static CliFunctionDataManager manager;
    return manager;
}

CliFunctionDataManager::CliFunctionDataManager()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager constructor called");
}

CliFunctionDataManager::~CliFunctionDataManager()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "CliFunctionDataManager destructor called");
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

int32_t CliFunctionDataManager::RegisterFunction(const FunctionInfo &function)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "RegisterFunction called: %{public}s/%{public}s",
        function.bundleName.c_str(), function.functionName.c_str());

    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_NO_INIT;
    }

    int32_t ret = StoreFunction(function);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to store function: %{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully registered function: %{public}s/%{public}s",
        function.bundleName.c_str(), function.functionName.c_str());
    return ERR_OK;
}

int32_t CliFunctionDataManager::GetFunctionByName(const std::string &bundleName,
    const std::string &functionName, FunctionInfo &function)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetFunctionByName called: %{public}s/%{public}s",
        bundleName.c_str(), functionName.c_str());

    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore not ready");
        return ERR_NO_INIT;
    }

    std::string keyStr = GenerateFunctionKey(bundleName, functionName);
    DistributedKv::Key key(keyStr);
    DistributedKv::Value value;
    DistributedKv::Status status = kvStorePtr_->Get(key, value);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "GetFunctionByName error: %{public}d", status);
        if (status == DistributedKv::Status::KEY_NOT_FOUND) {
            TAG_LOGW(AAFwkTag::CLI_TOOL, "function not found");
            return ERR_FUNCTION_NOT_EXIST;
        }
        RestoreKvStore(status);
        return status;
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

int32_t CliFunctionDataManager::StoreFunction(const FunctionInfo &function)
{
    std::string keyStr = GenerateFunctionKey(function.bundleName, function.functionName);
    DistributedKv::Key key(keyStr);
    DistributedKv::Value value(function.ParseToJson().dump());
    DistributedKv::Status status = kvStorePtr_->Put(key, value);

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Failed to store function: %{public}s/%{public}s, status: %{public}d",
            function.bundleName.c_str(), function.functionName.c_str(), static_cast<int>(status));
        return -1;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Stored function: %{public}s/%{public}s",
        function.bundleName.c_str(), function.functionName.c_str());
    return ERR_OK;
}

std::string CliFunctionDataManager::GenerateFunctionKey(const std::string &bundleName,
    const std::string &functionName)
{
    return bundleName + "/" + functionName;
}

DistributedKv::Status CliFunctionDataManager::RestoreKvStore(DistributedKv::Status status)
{
    if (status == DistributedKv::Status::DATA_CORRUPTED) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "KVStore data corrupted, deleting and recreating");
        DistributedKv::Options options = {
            .createIfMissing = true,
            .encrypt = false,
            .autoSync = false,
            .syncable = false,
            .securityLevel = DistributedKv::SecurityLevel::S2,
            .area = DistributedKv::EL1,
            .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
            .baseDir = STORAGE_DIR
        };
        dataManager_.DeleteKvStore(APP_ID, STORE_ID, options.baseDir);
        status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Recreated KVStore with status: %{public}d", static_cast<int>(status));
    }
    return status;
}

} // namespace CliTool
} // namespace OHOS
