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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_HIDDEN_START_OBSERVER_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_HIDDEN_START_OBSERVER_MANAGER_H

#include <set>
#include <map>
#include "ffrt.h"
#include "ihidden_start_observer.h"
#include "singleton.h"

namespace OHOS {
namespace AAFwk {
class HiddenStartObserverManager {
public:
    static HiddenStartObserverManager &GetInstance()
    {
        static HiddenStartObserverManager instance;
        return instance;
    }
    int32_t RegisterObserver(const sptr<IHiddenStartObserver> &observer)
    {
        return 0;
    }
    int32_t UnregisterObserver(const sptr<IHiddenStartObserver> &observer)
    {
        return 0;
    }
};
} // namespace AAFwk
} // namespace OHOS
#endif