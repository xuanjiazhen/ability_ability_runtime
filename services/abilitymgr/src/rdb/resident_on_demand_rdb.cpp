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

#include "resident_on_demand_rdb.h"

#include "ability_resident_process_rdb.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
const std::string RESIDENT_ON_DEMAND_TABLE_NAME = "resident_on_demand";
const std::string BUNDLE_NAME_COLUMN = "bundle_name";
const std::string ON_DEMAND_STARTUP_COLUMN = "on_demand_startup";
}

ResidentOnDemandRdbCallBack::ResidentOnDemandRdbCallBack(const AmsRdbConfig &rdbConfig)
    : rdbConfig_(rdbConfig) {}

int32_t ResidentOnDemandRdbCallBack::OnCreate(NativeRdb::RdbStore &rdbStore)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnCreate: creating resident_on_demand table");

    std::string createTableSql = "CREATE TABLE IF NOT EXISTS " + rdbConfig_.tableName +
        " (" + BUNDLE_NAME_COLUMN + " TEXT NOT NULL PRIMARY KEY, " +
        ON_DEMAND_STARTUP_COLUMN + " INTEGER NOT NULL);";
    auto result = rdbStore.ExecuteSql(createTableSql);
    if (result != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ExecuteSql failed: %{public}d", result);
    }
    return result;
}

int32_t ResidentOnDemandRdbCallBack::OnUpgrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnUpgrade: from %{public}d to %{public}d", currentVersion, targetVersion);
    return NativeRdb::E_OK;
}

int32_t ResidentOnDemandRdbCallBack::OnDowngrade(NativeRdb::RdbStore &rdbStore, int currentVersion, int targetVersion)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnDowngrade: from %{public}d to %{public}d", currentVersion, targetVersion);
    return NativeRdb::E_OK;
}

int32_t ResidentOnDemandRdbCallBack::OnOpen(NativeRdb::RdbStore &rdbStore)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "OnOpen: resident_on_demand database opened");
    // Ensure table exists even if database already exists
    std::string createTableSql = "CREATE TABLE IF NOT EXISTS " + rdbConfig_.tableName +
        " (" + BUNDLE_NAME_COLUMN + " TEXT NOT NULL PRIMARY KEY, " +
        ON_DEMAND_STARTUP_COLUMN + " INTEGER NOT NULL);";
    auto result = rdbStore.ExecuteSql(createTableSql);
    if (result != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "OnOpen ExecuteSql failed: %{public}d", result);
    }
    return result;
}

int32_t ResidentOnDemandRdbCallBack::onCorruption(std::string databaseFile)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "onCorruption: %{public}s", databaseFile.c_str());
    return NativeRdb::E_OK;
}

ResidentOnDemandRdb& ResidentOnDemandRdb::GetInstance()
{
    static ResidentOnDemandRdb instance;
    return instance;
}

int32_t ResidentOnDemandRdb::Init()
{
    if (rdbMgr_ != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ResidentOnDemandRdb already initialized");
        return NativeRdb::E_OK;
    }

    AmsRdbConfig config;
    config.tableName = RESIDENT_ON_DEMAND_TABLE_NAME;
    rdbMgr_ = std::make_unique<RdbDataManager>(config);
    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Create RdbDataManager failed");
        return NativeRdb::E_ERROR;
    }

    ResidentOnDemandRdbCallBack callback(config);
    int32_t result = rdbMgr_->Init(callback);
    if (result != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Init failed: %{public}d", result);
        rdbMgr_.reset();
        return result;
    }

    // Ensure table exists - this is needed because the database may already exist
    // from other components, so OnCreate might not be called
    auto rdbStore = rdbMgr_->GetRdbStore();
    if (rdbStore != nullptr) {
        std::string createTableSql = "CREATE TABLE IF NOT EXISTS " + RESIDENT_ON_DEMAND_TABLE_NAME +
            " (" + BUNDLE_NAME_COLUMN + " TEXT NOT NULL PRIMARY KEY, " +
            ON_DEMAND_STARTUP_COLUMN + " INTEGER NOT NULL);";
        result = rdbStore->ExecuteSql(createTableSql);
        if (result != NativeRdb::E_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Create table failed: %{public}d", result);
            return result;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Table resident_on_demand created or already exists");
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "ResidentOnDemandRdb initialized successfully");
    return NativeRdb::E_OK;
}

bool ResidentOnDemandRdb::GetOnDemandStartup(const std::string &bundleName)
{
    if (rdbMgr_ == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Rdb not initialized, returning false");
        return false;
    }

    if (bundleName.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "bundleName is empty");
        return false;
    }

    NativeRdb::AbsRdbPredicates predicates(RESIDENT_ON_DEMAND_TABLE_NAME);
    predicates.EqualTo(BUNDLE_NAME_COLUMN, bundleName);

    auto resultSet = rdbMgr_->QueryData(predicates);
    if (resultSet == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Query failed for %{public}s", bundleName.c_str());
        return false;
    }

    ScopeGuard guard([resultSet] { resultSet->Close(); });

    int rowCount = 0;
    resultSet->GetRowCount(rowCount);
    if (rowCount <= 0) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "No record found for %{public}s", bundleName.c_str());
        return false;
    }

    auto ret = resultSet->GoToFirstRow();
    if (ret != NativeRdb::E_OK) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "GoToFirstRow failed: %{public}d", ret);
        return false;
    }

    int onDemandValue = 0;
    resultSet->GetInt(1, onDemandValue);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetOnDemandStartup: %{public}s = %{public}d", bundleName.c_str(), onDemandValue);
    return onDemandValue == 1;
}

int32_t ResidentOnDemandRdb::SetOnDemandStartup(const std::string &bundleName, bool onDemandStartup)
{
    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Rdb not initialized");
        return NativeRdb::E_ERROR;
    }

    if (bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bundleName is empty");
        return NativeRdb::E_ERROR;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "SetOnDemandStartup: %{public}s = %{public}d", bundleName.c_str(), onDemandStartup);

    NativeRdb::ValuesBucket valuesBucket;
    valuesBucket.PutString(BUNDLE_NAME_COLUMN, bundleName);
    valuesBucket.PutInt(ON_DEMAND_STARTUP_COLUMN, onDemandStartup ? 1 : 0);

    NativeRdb::AbsRdbPredicates predicates(RESIDENT_ON_DEMAND_TABLE_NAME);
    predicates.EqualTo(BUNDLE_NAME_COLUMN, bundleName);

    int32_t result = rdbMgr_->UpdateOrInsertData(valuesBucket, predicates);
    if (result != NativeRdb::E_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UpdateOrInsert failed for %{public}s: %{public}d", bundleName.c_str(), result);
        return result;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "SetOnDemandStartup success for %{public}s", bundleName.c_str());
    return NativeRdb::E_OK;
}

int32_t ResidentOnDemandRdb::SetOnDemandStartupBatch(const std::map<std::string, bool> &configList)
{
    if (rdbMgr_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Rdb not initialized");
        return NativeRdb::E_ERROR;
    }

    if (configList.empty()) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "configList is empty");
        return NativeRdb::E_OK;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "SetOnDemandStartupBatch: %{public}zu items", configList.size());

    int32_t successCount = 0;
    for (const auto &item : configList) {
        int32_t result = SetOnDemandStartup(item.first, item.second);
        if (result == NativeRdb::E_OK) {
            successCount++;
        }
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Batch operation completed: %{public}d/%{public}zu success",
        successCount, configList.size());

    return (successCount > 0) ? NativeRdb::E_OK : NativeRdb::E_ERROR;
}

} // namespace AbilityRuntime
} // namespace OHOS