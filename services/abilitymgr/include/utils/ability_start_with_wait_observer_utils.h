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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_START_WITH_WAIT_OBSERVER_UTILS
#define OHOS_ABILITY_RUNTIME_ABILITY_START_WITH_WAIT_OBSERVER_UTILS

#include <map>
#include <mutex>

#include "ability_start_with_wait_observer_interface.h"
#include "ability_util.h"
#include "nocopyable.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AbilityStartWithWaitObserverUtils {
public:
    static AbilityStartWithWaitObserverUtils& GetInstance();
    void Regist(const Want &want, const sptr<IAbilityStartWithWaitObserver> &observer);
    void Unregist(const sptr<IAbilityStartWithWaitObserver> &observer);
    void Intercepted(const Want& want, AbilityUtil::InterceptedReason reason);

private:
    AbilityStartWithWaitObserverUtils() = default;
    ~AbilityStartWithWaitObserverUtils() = default;

    int32_t observerId_ = 0;
    std::mutex oberverMutex_;
    std::map<sptr<IAbilityStartWithWaitObserver>, int32_t> observerList_;
    DISALLOW_COPY_AND_MOVE(AbilityStartWithWaitObserverUtils);
 };
 } // namespace AAFwk
 } // namespace OHOS
 #endif // OHOS_ABILITY_RUNTIME_ABILITY_START_WITH_WAIT_OBSERVER_UTILS