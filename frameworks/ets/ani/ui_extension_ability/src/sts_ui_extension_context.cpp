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
#include "common_fun_ani.h"
#include "sts_ui_extension_context.h"
#include "ui_extension_context.h"
#include "ani_common_want.h"
#include "ability_manager_client.h"
#include "sts_context_utils.h"
#include "sts_error_utils.h"
#include "ets_extension_context.h"
#include "ani_common_start_options.h"
#include "ani_remote_object.h"

namespace OHOS {
namespace AbilityRuntime {
const char *INVOKE_METHOD_NAME = "invoke";
namespace {
    static std::mutex g_connectsMutex;
    int64_t g_serialNumber = 0;
    static std::map<UIExtensionConnectionKey, sptr<StsUiExtensionConnection>, key_compare> g_connects;
}
static void TerminateSelfSync([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object obj,
    [[maybe_unused]] ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelfSync");
    ani_class cls = nullptr;
    ani_long nativeContextLong;
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    ErrCode ret = ERR_INVALID_VALUE;
    if ((status = env->FindClass("Lapplication/UIExtensionContext/UIExtensionContext;", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->Object_GetField_Long(obj, contextField, &nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    ret = ((UIExtensionContext*)nativeContextLong)->TerminateSelf();
    AppExecFwk::AsyncCallback(env, callback,
        CreateStsErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
}
static void TerminateSelfWithResultSync([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object obj,
    [[maybe_unused]] ani_object abilityResult, [[maybe_unused]] ani_object callback)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "TerminateSelfWithResultSync");
    ani_class cls = nullptr;
    ani_long nativeContextLong;
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    ErrCode ret = ERR_INVALID_VALUE;
    if ((status = env->FindClass("Lapplication/UIExtensionContext/UIExtensionContext;", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->Object_GetField_Long(obj, contextField, &nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return;
    }
    auto context = ((UIExtensionContext*)nativeContextLong);
    if (!context) {
        TAG_LOGE(AAFwkTag::UI_EXT, "context is released");
        return;
    }

    AAFwk::Want want;
    int resultCode = 0;
    AppExecFwk::UnWrapAbilityResult(env, abilityResult, resultCode, want);
    auto token = context->GetToken();
    AAFwk::AbilityManagerClient::GetInstance()->TransferAbilityResultForExtension(token, resultCode, want);
    ret = context->TerminateSelf();
    if (ret != 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "TerminateSelf failed, errorCode is %{public}d", ret);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback,
        CreateStsErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
}

static void StartAbility([[maybe_unused]] ani_env *env,
    [[maybe_unused]] ani_object aniObj, ani_object wantObj, ani_object call)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartAbility");
    StsUIExtensionContext::GetInstance().StartAbilityInner(env, aniObj, wantObj, nullptr, call);
}

static void StartAbilityWithOption([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object aniObj,
    ani_object wantObj, ani_object opt, ani_object call)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "StartAbilityWithOption");
    StsUIExtensionContext::GetInstance().StartAbilityInner(env, aniObj, wantObj, opt, call);
}

ani_int StsUIExtensionContext::ConnectServiceExtensionAbility([[maybe_unused]]ani_env *env,
    [[maybe_unused]]ani_object aniObj, ani_object wantObj, ani_object connectOptionsObj)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ConnectServiceExtensionAbility call");
    auto context = StsUIExtensionContext::GetAbilityContext(env, aniObj);
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "UnwrapWant failed");
        ThrowStsInvalidParamError(env, "UnwrapWant failed");
    }
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status : %{public}d", status);
    }
    sptr<StsUiExtensionConnection> connection = new StsUiExtensionConnection(etsVm);
    if (!CheckConnectionParam(env, connectOptionsObj, connection, want)) {
        TAG_LOGE(AAFwkTag::UIABILITY, "CheckConnectionParam failed");
        ThrowStsInvalidParamError(env, "CheckConnectionParam failed");
    }
    auto innerErrCode = context->ConnectAbility(want, connection);
    if (innerErrCode) {
        connection->CallStsFailed(innerErrCode);
    }
    return innerErrCode;
}

void StsUIExtensionContext::DisconnectServiceExtensionAbilitySync([[maybe_unused]]ani_env *env,
    [[maybe_unused]]ani_object aniObj, ani_int connectId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "ConnectServiceExtensionAbility call");
    auto context = StsUIExtensionContext::GetAbilityContext(env, aniObj);
    AAFwk::Want want;
    ani_vm *etsVm = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GetVM(&etsVm)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status : %{public}d", status);
    }
 
    sptr<StsUiExtensionConnection> connection = new StsUiExtensionConnection(etsVm);

    {
        std::lock_guard<std::mutex> lock(g_connectsMutex);
        auto item = std::find_if(g_connects.begin(),
            g_connects.end(),
            [&connectId](const auto &obj) {
                return connectId == obj.first.id;
            });
        if (item != g_connects.end()) {
            want = item->first.want;
            connection = item->second;
            g_connects.erase(item);
        } else {
            TAG_LOGI(AAFwkTag::CONTEXT, "not find conn exist");
        }
    }
    context->DisconnectAbility(want, connection);
    ani_object aniObject = nullptr;
    if (!connection) {
        aniObject = CreateStsErrorByNativeErr(env, static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT));
    }
    AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
}

UIExtensionContext* StsUIExtensionContext::GetAbilityContext(ani_env *env, ani_object obj)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "GetAbilityContext start");
    ani_class cls = nullptr;
    ani_long nativeContextLong;
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return nullptr;
    }
    if ((status = env->FindClass("Lapplication/UIExtensionContext/UIExtensionContext;", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetAbilityContext find class status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetAbilityContext find field status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(obj, contextField, &nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetAbilityContext get filed status: %{public}d", status);
        return nullptr;
    }
    return (UIExtensionContext*)nativeContextLong;
}

void StsUIExtensionContext::AddFreeInstallObserver(ani_env *env, const AAFwk::Want &want,
    ani_object callback, UIExtensionContext*context)
{
    // adapter free install async return install and start result
    TAG_LOGD(AAFwkTag::UI_EXT, "AddFreeInstallObserver");
    int ret = 0;
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return;
    }
    if (freeInstallObserver_ == nullptr) {
        ani_vm *etsVm = nullptr;
        ani_status status = ANI_ERROR;
        if ((status = env->GetVM(&etsVm)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::STSRUNTIME, "status: %{public}d", status);
        }
        freeInstallObserver_ = new StsFreeInstallObserver(etsVm);
        ret = context->AddFreeInstallObserver(freeInstallObserver_);
    }
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "addFreeInstallObserver error");
    }
    std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
    TAG_LOGI(AAFwkTag::UI_EXT, "addStsObserver");
    std::string bundleName = want.GetElement().GetBundleName();
    std::string abilityName = want.GetElement().GetAbilityName();
    freeInstallObserver_->AddStsObserverObject(
        env, bundleName, abilityName, startTime, callback);
}

void StsUIExtensionContext::StartAbilityInner([[maybe_unused]] ani_env *env, [[maybe_unused]] ani_object aniObj,
    ani_object wantObj, ani_object opt, ani_object call)
{
    ani_object aniObject = nullptr;
    AAFwk::Want want;
    ErrCode innerErrCode = ERR_OK;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        aniObject = CreateStsInvalidParamError(env, "UnwrapWant filed");
        AppExecFwk::AsyncCallback(env, call, aniObject, nullptr);
        return;
    }
    auto context = StsUIExtensionContext::GetAbilityContext(env, aniObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "GetAbilityContext is nullptr");
        innerErrCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        aniObject = CreateStsError(env, static_cast<AbilityErrorCode>(innerErrCode));
        AppExecFwk::AsyncCallback(env, call, aniObject, nullptr);
        return;
    }
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
            system_clock::now().time_since_epoch()).count());
        want.SetParam(AAFwk::Want::PARAM_RESV_START_TIME, startTime);
        AddFreeInstallObserver(env, want, call, context);
    }
    if (opt != nullptr) {
        AAFwk::StartOptions startOptions;
        if (!AppExecFwk::UnwrapStartOptionsWithProcessOption(env, opt, startOptions)) {
            TAG_LOGE(AAFwkTag::UI_EXT, "UnwrapStartOptions filed");
            aniObject = CreateStsInvalidParamError(env, "UnwrapWant filed");
            AppExecFwk::AsyncCallback(env, call, aniObject, nullptr);
            return;
        }
        innerErrCode = context->StartAbility(want, startOptions);
    } else {
        innerErrCode = context->StartAbility(want);
    }
    aniObject = CreateStsError(env, AbilityErrorCode::ERROR_OK);
    if (innerErrCode != ERR_OK) {
        aniObject = CreateStsErrorByNativeErr(env, innerErrCode);
    }
    if ((want.GetFlags() & AAFwk::Want::FLAG_INSTALL_ON_DEMAND) == AAFwk::Want::FLAG_INSTALL_ON_DEMAND) {
        if (innerErrCode != ERR_OK && freeInstallObserver_ != nullptr) {
            std::string bundleName = want.GetElement().GetBundleName();
            std::string abilityName = want.GetElement().GetAbilityName();
            std::string startTime = want.GetStringParam(AAFwk::Want::PARAM_RESV_START_TIME);
            freeInstallObserver_->OnInstallFinished(bundleName, abilityName, startTime, innerErrCode);
        }
    } else {
        AppExecFwk::AsyncCallback(env, call, aniObject, nullptr);
    }
}

bool StsUIExtensionContext::CheckConnectionParam([[maybe_unused]]ani_env *env, ani_object connectOptionsObj,
    sptr<StsUiExtensionConnection>& connection, AAFwk::Want& want)
{
    ani_type type;
    ani_boolean res;
    ani_status status = ANI_ERROR;
    status = env->Object_GetType(connectOptionsObj, &type);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Object_GetType failed");
        return false;
    }
    status = env->Object_InstanceOf(connectOptionsObj, type, &res);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Object_InstanceOf failed");
        return false;
    }
    if (res != ANI_TRUE) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "CheckConnectionParam failed");
        return false;
    }
    connection->SetConnectionRef(connectOptionsObj);
    UIExtensionConnectionKey key;
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
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Unable to find connection, make new one");
    return true;
}

bool BindNativeMethods(ani_env *env, ani_class &cls)
{
    ani_status status = ANI_ERROR;
    std::array functions = {
        ani_native_function { "terminateSelfSync", nullptr, reinterpret_cast<ani_int*>(TerminateSelfSync) },
        ani_native_function { "terminateSelfWithResultSync", nullptr,
            reinterpret_cast<ani_int*>(TerminateSelfWithResultSync) },
        ani_native_function { "nativeStartAbilitySync",
            "L@ohos/app/ability/Want/Want;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void*>(StartAbility) },
        ani_native_function { "nativeStartAbilitySync",
            "L@ohos/app/ability/Want/Want;L@ohos/app/ability/StartOptions/StartOptions;Lutils/AbilityUtils/"
            "AsyncCallbackWrapper;:V",
            reinterpret_cast<void*>(StartAbilityWithOption) },
        ani_native_function { "nativeConnectServiceExtensionAbility",
            nullptr, reinterpret_cast<void*>(StsUIExtensionContext::ConnectServiceExtensionAbility) },
        ani_native_function { "nativeDisconnectServiceExtensionAbilitySync",
            nullptr, reinterpret_cast<void*>(StsUIExtensionContext::DisconnectServiceExtensionAbilitySync) },
    };
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return false;
    }
    return true;
}

ani_object CreateStsUIExtensionContext(ani_env *env, std::shared_ptr<UIExtensionContext> context,
    const std::shared_ptr<AppExecFwk::OHOSApplication> &application)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "CreateStsUIExtensionContext");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null env");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_field field = nullptr;
    ani_object contextObj = nullptr;
    if ((env->FindClass("Lapplication/UIExtensionContext/UIExtensionContext;", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if (!BindNativeMethods(env, cls)) {
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &contextObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    ani_long nativeContextLong = (ani_long)context.get();
    if ((status = env->Object_SetField_Long(contextObj, field, nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if (application == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "application null");
        return nullptr;
    }
    OHOS::AbilityRuntime::ContextUtil::StsCreatContext(env, cls, contextObj,
        application->GetApplicationCtxObjRef(), context);
    OHOS::AbilityRuntime::CreatEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());
    return contextObj;
}

StsUiExtensionConnection::StsUiExtensionConnection(ani_vm *etsVm) : etsVm_(etsVm) {}

StsUiExtensionConnection::~StsUiExtensionConnection()
{
    if (etsVm_ != nullptr && stsConnectionRef_ != nullptr) {
        ani_env* env = nullptr;
        if (etsVm_->GetEnv(ANI_VERSION_1, &env) == ANI_OK) {
            env->GlobalReference_Delete(stsConnectionRef_);
            stsConnectionRef_ = nullptr;
        }
    }
}

void StsUiExtensionConnection::SetConnectionId(int64_t id)
{
    connectionId_ = id;
}

void StsUiExtensionConnection::CallStsFailed(int32_t errorCode)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "called");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "etsVm_ is nullptr");
        return;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_OK;
    
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status : %{public}d", status);
        return;
    }
    
    if (stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null stsConnectionRef_");
        return;
    }

    ani_class cls;
    if ((status = env->FindClass("Lability/connectOptions/ConnectOptionsInner;", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status : %{public}d", status);
        return;
    }

    ani_method function;
    if ((status = env->Class_FindMethod(cls, "onFailed", "D:V", &function))) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Class_FindMethod status: %{public}d", status);
        return;
    }
 
    status = env->Object_CallMethod_Void(reinterpret_cast<ani_object>(stsConnectionRef_),
        function, static_cast<double>(errorCode));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_CallMethod_Void status: %{public}d", status);
        return;
    }
    return;
}

void StsUiExtensionConnection::SetConnectionRef(ani_object connectOptionsObj)
{
    TAG_LOGI(AAFwkTag::CONTEXT, "called");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "etsVm_ is nullptr");
        return;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status : %{public}d", status);
        return;
    }
    if ((status = env->GlobalReference_Create(connectOptionsObj, &stsConnectionRef_)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "status: %{public}d", status);
    }
}

void StsUiExtensionConnection::OnAbilityConnectDone(const AppExecFwk::ElementName &element,
    const sptr<IRemoteObject> &remoteObject, int resultCode)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "resultCode:%{public}d", resultCode);

    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "etsVm_ is nullptr");
        return;
    }

    if (stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null stsConnectionRef_");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if ((etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "status : %{public}d", status);
        return;
    }

    ani_class cls;
    if ((status = env->FindClass("Lability/connectOptions/ConnectOptionsInner;", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status : %{public}d", status);
        return;
    }
    ani_method function;
    if ((status = env->Class_FindMethod(cls, "onConnect",
        nullptr, &function))) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Class_FindMethod status: %{public}d", status);
        return;
    }

    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    ani_object refRemoteObject = ANI_ohos_rpc_CreateJsRemoteObject(env, remoteObject);
    status = env->Object_CallMethod_Void(reinterpret_cast<ani_object>(stsConnectionRef_), function, refElement,
        refRemoteObject);
    if (etsVm_->DetachCurrentThread() != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "status : %{public}d", status);
    }
        if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_CallMethod_Void status: %{public}d", status);
        return;
    }
    return;
}

void StsUiExtensionConnection::OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "resultCode:%{public}d", resultCode);

    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "etsVm_ is nullptr");
        return;
    }

    if (stsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null stsConnectionRef_");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if ((etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "status : %{public}d", status);
        return;
    }

    ani_class cls;
    if ((status = env->FindClass("Lability/connectOptions/ConnectOptionsInner;", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status : %{public}d", status);
        return;
    }
    ani_method function;
    if ((status = env->Class_FindMethod(cls, "onDisconnect",
        nullptr, &function))) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Class_FindMethod status: %{public}d", status);
        return;
    }

    ani_ref refElement = AppExecFwk::WrapElementName(env, element);
    status = env->Object_CallMethod_Void(reinterpret_cast<ani_object>(stsConnectionRef_), function, refElement);
    if (etsVm_->DetachCurrentThread() != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "status : %{public}d", status);
    }
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_CallMethod_Void status: %{public}d", status);
        return;
    }
    return;
}
} // AbilityRuntime
} // OHOS
