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
#include "insight_intent_rdb_data_mgr.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
namespace {
const std::string INTENT_KEY = "INTENT_KEY";
const int32_t INTENT_KEY_INDEX = 0;
const int32_t INTENT_VALUE_INDEX = 1;
constexpr int8_t CLOSE_TIME = 20; // delay 20s stop rdbStore
constexpr int32_t RETRY_TIMES = 3;
constexpr int32_t RETRY_INTERVAL = 500; // 500ms
} // namespace

InsightIntentRdbDataMgr::InsightIntentRdbDataMgr(const IntentRdbConfig &intentRdbConfig)
    : intentRdbConfig_(intentRdbConfig)
{
}

InsightIntentRdbDataMgr::~InsightIntentRdbDataMgr()
{
}

bool InsightIntentRdbDataMgr::InitIntentTable(const IntentRdbConfig &intentRdbConfig)
{
    TAG_LOGI("Init");
    if (intentRdbConfig.tableName.empty()) {
        TAG_LOGE("empty IntentRdbConfig");
        return false;
    }

    if (rdbStore_ == nullptr && GetRdbStore() != ERR_OK) {
        TAG_LOGE("null IntentRdbStore");
        return false;
    }
    std::string createTableSql = "CREATE TABLE IF NOT EXISTS " + intentRdbConfig.tableName
            + " (KEY TEXT NOT NULL PRIMARY KEY, VALUE TEXT NOT NULL);";
    int32_t ret = NativeRdb::E_OK;
    {
        std::shared_lock<std::shared_mutex> guard(rdbStoreMutex_);
        ret = rdbStore_->ExecuteSql(createTableSql);
    }

    if (ret != NativeRdb::E_OK) {
        TAG_LOGE("Create rdb table failed, ret:%{public}" PRId32 "", ret);
        return false;
    }

    return true;
}

std::shared_ptr<NativeRdb::RdbStore> InsightIntentRdbDataMgr::GetRdbStore()
{
    std::lock_guard<std::mutex> lock(rdbStoreMutex_);
    std::string rdbPath = intentRdbConfig_.dbPath + intentRdbConfig_.dbName;
    NativeRdb::RdbStoreConfig rdbStoreConfig(intentRdbConfig_.dbPath + intentRdbConfig_.dbName);
    rdbStoreConfig.SetSecurityLevel(NativeRdb::SecurityLevel::S1);
    rdbStoreConfig.SetWriteTime(WRITE_TIMEOUT);
    rdbStoreConfig.SetAllowRebuild(true);
    // for check db exist or not
    bool isNeedRebuildDb = false;
    std::string rdbFilePath = intentRdbConfig_.dbPath + std::string("/") + std::string(BMS_BACK_UP_RDB_NAME);
    if (access(rdbStoreConfig.GetPath().c_str(), F_OK) != 0) {
        TAG_LOGW("intent db :%{public}s is not exist, need to create. errno:%{public}d",
            rdbStoreConfig.GetPath().c_str(), errno);
        if (access(rdbFilePath.c_str(), F_OK) == 0) {
            isNeedRebuildDb = true;
        }
    }
    int32_t errCode = NativeRdb::E_OK;
    IntentRdbOpenCallback IntentRdbOpenCallback(intentRdbConfig_);
    rdbStore_ = NativeRdb::RdbHelper::GetRdbStore(
        rdbStoreConfig,
        intentRdbConfig_.version,
        IntentRdbOpenCallback,
        errCode);
    if (rdbStore_ == nullptr) {
        TAG_LOGE("GetRdbStore failed, errCode:%{public}d", errCode);
        return nullptr;
    }
    NativeRdb::RebuiltType rebuildType = NativeRdb::RebuiltType::NONE;
    int32_t rebuildCode = rdbStore_->GetRebuilt(rebuildType);
    if (rebuildType == NativeRdb::RebuiltType::REBUILT || isNeedRebuildDb) {
        TAG_LOGI("start %{public}s restore ret %{public}d, type:%{public}d", intentRdbConfig_.dbName.c_str(),
            rebuildCode, static_cast<int32_t>(rebuildType));
        int32_t restoreRet = rdbStore_->Restore(rdbFilePath);
        if (restoreRet != NativeRdb::E_OK) {
            TAG_LOGE("rdb restore failed ret:%{public}d", restoreRet);
        }
    }

    if (rdbStore_ != nullptr) {
        DelayCloseRdbStore();
    }
    return rdbStore_;
}

void InsightIntentRdbDataMgr::DelayCloseRdbStore()
{
    std::weak_ptr<InsightIntentRdbDataMgr> weakPtr = shared_from_this();
    auto task = [weakPtr]() {
        std::this_thread::sleep_for(std::chrono::seconds(CLOSE_TIME));
        auto sharedPtr = weakPtr.lock();
        if (sharedPtr == nullptr) {
            return;
        }
        std::lock_guard<std::mutex> lock(sharedPtr->rdbStoreMutex_);
        sharedPtr->rdbStore_ = nullptr;
    };
    std::thread closeRdbStoreThread(task);
    closeRdbStoreThread.detach();
}

bool InsightIntentRdbDataMgr::InsertData(const std::string &key, const std::string &value)
{
    TAG_LOGD("InsertData start");
    auto rdbStore = GetRdbStore();
    if (rdbStore == nullptr) {
        TAG_LOGE("RdbStore is null");
        return false;
    }

    int64_t rowId = -1;
    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutString(INTENT_KEY, key);
    valuesBucket.PutString(INTENT_VALUE, value);
    auto ret = InsertWithRetry(rdbStore, rowId, valuesBucket);
    return ret == NativeRdb::E_OK;
}

bool InsightIntentRdbDataMgr::UpdateData(const std::string &key, const std::string &value)
{
    TAG_LOGD("UpdateData start");
    auto rdbStore = GetRdbStore();
    if (rdbStore == nullptr) {
        TAG_LOGE("RdbStore is null");
        return false;
    }

    int32_t rowId = -1;
    NativeRdb::AbsRdbPredicates absRdbPredicates(intentRdbConfig_.tableName);
    absRdbPredicates.EqualTo(INTENT_KEY, key);
    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutString(INTENT_KEY, key);
    valuesBucket.PutString(INTENT_VALUE, value);
    auto ret = rdbStore->Update(rowId, valuesBucket, absRdbPredicates);
    return ret == NativeRdb::E_OK;
}

bool InsightIntentRdbDataMgr::DeleteData(const std::string &key)
{
    TAG_LOGD("DeleteData start");
    auto rdbStore = GetRdbStore();
    if (rdbStore == nullptr) {
        TAG_LOGE("RdbStore is null");
        return false;
    }

    int32_t rowId = -1;
    NativeRdb::AbsRdbPredicates absRdbPredicates(intentRdbConfig_.tableName);
    absRdbPredicates.EqualTo(INTENT_KEY, key);
    auto ret = rdbStore->Delete(rowId, absRdbPredicates);
    return ret == NativeRdb::E_OK;
}

bool InsightIntentRdbDataMgr::QueryData(const std::string &key, std::string &value)
{
    TAG_LOGD("QueryData start");
    auto rdbStore = GetRdbStore();
    if (rdbStore == nullptr) {
        TAG_LOGE("RdbStore is null");
        return false;
    }

    NativeRdb::AbsRdbPredicates absRdbPredicates(intentRdbConfig_.tableName);
    absRdbPredicates.EqualTo(INTENT_KEY, key);
    auto absSharedResultSet = rdbStore->QueryByStep(absRdbPredicates, std::vector<std::string>());
    if (absSharedResultSet == nullptr) {
        TAG_LOGE("absSharedResultSet failed");
        return false;
    }
    ScopeGuard stateGuard([&] { absSharedResultSet->Close(); });
    auto ret = absSharedResultSet->GoToFirstRow();
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE("GoToFirstRow failed, ret: %{public}d", ret);
        return false;
    }

    ret = absSharedResultSet->GetString(INTENT_VALUE_INDEX, value);
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE("QueryData failed, ret: %{public}d", ret);
        return false;
    }

    return true;
}

bool InsightIntentRdbDataMgr::QueryAllData(std::map<std::string, std::string> &datas)
{
    TAG_LOGD("QueryAllData start");
    auto rdbStore = GetRdbStore();
    if (rdbStore == nullptr) {
        TAG_LOGE("RdbStore is null");
        return false;
    }

    NativeRdb::AbsRdbPredicates absRdbPredicates(intentRdbConfig_.tableName);
    auto absSharedResultSet = rdbStore->QueryByStep(absRdbPredicates, std::vector<std::string>());
    if (absSharedResultSet == nullptr) {
        TAG_LOGE("absSharedResultSet failed");
        return false;
    }
    ScopeGuard stateGuard([&] { absSharedResultSet->Close(); });

    if (absSharedResultSet->GoToFirstRow() != NativeRdb::E_OK) {
        TAG_LOGE("GoToFirstRow failed");
        return false;
    }

    do {
        std::string key;
        if (absSharedResultSet->GetString(INTENT_KEY_INDEX, key) != NativeRdb::E_OK) {
            TAG_LOGE("GetString key failed");
            return false;
        }

        std::string value;
        if (absSharedResultSet->GetString(INTENT_VALUE_INDEX, value) != NativeRdb::E_OK) {
            TAG_LOGE("GetString value failed");
            return false;
        }

        datas.emplace(key, value);
    } while (absSharedResultSet->GoToNextRow() == NativeRdb::E_OK);
    return !datas.empty();
}

void InsightIntentRdbDataMgr::BackupRdb()
{
    TAG_LOGI("%{public}s backup start", intentRdbConfig_.dbName.c_str());
    auto rdbStore = GetRdbStore();
    if (rdbStore == nullptr) {
        TAG_LOGE("RdbStore is null");
        return;
    }
    auto ret = rdbStore->Backup(intentRdbConfig_.dbPath + std::string("/") + std::string(INTENT_BACK_UP_RDB_NAME));
    if (ret != NativeRdb::E_OK) {
        TAG_LOGE("Backup failed, errCode:%{public}d", ret);
    }
    TAG_LOGI("%{public}s backup end", intentRdbConfig_.dbName.c_str());
}

int32_t InsightIntentRdbDataMgr::InsertWithRetry(std::shared_ptr<NativeRdb::RdbStore> rdbStore, int64_t &rowId,
    const NativeRdb::ValuesBucket &valuesBucket)
{
    int32_t retryCnt = 0;
    int32_t ret = 0;
    do {
        ret = rdbStore->InsertWithConflictResolution(rowId, intentRdbConfig_.tableName,
            valuesBucket, NativeRdb::ConflictResolution::ON_CONFLICT_REPLACE);
        if (ret == NativeRdb::E_OK || !IsRetryErrCode(ret)) {
            break;
        }
        if (++retryCnt < RETRY_TIMES) {
            std::this_thread::sleep_for(std::chrono::milliseconds(RETRY_INTERVAL));
        }
        TAG_LOGW("rdb insert failed, retry count: %{public}d, ret: %{public}d", retryCnt, ret);
    } while (retryCnt < RETRY_TIMES);
    return ret;
}

bool InsightIntentRdbDataMgr::IsRetryErrCode(int32_t errCode)
{
    if (errCode == NativeRdb::E_DATABASE_BUSY ||
        errCode == NativeRdb::E_SQLITE_BUSY ||
        errCode == NativeRdb::E_SQLITE_LOCKED ||
        errCode == NativeRdb::E_SQLITE_NOMEM ||
        errCode == NativeRdb::E_SQLITE_IOERR) {
        return true;
    }
    return false;
}

IntentRdbOpenCallback::IntentRdbOpenCallback(const IntentRdbConfig &intentRdbConfig)\
    : intentRdbConfig_(intentRdbConfig) {}

int32_t IntentRdbOpenCallback::OnCreate(NativeRdb::RdbStore &rdbStore)
{
    TAG_LOGI("OnCreate");
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::OnUpgrade(
    NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    TAG_LOGI("OnUpgrade currentVersion: %{public}d, targetVersion: %{public}d",
        currentVersion, targetVersion);
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::OnDowngrade(
    NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    TAG_LOGI("OnDowngrade  currentVersion: %{public}d, targetVersion: %{public}d",
        currentVersion, targetVersion);
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::OnOpen(NativeRdb::RdbStore &rdbStore)
{
    TAG_LOGI("OnOpen");
    return NativeRdb::E_OK;
}

int32_t IntentRdbOpenCallback::onCorruption(std::string databaseFile)
{
    TAG_LOGI("onCorruption");
    return NativeRdb::E_OK;
}
} // namespace AppExecFwk
} // namespace OHOS