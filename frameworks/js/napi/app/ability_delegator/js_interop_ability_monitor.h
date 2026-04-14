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

#ifndef OHOS_ABILITY_RUNTIME_JS_INTEROP_ABILITY_MONITOR_H
#define OHOS_ABILITY_RUNTIME_JS_INTEROP_ABILITY_MONITOR_H

#include <memory>
#include <string>
#include "native_engine/native_reference.h"

namespace OHOS {
namespace AbilityDelegatorJs {
class JsInteropAbilityMonitor {
public:
    /**
     * A constructor used to create a JsInteropAbilityMonitor instance with the input parameter passed.
     *
     * @param abilityName Indicates the specified ability name.
     */
    explicit JsInteropAbilityMonitor(const std::string &abilityName);

    /**
     * A constructor used to create a JsInteropAbilityMonitor instance with the input parameter passed.
     *
     * @param abilityName Indicates the specified ability name.
     * @param moduleName Indicates the specified module name.
     */
    explicit JsInteropAbilityMonitor(const std::string &abilityName, const std::string &moduleName);

    /**
     * Default deconstructor used to deconstruct.
     */
    ~JsInteropAbilityMonitor() = default;

    /**
     * Called when ability is started, then call JS callback with null.
     */
    void OnAbilityCreate();

    /**
     * Called when ability is in foreground, then call JS callback with null.
     */
    void OnAbilityForeground();

    /**
     * Called when ability is in background, then call JS callback with null.
     */
    void OnAbilityBackground();

    /**
     * Called when ability is stopped, then call JS callback with null.
     */
    void OnAbilityDestroy();

    /**
     * Called when window stage is created, then call JS callback with null.
     */
    void OnWindowStageCreate();

    /**
     * Called when window stage is restored, then call JS callback with null.
     */
    void OnWindowStageRestore();

    /**
     * Called when window stage is destroyed, then call JS callback with null.
     */
    void OnWindowStageDestroy();

    /**
     * Sets the js object.
     *
     * @param env Indicates the native env.
     * @param monitor Indicates the js object.
     */
    void SetJsInteropAbilityMonitor(napi_env env, napi_value monitor);

    /**
     * Obtains the saved js object.
     *
     * @return the saved js object.
     */
    std::unique_ptr<NativeReference> &GetJsInteropAbilityMonitor()
    {
        return jsInteropMonitor_;
    }

private:
    napi_value CallLifecycleCBFunction(const std::string &functionName);

private:
    std::string abilityName_ = "";
    std::string moduleName_ = "";
    napi_env env_ = nullptr;
    std::unique_ptr<NativeReference> jsInteropMonitor_ = nullptr;
};
}  // namespace AbilityDelegatorJs
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_INTEROP_ABILITY_MONITOR_H
