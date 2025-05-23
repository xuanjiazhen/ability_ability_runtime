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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_FIRST_FRAME_STATE_OBSERVER_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_FIRST_FRAME_STATE_OBSERVER_MANAGER_H

#include <string>
#include "mock_common.h"

namespace OHOS {
namespace AppExecFwk {
class AbilityFirstFrameStateObserverManager {
public:
    static AbilityFirstFrameStateObserverManager &GetInstance()
    {
        static AbilityFirstFrameStateObserverManager instance;
        return instance;
    }

    void Init() {}

    int32_t RegisterAbilityFirstFrameStateObserver(const sptr<IAbilityFirstFrameStateObserver> &observer,
        const std::string &targetBundleName)
    {
        return 0;
    }

    int32_t UnregisterAbilityFirstFrameStateObserver(const sptr<IAbilityFirstFrameStateObserver> &observer)
    {
        return 0;
    }
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif
