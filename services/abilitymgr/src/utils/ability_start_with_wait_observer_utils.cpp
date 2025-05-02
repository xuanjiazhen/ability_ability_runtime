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
    std::lock_guard<std::mutex> guard(oberverMutex_);
    auto tmpId = (abilityForegroundInfo_.observerId_ == INT_MAX) ? 0 : (abilityForegroundInfo_.observerId_ + 1);
    abilityForegroundInfo_.observerId_ = tmpId;
    abilityForegroundInfo_.time_ = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    observerList_.emplace(observer, abilityForegroundInfo_);
    (const_cast<Want &>(want)).SetParam(AbilityUtil::OBSERVER_TOKEN_KEY, tmpId);
}

void AbilityStartWithWaitObserverUtils::Unregist(const sptr<IAbilityStartWithWaitObserver> &observer)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    std::lock_guard<std::mutex> guard(oberverMutex_);
}

void AbilityStartWithWaitObserverUtils::Intercepted(const Want& want, AbilityUtil::InterceptedReason reason)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
}

void AbilityStartWithWaitObserverUtils::NotifyAACompleteFirstFrameDrawing(
    std::shared_ptr<AbilityRecord> &abilityRecord)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call");
    if (!abilityRecord) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityRecord");
        return;
    }
    auto observerId = abilityRecord->GetWant().GetIntParam(AbilityUtil::OBSERVER_TOKEN_KEY, -1);
    {
        std::lock_guard<std::mutex> guard(oberverMutex_);
        for(auto& observer : observerList_) {
            if (!observer.first) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
                continue;
            }
            if (observerId == observer.second.observerId_ && observer.second.coldStart_) { // cold start
                AbilityStartWithWaitObserverData data;
                data.coldStart_ = observer.second.coldStart_;
                data.time_ = observer.second.time_;
                data.foregroundTime_ = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::system_clock::now().time_since_epoch()).count();
                data.bundleName_ = abilityRecord->GetAbilityInfo().bundleName;
                data.abilityName_ = abilityRecord->GetAbilityInfo().name;
                // notify aa
                TAG_LOGD(AAFwkTag::ABILITYMGR, "notify aa, bundleName:%{public}s ability:%{public}s",
                    data.bundleName_.c_str(), data.abilityName_.c_str());
                observer.first->OnStartAbilityWithWait(data);
            }
        }
    }
}
} // namespace AAFwk
} // namespace OHOS