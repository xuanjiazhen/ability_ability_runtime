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

#include "ability_start_with_wait_observer_utils.h"

#include <chrono>

#include "ability_start_with_wait_observer_data.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

AbilityStartWithWaitObserverUtils& AbilityStartWithWaitObserverUtils::GetInstance()
{
    static AbilityStartWithWaitObserverUtils instance;
    return instance;
}

void AbilityStartWithWaitObserverUtils::Regist(const Want &want, const sptr<IAbilityStartWithWaitObserver> &observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (!observer) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return;
    }

    std::lock_guard<std::mutex> guard(oberverMutex_);
    for (auto& ob : observerList_) {
        if (!ob.first) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer in observerList_");
            continue;
        }
        if (observer->AsObject() == ob.first->AsObject()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "duplicate observer");
            return ;
        }
    }
    auto tmpId = (abilityForegroundInfo_.observerId_ == INT_MAX) ? 0 : (abilityForegroundInfo_.observerId_ + 1);
    abilityForegroundInfo_.observerId_ = tmpId;
    abilityForegroundInfo_.startTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    observerList_.emplace(observer, abilityForegroundInfo_);
    AddDeathRecipient(observer);
    (const_cast<Want &>(want)).SetParam(AbilityUtil::OBSERVER_TOKEN_KEY, tmpId);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size:%{public}d", observerList_.size());
    TAG_LOGD(AAFwkTag::ABILITYMGR, "death recipient observer list size:%{public}d", deathRecipientList_.size());
}

void AbilityStartWithWaitObserverUtils::Unregist(const sptr<IAbilityStartWithWaitObserver> &observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    std::lock_guard<std::mutex> guard(oberverMutex_);
    UnregistInnerUnlocked(observer);
}

void AbilityStartWithWaitObserverUtils::Intercepted(const Want& want, AbilityUtil::InterceptedReason reason)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
}

void AbilityStartWithWaitObserverUtils::NotifyAATerminateWait(const std::shared_ptr<AbilityRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityRecord");
        return;
    }
    auto observerId = abilityRecord->GetWant().GetIntParam(AbilityUtil::OBSERVER_TOKEN_KEY, -1);
    std::lock_guard<std::mutex> guard(oberverMutex_);
    for (auto& observer : observerList_) {
        if (!observer.first) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
            continue;
        }
        if (observerId == observer.second.observerId_) {
            AbilityStartWithWaitObserverData data;
            data.coldStart_ = observer.second.coldStart_;
            data.startTime_ = observer.second.startTime_;
            data.foregroundTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            data.bundleName_ = abilityRecord->GetAbilityInfo().bundleName;
            data.abilityName_ = abilityRecord->GetAbilityInfo().name;
            TAG_LOGD(AAFwkTag::ABILITYMGR, "notify aa, time:%{public}" PRId64 "foregroundTime:%{public}" PRId64
                ",bundleName:%{public}s ability:%{public}s", data.startTime_, data.foregroundTime_,
                data.bundleName_.c_str(), data.abilityName_.c_str());
            observer.first->StartAbilityWithWait(data);
            UnregistInnerUnlocked(observer.first);
            break; // cant deleted for UnregisterInnerUnlocked erase map iter
        }
    }
}

void AbilityStartWithWaitObserverUtils::SetColdStartForShellCall(std::shared_ptr<AbilityRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityRecord");
        return;
    }
    auto observerId = abilityRecord->GetWant().GetIntParam(AbilityUtil::OBSERVER_TOKEN_KEY, -1);
    std::lock_guard<std::mutex> guard(oberverMutex_);
    for (auto& observer : observerList_) {
        if (!observer.first) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
            continue;
        }
        if (observerId == observer.second.observerId_) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "set cold start, bundleName:%{public}s ability:%{public}s",
                abilityRecord->GetAbilityInfo().bundleName.c_str(), abilityRecord->GetAbilityInfo().name.c_str());
            observer.second.coldStart_ = true;
        }
    }
}

void AbilityStartWithWaitObserverUtils::AddDeathRecipient(const sptr<IAbilityStartWithWaitObserver> &observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (!observer) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return;
    }
    auto deathRecipientFunc = [](const wptr<IRemoteObject> &remote) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "death recipient call");
        auto object = remote.promote();
        if (object == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "object null");
            return;
        }
        auto observer = iface_cast<IAbilityStartWithWaitObserver>(object);
        AbilityStartWithWaitObserverUtils::GetInstance().Unregist(observer);
    };
    auto deathRecipient = sptr<AbilityStartWithWaitObserverRecipient>::MakeSptr(deathRecipientFunc);
    if (deathRecipient == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null deathRecipient");
        return;
    }
    if (!observer->AsObject()->AddDeathRecipient(deathRecipient)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AddDeathRecipient fail");
    }
    deathRecipientList_.emplace(observer, deathRecipient);
}

void AbilityStartWithWaitObserverUtils::UnregistInnerUnlocked(const sptr<IAbilityStartWithWaitObserver> &observer)
{
    if (!observer) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return;
    }
    for (auto& ob : observerList_) {
        if (!ob.first) {
            continue;
        }
        if (observer->AsObject() == ob.first->AsObject()) {
            observerList_.erase(ob.first);
            break;
        }
    }
    for (auto& deathOb : deathRecipientList_) {
        if (!deathOb.first) {
            continue;
        }
        if (observer->AsObject() == deathOb.first->AsObject()) {
            deathRecipientList_.erase(deathOb.first);
            observer->AsObject()->RemoveDeathRecipient(deathOb.second);
            break;
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "observer list size:%{public}d", observerList_.size());
    TAG_LOGD(AAFwkTag::ABILITYMGR, "death recipient observer list size:%{public}d", deathRecipientList_.size());
}
} // namespace AAFwk
} // namespace OHOS