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

#include "ets_startup_task_executor.h"
#include "ets_runtime_utils.h"
#include "ets_startup_task_result.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
int32_t EtsStartupTaskExecutor::RunOnMainThread(STSRuntime &etsRuntime,
    const std::unique_ptr<STSNativeReference> &startup, const std::shared_ptr<STSNativeReference> &context,
    std::unique_ptr<StartupTaskResultCallback> callback)
{
    auto env = etsRuntime.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    ani_ref returnVal = nullptr;
    int32_t resultCode = CallStartupInit(env, startup, context, callback, returnVal);
    if (resultCode != ERR_OK) {
        return resultCode;
    }
    return HandleReturnVal(env, returnVal, callback);
}

int32_t EtsStartupTaskExecutor::RunOnTaskPool(
    STSRuntime &etsRuntime,
    const std::unique_ptr<STSNativeReference> &startup,
    const std::shared_ptr<STSNativeReference> &context,
    const std::unique_ptr<STSNativeReference> &asyncTaskExcutor,
    const std::unique_ptr<STSNativeReference> &asyncTaskCallback,
    const std::string &startupName)
{
    TAG_LOGD(AAFwkTag::STARTUP, "RunOnMainThread called");
    auto env = etsRuntime.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    if (startup == nullptr || context == nullptr || asyncTaskExcutor == nullptr || asyncTaskCallback == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null args");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    // napi_value asyncTaskExcutorValue = asyncTaskExcutor->GetNapiValue();
    // if (!CheckTypeForNapiValue(env, asyncTaskExcutorValue, napi_object)) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "not napi object");
    //     return ERR_STARTUP_INTERNAL_ERROR;
    // }
    // napi_value asyncPushTask = nullptr;
    // napi_get_named_property(env, asyncTaskExcutorValue, "asyncPushTask", &asyncPushTask);
    // if (asyncPushTask == nullptr) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "null asyncPushTask");
    //     return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    // }
    // bool isCallable = false;
    // napi_is_callable(env, asyncPushTask, &isCallable);
    // if (!isCallable) {
    //     TAG_LOGE(AAFwkTag::STARTUP, "AsyncPushTask not callable");
    //     return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    // }
    // napi_value returnVal = nullptr;
    // napi_value argv[] = { startup->GetNapiValue(), asyncTaskCallback->GetNapiValue(),
    //     context->GetNapiValue(), CreateJsValue(env, startupName) };
    // napi_call_function(env, asyncTaskExcutorValue, asyncPushTask, ARGC_FOUR, argv, &returnVal);
}

int32_t EtsStartupTaskExecutor::CallStartupInit(ani_env *env, const std::unique_ptr<NativeReference> &startup,
    const std::shared_ptr<STSNativeReference> &context, std::unique_ptr<StartupTaskResultCallback> &callback,
    ani_ref &returnVal)
{
    // if (startup == nullptr || context == nullptr) {
    //     ReplyFailed(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
    //         "startup task is null or context is null.");
    //     return ERR_STARTUP_INTERNAL_ERROR;
    // }
    // napi_value startupValue = startup->GetNapiValue();
    // if (!CheckTypeForNapiValue(env, startupValue, napi_object)) {
    //     ReplyFailed(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
    //         "startup task is not napi object.");
    //     return ERR_STARTUP_INTERNAL_ERROR;
    // }
    // napi_value startupInit = nullptr;
    // napi_get_named_property(env, startupValue, "init", &startupInit);
    // if (startupInit == nullptr) {
    //     ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
    //         "failed to get property init from startup task.");
    //     return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    // }
    // bool isCallable = false;
    // napi_is_callable(env, startupInit, &isCallable);
    // if (!isCallable) {
    //     ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
    //         "startup task init is not callable.");
    //     return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    // }
    // napi_value argv[1] = { context->GetNapiValue() };
    // napi_call_function(env, startupValue, startupInit, 1, argv, &returnVal);
    return ERR_OK;
}

int32_t EtsStartupTaskExecutor::HandleReturnVal(ani_env *env, napi_value returnVal,
    std::unique_ptr<StartupTaskResultCallback> &callback)
{
    // bool isPromise = false;
    // napi_is_promise(env, returnVal, &isPromise);
    // if (!isPromise) {
    //     ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
    //         "the return value of startup task init is not promise.");
    //     return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    // }

    // auto *callbackPtr = callback.release();
    // napi_value promiseThen = nullptr;
    // napi_get_named_property(env, returnVal, "then", &promiseThen);
    // napi_value promiseResolveCallback = nullptr;
    // napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
    //     ResolveResultCallback, callbackPtr, &promiseResolveCallback);
    // napi_value argvThen[1] = { promiseResolveCallback };
    // napi_call_function(env, returnVal, promiseThen, 1, argvThen, nullptr);

    // napi_value promiseCatch = nullptr;
    // napi_get_named_property(env, returnVal, "catch", &promiseCatch);
    // napi_value promiseRejectCallback = nullptr;
    // napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
    //     RejectResultCallback, callbackPtr, &promiseRejectCallback);
    // napi_value argvCatch[1] = { promiseRejectCallback };
    // napi_call_function(env, returnVal, promiseCatch, 1, argvCatch, nullptr);
    return ERR_OK;
}

napi_value EtsStartupTaskExecutor::ResolveResultCallback(ani_env *env, napi_callback_info info)
{
    // TAG_LOGD(AAFwkTag::STARTUP, "enter");
    // size_t argc = 1;
    // napi_value argv[1] = { nullptr };
    // void *data = nullptr;
    // napi_get_cb_info(env, info, &argc, argv, nullptr, &data);
    // auto *callback = static_cast<StartupTaskResultCallback *>(data);
    // napi_value resultJs = argv[0];
    // napi_ref resultRef = nullptr;
    // napi_create_reference(env, resultJs, 1, &resultRef);
    // std::shared_ptr<NativeReference> result(reinterpret_cast<NativeReference*>(resultRef));
    // ReplySucceeded(callback, result);
    return nullptr;
}

// napi_value EtsStartupTaskExecutor::RejectResultCallback(napi_env env, napi_callback_info info)
// {
//     TAG_LOGD(AAFwkTag::STARTUP, "enter");
//     void *data = nullptr;
//     napi_get_cb_info(env, info, nullptr, nullptr, nullptr, &data);
//     auto *callback = static_cast<StartupTaskResultCallback *>(data);
//     ReplyFailed(callback, ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
//         "the promise of startup task init is reject.");
//     return nullptr;
// }

// void EtsStartupTaskExecutor::ReplyFailed(StartupTaskResultCallback *callback,
//     int32_t resultCode, const std::string &resultMessage)
// {
//     TAG_LOGD(AAFwkTag::STARTUP, "enter");
//     if (callback == nullptr) {
//         return;
//     }
//     std::shared_ptr<StartupTaskResult> result = std::make_shared<JsStartupTaskResult>(resultCode, resultMessage);
//     callback->Call(result);
//     delete callback;
//     callback = nullptr;
// }

// void EtsStartupTaskExecutor::ReplyFailed(std::unique_ptr<StartupTaskResultCallback> callback,
//     int32_t resultCode, const std::string &resultMessage)
// {
//     TAG_LOGE(AAFwkTag::STARTUP, "execute failed: %{public}s", resultMessage.c_str());
//     if (callback == nullptr) {
//         return;
//     }
//     std::shared_ptr<StartupTaskResult> result = std::make_shared<JsStartupTaskResult>(resultCode, resultMessage);
//     callback->Call(result);
// }

// void EtsStartupTaskExecutor::ReplySucceeded(StartupTaskResultCallback *callback,
//     const std::shared_ptr<NativeReference> &resultRef)
// {
//     TAG_LOGD(AAFwkTag::STARTUP, "enter");
//     if (callback == nullptr) {
//         return;
//     }
//     std::shared_ptr<StartupTaskResult> result = std::make_shared<JsStartupTaskResult>(resultRef);
//     callback->Call(result);
//     delete callback;
//     callback = nullptr;
// }
} // namespace AbilityRuntime
} // namespace OHOS
