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

#include "ets_static_ability_lifecycle_callback.h"

#include "hilog_tag_wrapper.h"
#include "interop_guard.h"
#include "js_runtime_utils.h"
#include "sts_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *SIGNATURE_UIABILITY = "L@ohos/app/ability/UIAbility/UIAbility;:V";
constexpr const char *SIGNATURE_UIABILITY_WINDOW_STAGE =
    "L@ohos/app/ability/UIAbility/UIAbility;L@ohos/window/window/WindowStage;:V";
constexpr const char *SIGNATURE_STATIC_ABILITY_LIFECYCLE_CALLBACK =
    "L@ohos/app/ability/StaticAbilityLifecycleCallback/StaticAbilityLifecycleCallback;";
constexpr const int32_t ERROR_CODE_NULL_ENV = -1;
constexpr const int32_t ERROR_CODE_NULL_CALLBACK = -2;
constexpr const int32_t ERROR_CODE_NULL_REF = -3;
}

int32_t StsStaticAbilityLifecycleCallback::serialNumber_ = 0;

StsStaticAbilityLifecycleCallback::StsStaticAbilityLifecycleCallback()
{
}

void StsStaticAbilityLifecycleCallback::CallObjectMethod(ani_env *env, const char *methodName,
    const char *signature, std::shared_ptr<STSNativeReference> ability)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    if (ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability");
        return;
    }

    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(SIGNATURE_STATIC_ABILITY_LIFECYCLE_CALLBACK, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to find class, status=%{public}d", status);
        return;
    }

    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, methodName, signature, &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to find method, status=%{public}d", status);
        return;
    }

    ani_value aniAbility {};
    aniAbility.r = ability->aniRef;
    for (const auto &callback : callbacks_) {
        if (callback.second == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null callback");
            return;
        }
        if ((status = env->Object_CallMethod_Void(reinterpret_cast<ani_object>(callback.second),
            method, aniAbility)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to call method %{public}s, status=%{public}d", methodName, status);
            return;
        }
    }
}

void StsStaticAbilityLifecycleCallback::CallObjectMethod(ani_env *env, const char *methodName,
    const char *signature, std::shared_ptr<STSNativeReference> ability,
    std::shared_ptr<STSNativeReference> windowStage)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    if (ability == nullptr || windowStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null ability or windowStage");
        return;
    }

    ani_class cls = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(SIGNATURE_STATIC_ABILITY_LIFECYCLE_CALLBACK, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to find class, status=%{public}d", status);
        return;
    }

    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, methodName, signature, &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to find method, status=%{public}d", status);
        return;
    }

    ani_value aniAbility {};
    aniAbility.r = ability->aniRef;
    ani_value aniWindowStage {};
    aniWindowStage.r = windowStage->aniRef;
    for (const auto &callback : callbacks_) {
        if (callback.second == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null callback");
            return;
        }
        if ((status = env->Object_CallMethod_Void(reinterpret_cast<ani_object>(callback.second),
            method, aniAbility, aniWindowStage)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to call method %{public}s, status=%{public}d", methodName, status);
            return;
        }
    }
}

void StsStaticAbilityLifecycleCallback::OnAbilityCreate(ani_env *env, std::shared_ptr<STSNativeReference> ability)
{
    CallObjectMethod(env, "onAbilityCreate", SIGNATURE_UIABILITY, ability);
}

void StsStaticAbilityLifecycleCallback::OnWindowStageCreate(ani_env *env, std::shared_ptr<STSNativeReference> ability,
    std::shared_ptr<STSNativeReference> windowStage)
{
    CallObjectMethod(env, "onWindowStageCreate", SIGNATURE_UIABILITY_WINDOW_STAGE, ability, windowStage);
}

void StsStaticAbilityLifecycleCallback::OnAbilityWillCreate(ani_env *env, std::shared_ptr<STSNativeReference> ability)
{
    CallObjectMethod(env, "onAbilityWillCreate", SIGNATURE_UIABILITY, ability);
}

void StsStaticAbilityLifecycleCallback::OnWindowStageWillCreate(ani_env *env,
    std::shared_ptr<STSNativeReference> ability, std::shared_ptr<STSNativeReference> windowStage)
{
    CallObjectMethod(env, "onWindowStageWillCreate", SIGNATURE_UIABILITY_WINDOW_STAGE, ability, windowStage);
}

void StsStaticAbilityLifecycleCallback::OnAbilityDestroy(ani_env *env, std::shared_ptr<STSNativeReference> ability)
{
    CallObjectMethod(env, "onAbilityDestroy", SIGNATURE_UIABILITY, ability);
}

void StsStaticAbilityLifecycleCallback::OnWindowStageDestroy(ani_env *env,
    std::shared_ptr<STSNativeReference> ability, std::shared_ptr<STSNativeReference> windowStage)
{
    CallObjectMethod(env, "onWindowStageDestroy", SIGNATURE_UIABILITY_WINDOW_STAGE, ability, windowStage);
}

void StsStaticAbilityLifecycleCallback::OnAbilityForeground(ani_env *env, std::shared_ptr<STSNativeReference> ability)
{
    CallObjectMethod(env, "onAbilityForeground", SIGNATURE_UIABILITY, ability);
}

void StsStaticAbilityLifecycleCallback::OnAbilityBackground(ani_env *env, std::shared_ptr<STSNativeReference> ability)
{
    CallObjectMethod(env, "onAbilityBackground", SIGNATURE_UIABILITY, ability);
}

int32_t StsStaticAbilityLifecycleCallback::Register(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "call StsStaticAbilityLifecycleCallback::Register");
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

bool StsStaticAbilityLifecycleCallback::Unregister(int32_t callbackId)
{
    TAG_LOGI(AAFwkTag::APPKIT, "Unregister callbackId : %{public}d", callbackId);
    return callbacks_.erase(callbackId);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
