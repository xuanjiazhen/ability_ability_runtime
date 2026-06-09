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

#include "intent_execution_history.h"

#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {

bool IntentExecutionHistory::ReadFromParcel(Parcel &parcel)
{
    executionId = parcel.ReadUint64();
    timestamp = parcel.ReadInt64();
    hourOfDay = parcel.ReadInt32();
    dayOfWeek = parcel.ReadInt32();
    targetBundleName = parcel.ReadString();
    targetModuleName = parcel.ReadString();
    intentName = parcel.ReadString();
    callerBundleName = parcel.ReadString();
    callerUid = parcel.ReadInt32();
    locationTag = parcel.ReadString();
    deviceStateJson = parcel.ReadString();
    foregroundApp = parcel.ReadString();
    executeMode = parcel.ReadInt32();
    executeParamsJson = parcel.ReadString();
    resultCode = parcel.ReadInt32();
    executeDurationMs = parcel.ReadInt32();
    userId = parcel.ReadInt32();
    return true;
}

bool IntentExecutionHistory::Marshalling(Parcel &parcel) const
{
    parcel.WriteUint64(executionId);
    parcel.WriteInt64(timestamp);
    parcel.WriteInt32(hourOfDay);
    parcel.WriteInt32(dayOfWeek);
    parcel.WriteString(targetBundleName);
    parcel.WriteString(targetModuleName);
    parcel.WriteString(intentName);
    parcel.WriteString(callerBundleName);
    parcel.WriteInt32(callerUid);
    parcel.WriteString(locationTag);
    parcel.WriteString(deviceStateJson);
    parcel.WriteString(foregroundApp);
    parcel.WriteInt32(executeMode);
    parcel.WriteString(executeParamsJson);
    parcel.WriteInt32(resultCode);
    parcel.WriteInt32(executeDurationMs);
    parcel.WriteInt32(userId);
    return true;
}

IntentExecutionHistory *IntentExecutionHistory::Unmarshalling(Parcel &parcel)
{
    IntentExecutionHistory *history = new (std::nothrow) IntentExecutionHistory();
    if (history == nullptr) {
        return nullptr;
    }
    if (!history->ReadFromParcel(parcel)) {
        delete history;
        history = nullptr;
    }
    return history;
}

bool IntentExecutionHistory::MarshallingVector(Parcel &parcel, const std::vector<IntentExecutionHistory> &histories)
{
    parcel.WriteInt32(static_cast<int32_t>(histories.size()));
    for (const auto &history : histories) {
        history.Marshalling(parcel);
    }
    return true;
}

bool IntentExecutionHistory::UnmarshallingVector(Parcel &parcel, std::vector<IntentExecutionHistory> &histories)
{
    int32_t size = parcel.ReadInt32();
    histories.clear();
    for (int32_t i = 0; i < size; i++) {
        IntentExecutionHistory *history = IntentExecutionHistory::Unmarshalling(parcel);
        if (history != nullptr) {
            histories.push_back(*history);
            delete history;
        }
    }
    return true;
}

std::string IntentExecutionHistory::GetIntentKey() const
{
    return std::to_string(userId) + "/" + targetBundleName + "/" + targetModuleName + "/" + intentName;
}

std::string IntentExecutionHistory::ToJsonString() const
{
    nlohmann::json json;
    json["executionId"] = executionId;
    json["timestamp"] = timestamp;
    json["hourOfDay"] = hourOfDay;
    json["dayOfWeek"] = dayOfWeek;
    json["targetBundleName"] = targetBundleName;
    json["targetModuleName"] = targetModuleName;
    json["intentName"] = intentName;
    json["callerBundleName"] = callerBundleName;
    json["callerUid"] = callerUid;
    json["locationTag"] = locationTag;
    json["deviceStateJson"] = deviceStateJson;
    json["foregroundApp"] = foregroundApp;
    json["executeMode"] = executeMode;
    json["executeParamsJson"] = executeParamsJson;
    json["resultCode"] = resultCode;
    json["executeDurationMs"] = executeDurationMs;
    json["userId"] = userId;
    return json.dump();
}

IntentExecutionHistory IntentExecutionHistory::FromJsonString(const std::string &jsonStr)
{
    IntentExecutionHistory history;
    if (jsonStr.empty()) {
        return history;
    }
    nlohmann::json json = nlohmann::json::parse(jsonStr, nullptr, false);
    if (json.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "parse execution history json failed");
        return history;
    }
    
    if (json.contains("executionId")) history.executionId = json["executionId"].get<uint64_t>();
    if (json.contains("timestamp")) history.timestamp = json["timestamp"].get<int64_t>();
    if (json.contains("hourOfDay")) history.hourOfDay = json["hourOfDay"].get<int32_t>();
    if (json.contains("dayOfWeek")) history.dayOfWeek = json["dayOfWeek"].get<int32_t>();
    if (json.contains("targetBundleName")) history.targetBundleName = json["targetBundleName"].get<std::string>();
    if (json.contains("targetModuleName")) history.targetModuleName = json["targetModuleName"].get<std::string>();
    if (json.contains("intentName")) history.intentName = json["intentName"].get<std::string>();
    if (json.contains("callerBundleName")) history.callerBundleName = json["callerBundleName"].get<std::string>();
    if (json.contains("callerUid")) history.callerUid = json["callerUid"].get<int32_t>();
    if (json.contains("locationTag")) history.locationTag = json["locationTag"].get<std::string>();
    if (json.contains("deviceStateJson")) history.deviceStateJson = json["deviceStateJson"].get<std::string>();
    if (json.contains("foregroundApp")) history.foregroundApp = json["foregroundApp"].get<std::string>();
    if (json.contains("executeMode")) history.executeMode = json["executeMode"].get<int32_t>();
    if (json.contains("executeParamsJson")) history.executeParamsJson = json["executeParamsJson"].get<std::string>();
    if (json.contains("resultCode")) history.resultCode = json["resultCode"].get<int32_t>();
    if (json.contains("executeDurationMs")) history.executeDurationMs = json["executeDurationMs"].get<int32_t>();
    if (json.contains("userId")) history.userId = json["userId"].get<int32_t>();
    
    return history;
}

bool IntentHistoryQueryParam::ReadFromParcel(Parcel &parcel)
{
    bundleName = parcel.ReadString();
    moduleName = parcel.ReadString();
    intentName = parcel.ReadString();
    startTime = parcel.ReadInt64();
    endTime = parcel.ReadInt64();
    hourFilter = parcel.ReadInt32();
    locationFilter = parcel.ReadString();
    userId = parcel.ReadInt32();
    limit = parcel.ReadInt32();
    return true;
}

bool IntentHistoryQueryParam::Marshalling(Parcel &parcel) const
{
    parcel.WriteString(bundleName);
    parcel.WriteString(moduleName);
    parcel.WriteString(intentName);
    parcel.WriteInt64(startTime);
    parcel.WriteInt64(endTime);
    parcel.WriteInt32(hourFilter);
    parcel.WriteString(locationFilter);
    parcel.WriteInt32(userId);
    parcel.WriteInt32(limit);
    return true;
}

IntentHistoryQueryParam *IntentHistoryQueryParam::Unmarshalling(Parcel &parcel)
{
    IntentHistoryQueryParam *param = new (std::nothrow) IntentHistoryQueryParam();
    if (param == nullptr) {
        return nullptr;
    }
    if (!param->ReadFromParcel(parcel)) {
        delete param;
        param = nullptr;
    }
    return param;
}

} // namespace AbilityRuntime
} // namespace OHOS