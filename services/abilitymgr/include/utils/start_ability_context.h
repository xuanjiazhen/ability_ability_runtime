/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_START_ABILITY_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_START_ABILITY_CONTEXT_H

#include <memory>

#include "ability_info.h"

namespace OHOS {
namespace AAFwk {
struct StartAbilityInfo;  // forward declaration (full type in start_ability_utils.h)

// Centralized ERMS constant (replaces the anonymous-namespace duplicates in
// start_ability_utils.cpp, ecological_rule_interceptor.cpp and dialog_session_manager.cpp).
inline constexpr int32_t ERMS_ISALLOW_RESULTCODE = 10;

struct StartAbilityContext {
    bool skipCrowTest = false;
    bool skipErms = false;
    bool ermsSupportBackToCallerFlag = false;
    bool isWantWithAppCloneIndex = false;
    bool isSandBoxClone = false;
    int32_t ermsResultCode = ERMS_ISALLOW_RESULTCODE;

    std::shared_ptr<StartAbilityInfo> startAbilityInfo;        // target: carries 6 sub-fields
    std::shared_ptr<AppExecFwk::AbilityInfo> callerAbilityInfo; // caller: only abilityInfo is used

    bool ConsumeSkipCrowTest()
    {
        bool v = skipCrowTest;
        skipCrowTest = false;
        return v;
    }
    bool ConsumeSkipErms()
    {
        bool v = skipErms;
        skipErms = false;
        return v;
    }

    bool ConsumeIsWantWithAppCloneIndex()
    {
        bool v = isWantWithAppCloneIndex;
        isWantWithAppCloneIndex = false;
        return v;
    }

    void SyncFrom(const StartAbilityContext &other)
    {
        ermsResultCode = other.ermsResultCode;
        ermsSupportBackToCallerFlag = other.ermsSupportBackToCallerFlag;
        skipCrowTest = other.skipCrowTest;
        skipErms = other.skipErms;
    }
};
} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_START_ABILITY_CONTEXT_H
