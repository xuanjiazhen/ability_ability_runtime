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


#ifndef ABILITY_RUNTIME_ETS_STATIC_ABILITY_LIFECYCLE_CALLBACK_H
#define ABILITY_RUNTIME_ETS_STATIC_ABILITY_LIFECYCLE_CALLBACK_H

#include <map>
#include <memory>

#include "interop_ability_lifecycle_callback.h"

namespace OHOS {
namespace AbilityRuntime {
class EtsInteropAbilityLifecycleCallback : public InteropAbilityLifecycleCallback,
                                        public std::enable_shared_from_this<EtsInteropAbilityLifecycleCallback> {
public:
    EtsInteropAbilityLifecycleCallback(ani_env *env);
    void OnAbilityCreate(std::shared_ptr<InteropObject> ability) override;
    void OnAbilityWillCreate(std::shared_ptr<InteropObject> ability) override;
    void OnWindowStageCreate(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) override;
    void OnWindowStageWillCreate(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) override;
    void OnAbilityDestroy(std::shared_ptr<InteropObject> ability) override;
    void OnWindowStageDestroy(std::shared_ptr<InteropObject> ability,
        std::shared_ptr<InteropObject> windowStage) override;
    void OnAbilityForeground(std::shared_ptr<InteropObject> ability) override;
    void OnAbilityBackground(std::shared_ptr<InteropObject> ability) override;

    int32_t Register(ani_object callback);
    bool Unregister(int32_t callbackId);

private:
    ani_env *GetAniEnv();
    void CallObjectMethod(const char *methodName, const char *signature,
        std::shared_ptr<InteropObject> ability);
    void CallObjectMethod(const char *methodName, const char *signature,
        std::shared_ptr<InteropObject> ability, std::shared_ptr<InteropObject> windowStage);
    ani_ref CallObjectMethod(const char *className, const char *methodName, const char *signature,
        ani_object aniObject);

private:
    static int32_t serialNumber_;
    ani_vm *vm_ = nullptr;
    std::map<int32_t, ani_ref> callbacks_;
};
} // AbilityRuntime
} // OHOS

#endif // ABILITY_RUNTIME_ETS_STATIC_ABILITY_LIFECYCLE_CALLBACK_H
