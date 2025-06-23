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

#include "ets_ability_lifecycle_callback.h"

#include "hilog_tag_wrapper.h"
#include "interop_guard.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *SIGNATURE_UIABILITY = "L@ohos/app/ability/UIAbility/UIAbility;:V";
constexpr const char *SIGNATURE_UIABILITY_WINDOW_STAGE =
    "L@ohos/app/ability/UIAbility/UIAbility;L@ohos/window/window/WindowStage;:V";
constexpr const char *SIGNATURE_ABILITY_LIFECYCLE_CALLBACK =
    "L@ohos/app/ability/AbilityLifecycleCallback/AbilityLifecycleCallback;";
constexpr const int32_t ERROR_CODE_NULL_ENV = -1;
constexpr const int32_t ERROR_CODE_NULL_CALLBACK = -2;
constexpr const int32_t ERROR_CODE_NULL_REF = -3;
}
StsAbilityLifecycleCallback::StsAbilityLifecycleCallback(ani_env *env)
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

ani_env *StsAbilityLifecycleCallback::GetAniEnv()
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

int32_t StsAbilityLifecycleCallback::serialNumber_ = 0;

void StsAbilityLifecycleCallback::CallObjectMethod(napi_env env, const char *methodName,
    const char *signature, std::shared_ptr<NativeReference> ability)
{
    ani_env *aniEnv = GetAniEnv();
    if (aniEnv == nullptr || ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniEnv or ability");
        return;
    }
    InteropGuard guard(env, ability->GetNapiValue());
    ani_object aniAbilityObj = guard.GetAniValue(aniEnv);
    if (aniAbilityObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniAbilityObj");
        return;
    }
    ani_ref aniAbilityRef = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = aniEnv->GlobalReference_Create(aniAbilityObj, &aniAbilityRef)) != ANI_OK || aniAbilityRef == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create reference, status=%{public}d", status);
        return;
    }

    ani_class cls = nullptr;
    if ((status = aniEnv->FindClass(SIGNATURE_ABILITY_LIFECYCLE_CALLBACK, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to find class, status=%{public}d", status);
        return;
    }

    ani_method method = nullptr;
    if ((status = aniEnv->Class_FindMethod(cls, methodName, signature, &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to find method, status=%{public}d", status);
        return;
    }

    ani_value aniAbility {};
    aniAbility.r = aniAbilityRef;
    for (const auto &callback : callbacks_) {
        if (callback.second == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null callback");
            return;
        }
        if ((status = aniEnv->Object_CallMethod_Void(
            reinterpret_cast<ani_object>(callback.second), method, aniAbility)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to call method %{public}s, status=%{public}d", methodName, status);
            return;
        }
    }
}

void StsAbilityLifecycleCallback::CallObjectMethod(napi_env env, const char *methodName, const char *signature,
    std::shared_ptr<NativeReference> ability, std::shared_ptr<NativeReference> windowStage)
{
    ani_env *aniEnv = GetAniEnv();
    if (aniEnv == nullptr || ability == nullptr || windowStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniEnv or ability or windowStage");
        return;
    }
    InteropGuard abilityGuard(env, ability->GetNapiValue());
    ani_object aniAbilityObj = abilityGuard.GetAniValue(aniEnv);
    InteropGuard windowStageGuard(env, windowStage->GetNapiValue());
    ani_object aniWindowStageObj = windowStageGuard.GetAniValue(aniEnv);
    if (aniAbilityObj == nullptr || aniWindowStageObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniAbilityObj or aniWindowStageObj");
        return;
    }
    ani_ref aniAbilityRef = nullptr;
    ani_ref aniWindowStageRef = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = aniEnv->GlobalReference_Create(aniAbilityObj, &aniAbilityRef)) != ANI_OK ||
        (status = aniEnv->GlobalReference_Create(aniAbilityObj, &aniWindowStageRef)) != ANI_OK ||
        aniAbilityRef == nullptr || aniWindowStageRef == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create reference, status=%{public}d", status);
        return;
    }

    ani_class cls = nullptr;
    if ((status = aniEnv->FindClass(SIGNATURE_ABILITY_LIFECYCLE_CALLBACK, &cls)) != ANI_OK || cls == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to find class, status=%{public}d", status);
        return;
    }

    ani_method method = nullptr;
    if ((status = aniEnv->Class_FindMethod(cls, methodName, signature, &method)) != ANI_OK || method == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to find method, status=%{public}d", status);
        return;
    }

    ani_value aniAbility {};
    aniAbility.r = aniAbilityRef;
    ani_value aniWindowStage {};
    aniWindowStage.r = aniWindowStageRef;
    for (const auto &callback : callbacks_) {
        if (callback.second == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null callback");
            return;
        }
        if ((status = aniEnv->Object_CallMethod_Void(reinterpret_cast<ani_object>(callback.second),
            method, aniAbility, aniWindowStage)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to call method %{public}s, status=%{public}d", methodName, status);
            return;
        }
    }
}

void StsAbilityLifecycleCallback::OnAbilityCreate([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onAbilityCreate", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnWindowStageCreate([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability, std::shared_ptr<NativeReference> windowStage)
{
    CallObjectMethod(env, "onWindowStageCreate", SIGNATURE_UIABILITY_WINDOW_STAGE, ability, windowStage);
}

void StsAbilityLifecycleCallback::OnWindowStageDestroy([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability, std::shared_ptr<NativeReference> windowStage)
{
    CallObjectMethod(env, "onWindowStageDestroy", SIGNATURE_UIABILITY_WINDOW_STAGE, ability, windowStage);
}

void StsAbilityLifecycleCallback::OnWindowStageActive([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability, std::shared_ptr<NativeReference> windowStage)
{
    CallObjectMethod(env, "onWindowStageActive", SIGNATURE_UIABILITY_WINDOW_STAGE, ability, windowStage);
}

void StsAbilityLifecycleCallback::OnWindowStageInactive([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability, std::shared_ptr<NativeReference> windowStage)
{
    CallObjectMethod(env, "onWindowStageInactive", SIGNATURE_UIABILITY_WINDOW_STAGE, ability, windowStage);
}

void StsAbilityLifecycleCallback::OnAbilityDestroy([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onAbilityDestroy", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnAbilityForeground([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onAbilityForeground", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnAbilityBackground([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onAbilityBackground", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnAbilityContinue([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onAbilityContinue", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnAbilityWillContinue([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onAbilityWillContinue", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnWindowStageWillRestore([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability, std::shared_ptr<NativeReference> windowStage)
{
    CallObjectMethod(env, "onWindowStageWillRestore", SIGNATURE_UIABILITY_WINDOW_STAGE, ability, windowStage);
}

void StsAbilityLifecycleCallback::OnWindowStageRestore([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability, std::shared_ptr<NativeReference> windowStage)
{
    CallObjectMethod(env, "onWindowStageRestore", SIGNATURE_UIABILITY_WINDOW_STAGE, ability, windowStage);
}

void StsAbilityLifecycleCallback::OnAbilityWillSaveState([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onAbilityWillSaveState", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnAbilitySaveState([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onAbilitySaveState", SIGNATURE_UIABILITY, ability);
}

int32_t StsAbilityLifecycleCallback::Register(ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "enter StsAbilityLifecycleCallback::Register");
    ani_env *aniEnv = GetAniEnv();
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniEnv");
        return ERROR_CODE_NULL_ENV;
    }
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniEnv");
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
    if ((status = aniEnv->GlobalReference_Create(callback, &ref)) != ANI_OK || ref == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create reference, status=%{public}d", status);
        return ERROR_CODE_NULL_REF;
    }
    callbacks_.emplace(callbackId, ref);
    return callbackId;
}

bool StsAbilityLifecycleCallback::Unregister(int32_t callbackId)
{
    TAG_LOGI(AAFwkTag::APPKIT, "Unregister callbackId : %{public}d", callbackId);
    return callbacks_.erase(callbackId);
}

void StsAbilityLifecycleCallback::OnNewWant([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onNewWant", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnWillNewWant([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onWillNewWant", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnAbilityWillCreate([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onAbilityWillCreate", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnWindowStageWillCreate([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability, std::shared_ptr<NativeReference> windowStage)
{
    CallObjectMethod(env, "onWindowStageWillCreate", SIGNATURE_UIABILITY_WINDOW_STAGE, ability, windowStage);
}

void StsAbilityLifecycleCallback::OnWindowStageWillDestroy([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability, std::shared_ptr<NativeReference> windowStage)
{
    CallObjectMethod(env, "onWindowStageWillDestroy", SIGNATURE_UIABILITY_WINDOW_STAGE, ability, windowStage);
}

void StsAbilityLifecycleCallback::OnAbilityWillDestroy([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onAbilityWillDestroy", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnAbilityWillForeground([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onAbilityWillForeground", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnAbilityWillBackground([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onAbilityWillBackground", SIGNATURE_UIABILITY, ability);
}

bool StsAbilityLifecycleCallback::IsEmpty() const
{
    return callbacks_.empty();
}

void StsAbilityLifecycleCallback::OnWillForeground([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onWillForeground", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnDidForeground([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onDidForeground", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnWillBackground([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onWillBackground", SIGNATURE_UIABILITY, ability);
}

void StsAbilityLifecycleCallback::OnDidBackground([[maybe_unused]]napi_env env,
    std::shared_ptr<NativeReference> ability)
{
    CallObjectMethod(env, "onDidBackground", SIGNATURE_UIABILITY, ability);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
