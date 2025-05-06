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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_UTIL_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_PERMISSION_UTIL_H

#include <memory>

#include "ability_record.h"
#include "ffrt.h"
#include "iremote_object.h"
#include "nocopyable.h"
#include "permission_verification.h"

namespace OHOS {
namespace AAFwk {
struct AbilityRequest;

class Want;

class StartSelfUIAbilityRecordGuard {
public:
    StartSelfUIAbilityRecordGuard() = delete;

    StartSelfUIAbilityRecordGuard(pid_t pid, int32_t tokenId) {}

    ~StartSelfUIAbilityRecordGuard() {}
};

class AbilityPermissionUtil {
public:
    static AbilityPermissionUtil &GetInstance()
    {
        static AbilityPermissionUtil instance;
        return instance;
    }

    bool IsDominateScreen(const Want &want, bool isPendingWantCaller)
    {
        return false;
    }

    int32_t CheckMultiInstanceAndAppClone(Want &want, int32_t userId, int32_t appIndex,
        sptr<IRemoteObject> callerToken)
    {
        return 0;
    }

    int32_t CheckStartRecentAbility(const Want &want, AbilityRequest &request)
    {
        return 0;
    }

    int32_t CheckStartByCallPermissionOrHasFloatingWindow(
        const PermissionVerification::VerificationInfo &verificationInfo, const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }

    int32_t CheckCallServiceExtensionPermissionOrHasFloatingWindow(
        const PermissionVerification::VerificationInfo &verificationInfo, const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }

    int32_t CheckCallAbilityPermissionOrHasFloatingWindow(
        const PermissionVerification::VerificationInfo &verificationInfo, const sptr<IRemoteObject> &callerToken,
        bool isCallByShortcut)
    {
        return 0;
    }

    bool IsStartSelfUIAbility()
    {
        return false;
    }

    int32_t CheckPrepareTerminateEnable(const std::shared_ptr<AbilityRecord> &abilityRecord)
    {
        return 0;
    }

    int32_t CheckStartCallHasFloatingWindow(const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }
};
} // namespace AAFwk
} // namespace OHOS
#endif