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

#ifndef OHOS_ABILITY_RUNTIME_INTENT_RECOMMEND_PARAM_H
#define OHOS_ABILITY_RUNTIME_INTENT_RECOMMEND_PARAM_H

#include <string>
#include <vector>
#include <map>

#include "parcel.h"
#include "insight_intent_info_for_query.h"

namespace OHOS {
namespace AbilityRuntime {

struct DeviceStateInfo : public Parcelable {
    bool isCharging = false;
    int32_t batteryLevel = 0;
    bool isWifiConnected = false;
    bool isBluetoothConnected = false;
    std::string networkType;
    std::string motionState;
    bool isScreenOn = false;
    int32_t screenBrightness = 0;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static DeviceStateInfo *Unmarshalling(Parcel &parcel);
    std::string ToJsonString() const;
    static DeviceStateInfo FromJsonString(const std::string &jsonStr);
};

struct IntentRecommendQueryParam : public Parcelable {
    int64_t timestamp = 0;
    int32_t hourOfDay = -1;
    int32_t dayOfWeek = -1;
    std::string locationTag;
    double latitude = 0.0;
    double longitude = 0.0;
    DeviceStateInfo deviceState;
    std::string foregroundApp;
    std::string callerBundleName;
    int32_t topK = 10;
    double minScore = 0.0;
    int32_t userId = -1;
    std::vector<std::string> intentTypes;
    std::string domainFilter;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static IntentRecommendQueryParam *Unmarshalling(Parcel &parcel);
};

struct IntentRecommendResult : public Parcelable {
    std::string bundleName;
    std::string moduleName;
    std::string intentName;
    std::string displayName;
    std::string displayDescription;
    double score = 0.0;
    std::string reason;
    InsightIntentInfoForQuery intentInfo;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static IntentRecommendResult *Unmarshalling(Parcel &parcel);
    static bool MarshallingVector(Parcel &parcel, const std::vector<IntentRecommendResult> &results);
    static bool UnmarshallingVector(Parcel &parcel, std::vector<IntentRecommendResult> &results);
};

struct SceneInfo : public Parcelable {
    int64_t timestamp = 0;
    int32_t hourOfDay = 0;
    int32_t dayOfWeek = 0;
    std::string locationTag;
    DeviceStateInfo deviceState;
    std::string foregroundApp;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static SceneInfo *Unmarshalling(Parcel &parcel);
    std::string ToJsonString() const;
    static SceneInfo FromJsonString(const std::string &jsonStr);
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_INTENT_RECOMMEND_PARAM_H