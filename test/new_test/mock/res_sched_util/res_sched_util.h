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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_RES_SCHED_UTIL_H
#define MOCK_OHOS_ABILITY_RUNTIME_RES_SCHED_UTIL_H

#include <unordered_set>
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
constexpr int32_t INTENT_EXEMPTION_DURATION = 10000;
constexpr int64_t RES_TYPE_SCB_START_ABILITY = 0;
constexpr int64_t RES_TYPE_EXTENSION_START_ABILITY = 1;
class ResSchedUtil final {
public:
    static ResSchedUtil &GetInstance()
    {
        static ResSchedUtil instance;
        return instance;
    }
    void ReportAbilityStartInfoToRSS(const AppExecFwk::AbilityInfo &abilityInfo, int32_t pid, bool isColdStart,
        int32_t warmStartType) {}
    void ReportAbilityAssociatedStartInfoToRSS(
        const AppExecFwk::AbilityInfo &abilityInfo, int64_t resSchedType, int32_t callerUid, int32_t callerPid) {}
    bool NeedReportByPidWhenConnect(const AppExecFwk::AbilityInfo &abilityInfo)
    {
        return false;
    }
    void ReportEventToRSS(const int32_t uid, const std::string &bundleName, const std::string &reason,
        const int32_t pid = -1, const int32_t callerPid = -1) {}
    std::string GetThawReasonByAbilityType(const AppExecFwk::AbilityInfo &abilityInfo)
    {
        return "";
    }
    void ReportAbilityIntentExemptionInfoToRSS(int32_t callerUid, int32_t callerPid) {}
};
} // namespace AAFwk
} // namespace OHOS
#endif
