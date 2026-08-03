/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "ability_context.h"

#include <memory>
#include <new>

#include "hilog_tag_wrapper.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t EXPECTED_ARGC = 2;
constexpr size_t MAX_ARGC = EXPECTED_ARGC + 1;
constexpr char CREATE_HOLDER_FUNCTION[] = "createUIAbilityContextHolder";

napi_value ThrowTargetError(napi_env env, const char* message, bool typeError = false)
{
    if (env == nullptr) {
        return nullptr;
    }

    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    if (typeError) {
        napi_throw_type_error(env, nullptr, message);
    } else {
        napi_throw_error(env, nullptr, message);
    }
    return undefined;
}

bool GetExternalPointer(napi_env env, napi_value value, void** pointer)
{
    if (env == nullptr || value == nullptr || pointer == nullptr) {
        return false;
    }

    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, value, &type) != napi_ok || type != napi_external) {
        return false;
    }
    return napi_get_value_external(env, value, pointer) == napi_ok && *pointer != nullptr;
}

void FinalizeContextHolder(napi_env, void* data, void*)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "finalize UIAbilityContext interop holder");
    delete static_cast<std::weak_ptr<Context>*>(data);
}

napi_value CreateUIAbilityContextHolder(napi_env targetEnv, napi_callback_info info)
{
    if (targetEnv == nullptr || info == nullptr) {
        return nullptr;
    }

    size_t argc = MAX_ARGC;
    napi_value argv[MAX_ARGC] = { nullptr };
    if (napi_get_cb_info(targetEnv, info, &argc, argv, nullptr, nullptr) != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "get callback info failed");
        return ThrowTargetError(targetEnv, "Failed to read UIAbilityContext interop arguments.");
    }
    if (argc != EXPECTED_ARGC) {
        TAG_LOGE(AAFwkTag::CONTEXT, "invalid argument count: %{public}zu", argc);
        return ThrowTargetError(targetEnv,
            "createUIAbilityContextHolder requires sourceEnv and sourceValue.", true);
    }

    void* sourceEnvPointer = nullptr;
    void* sourceValuePointer = nullptr;
    if (!GetExternalPointer(targetEnv, argv[0], &sourceEnvPointer) ||
        !GetExternalPointer(targetEnv, argv[1], &sourceValuePointer)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "source env or value is not a valid external");
        return ThrowTargetError(targetEnv,
            "sourceEnv and sourceValue must be non-null external values.", true);
    }

    auto sourceEnv = reinterpret_cast<napi_env>(sourceEnvPointer);
    auto sourceValue = reinterpret_cast<napi_value>(sourceValuePointer);
    napi_valuetype sourceType = napi_undefined;
    if (napi_typeof(sourceEnv, sourceValue, &sourceType) != napi_ok || sourceType != napi_object) {
        TAG_LOGE(AAFwkTag::CONTEXT, "source value is not an object");
        return ThrowTargetError(targetEnv, "The source UIAbilityContext is not an object.", true);
    }

    void* wrapped = nullptr;
    if (napi_unwrap(sourceEnv, sourceValue, &wrapped) != napi_ok || wrapped == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "unwrap source UIAbilityContext failed");
        return ThrowTargetError(targetEnv, "Failed to unwrap the source UIAbilityContext.");
    }

    // Private bridge contract: sourceValue is an AbilityRuntime-created
    // UIAbilityContext wrapper whose native payload is weak_ptr<AbilityContext>.
    auto* weakAbilityContext = static_cast<std::weak_ptr<AbilityContext>*>(wrapped);
    auto abilityContext = weakAbilityContext->lock();
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "source UIAbilityContext has expired");
        return ThrowTargetError(targetEnv, "The source UIAbilityContext has expired.");
    }
    TAG_LOGI(AAFwkTag::CONTEXT, "source UIAbilityContext unwrap succeeded");

    napi_value holder = nullptr;
    if (napi_create_object(targetEnv, &holder) != napi_ok || holder == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "create target holder failed");
        return ThrowTargetError(targetEnv, "Failed to create the target UIAbilityContext holder.");
    }

    napi_value stageMode = nullptr;
    if (napi_get_boolean(targetEnv, true, &stageMode) != napi_ok ||
        napi_set_named_property(targetEnv, holder, "stageMode", stageMode) != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "set stageMode on target holder failed");
        return ThrowTargetError(targetEnv, "Failed to initialize the target UIAbilityContext holder.");
    }

    std::shared_ptr<Context> context = abilityContext;
    auto* weakContext = new (std::nothrow) std::weak_ptr<Context>(context);
    if (weakContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "allocate target weak context failed");
        return ThrowTargetError(targetEnv, "Failed to allocate the target UIAbilityContext holder.");
    }

    napi_status status = napi_wrap(
        targetEnv, holder, weakContext, FinalizeContextHolder, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "wrap target holder failed: %{public}d", status);
        delete weakContext;
        return ThrowTargetError(targetEnv, "Failed to wrap the target UIAbilityContext holder.");
    }

    TAG_LOGI(AAFwkTag::CONTEXT, "target UIAbilityContext holder created with independent weak context");
    return holder;
}

napi_value Init(napi_env env, napi_value exports)
{
    if (env == nullptr || exports == nullptr) {
        return nullptr;
    }

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION(CREATE_HOLDER_FUNCTION, CreateUIAbilityContextHolder),
    };
    napi_status status =
        napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "define UIAbilityContext interop exports failed: %{public}d", status);
        return ThrowTargetError(env, "Failed to define UIAbilityContext interop exports.");
    }
    TAG_LOGI(AAFwkTag::CONTEXT, "UIAbilityContext interop module loaded");
    return exports;
}
} // namespace
} // namespace AbilityRuntime
} // namespace OHOS

static napi_module g_uiAbilityContextInteropModule = {
    .nm_version = 0,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = OHOS::AbilityRuntime::Init,
    .nm_modname = "app.ability.uiAbilityContextInterop",
    .nm_priv = nullptr,
    .reserved = { nullptr },
};

extern "C" __attribute__((constructor)) void NapiAppAbilityUIAbilityContextInteropAutoRegister()
{
    napi_module_register(&g_uiAbilityContextInteropModule);
}
