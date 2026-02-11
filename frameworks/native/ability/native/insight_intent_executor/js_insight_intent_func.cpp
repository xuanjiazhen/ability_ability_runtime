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

#include "js_insight_intent_func.h"

#include <want_params.h>

#include "hilog_tag_wrapper.h"
#include "insight_intent_constant.h"
#include "insight_intent_execute_result.h"
#include "js_insight_intent_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_execute_result.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "native_reference.h"
#include "string_wrapper.h"

#undef STATE_PATTERN_NAIVE_H
#define STATE_PATTERN_NAIVE_STATE state_
#include "state_pattern_naive.h"

#define TMP_NAPI_ANONYMOUS_FUNC "_"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<JsInsightIntentFunc> JsInsightIntentFunc::Create(JsRuntime& runtime)
{
    return std::make_shared<JsInsightIntentFunc>(runtime);
}

JsInsightIntentFunc::JsInsightIntentFunc(JsRuntime& runtime) : runtime_(runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "constructor");
}

JsInsightIntentFunc::~JsInsightIntentFunc()
{
    state_ = State::DESTROYED;
    TAG_LOGI(AAFwkTag::INTENT, "destructor");
}
namespace {

std::unique_ptr<NativeReference> entityObj_ = nullptr;

napi_value GetTargetMethod1(napi_env env, napi_value constructor, const std::string &methodName)
{
    TAG_LOGD(AAFwkTag::INTENT, "methodName %{public}s", methodName.c_str());
    napi_value method;
    auto status = napi_get_named_property(env, constructor, methodName.c_str(), &method);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "get method failed %{public}d", status);
        return nullptr;
    }

    // Check validity of method
    napi_value undefined;
    status = napi_get_undefined(env, &undefined);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "get undefined failed %{public}d", status);
        return nullptr;
    }

    bool isUndefined;
    status = napi_strict_equals(env, method, undefined, &isUndefined);
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::INTENT, "target method %{public}s didn't exist", methodName.c_str());
        return nullptr;
    }

    return method;
}

// bool UnWrapItem(napi_env env, napi_value item)
// {
//     // get type
//     napi_valuetype type = napi_undefined;
//     napi_status status = napi_typeof(env, item, &type);
//     if (status != napi_ok || type == napi_undefined) {
//         TAG_LOGE(AAFwkTag::INTENT, "get type failed");
//         return napi_invalid_arg;
//     }

//     // get property name
//     std::string propName;
//     if (!AbilityRuntime::ConvertFromJsValue(env, item, propName)) {
//         TAG_LOGE(AAFwkTag::INTENT, "convert napi value failed");
//         return false;
//     }

//     switch (type) {
//         case napi_number:
//         {
//             TAG_LOGE(AAFwkTag::INTENT, "unwrap int32");
//             int32_t propValue;
//             if (!AppExecFwk::UnwrapInt32ByPropertyName(env, item, propName.c_str(), propValue)) {
//                 TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type %{public}s", propName.c_str());
//                 return false;
//             }
//             TAG_LOGE(AAFwkTag::INTENT, "testTag property name:%{public}s, value:%{public}d",
//                 propName.c_str(), propValue);
//             break;
//         }
//         case napi_string:
//         {
//             TAG_LOGE(AAFwkTag::INTENT, "unwrap string");
//             std::string propValue;
//             if (!AppExecFwk::UnwrapStringByPropertyName(env, item, propName.c_str(), propValue)) {
//                 TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type %{public}s", propName.c_str());
//                 return false;
//             }
//             TAG_LOGE(AAFwkTag::INTENT, "testTag property name:%{public}s, value:%{public}s",
//                 propName.c_str(), propValue.c_str());
//             break;
//         }
//         case napi_boolean:
//         {
//             TAG_LOGE(AAFwkTag::INTENT, "unwrap boolean");
//             bool propValue;
//             if (!AppExecFwk::UnwrapBooleanByPropertyName(env, item, propName.c_str(), propValue)) {
//                 TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type %{public}s", propName.c_str());
//                 return false;
//             }
//             break;
//         }
//         default:
//             TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type %{public}d", type);
//             return false;
//     }
//     return true;
// }

bool UnwrapExecuteParam(napi_env env, napi_value param)
{
    TAG_LOGE(AAFwkTag::INTENT, "testTag UnwrapExecuteParam start");
    if (!AppExecFwk::IsTypeForNapiValue(env, param, napi_object)) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid params");
        return false;
    }

    AppExecFwk::ComplexArrayData jsObjStringIntentEntity;
    if (!AppExecFwk::UnwrapArrayComplexFromJS(env, param, jsObjStringIntentEntity)) {
        TAG_LOGE(AAFwkTag::INTENT, "jsObjStringIntentEntity not array.");
        return false;
    }

    size_t jsObjStringIntentEntitySize = jsObjStringIntentEntity.objectList.size();
    TAG_LOGE(AAFwkTag::INTENT, "testTag jsObjStringIntentEntitySize %{public}d", jsObjStringIntentEntitySize);
    for(size_t i = 0; i < jsObjStringIntentEntitySize; i++) {
        std::string entityId {""};
        std::string displayName {""};

        auto item = jsObjStringIntentEntity.objectList[i];
        if (!AppExecFwk::UnwrapStringByPropertyName(env, item, "entityId", entityId)) {
            TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type entityId");
            continue;
        }

        if (AppExecFwk::IsExistsByPropertyName(env, item, "displayName")) {
            if (!AppExecFwk::UnwrapStringByPropertyName(env, item, "displayName", displayName)) {
                TAG_LOGE(AAFwkTag::INTENT, "Wrong argument type displayName");
                continue;
            }
        }

        TAG_LOGI(AAFwkTag::INTENT, "testTag entityId:%{public}s, displayName:%{public}s",
            entityId.c_str(), displayName.c_str());

        AAFwk::WantParams wp;
        if(!AppExecFwk::UnwrapWantParams(env, item, wp)) {
            TAG_LOGE(AAFwkTag::INTENT, "testTag UnwrapWantParams failed");
            continue;
        }
        TAG_LOGE(AAFwkTag::INTENT, "testTag item:%{public}s", wp.ToString().c_str());

        auto entityId1 = wp.GetStringParam("entityId");
        auto displayName1 = wp.GetStringParam("displayName");
        auto name = wp.GetStringParam("name");
        auto value = wp.GetStringParam("value");
        auto description = wp.GetStringParam("description");
        auto extension = wp.GetIntParam("extension", 0);

        TAG_LOGE(AAFwkTag::INTENT, "testTag %{public}s, %{public}s, %{public}s, %{public}s, %{public}s, %{public}d",
            entityId1.c_str(), displayName1.c_str(), name.c_str(), value.c_str(), description.c_str(), extension);
    }
    TAG_LOGE(AAFwkTag::INTENT, "UnwrapExecuteParam end");
    return true;
}

napi_value OnPromiseCallback(napi_env env, napi_callback_info info)
{
    TAG_LOGE(AAFwkTag::INTENT, "testTag OnPromiseCallback start");
    void *data = nullptr;
    size_t argc = ARGC_MAX_COUNT;
    napi_value argv[ARGC_MAX_COUNT] = {nullptr};
    NAPI_CALL_NO_THROW(napi_get_cb_info(env, info, &argc, argv, nullptr, &data), nullptr);

    auto *callbackInfo = static_cast<AppExecFwk::AbilityTransactionCallbackInfo<napi_value> *>(data);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null callbackInfo or unwrap flag result failed");
        return nullptr;
    }
    callbackInfo->Call(argv[0]);

    AppExecFwk::AbilityTransactionCallbackInfo<napi_value>::Destroy(callbackInfo);
    data = nullptr;
    TAG_LOGE(AAFwkTag::INTENT, "testTag OnPromiseCallback end");
    return nullptr;
}

std::unique_ptr<NativeReference> LoadJsEntityCode(
    const InsightIntentExecutorInfo& info,
    JsRuntime& runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "LoadJsEntityCode start");

    auto executeParam = info.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return std::unique_ptr<NativeReference>();
    }

    std::string moduleName(executeParam->moduleName_);
    std::string hapPath(info.hapPath);
    std::string srcEntrance(executeParam->srcEntrance_);
    TAG_LOGE(AAFwkTag::INTENT, "testTag moduleName:%{public}s, hapPath:%{public}s, srcEntrance:%{public}s, className:%{public}s",
        moduleName.c_str(), hapPath.c_str(), srcEntrance.c_str(), executeParam->className_.c_str());

    auto ret = runtime.ExecuteSecureWithOhmUrl(moduleName, hapPath, srcEntrance);
    if (!ret) {
        TAG_LOGE(AAFwkTag::INTENT, "execute failed");
        return std::unique_ptr<NativeReference>();
    }

    TAG_LOGE(AAFwkTag::INTENT, "insightIntentParam: %{public}s", executeParam->insightIntentParam_->ToString().c_str());
    std::string className = executeParam->className_;
    napi_value exportObj = runtime.GetExportObjectFromOhmUrl(srcEntrance, className);
    // napi_value exportObj = runtime.GetExportObjectFromOhmUrl(srcEntrance, "StringIntentEntityImpl");

    if (exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get default object failed");
        // return std::unique_ptr<NativeReference>();
    }
    TAG_LOGE(AAFwkTag::INTENT, "get default object success");

    auto* env = runtime.GetNapiEnv();
    napi_value instanceValue = nullptr;
    auto status = napi_new_instance(env, exportObj, 0, nullptr, &instanceValue);
    if (status != napi_ok || instanceValue == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "new instance failed %{public}d", status);
        return std::unique_ptr<NativeReference>();
    }

    napi_ref resultRef = nullptr;
    status = napi_create_reference(env, instanceValue, 1, &resultRef);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "create instance failed %{public}d", status);
        return std::unique_ptr<NativeReference>();
    }

    return std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(resultRef));
}

bool SetEntityObj(napi_env env, napi_value entityObj)
{
    TAG_LOGE(AAFwkTag::INTENT, "testTag SetEntityObj start");

    UnwrapExecuteParam(env, entityObj);
    
    // 设置entity到executorObj的targetStringEntities字段上
    // auto status = napi_set_named_property(env, executorObj, "targetStringEntities", entityObj);
    // if (status != napi_ok) {
    //     TAG_LOGE(AAFwkTag::INTENT, "set targetStringEntities property failed with %{public}d", status);
    //     return false;
    // }

    // 打印executorObj的所有字段
    // napi_value propertys;
    // NAPI_CALL_BASE(env, napi_get_property_names(env, executorObj, &propertys), false);
    // uint32_t propCnt;
    // NAPI_CALL_BASE(env, napi_get_array_length(env, propertys, &propCnt), false);
    // for (uint32_t i = 0; i < propCnt; i++) {
    //     napi_value propKey;
    //     NAPI_CALL_BASE(env, napi_get_element(env, propertys, i, &propKey), false);

    //     std::string propName;
    //     if (!ConvertFromJsValue(env, propKey, propName)) {
    //         TAG_LOGE(AAFwkTag::INTENT, "convert napi value failed");
    //         return false;
    //     }
    //     TAG_LOGE(AAFwkTag::INTENT, "testTag param %{public}s", propName.c_str());
    // }
    TAG_LOGE(AAFwkTag::INTENT, "testTag SetEntityObj end");
    return true;
}

napi_value callEntityQuery(napi_env env, const AAFwk::WantParams &param, std::unique_ptr<NativeReference>& entityObj)
{
    TAG_LOGE(AAFwkTag::INTENT, "testTag callEntityQuery start");

    napi_value obj = entityObj->GetNapiValue();
    napi_value argv[] = { AppExecFwk::WrapWantParams(env, param) };
    // return JsInsightIntentUtils::CallJsFunctionWithResult(env, obj, "entityQuery", ArraySize(argv), argv);

    // find method
    std::string methodName("entityQuery");
    auto method = GetTargetMethod1(env, obj, methodName);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get target method %{public}s failed", methodName.c_str());
        return nullptr;
    }

    // Call target method
    napi_value result;
    auto status = napi_call_function(env, obj, method, ArraySize(argv), argv, &result);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "call function failed %{public}d", status);
        return nullptr;
    }

    if (result == nullptr || !CheckTypeForNapiValue(env, result, napi_object)) {
        TAG_LOGE(AAFwkTag::INTENT, "queryType unimplemented");
        return nullptr;
    }
    TAG_LOGE(AAFwkTag::INTENT, "testTag get result success");

    bool isPromise = false;
    napi_is_promise(env, result, &isPromise);
    if (!isPromise) {
        TAG_LOGD(AAFwkTag::APPKIT, "result not promise");
        return nullptr;
    }
    TAG_LOGE(AAFwkTag::INTENT, "testTag isPromise success");

    // create callbackinfo
    auto callback = [env] (napi_value entityObj) {
        SetEntityObj(env, entityObj);
    };

    auto *callbackInfo = AppExecFwk::AbilityTransactionCallbackInfo<napi_value>::Create();
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null callbackInfo");
        return nullptr;
    }
    callbackInfo->Push(callback);

    bool callResult = false;
    do {
        napi_value then = nullptr;
        napi_get_named_property(env, result, "then", &then);
        if (then == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "null then");
            break;
        }
        bool isCallable = false;
        napi_is_callable(env, then, &isCallable);
        if (!isCallable) {
            TAG_LOGE(AAFwkTag::INTENT, "not callable property then");
            break;
        }
        napi_value promiseCallback = nullptr;
        napi_create_function(env, "promiseCallback", strlen("promiseCallback"),
            OnPromiseCallback, callbackInfo, &promiseCallback);
        napi_value argv[1] = { promiseCallback };
        auto status = napi_call_function(env, result, then, 1, argv, nullptr);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::INTENT, "call promise error %{public}d", status);
            break;
        }
        callResult = true;
    } while (false);

    if (!callResult) {
        TAG_LOGE(AAFwkTag::INTENT, "call promise error");
        return nullptr;
    }

    TAG_LOGE(AAFwkTag::INTENT, "testTag callEntityQuery end");
    return result;
}

}

bool JsInsightIntentFunc::Init(const InsightIntentExecutorInfo& insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "Init");
    STATE_PATTERN_NAIVE_ACCEPT(State::CREATED, false);
    state_ = State::INITIALIZED;
    InsightIntentExecutor::Init(insightIntentInfo);

    HandleScope handleScope(runtime_);
    auto ret = JsInsightIntentFunc::LoadJsCode(insightIntentInfo, runtime_);
    if (!ret) {
        TAG_LOGE(AAFwkTag::INTENT, "execute ohmurl failed");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    if (entityObj_ == nullptr) {
        entityObj_ = LoadJsEntityCode(insightIntentInfo, runtime_);
        if (entityObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "load js entity failed");
            STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
        }
    }

    return true;
}

bool JsInsightIntentFunc::HandleExecuteIntent(
    std::shared_ptr<InsightIntentExecuteParam> executeParam,
    const std::shared_ptr<NativeReference>& pageLoader,
    std::unique_ptr<InsightIntentExecutorAsyncCallback> callback,
    bool& isAsync)
{
    STATE_PATTERN_NAIVE_ACCEPT(State::INITIALIZED, false);
    state_ = State::EXECUTING;

    if (callback == nullptr || callback->IsEmpty()) {
        TAG_LOGE(AAFwkTag::INTENT, "null callback");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    callback_ = std::move(callback);
    bool successful = ExecuteInsightIntent(executeParam);
    isAsync = isAsync_;
    return successful;
}

bool JsInsightIntentFunc::LoadJsCode(const InsightIntentExecutorInfo& info, JsRuntime& runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "load module");
    auto executeParam = info.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return false;
    }

    std::string moduleName(executeParam->moduleName_);
    std::string hapPath(info.hapPath);
    std::string srcEntrance(executeParam->srcEntrance_);
    TAG_LOGD(AAFwkTag::INTENT, "moduleName %{public}s, hapPath %{private}s, srcEntrance %{private}s",
        moduleName.c_str(), hapPath.c_str(), srcEntrance.c_str());

    return runtime.ExecuteSecureWithOhmUrl(moduleName, hapPath, srcEntrance);
}

void JsInsightIntentFunc::ReplyFailedInner(InsightIntentInnerErr innerErr)
{
    TAG_LOGD(AAFwkTag::INTENT, "reply failed");
    state_ = State::INVALID;
    auto* callback = callback_.release();
    JsInsightIntentUtils::ReplyFailed(callback, innerErr);
}

void JsInsightIntentFunc::ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    TAG_LOGD(AAFwkTag::INTENT, "reply succeed");
    state_ = State::EXECUTATION_DONE;
    auto* callback = callback_.release();
    JsInsightIntentUtils::ReplySucceeded(callback, resultCpp);
}

// bool JsInsightIntentFunc::HandleResultReturnedFromJsFunc(napi_value resultJs)
// {
//     TAG_LOGD(AAFwkTag::INTENT, "handle result returned");
//     HandleScope handleScope(runtime_);
//     auto* env = runtime_.GetNapiEnv();
//     if (resultJs == nullptr) {
//         return ExecuteIntentCheckError();
//     }

//     bool isPromise = false;
//     NAPI_CALL_BASE(env, napi_is_promise(env, resultJs, &isPromise), ExecuteIntentCheckError());
//     isAsync_ = isPromise;

//     if (isPromise) {
//         TAG_LOGI(AAFwkTag::INTENT, "Is promise");
//         auto* callback = callback_.release();

//         napi_value then = nullptr;
//         NAPI_CALL_BASE(env, napi_get_named_property(env, resultJs, "then", &then), ExecuteIntentCheckError());
//         napi_value resolveCbJs = nullptr;
//         NAPI_CALL_BASE(env, napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
//             ResolveCbCpp, callback, &resolveCbJs), ExecuteIntentCheckError());
//         constexpr size_t argcThen = 1;
//         napi_value argvThen[argcThen] = { resolveCbJs };
//         NAPI_CALL_BASE(env, napi_call_function(env, resultJs, then, argcThen, argvThen, nullptr),
//             ExecuteIntentCheckError());

//         napi_value promiseCatch = nullptr;
//         NAPI_CALL_BASE(env, napi_get_named_property(env, resultJs, "catch", &promiseCatch), ExecuteIntentCheckError());
//         napi_value rejectCbJs = nullptr;
//         NAPI_CALL_BASE(env, napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
//             JsInsightIntentUtils::RejectCbCpp, callback, &rejectCbJs), ExecuteIntentCheckError());
//         constexpr size_t argcCatch = 1;
//         napi_value argvCatch[argcCatch] = { rejectCbJs };
//         NAPI_CALL_BASE(env, napi_call_function(env, resultJs, promiseCatch, argcCatch, argvCatch, nullptr),
//             ExecuteIntentCheckError());
//     } else {
//         TAG_LOGI(AAFwkTag::INTENT, "Not promise");
//         auto resultCpp = GetResultFromJs(env, resultJs);
//         if (resultCpp == nullptr) {
//             TAG_LOGE(AAFwkTag::INTENT, "null resultCpp");
//             return ExecuteIntentCheckError();
//         }
//         TAG_LOGD(AAFwkTag::INTENT, "Call succeed");
//         ReplySucceededInner(resultCpp);
//     }
//     return true;
// }

std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> JsInsightIntentFunc::GetResultFromJs(
    napi_env env, napi_value resultJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "Get result for intent func");
    auto resultCpp = std::make_shared<AppExecFwk::InsightIntentExecuteResult>();
    auto resultStr = JsInsightIntentUtils::StringifyObject(env, resultJs);
    auto wantParams = std::make_shared<AAFwk::WantParams>();
    wantParams->SetParam("methodResult", AAFwk::String::Box(resultStr));
    resultCpp->result = wantParams;
    resultCpp->code = InsightIntentInnerErr::INSIGHT_INTENT_ERR_OK;
    return resultCpp;
}

napi_value JsInsightIntentFunc::ResolveCbCpp(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::INTENT, "Resolve function");
    constexpr size_t argc = 1;
    napi_value argv[argc] = {nullptr};
    size_t actualArgc = argc;
    void* data = nullptr;
    NAPI_CALL_BASE(env, napi_get_cb_info(env, info, &actualArgc, argv, nullptr, &data), nullptr);

    auto* callback = static_cast<InsightIntentExecutorAsyncCallback*>(data);
    napi_value resultJs = argv[0];
    if (resultJs == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "callback invalid");
        JsInsightIntentUtils::ReplyFailed(callback);
        return nullptr;
    }

    auto resultCpp = GetResultFromJs(env, resultJs);
    JsInsightIntentUtils::ReplySucceeded(callback, resultCpp);
    return nullptr;
}

bool JsInsightIntentFunc::ExecuteIntentCheckError()
{
    ReplyFailedInner();
    STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
}

bool JsInsightIntentFunc::HandleResultReturnedFromJsFunc(napi_value resultJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "handle result returned");
    HandleScope handleScope(runtime_);
    auto* env = runtime_.GetNapiEnv();
    if (resultJs == nullptr) {
        return ExecuteIntentCheckError();
    }

    bool isPromise = false;
    NAPI_CALL_BASE(env, napi_is_promise(env, resultJs, &isPromise), ExecuteIntentCheckError());
    isAsync_ = isPromise;

    if (!isPromise) {
        // onExecute only support promise
        TAG_LOGW(AAFwkTag::INTENT, "no promise, don't support for now");
        return ExecuteIntentCheckError();
    }

    TAG_LOGI(AAFwkTag::INTENT, "Is promise");
    auto* callback = callback_.release();

    napi_value then = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, resultJs, "then", &then), ExecuteIntentCheckError());
    napi_value resolveCbJs = nullptr;
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null context");
        return ExecuteIntentCheckError();
    }
    if (context->GetDelayReturnMode() == InsightIntentReturnMode::CALLBACK) {
        NAPI_CALL_BASE(env, napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
            JsInsightIntentUtils::ResolveCbCpp, callback, &resolveCbJs), ExecuteIntentCheckError());
        // InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(context->GetIntentId());
    } else {
        NAPI_CALL_BASE(env, napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
            JsInsightIntentUtils::ResolveExecuteResultWithDelay, callback, &resolveCbJs), ExecuteIntentCheckError());
    }
    constexpr size_t argcThen = 1;
    napi_value argvThen[argcThen] = { resolveCbJs };
    NAPI_CALL_BASE(env, napi_call_function(env, resultJs, then, argcThen, argvThen, nullptr),
        ExecuteIntentCheckError());

    napi_value promiseCatch = nullptr;
    NAPI_CALL_BASE(env, napi_get_named_property(env, resultJs, "catch", &promiseCatch), ExecuteIntentCheckError());
    napi_value rejectCbJs = nullptr;
    NAPI_CALL_BASE(env, napi_create_function(env, TMP_NAPI_ANONYMOUS_FUNC, strlen(TMP_NAPI_ANONYMOUS_FUNC),
        JsInsightIntentUtils::RejectCbCpp, callback, &rejectCbJs), ExecuteIntentCheckError());
    constexpr size_t argcCatch = 1;
    napi_value argvCatch[argcCatch] = { rejectCbJs };
    NAPI_CALL_BASE(env, napi_call_function(env, resultJs, promiseCatch, argcCatch, argvCatch, nullptr),
        ExecuteIntentCheckError());

    return true;
}

bool JsInsightIntentFunc::ExecuteInsightIntent(std::shared_ptr<InsightIntentExecuteParam> executeParam)
{
    if (executeParam == nullptr || executeParam->insightIntentParam_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return ExecuteIntentCheckError();
    }

    auto name = executeParam->insightIntentName_;
    TAG_LOGD(AAFwkTag::INTENT, "execute intent %{public}s", name.c_str());

    HandleScope handleScope(runtime_);
    auto env = runtime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return ExecuteIntentCheckError();
    }

    std::string srcEntrance = executeParam->srcEntrance_;
    std::string className = executeParam->className_;
    std::string methodName = executeParam->methodName_;
    std::vector<std::string> methodParams = executeParam->methodParams_;
    std::unordered_map<std::string, size_t> paramMap;
    for (size_t i = 0; i < methodParams.size(); i++) {
        paramMap[methodParams[i]] = i;
    }

    size_t argc = 0;
    std::vector<napi_value> argv(1);
    auto param = executeParam->insightIntentParam_;
    auto ret = ParseParams(env, *param, paramMap, argc, argv);
    if (!ret) {
        TAG_LOGE(AAFwkTag::INTENT, "parse params failed");
        return ExecuteIntentCheckError();
    }

    auto objValue = callEntityQuery(env, *param, entityObj_);
    if (objValue == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "testTag callEntityQuery is nullptr");
    } else {
        TAG_LOGE(AAFwkTag::INTENT, "testTag callEntityQuery success");
    }

    // napi_value constructor = runtime_.GetExportObjectFromOhmUrl(srcEntrance, className);
    // if (constructor == nullptr) {
    //     TAG_LOGE(AAFwkTag::INTENT, "get class %{public}s failed", className.c_str());
    //     return ExecuteIntentCheckError();
    // }

    // auto method = GetTargetMethod(env, constructor, methodName);
    // if (method == nullptr) {
    //     TAG_LOGE(AAFwkTag::INTENT, "get target method %{public}s failed", methodName.c_str());
    //     return ExecuteIntentCheckError();
    // }

    // // Call target method
    // napi_value result;
    // auto status = napi_call_function(env, constructor, method, argc, argv.data(), &result);
    // if (status != napi_ok) {
    //     TAG_LOGE(AAFwkTag::INTENT, "call function failed %{public}d", status);
    //     return ExecuteIntentCheckError();
    // }

    return HandleResultReturnedFromJsFunc(objValue);
    // return HandleResultReturnedFromJsFunc(result);
}

bool JsInsightIntentFunc::ParseParams(napi_env env, const AAFwk::WantParams& param,
    const std::unordered_map<std::string, size_t> &paramMap, size_t &argc, std::vector<napi_value> &argv)
{
    AAFwk::WantParamWrapper wrapper(param);
    std::string parametersString = wrapper.ToString();
    TAG_LOGD(AAFwkTag::INTENT, "param string %{private}s", parametersString.c_str());

    napi_value srcObj = AppExecFwk::WrapWantParams(env, param);
    if (srcObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "wrap params failed");
        return false;
    }

    napi_value propertys;
    NAPI_CALL_BASE(env, napi_get_property_names(env, srcObj, &propertys), false);

    uint32_t propCnt;
    NAPI_CALL_BASE(env, napi_get_array_length(env, propertys, &propCnt), false);

    argc = paramMap.size();
    argv.resize(argc);

    for (uint32_t i = 0; i < propCnt; i++) {
        napi_value propKey;
        NAPI_CALL_BASE(env, napi_get_element(env, propertys, i, &propKey), false);

        std::string propName;
        if (!ConvertFromJsValue(env, propKey, propName)) {
            TAG_LOGE(AAFwkTag::DELEGATOR, "convert napi value failed");
            return false;
        }
        TAG_LOGD(AAFwkTag::INTENT, "param %{public}s", propName.c_str());

        napi_value propValue;
        NAPI_CALL_BASE(env, napi_get_property(env, srcObj, propKey, &propValue), false);

        auto it = paramMap.find(propName);
        if (it != paramMap.end()) {
            auto iter = it->second;
            TAG_LOGD(AAFwkTag::INTENT, "param %{public}s matched, id %{public}zu", propName.c_str(), iter);
            argv[iter] = propValue;
        }
    }

    return true;
}

napi_value JsInsightIntentFunc::GetTargetMethod(napi_env env, napi_value constructor, const std::string &methodName)
{
    TAG_LOGD(AAFwkTag::INTENT, "methodName %{private}s", methodName.c_str());
    napi_value method;
    auto status = napi_get_named_property(env, constructor, methodName.c_str(), &method);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "get method failed %{public}d", status);
        return nullptr;
    }

    // Check validity of method
    napi_value undefined;
    status = napi_get_undefined(env, &undefined);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "get undefined failed %{public}d", status);
        return nullptr;
    }

    bool isUndefined;
    status = napi_strict_equals(env, method, undefined, &isUndefined);
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::INTENT, "target method %{private}s didn't exist", methodName.c_str());
        return nullptr;
    }

    return method;
}
} // namespace AbilityRuntime
} // namespace OHOS
