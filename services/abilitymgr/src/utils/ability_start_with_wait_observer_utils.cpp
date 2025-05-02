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

namespace OHOS {
namespace AAFwk {

AbilityStartWithWaitObserverUtils& AbilityStartWithWaitObserverUtils::GetInstance()
{
    static AbilityStartWithWaitObserverUtils instance;
    return instance;
}

void AbilityStartWithWaitObserverUtils::RegistAbilityStartWithWaitObserver(
    const Want &want, const sptr<IAbilityStartWithWaitObserver> &observer)
{
    std::lock_guard<ffrt::mutex> guard(oberverMutex_);
    observerId_ = (observerId_ == INT_MAX) ? 0 : (observerId_ + 1);
    observerList_.emplace(observer, observerId_);
    want.SetParam("ohos.params.abilityStartWithObserver", observerId_);
}

void AbilityStartWithWaitObserverUtils::UnRegistAbilityStartWithWaitObserver(
    const sptr<IAbilityStartWithWaitObserver> &observer)
{
    std::lock_guard<ffrt::mutex> guard(oberverMutex_);
}
} // namespace AAFwk
} // namespace OHOS