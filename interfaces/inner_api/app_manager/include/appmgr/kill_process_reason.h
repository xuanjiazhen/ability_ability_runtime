/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_KILL_PROCESS_REASON_H
#define OHOS_ABILITY_RUNTIME_KILL_PROCESS_REASON_H

#include <string>
#include "parcel.h"

namespace OHOS {
namespace AAFwk {
enum KillReason {
    KILLREASON_MIN = 0,
    KILLREASON_UNKNOWN = KILLREASON_MIN,
    KILLREASON_ABILITY_NOT_RESPONDING,
    KILLREASON_NORMAL,
    KILLREASON_CPP_CRASH,
    KILLREASON_JS_ERROR,
    KILLREASON_APP_FREEZE,
    KILLREASON_PERFORMANCE_CONTROL,
    KILLREASON_RESOURCE_CONTROL,
    KILLREASON_UPGRADE,
    KILLREASON_MAX = KILLREASON_UPGRADE
};

struct KillProcessReason : public Parcelable {
    KillReason killReason = KillReason::KILLREASON_UNKNOWN;
    int32_t subKillReason = 0;
    std::string killMsg = "";

    bool ReadFromParcel(Parcel &parcel);
    virtual bool Marshalling(Parcel &parcel) const override;
    static KillProcessReason *Unmarshalling(Parcel &parcel);
};

struct ProcessExitDetailInfo {
    int32_t pid = 0;
    int32_t uid = 0;
    int32_t pssValue = 0;
    int32_t rssValue = 0;
    KillProcessReason reason;
    std::string processName = "";
    std::string timeStamp = "";
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_KILL_PROCESS_REASON_H
