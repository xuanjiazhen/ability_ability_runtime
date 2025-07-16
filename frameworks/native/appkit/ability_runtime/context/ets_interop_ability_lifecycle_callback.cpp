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

#include "ets_interop_ability_lifecycle_callback.h"

#include "hilog_tag_wrapper.h"
#include "interop_object.h"
#include "js_runtime_utils.h"
#include "sts_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *SIGNATURE_UIABILITY = "Lstd/core/Object;:V";
constexpr const char *SIGNATURE_UIABILITY_WINDOW_STAGE =
    "Lstd/core/Object;L@ohos/window/window/WindowStage;:V";
constexpr const char *SIGNATURE_INTEROP_ABILITY_LIFECYCLE_CALLBACK =
    "L@ohos/app/ability/InteropAbilityLifecycleCallback/InteropAbilityLifecycleCallback;";
constexpr const char *CLASS_NAME_ESVALUE = "Lstd/interop/ESValue;";
constexpr const char *METHOD_NAME_UNWRAP = "unwrap";
constexpr const char *SIGNATURE_ESVALUE_UNWRAP = ":Lstd/core/Object;";
constexpr const int32_t ERROR_CODE_NULL_ENV = -1;
constexpr const int32_t ERROR_CODE_NULL_CALLBACK = -2;
constexpr const int32_t ERROR_CODE_NULL_REF = -3;
}

int32_t EtsInteropAbilityLifecycleCallback::serialNumber_ = 0;

ani_env *EtsInteropAbilityLifecycleCallback::GetAniEnv()
{
    if (vm_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null vm_");
        return nullptr;
    }
    ani_env* env = nullptr;
    if (vm_->GetEnv(ANI_VERSION_1, &env) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetEnv failed");
        return nullptr;
    }
    return env;
}

EtsInteropAbilityLifecycleCallback::EtsInteropAbilityLifecycleCallback(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GetVM failed");
        return;
    }
    vm_ = aniVM;
}

ani_ref EtsInteropAbilityLifecycleCallback::CallObjectMethod(const char *className,
    const char *methodName, const char *signature, ani_object aniObject)
{
    ani_env *env = GetAniEnv();
    if (env == nullptr || aniObject == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or object");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(className, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to find class %{public}s, status=%{public}d", className, status);
        return nullptr;
    }

    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, methodName, signature, &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to find method, status=%{public}d", status);
        return nullptr;
    }
    ani_ref result;
    if ((status = env->Object_CallMethod_Ref(aniObject, method, &result)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to call method %{public}s, status=%{public}d", methodName, status);
        return nullptr;
    }

}

void EtsInteropAbilityLifecycleCallback::CallObjectMethod(const char *methodName,
    const char *signature, std::shared_ptr<InteropObject> ability)
{
    ani_env *env = GetEnv();
    if (env == nullptr || ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or ability");
        return;
    }
    ani_object abilityEsValue = ability->GetAniValue(env);
    if (abilityEsValue == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to get ability esvalue");
        return;
    }
    ani_ref abilityObjectRef = CallObjectMethod(CLASS_NAME_ESVALUE, SIGNATURE_ESVALUE_UNWRAP,
        METHOD_NAME_UNWRAP, abilityEsValue);
    if (abilityObjectRef == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to get abilityObjectRef");
        return;
    }
    ani_value abilityObject {};
    abilityObject.r = abilityObjectRef;

    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(SIGNATURE_INTEROP_ABILITY_LIFECYCLE_CALLBACK, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to find class, status=%{public}d", status);
        return;
    }

    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, methodName, signature, &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to find method, status=%{public}d", status);
        return;
    }

    for (const auto &callback : callbacks_) {
        if (callback.second == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null callback");
            return;
        }
        if ((status = env->Object_CallMethod_Void(reinterpret_cast<ani_object>(callback.second),
            method, abilityObject)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to call method %{public}s, status=%{public}d", methodName, status);
            return;
        }
    }
}

void EtsInteropAbilityLifecycleCallback::CallObjectMethod(const char *methodName,
    const char *signature, std::shared_ptr<InteropObject> ability,
    std::shared_ptr<InteropObject> windowStage)
{
    ani_env *env = GetEnv();
    if (env == nullptr || ability == nullptr || windowStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or ability or windowStage");
        return;
    }
    ani_object abilityEsValue = 

    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(SIGNATURE_INTEROP_ABILITY_LIFECYCLE_CALLBACK, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to find class, status=%{public}d", status);
        return;
    }

    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, methodName, signature, &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to find method, status=%{public}d", status);
        return;
    }

    ani_value abilityObject {};
    abilityObject.r = ability->aniRef;
    ani_value aniWindowStage {};
    aniWindowStage.r = windowStage->aniRef;
    for (const auto &callback : callbacks_) {
        if (callback.second == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null callback");
            return;
        }
        if ((status = env->Object_CallMethod_Void(reinterpret_cast<ani_object>(callback.second),
            method, abilityObject, aniWindowStage)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to call method %{public}s, status=%{public}d", methodName, status);
            return;
        }
    }
}

void EtsInteropAbilityLifecycleCallback::OnAbilityCreate(std::shared_ptr<InteropObject> ability)
{
    CallObjectMethod("onAbilityCreate", SIGNATURE_UIABILITY, ability);
}

void EtsInteropAbilityLifecycleCallback::OnWindowStageCreate(std::shared_ptr<InteropObject> ability,
    std::shared_ptr<InteropObject> windowStage)
{
    CallObjectMethod("onWindowStageCreate", SIGNATURE_UIABILITY_WINDOW_STAGE, ability, windowStage);
}

void EtsInteropAbilityLifecycleCallback::OnAbilityWillCreate(std::shared_ptr<InteropObject> ability)
{
    CallObjectMethod("onAbilityWillCreate", SIGNATURE_UIABILITY, ability);
}

void EtsInteropAbilityLifecycleCallback::OnWindowStageWillCreate(std::shared_ptr<InteropObject> ability,
    std::shared_ptr<InteropObject> windowStage)
{
    CallObjectMethod("onWindowStageWillCreate", SIGNATURE_UIABILITY_WINDOW_STAGE, ability, windowStage);
}

void EtsInteropAbilityLifecycleCallback::OnAbilityDestroy(std::shared_ptr<InteropObject> ability)
{
    CallObjectMethod("onAbilityDestroy", SIGNATURE_UIABILITY, ability);
}

void EtsInteropAbilityLifecycleCallback::OnWindowStageDestroy(std::shared_ptr<InteropObject> ability,
    std::shared_ptr<InteropObject> windowStage)
{
    CallObjectMethod("onWindowStageDestroy", SIGNATURE_UIABILITY_WINDOW_STAGE, ability, windowStage);
}

void EtsInteropAbilityLifecycleCallback::OnAbilityForeground(std::shared_ptr<InteropObject> ability)
{
    CallObjectMethod("onAbilityForeground", SIGNATURE_UIABILITY, ability);
}

void EtsInteropAbilityLifecycleCallback::OnAbilityBackground(std::shared_ptr<InteropObject> ability)
{
    CallObjectMethod("onAbilityBackground", SIGNATURE_UIABILITY, ability);
}

int32_t EtsInteropAbilityLifecycleCallback::Register(ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "call EtsInteropAbilityLifecycleCallback::Register");
    ani_env *env = GetEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return ERROR_CODE_NULL_ENV;
    }
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callback");
        return ERROR_CODE_NULL_CALLBACK;
    }
    int32_t callbackId = serialNumber_;
    if (serialNumber_ < INT32_MAX) {
        serialNumber_++;
    } else {
        serialNumber_ = 0;
    }

    ani_ref ref = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->GlobalReference_Create(callback, &ref)) != ANI_OK || ref == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create reference, status=%{public}d", status);
        return ERROR_CODE_NULL_REF;
    }
    callbacks_.emplace(callbackId, ref);
    return callbackId;
}

bool EtsInteropAbilityLifecycleCallback::Unregister(int32_t callbackId)
{
    TAG_LOGI(AAFwkTag::APPKIT, "Unregister callbackId : %{public}d", callbackId);
    return callbacks_.erase(callbackId);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
