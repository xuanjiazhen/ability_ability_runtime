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

#include "ability_util.h"
#include "hilog_tag_wrapper.h"
#include "res_sched_util.h"

namespace OHOS {
namespace AAFwk {
constexpr const char* IS_HOOK = "ohos.ability_runtime.is_hook";

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

void UIAbilityRecord::ForegroundAbility(uint32_t sceneFlag, bool hasLastWant)
{
    if (hasLastWant) {
        if (HasLastWant()) {
            SetWant(*lastWant_);
            lifeCycleStateInfo_.isNewWant = true;
            lastWant_ = nullptr;
        }
        SetBackgroundDrivenFlag(false);
    }
    AbilityRecord::ForegroundAbility(sceneFlag);
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