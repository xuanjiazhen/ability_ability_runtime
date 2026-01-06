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

#include "ui_ability_record.h"

#include "ability_util.h"

namespace OHOS {
namespace AAFwk {
UIAbilityRecord::UIAbilityRecord(const Want &want, const AppExecFwk::AbilityInfo &abilityInfo,
    const AppExecFwk::ApplicationInfo &applicationInfo, int requestCode)
    : AbilityRecord(want, abilityInfo, applicationInfo, requestCode) {}

UIAbilityRecordPtr UIAbilityRecord::CreateAbilityRecord(const AbilityRequest &abilityRequest)
{
    auto abilityRecord = std::make_shared<UIAbilityRecord>(
        abilityRequest.want, abilityRequest.abilityInfo, abilityRequest.appInfo, abilityRequest.requestCode);
    abilityRecord->Init(abilityRequest);
    return abilityRecord;
}

UIAbilityRecordPtr UIAbilityRecord::FromBaseRecord(std::shared_ptr<AbilityRecord> abilityRecord)
{
    if (abilityRecord == nullptr || abilityRecord->GetAbilityRecordType() != AbilityRecordType::UI_ABILITY) {
        return nullptr;
    }
    return std::static_pointer_cast<MissionAbilityRecord>(abilityRecord);
}

UIAbilityRecordPtr FromAbilityToken(sptr<IRemoteObject> token)
{
    return FromBaseRecord(Token::GetAbilityRecordByToken(token));
}

AbilityRecordType UIAbilityRecord::GetAbilityRecordType()
{
    return AbilityRecordType::UI_ABILITY;
}

void UIAbilityRecord::OnProcessDied()
{
    CancelPrepareTerminate();
    AbilityRecord::OnProcessDied();
}

void UIAbilityRecord::ScheduleCollaborate(const Want &want)
{
    std::lock_guard guard(collaborateWantLock_);
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

bool UIAbilityRecord::PrepareTerminateAbility(bool isSCBCall)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "PrepareTerminateAbility");
    if (!isSCBCall) {
        return AbilityRecord::PrepareTerminateAbility();
    }
    // execute onPrepareToTerminate until timeout
    std::unique_lock<std::mutex> lock(isPrepareTerminateAbilityMutex_);
    isPrepareTerminateAbilityCalled_.store(true);
    isPrepareTerminate_ = false;
    isPrepareTerminateAbilityDone_.store(false);
    auto condition = [weak = weak_from_this()] {
        auto self = FromBaseRecord(weak.lock());
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null self");
            return false;
        }
        return self->isPrepareTerminateAbilityDone_.load();
    };
    auto task = [weak = weak_from_this()]() {
        auto self = FromBaseRecord(weak.lock());
        if (self == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityRecord, prepareTerminateAbility error");
            return;
        }
        if (!self->lifecycleDeal_ || !self->lifecycleDeal_->PrepareTerminateAbility()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "PrepareTerminateAbility error");
            self->isPrepareTerminateAbilityDone_.store(true);
            self->isPrepareTerminateAbilityCalled_.store(false);
            self->isPrepareTerminateAbilityCv_.notify_all();
        }
    };
    ffrt::submit(task);
    if (!isPrepareTerminateAbilityCv_.wait_for(lock,
        std::chrono::milliseconds(GlobalConstant::PREPARE_TERMINATE_TIMEOUT_TIME), condition)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "wait timeout");
        return false;
    }
    return isPrepareTerminate_;
}

void UIAbilityRecord::PrepareTerminateAbilityDone(bool isTerminate)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call PrepareTerminateAbilityDone");
    std::unique_lock<std::mutex> lock(isPrepareTerminateAbilityMutex_);
    if (!isPrepareTerminateAbilityCalled_.load()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "prepare terminate not called");
        return;
    }
    isPrepareTerminate_ = isTerminate;
    isPrepareTerminateAbilityDone_.store(true);
    isPrepareTerminateAbilityCalled_.store(false);
    isPrepareTerminateAbilityCv_.notify_one();
}

void UIAbilityRecord::CancelPrepareTerminate()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "canceling prepare terminate,bundle=%{public}s,module=%{public}s,ability=%{public}s",
        abilityInfo_.bundleName.c_str(), abilityInfo_.moduleName.c_str(), abilityInfo_.name.c_str());
    std::unique_lock<std::mutex> lock(isPrepareTerminateAbilityMutex_);
    if (!isPrepareTerminateAbilityCalled_.load()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "prepare terminate not called");
        return;
    }
    isPrepareTerminate_ = true; // no need to terminate again
    isPrepareTerminateAbilityDone_.store(true);
    isPrepareTerminateAbilityCalled_.store(false);
    isPrepareTerminateAbilityCv_.notify_one();
}
}  // namespace AAFwk
}  // namespace OHOS