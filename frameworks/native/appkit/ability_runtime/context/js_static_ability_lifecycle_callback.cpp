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

#include "js_static_ability_lifecycle_callback.h"

#include "hilog_tag_wrapper.h"
#include "interop_guard.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "native_engine/native_engine.h"
#include "sts_runtime.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const size_t ARGC_ONE = 1;
constexpr const size_t ARGC_TWO = 2;
constexpr const int32_t ERROR_CODE_NULL_ENV = -1;
constexpr const int32_t ERROR_CODE_NULL_CALLBACK = -2;
}

JsStaticAbilityLifecycleCallback::JsStaticAbilityLifecycleCallback(napi_env env)
    : env_(env)
{
}

int32_t JsStaticAbilityLifecycleCallback::serialNumber_ = 0;

void JsStaticAbilityLifecycleCallback::CallObjectMethod(ani_env *env, const char *methodName,
    std::shared_ptr<STSNativeReference> ability)
{
    if (env == nullptr || ability == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or ability");
        return;
    }
    InteropGuard guard(env, ability->aniRef);
    napi_value nativeAbilityObj = guard.GetNapiValue(env_);
    if (nativeAbilityObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null nativeAbilityObj");
        return;
    }

    napi_value argv[] = { nativeAbilityObj };
    for (const auto &callback : callbacks_) {
        if (!callback.second) {
            TAG_LOGE(AAFwkTag::APPKIT, "Invalid jsCallback");
            return;
        }

        auto obj = callback.second->GetNapiValue();
        if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
            TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
            return;
        }

        napi_value method = nullptr;
        napi_get_named_property(env_, obj, methodName, &method);
        if (method == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null method %{public}s", methodName);
            return;
        }

        napi_call_function(env_, obj, method, ARGC_ONE, argv, nullptr);
    }
}

void JsStaticAbilityLifecycleCallback::CallObjectMethod(ani_env *env, const char *methodName,
    std::shared_ptr<STSNativeReference> ability, std::shared_ptr<STSNativeReference> windowStage)
{
    if (env == nullptr || ability == nullptr || windowStage == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or ability");
        return;
    }
    InteropGuard abilityGuard(env, ability->aniRef);
    napi_value nativeAbilityObj = abilityGuard.GetNapiValue(env_);
    if (nativeAbilityObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null nativeAbilityObj");
        return;
    }
    InteropGuard windowStageGuard(env, windowStage->aniRef);
    napi_value nativeWindowStageObj = windowStageGuard.GetNapiValue(env_);
    if (nativeWindowStageObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null nativeWindowStageObj");
        return;
    }

    napi_value argv[] = { nativeAbilityObj, nativeWindowStageObj };
    for (const auto &callback : callbacks_) {
        if (!callback.second) {
            TAG_LOGE(AAFwkTag::APPKIT, "Invalid jsCallback");
            return;
        }

        auto obj = callback.second->GetNapiValue();
        if (!CheckTypeForNapiValue(env_, obj, napi_object)) {
            TAG_LOGE(AAFwkTag::APPKIT, "get object failed");
            return;
        }

        napi_value method = nullptr;
        napi_get_named_property(env_, obj, methodName, &method);
        if (method == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null method %{public}s", methodName);
            return;
        }

        napi_call_function(env_, obj, method, ARGC_TWO, argv, nullptr);
    }
}

void JsStaticAbilityLifecycleCallback::OnAbilityCreate(ani_env *env, std::shared_ptr<STSNativeReference> ability)
{
    CallObjectMethod(env, "onAbilityCreate", ability);
}

void JsStaticAbilityLifecycleCallback::OnWindowStageCreate(ani_env *env, std::shared_ptr<STSNativeReference> ability,
    std::shared_ptr<STSNativeReference> windowStage)
{
    CallObjectMethod(env, "onWindowStageCreate", ability, windowStage);
}

void JsStaticAbilityLifecycleCallback::OnAbilityWillCreate(ani_env *env, std::shared_ptr<STSNativeReference> ability)
{
    CallObjectMethod(env, "onAbilityWillCreate", ability);
}

void JsStaticAbilityLifecycleCallback::OnWindowStageWillCreate(ani_env *env,
    std::shared_ptr<STSNativeReference> ability, std::shared_ptr<STSNativeReference> windowStage)
{
    CallObjectMethod(env, "onWindowStageWillCreate", ability, windowStage);
}

void JsStaticAbilityLifecycleCallback::OnAbilityDestroy(ani_env *env, std::shared_ptr<STSNativeReference> ability)
{
    CallObjectMethod(env, "onAbilityDestroy", ability);
}

void JsStaticAbilityLifecycleCallback::OnWindowStageDestroy(ani_env *env,
    std::shared_ptr<STSNativeReference> ability, std::shared_ptr<STSNativeReference> windowStage)
{
    CallObjectMethod(env, "onWindowStageDestroy", ability, windowStage);
}

void JsStaticAbilityLifecycleCallback::OnAbilityForeground(ani_env *env, std::shared_ptr<STSNativeReference> ability)
{
    CallObjectMethod(env, "onAbilityForeground", ability);
}

void JsStaticAbilityLifecycleCallback::OnAbilityBackground(ani_env *env, std::shared_ptr<STSNativeReference> ability)
{
    CallObjectMethod(env, "onAbilityBackground", ability);
}

int32_t JsStaticAbilityLifecycleCallback::Register(napi_value callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "enter JsStaticAbilityLifecycleCallback::Register");
    if (env_ == nullptr) {
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
    napi_ref ref = nullptr;
    napi_create_reference(env_, callback, 1, &ref);
    callbacks_.emplace(callbackId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
    return callbackId;
}

bool JsStaticAbilityLifecycleCallback::Unregister(int32_t callbackId)
{
    TAG_LOGI(AAFwkTag::APPKIT, "Unregister callbackId : %{public}d", callbackId);
    return callbacks_.erase(callbackId);
}
}  // namespace AbilityRuntime
}  // namespace OHOS
