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

#ifndef OHOS_APP_MANAGER_STS_ABILITY_FIRST_FRAME_STATE_OBSERVER_H
#define OHOS_APP_MANAGER_STS_ABILITY_FIRST_FRAME_STATE_OBSERVER_H
#ifdef SUPPORT_GRAPHICS
#include <mutex>
#include <set>
#include "ability_first_frame_state_observer_stub.h"
#include "ability_manager_interface.h"
#include "singleton.h"
#include "event_handler.h"
#include "native_engine/native_engine.h"
#include "ability_first_frame_state_data.h"
#include "sts_runtime.h"
#include "sts_app_manager_utils.h"
namespace OHOS {
namespace AppManagerSts {
    using OHOS::AppExecFwk::AbilityFirstFrameStateObserverStub;
    using OHOS::AppExecFwk::AbilityFirstFrameStateData;
    using OHOS::AbilityRuntime::STSNativeReference;
    class STSAbilityFirstFrameStateObserver : public AbilityFirstFrameStateObserverStub {
    public:
        explicit STSAbilityFirstFrameStateObserver(ani_env *env);
        virtual ~STSAbilityFirstFrameStateObserver() = default;
        void OnAbilityFirstFrameState(const AbilityFirstFrameStateData &abilityFirstFrameStateData) override;
        void HandleOnAbilityFirstFrameState(const AbilityFirstFrameStateData &abilityFirstFrameStateData);
        void SetStsObserverObject(const ani_object &stsObserverObject);
        void ResetStsObserverObject();
        std::shared_ptr<AutoSTSReference> GetAutoSTSReference();
    
    private:
        ani_env *env_;
        std::shared_ptr<AutoSTSReference> stsObserverObject_;
    };

    class STSAbilityFirstFrameStateObserverManager {
    public:
        static STSAbilityFirstFrameStateObserverManager *GetInstance()
        {
            static STSAbilityFirstFrameStateObserverManager instance;
            return &instance;
        }
        ~STSAbilityFirstFrameStateObserverManager() = default;
        void Init(ani_env *env);
        void AddSTSAbilityFirstFrameStateObserver(const sptr<STSAbilityFirstFrameStateObserver> observer);
        bool IsObserverObjectExist(const ani_object &stsObserverObject);
        void RemoveAllStsObserverObjects(sptr<OHOS::AAFwk::IAbilityManager> &abilityManager);
        void RemoveStsObserverObject(sptr<OHOS::AAFwk::IAbilityManager> &abilityManager,
            const ani_object &stsObserverObject);

    private:
        STSAbilityFirstFrameStateObserverManager() = default;
        DISALLOW_COPY_AND_MOVE(STSAbilityFirstFrameStateObserverManager);
        std::shared_ptr<AutoSTSReference> GetObserverObject(const ani_object &stsObserverObject);

    private:
        ani_env *env_;
        std::mutex observerListLock_;
        std::list<sptr<STSAbilityFirstFrameStateObserver>> stsAbilityFirstFrameStateObserverList_;
    };

}
}
#endif // SUPPORT_GRAPHICS
#endif // OHOS_APP_MANAGER_STS_ABILITY_FIRST_FRAME_STATE_OBSERVER_H