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

#ifndef OHOS_ABILITY_RUNTIME_CONTEXT_ETS_ABILITY_LIFECYCLE_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_CONTEXT_ETS_ABILITY_LIFECYCLE_CALLBACK_H

#include <map>
#include <memory>

#include "ability_lifecycle_callback.h"

class NativeReference;
typedef struct napi_env__* napi_env;
typedef struct __ani_env ani_env;
typedef struct __ani_vm ani_vm;
typedef class __ani_object *ani_object;
typedef class __ani_ref *ani_ref;

namespace OHOS {
namespace AbilityRuntime {
class StsAbilityLifecycleCallback : public AbilityLifecycleCallback,
                                   public std::enable_shared_from_this<StsAbilityLifecycleCallback> {
public:
    explicit StsAbilityLifecycleCallback(ani_env *env);
    void OnAbilityCreate([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnWindowStageCreate([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) override;
    void OnWindowStageDestroy([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) override;
    void OnWindowStageActive([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) override;
    void OnWindowStageInactive([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) override;
    void OnAbilityWillCreate([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnWindowStageWillCreate([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) override;
    void OnWindowStageWillDestroy([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) override;
    void OnAbilityWillDestroy([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnAbilityWillForeground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnAbilityWillBackground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnNewWant([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnWillNewWant([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnAbilityDestroy([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnAbilityForeground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnAbilityBackground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnAbilityContinue([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnAbilityWillContinue([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnWindowStageWillRestore([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) override;
    void OnWindowStageRestore([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability,
        std::shared_ptr<NativeReference> windowStage) override;
    void OnAbilityWillSaveState([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnAbilitySaveState([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnWillForeground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnDidForeground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnWillBackground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;
    void OnDidBackground([[maybe_unused]]napi_env env, std::shared_ptr<NativeReference> ability) override;

    int32_t Register(ani_object callback);
    bool Unregister(int32_t callbackId);
    bool IsEmpty() const;

private:
    ani_env *GetAniEnv();
    void CallObjectMethod(napi_env env, const char *methodName, const char *signature,
        std::shared_ptr<NativeReference> ability);
    void CallObjectMethod(napi_env env, const char *methodName, const char *signature,
        std::shared_ptr<NativeReference> ability, std::shared_ptr<NativeReference> windowStage);

private:
    static int32_t serialNumber_;
    ani_vm *vm_ = nullptr;
    std::map<int32_t, ani_ref> callbacks_;
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_CONTEXT_ETS_ABILITY_LIFECYCLE_CALLBACK_H
