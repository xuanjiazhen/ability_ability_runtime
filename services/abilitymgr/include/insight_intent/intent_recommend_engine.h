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

#ifndef OHOS_ABILITY_RUNTIME_INTENT_RECOMMEND_ENGINE_H
#define OHOS_ABILITY_RUNTIME_INTENT_RECOMMEND_ENGINE_H

#include <memory>
#include <vector>
#include <map>
#include <string>

#include "singleton.h"
#include "intent_recommend_param.h"
#include "intent_usage_stats.h"
#include "insight_intent_info_for_query.h"

namespace OHOS {
namespace AbilityRuntime {

class IntentRecommendEngine {
    DECLARE_DELAYED_SINGLETON(IntentRecommendEngine)
public:
    int32_t Init();
    void Destroy();

    std::vector<IntentRecommendResult> GetRecommendedIntents(
        const IntentRecommendQueryParam &queryParam);

    int32_t UpdateStats(const std::string &intentKey, const IntentUsageStats &stats);
    int32_t RemoveStats(const std::string &intentKey);

    int32_t LoadStatsFromDB();
    int32_t SaveStatsToDB();

    int32_t TrainModel();
    int32_t UpdateFeatureWeights(const std::string &intentKey, const IntentFeatureWeights &weights);

private:
    double CalculateSceneMatchScore(const IntentUsageStats &stats,
        const IntentRecommendQueryParam &queryParam);
    
    double CalculateTimeSimilarity(const std::vector<int64_t> &hourlyCount, int32_t queryHour);
    double CalculateDaySimilarity(const std::vector<int64_t> &dailyCount, int32_t queryDay);
    double CalculateLocationSimilarity(const std::map<std::string, int64_t> &locationCount,
        const std::string &queryLocation);
    double CalculateDeviceStateSimilarity(const std::map<std::string, int64_t> &deviceStateCount,
        const DeviceStateInfo &queryDeviceState);
    double CalculateCallerSimilarity(const std::map<std::string, int64_t> &callerCount,
        const std::string &queryCaller);
    
    double CalculateFrequencyScore(const IntentUsageStats &stats);
    double CalculateRecencyScore(const IntentUsageStats &stats, int64_t currentTimestamp);
    
    double CalculateFinalScore(const IntentUsageStats &stats,
        const IntentRecommendQueryParam &queryParam,
        const IntentFeatureWeights &weights);

    std::vector<IntentRecommendResult> FilterAndSort(
        std::vector<std::pair<IntentUsageStats, double>> &scoredIntents,
        const IntentRecommendQueryParam &queryParam);

    std::map<std::string, IntentUsageStats> statsCache_;
    std::map<std::string, IntentFeatureWeights> featureWeights_;
    std::map<std::string, InsightIntentInfoForQuery> intentInfoCache_;

    bool initialized_ = false;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_INTENT_RECOMMEND_ENGINE_H