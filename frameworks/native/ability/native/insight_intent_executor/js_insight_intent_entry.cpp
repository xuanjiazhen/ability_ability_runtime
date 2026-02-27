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

#include "js_insight_intent_entry.h"

#include <want_params.h>

#include "hilog_tag_wrapper.h"
#include "insight_intent_constant.h"
#include "insight_intent_delay_result_callback_mgr.h"
#include "insight_intent_execute_result.h"
#include "js_insight_intent_context.h"
#include "js_insight_intent_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi_common_execute_result.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "native_reference.h"

#undef STATE_PATTERN_NAIVE_H
#define STATE_PATTERN_NAIVE_STATE state_
#include "state_pattern_naive.h"

#define TMP_NAPI_ANONYMOUS_FUNC "_"

namespace OHOS {
namespace AbilityRuntime {
std::shared_ptr<JsInsightIntentEntry> JsInsightIntentEntry::Create(JsRuntime& runtime)
{
    return std::make_shared<JsInsightIntentEntry>(runtime);
}

JsInsightIntentEntry::JsInsightIntentEntry(JsRuntime& runtime) : runtime_(runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "constructor");
}

JsInsightIntentEntry::~JsInsightIntentEntry()
{
    state_ = State::DESTROYED;
    TAG_LOGI(AAFwkTag::INTENT, "destructor");
}

bool JsInsightIntentEntry::Init(const InsightIntentExecutorInfo& insightIntentInfo)
{
    TAG_LOGD(AAFwkTag::INTENT, "Init");
    STATE_PATTERN_NAIVE_ACCEPT(State::CREATED, false);
    state_ = State::INITIALIZED;
    InsightIntentExecutor::Init(insightIntentInfo);

    HandleScope handleScope(runtime_);
    if (jsObj_ == nullptr) {
        jsObj_ = JsInsightIntentEntry::LoadJsCode(insightIntentInfo, runtime_);
        if (jsObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "load js failed");
            STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
        }
    }

    if (entityObj_ == nullptr) {
        entityObj_ = JsInsightIntentEntry::LoadJsEntityCode(insightIntentInfo, runtime_);
        if (entityObj_ == nullptr) {
            TAG_LOGE(AAFwkTag::INTENT, "load js entity failed");
            STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
        }
    }

    auto env = runtime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null Context");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    napi_value contextObj = CreateJsInsightIntentContext(env, context);
    contextObj_ = JsRuntime::LoadSystemModuleByEngine(env, "app.ability.InsightIntentContext", &contextObj, 1);
    if (contextObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null contextObj_");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    auto executorNapiVal = jsObj_->GetNapiValue();
    auto contextNapiVal = contextObj_->GetNapiValue();
    napi_value jsInstanceId = nullptr;
    napi_create_int64(env, context->GetIntentId(), &jsInstanceId);
    napi_set_named_property(env, contextNapiVal, "instanceId", jsInstanceId);
    if (!CheckTypeForNapiValue(env, executorNapiVal, napi_object) ||
        !CheckTypeForNapiValue(env, contextNapiVal, napi_object) ||
        napi_set_named_property(env, executorNapiVal, "context", contextNapiVal) != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set context property failed");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    context->SetExecuteMode(insightIntentInfo.executeParam->executeMode_);
    return true;
}

bool JsInsightIntentEntry::HandleExecuteIntent(
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

    if (executeParam == nullptr || executeParam->insightIntentParam_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid execute param");
        STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
    }

    callback_ = std::move(callback);
    auto name = executeParam->insightIntentName_;
    TAG_LOGD(AAFwkTag::INTENT, "Execute Intent %{public}s", name.c_str());

    // Check parameter
    InsightIntentExecuteMode mode = static_cast<InsightIntentExecuteMode>(executeParam->executeMode_);
    if (!PrepareParameters(mode, pageLoader)) {
        return ExecuteIntentCheckError();
    }

    bool successful = ExecuteInsightIntent(name, *executeParam->insightIntentParam_, pageLoader);
    isAsync = isAsync_;
    return successful;
}

std::unique_ptr<NativeReference> JsInsightIntentEntry::LoadJsCode(
    const InsightIntentExecutorInfo& info,
    JsRuntime& runtime)
{
    TAG_LOGD(AAFwkTag::INTENT, "load module");
    auto executeParam = info.executeParam;
    if (executeParam == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null executeParam");
        return std::unique_ptr<NativeReference>();
    }

    HandleScope handleScope(runtime);
    std::string moduleName(executeParam->moduleName_);
    std::string hapPath(info.hapPath);
    std::string srcEntrance(executeParam->srcEntrance_);
    TAG_LOGE(AAFwkTag::INTENT, "moduleName %{public}s, hapPath %{public}s, srcEntrance %{public}s",
        moduleName.c_str(), hapPath.c_str(), srcEntrance.c_str());

    auto ret = runtime.ExecuteSecureWithOhmUrl(moduleName, hapPath, srcEntrance);
    if (!ret) {
        TAG_LOGE(AAFwkTag::INTENT, "execute failed");
        return std::unique_ptr<NativeReference>();
    }

    auto exportObj = runtime.GetExportObjectFromOhmUrl(srcEntrance, "default");
    if (exportObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "get default object failed");
        return std::unique_ptr<NativeReference>();
    }

    napi_value instanceValue = nullptr;
    auto* env = runtime.GetNapiEnv();
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

napi_value JsInsightIntentEntry::CallJsFunctionWithResultInner(
    const char* funcName,
    size_t argc,
    const napi_value* argv)
{
    TAG_LOGD(AAFwkTag::INTENT, "call js function");
    HandleEscape handleEscape(runtime_);
    auto* env = runtime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_valuetype::napi_object)) {
        TAG_LOGE(AAFwkTag::INTENT, "call js function type error");
        return nullptr;
    }
    return handleEscape.Escape(JsInsightIntentUtils::CallJsFunctionWithResult(env, obj, funcName, argc, argv));
}

void JsInsightIntentEntry::ReplyFailedInner(InsightIntentInnerErr innerErr)
{
    TAG_LOGD(AAFwkTag::INTENT, "reply failed");
    state_ = State::INVALID;
    auto* callback = callback_.release();
    JsInsightIntentUtils::ReplyFailed(callback, innerErr);
}

void JsInsightIntentEntry::ReplySucceededInner(std::shared_ptr<AppExecFwk::InsightIntentExecuteResult> resultCpp)
{
    TAG_LOGD(AAFwkTag::INTENT, "reply succeed");
    state_ = State::EXECUTATION_DONE;
    auto* callback = callback_.release();
    JsInsightIntentUtils::ReplySucceeded(callback, resultCpp);
}

bool JsInsightIntentEntry::HandleResultReturnedFromJsFunc(napi_value resultJs)
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
        InsightIntentDelayResultCallbackMgr::GetInstance().RemoveDelayResultCallback(context->GetIntentId());
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

bool JsInsightIntentEntry::ExecuteIntentCheckError()
{
    ReplyFailedInner();
    STATE_PATTERN_NAIVE_STATE_SET_AND_RETURN(State::INVALID, false);
}

bool JsInsightIntentEntry::ExecuteInsightIntent(
    const std::string& name,
    const AAFwk::WantParams& param,
    const std::shared_ptr<NativeReference>& windowStageJs)
{
    TAG_LOGD(AAFwkTag::INTENT, "execute insight intent %{public}s", name.c_str());
    HandleScope handleScope(runtime_);
    auto env = runtime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return ExecuteIntentCheckError();
    }

    auto objValue = callEntityQuery(env, param, entityObj_);
    if (objValue == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "testTag callEntityQuery is nullptr");
    } else {
        TAG_LOGE(AAFwkTag::INTENT, "testTag callEntityQuery success");
    }

    if (!AssignObject(env, param)) {
        TAG_LOGE(AAFwkTag::INTENT, "assign object failed");
        return ExecuteIntentCheckError();
    }

    napi_value result = nullptr;
    result = CallJsFunctionWithResultInner("onExecute", 0, nullptr);
    if (result == nullptr) {
        // error log has printed
        return ExecuteIntentCheckError();
    }

    return HandleResultReturnedFromJsFunc(objValue);
    // return HandleResultReturnedFromJsFunc(result);
}

bool JsInsightIntentEntry::PrepareParameters(InsightIntentExecuteMode mode,
    const std::shared_ptr<NativeReference>& pageLoader)
{
    auto env = runtime_.GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "null env");
        return false;
    }

    switch (mode) {
        case InsightIntentExecuteMode::UIABILITY_FOREGROUND:
            return PrepareParametersUIAbilityForeground(env, pageLoader);
        case InsightIntentExecuteMode::UIABILITY_BACKGROUND:
            return PrepareParametersUIAbilityBackground(env);
        case InsightIntentExecuteMode::UIEXTENSION_ABILITY:
            return PrepareParametersUIExtension(env, pageLoader);
        case InsightIntentExecuteMode::SERVICE_EXTENSION_ABILITY:
            return PrepareParametersServiceExtension(env);
        default:
            TAG_LOGE(AAFwkTag::INTENT, "InsightIntentExecuteMode not supported yet");
            return false;
    }

    return true;
}

bool JsInsightIntentEntry::PrepareParametersUIAbilityForeground(napi_env env,
    const std::shared_ptr<NativeReference>& pageLoader)
{
    // check pageloader and jsObj_
    if (pageLoader == nullptr || jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "page loader or object invalid");
        return false;
    }

    // assign ability pageloader
    HandleScope handleScope(env);
    auto executorNapiVal = jsObj_->GetNapiValue();
    auto pageLoaderNapiVal = pageLoader->GetNapiValue();
    if (!CheckTypeForNapiValue(env, executorNapiVal, napi_object) ||
        !CheckTypeForNapiValue(env, pageLoaderNapiVal, napi_object) ||
        napi_set_named_property(env, executorNapiVal, "windowStage", pageLoaderNapiVal) != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set windowStage property failed");
        return false;
    }

    // assign execute mode
    auto mode = InsightIntentExecuteMode::UIABILITY_FOREGROUND;
    auto modeNapiVal = AppExecFwk::WrapInt32ToJS(env, static_cast<int32_t>(mode));
    auto status = napi_set_named_property(env, executorNapiVal, "executeMode", modeNapiVal);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set executeMode property failed with %{public}d", status);
        return false;
    }

    return true;
}

bool JsInsightIntentEntry::PrepareParametersUIAbilityBackground(napi_env env)
{
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "js object null");
        return false;
    }
    AbilityRuntime::HandleScope handleScope(env);

    auto executorNapiVal = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, executorNapiVal, napi_object)) {
        TAG_LOGE(AAFwkTag::INTENT, "js object invalid");
        return false;
    }

    auto mode = InsightIntentExecuteMode::UIABILITY_BACKGROUND;
    auto modeNapiVal = AppExecFwk::WrapInt32ToJS(env, static_cast<int32_t>(mode));
    auto status = napi_set_named_property(env, executorNapiVal, "executeMode", modeNapiVal);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set executeMode property failed with %{public}d", status);
        return false;
    }

    return true;
}

bool JsInsightIntentEntry::PrepareParametersUIExtension(napi_env env,
    const std::shared_ptr<NativeReference>& pageLoader)
{
    // check pageloader and jsObj_
    if (pageLoader == nullptr || jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "page loader or object invalid");
        return false;
    }
    AbilityRuntime::HandleScope handleScope(env);

    // assign ability pageloader
    auto executorNapiVal = jsObj_->GetNapiValue();
    auto pageLoaderNapiVal = pageLoader->GetNapiValue();
    if (!CheckTypeForNapiValue(env, executorNapiVal, napi_object) ||
        !CheckTypeForNapiValue(env, pageLoaderNapiVal, napi_object) ||
        napi_set_named_property(env, executorNapiVal, "uiExtensionSession", pageLoaderNapiVal) != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set uiExtensionSession property failed");
        return false;
    }

    // assign execute mode
    auto mode = InsightIntentExecuteMode::UIEXTENSION_ABILITY;
    auto modeNapiVal = AppExecFwk::WrapInt32ToJS(env, static_cast<int32_t>(mode));
    auto status = napi_set_named_property(env, executorNapiVal, "executeMode", modeNapiVal);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set executeMode property failed with %{public}d", status);
        return false;
    }

    return true;
}

bool JsInsightIntentEntry::PrepareParametersServiceExtension(napi_env env)
{
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "js object null");
        return false;
    }
    AbilityRuntime::HandleScope handleScope(env);

    auto executorNapiVal = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, executorNapiVal, napi_object)) {
        TAG_LOGE(AAFwkTag::INTENT, "js object invalid");
        return false;
    }

    auto mode = InsightIntentExecuteMode::SERVICE_EXTENSION_ABILITY;
    auto modeNapiVal = AppExecFwk::WrapInt32ToJS(env, static_cast<int32_t>(mode));
    auto status = napi_set_named_property(env, executorNapiVal, "executeMode", modeNapiVal);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set executeMode property failed with %{public}d", status);
        return false;
    }

    return true;
}

bool JsInsightIntentEntry::AssignObject(napi_env env, const AAFwk::WantParams &wantParams)
{
    AAFwk::WantParamWrapper wrapper(wantParams);
    std::string parametersString = wrapper.ToString();
    TAG_LOGD(AAFwkTag::INTENT, "param string %{public}s", parametersString.c_str());

    auto executorNapiVal = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, executorNapiVal, napi_object)) {
        TAG_LOGE(AAFwkTag::INTENT, "check type failed");
        return false;
    }

    napi_value srcObj = AppExecFwk::WrapWantParams(env, wantParams);

    // Get all property
    napi_value propertys;
    NAPI_CALL_BASE(env, napi_get_property_names(env, srcObj, &propertys), false);

    uint32_t propCnt;
    NAPI_CALL_BASE(env, napi_get_array_length(env, propertys, &propCnt), false);

    for (uint32_t i = 0; i < propCnt; i++) {
        napi_value propKey;
        NAPI_CALL_BASE(env, napi_get_element(env, propertys, i, &propKey), false);

        std::string propName;
        if (!ConvertFromJsValue(env, propKey, propName)) {
            TAG_LOGE(AAFwkTag::INTENT, "convert napi value failed");
            return false;
        }
        TAG_LOGD(AAFwkTag::INTENT, "param %{public}s", propName.c_str());

        napi_value propValue;
        NAPI_CALL_BASE(env, napi_get_property(env, srcObj, propKey, &propValue), false);
        NAPI_CALL_BASE(env, napi_set_property(env, executorNapiVal, propKey, propValue), false);
    }

    return true;
}

namespace
{
    
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

}

napi_value JsInsightIntentEntry::GetTargetMethod(napi_env env, napi_value constructor, const std::string &methodName)
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
        TAG_LOGE(AAFwkTag::INTENT, "target method %{private}s didn't exist", methodName.c_str());
        return nullptr;
    }

    return method;
}

std::unique_ptr<NativeReference> JsInsightIntentEntry::LoadJsEntityCode(
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
    TAG_LOGE(AAFwkTag::INTENT, "testTag moduleName %{public}s, hapPath %{public}s, srcEntrance %{public}s",
        moduleName.c_str(), hapPath.c_str(), srcEntrance.c_str());

    auto ret = runtime.ExecuteSecureWithOhmUrl(moduleName, hapPath, srcEntrance);
    if (!ret) {
        TAG_LOGE(AAFwkTag::INTENT, "execute failed");
        return std::unique_ptr<NativeReference>();
    }

    TAG_LOGE(AAFwkTag::INTENT, "insightIntentParam: %{public}s", executeParam->insightIntentParam_->ToString().c_str());
    std::string className = executeParam->insightIntentParam_->GetStringParam("className");
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

napi_value JsInsightIntentEntry::callEntityQuery(napi_env env, const AAFwk::WantParams &param, std::unique_ptr<NativeReference>& entityObj)
{
    TAG_LOGE(AAFwkTag::INTENT, "testTag callEntityQuery start");

    napi_value obj = entityObj->GetNapiValue();
    napi_value argv[] = { AppExecFwk::WrapWantParams(env, param) };
    // return JsInsightIntentUtils::CallJsFunctionWithResult(env, obj, "entityQuery", ArraySize(argv), argv);

    // find method
    std::string methodName("entityQuery");
    auto method = GetTargetMethod(env, obj, methodName);
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
    auto callback = [this, env] (napi_value entityObj) {
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

bool JsInsightIntentEntry::SetEntityObj(napi_env env, napi_value entityObj)
{
    TAG_LOGE(AAFwkTag::INTENT, "testTag SetEntityObj start");

    UnwrapExecuteParam(env, entityObj);

    auto executorObj = jsObj_->GetNapiValue();
    
    // 设置entity到executorObj的targetStringEntities字段上
    auto status = napi_set_named_property(env, executorObj, "targetStringEntities", entityObj);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::INTENT, "set targetStringEntities property failed with %{public}d", status);
        return false;
    }

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

} // namespace AbilityRuntime
} // namespace OHOS
