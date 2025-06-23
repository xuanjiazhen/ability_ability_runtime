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


#ifndef ABILITY_RUNTIME_JS_STATIC_ABILITY_LIFECYCLE_CALLBACK_H
#define ABILITY_RUNTIME_JS_STATIC_ABILITY_LIFECYCLE_CALLBACK_H

#include <map>
#include <memory>

#include "static_ability_lifecycle_callback.h"

typedef struct napi_env__* napi_env;
typedef struct napi_value__* napi_value;
typedef struct __ani_env ani_env;

class NativeReference;

namespace OHOS {
namespace AbilityRuntime {
struct STSNativeReference;

class JsStaticAbilityLifecycleCallback : public StaticAbilityLifecycleCallback,
                                        public std::enable_shared_from_this<JsStaticAbilityLifecycleCallback> {
public:
    explicit JsStaticAbilityLifecycleCallback(napi_env env);
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

    int32_t Register(napi_value callback);
    bool Unregister(int32_t callbackId);

private:
    void CallObjectMethod(ani_env *env, const char *methodName, std::shared_ptr<STSNativeReference> ability);
    void CallObjectMethod(ani_env *env, const char *methodName, std::shared_ptr<STSNativeReference> ability,
        std::shared_ptr<STSNativeReference> windowStage);

private:
    static int32_t serialNumber_;
    napi_env env_ = nullptr;
    std::map<int32_t, std::shared_ptr<NativeReference>> callbacks_;
};
} // AbilityRuntime
} // OHOS

#endif // ABILITY_RUNTIME_JS_STATIC_ABILITY_LIFECYCLE_CALLBACK_H
