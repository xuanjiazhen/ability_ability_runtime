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


#ifndef ABILITY_RUNTIME_STATIC_ABILITY_LIFECYCLE_CALLBACK_H
#define ABILITY_RUNTIME_STATIC_ABILITY_LIFECYCLE_CALLBACK_H

#include <map>
#include <memory>

typedef struct __ani_env ani_env;

namespace OHOS {
namespace AbilityRuntime {
struct STSNativeReference;

class StaticAbilityLifecycleCallback {
public:
    virtual ~StaticAbilityLifecycleCallback() {}

    virtual void OnAbilityCreate(ani_env *env, std::shared_ptr<STSNativeReference> ability) = 0;
    virtual void OnAbilityWillCreate(ani_env *env, std::shared_ptr<STSNativeReference> ability)
    {}
    virtual void OnWindowStageCreate(ani_env *env, std::shared_ptr<STSNativeReference> ability,
        std::shared_ptr<STSNativeReference> windowStage) = 0;
    virtual void OnWindowStageWillCreate(ani_env *env, std::shared_ptr<STSNativeReference> ability,
        std::shared_ptr<STSNativeReference> windowStage) {}
    virtual void OnAbilityDestroy(ani_env *env, std::shared_ptr<STSNativeReference> ability) = 0;
    virtual void OnWindowStageDestroy(ani_env *env, std::shared_ptr<STSNativeReference> ability,
        std::shared_ptr<STSNativeReference> windowStage) = 0;
    virtual void OnAbilityForeground(ani_env *env, std::shared_ptr<STSNativeReference> ability) = 0;
    virtual void OnAbilityBackground(ani_env *env, std::shared_ptr<STSNativeReference> ability) = 0;
};
} // AbilityRuntime
} // OHOS

#endif // ABILITY_RUNTIME_STATIC_ABILITY_LIFECYCLE_CALLBACK_H
