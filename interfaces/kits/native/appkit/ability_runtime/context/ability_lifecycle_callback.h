/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_CONTEXT_ABILITY_LIFECYCLE_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_CONTEXT_ABILITY_LIFECYCLE_CALLBACK_H

#include <map>
#include <memory>

class NativeReference;
typedef struct napi_env__* napi_env;
typedef struct napi_value__* napi_value;
typedef struct __ani_env ani_env;
typedef class __ani_object *ani_object;
typedef class __ani_ref *ani_ref;

namespace OHOS {
namespace AbilityRuntime {
class AbilityLifecycleCallback {
public:
    virtual ~AbilityLifecycleCallback() {}
/*----------------------------------for arkts 1.1------------------------------------------------*/
    virtual void OnAbilityCreate(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnWindowStageCreate(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) {}
    virtual void OnWindowStageDestroy(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) {}
    virtual void OnWindowStageActive(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) {}
    virtual void OnWindowStageInactive(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) {}
    virtual void OnAbilityDestroy(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnNewWant(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnWillNewWant(const std::shared_ptr<NativeReference> &ability){}
    virtual void OnAbilityWillCreate(const std::shared_ptr<NativeReference> &ability){}
    virtual void OnWindowStageWillCreate(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) {}
    virtual void OnWindowStageWillDestroy(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) {}
    virtual void OnAbilityWillDestroy(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnAbilityWillForeground(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnAbilityWillBackground(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnAbilityForeground(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnAbilityBackground(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnAbilityContinue(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnAbilityWillContinue(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnWindowStageWillRestore(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) {}
    virtual void OnWindowStageRestore(const std::shared_ptr<NativeReference> &ability,
        const std::shared_ptr<NativeReference> &windowStage) {}
    virtual void OnAbilityWillSaveState(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnAbilitySaveState(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnWillForeground(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnDidForeground(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnWillBackground(const std::shared_ptr<NativeReference> &ability) {}
    virtual void OnDidBackground(const std::shared_ptr<NativeReference> &ability) {}

/*----------------------------------for arkts 1.2------------------------------------------------*/
    virtual void OnAbilityCreate([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
    virtual void OnWindowStageCreate([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) {}
    virtual void OnWindowStageDestroy([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) {}
    virtual void OnWindowStageActive([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) {}
    virtual void OnWindowStageInactive([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) {}
    virtual void OnAbilityDestroy([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
    virtual void OnNewWant([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability){}
    virtual void OnWillNewWant([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability){}
    virtual void OnAbilityWillCreate([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability){}
    virtual void OnWindowStageWillCreate([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) {}
    virtual void OnWindowStageWillDestroy([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) {}
    virtual void OnAbilityWillDestroy([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
    virtual void OnAbilityWillForeground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
    virtual void OnAbilityWillBackground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
    virtual void OnAbilityForeground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
    virtual void OnAbilityBackground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
    virtual void OnAbilityContinue([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
    virtual void OnAbilityWillContinue([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
    virtual void OnWindowStageWillRestore([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) {}
    virtual void OnWindowStageRestore([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) {}
    virtual void OnAbilityWillSaveState([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
    virtual void OnAbilitySaveState([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
    virtual void OnWillForeground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
    virtual void OnDidForeground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
    virtual void OnWillBackground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
    virtual void OnDidBackground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) {}
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CONTEXT_ABILITY_LIFECYCLE_CALLBACK_H
