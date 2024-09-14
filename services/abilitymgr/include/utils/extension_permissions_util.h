/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_EXTENSION_PERMISSIONS_UTIL_H
#define OHOS_EXTENSION_PERMISSIONS_UTIL_H

#include "extension_ability_info.h"
#include "ability_record.h"
#include "permission_verification.h"


namespace OHOS {
namespace AAFwk {
class ExtensionPermissionsUtil final {
public:
    static ExtensionPermissionsUtil &GetInstance();
    static bool CheckSAPermission(const AppExecFwk::ExtensionAbilityType &extensionType);
    static bool CheckSAPermissionMore(const AppExecFwk::ExtensionAbilityType &extensionType);

    /**
     * Check if Caller is allowed to start ServiceAbility(FA) or ServiceExtension(Stage) or DataShareExtension(Stage).
     *
     * @param abilityRequest, abilityRequest.
     * @param bool, isParamStartAbilityEnable_.
     * @return Returns whether the caller is allowed to start Service.
     */
    int CheckCallServicePermission(const AbilityRequest &abilityRequest, bool isParamStartAbilityEnable_ = false);

        /**
     * Check if Caller is allowed to start DataAbility(FA)
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start DataAbility.
     */
    int CheckCallDataAbilityPermission(AbilityRequest &abilityRequest, bool isShell, bool IsSACall = false, bool whiteListassociatedWakeUpFlag_ = true);

    /**
     * Check if Caller is allowed to start ServiceExtension(Stage) or DataShareExtension(Stage).
     *
     * @param abilityRequest, abilityRequest.
     * @param bool, isParamStartAbilityEnable_.
     * @return Returns whether the caller is allowed to start ServiceExtension.
     */
    int CheckCallServiceExtensionPermission(const AbilityRequest &abilityRequest, bool isParamStartAbilityEnable_ = false);

    /**
     * Check if Caller is allowed to start other Extension(Stage).
     *
     * @param abilityRequest, abilityRequest.
     * @param bool, isParamStartAbilityEnable_.
     * @return Returns whether the caller is allowed to start OtherExtension.
     */
    int CheckCallOtherExtensionPermission(const AbilityRequest &abilityRequest, bool isParamStartAbilityEnable_ = false);

private:
    /**
     * @brief Check some specified uiextension type should be a system app.
     * Consider expanding it to table-driven in the future.
     *
     * @param abilityRequest The ability request.
     * @return Returns ERR_OK when allowed, others when check failed.
     */
    int CheckUIExtensionPermission(const AbilityRequest &abilityRequest);

    bool CheckUIExtensionCallerIsForeground(const AbilityRequest &abilityRequest);

    bool CheckUIExtensionCallerIsUIAbility(const AbilityRequest &abilityRequest);

    /**
     * Check if Caller is allowed to start ServiceAbility(FA).
     *
     * @param abilityRequest, abilityRequest.
     * @return Returns whether the caller is allowed to start ServiceAbility.
     */
    int CheckCallServiceAbilityPermission(const AbilityRequest &abilityRequest);

    AAFwk::PermissionVerification::VerificationInfo CreateVerificationInfo(
        const AbilityRequest &abilityRequest, bool isData = false, bool isShell = false, bool isSA = false, bool whiteListassociatedWakeUpFlag_ = true);
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_EXTENSION_PERMISSIONS_UTIL_H