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

#include "intent_recommend_param.h"

#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {

bool DeviceStateInfo::ReadFromParcel(Parcel &parcel)
{
    isCharging = parcel.ReadBool();
    batteryLevel = parcel.ReadInt32();
    isWifiConnected = parcel.ReadBool();
    isBluetoothConnected = parcel.ReadBool();
    networkType = parcel.ReadString();
    motionState = parcel.ReadString();
    isScreenOn = parcel.ReadBool();
    screenBrightness = parcel.ReadInt32();
    return true;
}

bool DeviceStateInfo::Marshalling(Parcel &parcel) const
{
    parcel.WriteBool(isCharging);
    parcel.WriteInt32(batteryLevel);
    parcel.WriteBool(isWifiConnected);
    parcel.WriteBool(isBluetoothConnected);
    parcel.WriteString(networkType);
    parcel.WriteString(motionState);
    parcel.WriteBool(isScreenOn);
    parcel.WriteInt32(screenBrightness);
    return true;
}

DeviceStateInfo *DeviceStateInfo::Unmarshalling(Parcel &parcel)
{
    DeviceStateInfo *info = new (std::nothrow) DeviceStateInfo();
    if (info == nullptr) {
        return nullptr;
    }
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

std::string DeviceStateInfo::ToJsonString() const
{
    nlohmann::json json;
    json["isCharging"] = isCharging;
    json["batteryLevel"] = batteryLevel;
    json["isWifiConnected"] = isWifiConnected;
    json["isBluetoothConnected"] = isBluetoothConnected;
    json["networkType"] = networkType;
    json["motionState"] = motionState;
    json["isScreenOn"] = isScreenOn;
    json["screenBrightness"] = screenBrightness;
    return json.dump();
}

DeviceStateInfo DeviceStateInfo::FromJsonString(const std::string &jsonStr)
{
    DeviceStateInfo info;
    if (jsonStr.empty()) {
        return info;
    }
    nlohmann::json json = nlohmann::json::parse(jsonStr, nullptr, false);
    if (json.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "parse device state json failed");
        return info;
    }
    if (json.contains("isCharging") && json["isCharging"].is_boolean()) {
        info.isCharging = json["isCharging"].get<bool>();
    }
    if (json.contains("batteryLevel") && json["batteryLevel"].is_number_integer()) {
        info.batteryLevel = json["batteryLevel"].get<int32_t>();
    }
    if (json.contains("isWifiConnected") && json["isWifiConnected"].is_boolean()) {
        info.isWifiConnected = json["isWifiConnected"].get<bool>();
    }
    if (json.contains("isBluetoothConnected") && json["isBluetoothConnected"].is_boolean()) {
        info.isBluetoothConnected = json["isBluetoothConnected"].get<bool>();
    }
    if (json.contains("networkType") && json["networkType"].is_string()) {
        info.networkType = json["networkType"].get<std::string>();
    }
    if (json.contains("motionState") && json["motionState"].is_string()) {
        info.motionState = json["motionState"].get<std::string>();
    }
    if (json.contains("isScreenOn") && json["isScreenOn"].is_boolean()) {
        info.isScreenOn = json["isScreenOn"].get<bool>();
    }
    if (json.contains("screenBrightness") && json["screenBrightness"].is_number_integer()) {
        info.screenBrightness = json["screenBrightness"].get<int32_t>();
    }
    return info;
}

bool IntentRecommendQueryParam::ReadFromParcel(Parcel &parcel)
{
    timestamp = parcel.ReadInt64();
    hourOfDay = parcel.ReadInt32();
    dayOfWeek = parcel.ReadInt32();
    locationTag = parcel.ReadString();
    latitude = parcel.ReadDouble();
    longitude = parcel.ReadDouble();
    
    DeviceStateInfo *deviceStatePtr = DeviceStateInfo::Unmarshalling(parcel);
    if (deviceStatePtr != nullptr) {
        deviceState = *deviceStatePtr;
        delete deviceStatePtr;
    }
    
    foregroundApp = parcel.ReadString();
    callerBundleName = parcel.ReadString();
    topK = parcel.ReadInt32();
    minScore = parcel.ReadDouble();
    userId = parcel.ReadInt32();
    
    int32_t intentTypesSize = parcel.ReadInt32();
    for (int32_t i = 0; i < intentTypesSize; i++) {
        intentTypes.push_back(parcel.ReadString());
    }
    domainFilter = parcel.ReadString();
    return true;
}

bool IntentRecommendQueryParam::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt64(timestamp);
    parcel.WriteInt32(hourOfDay);
    parcel.WriteInt32(dayOfWeek);
    parcel.WriteString(locationTag);
    parcel.WriteDouble(latitude);
    parcel.WriteDouble(longitude);
    deviceState.Marshalling(parcel);
    parcel.WriteString(foregroundApp);
    parcel.WriteString(callerBundleName);
    parcel.WriteInt32(topK);
    parcel.WriteDouble(minScore);
    parcel.WriteInt32(userId);
    
    parcel.WriteInt32(static_cast<int32_t>(intentTypes.size()));
    for (const auto &type : intentTypes) {
        parcel.WriteString(type);
    }
    parcel.WriteString(domainFilter);
    return true;
}

IntentRecommendQueryParam *IntentRecommendQueryParam::Unmarshalling(Parcel &parcel)
{
    IntentRecommendQueryParam *param = new (std::nothrow) IntentRecommendQueryParam();
    if (param == nullptr) {
        return nullptr;
    }
    if (!param->ReadFromParcel(parcel)) {
        delete param;
        param = nullptr;
    }
    return param;
}

bool IntentRecommendResult::ReadFromParcel(Parcel &parcel)
{
    bundleName = parcel.ReadString();
    moduleName = parcel.ReadString();
    intentName = parcel.ReadString();
    displayName = parcel.ReadString();
    displayDescription = parcel.ReadString();
    score = parcel.ReadDouble();
    reason = parcel.ReadString();
    
    InsightIntentInfoForQuery *intentInfoPtr = InsightIntentInfoForQuery::Unmarshalling(parcel);
    if (intentInfoPtr != nullptr) {
        intentInfo = *intentInfoPtr;
        delete intentInfoPtr;
    }
    return true;
}

bool IntentRecommendResult::Marshalling(Parcel &parcel) const
{
    parcel.WriteString(bundleName);
    parcel.WriteString(moduleName);
    parcel.WriteString(intentName);
    parcel.WriteString(displayName);
    parcel.WriteString(displayDescription);
    parcel.WriteDouble(score);
    parcel.WriteString(reason);
    intentInfo.Marshalling(parcel);
    return true;
}

IntentRecommendResult *IntentRecommendResult::Unmarshalling(Parcel &parcel)
{
    IntentRecommendResult *result = new (std::nothrow) IntentRecommendResult();
    if (result == nullptr) {
        return nullptr;
    }
    if (!result->ReadFromParcel(parcel)) {
        delete result;
        result = nullptr;
    }
    return result;
}

bool IntentRecommendResult::MarshallingVector(Parcel &parcel, const std::vector<IntentRecommendResult> &results)
{
    parcel.WriteInt32(static_cast<int32_t>(results.size()));
    for (const auto &result : results) {
        result.Marshalling(parcel);
    }
    return true;
}

bool IntentRecommendResult::UnmarshallingVector(Parcel &parcel, std::vector<IntentRecommendResult> &results)
{
    int32_t size = parcel.ReadInt32();
    results.clear();
    for (int32_t i = 0; i < size; i++) {
        IntentRecommendResult *result = IntentRecommendResult::Unmarshalling(parcel);
        if (result != nullptr) {
            results.push_back(*result);
            delete result;
        }
    }
    return true;
}

bool SceneInfo::ReadFromParcel(Parcel &parcel)
{
    timestamp = parcel.ReadInt64();
    hourOfDay = parcel.ReadInt32();
    dayOfWeek = parcel.ReadInt32();
    locationTag = parcel.ReadString();
    
    DeviceStateInfo *deviceStatePtr = DeviceStateInfo::Unmarshalling(parcel);
    if (deviceStatePtr != nullptr) {
        deviceState = *deviceStatePtr;
        delete deviceStatePtr;
    }
    
    foregroundApp = parcel.ReadString();
    return true;
}

bool SceneInfo::Marshalling(Parcel &parcel) const
{
    parcel.WriteInt64(timestamp);
    parcel.WriteInt32(hourOfDay);
    parcel.WriteInt32(dayOfWeek);
    parcel.WriteString(locationTag);
    deviceState.Marshalling(parcel);
    parcel.WriteString(foregroundApp);
    return true;
}

SceneInfo *SceneInfo::Unmarshalling(Parcel &parcel)
{
    SceneInfo *info = new (std::nothrow) SceneInfo();
    if (info == nullptr) {
        return nullptr;
    }
    if (!info->ReadFromParcel(parcel)) {
        delete info;
        info = nullptr;
    }
    return info;
}

std::string SceneInfo::ToJsonString() const
{
    nlohmann::json json;
    json["timestamp"] = timestamp;
    json["hourOfDay"] = hourOfDay;
    json["dayOfWeek"] = dayOfWeek;
    json["locationTag"] = locationTag;
    json["deviceState"] = deviceState.ToJsonString();
    json["foregroundApp"] = foregroundApp;
    return json.dump();
}

SceneInfo SceneInfo::FromJsonString(const std::string &jsonStr)
{
    SceneInfo info;
    if (jsonStr.empty()) {
        return info;
    }
    nlohmann::json json = nlohmann::json::parse(jsonStr, nullptr, false);
    if (json.is_discarded()) {
        TAG_LOGE(AAFwkTag::INTENT, "parse scene info json failed");
        return info;
    }
    if (json.contains("timestamp") && json["timestamp"].is_number_integer()) {
        info.timestamp = json["timestamp"].get<int64_t>();
    }
    if (json.contains("hourOfDay") && json["hourOfDay"].is_number_integer()) {
        info.hourOfDay = json["hourOfDay"].get<int32_t>();
    }
    if (json.contains("dayOfWeek") && json["dayOfWeek"].is_number_integer()) {
        info.dayOfWeek = json["dayOfWeek"].get<int32_t>();
    }
    if (json.contains("locationTag") && json["locationTag"].is_string()) {
        info.locationTag = json["locationTag"].get<std::string>();
    }
    if (json.contains("deviceState") && json["deviceState"].is_string()) {
        info.deviceState = DeviceStateInfo::FromJsonString(json["deviceState"].get<std::string>());
    }
    if (json.contains("foregroundApp") && json["foregroundApp"].is_string()) {
        info.foregroundApp = json["foregroundApp"].get<std::string>();
    }
    return info;
}

} // namespace AbilityRuntime
} // namespace OHOS