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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_PERMISSION_VERIFICATION_H
#define MOCK_OHOS_ABILITY_RUNTIME_PERMISSION_VERIFICATION_H

#include "singleton.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class PermissionVerification : public DelayedSingleton<PermissionVerification> {
public:
struct VerificationInfo {
    bool visible = false;
    bool isBackgroundCall = true;
    bool associatedWakeUp = false;
    bool withContinuousTask = false;
    uint32_t accessTokenId = 0;
    int32_t apiTargetVersion = 0;
    uint32_t specifyTokenId = 0;
};

    PermissionVerification() = default;
    ~PermissionVerification() = default;

    bool VerifyPermissionByTokenId(const int &tokenId, const std::string &permissionName) const
    {
        return false;
    }

    bool VerifyCallingPermission(const std::string &permissionName, const uint32_t specifyTokenId = 0) const
    {
        return false;
    }

    bool IsSACall() const
    {
        return false;
    }

    bool IsSACallByTokenId(uint32_t callerTokenId) const
    {
        return false;
    }

    bool IsShellCall() const
    {
        return false;
    }
    bool IsShellCallByTokenId(uint32_t callerTokenId) const
    {
        return false;
    }

    bool CheckSpecificSystemAbilityAccessPermission(const std::string &processName) const
    {
        return false;
    }

    bool CheckObserverCallerPermission() const
    {
        return false;
    }

    bool VerifyRunningInfoPerm() const
    {
        return false;
    }

    bool VerifyControllerPerm() const
    {
        return false;
    }

    int VerifyAccountPermission() const
    {
        return 0;
    }

    bool VerifyMissionPermission() const
    {
        return false;
    }

    int CheckCallDataAbilityPermission(const VerificationInfo &verificationInfo, bool isShell) const
    {
        return 0;
    }

    int CheckCallServiceAbilityPermission(const VerificationInfo &verificationInfo) const
    {
        return 0;
    }

    int CheckCallAbilityPermission(const VerificationInfo &verificationInfo, bool isCallByShortcut = false) const
    {
        return 0;
    }

    int CheckCallServiceExtensionPermission(const VerificationInfo &verificationInfo) const
    {
        return 0;
    }

    int CheckStartByCallPermission(const VerificationInfo &verificationInfo) const
    {
        return 0;
    }

    bool JudgeCallerIsAllowedToUseSystemAPI() const
    {
        return false;
    }

    bool IsSystemAppCall() const
    {
        return false;
    }

    bool IsSystemAppCallByTokenId(uint32_t callerTokenId) const
    {
        return false;
    }

    bool VerifyShellStartExtensionType(int32_t type) const
    {
        return false;
    }

    bool VerifyPreStartAtomicServicePermission() const
    {
        return false;
    }

    bool VerifyBlockAllAppStartPermission() const
    {
        return false;
    }

    bool VerifyStartUIAbilityToHiddenPermission() const
    {
        return false;
    }

    bool VerifyStartLocalDebug(int32_t tokenId) const
    {
        return false;
    }

    bool VerifyStartSelfUIAbility(int tokenId) const
    {
        return false;
    }

    bool VerifyFusionAccessPermission() const
    {
        return false;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
