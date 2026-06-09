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

#ifndef OHOS_ABILITY_RUNTIME_INTENT_EXECUTION_HISTORY_H
#define OHOS_ABILITY_RUNTIME_INTENT_EXECUTION_HISTORY_H

#include <string>
#include <cstdint>

#include "parcel.h"
#include "intent_recommend_param.h"

namespace OHOS {
namespace AbilityRuntime {

struct IntentExecutionHistory : public Parcelable {
    uint64_t executionId = 0;
    int64_t timestamp = 0;
    int32_t hourOfDay = 0;
    int32_t dayOfWeek = 0;

    std::string targetBundleName;
    std::string targetModuleName;
    std::string intentName;

    std::string callerBundleName;
    int32_t callerUid = 0;

    std::string locationTag;
    std::string deviceStateJson;
    std::string foregroundApp;

    int32_t executeMode = 0;
    std::string executeParamsJson;

    int32_t resultCode = 0;
    int32_t executeDurationMs = 0;

    int32_t userId = 0;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static IntentExecutionHistory *Unmarshalling(Parcel &parcel);
    static bool MarshallingVector(Parcel &parcel, const std::vector<IntentExecutionHistory> &histories);
    static bool UnmarshallingVector(Parcel &parcel, std::vector<IntentExecutionHistory> &histories);

    std::string GetIntentKey() const;
    std::string ToJsonString() const;
    static IntentExecutionHistory FromJsonString(const std::string &jsonStr);
};

struct IntentHistoryQueryParam : public Parcelable {
    std::string bundleName;
    std::string moduleName;
    std::string intentName;
    int64_t startTime = 0;
    int64_t endTime = 0;
    int32_t hourFilter = -1;
    std::string locationFilter;
    int32_t userId = -1;
    int32_t limit = 100;

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static IntentHistoryQueryParam *Unmarshalling(Parcel &parcel);
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_INTENT_EXECUTION_HISTORY_H