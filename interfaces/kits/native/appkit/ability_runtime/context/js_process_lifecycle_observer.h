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

#ifndef OHOS_ABILITY_RUNTIME_JS_PROCESS_LIFECYCLE_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_JS_PROCESS_LIFECYCLE_OBSERVER_H

#include <atomic>
#include <map>
#include <mutex>

#include "native_engine/native_engine.h"
#include "nocopyable.h"
#include "process_lifecycle_observer_stub.h"

namespace OHOS {
namespace AppExecFwk {
class EventHandler;
}
}

namespace OHOS {
namespace AbilityRuntime {
class JsProcessLifecycleObserver : public OHOS::AppExecFwk::ProcessLifecycleObserverStub {
public:
    explicit JsProcessLifecycleObserver(napi_env env);
    ~JsProcessLifecycleObserver() override = default;
    void OnProcessCreated(const OHOS::AppExecFwk::ProcessLifecycleData &data) override;
    void OnProcessExited(const OHOS::AppExecFwk::ProcessLifecycleData &data) override;
    void HandleOnProcessLifecycleEvent(const OHOS::AppExecFwk::ProcessLifecycleData &data,
        const std::string &methodName);
    void CallJsFunction(napi_value value, const char *methodName, const napi_value *argv, size_t argc);
    int32_t AddJsObserverObjectWithId(int32_t observerId, const napi_value &jsObserverObject);
    void RemoveJsObserverObjectById(int32_t observerId);
    bool IsEmpty();

private:
    napi_env env_;
    std::shared_ptr<OHOS::AppExecFwk::EventHandler> handler_;
    std::map<int32_t, std::shared_ptr<NativeReference>> jsObserverMap_;
    std::mutex jsObserverMapLock_;

    DISALLOW_COPY_AND_MOVE(JsProcessLifecycleObserver);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_PROCESS_LIFECYCLE_OBSERVER_H
