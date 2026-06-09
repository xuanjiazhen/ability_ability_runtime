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

#ifndef OHOS_ABILITY_RUNTIME_INTENT_EXECUTION_RECORDER_H
#define OHOS_ABILITY_RUNTIME_INTENT_EXECUTION_RECORDER_H

#include <memory>
#include <vector>

#include "singleton.h"
#include "intent_execution_history.h"
#include "intent_usage_stats.h"
#include "scene_info_collector.h"
#include "cpp/mutex.h"

namespace OHOS {
namespace AbilityRuntime {

class IntentHistoryRdbStore;

class IntentExecutionRecorder {
    DECLARE_DELAYED_SINGLETON(IntentExecutionRecorder)
public:
    int32_t Init();
    void Destroy();

    int32_t RecordExecution(const std::string &targetBundleName,
        const std::string &targetModuleName,
        const std::string &intentName,
        const std::string &callerBundleName,
        int32_t callerUid,
        int32_t executeMode,
        const std::string &executeParamsJson,
        int32_t resultCode,
        int32_t executeDurationMs,
        int32_t userId);

    int32_t QueryHistory(const IntentHistoryQueryParam &queryParam,
        std::vector<IntentExecutionHistory> &histories);

    int32_t ClearHistory(int64_t beforeTimestamp);

    int32_t GetUsageStats(const std::string &intentKey, IntentUsageStats &stats);
    int32_t GetAllUsageStats(std::vector<IntentUsageStats> &stats);

private:
    int32_t SaveHistoryToDB(const IntentExecutionHistory &history);
    int32_t UpdateStatsInMemory(const IntentExecutionHistory &history);
    int32_t AggregateToStats(const std::vector<IntentExecutionHistory> &histories);

    std::shared_ptr<IntentHistoryRdbStore> rdbStore_;
    std::shared_ptr<SceneInfoCollector> sceneCollector_;
    std::map<std::string, IntentUsageStats> statsMap_;

    bool initialized_ = false;
    ffrt::mutex mutex_;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_INTENT_EXECUTION_RECORDER_H