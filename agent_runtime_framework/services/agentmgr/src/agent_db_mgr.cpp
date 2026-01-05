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

#include "agent_db_mgr.h"

#include <unistd.h>

#include "hilog_tag_wrapper.h"
#include "json_utils.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms
constexpr const char *KEEP_ALIVE_STORAGE_DIR = "/data/service/el1/public/database/keep_alive_service";
const std::string JSON_KEY_BUNDLE_NAME = "bundleName";
const std::string JSON_KEY_USER_ID = "userId";
const std::string JSON_KEY_URL = "url";
} // namespace

const DistributedKv::AppId AgentDbMgr::APP_ID = { "keep_alive_storage" };
const DistributedKv::StoreId AgentDbMgr::STORE_ID = { "keep_alive_infos" };

AgentDbMgr &AgentDbMgr::GetInstance()
{
    static AgentDbMgr instance;
    return instance;
}

AgentDbMgr::AgentDbMgr() {}

AgentDbMgr::~AgentDbMgr()
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(APP_ID, kvStorePtr_);
    }
}

DistributedKv::Status AgentDbMgr::RestoreKvStore(DistributedKv::Status status)
{
    if (status == DistributedKv::Status::DATA_CORRUPTED) {
        DistributedKv::Options options = {
            .createIfMissing = true,
            .encrypt = false,
            .autoSync = false,
            .syncable = false,
            .securityLevel = DistributedKv::SecurityLevel::S2,
            .area = DistributedKv::EL1,
            .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
            .baseDir = KEEP_ALIVE_STORAGE_DIR,
        };
        TAG_LOGE(AAFwkTag::SER_ROUTER, "corrupted, deleting db");
        dataManager_.DeleteKvStore(APP_ID, STORE_ID, options.baseDir);
        TAG_LOGE(AAFwkTag::SER_ROUTER, "deleted corrupted db, recreating db");
        status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
        TAG_LOGE(AAFwkTag::SER_ROUTER, "recreate db result:%{public}d", status);
    }
    return status;
}

DistributedKv::Status AgentDbMgr::GetKvStore()
{
    DistributedKv::Options options = {
        .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = KEEP_ALIVE_STORAGE_DIR,
    };

    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, APP_ID, STORE_ID, kvStorePtr_);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Error: %{public}d", status);
        status = RestoreKvStore(status);
        return status;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "Get kvStore success");
    return status;
}

bool AgentDbMgr::CheckKvStore()
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
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Try times: %{public}d", tryTimes);
        usleep(CHECK_INTERVAL);
        tryTimes--;
    }
    return kvStorePtr_ != nullptr;
}

int32_t AgentDbMgr::InsertData(const std::string &bundleName, int32_t userId, const AgentCard &info)
{
    // 检查并获取KV存储锁
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        if (!CheckKvStore()) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "null kvStore");
            return ERR_NO_INIT;
        }
    }

    // 转换数据为键值对
    DistributedKv::Key key = ConvertKey(bundleName, userId);
    DistributedKv::Value value = ConvertValue(info);
    DistributedKv::Status status;
    // 获取存储锁并插入数据
    {
        std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
        status = kvStorePtr_->Put(key, value);
    }

    // 处理插入失败情况
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "kvStore insert error: %{public}d", status);
        // 尝试恢复KV存储
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            status = RestoreKvStore(status);
        }
        return ERR_INVALID_OPERATION;
    }
    // 插入成功返回成功码
    return ERR_OK;
}

int32_t AgentDbMgr::DeleteData(const std::string &bundleName, int32_t userId)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null kvStore");
        return ERR_NO_INIT;
    }

    DistributedKv::Key key = ConvertKey(bundleName, userId);
    DistributedKv::Status status = kvStorePtr_->Delete(key);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "kvStore delete error: %{public}d", status);
        status = RestoreKvStore(status);
        return ERR_INVALID_OPERATION;
    }

    return ERR_OK;
}

int32_t AgentDbMgr::QueryData(const std::string &bundleName, int32_t userId, AgentCard &info)
{
    std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null kvStore");
        return ERR_NO_INIT;
    }
    DistributedKv::Key key = ConvertKey(bundleName, userId);
    DistributedKv::Value value;
    DistributedKv::Status status = kvStorePtr_->Get(key, value);
    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "QueryData error: %{public}d", status);
        {
            std::lock_guard<std::mutex> lock(kvStorePtrMutex_);
            RestoreKvStore(status);
        }
        return ERR_INVALID_OPERATION;
    }
    nlohmann::json jsonObject = nlohmann::json::parse(value.ToString(), nullptr, false);
    info = AgentCard::FromJson(jsonObject);
    return ERR_OK;
}

DistributedKv::Value AgentDbMgr::ConvertValue(const AgentCard &info)
{
    nlohmann::json jsonObject = info.ToJson();
    DistributedKv::Value value(jsonObject.dump());
    TAG_LOGD(AAFwkTag::SER_ROUTER, "value: %{public}s", value.ToString().c_str());
    return value;
}

DistributedKv::Key AgentDbMgr::ConvertKey(const std::string &bundleName, int32_t userId)
{
    nlohmann::json jsonObject = nlohmann::json {
        { JSON_KEY_BUNDLE_NAME, bundleName },
        { JSON_KEY_USER_ID, userId },
    };
    DistributedKv::Key key(jsonObject.dump());
    TAG_LOGD(AAFwkTag::SER_ROUTER, "key: %{public}s", key.ToString().c_str());
    return key;
}
} // namespace AgentRuntime
} // namespace OHOS
