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

#include "js_process_lifecycle_observer.h"

#include "event_handler.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr char ON_PROCESS_CREATED[] = "onProcessCreated";
constexpr char ON_PROCESS_EXITED[] = "onProcessExited";
constexpr char PID[] = "pid";
constexpr char PROCESS_NAME[] = "processName";
constexpr char UID[] = "uid";
constexpr char STATE[] = "state";
}  // namespace

JsProcessLifecycleObserver::JsProcessLifecycleObserver(napi_env env) : env_(env) {}

void JsProcessLifecycleObserver::CallJsFunction(napi_value value, const char *methodName,
    const napi_value *argv, size_t argc)
{
    if (value == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null value");
        return;
    }
    napi_value method = nullptr;
    napi_get_named_property(env_, value, methodName, &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null method %{public}s", methodName);
        return;
    }
    napi_call_function(env_, value, method, argc, argv, nullptr);
}

void JsProcessLifecycleObserver::HandleOnProcessLifecycleEvent(
    const OHOS::AppExecFwk::ProcessLifecycleData &data, const std::string &methodName)
{
    TAG_LOGD(AAFwkTag::APPKIT, "MethodName = %{public}s, pid:%{public}d", methodName.c_str(), data.pid);
    std::map<int32_t, std::shared_ptr<NativeReference>> jsObserverMapCopy;
    {
        std::lock_guard<std::mutex> lock(jsObserverMapLock_);
        jsObserverMapCopy = jsObserverMap_;
    }
    napi_value infoObj = nullptr;
    napi_create_object(env_, &infoObj);
    if (infoObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "create info object failed");
        return;
    }
    napi_value pidValue = nullptr;
    napi_create_int32(env_, data.pid, &pidValue);
    napi_set_named_property(env_, infoObj, PID, pidValue);
    napi_value nameValue = nullptr;
    napi_create_string_utf8(env_, data.processName.c_str(), NAPI_AUTO_LENGTH, &nameValue);
    napi_set_named_property(env_, infoObj, PROCESS_NAME, nameValue);
    napi_value uidValue = nullptr;
    napi_create_int32(env_, data.uid, &uidValue);
    napi_set_named_property(env_, infoObj, UID, uidValue);
    napi_value stateValue = nullptr;
    napi_create_string_utf8(env_, data.state.c_str(), NAPI_AUTO_LENGTH, &stateValue);
    napi_set_named_property(env_, infoObj, STATE, stateValue);
    for (const auto &item : jsObserverMapCopy) {
        auto obj = item.second->GetNapiValue();
        if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
            TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
            continue;
        }
        CallJsFunction(obj, methodName.c_str(), &infoObj, 1);
    }
}

void JsProcessLifecycleObserver::OnProcessCreated(const OHOS::AppExecFwk::ProcessLifecycleData &data)
{
    HandleOnProcessLifecycleEvent(data, ON_PROCESS_CREATED);
}

void JsProcessLifecycleObserver::OnProcessExited(const OHOS::AppExecFwk::ProcessLifecycleData &data)
{
    HandleOnProcessLifecycleEvent(data, ON_PROCESS_EXITED);
}

int32_t JsProcessLifecycleObserver::AddJsObserverObjectWithId(int32_t observerId, const napi_value &jsObserverObject)
{
    if (env_ == nullptr || jsObserverObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or jsObserverObject");
        return ERR_INVALID_VALUE;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsObserverObject, 1, &ref);
    std::shared_ptr<NativeReference> nativeRef(reinterpret_cast<NativeReference*>(ref));
    std::lock_guard<std::mutex> lock(jsObserverMapLock_);
    jsObserverMap_[observerId] = nativeRef;
    return ERR_OK;
}

void JsProcessLifecycleObserver::RemoveJsObserverObjectById(int32_t observerId)
{
    std::lock_guard<std::mutex> lock(jsObserverMapLock_);
    jsObserverMap_.erase(observerId);
}

bool JsProcessLifecycleObserver::IsEmpty()
{
    std::lock_guard<std::mutex> lock(jsObserverMapLock_);
    return jsObserverMap_.empty();
}
}  // namespace AbilityRuntime
}  // namespace OHOS
