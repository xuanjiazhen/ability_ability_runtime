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

#include "scene_board/ui_ability_record.h"

namespace OHOS {
namespace AAFwk {
UIAbilityRecord::UIAbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode)
    : AbilityRecord(want, abilityInfo, applicationInfo, requestCode) {}

std::shared_ptr<UIAbilityRecord> UIAbilityRecord::CreateAbilityRecord(const AbilityRequest &abilityRequest)
{
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->Init(abilityRequest);
    return abilityRecord;
}

AbilityRecordType UIAbilityRecord::GetAbilityRecordType()
{
    return AbilityRecordType::UI_ABILITY;
}

void UIAbilityRecord::Init(const AbilityRequest &abilityRequest)
{
    AbilityRecord::Init(abilityRequest);
    if (AbilityRuntime::StartupUtil::IsStartPlugin(abilityRequest.want)) {
        isPluginAbility_ = true;
    }
    collaboratorType_ = abilityRequest.collaboratorType;
}

void UIAbilityRecord::ConfigLoadParam(AbilityRuntime::LoadParam &loadParam)
{
    loadParam.isPreloadStart = isPreloadStart_;
}

void UIAbilityRecord::ScheduleCollaborate(const Want &want)
{
    CHECK_POINTER(lifecycleDeal_);
    lifecycleDeal_->ScheduleCollaborate(want);
}

void UIAbilityRecord::AfterStateUpdated()
{
    auto collaborator = DelayedSingleton<AbilityManagerService>::GetInstance()->GetCollaborator(
        collaboratorType_);
    if (collaborator != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "notify collaborator, missionId:%{public}d, state:%{public}d", missionId_,
            static_cast<int32_t>(state));
        int ret = ERR_OK;

        auto sessionInfo = GetSessionInfo();
        if (sessionInfo == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null sessionInfo");
            return;
        }
        int32_t persistentId = sessionInfo->persistentId;
        switch (state) {
            case AbilityState::BACKGROUNDING: {
                ret = collaborator->NotifyMoveMissionToBackground(persistentId, GetOwnerMissionUserId());
                break;
            }
            case AbilityState::TERMINATING: {
                ret = collaborator->NotifyTerminateMission(persistentId, GetOwnerMissionUserId());
                break;
            }
            default:
                break;
        }
        if (ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "failed,err: %{public}d", ret);
        }
    }
}

void UIAbilityRecord::ProcessForegroundAbility(uint32_t tokenId, const ForegroundOptions &options)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "ability record: %{public}s/%{public}s", GetElementName().GetBundleName().c_str(),
        GetElementName().GetAbilityName().c_str());
#ifdef SUPPORT_UPMS
    {
        std::lock_guard guard(wantLock_);
        std::string targetBundleName = abilityInfo_.applicationInfo.bundleName;
        bool isNotifyCollaborator = !options.targetGrantBundleName.empty();
        if (isNotifyCollaborator) {
            targetBundleName = options.targetGrantBundleName;
        }
        GrantUriPermission(want_, targetBundleName, false, tokenId, isNotifyCollaborator);
    }
#endif // SUPPORT_UPMS
    if (!isReady_) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "To load ability.");
        lifeCycleStateInfo_.sceneFlagBak = options.sceneFlag;
        LoadAbility(options.isShellCall, options.isStartupHide, options.callingPid, options.loadAbilityCallbackId);
        return;
    }

    DelayedSingleton<AppScheduler>::GetInstance()->NotifyLoadAbilityFinished(options.callingPid,
        GetPid(), options.loadAbilityCallbackId);
    if (GetRequestCode() != DEFAULT_REQUEST_CODE &&
        ForegroundAppConnectionManager::IsForegroundAppConnection(GetAbilityInfo(), GetCallerRecord())) {
        ReportAbilityConnectionRelations();
    }

    PostForegroundTimeoutTask();
    if (IsAbilityState(AbilityState::FOREGROUND)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Activate %{public}s/%{public}s", GetElementName().GetBundleName().c_str(),
            GetElementName().GetAbilityName().c_str());
        if (IsFrozenByPreload()) {
            SetFrozenByPreload(false);
            auto ret =
                DelayedSingleton<AppScheduler>::GetInstance()->NotifyPreloadAbilityStateChanged(token_, false);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "NotifyPreloadAbilityStateChanged by start, ret: %{public}d", ret);
        }
        ForegroundAbility(options.sceneFlag);
        return;
    }
    // background to active state
    TAG_LOGD(AAFwkTag::ABILITYMGR, "MoveToForeground, %{public}s/%{public}s", GetElementName().GetBundleName().c_str(),
        GetElementName().GetAbilityName().c_str());
    lifeCycleStateInfo_.sceneFlagBak = options.sceneFlag;
    ResSchedUtil::GetInstance().ReportEventToRSS(GetUid(), GetAbilityInfo().bundleName,
        "THAW_BY_FOREGROUND_ABILITY", GetPid(), GetCallerRecord() ? GetCallerRecord()->GetPid() : -1);
    SendAppStartupTypeEvent(AppExecFwk::AppStartType::HOT);
    SetAbilityStateInner(AbilityState::FOREGROUNDING);
    DelayedSingleton<AppScheduler>::GetInstance()->MoveToForeground(token_);
}

void UIAbilityRecord::SetPromotePriority(bool promotePriority)
{
    uiAbilityProperty_ = promotePriority;
}

bool UIAbilityRecord::GetPromotePriority()
{
    return uiAbilityProperty_.value_or(false) && GetPid() > 0;
}

void UIAbilityRecord::PromotePriority()
{
    auto callerInfo = GetCallerInfo();
    if (IsStartedByCall() && GetPromotePriority() && callerInfo != nullptr) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "promoting priority: %{public}s", GetAbilityInfo().bundleName.c_str());
        ResSchedUtil::GetInstance().PromotePriorityToRSS(callerInfo->callerUid, callerInfo->callerPid,
            GetAbilityInfo().bundleName, GetUid(), GetPid());
    }
}
}  // namespace AAFwk
}  // namespace OHOS