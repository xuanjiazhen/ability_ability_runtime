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

#ifndef OHOS_ABILITY_RUNTIME_INTENT_HISTORY_RDB_STORE_H
#define OHOS_ABILITY_RUNTIME_INTENT_HISTORY_RDB_STORE_H

#include <memory>
#include <vector>
#include <string>

#include "intent_execution_history.h"
#include "intent_usage_stats.h"

namespace OHOS {
namespace AbilityRuntime {

class IntentHistoryRdbStore {
public:
    IntentHistoryRdbStore();
    ~IntentHistoryRdbStore();

    int32_t Init();
    void Destroy();

    int32_t InsertExecutionHistory(const IntentExecutionHistory &history);
    int32_t QueryExecutionHistory(const IntentHistoryQueryParam &queryParam,
        std::vector<IntentExecutionHistory> &histories);
    int32_t DeleteExecutionHistory(int64_t beforeTimestamp);
    int32_t CountExecutionHistory(const std::string &intentKey, int64_t startTime, int64_t endTime);

    int32_t InsertOrUpdateUsageStats(const IntentUsageStats &stats);
    int32_t QueryUsageStats(const std::string &intentKey, IntentUsageStats &stats);
    int32_t QueryAllUsageStats(std::vector<IntentUsageStats> &stats);
    int32_t DeleteUsageStats(const std::string &intentKey);

    int32_t InsertOrUpdateFeatureWeights(const IntentFeatureWeights &weights);
    int32_t QueryFeatureWeights(const std::string &intentKey, IntentFeatureWeights &weights);
    int32_t QueryAllFeatureWeights(std::vector<IntentFeatureWeights> &weights);

private:
    int32_t CreateTables();
    int32_t UpgradeDatabase(int32_t oldVersion, int32_t newVersion);

    std::string GetDatabasePath();
    std::string BuildIntentKey(const std::string &bundleName, const std::string &moduleName,
        const std::string &intentName, int32_t userId);

    bool initialized_ = false;
    std::shared_ptr<OHOS::NativeRdb::RdbStore> rdbStore_;
};

constexpr const char* INTENT_HISTORY_DB_NAME = "intent_history.db";
constexpr int32_t INTENT_HISTORY_DB_VERSION = 1;

constexpr const char* TABLE_EXECUTION_HISTORY = "intent_execution_history";
constexpr const char* TABLE_USAGE_STATS = "intent_usage_stats";
constexpr const char* TABLE_FEATURE_WEIGHTS = "intent_feature_weights";

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_INTENT_HISTORY_RDB_STORE_H