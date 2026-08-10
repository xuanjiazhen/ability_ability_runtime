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

#include <cstdint>
#include <cstring>
#include <memory>
#include <new>

#include "cj_ability_context.h"
#include "ffi_remote_data.h"
#include "hilog_tag_wrapper.h"
#include "js_ability_context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t HYBRID_ARGC = 2;
constexpr size_t FFI_ARGC = 1;
constexpr size_t HYBRID_MAX_ARGC = HYBRID_ARGC + 1;
constexpr size_t FFI_MAX_ARGC = FFI_ARGC + 1;
constexpr char CREATE_HOLDER_FUNCTION[] = "createUIAbilityContextHolder";
constexpr char CREATE_HOLDER_FROM_FFI_FUNCTION[] = "createUIAbilityContextHolderFromFfi";
constexpr char CONTEXT_IMPL_NAME[] = "__context_impl__";
constexpr char BASE_CONTEXT_NAME[] = "__base_context_ptr__";

constexpr const char* FORWARDED_PROPERTIES[] = {
    "applicationInfo",
    "currentHapModuleInfo",
    "abilityInfo",
    "config",
    "resourceManager",
    "cacheDir",
    "tempDir",
    "resourceDir",
    "filesDir",
    "distributedFilesDir",
    "databaseDir",
    "preferencesDir",
    "bundleCodeDir",
    "cloudFileDir",
    "logFileDir",
    "processName",
    "area",
};

napi_value ThrowTargetError(napi_env env, const char* message, bool typeError = false)
{
    if (env == nullptr) {
        return nullptr;
    }

    bool pending = false;
    if (napi_is_exception_pending(env, &pending) == napi_ok && pending) {
        napi_value ignored = nullptr;
        (void)napi_get_and_clear_last_exception(env, &ignored);
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

bool GetThisAndData(napi_env env, napi_callback_info info, napi_value& thisVar, void** data)
{
    size_t argc = 0;
    thisVar = nullptr;
    return napi_get_cb_info(env, info, &argc, nullptr, &thisVar, data) == napi_ok && thisVar != nullptr;
}

template<typename T>
bool VerifyWrappedPayload(napi_env env, napi_value object, T* expected)
{
    void* wrapped = nullptr;
    return expected != nullptr && napi_unwrap(env, object, &wrapped) == napi_ok && wrapped == expected;
}

void FinalizeContextHolder(napi_env, void* data, void*)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "finalize UIAbilityContext interop holder");
    delete static_cast<std::weak_ptr<AbilityContext>*>(data);
}

napi_value ForwardInnerProperty(napi_env env, napi_callback_info info)
{
    napi_value outer = nullptr;
    void* data = nullptr;
    if (!GetThisAndData(env, info, outer, &data) || data == nullptr) {
        return ThrowTargetError(env, "Failed to read TAG UIAbilityContext property callback data.");
    }

    napi_value inner = nullptr;
    if (napi_get_named_property(env, outer, CONTEXT_IMPL_NAME, &inner) != napi_ok || inner == nullptr) {
        return ThrowTargetError(env, "The TAG UIAbilityContext has no inner Context object.");
    }

    napi_value result = nullptr;
    if (napi_get_named_property(env, inner, static_cast<const char*>(data), &result) != napi_ok) {
        return ThrowTargetError(env, "Failed to read a TAG UIAbilityContext property.");
    }
    return result;
}

napi_value ForwardAreaSetter(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value outer = nullptr;
    if (napi_get_cb_info(env, info, &argc, argv, &outer, nullptr) != napi_ok ||
        argc != 1 || argv[0] == nullptr || outer == nullptr) {
        return ThrowTargetError(env, "Failed to read the TAG UIAbilityContext area value.", true);
    }

    napi_value inner = nullptr;
    napi_value switchArea = nullptr;
    napi_value result = nullptr;
    if (napi_get_named_property(env, outer, CONTEXT_IMPL_NAME, &inner) != napi_ok || inner == nullptr ||
        napi_get_named_property(env, inner, "switchArea", &switchArea) != napi_ok || switchArea == nullptr) {
        return ThrowTargetError(env, "Failed to switch the TAG UIAbilityContext area.");
    }
    if (napi_call_function(env, inner, switchArea, 1, argv, &result) != napi_ok) {
        bool pending = false;
        if (napi_is_exception_pending(env, &pending) == napi_ok && pending) {
            return nullptr;
        }
        return ThrowTargetError(env, "Failed to switch the TAG UIAbilityContext area.");
    }
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    return undefined;
}

bool BindOuterForwardedProperties(napi_env env, napi_value outer)
{
    napi_property_descriptor descriptors[sizeof(FORWARDED_PROPERTIES) / sizeof(FORWARDED_PROPERTIES[0])] = {};
    for (size_t index = 0; index < sizeof(FORWARDED_PROPERTIES) / sizeof(FORWARDED_PROPERTIES[0]); ++index) {
        descriptors[index] = {
            FORWARDED_PROPERTIES[index],
            nullptr,
            nullptr,
            ForwardInnerProperty,
            std::strcmp(FORWARDED_PROPERTIES[index], "area") == 0 ? ForwardAreaSetter : nullptr,
            nullptr,
            napi_default,
            const_cast<char*>(FORWARDED_PROPERTIES[index]),
        };
    }
    return napi_define_properties(env, outer,
        sizeof(descriptors) / sizeof(descriptors[0]), descriptors) == napi_ok;
}

bool SetNamedProperty(napi_env env, napi_value object, const char* name, napi_value value)
{
    return value != nullptr && napi_set_named_property(env, object, name, value) == napi_ok;
}

bool HasSameNamedProperty(napi_env env, napi_value first, napi_value second, const char* name)
{
    napi_value firstValue = nullptr;
    napi_value secondValue = nullptr;
    bool same = false;
    return napi_get_named_property(env, first, name, &firstValue) == napi_ok &&
        napi_get_named_property(env, second, name, &secondValue) == napi_ok &&
        napi_strict_equals(env, firstValue, secondValue, &same) == napi_ok && same;
}

bool ClearPendingException(napi_env env)
{
    bool pending = false;
    if (napi_is_exception_pending(env, &pending) != napi_ok) {
        return false;
    }
    if (!pending) {
        return true;
    }
    napi_value exception = nullptr;
    return napi_get_and_clear_last_exception(env, &exception) == napi_ok;
}

bool ValidateAbilityContextInner(napi_env env, napi_value inner,
    const std::shared_ptr<AbilityContext>& abilityContext)
{
    if (inner == nullptr || abilityContext == nullptr) {
        return false;
    }
    void* wrapped = nullptr;
    if (napi_unwrap(env, inner, &wrapped) != napi_ok || wrapped == nullptr) {
        return false;
    }
    auto* jsAbilityContext = static_cast<JsAbilityContext*>(wrapped);
    if (jsAbilityContext->GetAbilityContext() != abilityContext) {
        return false;
    }

    napi_value baseObject = nullptr;
    void* baseContext = nullptr;
    return napi_get_named_property(env, inner, BASE_CONTEXT_NAME, &baseObject) == napi_ok &&
        baseObject != nullptr && napi_unwrap(env, baseObject, &baseContext) == napi_ok &&
        baseContext != nullptr;
}

bool HasRequiredInnerProperty(napi_env env, napi_value inner, const char* name, bool required)
{
    if (!required) {
        return true;
    }
    bool present = false;
    return napi_has_named_property(env, inner, name, &present) == napi_ok && present;
}

bool ValidateFactoryDataProperties(napi_env env, napi_value inner,
    const std::shared_ptr<AbilityContext>& abilityContext)
{
    if (!HasRequiredInnerProperty(env, inner, "applicationInfo",
            abilityContext->GetApplicationInfo() != nullptr) ||
        !HasRequiredInnerProperty(env, inner, "currentHapModuleInfo",
            abilityContext->GetHapModuleInfo() != nullptr) ||
        !HasRequiredInnerProperty(env, inner, "abilityInfo",
            abilityContext->GetAbilityInfo() != nullptr) ||
        !HasRequiredInnerProperty(env, inner, "resourceManager",
            abilityContext->GetResourceManager() != nullptr) ||
        !HasRequiredInnerProperty(env, inner, "config",
            abilityContext->GetConfiguration() != nullptr)) {
        return false;
    }
    if (abilityContext->GetConfiguration() == nullptr) {
        return true;
    }
    napi_value config = nullptr;
    bool hasLocale = false;
    return napi_get_named_property(env, inner, "config", &config) == napi_ok && config != nullptr &&
        napi_has_named_property(env, config, "locale", &hasLocale) == napi_ok && hasLocale;
}

napi_value CreateTagUIAbilityContextHolder(
    napi_env targetEnv, const std::shared_ptr<AbilityContext>& abilityContext)
{
    if (targetEnv == nullptr || abilityContext == nullptr) {
        return ThrowTargetError(targetEnv, "The native UIAbilityContext is invalid.");
    }

    napi_value inner = CreateJsAbilityContext(targetEnv, abilityContext);
    bool pendingException = false;
    if (napi_is_exception_pending(targetEnv, &pendingException) != napi_ok ||
        inner == nullptr || pendingException ||
        !ValidateAbilityContextInner(targetEnv, inner, abilityContext) ||
        !ValidateFactoryDataProperties(targetEnv, inner, abilityContext)) {
        (void)ClearPendingException(targetEnv);
        return ThrowTargetError(targetEnv,
            "Failed to create the TAG UIAbilityContext through CreateJsAbilityContext.");
    }

    napi_value outer = nullptr;
    if (napi_create_object(targetEnv, &outer) != napi_ok || outer == nullptr) {
        return ThrowTargetError(targetEnv, "Failed to create the TAG UIAbilityContext holder.");
    }
    napi_value stageMode = nullptr;
    if (!SetNamedProperty(targetEnv, outer, CONTEXT_IMPL_NAME, inner) ||
        napi_get_boolean(targetEnv, true, &stageMode) != napi_ok ||
        !SetNamedProperty(targetEnv, outer, "stageMode", stageMode) ||
        !BindOuterForwardedProperties(targetEnv, outer)) {
        return ThrowTargetError(targetEnv, "Failed to initialize the TAG UIAbilityContext holder.");
    }
    if (!HasSameNamedProperty(targetEnv, outer, inner, "applicationInfo") ||
        !HasSameNamedProperty(targetEnv, outer, inner, "abilityInfo") ||
        !HasSameNamedProperty(targetEnv, outer, inner, "currentHapModuleInfo") ||
        !HasSameNamedProperty(targetEnv, outer, inner, "config") ||
        !HasSameNamedProperty(targetEnv, outer, inner, "resourceManager")) {
        return ThrowTargetError(targetEnv, "The TAG UIAbilityContext data properties are inconsistent.");
    }
    pendingException = false;
    if (napi_is_exception_pending(targetEnv, &pendingException) != napi_ok || pendingException) {
        if (pendingException) {
            (void)ClearPendingException(targetEnv);
        }
        return ThrowTargetError(targetEnv, "The TAG UIAbilityContext factory left a pending exception.");
    }

    auto* weakAbilityContext = new (std::nothrow) std::weak_ptr<AbilityContext>(abilityContext);
    if (weakAbilityContext == nullptr) {
        return ThrowTargetError(targetEnv, "Failed to allocate the TAG UIAbilityContext holder payload.");
    }
    if (napi_wrap(targetEnv, outer, weakAbilityContext, FinalizeContextHolder, nullptr, nullptr) != napi_ok) {
        delete weakAbilityContext;
        return ThrowTargetError(targetEnv, "Failed to wrap the TAG UIAbilityContext holder.");
    }
    if (!VerifyWrappedPayload(targetEnv, outer, weakAbilityContext) ||
        weakAbilityContext->lock() != abilityContext) {
        return ThrowTargetError(targetEnv, "The TAG UIAbilityContext holder payload is inconsistent.");
    }
    pendingException = false;
    if (napi_is_exception_pending(targetEnv, &pendingException) != napi_ok || pendingException) {
        if (pendingException) {
            (void)ClearPendingException(targetEnv);
        }
        return ThrowTargetError(targetEnv, "The TAG UIAbilityContext holder left a pending exception.");
    }

    TAG_LOGI(AAFwkTag::CONTEXT, "TAG UIAbilityContext holder created");
    return outer;
}

napi_value CreateUIAbilityContextHolder(napi_env targetEnv, napi_callback_info info)
{
    if (targetEnv == nullptr || info == nullptr) {
        return nullptr;
    }

    size_t argc = HYBRID_MAX_ARGC;
    napi_value argv[HYBRID_MAX_ARGC] = { nullptr };
    if (napi_get_cb_info(targetEnv, info, &argc, argv, nullptr, nullptr) != napi_ok) {
        return ThrowTargetError(targetEnv, "Failed to read UIAbilityContext interop arguments.");
    }
    if (argc != HYBRID_ARGC) {
        return ThrowTargetError(targetEnv,
            "createUIAbilityContextHolder requires sourceEnv and sourceValue.", true);
    }

    void* sourceEnvPointer = nullptr;
    void* sourceValuePointer = nullptr;
    if (!GetExternalPointer(targetEnv, argv[0], &sourceEnvPointer) ||
        !GetExternalPointer(targetEnv, argv[1], &sourceValuePointer)) {
        return ThrowTargetError(targetEnv,
            "sourceEnv and sourceValue must be non-null external values.", true);
    }

    auto sourceEnv = reinterpret_cast<napi_env>(sourceEnvPointer);
    auto sourceValue = reinterpret_cast<napi_value>(sourceValuePointer);
    napi_valuetype sourceType = napi_undefined;
    if (napi_typeof(sourceEnv, sourceValue, &sourceType) != napi_ok || sourceType != napi_object) {
        return ThrowTargetError(targetEnv, "The source UIAbilityContext is not an object.", true);
    }

    void* wrapped = nullptr;
    if (napi_unwrap(sourceEnv, sourceValue, &wrapped) != napi_ok || wrapped == nullptr) {
        return ThrowTargetError(targetEnv, "Failed to unwrap the source UIAbilityContext.");
    }
    // Private bridge contract: sourceValue is the outer UIAbilityContext
    // created by AbilityRuntime, whose payload is weak_ptr<AbilityContext>.
    auto abilityContext = static_cast<std::weak_ptr<AbilityContext>*>(wrapped)->lock();
    if (abilityContext == nullptr) {
        return ThrowTargetError(targetEnv, "The source UIAbilityContext has expired.");
    }
    return CreateTagUIAbilityContextHolder(targetEnv, abilityContext);
}

napi_value CreateUIAbilityContextHolderFromFfi(napi_env targetEnv, napi_callback_info info)
{
    if (targetEnv == nullptr || info == nullptr) {
        return nullptr;
    }

    size_t argc = FFI_MAX_ARGC;
    napi_value argv[FFI_MAX_ARGC] = { nullptr };
    if (napi_get_cb_info(targetEnv, info, &argc, argv, nullptr, nullptr) != napi_ok) {
        return ThrowTargetError(targetEnv, "Failed to read the FFI UIAbilityContext argument.");
    }
    if (argc != FFI_ARGC) {
        return ThrowTargetError(targetEnv,
            "createUIAbilityContextHolderFromFfi requires an FFI id external.", true);
    }

    void* ffiIdPointer = nullptr;
    if (!GetExternalPointer(targetEnv, argv[0], &ffiIdPointer)) {
        return ThrowTargetError(targetEnv, "ffiId must be a non-null external value.", true);
    }
    const int64_t ffiId = *static_cast<int64_t*>(ffiIdPointer);
    auto cjContext = FFI::FFIData::GetData<CJAbilityContext>(ffiId);
    if (cjContext == nullptr) {
        return ThrowTargetError(targetEnv, "The FFI id is not a UIAbilityContext.", true);
    }
    auto abilityContext = cjContext->GetAbilityContext();
    if (abilityContext == nullptr) {
        return ThrowTargetError(targetEnv, "The FFI UIAbilityContext has expired.");
    }
    return CreateTagUIAbilityContextHolder(targetEnv, abilityContext);
}

napi_value Init(napi_env env, napi_value exports)
{
    if (env == nullptr || exports == nullptr) {
        return nullptr;
    }

    napi_property_descriptor properties[] = {
        DECLARE_NAPI_FUNCTION(CREATE_HOLDER_FUNCTION, CreateUIAbilityContextHolder),
        DECLARE_NAPI_FUNCTION(CREATE_HOLDER_FROM_FFI_FUNCTION, CreateUIAbilityContextHolderFromFfi),
    };
    napi_status status =
        napi_define_properties(env, exports, sizeof(properties) / sizeof(properties[0]), properties);
    if (status != napi_ok) {
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
