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

#ifndef OHOS_ABILITY_RUNTIME_INTENT_USAGE_STATS_H
#define OHOS_ABILITY_RUNTIME_INTENT_USAGE_STATS_H

#include <string>
#include <vector>
#include <map>
#include <cstdint>

#include "parcel.h"

namespace OHOS {
namespace AbilityRuntime {

struct IntentUsageStats : public Parcelable {
    std::string intentKey;
    std::string bundleName;
    std::string moduleName;
    std::string intentName;

    int64_t totalExecutions = 0;
    int64_t lastExecutionTime = 0;
    int64_t firstExecutionTime = 0;

    std::vector<int64_t> hourlyCount;
    std::vector<int64_t> dailyCount;

    std::map<std::string, int64_t> locationCount;
    std::map<std::string, int64_t> deviceStateCount;
    std::map<std::string, int64_t> callerCount;

    double avgExecuteDurationMs = 0.0;
    int32_t successRate = 0;

    int32_t userId = 0;
    int64_t updatedAt = 0;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static IntentUsageStats *Unmarshalling(Parcel &parcel);
    static bool MarshallingVector(Parcel &parcel, const std::vector<IntentUsageStats> &stats);
    static bool UnmarshallingVector(Parcel &parcel, std::vector<IntentUsageStats> &stats);

    std::string ToJsonString() const;
    static IntentUsageStats FromJsonString(const std::string &jsonStr);

    int64_t GetHourlyCount(int32_t hour) const;
    int64_t GetDailyCount(int32_t day) const;
    int64_t GetLocationCount(const std::string &location) const;
    int64_t GetDeviceStateCount(const std::string &state) const;
    int64_t GetCallerCount(const std::string &caller) const;

    void IncrementHourlyCount(int32_t hour);
    void IncrementDailyCount(int32_t day);
    void IncrementLocationCount(const std::string &location);
    void IncrementDeviceStateCount(const std::string &state);
    void IncrementCallerCount(const std::string &caller);
};

struct IntentUsageStatsQueryParam : public Parcelable {
    std::string bundleName;
    std::string moduleName;
    std::string intentName;
    int32_t userId = -1;
    int64_t minExecutions = 0;
    int32_t limit = 100;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static IntentUsageStatsQueryParam *Unmarshalling(Parcel &parcel);
};

struct IntentFeatureWeights : public Parcelable {
    std::string intentKey;
    double timeWeight = 0.0;
    double locationWeight = 0.0;
    double deviceStateWeight = 0.0;
    double callerWeight = 0.0;
    double frequencyWeight = 0.0;
    double recencyWeight = 0.0;

    int64_t updatedAt = 0;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static IntentFeatureWeights *Unmarshalling(Parcel &parcel);
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_INTENT_USAGE_STATS_H