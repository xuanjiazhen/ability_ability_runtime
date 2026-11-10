/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "js_insight_intent_driver.h"

#include "ability_business_error.h"
#include "ability_manager_client.h"
#include "event_handler.h"
#include "event_runner.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_callback_interface.h"
#include "insight_intent_host_client.h"
#include "insight_intent_execute_result.h"
#include "js_error_utils.h"
#include "js_insight_intent_driver_utils.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_callback.h"
#include "napi_common_execute_param.h"
#include "napi_common_intent_info_filter.h"
#include "napi_common_query_entity_param.h"
#include "napi_common_util.h"
#include "native_engine/native_value.h"
#include "string_wrapper.h"
#include "int_wrapper.h"
#ifdef HIVIEWDFX_RUNTIME_API_METRICS
#include "histogram_plugin_macros.h"
#endif

#include <atomic>
#include <cstdint>
#include <mutex>
#include <random>

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t INDEX_THREE = 3;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
constexpr size_t ARGC_FOUR = 4;
constexpr const char *DISTRIBUTE_LIBNAME = "libhmos_distribute.z.so";
using Distribute = void(*)(InsightIntentExecuteParam&);
Distribute g_distributeFunc = nullptr;
std::mutex g_distributeMutex;
std::atomic<bool> g_distributeResolved{false};

Distribute GetDistributeFunc()
{
    if (g_distributeResolved.load(std::memory_order_acquire)) {
        return g_distributeFunc;
    }
    std::lock_guard<std::mutex> lock(g_distributeMutex);
    if (!g_distributeResolved.load(std::memory_order_relaxed)) {
        auto handle = dlopen(DISTRIBUTE_LIBNAME, RTLD_LAZY);
        if (handle == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "dlopen %{public}s error: %{public}s",
                DISTRIBUTE_LIBNAME, dlerror());
            return nullptr;
        }
        auto symbol = dlsym(handle, "Distribute");
        if (symbol == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "dlsym error: %{public}s", dlerror());
            dlclose(handle);
            return nullptr;
        }
        g_distributeFunc = reinterpret_cast<Distribute>(symbol);
        g_distributeResolved.store(true, std::memory_order_release);
    }
    return g_distributeFunc;
}

// Caller-side CancellationSignal: the object the caller creates and passes to
// execute(param, signal). It owns a NAPI-generated signalId (== the session's
// executeId) plus two shared flags:
//   - canceled: flipped locally by cancel() before the AMS IPC, so isCancelled()
//     turns true synchronously;
//   - bound: true while the signal is attached to an in-flight execution, so a
//     second execute() on the same signal can be rejected with a busy error.
struct JsCancellationSignalData {
    // UUID v4, aligned with the design's readonly signalId: string.
    std::string signalId;
    std::shared_ptr<std::atomic<bool>> canceled = nullptr;
    std::shared_ptr<std::atomic<bool>> bound = nullptr;
};

std::string GenerateSignalId()
{
    static thread_local std::mt19937_64 generator { std::random_device{}() };
    static constexpr char HEX_DIGITS[] = "0123456789abcdef";
    uint8_t bytes[16];
    for (size_t i = 0; i < sizeof(bytes); i += sizeof(uint64_t)) {
        uint64_t word = generator();
        for (size_t j = 0; j < sizeof(uint64_t); ++j) {
            bytes[i + j] = static_cast<uint8_t>(word >> (8 * j));
        }
    }
    // RFC 4122 version 4 and variant bits.
    bytes[6] = static_cast<uint8_t>((bytes[6] & 0x0F) | 0x40);
    bytes[8] = static_cast<uint8_t>((bytes[8] & 0x3F) | 0x80);
    std::string uuid;
    uuid.reserve(36);
    for (size_t i = 0; i < sizeof(bytes); ++i) {
        if (i == 4 || i == 6 || i == 8 || i == 10) {
            uuid += '-';
        }
        uuid += HEX_DIGITS[bytes[i] >> 4];
        uuid += HEX_DIGITS[bytes[i] & 0x0F];
    }
    return uuid;
}

void JsCancellationSignalFinalizer(napi_env env, void *data, void *hint)
{
    if (data != nullptr) {
        delete static_cast<JsCancellationSignalData *>(data);
    }
}

napi_value JsCancellationSignalConstructor(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    auto data = new (std::nothrow) JsCancellationSignalData();
    if (data == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "new JsCancellationSignalData failed");
        return thisVar;
    }
    data->signalId = GenerateSignalId();
    data->canceled = std::make_shared<std::atomic<bool>>(false);
    data->bound = std::make_shared<std::atomic<bool>>(false);
    napi_wrap(env, thisVar, data, JsCancellationSignalFinalizer, nullptr, nullptr);
    return thisVar;
}

napi_value JsCancellationSignalCancel(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    JsCancellationSignalData *data = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&data));
    if (data != nullptr) {
        // Flip the local flag first so isCancelled() is true before the IPC round
        // trip; cancel() is void and idempotent (no error even if the execution
        // already finished or the signal is not bound).
        if (data->canceled != nullptr) {
            data->canceled->store(true, std::memory_order_release);
        }
        AbilityManagerClient::GetInstance()->CancelInsightIntent(data->signalId);
    }
    return CreateJsUndefined(env);
}

napi_value JsCancellationSignalGetSignalId(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    JsCancellationSignalData *data = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&data));
    napi_value result = nullptr;
    if (data != nullptr) {
        napi_create_string_utf8(env, data->signalId.c_str(), data->signalId.length(), &result);
    }
    return result;
}

napi_value JsCancellationSignalIsCancelled(napi_env env, napi_callback_info info)
{
    napi_value thisVar = nullptr;
    napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    JsCancellationSignalData *data = nullptr;
    napi_unwrap(env, thisVar, reinterpret_cast<void **>(&data));
    bool isCancelled = data != nullptr && data->canceled != nullptr &&
        data->canceled->load(std::memory_order_acquire);
    napi_value result = nullptr;
    napi_get_boolean(env, isCancelled, &result);
    return result;
}

napi_value JsCancellationSignalInit(napi_env env, napi_value exportObj)
{
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION("cancel", JsCancellationSignalCancel),
        DECLARE_NAPI_FUNCTION("isCancelled", JsCancellationSignalIsCancelled),
        DECLARE_NAPI_GETTER("signalId", JsCancellationSignalGetSignalId),
    };
    napi_value constructor = nullptr;
    napi_define_class(env, "CancellationSignal", NAPI_AUTO_LENGTH, JsCancellationSignalConstructor, nullptr,
        sizeof(properties) / sizeof(properties[0]), properties, &constructor);
    if (constructor == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "define js class CancellationSignal failed");
        return nullptr;
    }
    napi_set_named_property(env, exportObj, "CancellationSignal", constructor);
    return constructor;
}
}
class JsInsightIntentExecuteCallbackClient : public InsightIntentExecuteCallbackInterface,
    public std::enable_shared_from_this<JsInsightIntentExecuteCallbackClient> {
public:
    JsInsightIntentExecuteCallbackClient(napi_env env, napi_deferred nativeDeferred, napi_ref callbackRef,
        std::shared_ptr<std::atomic<bool>> canceled = nullptr,
        std::shared_ptr<std::atomic<bool>> bound = nullptr)
        : env_(env), nativeDeferred_(nativeDeferred), callbackRef_(callbackRef), canceled_(canceled), bound_(bound) {}

    virtual ~JsInsightIntentExecuteCallbackClient() = default;

    void ProcessInsightIntentExecute(int32_t resultCode,
        AppExecFwk::InsightIntentExecuteResult executeResult) override
    {
        TAG_LOGI(AAFwkTag::INTENT, "complete");
        if (resultCode == AAFwk::INTENT_CANCELED && canceled_ != nullptr) {
            canceled_->store(true, std::memory_order_release);
        }
        // The execution session ends here; unbind the signal so it can be reused
        // for a new execute() call.
        if (bound_ != nullptr) {
            bound_->store(false, std::memory_order_release);
        }
        NapiAsyncTask::CompleteCallback complete = [resultCode = resultCode, executeResult = executeResult]
            (napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (resultCode != 0) {
                task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(resultCode)));
            } else {
                task.ResolveWithNoError(env, CreateJsExecuteResult(env, executeResult));
            }
        };
        std::unique_ptr<NapiAsyncTask> asyncTask = nullptr;
        if (nativeDeferred_) {
            asyncTask = std::make_unique<NapiAsyncTask>(nativeDeferred_, nullptr,
                std::make_unique<NapiAsyncTask::CompleteCallback>(std::move(complete)));
        } else {
            asyncTask = std::make_unique<NapiAsyncTask>(callbackRef_, nullptr,
                std::make_unique<NapiAsyncTask::CompleteCallback>(std::move(complete)));
        }
        NapiAsyncTask::Schedule("JsInsightIntentDriver::OnExecute", env_, std::move(asyncTask));
    }
private:
    napi_env env_;
    napi_deferred nativeDeferred_ = nullptr;
    napi_ref callbackRef_ = nullptr;
    std::shared_ptr<std::atomic<bool>> canceled_ = nullptr;
    std::shared_ptr<std::atomic<bool>> bound_ = nullptr;
};

class JsInsightIntentDriver {
public:
    JsInsightIntentDriver() = default;
    ~JsInsightIntentDriver() = default;

    static void Finalizer(napi_env env, void *data, void *hint)
    {
        TAG_LOGI(AAFwkTag::INTENT, "called");
        std::unique_ptr<JsInsightIntentDriver>(static_cast<JsInsightIntentDriver*>(data));
    }

    static napi_value Execute(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentDriver, OnExecute);
    }

    static napi_value GetAllInsightIntentInfo(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentDriver, OnGetAllInsightIntentInfo);
    }

    static napi_value GetInsightIntentInfoByBundleName(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentDriver, OnGetInsightIntentInfoByBundleName);
    }

    static napi_value GetInsightIntentInfoByIntentName(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentDriver, OnGetInsightIntentInfoByIntentName);
    }

    static napi_value GetInsightIntentInfoByFilter(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentDriver, OnGetInsightIntentInfoByFilter);
    }

    static napi_value QueryEntityInfo(napi_env env, napi_callback_info info)
    {
        GET_NAPI_INFO_AND_CALL(env, info, JsInsightIntentDriver, OnQueryEntityInfo);
    }

private:
    void ParseParam(InsightIntentExecuteParam &param)
    {
        Distribute func = GetDistributeFunc();
        if (func != nullptr) {
            func(param);
        }
    }

    std::shared_ptr<AbilityContext> GetAbilityContext(napi_env env, napi_value object)
    {
        void* wrapped = nullptr;
        napi_status ret = napi_unwrap(env, object, &wrapped);
        if (ret != napi_ok) {
            return nullptr;
        }

        auto weakContext = static_cast<std::weak_ptr<AbilityContext>*>(wrapped);
        return weakContext != nullptr ? weakContext->lock() : nullptr;
    }

    std::shared_ptr<JsUIExtensionCallback> GetJsUIExtensionCallback(napi_env env)
    {
        std::shared_ptr<JsUIExtensionCallback> callback = std::make_shared<JsUIExtensionCallback>(env);
        napi_value callbackFunc = nullptr;
        auto func = [](napi_env env, napi_callback_info info) -> napi_value { return nullptr; };
        napi_create_function(env, "", 0, func, nullptr, &callbackFunc);
        callback->SetJsCallbackObject(callbackFunc);

        return callback;
    }

    napi_value HandleServiceMatch(napi_env env, NapiCallbackInfo& info, InsightIntentExecuteParam& param)
    {
        HandleScope handleScope(env);
        napi_value napiIntentParam = nullptr;
        napi_get_named_property(env, info.argv[INDEX_ZERO], "insightIntentParam", &napiIntentParam);
        if (napiIntentParam == nullptr) {
            ThrowInvalidParamError(env, "Invalid insightIntentParam");
            return CreateJsUndefined(env);
        }
        napi_value inputContext = nullptr;
        napi_get_named_property(env, napiIntentParam, "abilityContext", &inputContext);
        if (inputContext == nullptr) {
            ThrowInvalidParamError(env, "Invalid abilityContext");
            return CreateJsUndefined(env);
        }
        auto abilityContext = GetAbilityContext(env, inputContext);
        if (abilityContext == nullptr) {
            ThrowInvalidParamError(env, "Invalid abilityContext");
            return CreateJsUndefined(env);
        }
        auto callback = GetJsUIExtensionCallback(env);
        if (callback == nullptr) {
            ThrowInvalidParamError(env, "Invalid callback");
            return CreateJsUndefined(env);
        }

        if (param.insightIntentParam_ != nullptr) {
            if (param.insightIntentParam_->GetStringParam("startType") == "startAbilityByType") {
                auto type = param.insightIntentParam_->GetStringParam("type");
                param.insightIntentParam_->Remove("startType");
                param.insightIntentParam_->Remove("type");
                abilityContext->StartAbilityByType(type, *param.insightIntentParam_, callback);
            }
        }
        return CreateJsUndefined(env);
    }

    bool IsServiceMatch(InsightIntentExecuteParam &param)
    {
        if (param.insightIntentParam_ != nullptr &&
            param.insightIntentParam_->GetStringParam("executeFlag") == "service_match") {
            param.isServiceMatch_ = false;
            ParseParam(param);
            if (param.isServiceMatch_) {
                #ifdef HIVIEWDFX_RUNTIME_API_METRICS
                    HISTOGRAM_BOOLEAN("Ability.InsightIntentDriver.Execute.service_match", 1);
                #endif
                return true;
            }
        }
        return false;
    }

    bool NeedStartByContext(const InsightIntentExecuteParam &param)
    {
        if (param.insightIntentParam_ != nullptr &&
            !param.insightIntentParam_->GetStringParam("startType").empty()) {
            return true;
        }
        return false;
    }
    napi_value OnExecute(napi_env env, NapiCallbackInfo& info)
    {
        HandleEscape handleEscape(env);
        TAG_LOGI(AAFwkTag::INTENT, "on execute");
        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::INTENT, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        InsightIntentExecuteParam param;
        if (!UnwrapExecuteParam(env, info.argv[INDEX_ZERO], param)) {
            TAG_LOGE(AAFwkTag::INTENT, "parse on off type failed");
            ThrowInvalidParamError(env, "Parameter error: Parse param failed, param must be a ExecuteParam.");
            return CreateJsUndefined(env);
        }

        // The optional second argument is disambiguated by type: a function is the
        // legacy success/error callback; a CancellationSignal instance enables
        // caller-driven cancellation (v0.5 execute(param, cancellationSignal)).
        napi_value lastParam = (info.argc == 1) ? nullptr : info.argv[INDEX_ONE];
        napi_valuetype type = napi_undefined;
        napi_typeof(env, lastParam, &type);

        JsCancellationSignalData *signalData = nullptr;
        if (lastParam != nullptr && type != napi_function) {
            napi_unwrap(env, lastParam, reinterpret_cast<void **>(&signalData));
        }

        napi_value result = nullptr;
        napi_deferred nativeDeferred = nullptr;
        napi_ref callbackRef = nullptr;
        std::unique_ptr<NapiAsyncTask> asyncTask = nullptr;
        if (lastParam != nullptr && type == napi_function) {
            napi_get_undefined(env, &result);
            napi_create_reference(env, lastParam, 1, &callbackRef);
            asyncTask = std::make_unique<NapiAsyncTask>(callbackRef, nullptr, nullptr);
        } else {
            napi_create_promise(env, &nativeDeferred, &result);
            asyncTask = std::make_unique<NapiAsyncTask>(nativeDeferred, nullptr, nullptr);
        }

        if (asyncTask == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null asyncTask");
            return CreateJsUndefined(env);
        }

        std::shared_ptr<std::atomic<bool>> canceled = nullptr;
        std::shared_ptr<std::atomic<bool>> bound = nullptr;
        if (signalData != nullptr) {
            canceled = signalData->canceled;
            bound = signalData->bound;
            // Fail fast: an already-cancelled signal is rejected at submission time
            // without dispatching (BusinessError 16000139, pending formal code).
            if (canceled != nullptr && canceled->load(std::memory_order_acquire)) {
                asyncTask->Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(AAFwk::INTENT_CANCELED)));
                return handleEscape.Escape(result);
            }
            // A signal can be bound to at most one in-flight execution at a time.
            if (bound != nullptr && bound->load(std::memory_order_acquire)) {
                asyncTask->Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
                return handleEscape.Escape(result);
            }
            param.signalId_ = signalData->signalId;
            if (bound != nullptr) {
                bound->store(true, std::memory_order_release);
            }
        }

        auto client = std::make_shared<JsInsightIntentExecuteCallbackClient>(env, nativeDeferred,
            callbackRef, canceled, bound);
        uint64_t key = InsightIntentHostClient::GetInstance()->AddInsightIntentExecute(client);
        if (IsServiceMatch(param) && NeedStartByContext(param)) {
            return HandleServiceMatch(env, info, param);
        }
        auto err = AbilityManagerClient::GetInstance()->ExecuteIntent(key,
            InsightIntentHostClient::GetInstance(), param);
        if (err != 0) {
            asyncTask->Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(err)));
            InsightIntentHostClient::GetInstance()->RemoveInsightIntentExecute(key);
            if (bound != nullptr) {
                bound->store(false, std::memory_order_release);
            }
        }
        return handleEscape.Escape(result);
    }

    napi_value OnGetAllInsightIntentInfo(napi_env env, NapiCallbackInfo& info)
    {
        HandleEscape handleEscape(env);
        TAG_LOGI(AAFwkTag::INTENT, "OnGetAllInsightIntentInfo");
        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::INTENT, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        GetInsightIntentFlag flag;
        if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], flag) ||
            (flag != GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT &&
            flag != GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT &&
            flag != (GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO) &&
            flag != (GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO))) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse flag failed");
            ThrowInvalidParamError(env, "Parse param flag failed, flag must be GetInsightIntentFlag.");
            return CreateJsUndefined(env);
        }
        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        auto infos = std::make_shared<std::vector<InsightIntentInfoForQuery>>();
        NapiAsyncTask::ExecuteCallback execute = [infos, flag, innerErrorCode]() {
            *innerErrorCode = AbilityManagerClient::GetInstance()->GetAllInsightIntentInfo(flag, *infos);
        };
        NapiAsyncTask::CompleteCallback complete =
            [innerErrorCode, infos](napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                if (*innerErrorCode == 0) {
                    task.ResolveWithNoError(env, CreateInsightIntentInfoForQueryArray(env, *infos));
                } else {
                    task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*innerErrorCode)));
                }
            };
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsInsightIntentDriver::OnGetAllInsightIntentInfo",
            env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnGetInsightIntentInfoByBundleName(napi_env env, NapiCallbackInfo& info)
    {
        HandleEscape handleEscape(env);
        TAG_LOGI(AAFwkTag::INTENT, "OnGetInsightIntentInfoByBundleName");
        if (info.argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::INTENT, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string bundleName;
        if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], bundleName)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse bundleName failed");
            ThrowInvalidParamError(env, "Parse param bundleName failed, bundleName must be string.");
            return CreateJsUndefined(env);
        }
        GetInsightIntentFlag flag;
        if (!ConvertFromJsValue(env, info.argv[INDEX_ONE], flag) ||
            (flag != GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT &&
            flag != GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT &&
            flag != (GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO) &&
            flag != (GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO))) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse flag failed");
            ThrowInvalidParamError(env, "Parse param flag failed, flag must be GetInsightIntentFlag.");
            return CreateJsUndefined(env);
        }
        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        auto infos = std::make_shared<std::vector<InsightIntentInfoForQuery>>();
        NapiAsyncTask::ExecuteCallback execute = [infos, flag, bundleName, innerErrorCode]() {
            *innerErrorCode = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByBundleName(
                flag, bundleName, *infos);
        };
        NapiAsyncTask::CompleteCallback complete =
            [innerErrorCode, infos](napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                if (*innerErrorCode == 0) {
                    task.ResolveWithNoError(env, CreateInsightIntentInfoForQueryArray(env, *infos));
                } else {
                    task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*innerErrorCode)));
                }
            };
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsInsightIntentDriver::OnGetInsightIntentInfoByBundleName",
            env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnGetInsightIntentInfoByIntentName(napi_env env, NapiCallbackInfo& info)
    {
        HandleEscape handleEscape(env);
        TAG_LOGI(AAFwkTag::INTENT, "OnGetInsightIntentInfoByIntentName");
        if (info.argc < ARGC_FOUR) {
            TAG_LOGE(AAFwkTag::INTENT, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }
        std::string bundleName;
        if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], bundleName)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse bundleName failed");
            ThrowInvalidParamError(env, "Parse param bundleName failed, bundleName must be string.");
            return CreateJsUndefined(env);
        }
        std::string moduleName;
        if (!ConvertFromJsValue(env, info.argv[INDEX_ONE], moduleName)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse intentName failed");
            ThrowInvalidParamError(env, "Parse param moduleName failed, moduleName must be string.");
            return CreateJsUndefined(env);
        }
        std::string intentName;
        if (!ConvertFromJsValue(env, info.argv[INDEX_TWO], intentName)) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse intentName failed");
            ThrowInvalidParamError(env, "Parse param intentName failed, intentName must be string.");
            return CreateJsUndefined(env);
        }
        GetInsightIntentFlag flag;
        if (!ConvertFromJsValue(env, info.argv[INDEX_THREE], flag) ||
            (flag != GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT &&
            flag != GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT &&
            flag != (GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO) &&
            flag != (GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT | GetInsightIntentFlag::GET_ENTITY_INFO))) {
            TAG_LOGE(AAFwkTag::INTENT, "Parse flag failed");
            ThrowInvalidParamError(env, "Parse param flag failed, flag must be GetInsightIntentFlag.");
            return CreateJsUndefined(env);
        }
        auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
        auto intentInfo = std::make_shared<InsightIntentInfoForQuery>();
        NapiAsyncTask::ExecuteCallback execute =
            [intentInfo, flag, bundleName, moduleName, intentName, innerErrorCode]() {
                *innerErrorCode = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByIntentName(
                    flag, bundleName, moduleName, intentName, *intentInfo);
            };
        NapiAsyncTask::CompleteCallback complete =
            [innerErrorCode, intentInfo](napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                if (*innerErrorCode == 0) {
                    task.ResolveWithNoError(env, CreateInsightIntentInfoForQuery(env, *intentInfo));
                } else {
                    task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*innerErrorCode)));
                }
            };
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsInsightIntentDriver::OnGetInsightIntentInfoByIntentName",
            env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnGetInsightIntentInfoByFilter(napi_env env, NapiCallbackInfo& info)
    {
        HandleEscape handleEscape(env);
        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::INTENT, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        if (!CheckValidIntentInfoFilter(env, info.argv[INDEX_ZERO])) {
            TAG_LOGE(AAFwkTag::INTENT, "check filter failed");
            ThrowInvalidParamError(env, "Param error: filter must be a valid InsightIntentInfoFilter.");
            return CreateJsUndefined(env);
        }

        InsightIntentInfoFilter filter;
        if (!UnwrapIntentInfoFilter(env, info.argv[INDEX_ZERO], filter)) {
            TAG_LOGE(AAFwkTag::INTENT, "parse filter failed");
            ThrowInvalidParamError(env, "Param error: filter must be a InsightIntentInfoFilter.");
            return CreateJsUndefined(env);
        }

        auto error = std::make_shared<int32_t>(ERR_OK);
        auto infos = std::make_shared<std::vector<InsightIntentInfoForQuery>>();
        NapiAsyncTask::ExecuteCallback execute = [infos, error, filter]() {
            if (filter.bundleName_.empty()) {
                *error = AbilityManagerClient::GetInstance()->GetAllInsightIntentInfo(
                    filter.intentFlags_, *infos, filter.userId_);
            } else if (!filter.moduleName_.empty() && !filter.intentName_.empty()) {
                auto intentInfo = std::make_shared<InsightIntentInfoForQuery>();
                *error = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByIntentName(
                    filter.intentFlags_, filter.bundleName_, filter.moduleName_,
                    filter.intentName_, *intentInfo, filter.userId_);
                if (intentInfo != nullptr && (!intentInfo->intentType.empty() || intentInfo->isConfig)) {
                    infos->push_back(*intentInfo);
                }
            } else if (filter.moduleName_.empty() && filter.intentName_.empty()) {
                *error = AbilityManagerClient::GetInstance()->GetInsightIntentInfoByBundleName(
                    filter.intentFlags_, filter.bundleName_, *infos, filter.userId_);
            }
        };
        NapiAsyncTask::CompleteCallback complete = [error, infos](napi_env env, NapiAsyncTask &task, int32_t status) {
                HandleScope handleScope(env);
                if (*error == 0) {
                    task.ResolveWithNoError(env, CreateInsightIntentInfoForQueryArray(env, *infos));
                } else {
                    task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*error)));
                }
            };
        napi_value result = nullptr;
        NapiAsyncTask::ScheduleHighQos("JsInsightIntentDriver::OnGetInsightIntentInfoByFilter",
            env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnQueryEntityInfo(napi_env env, NapiCallbackInfo& info)
    {
        TAG_LOGD(AAFwkTag::INTENT, "called");
        HandleEscape handleEscape(env);
        if (info.argc < ARGC_ONE) {
            TAG_LOGE(AAFwkTag::INTENT, "invalid argc");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        InsightIntentQueryParam param;
        if (!UnwrapQueryEntityParam(env, info.argv[INDEX_ZERO], param)) {
            TAG_LOGE(AAFwkTag::INTENT, "parse param failed");
            ThrowInvalidParamError(env, "Parameter error: Parse param failed, param must be a QueryEntityParam.");
            return CreateJsUndefined(env);
        }

        TAG_LOGD(AAFwkTag::INTENT, "UnwrapQueryParam, bundleName %{public}s, moduleName %{public}s,"
            "intentName %{public}s, className %{public}s, userId %{public}d, queryType %{public}s",
            param.bundleName_.c_str(), param.moduleName_.c_str(), param.intentName_.c_str(), param.className_.c_str(),
            param.userId_, param.queryEntityParam_.queryType_.c_str());

        if (param.queryEntityParam_.queryType_.compare("byProperty") == 0 &&
            param.queryEntityParam_.parameters_ == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "byProperty queryType must have parameters.");
            ThrowInvalidParamError(env, "Parameter error: byProperty queryType must have parameters.");
            return CreateJsUndefined(env);
        }

        napi_value result = nullptr;
        napi_deferred nativeDeferred = nullptr;
        napi_ref callbackRef = nullptr;
        std::unique_ptr<NapiAsyncTask> asyncTask = nullptr;
        napi_create_promise(env, &nativeDeferred, &result);
        asyncTask = std::make_unique<NapiAsyncTask>(nativeDeferred, nullptr, nullptr);

        if (asyncTask == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null asyncTask");
            return CreateJsUndefined(env);
        }
        auto client = std::make_shared<JsInsightIntentExecuteCallbackClient>(env, nativeDeferred, callbackRef);
        uint64_t key = InsightIntentHostClient::GetInstance()->AddInsightIntentExecute(client);
        auto err = AbilityManagerClient::GetInstance()->QueryEntityInfo(key,
            InsightIntentHostClient::GetInstance(), param);
        if (err == INTENT_NOT_EXIST) {
            std::vector<std::shared_ptr<AAFwk::WantParams>> queryResults;
            asyncTask->ResolveWithNoError(env, CreateQueryEntityResult(env, queryResults));
            InsightIntentHostClient::GetInstance()->RemoveInsightIntentExecute(key);
        } else if (err != 0) {
            asyncTask->Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(err)));
            InsightIntentHostClient::GetInstance()->RemoveInsightIntentExecute(key);
        }
        return handleEscape.Escape(result);
    }
};

static napi_status SetEnumItem(napi_env env, napi_value napiObject, const char* name, int32_t value)
{
    HandleScope handleScope(env);
    napi_status status;
    napi_value itemName;
    napi_value itemValue;

    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName), status);
    NAPI_CALL_BASE(env, status = napi_create_int32(env, value, &itemValue), status);

    NAPI_CALL_BASE(env, status = napi_set_property(env, napiObject, itemName, itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, napiObject, itemValue, itemName), status);

    return napi_ok;
}

static napi_status SetEnumItem(napi_env env, napi_value napiObject, const char* name, const char* value)
{
    HandleScope handleScope(env);
    napi_status status;
    napi_value itemName;
    napi_value itemValue;

    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, name, NAPI_AUTO_LENGTH, &itemName), status);
    NAPI_CALL_BASE(env, status = napi_create_string_utf8(env, value, NAPI_AUTO_LENGTH, &itemValue), status);

    NAPI_CALL_BASE(env, status = napi_set_property(env, napiObject, itemName, itemValue), status);
    NAPI_CALL_BASE(env, status = napi_set_property(env, napiObject, itemValue, itemName), status);

    return napi_ok;
}

static napi_value InitGetInsightIntentFlagObject(napi_env env)
{
    HandleEscape handleEscape(env);
    napi_value napiObject;
    NAPI_CALL(env, napi_create_object(env, &napiObject));

    NAPI_CALL(env, SetEnumItem(
        env, napiObject, "GET_FULL_INSIGHT_INTENT", GetInsightIntentFlag::GET_FULL_INSIGHT_INTENT));
    NAPI_CALL(env, SetEnumItem(
        env, napiObject, "GET_SUMMARY_INSIGHT_INTENT", GetInsightIntentFlag::GET_SUMMARY_INSIGHT_INTENT));
    NAPI_CALL(env, SetEnumItem(
        env, napiObject, "GET_ENTITY_INFO", GetInsightIntentFlag::GET_ENTITY_INFO));

    return handleEscape.Escape(napiObject);
}

static napi_value InitInsightIntentTypeObject(napi_env env)
{
    HandleEscape handleEscape(env);
    napi_value napiObject;
    NAPI_CALL(env, napi_create_object(env, &napiObject));

    NAPI_CALL(env, SetEnumItem(env, napiObject, "LINK", "@InsightIntentLink"));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "PAGE", "@InsightIntentPage"));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "ENTRY", "@InsightIntentEntry"));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "FUNCTION", "@InsightIntentFunctionMethod"));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "FORM", "@InsightIntentForm"));

    return handleEscape.Escape(napiObject);
}

static napi_value InitIntentDevelopTypeObject(napi_env env)
{
    HandleEscape handleEscape(env);
    napi_value napiObject;
    NAPI_CALL(env, napi_create_object(env, &napiObject));

    NAPI_CALL(env, SetEnumItem(env, napiObject, "CONFIGURATION", "configuration"));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "DECORATOR", "decorator"));

    return handleEscape.Escape(napiObject);
}

static napi_value InitExecuteModeForConfigurationObject(napi_env env)
{
    HandleEscape handleEscape(env);
    napi_value napiObject;
    NAPI_CALL(env, napi_create_object(env, &napiObject));

    NAPI_CALL(env, SetEnumItem(env, napiObject, "FOREGROUND", 0));
    NAPI_CALL(env, SetEnumItem(env, napiObject, "BACKGROUND", 1));

    return handleEscape.Escape(napiObject);
}

napi_value JsInsightIntentDriverInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::INTENT, "called");
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env or exportObj");
        return nullptr;
    }

    HandleScope handleScope(env);
    std::unique_ptr<JsInsightIntentDriver> jsIntentDriver = std::make_unique<JsInsightIntentDriver>();
    napi_wrap(env, exportObj, jsIntentDriver.release(), JsInsightIntentDriver::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsInsightIntentDriver";
    BindNativeFunction(env, exportObj, "execute", moduleName, JsInsightIntentDriver::Execute);
    JsCancellationSignalInit(env, exportObj);
    BindNativeFunction(env, exportObj,
        "getAllInsightIntentInfo", moduleName, JsInsightIntentDriver::GetAllInsightIntentInfo);
    BindNativeFunction(env, exportObj,
        "getInsightIntentInfoByBundleName", moduleName, JsInsightIntentDriver::GetInsightIntentInfoByBundleName);
    BindNativeFunction(env, exportObj,
        "getInsightIntentInfoByIntentName", moduleName, JsInsightIntentDriver::GetInsightIntentInfoByIntentName);
    BindNativeFunction(env, exportObj,
        "getInsightIntentInfoByFilter", moduleName, JsInsightIntentDriver::GetInsightIntentInfoByFilter);
    BindNativeFunction(env, exportObj,
        "queryEntityInfo", moduleName, JsInsightIntentDriver::QueryEntityInfo);
    napi_value getInsightIntentFlag = InitGetInsightIntentFlagObject(env);
    NAPI_ASSERT(env, getInsightIntentFlag != nullptr, "failed to create getInsightIntent flag object");
    napi_value insightIntentType = InitInsightIntentTypeObject(env);
    NAPI_ASSERT(env, insightIntentType != nullptr, "failed to create insightIntent type flag object");
    napi_value intentDevelopType = InitIntentDevelopTypeObject(env);
    NAPI_ASSERT(env, intentDevelopType != nullptr, "failed to create develop type object");
    napi_value executeModeForConfiguration = InitExecuteModeForConfigurationObject(env);
    NAPI_ASSERT(env, executeModeForConfiguration != nullptr,
        "failed to create execute mode for configuration object");

    napi_property_descriptor exportObjs[] = {
        DECLARE_NAPI_PROPERTY("GetInsightIntentFlag", getInsightIntentFlag),
        DECLARE_NAPI_PROPERTY("InsightIntentType", insightIntentType),
        DECLARE_NAPI_PROPERTY("DevelopType", intentDevelopType),
        DECLARE_NAPI_PROPERTY("ExecuteModeForConfiguration", executeModeForConfiguration),
    };
    napi_status status = napi_define_properties(env, exportObj, sizeof(exportObjs) / sizeof(exportObjs[0]), exportObjs);
    NAPI_ASSERT(env, status == napi_ok, "failed to define properties for exportObj");
    return CreateJsUndefined(env);
}
} // namespace AbilityRuntime
} // namespace OHOS
