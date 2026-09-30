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

#include "process_lifecycle_callback.h"

#include "hilog_tag_wrapper.h"
#include "js_data_struct_converter.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
JsProcessLifecycleCallback::JsProcessLifecycleCallback(napi_env env) : env_(env) {}

napi_value ConvertProcessDataToJS(napi_env env, const ProcessLifecycleData &data)
{
    napi_value obj = nullptr;
    napi_create_object(env, &obj);
    napi_value pid = nullptr;
    napi_create_int32(env, data.pid, &pid);
    napi_set_named_property(env, obj, "pid", pid);
    napi_value processName = nullptr;
    napi_create_string_utf8(env, data.processName.c_str(), NAPI_AUTO_LENGTH, &processName);
    napi_set_named_property(env, obj, "processName", processName);
    napi_value uid = nullptr;
    napi_create_int32(env, data.uid, &uid);
    napi_set_named_property(env, obj, "uid", uid);
    napi_value state = nullptr;
    std::string stateStr = (data.state == ProcessLifecycleState::CREATED) ? "CREATED" : "EXITED";
    napi_create_string_utf8(env, stateStr.c_str(), NAPI_AUTO_LENGTH, &state);
    napi_set_named_property(env, obj, "state", state);
    return obj;
}

void JsProcessLifecycleCallback::CallJsMethod(const std::string &methodName, const ProcessLifecycleData &data)
{
    TAG_LOGD(AAFwkTag::APPKIT, "MethodName = %{public}s", methodName.c_str());
    std::weak_ptr<JsProcessLifecycleCallback> thisWeakPtr(shared_from_this());
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [thisWeakPtr, methodName, data, callbacks = callbacks_]
        (napi_env env, NapiAsyncTask &task, int32_t status) {
            std::shared_ptr<JsProcessLifecycleCallback> jsCallback = thisWeakPtr.lock();
            if (!jsCallback) {
                return;
            }
            napi_value jsData = ConvertProcessDataToJS(env, data);
            for (auto &callback : callbacks) {
                if (!callback) {
                    TAG_LOGE(AAFwkTag::APPKIT, "Invalid jsCallback");
                    continue;
                }
                auto obj = callback->GetNapiValue();
                if (!CheckTypeForNapiValue(env, obj, napi_object)) {
                    TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
                    continue;
                }
                napi_value method = nullptr;
                napi_get_named_property(env, obj, methodName.data(), &method);
                if (method == nullptr) {
                    TAG_LOGE(AAFwkTag::APPKIT, "null method %{public}s", methodName.data());
                    continue;
                }
                napi_call_function(env, obj, method, 1, &jsData, nullptr);
            }
        }
    );
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsProcessLifecycleCallback::CallJsMethod:" + methodName,
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsProcessLifecycleCallback::OnProcessCreated(const ProcessLifecycleData &data)
{
    CallJsMethod("onProcessCreated", data);
}

void JsProcessLifecycleCallback::OnProcessExited(const ProcessLifecycleData &data)
{
    CallJsMethod("onProcessExited", data);
}

void JsProcessLifecycleCallback::Register(napi_value jsCallback)
{
    if (env_ == nullptr || jsCallback == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or jsCallback");
        return;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env_, jsCallback, 1, &ref);
    callbacks_.emplace(std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
}

bool JsProcessLifecycleCallback::UnRegister(napi_value jsCallback)
{
    if (jsCallback == nullptr) {
        TAG_LOGI(AAFwkTag::APPKIT, "null jsCallback");
        callbacks_.clear();
        return true;
    }
    for (auto &callback : callbacks_) {
        if (!callback) {
            TAG_LOGE(AAFwkTag::APPKIT, "Invalid jsCallback");
            continue;
        }
        napi_value value = callback->GetNapiValue();
        if (value == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null value");
            continue;
        }
        bool isEqual = false;
        napi_strict_equals(env_, value, jsCallback, &isEqual);
        if (isEqual) {
            return callbacks_.erase(callback) == 1;
        }
    }
    return false;
}

bool JsProcessLifecycleCallback::IsEmpty() const
{
    return callbacks_.empty();
}
} // namespace AbilityRuntime
} // namespace OHOS
