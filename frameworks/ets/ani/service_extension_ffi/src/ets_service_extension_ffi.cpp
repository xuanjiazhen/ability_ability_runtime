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

#include "ets_service_extension_ffi.h"

#include <cstddef>
#include <cstdint>
#include <memory>

#include "ability_info.h"
#include "ability_manager_client.h"
#include "ani_common_want.h"
#include "ani_remote_object.h"
#include "configuration_utils.h"
#include "ets_envsetup.h"
#include "ets_runtime.h"
#include "ets_service_extension.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_service_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *CLASSNAME_SERVICE_ABILITY =
    "L@ohos/app/ability/ServiceExtensionAbility/ServiceExtensionAbility;";
constexpr const char *NATIVE_ONCONNECT_CALLBACK_SIGNATURE = "L@ohos/rpc/rpc/RemoteObject;:Z";
constexpr const char *ON_CREATE_SIGNATURE = "L@ohos/app/ability/Want/Want;:V";
constexpr const char *VOID_SIGNATURE = ":V";
constexpr const char *ON_CONNECT_SIGNATURE = "L@ohos/app/ability/Want/Want;:Lstd/core/Object;";
constexpr const char *CHECK_PROMISE_SIGNATURE = "Lstd/core/Object;:Z";
constexpr const char *CALL_PROMISE_SIGNATURE = "Lstd/core/Object;:Z";
constexpr const char *ON_DISCONNECT_SIGNATURE = "L@ohos/app/ability/Want/Want;:V";
constexpr const char *ON_REQUEST_SIGNATURE = "L@ohos/app/ability/Want/Want;D:V";
} // namespace

void EtsServiceExtensionFFI::DisconnectPromiseCallback(ani_env *env, ani_object aniObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "DisconnectPromiseCallback");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    ani_long disconnectCallbackPoint = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "disconnectCallbackPoint", &disconnectCallbackPoint)) !=
        ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    auto *callbackInfo = reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<void> *>(disconnectCallbackPoint);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null callbackInfo");
        return;
    }
    callbackInfo->Call();
    AppExecFwk::AbilityTransactionCallbackInfo<>::Destroy(callbackInfo);
}

void EtsServiceExtensionFFI::ConnectPromiseCallback(ani_env *env, ani_object aniObj, ani_object obj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "ConnectPromiseCallback");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    ani_long connectCallbackPoint = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "connectCallbackPoint", &connectCallbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    auto remoteObject = AniGetNativeRemoteObject(env, obj);
    if (remoteObject == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null remoteObject");
    }
    auto *callbackInfo =
        reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *>(connectCallbackPoint);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null callbackInfo");
        return;
    }

    callbackInfo->Call(remoteObject);
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>>::Destroy(callbackInfo);
}

EtsServiceExtensionFFI::EtsServiceExtensionFFI(AbilityHandle ability, RuntimeHandle runtime)
{
    ability_ = reinterpret_cast<AbilityRuntime::EtsServiceExtension *>(ability);
    runtime_ = reinterpret_cast<AbilityRuntime::ETSRuntime *>(runtime);
}

EtsServiceExtensionFFI::~EtsServiceExtensionFFI()
{
}

bool EtsServiceExtensionFFI::LoadModule(const std::string &moduleName, const std::string &modulePath, const std::string &hapPath,
    bool esmodule, bool useCommonChunk, const std::string &srcEntrance)
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null runtime_");
        return false;
    }

    etsAbilityObj_ = runtime_->LoadModule(moduleName, modulePath, hapPath, esmodule, useCommonChunk, srcEntrance);
    if (etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsAbilityObj_");
        return false;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return false;
    }
    std::array functions = {
        ani_native_function {
            "nativeOnDisconnectCallback", VOID_SIGNATURE,
            reinterpret_cast<void *>(EtsServiceExtensionFFI::DisconnectPromiseCallback) },
        ani_native_function { "nativeOnConnectCallback", NATIVE_ONCONNECT_CALLBACK_SIGNATURE,
            reinterpret_cast<void *>(EtsServiceExtensionFFI::ConnectPromiseCallback) },
    };
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(CLASSNAME_SERVICE_ABILITY, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
        return false;
    }
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Class_BindNativeMethods is fail %{public}d", status);
        return false;
    }
    return true;
}

void EtsServiceExtensionFFI::OnStart(const AAFwk::Want &want)
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null runtime_");
        return;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return;
    }

    CallObjectMethod(false, "onCreate", ON_CREATE_SIGNATURE, wantRef);
}

void EtsServiceExtensionFFI::OnStop()
{
    CallObjectMethod(false, "onDestroy", VOID_SIGNATURE);
}

sptr<IRemoteObject> EtsServiceExtensionFFI::OnConnect(const OHOS::AAFwk::Want &want)
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null runtime_");
        return nullptr;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return nullptr;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return nullptr;
    }
    ani_ref result = CallObjectMethod(true, "onConnect", ON_CONNECT_SIGNATURE, wantRef);
    auto obj = reinterpret_cast<ani_object>(result);
    auto remoteObj = AniGetNativeRemoteObject(env, obj);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "remoteObj null");
        return nullptr;
    }
    return remoteObj;
}

sptr<IRemoteObject> EtsServiceExtensionFFI::OnConnectCallback(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    if (runtime_ == nullptr || etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null runtime_ or etsAbilityObj_");
        return nullptr;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return nullptr;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return nullptr;
    }
    ani_long connectCallbackPoint = (ani_long)callbackInfo;
    ani_status status = ANI_ERROR;
    ani_field callbackField = nullptr;
    if ((status = env->Class_FindField(etsAbilityObj_->aniCls, "connectCallbackPoint", &callbackField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_SetField_Long(etsAbilityObj_->aniObj, callbackField, connectCallbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return nullptr;
    }
    ani_ref aniRemoteRef = CallObjectMethod(true, "onConnect", ON_CONNECT_SIGNATURE, wantRef);
    auto aniRemoteobj = reinterpret_cast<ani_object>(aniRemoteRef);
    ani_method method {};
    if ((status = env->Class_FindMethod(etsAbilityObj_->aniCls, "checkPromise",
        CHECK_PROMISE_SIGNATURE, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return nullptr;
    }
    ani_boolean isPromise = ANI_FALSE;
    if ((status = env->Object_CallMethod_Boolean(etsAbilityObj_->aniObj, method, &isPromise, aniRemoteobj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return nullptr;
    }
    if (!isPromise) {
        isAsyncCallback = false;
        auto remoteObj = AniGetNativeRemoteObject(env, aniRemoteobj);
        if (remoteObj == nullptr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "null remoteObj");
        }
        return remoteObj;
    }
    return OnConnectInner(env, aniRemoteobj, isAsyncCallback);
}

sptr<IRemoteObject> EtsServiceExtensionFFI::OnConnectInner(ani_env *env, ani_object &aniRemoteobj,
    bool &isAsyncCallback)
{
    ani_status status = ANI_ERROR;
    ani_method method {};
    if ((status = env->Class_FindMethod(etsAbilityObj_->aniCls, "callPromise",
        CALL_PROMISE_SIGNATURE, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return nullptr;
    }
    ani_boolean callResult = ANI_FALSE;
    if ((status = env->Object_CallMethod_Boolean(etsAbilityObj_->aniObj, method,
        &callResult, aniRemoteobj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return nullptr;
    }
    isAsyncCallback = callResult;
    return nullptr;
}

void EtsServiceExtensionFFI::OnDisconnect(const OHOS::AAFwk::Want &want)
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null runtime_");
        return;
    }
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return;
    }
    CallObjectMethod(false, "onDisconnect", ON_DISCONNECT_SIGNATURE, wantRef);
}

void EtsServiceExtensionFFI::OnDisconnectCallback(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo,
    bool &isAsyncCallback)
{
    if (runtime_ == nullptr || etsAbilityObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null runtime_ or etsAbilityObj_");
        return;
    }
    auto env = runtime_->GetAniEnv();
    if (env) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return;
    }
    if (callbackInfo == nullptr) {
        isAsyncCallback = false;
        OnDisconnect(want);
        return;
    }
    ani_long disconnectCallbackPoint = (ani_long)callbackInfo;
    ani_status status = ANI_ERROR;
    ani_field field = nullptr;
    if ((status = env->Class_FindField(etsAbilityObj_->aniCls, "disconnectCallbackPoint", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    if ((status = env->Object_SetField_Long(etsAbilityObj_->aniObj, field, disconnectCallbackPoint)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        return;
    }
    CallObjectMethod(false, "callOnDisconnect", ON_DISCONNECT_SIGNATURE, wantRef);
}

void EtsServiceExtensionFFI::OnCommand(const AAFwk::Want &want, int startId)
{
    if (runtime_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null runtime_");
        return;
    }
    auto env = runtime_->GetAniEnv();
    if (env) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null wantRef");
        return;
    }
    ani_int iStartId = static_cast<ani_int>(startId);
    CallObjectMethod(false, "onRequest", ON_REQUEST_SIGNATURE, wantRef, iStartId);
}

ani_ref EtsServiceExtensionFFI::CallObjectMethod(bool withResult, const char *name, const char *signature, ...)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    if (runtime_ == nullptr || etsAbilityObj_ != nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null runtime_");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    auto env = runtime_->GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(etsAbilityObj_->aniCls, name, signature, &method)) != ANI_OK) {
        return nullptr;
    }
    if (method == nullptr) {
        return nullptr;
    }
    ani_ref res = nullptr;
    va_list args;
    if (withResult) {
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Ref_V(etsAbilityObj_->aniObj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
            return nullptr;
        }
        va_end(args);
        return res;
    }
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsAbilityObj_->aniObj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
    }
    va_end(args);
    return nullptr;
}

FFIHandle EtsServiceExtensionFFI::FFICreate(AbilityHandle ability, RuntimeHandle runtime)
{
    return new (std::nothrow) EtsServiceExtensionFFI(ability, runtime);
}

void EtsServiceExtensionFFI::FFIDelete(FFIHandle ffi)
{
    if (ffi != nullptr) {
        delete reinterpret_cast<EtsServiceExtensionFFI *>(ffi);
    }
}

bool EtsServiceExtensionFFI::FFILoadModule(FFIHandle ffi, const std::string &moduleName,
    const std::string &modulePath, const std::string &hapPath,
    bool esmodule, bool useCommonChunk, const std::string &srcEntrance)
{
    if (ffi == nullptr) {
        TAG_LOGI(AAFwkTag::SERVICE_EXT, "null ffi");
        return false;
    }
    EtsServiceExtensionFFI *pFFI = reinterpret_cast<EtsServiceExtensionFFI *>(ffi);
    return pFFI->LoadModule(moduleName, modulePath, hapPath, esmodule, useCommonChunk, srcEntrance);
}

void EtsServiceExtensionFFI::FFIOnStart(FFIHandle ffi, const OHOS::AAFwk::Want &want)
{
    if (ffi == nullptr) {
        TAG_LOGI(AAFwkTag::SERVICE_EXT, "null ffi");
        return;
    }
    EtsServiceExtensionFFI *pFFI = reinterpret_cast<EtsServiceExtensionFFI *>(ffi);
    pFFI->OnStart(want);
}

void EtsServiceExtensionFFI::FFIOnStop(FFIHandle ffi)
{
    if (ffi == nullptr) {
        TAG_LOGI(AAFwkTag::SERVICE_EXT, "null ffi");
        return;
    }
    EtsServiceExtensionFFI *pFFI = reinterpret_cast<EtsServiceExtensionFFI *>(ffi);
    pFFI->OnStop();
}

OHOS::sptr<OHOS::IRemoteObject> EtsServiceExtensionFFI::FFIOnConnect(FFIHandle ffi, const OHOS::AAFwk::Want &want)
{
    if (ffi == nullptr) {
        TAG_LOGI(AAFwkTag::SERVICE_EXT, "null ffi");
        return nullptr;
    }
    EtsServiceExtensionFFI *pFFI = reinterpret_cast<EtsServiceExtensionFFI *>(ffi);
    return pFFI->OnConnect(want);
}

OHOS::sptr<OHOS::IRemoteObject> EtsServiceExtensionFFI::FFIOnConnectCallback(FFIHandle ffi,
    const OHOS::AAFwk::Want &want,
    OHOS::AppExecFwk::AbilityTransactionCallbackInfo<OHOS::sptr<OHOS::IRemoteObject>> *callbackInfo,
    bool &isAsyncCallback)
{
    if (ffi == nullptr) {
        TAG_LOGI(AAFwkTag::SERVICE_EXT, "null ffi");
        return nullptr;
    }
    EtsServiceExtensionFFI *pFFI = reinterpret_cast<EtsServiceExtensionFFI *>(ffi);
    return pFFI->OnConnectCallback(want, callbackInfo, isAsyncCallback);
}

void EtsServiceExtensionFFI::FFIOnDisconnect(FFIHandle ffi, const OHOS::AAFwk::Want &want)
{
    if (ffi == nullptr) {
        TAG_LOGI(AAFwkTag::SERVICE_EXT, "null ffi");
        return;
    }
    EtsServiceExtensionFFI *pFFI = reinterpret_cast<EtsServiceExtensionFFI *>(ffi);
    return pFFI->OnDisconnect(want);
}

void EtsServiceExtensionFFI::FFIOnDisconnectCallback(FFIHandle ffi, const OHOS::AAFwk::Want &want,
    OHOS::AppExecFwk::AbilityTransactionCallbackInfo<void> *callbackInfo,
    bool &isAsyncCallback)
{
    if (ffi == nullptr) {
        TAG_LOGI(AAFwkTag::SERVICE_EXT, "null ffi");
        return;
    }
    EtsServiceExtensionFFI *pFFI = reinterpret_cast<EtsServiceExtensionFFI *>(ffi);
    return pFFI->OnDisconnectCallback(want, callbackInfo, isAsyncCallback);
}

void EtsServiceExtensionFFI::FFIOnCommand(FFIHandle ffi, const OHOS::AAFwk::Want &want, int startId)
{
    if (ffi == nullptr) {
        TAG_LOGI(AAFwkTag::SERVICE_EXT, "null ffi");
        return;
    }
    EtsServiceExtensionFFI *pFFI = reinterpret_cast<EtsServiceExtensionFFI *>(ffi);
    return pFFI->OnCommand(want, startId);
}

ETSServiceExtensionFuncs *EtsServiceExtensionFFI::RegisterFuncs()
{
    static ETSServiceExtensionFuncs funcs = {
        .CreateEtsFFI = EtsServiceExtensionFFI::FFICreate,
        .DeleteEtsFFI = EtsServiceExtensionFFI::FFIDelete,
        .LoadModule = EtsServiceExtensionFFI::FFILoadModule,
        .OnStart = EtsServiceExtensionFFI::FFIOnStart,
        .OnStop = EtsServiceExtensionFFI::FFIOnStop,
        .OnConnect = EtsServiceExtensionFFI::FFIOnConnect,
        .OnConnectCallback = EtsServiceExtensionFFI::FFIOnConnectCallback,
        .OnDisconnect = EtsServiceExtensionFFI::FFIOnDisconnect,
        .OnDisconnectCallback = EtsServiceExtensionFFI::FFIOnDisconnectCallback,
        .OnCommand = EtsServiceExtensionFFI::FFIOnCommand,
    };

    return &funcs;
}
} // namespace AbilityRuntime
} // namespace OHOS

ETS_EXPORT extern "C" ETSServiceExtensionFuncs *OHOS_ETS_Service_Extension_RegisterFuncs()
{
    return OHOS::AbilityRuntime::EtsServiceExtensionFFI::RegisterFuncs();
}