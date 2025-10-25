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

#include "ets_start_abilities_observer.h"

#include "hilog_tag_wrapper.h"
namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const int32_t ERR_OK = 0;
constexpr const char *BUSINESS_ERROR_CLASS = "L@ohos/base/BusinessError;";
constexpr const char* CLASSNAME_ASYNC_CALLBACK_WRAPPER = "Lutils/AbilityUtils/AsyncCallbackWrapper;";
constexpr const char *ERROR_CLASS_NAME = "Lescompat/Error;";
}

EtsStartAbilitiesObserver &EtsStartAbilitiesObserver::GetInstance()
{
    static EtsStartAbilitiesObserver instance;
    return instance;
}

EtsStartAbilitiesObserver::~EtsStartAbilitiesObserver()
{
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null etsVm_");
        return;
    }
    if ((etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "status: %{public}d", status);
        return;
    }
    for (auto it = etsObserverObjectList_.begin(); it != etsObserverObjectList_.end();) {
        env->GlobalReference_Delete(it->second);
        it++;
    }
    if ((status = etsVm_->DetachCurrentThread()) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "status: %{public}d", status);
    }
}

void EtsStartAbilitiesObserver::AddObserver(ani_env *env, const std::string &requestKey, ani_object callback)
{
    std::lock_guard lock(etsObserverObjectListLock_);
    if (env == nullptr) {
        return;
    }
    if (etsVm_ == nullptr) {
        ani_status status = ANI_ERROR;
        if ((status = env->GetVM(&etsVm_)) != ANI_OK || etsVm_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITY, "status: %{public}d", status);
        }
    }
    ani_ref global = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GlobalReference_Create(callback, &global)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "status: %{public}d", status);
        return;
    }
    etsObserverObjectList_.emplace(requestKey, global);
}

void EtsStartAbilitiesObserver::HandleFinished(const std::string &requestKey, int32_t resultCode)
{
    ani_ref callbackRef = nullptr;
    if (etsVm_ != nullptr) {
        {
            std::lock_guard lock(etsObserverObjectListLock_);
            auto it = etsObserverObjectList_.find(requestKey);
            if (it != etsObserverObjectList_.end()) {
                callbackRef = std::move(it->second);
                etsObserverObjectList_.erase(it);
            }
        }
    }
    if (callbackRef) {
        CallCallback(static_cast<ani_object>(callbackRef), resultCode);
        return;
    }
    TAG_LOGE(AAFwkTag::ABILITY, "null callback");
}

void EtsStartAbilitiesObserver::CallCallback(ani_object callback, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::ABILITY, "CallCallback");
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null callback");
        return;
    }
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null etsVm");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "Failed to getEnv, status: %{public}d", status);
        return;
    }
    ani_object aniObject = CreateError(env, AbilityErrorCode::ERROR_OK);
    if (resultCode != ERR_OK) {
        aniObject = CreateErrorByNativeErr(env, resultCode);
    }
    AsyncCallback(env, callback, aniObject, nullptr);
    env->GlobalReference_Delete(callback);
}

ani_object EtsStartAbilitiesObserver::CreateError(ani_env *env, const AbilityErrorCode &err)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null env");
        return nullptr;
    }
    return EtsStartAbilitiesObserver::CreateError(env, static_cast<int32_t>(err), GetErrorMsg(err));
}
ani_object EtsStartAbilitiesObserver::CreateError(ani_env *env, ani_int code, const std::string &msg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null env");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_class cls = nullptr;
    if ((status = env->FindClass(BUSINESS_ERROR_CLASS, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass failed %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "ILescompat/Error;:V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "Class_FindMethod failed %{public}d", status);
        return nullptr;
    }
    ani_object error = WrapError(env, msg);
    if (error == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "error nulll");
        return nullptr;
    }
    ani_object obj = nullptr;
    if ((status = env->Object_New(cls, method, &obj, code, error)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "Object_New failed %{public}d", status);
        return nullptr;
    }
    return obj;
}

ani_object EtsStartAbilitiesObserver::CreateErrorByNativeErr(ani_env *env, int32_t err, const std::string &permission)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "null env");
        return nullptr;
    }
    auto errCode = GetJsErrorCodeByNativeError(err);
    auto errMsg = (errCode == AbilityErrorCode::ERROR_CODE_PERMISSION_DENIED && !permission.empty())
                      ? GetNoPermissionErrorMsg(permission)
                      : GetErrorMsg(errCode);
    return CreateError(env, static_cast<int32_t>(errCode), errMsg);
}

bool EtsStartAbilitiesObserver::AsyncCallback(ani_env *env, ani_object call, ani_object error, ani_object result)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return false;
    }
    ani_class clsCall = nullptr;
    ani_status status = env->FindClass(CLASSNAME_ASYNC_CALLBACK_WRAPPER, &clsCall);
    if (status!= ANI_OK || clsCall == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass status: %{public}d, or null clsCall", status);
        return false;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(clsCall, "invoke", nullptr, &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "Class_FindMethod status: %{public}d, or null method", status);
        return false;
    }
    if (error == nullptr) {
        ani_ref nullRef = nullptr;
        env->GetNull(&nullRef);
        error = reinterpret_cast<ani_object>(nullRef);
    }
    if (result == nullptr) {
        ani_ref undefinedRef = nullptr;
        env->GetUndefined(&undefinedRef);
        result = reinterpret_cast<ani_object>(undefinedRef);
    }
    if ((status = env->Object_CallMethod_Void(call, method, error, result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_CallMethod_Void status: %{public}d", status);
        return false;
    }
    return true;
}

ani_object EtsStartAbilitiesObserver::WrapError(ani_env *env, const std::string &msg)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null env");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_string aniMsg = nullptr;
    if ((status = env->String_NewUTF8(msg.c_str(), msg.size(), &aniMsg)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "String_NewUTF8 failed %{public}d", status);
        return nullptr;
    }
    ani_ref undefRef;
    if ((status = env->GetUndefined(&undefRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetUndefined failed %{public}d", status);
        return nullptr;
    }
    ani_class cls = nullptr;
    if ((status = env->FindClass(ERROR_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "FindClass failed %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", "Lstd/core/String;Lescompat/ErrorOptions;:V", &method)) !=
        ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Class_FindMethod failed %{public}d", status);
        return nullptr;
    }
    ani_object obj = nullptr;
    if ((status = env->Object_New(cls, method, &obj, aniMsg, undefRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Object_New failed %{public}d", status);
        return nullptr;
    }
    return obj;
}

} // namespace AbilityRuntime
} // namespace OHOS