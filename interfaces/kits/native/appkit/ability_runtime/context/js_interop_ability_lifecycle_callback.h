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

#include "interop_ability_lifecycle_callback.h"

namespace OHOS {
namespace AbilityRuntime {
class JsInteropAbilityLifecycleCallback : public InteropAbilityLifecycleCallback,
                                        public std::enable_shared_from_this<JsInteropAbilityLifecycleCallback> {
public:
    explicit JsInteropAbilityLifecycleCallback(napi_env env);
    void OnAbilityCreate(std::shared_ptr<InteropObject> ability) override;
    void OnAbilityWillCreate(std::shared_ptr<InteropObject> ability) override;
    void OnWindowStageCreate(std::shared_ptr<InteropObject> ability) override;
    void OnWindowStageWillCreate(std::shared_ptr<InteropObject> ability) override;
    void OnAbilityDestroy(std::shared_ptr<InteropObject> ability) override;
    void OnWindowStageDestroy(std::shared_ptr<InteropObject> ability) override;
    void OnAbilityForeground(std::shared_ptr<InteropObject> ability) override;
    void OnAbilityBackground(std::shared_ptr<InteropObject> ability) override;

    int32_t Register(napi_value callback);
    bool Unregister(int32_t callbackId);

private:
    void CallObjectMethod(const char *methodName, std::shared_ptr<InteropObject> ability);

#ifdef SUPPORT_SCREEN
    void CallObjectMethodWithWindowStage(const char *methodName, std::shared_ptr<InteropObject> ability);
#endif

private:
    static int32_t serialNumber_;
    napi_env env_ = nullptr;
    std::map<int32_t, std::shared_ptr<NativeReference>> callbacks_;
};
} // AbilityRuntime
} // OHOS

#endif // ABILITY_RUNTIME_JS_STATIC_ABILITY_LIFECYCLE_CALLBACK_H
