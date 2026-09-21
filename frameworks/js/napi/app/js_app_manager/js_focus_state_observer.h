/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_FOCUS_STATE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_JS_FOCUS_STATE_OBSERVER_H

#include <map>
#include <mutex>

#include "event_handler.h"
#include "focus_state_data.h"
#include "focus_state_observer_stub.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
using OHOS::AppExecFwk::FocusStateObserverStub;
using OHOS::AppExecFwk::FocusStateData;
class JSFocusStateObserver : public FocusStateObserverStub {
public:
    explicit JSFocusStateObserver(napi_env engine);
    virtual ~JSFocusStateObserver() = default;
    void OnFocusStateChanged(const FocusStateData &focusStateData) override;
    void HandleOnFocusStateChanged(const FocusStateData &focusStateData);
    void CallJsFunction(const napi_value value, const char *methodName, const napi_value *argv, const size_t argc);
    void AddJsObserverObjectWithId(int32_t observerId, const napi_value &jsObserverObject);
    void RemoveJsObserverObjectById(int32_t observerId);
    void RemoveAllJsObserverObjects();
    bool IsEmpty();
    void SetValid(bool valid);

private:
    napi_env env_;
    volatile bool valid_ = true;
    std::map<int32_t, std::shared_ptr<NativeReference>> jsObserverMap_;
    std::mutex jsObserverMapLock_;
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_FOCUS_STATE_OBSERVER_H
