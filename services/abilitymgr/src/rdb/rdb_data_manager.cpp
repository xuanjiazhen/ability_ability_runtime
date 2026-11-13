/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "rdb_data_manager.h"

#include <chrono>

#include "ability_resident_process_rdb.h"
#include "ffrt.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int64_t RELEASE_DELAY_US = 3 * 60 * 1000 * 1000; // 3 minutes
}

int32_t RdbDataManager::Init(NativeRdb::RdbOpenCallback &rdbCallback)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    if (rdbStore_ != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Ability mgr rdb has existed");
        return NativeRdb::E_OK;
    }

    NativeRdb::RdbStoreConfig rdbStoreConfig(amsRdbConfig_.dbPath + amsRdbConfig_.dbName);
    rdbStoreConfig.SetSecurityLevel(NativeRdb::SecurityLevel::S1);

    int32_t ret = NativeRdb::E_OK;
    rdbStore_ = NativeRdb::RdbHelper::GetRdbStore(rdbStoreConfig, amsRdbConfig_.version, rdbCallback, ret);
    if (rdbStore_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null rdbStore_");
        return NativeRdb::E_ERROR;
    }

    return NativeRdb::E_OK;
}

int32_t RdbDataManager::EnsureConnection()
{
    if (rdbStore_ != nullptr) {
        return NativeRdb::E_OK;
    }
    AmsResidentProcessRdbCallBack callback(amsRdbConfig_);
    return Init(callback);
}

void RdbDataManager::UpdateLastAccessTime()
{
    lastAccessTime_.store(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count());
}

int32_t RdbDataManager::InsertData(const std::string &tableName, const NativeRdb::ValuesBucket &valuesBucket)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    UpdateLastAccessTime();
    if (EnsureConnection() != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null rdbStore_");
        return NativeRdb::E_ERROR;
    }
    if (tableName != amsRdbConfig_.tableName && tableName != amsRdbConfig_.metaTableName) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid rdb store table");
        return NativeRdb::E_ERROR;
    }
    int64_t rowId = -1;
    int32_t errCode = rdbStore_->InsertWithConflictResolution(
        rowId, tableName, valuesBucket, NativeRdb::ConflictResolution::ON_CONFLICT_REPLACE);
    if (errCode == NativeRdb::E_OK) {
        dbWriteCounter_.UpdateWriteCount(ABILITY_RDB_PATH);
    }
    ScheduleRelease();
    return errCode;
}

int32_t RdbDataManager::UpdateData(
    const NativeRdb::ValuesBucket &valuesBucket, const NativeRdb::AbsRdbPredicates &absRdbPredicates)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    UpdateLastAccessTime();
    if (EnsureConnection() != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null rdbStore_");
        return NativeRdb::E_ERROR;
    }
    const auto &tableName = absRdbPredicates.GetTableName();
    if (tableName != amsRdbConfig_.tableName && tableName != amsRdbConfig_.metaTableName) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid rdb store table");
        return NativeRdb::E_ERROR;
    }
    int32_t rowId = -1;
    int32_t errCode = rdbStore_->Update(rowId, valuesBucket, absRdbPredicates);
    if (errCode == NativeRdb::E_OK) {
        dbWriteCounter_.UpdateWriteCount(ABILITY_RDB_PATH);
    }
    ScheduleRelease();
    return errCode;
}

int32_t RdbDataManager::DeleteData(const NativeRdb::AbsRdbPredicates &absRdbPredicates)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    UpdateLastAccessTime();
    if (EnsureConnection() != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null rdbStore_");
        return NativeRdb::E_ERROR;
    }
    const auto &tableName = absRdbPredicates.GetTableName();
    if (tableName != amsRdbConfig_.tableName && tableName != amsRdbConfig_.metaTableName) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid rdb store table");
        return NativeRdb::E_ERROR;
    }
    int32_t rowId = -1;
    int32_t errCode = rdbStore_->Delete(rowId, absRdbPredicates);
    ScheduleRelease();
    return errCode;
}

std::shared_ptr<NativeRdb::AbsSharedResultSet> RdbDataManager::QueryData(
    const NativeRdb::AbsRdbPredicates &absRdbPredicates)
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    UpdateLastAccessTime();
    if (EnsureConnection() != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null rdbStore_");
        return nullptr;
    }
    const auto &tableName = absRdbPredicates.GetTableName();
    if (tableName != amsRdbConfig_.tableName && tableName != amsRdbConfig_.metaTableName) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid rdb store table");
        return nullptr;
    }
    auto absSharedResultSet = rdbStore_->Query(absRdbPredicates, std::vector<std::string>());
    ScheduleRelease();
    if (absSharedResultSet == nullptr || !absSharedResultSet->HasBlock()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "query data fail");
        return nullptr;
    }
    return absSharedResultSet;
}

void RdbDataManager::ReleaseConnection()
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    auto now = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    if (now - lastAccessTime_.load() < RELEASE_DELAY_US) {
        releaseScheduled_.store(false);
        return;
    }
    rdbStore_.reset();
    NativeRdb::RdbHelper::ClearCache();
    releaseScheduled_.store(false);
    TAG_LOGI(AAFwkTag::ABILITYMGR, "rdbStore connection released");
}

void RdbDataManager::ScheduleRelease()
{
    if (releaseScheduled_.exchange(true)) {
        return;
    }
    auto ffrtTaskHandle = ffrt::submit_h([this]() {
        ReleaseConnection();
    }, {}, {}, ffrt::task_attr().name("RdbDataRelease").delay(static_cast<uint64_t>(RELEASE_DELAY_US)));
    if (ffrtTaskHandle == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "submit rdb release failed, restore flag");
        releaseScheduled_.store(false);
    }
}

void RdbDataManager::ClearCache()
{
    std::lock_guard<std::mutex> lock(rdbMutex_);
    NativeRdb::RdbHelper::ClearCache();
}
} // namespace AbilityRuntime
} // namespace OHOS