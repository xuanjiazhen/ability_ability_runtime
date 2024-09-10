/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "utils/extension_permissions_util.h"

#include "hilog_tag_wrapper.h"
#include "permission_verification.h"

namespace OHOS {
namespace AAFwk {

bool ExtensionPermissionsUtil::CheckSAPermission(const AppExecFwk::ExtensionAbilityType &extensionType)
{
    auto checkRet = false;
    if (!PermissionVerification::GetInstance()->IsSACall()) {
        return true;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "CheckSAPermission, extensionType: %{public}d.", extensionType);
    if (extensionType == AppExecFwk::ExtensionAbilityType::FORM) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_FORM_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::WORK_SCHEDULER) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_WORK_SCHEDULER_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::INPUTMETHOD) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_INPUT_METHOD_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::ACCESSIBILITY) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_ACCESSIBILITY_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::STATICSUBSCRIBER) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_STATIC_SUBSCRIBER_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::WALLPAPER) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_WALLPAPER_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::BACKUP) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_BACKUP_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::ENTERPRISE_ADMIN) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_ENTERPRISE_ADMIN_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::PRINT) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_PRINT_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::VPN) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_VPN_EXTENSION");
    } else {
        checkRet = CheckSAPermissionMore(extensionType);
    }
    if (!checkRet) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "SA connect permission verification failed.");
        return false;
    }

    return true;
}

bool ExtensionPermissionsUtil::CheckSAPermissionMore(const AppExecFwk::ExtensionAbilityType &extensionType)
{
    auto checkRet = false;
    if (extensionType == AppExecFwk::ExtensionAbilityType::FILEACCESS_EXTENSION) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_FILE_ACCESS_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::REMOTE_NOTIFICATION) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_REMOTE_NOTIFICATION_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::REMOTE_LOCATION) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_REMOTE_LOCATION_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::DRIVER) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_DRIVER_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::PUSH) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_PUSH_EXTENSION");
    } else if (extensionType == AppExecFwk::ExtensionAbilityType::VOIP) {
        checkRet = PermissionVerification::GetInstance()->VerifyCallingPermission(
            "ohos.permission.CONNECT_VOIP_EXTENSION");
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "No need connect permission for extension type %{public}d.", extensionType);
        return true;
    }

    return checkRet;
}

int ExtensionPermissionsUtil::CheckCallServicePermission(const AbilityRequest &abilityRequest)
{
    if (abilityRequest.want.GetIntParam(Want::PARAM_RESV_CALLER_UID, IPCSkeleton::GetCallingUid()) ==
        AppUtils::GetInstance().GetCollaboratorBrokerUID() &&
        abilityRequest.want.GetElement().GetBundleName() == AppUtils::GetInstance().GetBrokerDelegateBundleName()) {
        auto collaborator = GetCollaborator(CollaboratorType::RESERVE_TYPE);
        if (collaborator != nullptr) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Collaborator CheckCallAbilityPermission");
            return collaborator->CheckCallAbilityPermission(abilityRequest.want);
        }
    }
    if (abilityRequest.abilityInfo.isStageBasedModel) {
        auto extensionType = abilityRequest.abilityInfo.extensionAbilityType;
        TAG_LOGD(AAFwkTag::ABILITYMGR, "extensionType is %{public}d.", static_cast<int>(extensionType));
        if (extensionType == AppExecFwk::ExtensionAbilityType::SERVICE ||
            extensionType == AppExecFwk::ExtensionAbilityType::DATASHARE ||
            extensionType == AppExecFwk::ExtensionAbilityType::UI_SERVICE) {
            return CheckCallServiceExtensionPermission(abilityRequest);
        } else {
            return CheckCallOtherExtensionPermission(abilityRequest);
        }
    } else {
        return CheckCallServiceAbilityPermission(abilityRequest);
    }
}

int ExtensionPermissionsUtil::CheckCallDataAbilityPermission(AbilityRequest &abilityRequest, bool isShell, bool isSACall)
{
    abilityRequest.appInfo = abilityRequest.abilityInfo.applicationInfo;
    abilityRequest.uid = abilityRequest.appInfo.uid;
    if (abilityRequest.appInfo.name.empty() || abilityRequest.appInfo.bundleName.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid app info acquiring");
        return ERR_INVALID_VALUE;
    }
    if (abilityRequest.abilityInfo.type != AppExecFwk::AbilityType::DATA) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "BMS query result not data ability");
        return ERR_WRONG_INTERFACE_CALL;
    }

    AAFwk::PermissionVerification::VerificationInfo verificationInfo = CreateVerificationInfo(abilityRequest,
        true, isShell, isSACall);
    if (isShell) {
        verificationInfo.isBackgroundCall = true;
    }
    if (!isShell && IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall, true) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }
    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallDataAbilityPermission(verificationInfo,
        isShell);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "without start DataAbility permission");
        return result;
    }

    return ERR_OK;
}

int ExtensionPermissionsUtil::CheckCallServiceExtensionPermission(const AbilityRequest &abilityRequest)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "begin");

    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    verificationInfo.withContinuousTask = IsBackgroundTaskUid(IPCSkeleton::GetCallingUid());
    verificationInfo.isBackgroundCall = false;
    if (isParamStartAbilityEnable_) {
        bool stopContinuousTaskFlag = ShouldPreventStartAbility(abilityRequest);
        if (stopContinuousTaskFlag) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "no have permission to start ServiceExtension");
            return CHECK_PERMISSION_FAILED;
        }
    }

    if (abilityRequest.abilityInfo.extensionAbilityType == AppExecFwk::ExtensionAbilityType::UI_SERVICE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "background startup UI_SERVICE");
        verificationInfo.isBackgroundCall = true;
        std::shared_ptr<AbilityRecord> callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
        if (callerAbility) {
            verificationInfo.apiTargetVersion = callerAbility->GetApplicationInfo().apiTargetVersion;
        }
        if (IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall) != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "not background startup UI_SERVICE");
            return ERR_INVALID_VALUE;
        }
    }

    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceExtensionPermission(verificationInfo);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "without start serviceExtension or sataShareExtension permission");
    }
    return result;
}


int ExtensionPermissionsUtil::CheckCallOtherExtensionPermission(const AbilityRequest &abilityRequest)
{
    if (IPCSkeleton::GetCallingUid() != AppUtils::GetInstance().GetCollaboratorBrokerUID() &&
        AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
        return ERR_OK;
    }

    auto extensionType = abilityRequest.abilityInfo.extensionAbilityType;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OtherExtension type: %{public}d.", static_cast<int32_t>(extensionType));
    if (system::GetBoolParameter(DEVELOPER_MODE_STATE, false) &&
        PermissionVerification::GetInstance()->VerifyShellStartExtensionType(static_cast<int32_t>(extensionType))) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "CheckCallOtherExtensionPermission, allow aa start with debug mode.");
        return ERR_OK;
    }
    if (extensionType == AppExecFwk::ExtensionAbilityType::WINDOW) {
        CHECK_CALLER_IS_SYSTEM_APP;
        return ERR_OK;
    }
    if (extensionType == AppExecFwk::ExtensionAbilityType::ADS_SERVICE) {
        return ERR_OK;
    }
    if (extensionType == AppExecFwk::ExtensionAbilityType::AUTO_FILL_PASSWORD ||
        extensionType == AppExecFwk::ExtensionAbilityType::AUTO_FILL_SMART) {
        if (!abilityRequest.appInfo.isSystemApp) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "application requesting call isn't system application");
            return CHECK_PERMISSION_FAILED;
        }
        std::string jsonDataStr = abilityRequest.want.GetStringParam(WANT_PARAMS_VIEW_DATA_KEY);
        AbilityBase::ViewData viewData;
        viewData.FromJsonString(jsonDataStr.c_str());
        if (!CheckCallingTokenId(viewData.bundleName)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "no %{public}s called, no allowed", viewData.bundleName.c_str());
            return ERR_WRONG_INTERFACE_CALL;
        }
        return ERR_OK;
    }
    if (AAFwk::UIExtensionUtils::IsUIExtension(extensionType)) {
        return CheckUIExtensionPermission(abilityRequest);
    }
    if (extensionType == AppExecFwk::ExtensionAbilityType::VPN) {
        return ERR_OK;
    }

    const std::string fileAccessPermission = "ohos.permission.FILE_ACCESS_MANAGER";
    if (extensionType == AppExecFwk::ExtensionAbilityType::FILEACCESS_EXTENSION &&
        AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(fileAccessPermission)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Temporary, FILEACCESS_EXTENSION use serviceExtension start-up rule.");
        return CheckCallServiceExtensionPermission(abilityRequest);
    }

    TAG_LOGE(AAFwkTag::ABILITYMGR, "not SA, can't start other extension");
    return CHECK_PERMISSION_FAILED;
}

int ExtensionPermissionsUtil::CheckUIExtensionPermission(const AbilityRequest &abilityRequest)
{
    if (abilityRequest.want.HasParameter(AAFwk::SCREEN_MODE_KEY)) {
        // If started by embedded atomic service, allow it.
        return ERR_OK;
    }

    auto extensionType = abilityRequest.abilityInfo.extensionAbilityType;
    if (AAFwk::UIExtensionUtils::IsSystemUIExtension(extensionType)) {
        auto callerRecord = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
        if (callerRecord == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid caller");
            return NO_FOUND_ABILITY_BY_CALLER;
        }

        if (!abilityRequest.appInfo.isSystemApp && !callerRecord->GetApplicationInfo().isSystemApp) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "bundle %{public}s want start or caller bundle %{public}s "
                "isn't system app, type %{public}d not allowed", abilityRequest.appInfo.bundleName.c_str(),
                callerRecord->GetApplicationInfo().bundleName.c_str(), extensionType);
            return CHECK_PERMISSION_FAILED;
        }
    }

    if (AAFwk::UIExtensionUtils::IsSystemCallerNeeded(extensionType)) {
        auto callerRecord = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
        if (callerRecord == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid caller");
            return NO_FOUND_ABILITY_BY_CALLER;
        }

        if (!callerRecord->GetApplicationInfo().isSystemApp
            && !AAFwk::PermissionVerification::GetInstance()->IsSACall()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                     "bundle %{public}s want start but caller bundle %{public}s "
                     "isn't system app, type %{public}d not allowed",
                     abilityRequest.appInfo.bundleName.c_str(), callerRecord->GetApplicationInfo().bundleName.c_str(),
                     extensionType);
            return CHECK_PERMISSION_FAILED;
        }
    }

    if (!CheckUIExtensionCallerIsForeground(abilityRequest)) {
        return CHECK_PERMISSION_FAILED;
    }

    if (!CheckUIExtensionCallerPidByHostWindowId(abilityRequest)) {
        return ERR_INVALID_CALLER;
    }

    return ERR_OK;
}

bool ExtensionPermissionsUtil::CheckUIExtensionCallerIsForeground(const AbilityRequest &abilityRequest)
{
    if (!CheckUIExtensionCallerIsUIAbility(abilityRequest)) {
        // Check only if the caller is uiability, if caller is not a uiability, don't check.
        return true;
    }

    bool isBackgroundCall = true;
    auto ret = IsCallFromBackground(abilityRequest, isBackgroundCall);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "start uea when background");
        return false;
    }

    if (!isBackgroundCall) {
        return true;
    }

    auto callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerAbility != nullptr) {
        if (UIExtensionUtils::IsUIExtension(callerAbility->GetAbilityInfo().extensionAbilityType)) {
            auto tokenId = callerAbility->GetApplicationInfo().accessTokenId;
            bool isFocused = false;
            if (CheckUIExtensionIsFocused(tokenId, isFocused) == ERR_OK && isFocused) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "Root caller is foreground");
                return true;
            }
        }

        if (callerAbility->IsSceneBoard()) {
            return true;
        }
    }

    if (PermissionVerification::GetInstance()->VerifyCallingPermission(
        PermissionConstants::PERMISSION_START_ABILITIES_FROM_BACKGROUND)) {
        return true;
    }

    TAG_LOGE(AAFwkTag::ABILITYMGR, "caller app not foreground, can't start %{public}s",
        abilityRequest.want.GetElement().GetURI().c_str());
    return false;
}

bool ExtensionPermissionsUtil::CheckUIExtensionCallerIsUIAbility(const AbilityRequest &abilityRequest)
{
    auto callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerAbility == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "caller ability invalid");
        return false;
    }

    if (callerAbility->GetAbilityInfo().type == AppExecFwk::AbilityType::PAGE) {
        return true;
    }

    if (UIExtensionUtils::IsUIExtension(callerAbility->GetAbilityInfo().extensionAbilityType)) {
        callerAbility = GetUIExtensionRootCaller(abilityRequest.callerToken, abilityRequest.userId);
        if (callerAbility != nullptr && callerAbility->GetAbilityInfo().type == AppExecFwk::AbilityType::PAGE) {
            return true;
        }
    }

    return false;
}

int ExtensionPermissionsUtil::CheckCallServiceAbilityPermission(const AbilityRequest &abilityRequest)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call");
    AAFwk::PermissionVerification::VerificationInfo verificationInfo = CreateVerificationInfo(abilityRequest);
    if (IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallServiceAbilityPermission(verificationInfo);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "without start serviceAbility permission");
    }
    return result;
}

AAFwk::PermissionVerification::VerificationInfo ExtensionPermissionsUtil::CreateVerificationInfo(
    const AbilityRequest &abilityRequest, bool isData, bool isShell, bool isSA)
{
    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    verificationInfo.withContinuousTask = IsBackgroundTaskUid(IPCSkeleton::GetCallingUid());
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call ServiceAbility or DataAbility, target bundleName: %{public}s.",
        abilityRequest.appInfo.bundleName.c_str());
    if (whiteListassociatedWakeUpFlag_ &&
        abilityRequest.appInfo.bundleName == BUNDLE_NAME_SETTINGSDATA) {
        TAG_LOGD(AAFwkTag::ABILITYMGR,
            "Call ServiceAbility or DataAbility, target bundle in white-list, allow associatedWakeUp.");
        verificationInfo.associatedWakeUp = true;
    } else {
        verificationInfo.associatedWakeUp = abilityRequest.appInfo.associatedWakeUp;
    }
    if (!isData) {
        isSA = AAFwk::PermissionVerification::GetInstance()->IsSACall();
        isShell = AAFwk::PermissionVerification::GetInstance()->IsShellCall();
    }
    if (isSA || isShell) {
        return verificationInfo;
    }
    std::shared_ptr<AbilityRecord> callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerAbility) {
        verificationInfo.apiTargetVersion = callerAbility->GetApplicationInfo().apiTargetVersion;
    }

    return verificationInfo;
}

} // namespace AAFwk
} // namespace OHOS
