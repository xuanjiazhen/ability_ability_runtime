/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "permission_verification.h"
#include "mock_my_status.h"

namespace OHOS {
namespace AAFwk {

enum FLAG {
    IS_SA_CALL = 1,
    IS_SHELL_CALL,
    IS_SA_AND_SHELL_CALL,
};
bool PermissionVerification::VerifyCallingPermission(const std::string &permissionName,
    const uint32_t specifyTokenId) const
{
    return !!(MyStatus::GetInstance().permPermission_);
}
bool PermissionVerification::IsSACall() const
{
    return (MyStatus::GetInstance().permPermission_ & FLAG::IS_SA_CALL);
}
bool PermissionVerification::IsShellCall() const
{
    return (MyStatus::GetInstance().permPermission_ & FLAG::IS_SHELL_CALL);
}
bool PermissionVerification::VerifyRunningInfoPerm() const
{
    return !!(MyStatus::GetInstance().permPermission_);
}
bool PermissionVerification::VerifyControllerPerm() const
{
    return !!(MyStatus::GetInstance().permPermission_);
}
#ifdef WITH_DLP
bool PermissionVerification::VerifyDlpPermission(Want &want) const
{
    return !!(MyStatus::GetInstance().permPermission_);
}
#endif // WITH_DLP
int PermissionVerification::VerifyAccountPermission() const
{
    return MyStatus::GetInstance().permPermission_;
}
bool PermissionVerification::VerifyMissionPermission() const
{
    return !!(MyStatus::GetInstance().permPermission_);
}
int PermissionVerification::VerifyAppStateObserverPermission() const
{
    return MyStatus::GetInstance().permPermission_;
}
int32_t PermissionVerification::VerifyUpdateConfigurationPerm() const
{
    return static_cast<int32_t>(MyStatus::GetInstance().permPermission_);
}
bool PermissionVerification::VerifyInstallBundlePermission() const
{
    return !!(MyStatus::GetInstance().permPermission_);
}
bool PermissionVerification::VerifyGetBundleInfoPrivilegedPermission() const
{
    return !!(MyStatus::GetInstance().permPermission_);
}
int PermissionVerification::CheckCallDataAbilityPermission(const VerificationInfo &verificationInfo, bool isShell) const
{
    return MyStatus::GetInstance().permPermission_;
}
int PermissionVerification::CheckCallServiceAbilityPermission(const VerificationInfo &verificationInfo) const
{
    return MyStatus::GetInstance().permPermission_;
}
int PermissionVerification::CheckCallServiceExtensionPermission(const VerificationInfo &verificationInfo) const
{
    return MyStatus::GetInstance().permPermission_;
}
int PermissionVerification::CheckStartByCallPermission(const VerificationInfo &verificationInfo) const
{
    return MyStatus::GetInstance().permPermission_;
}
unsigned int PermissionVerification::GetCallingTokenID() const
{
    return static_cast<unsigned int>(MyStatus::GetInstance().permPermission_);
}
bool PermissionVerification::JudgeStartAbilityFromBackground(const bool isBackgroundCall) const
{
    return !!(MyStatus::GetInstance().permPermission_);
}
bool PermissionVerification::JudgeAssociatedWakeUp(const uint32_t accessTokenId, const bool associatedWakeUp) const
{
    return !!(MyStatus::GetInstance().permPermission_);
}
bool PermissionVerification::JudgeCallerIsAllowedToUseSystemAPI() const
{
    return MyStatus::GetInstance().perJudgeCallerIsAllowedToUseSystemAPI_;
}
bool PermissionVerification::IsSystemAppCall() const
{
    return true;
}
bool PermissionVerification::VerifyPreloadApplicationPermission() const
{
    return MyStatus::GetInstance().retVerifyPreloadApplicationPermission_;
}
bool PermissionVerification::VerifyStartLocalDebug(int32_t tokenId) const
{
    return false;
}
bool PermissionVerification::CheckSpecificSystemAbilityAccessPermission(const std::string &processName) const
{
    return false;
}
bool PermissionVerification::VerifyPermissionByTokenId(const int &tokenId, const std::string &permissionName) const
{
    return false;
}
bool PermissionVerification::CheckObserverCallerPermission() const
{
    return false;
}
bool PermissionVerification::VerifyPreStartAtomicServicePermission() const
{
    return false;
}
bool PermissionVerification::VerifyBlockAllAppStartPermission() const
{
    return false;
}
bool PermissionVerification::VerifyStartUIAbilityToHiddenPermission() const
{
    return false;
}
bool PermissionVerification::VerifyShellStartExtensionType(int32_t type) const
{
    return false;
}
int PermissionVerification::CheckCallAbilityPermission(const VerificationInfo &verificationInfo,
    bool isCallByShortcut) const
{
    return 0;
}
bool PermissionVerification::IsSACallByTokenId(uint32_t callerTokenId) const
{
    return false;
}
bool PermissionVerification::IsSystemAppCallByTokenId(uint32_t callerTokenId) const
{
    return false;
}
bool PermissionVerification::IsShellCallByTokenId(uint32_t callerTokenId) const
{
    return false;
}
bool PermissionVerification::VerifyStartSelfUIAbility(int tokenId) const
{
    return false;
}
bool PermissionVerification::VerifyBackgroundCallPermission(const bool isBackgroundCall) const
{
    return false;
}
bool PermissionVerification::VerifyPrepareTerminatePermission(const int &tokenId) const
{
    return false;
}
bool PermissionVerification::VerifyStartRecentAbilityPermission() const
{
    return false;
}
} // namespace AAFwk
} // namespace OHOS
