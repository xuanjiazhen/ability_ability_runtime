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

#include "static_ability_lifecycle_callback.h"

typedef struct __ani_env ani_env;
typedef class __ani_object *ani_object;
typedef class __ani_ref *ani_ref;

namespace OHOS {
namespace AbilityRuntime {
struct STSNativeReference;

class StsStaticAbilityLifecycleCallback : public StaticAbilityLifecycleCallback,
                                        public std::enable_shared_from_this<StsStaticAbilityLifecycleCallback> {
public:
    StsStaticAbilityLifecycleCallback();
    void OnAbilityCreate(ani_env *env, std::shared_ptr<STSNativeReference> ability) override;
    void OnAbilityWillCreate(ani_env *env, std::shared_ptr<STSNativeReference> ability) override;
    void OnWindowStageCreate(ani_env *env, std::shared_ptr<STSNativeReference> ability,
        std::shared_ptr<STSNativeReference> windowStage) override;
    void OnWindowStageWillCreate(ani_env *env, std::shared_ptr<STSNativeReference> ability,
        std::shared_ptr<STSNativeReference> windowStage) override;
    void OnAbilityDestroy(ani_env *env, std::shared_ptr<STSNativeReference> ability) override;
    void OnWindowStageDestroy(ani_env *env, std::shared_ptr<STSNativeReference> ability,
        std::shared_ptr<STSNativeReference> windowStage) override;
    void OnAbilityForeground(ani_env *env, std::shared_ptr<STSNativeReference> ability) override;
    void OnAbilityBackground(ani_env *env, std::shared_ptr<STSNativeReference> ability) override;

    int32_t Register(ani_env *env, ani_object callback);
    bool Unregister(int32_t callbackId);

private:
    void CallObjectMethod(ani_env *env, const char *methodName, const char *signature,
        std::shared_ptr<STSNativeReference> ability);
    void CallObjectMethod(ani_env *env, const char *methodName, const char *signature,
        std::shared_ptr<STSNativeReference> ability, std::shared_ptr<STSNativeReference> windowStage);

private:
    static int32_t serialNumber_;
    std::map<int32_t, ani_ref> callbacks_;
};
} // AbilityRuntime
} // OHOS

#endif // ABILITY_RUNTIME_ETS_STATIC_ABILITY_LIFECYCLE_CALLBACK_H
