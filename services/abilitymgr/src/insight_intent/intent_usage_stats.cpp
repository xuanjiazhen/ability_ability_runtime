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

#include "intent_usage_stats.h"

#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {

bool IntentUsageStats::ReadFromParcel(Parcel &parcel)
{
    intentKey = parcel.ReadString();
    bundleName = parcel.ReadString();
    moduleName = parcel.ReadString();
    intentName = parcel.ReadString();
    totalExecutions = parcel.ReadInt64();
    lastExecutionTime = parcel.ReadInt64();
    firstExecutionTime = parcel.ReadInt64();
    
    int32_t hourlySize = parcel.ReadInt32();
    hourlyCount.clear();
    for (int32_t i = 0; i < hourlySize; i++) {
        hourlyCount.push_back(parcel.ReadInt64());
    }
    
    int32_t dailySize = parcel.ReadInt32();
    dailyCount.clear();
    for (int32_t i = 0; i < dailySize; i++) {
        dailyCount.push_back(parcel.ReadInt64());
    }
    
    int32_t locationSize = parcel.ReadInt32();
    locationCount.clear();
    for (int32_t i = 0; i < locationSize; i++) {
        std::string key = parcel.ReadString();
        int64_t value = parcel.ReadInt64();
        locationCount[key] = value;
    }
    
    int32_t deviceStateSize = parcel.ReadInt32();
    deviceStateCount.clear();
    for (int32_t i = 0; i < deviceStateSize; i++) {
        std::string key = parcel.ReadString();
        int64_t value = parcel.ReadInt64();
        deviceStateCount[key] = value;
    }
    
    int32_t callerSize = parcel.ReadInt32();
    callerCount.clear();
    for (int32_t i = 0; i < callerSize; i++) {
        std::string key = parcel.ReadString();
        int64_t value = parcel.ReadInt64();
        callerCount[key] = value;
    }
    
    avgExecuteDurationMs = parcel.ReadDouble();
    successRate = parcel.ReadInt32();
    userId = parcel.ReadInt32();
    updatedAt = parcel.ReadInt64();
    return true;
}

bool IntentUsageStats::Marshalling(Parcel &parcel) const
{
    parcel.WriteString(intentKey);
    parcel.WriteString(bundleName);
    parcel.WriteString(moduleName);
    parcel.WriteString(intentName);
    parcel.WriteInt64(totalExecutions);
    parcel.WriteInt64(lastExecutionTime);
    parcel.WriteInt64(firstExecutionTime);
    
    parcel.WriteInt32(static_cast<int32_t>(hourlyCount.size()));
    for (const auto &count : hourlyCount) {
        parcel.WriteInt64(count);
    }
    
    parcel.WriteInt32(static_cast<int32_t>(dailyCount.size()));
    for (const auto &count : dailyCount) {
        parcel.WriteInt64(count);
    }
    
    parcel.WriteInt32(static_cast<int32_t>(locationCount.size()));
    for (const auto &pair : locationCount) {
        parcel.WriteString(pair.first);
        parcel.WriteInt64(pair.second);
    }
    
    parcel.WriteInt32(static_cast<int32_t>(deviceStateCount.size()));
    for (const auto &pair : deviceStateCount) {
        parcel.WriteString(pair.first);
        parcel.WriteInt64(pair.second);
    }
    
    parcel.WriteInt32(static_cast<int32_t>(callerCount.size()));
    for (const auto &pair : callerCount) {
        parcel.WriteString(pair.first);
        parcel.WriteInt64(pair.second);
    }
    
    parcel.WriteDouble(avgExecuteDurationMs);
    parcel.WriteInt32(successRate);
    parcel.WriteInt32(userId);
    parcel.WriteInt64(updatedAt);
    return true;
}

IntentUsageStats *IntentUsageStats::Unmarshalling(Parcel &parcel)
{
    IntentUsageStats *stats = new (std::nothrow) IntentUsageStats();
    if (stats == nullptr) {
        return nullptr;
    }
    if (!stats->ReadFromParcel(parcel)) {
        delete stats;
        stats = nullptr;
    }
    return stats;
}

bool IntentUsageStats::MarshallingVector(Parcel &parcel, const std::vector<IntentUsageStats> &stats)
{
    parcel.WriteInt32(static_cast<int32_t>(stats.size()));
    for (const auto &stat : stats) {
        stat.Marshalling(parcel);
    }
    return true;
}

bool IntentUsageStats::UnmarshallingVector(Parcel &parcel, std::vector<IntentUsageStats> &stats)
{
    int32_t size = parcel.ReadInt32();
    stats.clear();
    for (int32_t i = 0; i < size; i++) {
        IntentUsageStats *stat = IntentUsageStats::Unmarshalling(parcel);
        if (stat != nullptr) {
            stats.push_back(*stat);
            delete stat;
        }
    }
    return true;
}

std::string IntentUsageStats::ToJsonString() const
{
    nlohmann::json json;
    json["intentKey"] = intentKey;
    json["bundleName"] = bundleName;
    json["moduleName"] = moduleName;
    json["intentName"] = intentName;
    json["totalExecutions"] = totalExecutions;
    json["lastExecutionTime"] = lastExecutionTime;
    json["firstExecutionTime"] = firstExecutionTime;
    json["hourlyCount"] = hourlyCount;
    json["dailyCount"] = dailyCount;
    json["locationCount"] = locationCount;
    json["deviceStateCount"] = deviceStateCount;
    json["callerCount"] = callerCount;
    json["avgExecuteDurationMs"] = avgExecuteDurationMs;
    json["successRate"] = successRate;
    json["userId"] = userId;
    json["updatedAt"] = updatedAt;
    return json.dump();
}

IntentUsageStats IntentUsageStats::FromJsonString(const std::string &jsonStr)
{
    IntentUsageStats stats;
    if (jsonStr.empty()) {
        return stats;
    }
    nlohmann::json json = nlohmann::json::parse(jsonStr, nullptr, false);
    if (json.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "parse usage stats json failed");
        return stats;
    }
    
    if (json.contains("intentKey")) stats.intentKey = json["intentKey"].get<std::string>();
    if (json.contains("bundleName")) stats.bundleName = json["bundleName"].get<std::string>();
    if (json.contains("moduleName")) stats.moduleName = json["moduleName"].get<std::string>();
    if (json.contains("intentName")) stats.intentName = json["intentName"].get<std::string>();
    if (json.contains("totalExecutions")) stats.totalExecutions = json["totalExecutions"].get<int64_t>();
    if (json.contains("lastExecutionTime")) stats.lastExecutionTime = json["lastExecutionTime"].get<int64_t>();
    if (json.contains("firstExecutionTime")) stats.firstExecutionTime = json["firstExecutionTime"].get<int64_t>();
    if (json.contains("hourlyCount") && json["hourlyCount"].is_array()) {
        stats.hourlyCount = json["hourlyCount"].get<std::vector<int64_t>>();
    }
    if (json.contains("dailyCount") && json["dailyCount"].is_array()) {
        stats.dailyCount = json["dailyCount"].get<std::vector<int64_t>>();
    }
    if (json.contains("locationCount") && json["locationCount"].is_object()) {
        stats.locationCount = json["locationCount"].get<std::map<std::string, int64_t>>();
    }
    if (json.contains("deviceStateCount") && json["deviceStateCount"].is_object()) {
        stats.deviceStateCount = json["deviceStateCount"].get<std::map<std::string, int64_t>>();
    }
    if (json.contains("callerCount") && json["callerCount"].is_object()) {
        stats.callerCount = json["callerCount"].get<std::map<std::string, int64_t>>();
    }
    if (json.contains("avgExecuteDurationMs")) stats.avgExecuteDurationMs = json["avgExecuteDurationMs"].get<double>();
    if (json.contains("successRate")) stats.successRate = json["successRate"].get<int32_t>();
    if (json.contains("userId")) stats.userId = json["userId"].get<int32_t>();
    if (json.contains("updatedAt")) stats.updatedAt = json["updatedAt"].get<int64_t>();
    
    return stats;
}

int64_t IntentUsageStats::GetHourlyCount(int32_t hour) const
{
    if (hour < 0 || hour >= static_cast<int32_t>(hourlyCount.size())) {
        return 0;
    }
    return hourlyCount[hour];
}

int64_t IntentUsageStats::GetDailyCount(int32_t day) const
{
    if (day < 1 || day > static_cast<int32_t>(dailyCount.size())) {
        return 0;
    }
    return dailyCount[day - 1];
}

int64_t IntentUsageStats::GetLocationCount(const std::string &location) const
{
    auto it = locationCount.find(location);
    return (it != locationCount.end()) ? it->second : 0;
}

int64_t IntentUsageStats::GetDeviceStateCount(const std::string &state) const
{
    auto it = deviceStateCount.find(state);
    return (it != deviceStateCount.end()) ? it->second : 0;
}

int64_t IntentUsageStats::GetCallerCount(const std::string &caller) const
{
    auto it = callerCount.find(caller);
    return (it != callerCount.end()) ? it->second : 0;
}

void IntentUsageStats::IncrementHourlyCount(int32_t hour)
{
    if (hour < 0 || hour >= 24) {
        return;
    }
    if (static_cast<int32_t>(hourlyCount.size()) < 24) {
        hourlyCount.resize(24, 0);
    }
    hourlyCount[hour]++;
}

void IntentUsageStats::IncrementDailyCount(int32_t day)
{
    if (day < 1 || day > 7) {
        return;
    }
    if (static_cast<int32_t>(dailyCount.size()) < 7) {
        dailyCount.resize(7, 0);
    }
    dailyCount[day - 1]++;
}

void IntentUsageStats::IncrementLocationCount(const std::string &location)
{
    locationCount[location]++;
}

void IntentUsageStats::IncrementDeviceStateCount(const std::string &state)
{
    deviceStateCount[state]++;
}

void IntentUsageStats::IncrementCallerCount(const std::string &caller)
{
    callerCount[caller]++;
}

bool IntentUsageStatsQueryParam::ReadFromParcel(Parcel &parcel)
{
    bundleName = parcel.ReadString();
    moduleName = parcel.ReadString();
    intentName = parcel.ReadString();
    userId = parcel.ReadInt32();
    minExecutions = parcel.ReadInt64();
    limit = parcel.ReadInt32();
    return true;
}

bool IntentUsageStatsQueryParam::Marshalling(Parcel &parcel) const
{
    parcel.WriteString(bundleName);
    parcel.WriteString(moduleName);
    parcel.WriteString(intentName);
    parcel.WriteInt32(userId);
    parcel.WriteInt64(minExecutions);
    parcel.WriteInt32(limit);
    return true;
}

IntentUsageStatsQueryParam *IntentUsageStatsQueryParam::Unmarshalling(Parcel &parcel)
{
    IntentUsageStatsQueryParam *param = new (std::nothrow) IntentUsageStatsQueryParam();
    if (param == nullptr) {
        return nullptr;
    }
    if (!param->ReadFromParcel(parcel)) {
        delete param;
        param = nullptr;
    }
    return param;
}

bool IntentFeatureWeights::ReadFromParcel(Parcel &parcel)
{
    intentKey = parcel.ReadString();
    timeWeight = parcel.ReadDouble();
    locationWeight = parcel.ReadDouble();
    deviceStateWeight = parcel.ReadDouble();
    callerWeight = parcel.ReadDouble();
    frequencyWeight = parcel.ReadDouble();
    recencyWeight = parcel.ReadDouble();
    updatedAt = parcel.ReadInt64();
    return true;
}

bool IntentFeatureWeights::Marshalling(Parcel &parcel) const
{
    parcel.WriteString(intentKey);
    parcel.WriteDouble(timeWeight);
    parcel.WriteDouble(locationWeight);
    parcel.WriteDouble(deviceStateWeight);
    parcel.WriteDouble(callerWeight);
    parcel.WriteDouble(frequencyWeight);
    parcel.WriteDouble(recencyWeight);
    parcel.WriteInt64(updatedAt);
    return true;
}

IntentFeatureWeights *IntentFeatureWeights::Unmarshalling(Parcel &parcel)
{
    IntentFeatureWeights *weights = new (std::nothrow) IntentFeatureWeights();
    if (weights == nullptr) {
        return nullptr;
    }
    if (!weights->ReadFromParcel(parcel)) {
        delete weights;
        weights = nullptr;
    }
    return weights;
}

} // namespace AbilityRuntime
} // namespace OHOS