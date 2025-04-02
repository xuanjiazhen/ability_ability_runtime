/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_STS_APP_FOREGROUND_STATE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_STS_APP_FOREGROUND_STATE_OBSERVER_H

#include <mutex>
#include <set>

#include "app_foreground_state_observer_stub.h"
#include "event_handler.h"
#include "native_engine/native_engine.h"
#include "sts_runtime.h"
#include "sts_app_manager_utils.h"

namespace OHOS {
namespace AppManagerSts {
using OHOS::AppExecFwk::AppForegroundStateObserverStub;
using OHOS::AppExecFwk::AppStateData;
using OHOS::AbilityRuntime::STSNativeReference;
class AniAppForegroundStateObserver : public AppForegroundStateObserverStub {
public:
    explicit AniAppForegroundStateObserver(ani_env *env);
    virtual ~AniAppForegroundStateObserver() = default;
    void OnAppStateChanged(const AppStateData &appStateData);
    void HandleOnAppStateChanged(const AppStateData &appStateData);
    void AddAniObserverObject(const ani_object &aniServerObject);
    void RemoveAniObserverObject(const ani_object &aniServerObject);
    void RemoveAllAniObserverObjects();
    bool IsEmpty();
private:
    std::shared_ptr<AutoSTSReference> GetObserverObject(const ani_object &aniServerObject);

private:
    ani_env *env_ = nullptr;
    std::set<std::shared_ptr<AutoSTSReference>> aniObserverObjectSet_;
    std::mutex aniObserverObjectSetLock_;
};
} // namespace AppManagerSts
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_STS_APP_FOREGROUND_STATE_OBSERVER_H