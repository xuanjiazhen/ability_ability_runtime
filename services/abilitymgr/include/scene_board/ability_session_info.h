/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_SESSION_INFO_H
#define OHOS_ABILITY_RUNTIME_ABILITY_SESSION_INFO_H

#include <cstdint>
#include <string>

#include "want_params.h"

struct AbilitySessionInfo {
    std::string callerBundleName;
    uint32_t callerTokenId = 0;
    bool isWebSandBoxClone = false;
    int32_t sandBoxCloneIndex = 0;
    std::string creatorBundleName;
    uint32_t specifyTokenId = 0;
    int32_t callerUid = 0;
    int32_t callerPid = 0;
    std::string callerNativeName;
    bool hasCallerNativeName = false;
    std::string callerAbilityName;
    std::string callerAppId;
    std::string callerAppIdentifier;
    int32_t callerAppCloneIndex = 0;
    // Snapshot of the insight intent identity params taken from the sanitized Want before the
    // SCB relay; verified against the Want when SCB calls back into AMS.
    OHOS::AAFwk::WantParams insightIntentParams;
    bool hasInsightIntent = false;
};

#endif // OHOS_ABILITY_RUNTIME_ABILITY_SESSION_INFO_H
