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

#include "ets_app_service_extension_context.h"

#include <chrono>
#include <cstdint>
#include <thread>
#include "ability_manager_client.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ets_context_utils.h"
#include "ets_data_struct_converter.h"
#include "ets_error_utils.h"
#include "ets_extension_context.h"
#include "ets_runtime.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "js_runtime_utils.h"
#include "napi_common_ability.h"
#include "napi_common_start_options.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#include "open_link_options.h"
#include "start_options.h"
#include "uri.h"
#include "ability_runtime/js_caller_complex.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
    constexpr const char *CONTEXT_CLASS_NAME = "Lapplication/AppServiceExtensionContext/AppServiceExtensionContext;";
    static std::mutex g_connectsMutex;
    static std::recursive_mutex g_connectsLock;
    static std::map<EtsConnectionKey, std::shared_ptr<EtsAppServiceExtensionContext>, EtsKeyCompare> g_connects;
    static int64_t g_serialNumber = 0;
    constexpr const int FAILED_CODE = -1;
}

void EtsAppServiceExtensionContext::Finalizer(ani_env *env, void *data, void *hint)
{
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "EtsAppServiceExtensionContext::Finalizer called");
    std::unique_ptr<EtsAppServiceExtensionContext>(static_cast<EtsAppServiceExtensionContext*>(data));
}

void EtsAppServiceExtensionContext::SetConnectionId(int32_t id)
{
    connectionId_ = id;
}

int32_t EtsAppServiceExtensionContext::InsertConnection(std::shared_ptr<EtsAppServiceExtensionContext> connection,
    const AAFwk::Want &want, int32_t accountId)
{
    std::lock_guard<std::recursive_mutex> lock(g_connectsLock);
    if (connection == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null connection");
        return FAILED_CODE;
    }
    int32_t connectId = static_cast<int32_t>(g_serialNumber);
    EtsConnectionKey key;
    key.id = g_serialNumber;
    key.want = want;
    key.accountId = accountId;
    connection->SetConnectionId(key.id);
    g_connects.emplace(key, connection);
    g_serialNumber++;
    return connectId;
}

void EtsAppServiceExtensionContext::RemoveConnection(int32_t connectId)
{
    std::lock_guard<std::recursive_mutex> lock(g_connectsLock);
    auto item = std::find_if(g_connects.begin(), g_connects.end(),
    [&connectId](const auto &obj) {
        return connectId == obj.first.id;
    });
    if (item != g_connects.end()) {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "remove connection ability exist");
        if (item->second) {
            item->second->RemoveConnectionObject();
        }
        g_connects.erase(item);
    } else {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "remove connection ability not exist");
    }
}

void EtsAppServiceExtensionContext::RemoveConnectionObject()
{
    if (etsVm_ != nullptr && etsConnectionRef_ != nullptr) {
        ani_env *env = nullptr;
        if (etsVm_->GetEnv(ANI_VERSION_1, &env) == ANI_OK && env != nullptr) {
            env->GlobalReference_Delete(etsConnectionRef_);
            etsConnectionRef_ = nullptr;
        }
    }
}

void EtsAppServiceExtensionContext::CallEtsFailed(int32_t errorCode)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "CallEtsFailed");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsVm");
        return;
    }
    if (etsConnectionRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null etsConnectionRef_");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_OK;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to get env, status: %{public}d", status);
        return;
    }
    status = env->Object_CallMethodByName_Void(reinterpret_cast<ani_object>(etsConnectionRef_), "onFailed", "D:V",
        static_cast<double>(errorCode));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to call onFailed, status: %{public}d", status);
    }
}

void EtsAppServiceExtensionContext::OnTerminateSelf(ani_env *env, ani_object obj, ani_object callback)
{
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "called");
    if (env == nullptr || callback == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Invalid environment or callback");
        return;
    }
    auto context = GetContext(env, obj);
    if (!context) {
        TAG_LOGW(AAFwkTag::SERVICE_EXT, "context released");
        ani_object error = EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(AAFwk::ERR_INVALID_CONTEXT));
        AppExecFwk::AsyncCallback(env, callback, error, nullptr);
        return;
    }
    std::thread([env, callback, context]() {
        int32_t errCode = 0;
        ani_object result = EtsErrorUtil::CreateErrorByNativeErr(env, errCode);
        AppExecFwk::AsyncCallback(env, callback, result, nullptr);
    }).detach();
}

void EtsAppServiceExtensionContext::SetConnectionRef(ani_object connectOptionsObj)
{
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "etsVm_ is nullptr");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        return;
    }
    if ((status = env->GlobalReference_Create(connectOptionsObj, &etsConnectionRef_)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
    }
}

ani_int EtsAppServiceExtensionContext::OnConnectAbility(ani_env *env, ani_object aniObj, ani_object wantObj,
    ani_object connectOptionsObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnConnectAbility call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return FAILED_CODE;
    }
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to UnwrapWant");
        EtsErrorUtil::ThrowInvalidParamError(env, "Failed to UnwrapWant");
        return FAILED_CODE;
    }
    std::shared_ptr<AppServiceExtensionContext> sharedContext(GetContext(env, aniObj));
    auto connection = std::make_shared<EtsAppServiceExtensionContext>(sharedContext);
    if (!connection) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to create EtsAppServiceExtensionContext instance");
        return -1;
    }
    connection->SetConnectionRef(connectOptionsObj);
    int32_t connectId = InsertConnection(connection, want);
    if (connectId < 0) {
        RemoveConnection(connectId);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return -1;
    }
    return connectId;
}

void EtsAppServiceExtensionContext::OnDisconnectAbility(ani_env *env, ani_object aniObj, ani_int connectId,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "OnDisconnectAbility call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    auto context = GetContext(env, aniObj);
    ani_object errorObject = nullptr;
    ErrCode errCode = ERR_OK;
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        errorObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(errCode));
        AppExecFwk::AsyncCallback(env, callback, errorObject, nullptr);
        return;
    }
    std::shared_ptr<EtsAppServiceExtensionContext> connection = nullptr;
    AAFwk::Want want;
    int32_t accountId = -1;
    {
        std::lock_guard<std::mutex> lock(g_connectsMutex);
        auto iter = std::find_if(
            g_connects.begin(), g_connects.end(), [&connectId](const auto &obj) { return connectId == obj.first.id; });
        if (iter != g_connects.end()) {
            want = iter->first.want;
            connection = iter->second;
            accountId = iter->first.accountId;
            g_connects.erase(iter);
        } else {
            TAG_LOGI(AAFwkTag::SERVICE_EXT, "Failed to found connection");
        }
    }
    if (!connection) {
        errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        errorObject = EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(errCode));
        AppExecFwk::AsyncCallback(env, callback, errorObject, nullptr);
        return;
    }
    errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateError(env,
        static_cast<AbilityErrorCode>(errCode)), nullptr);
}

ani_object EtsAppServiceExtensionContext::OnStartAbility(ani_env* env, ani_object aniObj, ani_object wantObj,
    ani_object optionsObj)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartAbility");
    auto context = GetContext(env, aniObj);
    ErrCode errCode = ERR_OK;
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(errCode));
    }
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "parse want failed");
        return EtsErrorUtil::CreateInvalidParamError(env, "Parse param want failed, want must be Want.");
    }
    AAFwk::StartOptions startOptions;
    if (optionsObj != nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "parse startOptions failed");
        return EtsErrorUtil::CreateInvalidParamError(env, "Parse param startOptions failed.");
    }
    return StartAbilityInner(env, want, startOptions, optionsObj);
}

AppServiceExtensionContext *EtsAppServiceExtensionContext::GetContext(ani_env *env, ani_object aniObj)
{
    ani_long nativeContextLong;
    ani_class cls {};
    ani_field contextField = nullptr;
    ani_status status = ANI_ERROR;
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null env");
        return nullptr;
    }
    if ((status = env->FindClass(CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_GetField_Long(aniObj, contextField, &nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
        return nullptr;
    }
    return reinterpret_cast<AppServiceExtensionContext *>(nativeContextLong);
}

ani_object EtsAppServiceExtensionContext::StartAbilityInner(ani_env* env, AAFwk::Want& want,
    AAFwk::StartOptions& startOptions, ani_object optionsObj)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "StartAbilityInner called");
    auto context = GetContext(env, optionsObj);
    ErrCode errCode = ERR_OK;
    if (!context) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(errCode));
    }
    errCode = static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
    return EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(errCode));
}

ani_object CreateEtsAppServiceExtensionContext(ani_env *env, std::shared_ptr<AppServiceExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "CreateEtsAppServiceExtensionContext called");
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    ani_object contextObj = nullptr;
    if ((env->FindClass(CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", "J:V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
        return nullptr;
    }
    std::unique_ptr<EtsAppServiceExtensionContext> etsContext =
        std::make_unique<EtsAppServiceExtensionContext>(context);
    if ((status = env->Object_New(cls, method, &contextObj,
        reinterpret_cast<ani_long>(etsContext.release()))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
        return nullptr;
    }
    std::array AppServiceExtensionContext = {
        ani_native_function { "nativeConnectServiceExtensionAbility",
            "L@ohos/app/ability/Want/Want;Lability/ConnectOptions/ConnectOptions;:D",
            reinterpret_cast<ani_int*>(EtsAppServiceExtensionContext::OnConnectAbility) },
        ani_native_function { "nativeDisconnectServiceExtensionAbility",
            "D;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void*>(EtsAppServiceExtensionContext::OnDisconnectAbility) },
        ani_native_function { "nativeStartAbility",
            "L@ohos/app/ability/Want/Want;L@ohos/app/ability/StartOptions/StartOptions;"
            "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<ani_object *>(EtsAppServiceExtensionContext::OnStartAbility) },
        ani_native_function { "nativeTerminateSelf",
            "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsAppServiceExtensionContext::OnTerminateSelf) },
    };
    if ((status = env->Class_BindNativeMethods(cls, AppServiceExtensionContext.data(),
        AppServiceExtensionContext.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_SetFieldByName_Long(contextObj, "nativeContext", (ani_long)context.get())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "status: %{public}d", status);
        return nullptr;
    }
    OHOS::AbilityRuntime::ContextUtil::CreateEtsBaseContext(env, cls, contextObj, context);
    OHOS::AbilityRuntime::CreateEtsExtensionContext(env, cls, contextObj, context, context->GetAbilityInfo());
    return contextObj;
}
}  // namespace AbilityRuntime
}  // namespace OHOS