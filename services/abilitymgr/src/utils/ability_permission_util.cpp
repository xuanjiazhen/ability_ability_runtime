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

#include "utils/ability_permission_util.h"

#include "ability_connect_manager.h"
#include "ability_info.h"
#include "ability_util.h"
#include "app_utils.h"
#include "accesstoken_kit.h"
#include "hitrace_meter.h"
#include "insight_intent_execute_param.h"
#include "ipc_skeleton.h"
#include "permission_constants.h"
#include "permission_verification.h"

using OHOS::Security::AccessToken::AccessTokenKit;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* IS_DELEGATOR_CALL = "isDelegatorCall";
constexpr const char* SETTINGS = "settings";
constexpr int32_t BASE_USER_RANGE = 200000;
}

AbilityPermissionUtil &AbilityPermissionUtil::GetInstance()
{
    static AbilityPermissionUtil instance;
    return instance;
}

inline bool AbilityPermissionUtil::IsDelegatorCall(const AppExecFwk::RunningProcessInfo &processInfo,
    const AbilityRequest &abilityRequest) const
{
    /*  To make sure the AbilityDelegator is not counterfeited
     *   1. The caller-process must be test-process
     *   2. The callerToken must be nullptr
     */
    if (processInfo.isTestProcess &&
        !abilityRequest.callerToken && abilityRequest.want.GetBoolParam(IS_DELEGATOR_CALL, false)) {
        return true;
    }
    return false;
}

bool AbilityPermissionUtil::IsDominateScreen(const Want &want, bool isPendingWantCaller)
{
    if (!isPendingWantCaller &&
        !PermissionVerification::GetInstance()->IsSACall() && !PermissionVerification::GetInstance()->IsShellCall()) {
        auto callerPid = IPCSkeleton::GetCallingPid();
        AppExecFwk::RunningProcessInfo processInfo;
        DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(callerPid, processInfo);
        bool isDelegatorCall = processInfo.isTestProcess && want.GetBoolParam(IS_DELEGATOR_CALL, false);
        if (isDelegatorCall || InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "not dominate screen.");
            return false;
        }
        // add temporarily
        std::string bundleName = want.GetElement().GetBundleName();
        std::string abilityName = want.GetElement().GetAbilityName();
        bool withoutSettings = bundleName.find(SETTINGS) == std::string::npos &&
            abilityName.find(SETTINGS) == std::string::npos;
        if (withoutSettings && AppUtils::GetInstance().IsAllowStartAbilityWithoutCallerToken(bundleName, abilityName)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "not dominate screen, allow.");
            return false;
        } else if (AppUtils::GetInstance().IsAllowStartAbilityWithoutCallerToken(bundleName, abilityName)) {
            auto bms = AbilityUtil::GetBundleManagerHelper();
            CHECK_POINTER_RETURN_BOOL(bms);
            int32_t callerUid = IPCSkeleton::GetCallingUid();
            std::string callerBundleName;
            if (IN_PROCESS_CALL(bms->GetNameForUid(callerUid, callerBundleName)) != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get caller bundle name.");
                return false;
            }
            auto userId = callerUid / BASE_USER_RANGE;
            AppExecFwk::BundleInfo info;
            if (!IN_PROCESS_CALL(
                bms->GetBundleInfo(callerBundleName, AppExecFwk::BundleFlag::GET_BUNDLE_DEFAULT, info, userId))) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get bundle info.");
                return false;
            }
            if (info.applicationInfo.needAppDetail) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "not dominate screen, app detail.");
                return false;
            }
        }
        TAG_LOGE(AAFwkTag::ABILITYMGR, "dominate screen.");
        return true;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "not dominate screen.");
    return false;
}

int AbilityPermissionUtil::CheckCallAbilityPermission(const AbilityRequest &abilityRequest, uint32_t specifyTokenId,
    bool isCallByShortcut)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call");

    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    verificationInfo.withContinuousTask = IsBackgroundTaskUid(IPCSkeleton::GetCallingUid());
    verificationInfo.specifyTokenId = specifyTokenId;
    if (IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

    int result = AAFwk::PermissionVerification::GetInstance()->CheckCallAbilityPermission(
        verificationInfo, isCallByShortcut);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "without start pageAbility(FA) or ability(Stage) permission");
    }
    return result;
}

int AbilityPermissionUtil::CheckStartByCallPermission(const AbilityRequest &abilityRequest)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Call");
    // check whether the target ability is page type and not specified mode.
    if (abilityRequest.abilityInfo.type != AppExecFwk::AbilityType::PAGE ||
        abilityRequest.abilityInfo.launchMode == AppExecFwk::LaunchMode::SPECIFIED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "called ability not common ability");
        return RESOLVE_CALL_ABILITY_TYPE_ERR;
    }

    AAFwk::PermissionVerification::VerificationInfo verificationInfo;
    verificationInfo.accessTokenId = abilityRequest.appInfo.accessTokenId;
    verificationInfo.visible = abilityRequest.abilityInfo.visible;
    verificationInfo.withContinuousTask = IsBackgroundTaskUid(IPCSkeleton::GetCallingUid());
    if (IsCallFromBackground(abilityRequest, verificationInfo.isBackgroundCall) != ERR_OK) {
        return ERR_INVALID_VALUE;
    }

    if (AAFwk::PermissionVerification::GetInstance()->CheckStartByCallPermission(verificationInfo) != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "without startAbilityByCall permission");
        return RESOLVE_CALL_NO_PERMISSIONS;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "caller has permission to resolve the call proxy of common ability");
    return ERR_OK;
}

int AbilityPermissionUtil::IsCallFromBackground(const AbilityRequest &abilityRequest, bool &isBackgroundCall,
    bool isData)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!isData && AAFwk::PermissionVerification::GetInstance()->IsShellCall()) {
        isBackgroundCall = true;
        return ERR_OK;
    }

    if (!isData && (AAFwk::PermissionVerification::GetInstance()->IsSACall() ||
        AbilityUtil::IsStartFreeInstall(abilityRequest.want))) {
        isBackgroundCall = false;
        return ERR_OK;
    }

    AppExecFwk::RunningProcessInfo processInfo;
    std::shared_ptr<AbilityRecord> callerAbility = Token::GetAbilityRecordByToken(abilityRequest.callerToken);
    if (callerAbility && callerAbility->GetAbilityInfo().bundleName == BUNDLE_NAME_DIALOG) {
        callerAbility = callerAbility->GetCallerRecord();
    }
    if (callerAbility) {
        if (callerAbility->IsForeground() || callerAbility->GetAbilityForegroundingFlag()) {
            isBackgroundCall = false;
            return ERR_OK;
        }
        // CallerAbility is not foreground, so check process state
        DelayedSingleton<AppScheduler>::GetInstance()->
            GetRunningProcessInfoByToken(callerAbility->GetToken(), processInfo);
        if (IsDelegatorCall(processInfo, abilityRequest)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "The call is from AbilityDelegator, allow background-call.");
            isBackgroundCall = false;
            return ERR_OK;
        }
        auto abilityState = callerAbility->GetAbilityState();
        if (abilityState == AbilityState::BACKGROUND || abilityState == AbilityState::BACKGROUNDING ||
            // If uiability or uiextensionability ability state is foreground when terminate,
            // it will move to background firstly. So if startAbility in onBackground() lifecycle,
            // the actual ability state may be had changed to terminating from background or backgrounding.
            abilityState == AbilityState::TERMINATING) {
            return ERR_OK;
        }
    } else {
        auto callerPid = IPCSkeleton::GetCallingPid();
        DelayedSingleton<AppScheduler>::GetInstance()->GetRunningProcessInfoByPid(callerPid, processInfo);
        if (processInfo.processName_.empty()) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Can not find caller application by callerPid: %{private}d.", callerPid);
            if (AAFwk::PermissionVerification::GetInstance()->VerifyCallingPermission(
                PermissionConstants::PERMISSION_START_ABILITIES_FROM_BACKGROUND)) {
                TAG_LOGD(AAFwkTag::ABILITYMGR, "Caller has PERMISSION_START_ABILITIES_FROM_BACKGROUND, PASS.");
                isBackgroundCall = false;
                return ERR_OK;
            }
            TAG_LOGE(AAFwkTag::ABILITYMGR, "without PERMISSION_START_ABILITIES_FROM_BACKGROUND, REJECT");
            return ERR_INVALID_VALUE;
        }
    }
    return SetBackgroundCall(processInfo, abilityRequest, isBackgroundCall);
}

int32_t AbilityPermissionUtil::SetBackgroundCall(const AppExecFwk::RunningProcessInfo &processInfo,
    const AbilityRequest &abilityRequest, bool &isBackgroundCall) const
{
    if (IsDelegatorCall(processInfo, abilityRequest)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "The call is from AbilityDelegator, allow background-call.");
        isBackgroundCall = false;
        return ERR_OK;
    }

    if (backgroundJudgeFlag_) {
        isBackgroundCall = processInfo.state_ != AppExecFwk::AppProcessState::APP_STATE_FOREGROUND &&
            !processInfo.isFocused && !processInfo.isAbilityForegrounding;
    } else {
        isBackgroundCall = !processInfo.isFocused;
        if (!processInfo.isFocused && processInfo.state_ == AppExecFwk::AppProcessState::APP_STATE_FOREGROUND) {
            // Allow background startup within 1 second after application startup if state is FOREGROUND
            int64_t aliveTime = AbilityUtil::SystemTimeMillis() - processInfo.startTimeMillis_;
            isBackgroundCall = aliveTime > APP_ALIVE_TIME_MS;
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Process %{public}s is alive %{public}s ms.",
                processInfo.processName_.c_str(), std::to_string(aliveTime).c_str());
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR,
        "backgroundJudgeFlag: %{public}d, isBackgroundCall: %{public}d, callerAppState: %{public}d.",
        static_cast<int32_t>(backgroundJudgeFlag_),
        static_cast<int32_t>(isBackgroundCall),
        static_cast<int32_t>(processInfo.state_));

    return ERR_OK;
}
} // AAFwk
} // OHOS