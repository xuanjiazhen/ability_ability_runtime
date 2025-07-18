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

#ifdef SUPPORT_GRAPHICS
#include "ets_ability_first_frame_state_observer.h"

#include "hilog_tag_wrapper.h"
#include "sts_app_manager_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* ABILITY_FIRST_FRAME_STATE_OBSERVER_CLASS_NAME =
    "Lapplication/AbilityFirstFrameStateObserver/AbilityFirstFrameStateObserver;";
}
ETSAbilityFirstFrameStateObserver::ETSAbilityFirstFrameStateObserver(ani_vm *vm) : etsVm_(vm) {}

ani_status ETSAbilityFirstFrameStateObserver::AniSendEvent(const std::function<void()> task)
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

void ETSAbilityFirstFrameStateObserver::OnAbilityFirstFrameState(
    const AbilityFirstFrameStateData &abilityFirstFrameStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnAbilityFirstFrameState called");
    auto task = [&abilityFirstFrameStateData, this] () {
        HandleOnAbilityFirstFrameState(abilityFirstFrameStateData);
    };
    if (AniSendEvent(task) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to aniSendEvent");
    }
    TAG_LOGD(AAFwkTag::APPMGR, "OnAbilityFirstFrameState end");
}

void ETSAbilityFirstFrameStateObserver::HandleOnAbilityFirstFrameState(
    const AbilityFirstFrameStateData &abilityFirstFrameStateData)
{
    TAG_LOGD(AAFwkTag::APPMGR, "HandleOnAbilityFirstFrameState called");
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "etsVm_ nullptr");
        return;
    }
    ani_env *env = nullptr;
    if (!AppManagerSts::AttachAniEnv(etsVm_, env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Failed to AttachAniEnv");
        AppManagerSts::DetachAniEnv(etsVm_);
        return;
    }
    auto paramObj = AppManagerSts::WrapAbilityFirstFrameStateData(env, abilityFirstFrameStateData);
    if (paramObj != nullptr && etsObserverObject_ != nullptr) {
        CallStsFunction(env, *etsObserverObject_, "onAbilityFirstFrameDrawn", nullptr, paramObj);
    }
    AppManagerSts::DetachAniEnv(etsVm_);
}

void ETSAbilityFirstFrameStateObserver::CallStsFunction(ani_env* env, ani_object etsObserverObject,
    const char *methodName, const char *signature, ...)
{
    TAG_LOGD(AAFwkTag::APPMGR, "call method:%{public}s", methodName);
    ani_class cls;
    ani_method method = nullptr;
    ani_status status = ANI_OK;
    if ((status = env->FindClass(ABILITY_FIRST_FRAME_STATE_OBSERVER_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "find observer failed status : %{public}d", status);
        return;
    }
    if ((status = env->Class_FindMethod(cls, methodName, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status: %{public}d", status);
        return;
    }
    env->ResetError();
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsObserverObject, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "status: %{public}d", status);
        return;
    }
    va_end(args);
    TAG_LOGD(AAFwkTag::APPMGR, "end");
    return;
}

void ETSAbilityFirstFrameStateObserver::SetEtsObserverObject(const ani_object &etsObserverObject)
{
    if (etsVm_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null etsVm_");
        return;
    }
    ani_env *env = nullptr;
    ani_status status = etsVm_->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetEnv failed status: %{public}d", status);
        return;
    }
    if (etsObserverObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        return;
    }
    ani_ref observerRef = nullptr;
    if ((status = env->GlobalReference_Create(etsObserverObject, &observerRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status : %{public}d", status);
        return;
    }
    etsObserverObject_ = std::make_shared<ani_object>(etsObserverObject);
}

void ETSAbilityFirstFrameStateObserver::ResetEtsObserverObject()
{
    if (etsObserverObject_) {
        if (etsVm_ == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null etsVm_");
            return;
        }
        ani_env *env = nullptr;
        ani_status status = etsVm_->GetEnv(ANI_VERSION_1, &env);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "GetEnv failed status: %{public}d", status);
            return;
        }
        if ((status = env->GlobalReference_Delete(*etsObserverObject_)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "GlobalReference_Delete failed status = %{public}d", status);
            return;
        }
        etsObserverObject_.reset();
    }
}

void ETSAbilityFirstFrameStateObserverManager::AddEtsAbilityFirstFrameStateObserver(
    const sptr<ETSAbilityFirstFrameStateObserver> observer)
{
    if (observer == nullptr || observer->GetAniObserver() == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        return;
    }
    std::lock_guard<std::mutex> lock(observerListLock_);
    etsAbilityFirstFrameStateObserverList_.emplace_back(observer);
}

bool ETSAbilityFirstFrameStateObserverManager::IsObserverObjectExist(const ani_object &esObserverObject)
{
    if (GetObserverObject(esObserverObject) == nullptr) {
        TAG_LOGD(AAFwkTag::APPMGR, "observer not exist");
        return false;
    }
    TAG_LOGD(AAFwkTag::APPMGR, "observer exist");
    return true;
}

void ETSAbilityFirstFrameStateObserverManager::RemoveAllEtsObserverObjects(
    sptr<OHOS::AAFwk::IAbilityManager> &abilityManager)
{
    TAG_LOGD(AAFwkTag::APPMGR, "RemoveAllEtsObserverObject called");
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityMgr");
        return;
    }
    std::lock_guard<std::mutex> lock(observerListLock_);
    for (auto observer : etsAbilityFirstFrameStateObserverList_) {
        abilityManager->UnregisterAbilityFirstFrameStateObserver(observer);
        observer->ResetEtsObserverObject();
    }
    etsAbilityFirstFrameStateObserverList_.clear();
}

void ETSAbilityFirstFrameStateObserverManager::RemoveEtsObserverObject(
    sptr<OHOS::AAFwk::IAbilityManager> &abilityManager, const ani_object &etsObserverObject)
{
    TAG_LOGD(AAFwkTag::APPMGR, "called");
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null abilityMgr");
        return;
    }
    if (etsObserverObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        return;
    }
    auto it = etsAbilityFirstFrameStateObserverList_.begin();
    for (; it != etsAbilityFirstFrameStateObserverList_.end(); ++it) {
        if (*it == nullptr) {
            continue;
        }
        std::shared_ptr<ani_object> tmpObject = (*it)->GetAniObserver();
        if (tmpObject == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null value");
            continue;
        }
        if (*tmpObject == etsObserverObject) {
            abilityManager->UnregisterAbilityFirstFrameStateObserver(*it);
            (*it)->ResetEtsObserverObject();
            etsAbilityFirstFrameStateObserverList_.erase(it);
            return;
        }
    }
}

std::shared_ptr<ani_object> ETSAbilityFirstFrameStateObserverManager::GetObserverObject(
    const ani_object &etsObserverObject)
{
    if (etsObserverObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null observer");
        return nullptr;
    }
    std::lock_guard<std::mutex> lock(observerListLock_);
    for (auto &observer : etsAbilityFirstFrameStateObserverList_) {
        if (observer == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "null observer");
            continue;
        }
        std::shared_ptr<ani_object> tmpObject = observer->GetAniObserver();
        if (tmpObject && *tmpObject == etsObserverObject) {
            return tmpObject;
        }
    }
    return nullptr;
}
} // namespace AbilityRuntime
} // namespace OHOS
#endif // SUPPORT_GRAPHICS