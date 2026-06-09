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

#ifndef OHOS_ABILITY_RUNTIME_INTENT_ANALYSIS_SCHEDULER_H
#define OHOS_ABILITY_RUNTIME_INTENT_ANALYSIS_SCHEDULER_H

#include <memory>

#include "singleton.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AbilityRuntime {

class IntentExecutionRecorder;
class IntentRecommendEngine;

class IntentAnalysisScheduler {
    DECLARE_DELAYED_SINGLETON(IntentAnalysisScheduler)
public:
    int32_t Init();
    void Destroy();

    void OnDailyAnalysis();
    void OnHourlyAnalysis();

private:
    int32_t AggregateExecutionHistory();
    int32_t CalculateFeatureWeights();
    int32_t TrainModel();
    int32_t UpdateRecommendCache();
    int32_t CleanupOldHistory();

    void ScheduleDailyTask();
    void ScheduleHourlyTask();

    std::shared_ptr<TaskHandlerWrap> taskHandler_;
    std::shared_ptr<IntentExecutionRecorder> recorder_;
    std::shared_ptr<IntentRecommendEngine> engine_;

    bool initialized_ = false;
    int64_t lastDailyAnalysisTime_ = 0;
    int64_t lastHourlyAnalysisTime_ = 0;
};

constexpr int64_t DAILY_ANALYSIS_HOUR = 2;
constexpr int64_t HISTORY_RETENTION_DAYS = 30;

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_INTENT_ANALYSIS_SCHEDULER_H