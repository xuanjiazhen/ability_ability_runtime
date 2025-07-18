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

#include "ets_ability_foreground_state_observer.h"

#include "ani_common_util.h"
#include "ets_ability_manager_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* ABILITY_FOREGROUND_STATE_OBSERVER_CLASS_NAME =
    "Lapplication/AbilityForegroundStateObserver/AbilityForegroundStateObserver;";
constexpr const char *SIGNATURE_ABILITY_STATE_DATA = "Lapplication/AbilityStateData/AbilityStateData;:V";
}

ETSAbilityForegroundStateObserver::ETSAbilityForegroundStateObserver(ani_vm *etsVm) : etsVm_(etsVm) {}

ani_status ETSAbilityForegroundStateObserver::AniSendEvent(const std::function<void()> task)
{
    TAG_LOGD(AAFwkTag::APPMGR, "AniSendEvent");
    if (task == nullptr) {
        TAG_LOGD(AAFwkTag::APPMGR, "null task");
        return ani_status::ANI_INVALID_ARGS;
    }
    if (!mainHandler_) {
        std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner = OHOS::AppExecFwk::EventRunner::GetMainEventRunner();
        if (!runner) {
            TAG_LOGD(AAFwkTag::APPMGR, "null EventRunner");
            return ani_status::ANI_NOT_FOUND;
        }
        mainHandler_ = std::make_shared<OHOS::AppExecFwk::EventHandler>(runner);
    }
    if (mainHandler_ == nullptr) {
        TAG_LOGD(AAFwkTag::APPMGR, "null mainHandler");
        return ani_status::ANI_NOT_FOUND;
    }
    mainHandler_->PostTask(std::move(task));
    return ani_status::ANI_OK;
}

void ETSAbilityForegroundStateObserver::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid appms");
        return;
    }
    wptr<ETSAbilityForegroundStateObserver> weakPtr = this;
    auto task = [&abilityStateData, weakPtr] () {
        auto abilityForegroundStateObserver = weakPtr.promote();
        if (abilityForegroundStateObserver == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null observerPtr");
            return;
        }
        abilityForegroundStateObserver->HandleOnAbilityStateChanged(abilityStateData);
    };
    if (AniSendEvent(task) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to aniSendEvent");
    }
    TAG_LOGD(AAFwkTag::APPMGR, "OnAbilityStateChanged end");
}

void ETSAbilityForegroundStateObserver::CallStsFunction(ani_env* env, ani_object stsObserverObject,
    const char *methodName, const char *signature, ...)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call method:%{public}s", methodName);
    ani_class cls;
    ani_method method = nullptr;
    ani_status status = ANI_OK;
    if ((status = env->FindClass(ABILITY_FOREGROUND_STATE_OBSERVER_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "find observer failed status : %{public}d", status);
        return;
    }
    if ((status = env->Class_FindMethod(cls, methodName, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "status: %{public}d", status);
        return;
    }
    env->ResetError();
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(stsObserverObject, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "status: %{public}d", status);
        return;
    }
    va_end(args);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "end");
    return;
}

void ETSAbilityForegroundStateObserver::HandleOnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "HandleOnAbilityStateChanged called");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "etsVm_ nullptr");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    ani_option interopEnabled { "--interop=disable", nullptr };
    ani_options aniArgs { 1, &interopEnabled };
    if ((status = etsVm_->AttachCurrentThread(&aniArgs, ANI_VERSION_1, &env)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "status : %{public}d", status);
        return;
    }
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "env nullptr");
        return;
    }
    for (auto &item : etsObserverObjects_) {
        auto abilityStateDataObj = AbilityManagerEts::WrapAbilityStateData(env, abilityStateData);
        if (abilityStateDataObj != nullptr && item.aniRef != nullptr) {
            CallStsFunction(env, reinterpret_cast<ani_object>(item.aniRef),
                "onAbilityStateChanged", SIGNATURE_ABILITY_STATE_DATA, abilityStateDataObj);
        }
    }
    if (etsVm_->DetachCurrentThread() != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "status : %{public}d", status);
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "HandleOnAbilityStateChanged end");
}

void ETSAbilityForegroundStateObserver::AddEtsObserverObject(ani_env *env, ani_object etsObserverObject)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AddEtsObserverObject called");
    if (etsObserverObject == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return;
    }
    if (GetObserverObject(etsObserverObject).aniObj != nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "observer exist");
        return;
    }
    ani_ref global = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GlobalReference_Create(etsObserverObject, &global)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "status : %{public}d", status);
        return;
    }
    AppExecFwk::ETSNativeReference stRef;
    stRef.aniObj = etsObserverObject;
    stRef.aniRef = global;
    etsObserverObjects_.emplace_back(stRef);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AddEtsObserverObject end");
}

AppExecFwk::ETSNativeReference ETSAbilityForegroundStateObserver::GetObserverObject(const ani_object &observerObject)
{
    if (observerObject == nullptr) {
        return AppExecFwk::ETSNativeReference();
    }
    for (const auto& observer : etsObserverObjects_) {
        if (observer.aniObj == observerObject) {
            return observer;
        }
    }
    return AppExecFwk::ETSNativeReference();
}

void ETSAbilityForegroundStateObserver::ReleaseObjectReference(ani_ref etsObjRef)
{
    if (etsObjRef == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null etsObj");
        return;
    }
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null etsVm_");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = etsVm_->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetEnv failed status: %{public}d", status);
        return;
    }
    if ((status = env->GlobalReference_Delete(etsObjRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GlobalReference_Delete failed status = %{public}d", status);
        return;
    }
}

bool ETSAbilityForegroundStateObserver::RemoveEtsObserverObject(const ani_object &observerObj)
{
    if (observerObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return false;
    }
    auto etsNativeReference = GetObserverObject(observerObj);
    if (etsNativeReference.aniObj != nullptr) {
        etsObserverObjects_.emplace_back(etsNativeReference);
        ReleaseObjectReference(etsNativeReference.aniRef);
    }
    return true;
}

void ETSAbilityForegroundStateObserver::RemoveAllEtsObserverObject()
{
    auto itr = etsObserverObjects_.begin();
    for (; itr != etsObserverObjects_.end(); ++itr) {
        ReleaseObjectReference(itr->aniRef);
        itr = etsObserverObjects_.erase(itr);
    }
}

void ETSAbilityForegroundStateObserver::SetValid(const bool valid)
{
    valid_ = valid;
}

bool ETSAbilityForegroundStateObserver::IsEmpty()
{
    return etsObserverObjects_.empty();
}
} // namespace AbilityRuntime
} // namespace OHOS
