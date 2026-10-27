/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "ets_auto_fill_extension_context.h"

#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ets_auto_fill_extension_util.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_extension_context.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* AUTO_FILL_EXTENSION_CONTEXT_CLASS_NAME =
    "application.AutoFillExtensionContext.AutoFillExtensionContext";
constexpr const char* CLEANER_CLASS = "application.AutoFillExtensionContext.Cleaner";
constexpr const char* SIGNATURE_CONNECT_SERVICE_EXTENSION =
    "C{@ohos.app.ability.Want.Want}C{ability.connectOptions.ConnectOptions}:l";
constexpr const char* SIGNATURE_DISCONNECT_SERVICE_EXTENSION = "lC{utils.AbilityUtils.AsyncCallbackWrapper}:";
constexpr const int FAILED_CODE = -1;
static std::mutex g_connectsMutex;
static int32_t g_serialNumber = 0;
static std::map<EtsUIExtensionConnectionKey, sptr<EtsUIExtensionConnection>, Etskey_compare> g_connects;

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

bool CheckConnectionParam(ani_env *env, ani_object connectOptionsObj,
    sptr<EtsAutoFillExtensionConnection> &connection, AAFwk::Want &want)
{
    ani_type type = nullptr;
    ani_boolean res = ANI_FALSE;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetType(connectOptionsObj, &type)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Failed to Object_GetType, status: %{public}d", status);
        return false;
    }
    if (type == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null type");
        return false;
    }
    if ((status = env->Object_InstanceOf(connectOptionsObj, type, &res)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Failed to Object_InstanceOf, status: %{public}d", status);
        return false;
    }
    if (res != ANI_TRUE) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Failed to CheckConnectionParam");
        return false;
    }
    connection->SetConnectionRef(connectOptionsObj);
    EtsUIExtensionConnectionKey key;
    {
        std::lock_guard guard(g_connectsMutex);
        key.id = g_serialNumber;
        key.want = want;
        connection->SetConnectionId(key.id);
        g_connects.emplace(key, connection);
        if (g_serialNumber < INT32_MAX) {
            g_serialNumber++;
        } else {
            g_serialNumber = 0;
        }
    }
    return true;
}
} // namespace

ani_object EtsAutoFillExtensionContext::SetEtsAutoFillExtensionContext(ani_env *env,
    std::shared_ptr<AutoFillExtensionContext> context)
{
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env or context");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_class cls = nullptr;
    if ((status = env->FindClass(AUTO_FILL_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find class status: %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "l:", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find method status: %{public}d", status);
        return nullptr;
    }
    auto etsAutoFillExtensionContext = new (std::nothrow) EtsAutoFillExtensionContext(context);
    if (etsAutoFillExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "etsAutoFillExtensionContext nullptr");
        return nullptr;
    }
    ani_object contextObj = nullptr;
    if ((status = env->Object_New(cls, method, &contextObj, reinterpret_cast<ani_long>(etsAutoFillExtensionContext)))
        != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "new object status: %{public}d", status);
        delete etsAutoFillExtensionContext;
        etsAutoFillExtensionContext = nullptr;
        return nullptr;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AutoFillExtensionContext>(
        etsAutoFillExtensionContext->context_);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "workContext nullptr");
        delete etsAutoFillExtensionContext;
        etsAutoFillExtensionContext = nullptr;
        return nullptr;
    }
    if (!ContextUtil::SetNativeContextLong(env, contextObj, reinterpret_cast<ani_long>(workContext))) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "SetNativeContextLong failed");
        delete workContext;
        workContext = nullptr;
        delete etsAutoFillExtensionContext;
        etsAutoFillExtensionContext = nullptr;
        return nullptr;
    }
    return contextObj;
}

EtsAutoFillExtensionContext *EtsAutoFillExtensionContext::GetEtsAutoFillExtensionContext(ani_env *env,
    ani_object object)
{
    if (env == nullptr || object == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env or object");
        return nullptr;
    }
    ani_long autoFillExtensionContextPtr = 0;
    ani_status status = env->Object_GetFieldByName_Long(object, "autoFillExtensionContextPtr",
        &autoFillExtensionContextPtr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "autoFillExtensionContextPtr GetField status: %{public}d", status);
        return nullptr;
    }
    auto etsAutoFillExtensionContext = reinterpret_cast<EtsAutoFillExtensionContext *>(autoFillExtensionContextPtr);
    if (etsAutoFillExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "etsAutoFillExtensionContext null");
        return nullptr;
    }
    return etsAutoFillExtensionContext;
}

void EtsAutoFillExtensionContext::Clean(ani_env *env, ani_object object)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return;
    }
    ani_long ptr = 0;
    ani_status status = env->Object_GetFieldByName_Long(object, "ptr", &ptr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "ptr GetField status: %{public}d", status);
        return;
    }
    if (ptr != 0) {
        delete reinterpret_cast<EtsAutoFillExtensionContext *>(ptr);
    }
}

void EtsAutoFillExtensionContext::ReloadInModal(ani_env *env, ani_object object, ani_object customDataObj,
    ani_object callback)
{
    auto etsAutoFillExtensionContext = GetEtsAutoFillExtensionContext(env, object);
    if (etsAutoFillExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null etsAutoFillExtensionContext");
        return;
    }
    etsAutoFillExtensionContext->OnReloadInModal(env, object, customDataObj, callback);
}

void EtsAutoFillExtensionContext::OnReloadInModal(ani_env *env, ani_object object, ani_object customDataObj,
    ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_ref etsCustomData = nullptr;
    if ((status = env->Object_GetPropertyByName_Ref(customDataObj, "data", &etsCustomData)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Object_GetPropertyByName_Ref failed, status: %{public}d", status);
        return;
    }
    CustomData customData;
    if (etsCustomData == nullptr || !AppExecFwk::UnwrapWantParams(env, etsCustomData, customData.data)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Parse custom data failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    auto ret = context->ReloadInModal(customData);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "error is %{public}d", ret);
    }
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
}

ani_long EtsAutoFillExtensionContext::ConnectServiceExtensionAbility(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object connectOptionsObj)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "ConnectServiceExtensionAbility");
    auto etsAutoFillExtensionContext = GetEtsAutoFillExtensionContext(env, aniObj);
    if (etsAutoFillExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null etsAutoFillExtensionContext");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    return etsAutoFillExtensionContext->OnConnectServiceExtensionAbility(env, aniObj, wantObj, connectOptionsObj);
}

ani_long EtsAutoFillExtensionContext::OnConnectServiceExtensionAbility(ani_env *env, ani_object aniObj,
    ani_object wantObj, ani_object connectOptionsObj)
{
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return FAILED_CODE;
    }
    ani_status status = ANI_ERROR;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return FAILED_CODE;
    }
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Failed to UnwrapWant");
        EtsErrorUtil::ThrowInvalidParamError(env, "Failed to UnwrapWant");
        return FAILED_CODE;
    }
    ani_vm *etsVm = nullptr;
    if ((status = env->GetVM(&etsVm)) != ANI_OK || etsVm == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Failed to getVM, status: %{public}d", status);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    auto connection = sptr<EtsAutoFillExtensionConnection>::MakeSptr(etsVm);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null connection");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    if (!CheckConnectionParam(env, connectOptionsObj, connection, want)) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Failed to CheckConnectionParam");
        EtsErrorUtil::ThrowInvalidParamError(env, "Failed to CheckConnectionParam");
        return FAILED_CODE;
    }
    auto innerErrCode = context->ConnectAbility(want, connection);
    int32_t errcode = static_cast<int32_t>(GetJsErrorCodeByNativeError(innerErrCode));
    int64_t connectId = connection->GetConnectionId();
    if (errcode != ERR_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Failed to ConnectAbility, innerErrCode is %{public}d", innerErrCode);
        connection->CallEtsFailed(errcode);
        RemoveConnection(connection->GetConnectionId());
        return FAILED_CODE;
    }
    return connectId;
}

void EtsAutoFillExtensionContext::DisconnectServiceExtensionAbility(ani_env *env, ani_object aniObj,
    ani_long connectId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "DisconnectServiceExtensionAbility");
    auto etsAutoFillExtensionContext = GetEtsAutoFillExtensionContext(env, aniObj);
    if (etsAutoFillExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null etsAutoFillExtensionContext");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    etsAutoFillExtensionContext->OnDisconnectServiceExtensionAbility(env, aniObj, connectId, callback);
}

void EtsAutoFillExtensionContext::CheckConnectionAbility(ani_env *env, ani_object aniObj)
{
    if (!AppExecFwk::CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
}

void EtsAutoFillExtensionContext::OnDisconnectServiceExtensionAbility(ani_env *env, ani_object aniObj,
    ani_long connectId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::AUTOFILL_EXT, "OnDisconnectServiceExtensionAbility");
    ani_object aniObject = nullptr;
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null context");
        aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        return;
    }
    AAFwk::Want want;
    sptr<EtsUIExtensionConnection> connection = nullptr;
    {
        std::lock_guard<std::mutex> lock(g_connectsMutex);
        auto item = std::find_if(
            g_connects.begin(), g_connects.end(), [&connectId](const auto &obj) { return connectId == obj.first.id; });
        if (item != g_connects.end()) {
            want = item->first.want;
            connection = item->second;
        } else {
            TAG_LOGI(AAFwkTag::AUTOFILL_EXT, "Failed to found connection");
        }
    }
    if (!connection) {
        aniObject = EtsErrorUtil::CreateError(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER),
            GetInnerErrorMsg(AbilityInnerErrorMsg::CONNECTION_NOT_FOUND));
        AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null connection");
        return;
    }
    ErrCode ret = context->DisconnectAbility(want, connection);
    aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, ret);
    AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
}

ani_object EtsAutoFillExtensionContext::CreateEtsAutoFillExtensionContext(ani_env *env,
    std::shared_ptr<AutoFillExtensionContext> context)
{
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null env or context");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_class cls = nullptr;
    if ((status = env->FindClass(AUTO_FILL_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "find class status: %{public}d", status);
        return nullptr;
    }
    std::array functions = {
        ani_native_function { "nativeReloadInModal",
            "C{application.CustomData.CustomData}C{utils.AbilityUtils.AsyncCallbackWrapper}:",
            reinterpret_cast<void*>(EtsAutoFillExtensionContext::ReloadInModal) },
        ani_native_function { "nativeConnectServiceExtensionAbility", SIGNATURE_CONNECT_SERVICE_EXTENSION,
            reinterpret_cast<void*>(EtsAutoFillExtensionContext::ConnectServiceExtensionAbility) },
        ani_native_function { "nativeDisconnectServiceExtensionAbilitySync", SIGNATURE_DISCONNECT_SERVICE_EXTENSION,
            reinterpret_cast<void*>(EtsAutoFillExtensionContext::DisconnectServiceExtensionAbility) },
        ani_native_function { "nativeCheckConnectionAbility", ":",
            reinterpret_cast<void*>(EtsAutoFillExtensionContext::CheckConnectionAbility) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "bind method status : %{public}d", status);
        return nullptr;
    }
    ani_class cleanerCls = nullptr;
    if ((status = env->FindClass(CLEANER_CLASS, &cleanerCls)) != ANI_OK || cleanerCls == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "Cleaner FindClass failed status: %{public}d, or null cleanerCls", status);
        return nullptr;
    }
    std::array cleanerMethods = {
        ani_native_function {"clean", nullptr, reinterpret_cast<void *>(EtsAutoFillExtensionContext::Clean) },
    };
    if ((status = env->Class_BindNativeMethods(cleanerCls, cleanerMethods.data(), cleanerMethods.size())) !=
        ANI_OK && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "cleanerCls Class_BindNativeMethods failed status: %{public}d", status);
        return nullptr;
    }
    ani_object contextObj = SetEtsAutoFillExtensionContext(env, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "null contextObj");
        return nullptr;
    }
    AbilityRuntime::ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);
    AbilityRuntime::CreateEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());
    ani_ref *contextGlobalRef = new (std::nothrow) ani_ref;
    if (contextGlobalRef == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "new contextGlobalRef failed");
        return nullptr;
    }
    if ((status = env->GlobalReference_Create(contextObj, contextGlobalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "GlobalReference_Create failed status: %{public}d", status);
        delete contextGlobalRef;
        return nullptr;
    }
    context->Bind(contextGlobalRef);
    return contextObj;
}

void EtsAutoFillExtensionConnection::HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element,
    int resultCode)
{
    bool isAttachThread = false;
    ani_env *env = AppExecFwk::AttachAniEnv(etsVm_, isAttachThread);
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::AUTOFILL_EXT, "GetEnv failed");
        return;
    }
    EtsUIExtensionConnection::HandleOnAbilityDisconnectDone(element, resultCode);
    RemoveConnection(connectionId_);
    AppExecFwk::DetachAniEnv(etsVm_, isAttachThread);
}
} // namespace AbilityRuntime
} // namespace OHOS