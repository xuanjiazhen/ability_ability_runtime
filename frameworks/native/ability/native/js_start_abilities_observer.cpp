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

#include "js_start_abilities_observer.h"

#include "hilog_tag_wrapper.h"
 
namespace OHOS {
namespace AbilityRuntime {
JsStartAbilitiesObserver &JsStartAbilitiesObserver::GetInstance()
{
    static JsStartAbilitiesObserver instance;
    return instance;
}

JsStartAbilitiesObserver::~JsStartAbilitiesObserver(){
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "null etsVm_");
        return;
    }
    if ((etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "status: %{public}d", status);
        return;
    }
    for (auto it = etsObserverObjectList_.begin(); it != etsObserverObjectList_.end();) {
        env->GlobalReference_Delete(it->second);
        it++;
    }
    if ((status = etsVm_->DetachCurrentThread()) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "status: %{public}d", status);
    }
}

void JsStartAbilitiesObserver::AddObserver(const std::string &requestKey, std::function<void(int32_t)> &&callback)
{
    std::lock_guard lock(jsObserverObjectListLock_);
    jsObserverObjectList_.emplace(requestKey, std::move(callback));
}

void JsStartAbilitiesObserver::AddEtsObserver(ani_env *env, const std::string &requestKey, ani_object callback)
{
    std::lock_guard lock(jsObserverObjectListLock_);
    if (env == nullptr) {
        return;
    }
    if (etsVm_ == nullptr) {
        ani_status status = ANI_ERROR;
        if ((status = env->GetVM(&etsVm_)) != ANI_OK || etsVm_ == nullptr) {
            TAG_LOGE(AAFwkTag::CONTEXT, "status: %{public}d", status);
        }
    }
    ani_ref global = nullptr;
    if ((status = env->GlobalReference_Create(callback, &global)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status: %{public}d", status);
        return;
    }
    etsObserverObjectList_.emplace(requestKey, global);
}

void JsStartAbilitiesObserver::CallCallback(ani_object callback, int32_t resultCode)
{
    TAG_LOGD(AAFwkTag::FREE_INSTALL, "CallCallback");
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "null callback");
        return;
    }
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "null etsVm");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if ((status = etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "Failed to getEnv, status: %{public}d", status);
    } else if ((status = etsVm_->GetEnv(ANI_VERSION_1, &env)) != ANI_OK || env == nullptr) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "Failed to getEnv, status: %{public}d", status);
        return;
    }
    ani_object aniObject = EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK);
    if (resultCode != ERR_OK) {
        aniObject = EtsErrorUtil::CreateErrorByNativeErr(env, resultCode);
    }
    AppExecFwk::AsyncCallback(env, callback, aniObject, nullptr);
    env->GlobalReference_Delete(callback);
    if ((status = etsVm_->DetachCurrentThread()) != ANI_OK) {
        TAG_LOGE(AAFwkTag::FREE_INSTALL, "status: %{public}d", status);
    }
}

void JsStartAbilitiesObserver::HandleFinished(const std::string &requestKey, int32_t resultCode)
{
    std::function<void(int32_t)> callback;
    {
        std::lock_guard lock(jsObserverObjectListLock_);
        auto it = jsObserverObjectList_.find(requestKey);
        if (it != jsObserverObjectList_.end()) {
            callback = std::move(it->second);
            jsObserverObjectList_.erase(it);
        }
    }
    if (callback) {
        callback(resultCode);
        return;
    }
    ani_ref callbackRef = nullptr;
    if (etsVm_ != nullptr) {
        {
            std::lock_guard lock(jsObserverObjectListLock_);
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
} // namespace AbilityRuntime
} // namespace OHOS