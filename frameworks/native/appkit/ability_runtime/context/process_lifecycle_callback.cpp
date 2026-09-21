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

#include "process_lifecycle_callback.h"

#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
napi_value CreateJsProcessLifecycleInfo(napi_env env, const ProcessLifecycleInfo &info)
{
    napi_value obj = nullptr;
    napi_create_object(env, &obj);

    napi_value pid = nullptr;
    napi_create_int32(env, info.pid, &pid);
    napi_set_named_property(env, obj, "pid", pid);

    napi_value processName = nullptr;
    napi_create_string_utf8(env, info.processName.c_str(), info.processName.length(), &processName);
    napi_set_named_property(env, obj, "processName", processName);

    napi_value uid = nullptr;
    napi_create_int32(env, info.uid, &uid);
    napi_set_named_property(env, obj, "uid", uid);

    napi_value state = nullptr;
    napi_create_string_utf8(env, info.state.c_str(), info.state.length(), &state);
    napi_set_named_property(env, obj, "state", state);

    return obj;
}
} // namespace

JsProcessLifecycleCallback::JsProcessLifecycleCallback(napi_env env, napi_value callback) : env_(env)
{
    if (env_ == nullptr || callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or callback");
        return;
    }
    napi_ref ref = nullptr;
    napi_create_reference(env_, callback, 1, &ref);
    callback_ = std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref));
}

void JsProcessLifecycleCallback::CallJsMethod(const ProcessLifecycleInfo &info, const std::string &methodName)
{
    TAG_LOGD(AAFwkTag::APPKIT, "MethodName = %{public}s, pid = %{public}d", methodName.c_str(), info.pid);
    if (env_ == nullptr || callback_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or callback");
        return;
    }
    std::weak_ptr<JsProcessLifecycleCallback> thisWeakPtr(shared_from_this());
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>(
        [thisWeakPtr, info, methodName](napi_env env, NapiAsyncTask &task, int32_t status) {
            std::shared_ptr<JsProcessLifecycleCallback> jsCallback = thisWeakPtr.lock();
            if (jsCallback == nullptr || jsCallback->callback_ == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "null jsCallback or callback");
                return;
            }
            auto obj = jsCallback->callback_->GetNapiValue();
            if (!CheckTypeForNapiValue(env, obj, napi_object)) {
                TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
                return;
            }
            napi_value method = nullptr;
            napi_get_named_property(env, obj, methodName.data(), &method);
            if (method == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "null method %{public}s", methodName.data());
                return;
            }
            napi_value infoArg = CreateJsProcessLifecycleInfo(env, info);
            napi_call_function(env, obj, method, 1, &infoArg, nullptr);
        }
    );
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsProcessLifecycleCallback::CallJsMethod:" + methodName,
        env_, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
}

void JsProcessLifecycleCallback::OnProcessCreated(const ProcessLifecycleInfo &info)
{
    CallJsMethod(info, "onProcessCreated");
}

void JsProcessLifecycleCallback::OnProcessExited(const ProcessLifecycleInfo &info)
{
    CallJsMethod(info, "onProcessExited");
}
} // namespace AbilityRuntime
} // namespace OHOS
