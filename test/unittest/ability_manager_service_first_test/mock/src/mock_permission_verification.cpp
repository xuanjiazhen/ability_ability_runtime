/*
 * Copyright (c) 2022 - 2025 Huawei Device Co., Ltd.
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

#include "ability_manager_errors.h"
#include "permission_verification.h"
#include "mock_my_flag.h"

namespace OHOS {
namespace AAFwk {

bool PermissionVerification::VerifyPermissionByTokenId(const int &tokenId, const std::string &permissionName) const
{
    return !!(MyFlag::flag_);
}

bool PermissionVerification::VerifyCallingPermission(
    const std::string &permissionName, const uint32_t specifyTokenId) const
{
    return !!(MyFlag::flag_);
}

bool PermissionVerification::IsSACall() const
{
    return (MyFlag::flag_ & MyFlag::FLAG::IS_SA_CALL);
}

bool PermissionVerification::IsSACallByTokenId(uint32_t callerTokenId) const
{
    return !!(MyFlag::flag_);
}

bool PermissionVerification::IsShellCall() const
{
    return (MyFlag::flag_ & MyFlag::FLAG::IS_SHELL_CALL);
}

bool PermissionVerification::IsShellCallByTokenId(uint32_t callerTokenId) const
{
    return !!(MyFlag::flag_);
}

bool PermissionVerification::CheckSpecificSystemAbilityAccessPermission(const std::string &processName) const
{
    return !!(MyFlag::flag_);
}

bool PermissionVerification::CheckObserverCallerPermission() const
{
    return !!(MyFlag::flag_);
}

bool PermissionVerification::VerifyRunningInfoPerm() const
{
    return !!(MyFlag::flag_);
}

bool PermissionVerification::VerifyCustomSandbox(uint32_t accessTokenId) const
{
    return !!(MyFlag::flag_);
}

bool PermissionVerification::VerifyControllerPerm() const
{
    return !!(MyFlag::flag_);
}

#ifdef WITH_DLP
bool PermissionVerification::VerifyDlpPermission(Want &want) const
{
    return !!(MyFlag::flag_);
}
#endif // WITH_DLP
int PermissionVerification::VerifyAccountPermission() const
{
    return MyFlag::flag_;
}

bool PermissionVerification::VerifyMissionPermission() const
{
    return !!(MyFlag::flag_);
}

int PermissionVerification::VerifyAppStateObserverPermission() const
{
    return MyFlag::flag_;
}

int32_t PermissionVerification::VerifyUpdateConfigurationPerm() const
{
    return static_cast<int32_t>(MyFlag::flag_);
}

bool PermissionVerification::VerifyInstallBundlePermission() const
{
    return !!(MyFlag::flag_);
}

bool PermissionVerification::VerifyGetBundleInfoPrivilegedPermission() const
{
    return !!(MyFlag::flag_);
}

bool PermissionVerification::VerifyStartRecentAbilityPermission() const
{
    return !!(MyFlag::flag_);
}

int PermissionVerification::CheckCallDataAbilityPermission(const VerificationInfo &verificationInfo, bool isShell) const
{
    return MyFlag::flag_;
}

int PermissionVerification::CheckCallServiceAbilityPermission(const VerificationInfo &verificationInfo) const
{
    return ERR_OK;
}

int PermissionVerification::CheckCallAbilityPermission(const VerificationInfo &verificationInfo,
    bool isCallByShortcut) const
{
    return MyFlag::flag_;
}

int PermissionVerification::CheckCallServiceExtensionPermission(const VerificationInfo &verificationInfo) const
{
    return ERR_OK;
}

int PermissionVerification::CheckStartByCallPermission(const VerificationInfo &verificationInfo) const
{
    return ERR_OK;
}

unsigned int PermissionVerification::GetCallingTokenID() const
{
    return static_cast<unsigned int>(MyFlag::flag_);
}

bool PermissionVerification::JudgeStartInvisibleAbility(const uint32_t accessTokenId, const bool visible,
    const uint32_t specifyTokenId) const
{
    return !!(MyFlag::flag_);
}

bool PermissionVerification::JudgeStartAbilityFromBackground(const bool isBackgroundCall) const
{
    return !!(MyFlag::flag_);
}

bool PermissionVerification::JudgeAssociatedWakeUp(const uint32_t accessTokenId, const bool associatedWakeUp) const
{
    return !!(MyFlag::flag_);
}

int PermissionVerification::JudgeInvisibleAndBackground(const VerificationInfo &verificationInfo,
    bool isCallByShortcut) const
{
    return MyFlag::flag_;
}

bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI() const
{
    return true;
}

bool PermissionVerification::VerifyPrepareTerminatePermission() const
{
    return true;
}

bool PermissionVerification::VerifyPrepareTerminatePermission(const int &tokenId) const
{
    return true;
}

bool PermissionVerification::VerifyShellStartExtensionType(int32_t type) const
{
    return MyFlag::flag_;
}

bool PermissionVerification::VerifyPreloadApplicationPermission() const
{
    return MyFlag::flag_;
}

bool PermissionVerification::VerifyPreStartAtomicServicePermission() const
{
    return MyFlag::flag_;
}

bool PermissionVerification::IsSystemAppCall() const
{
    return true;
}

bool PermissionVerification::IsSystemAppCallByTokenId(uint32_t callerTokenId) const
{
    return true;
}

bool PermissionVerification::VerifyBackgroundCallPermission(const bool isBackgroundCall) const
{
    return MyFlag::flag_;
}

bool PermissionVerification::VerifyKillProcessDependedOnWebPermission() const
{
    return true;
}

bool PermissionVerification::VerifyBlockAllAppStartPermission() const
{
    return true;
}

bool PermissionVerification::VerifyStartUIAbilityToHiddenPermission() const
{
    return true;
}

bool PermissionVerification::VerifySuperviseKiaServicePermission() const
{
    return true;
}

bool PermissionVerification::VerifyStartLocalDebug(int32_t tokenId) const
{
    return true;
}

bool PermissionVerification::VerifyStartSelfUIAbility(int tokenId) const
{
    return true;
}

bool PermissionVerification::VerifyFusionAccessPermission() const
{
    return true;
}
}  // namespace AAFwk
}  // namespace OHOS