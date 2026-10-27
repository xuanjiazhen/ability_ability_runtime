/*
 * Copyright (c) 2023-2026 Huawei Device Co., Ltd.
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

#include "js_auto_fill_extension_context.h"

#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "js_auto_fill_extension_util.h"
#include "js_error_utils.h"
#include "js_extension_context.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_want.h"
#include "napi_common_util.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;

static std::mutex g_connectsMutex;
static std::map<UIExtensionConnectionKey, sptr<JSAutoFillExtensionConnection>, key_compare> g_connects;
static int64_t g_serialNumber = 0;

void RemoveConnection(int64_t connectId)
{
    std::lock_guard guard(g_connectsMutex);
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
        [connectId](const auto &obj) { return connectId == obj.first.id; });
    if (item != g_connects.end()) {
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    }
}

void FindConnection(AAFwk::Want &want, sptr<JSAutoFillExtensionConnection> &connection, int64_t &connectId)
{
    std::lock_guard guard(g_connectsMutex);
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
        [connectId](const auto &obj) { return connectId == obj.first.id; });
    if (item != g_connects.end()) {
        want = item->first.want;
        connection = item->second;
    }
}

bool CheckConnectionParam(napi_env env, napi_value value,
    sptr<JSAutoFillExtensionConnection> &connection, AAFwk::Want &want)
{
    if (!CheckTypeForNapiValue(env, value, napi_object)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "get object failed");
        return false;
    }
    connection->SetJsConnectionObject(value);
    std::lock_guard guard(g_connectsMutex);
    UIExtensionConnectionKey key;
    key.id = g_serialNumber;
    key.want = want;
    connection->SetConnectionId(key.id);
    g_connects.emplace(key, connection);
    g_serialNumber = (g_serialNumber + 1) % INT32_MAX;
    return true;
}

} // namespace

void JsAutoFillExtensionContext::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    std::unique_ptr<JsAutoFillExtensionContext>(static_cast<JsAutoFillExtensionContext*>(data));
}

napi_value JsAutoFillExtensionContext::ReloadInModal(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAutoFillExtensionContext, OnReloadInModal);
}

napi_value JsAutoFillExtensionContext::OnReloadInModal(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "invalid argc");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    HandleEscape handleEscape(env);
    napi_value jsCustomData = GetPropertyValueByPropertyName(env, info.argv[INDEX_ZERO], "data", napi_object);
    CustomData customData;
    if (jsCustomData == nullptr || !AppExecFwk::UnwrapWantParams(env, jsCustomData, customData.data)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Parse custom data failed");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return CreateJsUndefined(env);
    }

    auto retVal = std::make_shared<int32_t>(0);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, customData, ret = retVal, env]() {
        auto context = weak.lock();
        if (context == nullptr) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
            ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        if (ret == nullptr) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null ret");
            return;
        }
        *ret = context->ReloadInModal(customData);
    };

    NapiAsyncTask::CompleteCallback complete = [ret = retVal](napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (ret == nullptr) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null ret");
            task.Reject(env, CreateJsError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
                GetInnerErrorMsg(AbilityInnerErrorMsg::RELOAD_IN_MODAL_RESULT_NULL)));
            return;
        }
        if (*ret != ERR_OK) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "error %{public}d", *ret);
            task.Reject(env, CreateJsError(env, GetJsErrorCodeByNativeError(*ret)));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAutoFillExtensionContext::OnReloadInModal",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JsAutoFillExtensionContext::ConnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAutoFillExtensionContext, OnConnectAbility);
}

napi_value JsAutoFillExtensionContext::DisconnectAbility(napi_env env, napi_callback_info info)
{
    GET_NAPI_INFO_AND_CALL(env, info, JsAutoFillExtensionContext, OnDisconnectAbility);
}

napi_value JsAutoFillExtensionContext::OnConnectAbility(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(IPCSkeleton::GetSelfTokenID())) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    if (info.argc < ARGC_TWO) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    auto connection = sptr<JSAutoFillExtensionConnection>::MakeSptr(env);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null connection");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, info.argv[INDEX_ZERO], want) ||
        !CheckConnectionParam(env, info.argv[INDEX_ONE], connection, want)) {
        ThrowInvalidParamError(env,
            "Parse param want or connection failed, want must be Want and connection must be Connection.");
        return CreateJsUndefined(env);
    }
    int64_t connectId = connection->GetConnectionId();
    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, connection, connectId, innerErrCode]() {
        auto context = weak.lock();
        if (!context) {
            TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
            *innerErrCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "ConnectAbility connection:%{public}d", static_cast<int32_t>(connectId));
        *innerErrCode = context->ConnectAbility(want, connection);
    };
    NapiAsyncTask::CompleteCallback complete =
        [connection, connectId, innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            int32_t errcode = (*innerErrCode == static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)) ?
                *innerErrCode : static_cast<int32_t>(GetJsErrorCodeByNativeError(*innerErrCode));
            if (errcode != ERR_OK) {
                connection->CallJsFailed(errcode);
                RemoveConnection(connectId);
            }
            task.Resolve(env, CreateJsUndefined(env));
        };
    napi_value result = nullptr;
    NapiAsyncTask::ScheduleHighQos("JsAutoFillExtensionContext::OnConnectAbility",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return CreateJsValue(env, connectId);
}

napi_value JsAutoFillExtensionContext::OnDisconnectAbility(napi_env env, NapiCallbackInfo &info)
{
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(IPCSkeleton::GetSelfTokenID())) {
        ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return CreateJsUndefined(env);
    }
    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "invalid argc");
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    int64_t connectId = -1;
    if (!AppExecFwk::UnwrapInt64FromJS2(env, info.argv[INDEX_ZERO], connectId)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Invalid connectId");
        ThrowInvalidParamError(env, "Parse param connectId failed, connectId must be number.");
        return CreateJsUndefined(env);
    }
    AAFwk::Want want;
    sptr<JSAutoFillExtensionConnection> connection = nullptr;
    FindConnection(want, connection, connectId);
    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [weak = context_, want, connection, innerErrCode]() {
        auto context = weak.lock();
        if (!context) {
            TAG_LOGW(AAFwkTag::AUTOFILL_EXT, "null context");
            *innerErrCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
            return;
        }
        if (!connection) {
            TAG_LOGW(AAFwkTag::AUTOFILL_EXT, "null connection");
            *innerErrCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
            return;
        }
        TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "context->DisconnectAbility");
        *innerErrCode = context->DisconnectAbility(want, connection);
    };
    NapiAsyncTask::CompleteCallback complete =
        [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            if (*innerErrCode == ERR_OK) {
                task.Resolve(env, CreateJsUndefined(env));
            } else if (*innerErrCode == static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT)) {
                task.Reject(env, CreateJsError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
            } else if (*innerErrCode == static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER)) {
                task.Reject(env, CreateJsError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
                    GetInnerErrorMsg(AbilityInnerErrorMsg::CONNECTION_NOT_FOUND)));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JsAutoFillExtensionContext::OnDisconnectAbility",
        env, CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return result;
}

napi_value JsAutoFillExtensionContext::CreateJsAutoFillExtensionContext(
    napi_env env, const std::shared_ptr<AutoFillExtensionContext> &context)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "called");
    std::shared_ptr<OHOS::AppExecFwk::AbilityInfo> abilityInfo = nullptr;
    if (context != nullptr) {
        abilityInfo = context->GetAbilityInfo();
    }
    HandleEscape handleEscape(env);
    napi_value objValue = CreateJsExtensionContext(env, context, abilityInfo);

    auto jsContext = std::make_unique<JsAutoFillExtensionContext>(context);
    napi_wrap(env, objValue, jsContext.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsAutoFillExtensionContext";
    BindNativeFunction(env, objValue, "reloadInModal", moduleName, ReloadInModal);
    BindNativeFunction(env, objValue, "connectServiceExtensionAbility", moduleName, ConnectAbility);
    BindNativeFunction(env, objValue, "disconnectServiceExtensionAbility", moduleName, DisconnectAbility);

    std::string type = "AutoFillExtensionContext";
    napi_set_named_property(env, objValue, "contextType", CreateJsValue(env, type));

    return handleEscape.Escape(objValue);
}

void JSAutoFillExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "resultCode:%{public}d", resultCode);
    HandleScope handleScope(env_);
    napi_value napiElementName = OHOS::AppExecFwk::WrapElementName(env_, element);
    napi_value argv[] = {napiElementName};
    if (jsConnectionObject_ == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null jsConnectionObject_");
        RemoveConnection(connectionId_);
        return;
    }
    napi_value obj = jsConnectionObject_->GetNapiValue();
    if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "get object failed");
        RemoveConnection(connectionId_);
        return;
    }
    napi_value method = nullptr;
    napi_get_named_property(env_, obj, "onDisconnect", &method);
    RemoveConnection(connectionId_);
    if (!CheckTypeForNapiValue(env_, method, napi_function)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "invalid onDisconnect");
        return;
    }

    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "Call onDisconnect");
    napi_status status = napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "call js func failed %{public}d", status);
    }
}

} // namespace AbilityRuntime
} // namespace OHOS
