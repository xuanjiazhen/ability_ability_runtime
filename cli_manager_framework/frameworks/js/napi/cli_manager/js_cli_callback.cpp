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

#include "js_cli_callback.h"

#include "hilog_tag_wrapper.h"
#include "js_cli_manager_utils.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t ERROR_CODE = -1;
constexpr int32_t ERR_OK = 0;
constexpr size_t CALLBACK_ARGC = 1;
}

CliCallbackNapi::CliCallbackNapi(napi_env env, napi_value jsCallback)
    : env_(env)
{
    napi_ref ref = nullptr;
    napi_status status = napi_create_reference(env_, jsCallback, 1, &ref);
    if (status != napi_ok || ref == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to create session callback reference: %{public}d", status);
        return;
    }
    callback_.reset(reinterpret_cast<NativeReference *>(ref));
}

CliCallbackNapi::~CliCallbackNapi()
{
    FreeNativeReference(callback_);
}

bool CliCallbackNapi::IsValid() const
{
    return env_ != nullptr && callback_ != nullptr;
}

void CliCallbackNapi::FreeNativeReference(std::unique_ptr<NativeReference> &reference)
{
    if (reference == nullptr || env_ == nullptr) {
        return;
    }

    uv_loop_t *loop = nullptr;
    napi_get_uv_event_loop(env_, &loop);
    if (loop == nullptr) {
        return;
    }

    uv_work_t *work = new (std::nothrow) uv_work_t;
    if (work == nullptr) {
        return;
    }
    work->data = reinterpret_cast<void *>(reference.release());
    int ret = uv_queue_work(loop, work, [](uv_work_t *work) {},
        [](uv_work_t *work, int status) {
            if (work == nullptr) {
                return;
            }
            delete reinterpret_cast<NativeReference *>(work->data);
            work->data = nullptr;
            delete work;
        });
    if (ret != 0) {
        delete reinterpret_cast<NativeReference *>(work->data);
        work->data = nullptr;
        delete work;
    }
}

int32_t CliCallbackNapi::DispatchEvent(const ToolEvent &event)
{
    if (!IsValid()) {
        return ERROR_CODE;
    }

    auto protect = shared_from_this();
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete =
        std::make_unique<NapiAsyncTask::CompleteCallback>(
            [protect, event](napi_env env, NapiAsyncTask &task, int32_t status) {
                if (protect != nullptr) {
                    protect->CallJs(event);
                }
            });
    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("CliCallbackNapi::DispatchEvent", env_,
        std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
    return ERR_OK;
}

void CliCallbackNapi::CallJs(const ToolEvent &event)
{
    if (!IsValid()) {
        return;
    }

    HandleScope handleScope(env_);
    napi_value callback = callback_->GetNapiValue();
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid session callback");
        return;
    }

    napi_value jsEvent = CreateJsToolEvent(env_, event);
    napi_value undefinedValue = nullptr;
    napi_get_undefined(env_, &undefinedValue);
    napi_call_function(env_, undefinedValue, callback, CALLBACK_ARGC, &jsEvent, nullptr);
}

} // namespace AbilityRuntime
} // namespace OHOS
