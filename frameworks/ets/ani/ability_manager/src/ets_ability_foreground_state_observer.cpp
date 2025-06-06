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
#include "sts_ability_manager_utils.h"
#include "hilog_tag_wrapper.h"
#include "ani_common_util.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* ABILITY_FOREGROUND_STATE_OBSERVER_CLASS_NAME =
    "Lapplication/AbilityForegroundStateObserver/AbilityForegroundStateObserver;";
constexpr const char *SIGNATURE_ABILITY_STATE_DATA = "Lapplication/AbilityStateData/AbilityStateData;:V";
}
ETSAbilityForegroundStateObserver::ETSAbilityForegroundStateObserver(ani_vm *etsVm) : etsVm_(etsVm) {}

void ETSAbilityForegroundStateObserver::OnAbilityStateChanged(const AbilityStateData &abilityStateData)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "OnAbilityStateChanged called");
    if (!valid_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "invalid appms");
        return;
    }
    HandleOnAbilityStateChanged(abilityStateData);
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
        TAG_LOGE(AAFwkTag::STSRUNTIME, "status : %{public}d", status);
        return;
    }
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "env nullptr");
        return;
    }
    for (auto &item : etsObserverObjects_) {
        auto abilityStateDataObj = AbilityManagerSts::WrapAbilityStateData(env, abilityStateData);
        if (abilityStateDataObj != nullptr && item.aniRef != nullptr) {
            CallStsFunction(env, reinterpret_cast<ani_object>(item.aniRef),
                "onAbilityStateChanged", SIGNATURE_ABILITY_STATE_DATA, abilityStateDataObj);
        }
    }
    if (etsVm_->DetachCurrentThread() != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "status : %{public}d", status);
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
    STSNativeReference stRef;
    stRef.aniObj = etsObserverObject;
    stRef.aniRef = global;
    etsObserverObjects_.emplace_back(stRef);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AddEtsObserverObject end");
}

STSNativeReference ETSAbilityForegroundStateObserver::GetObserverObject(const ani_object &observerObject)
{
    if (observerObject == nullptr) {
        return STSNativeReference();
    }
    for (const auto& observer : etsObserverObjects_) {
        if (observer.aniObj == observerObject) {
            return observer;
        }
    }
    return STSNativeReference();
}

void ETSAbilityForegroundStateObserver::ReleaseObjectRefrence(ani_ref etsObjRef)
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
        TAG_LOGE(AAFwkTag::APPMGR, "GlobalReference_Delete failed status = %{public}d", status);
        return;
    }
}

bool ETSAbilityForegroundStateObserver::RemoveEtsObserverObject(const ani_object &observerObj)
{
    if (observerObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        return false;
    }
    auto etsNativeRefrence = GetObserverObject(observerObj);
    if (etsNativeRefrence.aniObj != nullptr) {
        etsObserverObjects_.emplace_back(etsNativeRefrence);
        ReleaseObjectRefrence(etsNativeRefrence.aniRef);
    }
    return true;
}

void ETSAbilityForegroundStateObserver::RemoveAllEtsObserverObject()
{
    auto itr = etsObserverObjects_.begin();
    for (; itr != etsObserverObjects_.end(); ++itr) {
        ReleaseObjectRefrence(itr->aniRef);
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
