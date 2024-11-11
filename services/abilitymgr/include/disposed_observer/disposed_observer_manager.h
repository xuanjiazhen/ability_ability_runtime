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

#ifndef OHOS_ABILITY_RUNTIME_DISPOSED_OBSERVER_MANAGER_H
#define OHOS_ABILITY_RUNTIME_DISPOSED_OBSERVER_MANAGER_H

#include <string>
#include <map>

#include "singleton.h"
#include "ffrt.h"
#include "disposed_observer/disposed_app_state_observer.h"
#include "disposed_observer/disposed_first_frame_observer.h"

namespace OHOS {
namespace AAFwk {
class DisposedObserverManager {
    DECLARE_DELAYED_SINGLETON(DisposedObserverManager)
public:
    int32_t RegisterFirstFrameObserver(const std::string &bundleName, AppExecFwk::DisposedRule disposedRule,
        DisposedModalType type);
    int32_t RegisterAppStateObserver(const std::string &bundleName, AppExecFwk::DisposedRule disposedRule);
    void UnregisterObserver(const std::string &bundleName);
    bool IsExistObserver(const std::string &bundleName);
private:
    ffrt::mutex observerLock_;
    std::map<std::string, sptr<DisposedFirstFrameObserver>> disposedFirstFrameObserverMap_;
    std::map<std::string, sptr<DisposedAppStateObserver>> disposedAppStateObserverMap_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DISPOSED_OBSERVER_H