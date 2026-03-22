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

#ifndef OHOS_ABILITY_RUNTIME_JS_ABILITY_FOREGROUND_STATE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_JS_ABILITY_FOREGROUND_STATE_OBSERVER_H

#include <set>

#include "ability_foreground_state_observer_stub.h"
#include "ability_state_data.h"
#include "event_handler.h"
#include "js_ability_manager_utils.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
using OHOS::AppExecFwk::AbilityForegroundStateObserverStub;
using OHOS::AppExecFwk::AbilityStateData;

/**
 * @class JSAbilityForegroundStateObserver
 * @brief JavaScript layer Ability foreground state observer for monitoring Ability foreground state changes and notifying JS layer
 *
 * This class inherits from AbilityForegroundStateObserverStub and implements the monitoring
 * of Ability foreground state changes. When an Ability enters foreground or switches from background,
 * corresponding callbacks are triggered and JavaScript layer callback functions are invoked via napi.
 */
class JSAbilityForegroundStateObserver : public AbilityForegroundStateObserverStub {
public:
    /**
     * @brief Constructor
     * @param engine NAPI environment pointer for interacting with JavaScript layer
     */
    explicit JSAbilityForegroundStateObserver(napi_env engine);
    virtual ~JSAbilityForegroundStateObserver() = default;

    /**
     * @brief Static function to clean up resources, used to release observer-related data
     * @param data Pointer to the data to be cleaned up
     */
    static void CleanUp(void *data);

    /**
     * @brief Callback function when Ability state changes
     * @param abilityStateData Ability state data containing Ability basic information and state
     *
     * This method is called by the system when the Ability foreground state changes
     * (e.g., switching from background to foreground).
     */
    void OnAbilityStateChanged(const AbilityStateData &abilityStateData);

    /**
     * @brief Handle Ability state change event, asynchronously invoke JS layer callback function
     * @param abilityStateData Ability state data containing Ability basic information and state
     */
    void HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData);

    /**
     * @brief Invoke JavaScript layer function
     * @param value JavaScript object
     * @param methodName Name of the JavaScript method to call
     * @param argv Argument array
     * @param argc Number of arguments
     */
    void CallJsFunction(const napi_value &value, const char *methodName, const napi_value *argv, const size_t argc);

    /**
     * @brief Add JavaScript observer object
     * @param jsObserverObject JavaScript layer observer object
     */
    void AddJsObserverObject(const napi_value &jsObserverObject);

    /**
     * @brief Remove specified JavaScript observer object
     * @param jsObserverObject JavaScript observer object to be removed
     */
    void RemoveJsObserverObject(const napi_value &jsObserverObject);

    /**
     * @brief Remove all JavaScript observer objects
     */
    void RemoveAllJsObserverObject();

    /**
     * @brief Get specified observer object
     * @param jsObserverObject JavaScript observer object
     * @return NativeReference pointer of the observer object
     */
    std::shared_ptr<NativeReference> GetObserverObject(const napi_value &jsObserverObject);

    /**
     * @brief Check if specified observer object exists
     * @param jsObserverObject JavaScript observer object
     * @return Returns true if object exists, otherwise returns false
     */
    bool IsObserverObjectExsit(const napi_value &jsObserverObject);

    /**
     * @brief Check if observer object collection is empty
     * @return Returns true if collection is empty, otherwise returns false
     */
    bool IsEmpty();

    /**
     * @brief Set validity status of the observer
     * @param valid Validity flag, true indicates valid, false indicates invalid
     */
    void SetValid(bool valid);

private:
    napi_env env_;
    volatile bool valid_ = true;
    std::set<std::shared_ptr<NativeReference>> jsObserverObjectSet_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_ABILITY_FOREGROUND_STATE_OBSERVER_H
