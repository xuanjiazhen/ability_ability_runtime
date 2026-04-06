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

#include "ets_form_extension_context.h"

#include <algorithm>
#include <cstdio>
#include <iterator>
#include <sys/syscall.h>
#include <unistd.h>

#include "ani_common_configuration.h"
#include "ani_common_want.h"
#include "ani_common_util.h"
#include "context_transfer.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_extension_context.h"
#include "form_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "interop_js/arkts_esvalue.h"
#include "interop_js/arkts_interop_js_api.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include "js_runtime_utils.h"
#include "js_form_extension_context.h"
#include "native_engine/native_engine.h"
#include "remote_object_taihe_ani.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr ani_long ERROR_LONG_VALUE = -1;
constexpr const char *FORM_EXTENSION_CONTEXT_CLASS_NAME = "application.FormExtensionContext.FormExtensionContext";
constexpr const char *CLEANER_CLASS_NAME = "application.FormExtensionContext.Cleaner";
constexpr const char *SIGNATURE_START_ABILITY_FORM_EXTENSION =
    "C{@ohos.app.ability.Want.Want}C{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_CONNECT_FORM_EXTENSION =
    "C{@ohos.app.ability.Want.Want}C{ability.connectOptions.ConnectOptions}:l";
constexpr const char *SIGNATURE_DISCONNECT_FORM_EXTENSION = "lC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const char *SIGNATURE_CHECK_WANT = "C{@ohos.app.ability.Want.Want}:";
constexpr const char *SIGNATURE_CHECK_CONNECTION = ":";
constexpr int32_t ARGC_ONE = 1;
constexpr int32_t ARGC_TWO = 2;

std::map<ConnectionKey, sptr<ETSFormExtensionConnection>, key_compare> g_connects;
std::mutex g_connectsMutex_;
int64_t g_serialNumber = 0;

bool BindNativeMethods(ani_env *env, ani_class &cls)
{
    ani_status status = ANI_ERROR;
    std::array functions = {
        ani_native_function { "nativeStartAbility", SIGNATURE_START_ABILITY_FORM_EXTENSION,
            reinterpret_cast<void *>(ETSFormExtensionContext::StartAbility) },
        ani_native_function { "nativeConnectServiceExtensionAbility", SIGNATURE_CONNECT_FORM_EXTENSION,
            reinterpret_cast<void *>(ETSFormExtensionContext::ConnectAbility) },
        ani_native_function { "nativeDisconnectServiceExtensionAbility", SIGNATURE_DISCONNECT_FORM_EXTENSION,
            reinterpret_cast<void *>(ETSFormExtensionContext::DisconnectAbility) },
        ani_native_function { "nativeCheckWant", SIGNATURE_CHECK_WANT,
            reinterpret_cast<void *>(ETSFormExtensionContext::CheckWant) },
        ani_native_function { "nativeCheckConnectionAbility", SIGNATURE_CHECK_CONNECTION,
            reinterpret_cast<void *>(ETSFormExtensionContext::CheckConnectionAbility) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "bind method status: %{public}d", status);
        return false;
    }
    ani_class cleanerCls = nullptr;
    status = env->FindClass(CLEANER_CLASS_NAME, &cleanerCls);
    if (status != ANI_OK || cleanerCls == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to find class, status: %{public}d", status);
        return false;
    }
    std::array CleanerMethods = {
        ani_native_function { "clean", nullptr, reinterpret_cast<void *>(ETSFormExtensionContext::Finalizer) },
    };
    if ((status = env->Class_BindNativeMethods(cleanerCls, CleanerMethods.data(), CleanerMethods.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "bind method status: %{public}d", status);
        return false;
    }
    return true;
}
bool CheckConnectionParam(ani_object connectOptionsObj,
    sptr<ETSFormExtensionConnection>& connection, AAFwk::Want& want)
{
    connection->SetConnectionRef(connectOptionsObj);
    ConnectionKey key;
    key.id = g_serialNumber;
    key.want = want;
    connection->SetConnectionId(key.id);
    {
        std::lock_guard<std::mutex> lock(g_connectsMutex_);
        g_connects.emplace(key, connection);
    }
    if (g_serialNumber < INT32_MAX) {
        g_serialNumber++;
    } else {
        g_serialNumber = 0;
    }
    TAG_LOGD(AAFwkTag::FORM_EXT, "not find connection");
    return true;
}

void FindConnection(AAFwk::Want& want,
    sptr<ETSFormExtensionConnection>& connection, int64_t& connectId)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "connection:%{public}d", static_cast<int32_t>(connectId));
    std::lock_guard<std::mutex> lock(g_connectsMutex_);
    auto item = std::find_if(g_connects.begin(),
        g_connects.end(),
        [&connectId](const auto &obj) {
            return connectId == obj.first.id;
        });
    if (item != g_connects.end()) {
        // match id
        want = item->first.want;
        connection = item->second;
        TAG_LOGD(AAFwkTag::FORM_EXT, "ability not exist");
    }
    return;
}

void RemoveConnection(int64_t connectId)
{
    std::lock_guard<std::mutex> lock(g_connectsMutex_);
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
    [&connectId](const auto &obj) {
        return connectId == obj.first.id;
    });
    if (item != g_connects.end()) {
        TAG_LOGD(AAFwkTag::FORM_EXT, "ability exist");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    } else {
        TAG_LOGD(AAFwkTag::FORM_EXT, "ability not exist");
    }
}
} // namespace

void ETSFormExtensionContext::Finalizer(ani_env *env, ani_object obj)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "Finalizer");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    ani_long nativeEtsContextPtr;
    if (env->Object_GetFieldByName_Long(obj, "nativeEtsContext", &nativeEtsContextPtr) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to get nativeEtsContext");
        return;
    }
    if (nativeEtsContextPtr != 0) {
        delete reinterpret_cast<ETSFormExtensionContext *>(nativeEtsContextPtr);
    }
}

void ETSFormExtensionContext::StartAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "StartAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    auto etsFormExtensionContext = ETSFormExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsFormExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null etsFormExtensionContext");
        return;
    }
    etsFormExtensionContext->OnStartAbility(env, aniObj, wantObj, call);
}

ani_long ETSFormExtensionContext::ConnectAbility(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object connectOptionsObj)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "ConnectAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return ERROR_LONG_VALUE;
    }
    auto etsFormExtensionContext = ETSFormExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsFormExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null etsFormExtensionContext");
        return ERROR_LONG_VALUE;
    }
    return etsFormExtensionContext->OnConnectAbility(env, aniObj, wantObj, connectOptionsObj);
}

void ETSFormExtensionContext::DisconnectAbility(ani_env *env, ani_object aniObj, ani_long connectId,
    ani_object callback)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "DisconnectAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    auto etsFormExtensionContext = ETSFormExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsFormExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null etsFormExtensionContext");
        return;
    }
    etsFormExtensionContext->OnDisconnectAbility(env, aniObj, connectId, callback);
}

void ETSFormExtensionContext::CheckConnectionAbility(ani_env *env, ani_object aniObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    auto etsFormExtensionContext = ETSFormExtensionContext::GetEtsAbilityContext(env, aniObj);
    if (etsFormExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null etsFormExtensionContext");
        return;
    }
    etsFormExtensionContext->OnCheckConnectionAbility(env, aniObj);
}

void ETSFormExtensionContext::OnCheckConnectionAbility(ani_env *env, ani_object aniObj)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
}

ETSFormExtensionContext *ETSFormExtensionContext::GetEtsAbilityContext(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "GetEtsAbilityContext");
    ani_class cls = nullptr;
    ani_long nativeContextLong;
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(FORM_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to find class, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeEtsContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to find filed, status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(aniObj, contextField, &nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to get filed, status : %{public}d", status);
        return nullptr;
    }
    auto weakContext = reinterpret_cast<ETSFormExtensionContext *>(nativeContextLong);
    return weakContext;
}

void ETSFormExtensionContext::OnStartAbility(ani_env *env, ani_object aniObj, ani_object wantObj, ani_object call)
{
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "unwrap want failed");
        EtsErrorUtil::ThrowError(env, (int32_t)AbilityErrorCode::ERROR_CODE_INVALID_PARAM,
            "Parameter error. The type of \"want\" must be Want.");
        return;
    }
    TAG_LOGI(AAFwkTag::FORM_EXT, "Start bundle: %{public}s ability: %{public}s",
        want.GetBundle().c_str(),
        want.GetElement().GetAbilityName().c_str());
    auto context = context_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null context");
        AppExecFwk::AsyncCallback(env, call,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    ErrCode innerErrorCode = context->StartAbility(want);
    if (innerErrorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Start failed: %{public}d", innerErrorCode);
        AppExecFwk::AsyncCallback(env, call,
            EtsErrorUtil::CreateErrorByNativeErr(env, (int32_t)innerErrorCode), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, call, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

void ETSFormExtensionContext::CheckWant(ani_env *env, ani_object aniObj, ani_object wantObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "unwrap want failed");
        EtsErrorUtil::ThrowError(env, (int32_t)AbilityErrorCode::ERROR_CODE_INVALID_PARAM,
            "Parameter error. The type of \"want\" must be Want.");
        return;
    }
}

ani_long ETSFormExtensionContext::OnConnectAbility(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object connectOptionsObj)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return ERROR_LONG_VALUE;
    }
    ani_vm *etsVm = nullptr;
    if (env->GetVM(&etsVm) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to getVM");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return ERROR_LONG_VALUE;
    }
    AAFwk::Want want;
    sptr<ETSFormExtensionConnection> connection = new ETSFormExtensionConnection(etsVm);
    if (!AppExecFwk::UnwrapWant(env, wantObj, want) ||
        !CheckConnectionParam(connectOptionsObj, connection, want)) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return ERROR_LONG_VALUE;
    }
    int64_t connectId = connection->GetConnectionId();
    auto context = context_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        RemoveConnection(connectId);
        return ERROR_LONG_VALUE;
    }
    auto innerErrorCode = context->ConnectAbility(want, connection);
    int32_t errcode = static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrorCode));
    if (errcode) {
        connection->CallEtsFailed(errcode);
        RemoveConnection(connectId);
        return ERROR_LONG_VALUE;
    }
    return connectId;
}

void ETSFormExtensionContext::OnDisconnectAbility(ani_env *env, ani_object aniObj, ani_long connectId,
    ani_object callback)
{
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
    AAFwk::Want want;
    sptr<ETSFormExtensionConnection> connection = nullptr;
    FindConnection(want, connection, connectId);
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null context");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    if (!connection) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null connection");
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }
    auto innerErrorCode = context->DisconnectAbility(want, connection);
    if (innerErrorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Disconnect failed: %{public}d", innerErrorCode);
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateErrorByNativeErr(env, (int32_t)innerErrorCode), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

ani_object CreateEtsFormExtensionContext(ani_env *env, std::shared_ptr<FormExtensionContext> &context)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "CreateEtsFormExtensionContext call");
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null env or context");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(FORM_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to find class, status: %{public}d", status);
        return nullptr;
    }
    if (!BindNativeMethods(env, cls)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to BindNativeMethods");
        return nullptr;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "l:", &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to find constructor, status : %{public}d", status);
        return nullptr;
    }
    std::unique_ptr<ETSFormExtensionContext> workContext = std::make_unique<ETSFormExtensionContext>(context);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to create etsFormExtensionContext");
        return nullptr;
    }
    auto formContextPtr = new std::weak_ptr<FormExtensionContext> (workContext->GetAbilityContext());
    if (formContextPtr == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "formContextPtr is nullptr");
        return nullptr;
    }
    ani_object contextObj = nullptr;
    if ((status = env->Object_New(cls, method, &contextObj, (ani_long)workContext.release())) != ANI_OK ||
        contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to create object, status : %{public}d", status);
        return nullptr;
    }
    if (!ContextUtil::SetNativeContextLong(env, contextObj, (ani_long)(formContextPtr))) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to setNativeContextLong ");
        return nullptr;
    }
    ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);
    CreateEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());
    return contextObj;
}

ETSFormExtensionConnection::ETSFormExtensionConnection(ani_vm *etsVm) : etsVm_(etsVm) {}

ETSFormExtensionConnection::~ETSFormExtensionConnection()
{
    RemoveConnectionObject();
}

void ETSFormExtensionConnection::SetConnectionId(int32_t id)
{
    connectionId_ = id;
}

void ETSFormExtensionConnection::RemoveConnectionObject()
{
    if (etsVm_ != nullptr && stsConnectionRef_ != nullptr) {
        ani_env *env = nullptr;
        if (etsVm_->GetEnv(ANI_VERSION_1, &env) == ANI_OK && env != nullptr) {
            env->GlobalReference_Delete(stsConnectionRef_);
            stsConnectionRef_ = nullptr;
        }
    }
}

void ETSFormExtensionConnection::CallEtsFailed(int32_t errorCode)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "CallEtsFailed");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null etsVm");
        return;
    }
    if (stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null stsConnectionRef_");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_OK;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to get env, status: %{public}d", status);
        return;
    }
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(stsConnectionRef_),
        "onFailed", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get onFailed failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::FORM_EXT, "invalid onFailed property");
        return;
    }
    ani_object errorCodeObj = AppExecFwk::CreateInt(env, errorCode);
    if (errorCodeObj == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null errorCodeObj");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { errorCodeObj };
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to call onFailed, status: %{public}d", status);
    }
}

void ETSFormExtensionConnection::SetConnectionRef(ani_object connectOptionsObj)
{
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "etsVm_ is nullptr");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->GlobalReference_Create(connectOptionsObj, &stsConnectionRef_)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status: %{public}d", status);
    }
}

void ETSFormExtensionConnection::OnAbilityConnectDone(
    const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int32_t resultCode)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnAbilityConnectDone");
    if (etsVm_ == nullptr || stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null stsConnectionRef or etsVm");
        return;
    }
    ani_env *env = AttachCurrentThread();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "GetEnv failed");
        return;
    }
    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    if (refElement == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null refElement");
        DetachCurrentThread();
        return;
    }
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null remoteObject");
        DetachCurrentThread();
        return;
    }
    ani_object refRemoteObject = ANI_ohos_rpc_CreateJsRemoteObject(env, remoteObject);
    if (refRemoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null refRemoteObject");
        DetachCurrentThread();
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(stsConnectionRef_),
        "onConnect", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get onConnect failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::FORM_EXT, "invalid onConnect property");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { refElement, refRemoteObject};
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_TWO, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to call onConnect, status: %{public}d", status);
    }
    DetachCurrentThread();
}

void ETSFormExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode)
{
    TAG_LOGI(AAFwkTag::FORM_EXT, "OnAbilityDisconnectDone");
    if (etsVm_ == nullptr || stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null stsConnectionRef or etsVm");
        return;
    }
    ani_env *env = AttachCurrentThread();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "GetEnv failed");
        return;
    }
    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    if (refElement == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null refElement");
        DetachCurrentThread();
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref funRef;
    if ((status = env->Object_GetPropertyByName_Ref(reinterpret_cast<ani_object>(stsConnectionRef_),
        "onDisconnect", &funRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get onDisconnect failed status : %{public}d", status);
        return;
    }
    if (!AppExecFwk::IsValidProperty(env, funRef)) {
        TAG_LOGI(AAFwkTag::FORM_EXT, "invalid onDisconnect property");
        return;
    }
    ani_ref result;
    std::vector<ani_ref> argv = { refElement };
    if ((status = env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(funRef), ARGC_ONE, argv.data(),
        &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Failed to call onDisconnect, status: %{public}d", status);
    }
    DetachCurrentThread();
}

ani_env *ETSFormExtensionConnection::AttachCurrentThread()
{
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) == ANI_OK) {
        return env;
    }
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if ((status = etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "status: %{public}d", status);
        return nullptr;
    }
    isAttachThread_ = true;
    return env;
}

void ETSFormExtensionConnection::DetachCurrentThread()
{
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null etsVm");
        return;
    }
    if (isAttachThread_) {
        etsVm_->DetachCurrentThread();
        isAttachThread_ = false;
    }
}

ani_object NativeTransferStatic(ani_env *aniEnv, ani_object, ani_object input, ani_object type)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "transfer static FormExtensionContext");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null aniEnv");
        return nullptr;
    }

    void *unwrapResult = nullptr;
    bool success = arkts_esvalue_unwrap(aniEnv, input, &unwrapResult);
    if (!success) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "failed to unwrap");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }
    if (unwrapResult == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null unwrapResult");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto context = reinterpret_cast<std::weak_ptr<FormExtensionContext> *>(unwrapResult)->lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null FormExtensionContext");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto formExtContext = Context::ConvertTo<FormExtensionContext>(context);
    if (formExtContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "invalid formExtContext");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto &bindingObj = formExtContext->GetBindingObject();
    if (bindingObj == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null bindingObj");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto staticContext = bindingObj->Get<ani_ref>();
    if (staticContext != nullptr) {
        TAG_LOGI(AAFwkTag::FORM_EXT, "there exist a staticContext");
        return reinterpret_cast<ani_object>(*staticContext);
    }

    // if not exist, create a new one
    std::string contextType;
    if (!AppExecFwk::GetStdString(aniEnv, reinterpret_cast<ani_string>(type), contextType)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "GetStdString failed");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::FORM_EXT, "contextType %{public}s", contextType.c_str());

    auto newContext = ContextTransfer::GetInstance().GetStaticObject(contextType, aniEnv, context);
    if (newContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "create formExtContext failed");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    return newContext;
}

napi_value AttachFormExtensionContext(napi_env env, void *value, void *)
{
    if (value == nullptr) {
        TAG_LOGW(AAFwkTag::FORM_EXT, "null value");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AbilityRuntime::FormExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::FORM_EXT, "null ptr");
        return nullptr;
    }
    napi_value object = AbilityRuntime::CreateJsFormExtensionContext(env, ptr);
    auto sysModule =
        AbilityRuntime::JsRuntime::LoadSystemModuleByEngine(env, "application.FormExtensionContext", &object, 1);
    if (sysModule == nullptr) {
        TAG_LOGW(AAFwkTag::FORM_EXT, "null sysModule");
        return nullptr;
    }
    auto contextObj = sysModule->GetNapiValue();
    napi_coerce_to_native_binding_object(
        env, contextObj, AbilityRuntime::DetachCallbackFunc, AttachFormExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AbilityRuntime::FormExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::FORM_EXT, "Finalizer for weak_ptr form extension context is called");
            delete static_cast<std::weak_ptr<AbilityRuntime::FormExtensionContext> *>(data);
        },
        nullptr, nullptr);
    return contextObj;
}

std::unique_ptr<NativeReference> CreateNativeReference(
    napi_env napiEnv, std::shared_ptr<FormExtensionContext> formExtContext)
{
    if (napiEnv == nullptr || formExtContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null param");
        return nullptr;
    }

    auto value = CreateJsFormExtensionContext(napiEnv, formExtContext);
    auto systemModule =
        JsRuntime::LoadSystemModuleByEngine(napiEnv, "application.FormExtensionContext", &value, 1);
    if (systemModule == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null systemModule");
        return nullptr;
    }

    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(napiEnv, object, napi_object)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "check type failed");
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<FormExtensionContext>(formExtContext);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null workContext");
        return nullptr;
    }
    auto status = napi_coerce_to_native_binding_object(
        napiEnv, object, DetachCallbackFunc, AttachFormExtensionContext, workContext, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "coerce FormExtensionContext failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    status = napi_wrap(napiEnv, object, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::FORM_EXT, "finalizer for weak_ptr FormExtensionContext");
            delete static_cast<std::weak_ptr<FormExtensionContext> *>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "wrap failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return systemModule;
}

napi_value GetOrCreateDynamicObject(
    napi_env napiEnv, std::shared_ptr<FormExtensionContext> formExtContext)
{
    if (napiEnv == nullptr || formExtContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null param");
        return nullptr;
    }

    // if sub-thread, create a new formExtContext and return
    if (getpid() != syscall(SYS_gettid)) {
        auto subThreadObj =
            static_cast<NativeReference *>(formExtContext->GetSubThreadObject(static_cast<void *>(napiEnv)));
        if (subThreadObj != nullptr) {
            return subThreadObj->Get();
        }
        auto subThreadRef = CreateNativeReference(napiEnv, formExtContext);
        if (subThreadRef == nullptr) {
            return nullptr;
        }
        auto newObject = subThreadRef->Get();
        formExtContext->BindSubThreadObject(
            static_cast<void *>(napiEnv), static_cast<void *>(subThreadRef.release()));
        return newObject;
    }

    // if main-thread, get bindingObj firstly
    auto &bindingObj = formExtContext->GetBindingObject();
    if (bindingObj == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null bindingObj");
        return nullptr;
    }

    // if main-thread bindingObj exist, return it directly
    auto dynamicContext = bindingObj->Get<NativeReference>();
    if (dynamicContext != nullptr) {
        TAG_LOGI(AAFwkTag::FORM_EXT, "there exist a dynamicContext");
        return dynamicContext->Get();
    }

    // if main-thread bindingObj didn't exist, create and bind
    auto nativeRef = CreateNativeReference(napiEnv, formExtContext);
    if (nativeRef == nullptr) {
        return nullptr;
    }

    auto object = nativeRef->Get();
    formExtContext->Bind(nativeRef.release());
    return object;
}

ani_object NativeTransferDynamic(ani_env *aniEnv, ani_class aniCls, ani_object input)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "transfer dynamic FormExtensionContext");
    if (!IsInstanceOf(aniEnv, input)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "not FormExtensionContext");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto context = AbilityRuntime::GetStageModeContext(aniEnv, input);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null context");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    std::shared_ptr<FormExtensionContext> formExtContext =
        Context::ConvertTo<FormExtensionContext>(context);
    if (formExtContext == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "invalid formExtContext");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    std::string contextType;
    if (!AppExecFwk::GetStaticFieldString(aniEnv, aniCls, "contextType", contextType)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "get context type failed");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::FORM_EXT, "contextType %{public}s", contextType.c_str());

    // get napiEnv from aniEnv
    napi_env napiEnv = {};
    if (!arkts_napi_scope_open(aniEnv, &napiEnv)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "arkts_napi_scope_open failed");
        return nullptr;
    }

    // create normal form extension context
    auto contextObj = ContextTransfer::GetInstance().GetDynamicObject(contextType, napiEnv, formExtContext);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "create FormExtensionContext failed");
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    hybridgref ref = nullptr;
    bool success = hybridgref_create_from_napi(napiEnv, contextObj, &ref);
    if (!success) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "hybridgref_create_from_napi failed");
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    ani_object result = nullptr;
    success = hybridgref_get_esvalue(aniEnv, ref, &result);
    if (!success) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "hybridgref_get_esvalue failed");
        hybridgref_delete_from_napi(napiEnv, ref);
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    hybridgref_delete_from_napi(napiEnv, ref);

    if (!arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr)) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "arkts_napi_scope_close_n failed");
        return nullptr;
    }

    return result;
}

bool IsInstanceOf(ani_env *aniEnv, ani_object aniObj)
{
    ani_class cls {};
    ani_status status = ANI_ERROR;
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null aniEnv");
        return false;
    }
    if ((status = aniEnv->FindClass(FORM_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "FindClass status: %{public}d", status);
        return false;
    }
    ani_boolean isInstanceOf = false;
    if ((status = aniEnv->Object_InstanceOf(aniObj, cls, &isInstanceOf)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Object_InstanceOf status: %{public}d", status);
        return false;
    }
    return isInstanceOf;
}

void EtsFormExtensionContextModuleInit(ani_env *aniEnv)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "Init FormExtensionContext kit");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null ani env");
        return;
    }

    ani_class formExtContextCls = nullptr;
    auto status = aniEnv->FindClass(FORM_EXTENSION_CONTEXT_CLASS_NAME, &formExtContextCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "FindClass FormExtensionContext failed status: %{public}d", status);
        return;
    }

    std::array nativeFuncs = {
        ani_native_function { "nativeTransferStatic", "C{std.interop.ESValue}C{std.core.String}:C{std.core.Object}",
            reinterpret_cast<void *>(NativeTransferStatic) },
        ani_native_function { "nativeTransferDynamic", "C{std.core.Object}:C{std.interop.ESValue}",
            reinterpret_cast<void *>(NativeTransferDynamic) },
    };
    status = aniEnv->Class_BindStaticNativeMethods(formExtContextCls, nativeFuncs.data(), nativeFuncs.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "Class_BindStaticNativeMethods failed status: %{public}d", status);
        return;
    }

    ContextTransfer::GetInstance().RegisterStaticObjectCreator("FormExtensionContext",
        [](ani_env *aniEnv, std::shared_ptr<Context> context) -> ani_object {
            auto formExtContext = Context::ConvertTo<FormExtensionContext>(context);
            if (formExtContext == nullptr) {
                TAG_LOGE(AAFwkTag::FORM_EXT, "invalid formExtContext");
                return nullptr;
            }
            auto newContext = CreateEtsFormExtensionContext(aniEnv, formExtContext);
            if (newContext == nullptr) {
                TAG_LOGE(AAFwkTag::FORM_EXT, "create formExtContext failed");
                return nullptr;
            }
            return newContext;
        });

    ContextTransfer::GetInstance().RegisterDynamicObjectCreator("FormExtensionContext",
        [](napi_env napiEnv, std::shared_ptr<Context> context) -> napi_value {
            auto formExtContext = Context::ConvertTo<FormExtensionContext>(context);
            if (formExtContext == nullptr) {
                TAG_LOGE(AAFwkTag::FORM_EXT, "invalid formExtContext");
                return nullptr;
            }

            auto object = GetOrCreateDynamicObject(napiEnv, formExtContext);
            if (object == nullptr) {
                TAG_LOGE(AAFwkTag::FORM_EXT, "get or create object failed");
                return nullptr;
            }
            return object;
        });

    TAG_LOGD(AAFwkTag::FORM_EXT, "Init FormExtensionContext kit end");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::FORM_EXT, "ANI_Constructor");
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "null vm");
        return ANI_ERROR;
    }

    ani_env *aniEnv = nullptr;
    ani_status status = vm->GetEnv(ANI_VERSION_1, &aniEnv);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::FORM_EXT, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    EtsFormExtensionContextModuleInit(aniEnv);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::FORM_EXT, "ANI_Constructor finish");
    return ANI_OK;
}
}

} // namespace AbilityRuntime
} // namespace OHOS