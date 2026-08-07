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

#include <cstring>
#include <memory>
#include <new>
#include <vector>

#include "ability_runtime/js_ability_context.h"
#include "hilog_tag_wrapper.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t EXPECTED_ARGC = 2;
constexpr size_t MAX_ARGC = EXPECTED_ARGC + 1;
constexpr char CREATE_HOLDER_FUNCTION[] = "createUIAbilityContextHolder";
constexpr char CONTEXT_IMPL_PROPERTY[] = "__context_impl__";
constexpr char BASE_CONTEXT_PROPERTY[] = "__base_context_ptr__";
constexpr char IS_TERMINATING_METHOD[] = "isTerminating";

// Keep only methods whose public JS wrapper directly delegates to __context_impl__.
// Context factories, ResourceManager factories, and startAbilityByCall* require
// additional wrapper semantics and are intentionally excluded.
constexpr const char* FORWARDED_METHODS[] = {
    "getGroupDir",
    "startAbility",
    "openLink",
    "startAbilityAsCaller",
    "startRecentAbility",
    "startAbilityWithAccount",
    "startAbilityForResult",
    "startAbilityForResultWithAccount",
    "startUIServiceExtensionAbility",
    "connectUIServiceExtensionAbility",
    "disconnectUIServiceExtensionAbility",
    "startServiceExtensionAbility",
    "startServiceExtensionAbilityWithAccount",
    "stopServiceExtensionAbility",
    "stopServiceExtensionAbilityWithAccount",
    "connectServiceExtensionAbility",
    "connectAbilityWithAccount",
    "connectServiceExtensionAbilityWithAccount",
    "disconnectAbility",
    "disconnectServiceExtensionAbility",
    "terminateSelf",
    "isTerminating",
    "terminateSelfWithResult",
    "backToCallerAbilityWithResult",
    "restoreWindowStage",
    "setMissionContinueState",
    "setMissionLabel",
    "setMissionIcon",
    "setMissionWindowIcon",
    "setAbilityInstanceInfo",
    "requestDialogService",
    "reportDrawnCompleted",
    "startAbilityByType",
    "requestModalUIExtension",
    "showAbility",
    "hideAbility",
    "openAtomicService",
    "moveAbilityToBackground",
    "setRestoreEnabled",
    "setColorMode",
    "revokeDelegator",
    "startAppServiceExtensionAbility",
    "stopAppServiceExtensionAbility",
    "connectAppServiceExtensionAbility",
    "disconnectAppServiceExtensionAbility",
    "setOnNewWantSkipScenarios",
    "startSelfUIAbilityInCurrentProcess",
    "restartApp",
};

constexpr const char* FORWARDED_PROPERTIES[] = {
    "applicationInfo",
    "abilityInfo",
    "currentHapModuleInfo",
    "config",
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
};

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

napi_value ThrowForwardError(napi_env env, const char* message)
{
    bool pending = false;
    if (env != nullptr && napi_is_exception_pending(env, &pending) == napi_ok && pending) {
        return nullptr;
    }
    return ThrowTargetError(env, message);
}

bool GetInnerObject(napi_env env, napi_value holder, napi_value& inner)
{
    if (env == nullptr || holder == nullptr ||
        napi_get_named_property(env, holder, CONTEXT_IMPL_PROPERTY, &inner) != napi_ok || inner == nullptr) {
        ThrowForwardError(env, "Failed to get UIAbilityContext.__context_impl__.");
        return false;
    }

    napi_valuetype type = napi_undefined;
    if (napi_typeof(env, inner, &type) != napi_ok || type != napi_object) {
        ThrowForwardError(env, "UIAbilityContext.__context_impl__ is not an object.");
        return false;
    }
    return true;
}

napi_value ForwardMethod(napi_env env, napi_callback_info info)
{
    if (env == nullptr || info == nullptr) {
        return nullptr;
    }

    size_t argc = 0;
    napi_value holder = nullptr;
    void* data = nullptr;
    if (napi_get_cb_info(env, info, &argc, nullptr, &holder, &data) != napi_ok ||
        holder == nullptr || data == nullptr) {
        return ThrowForwardError(env, "Failed to read UIAbilityContext method arguments.");
    }

    std::vector<napi_value> argv(argc, nullptr);
    if (argc > 0 && napi_get_cb_info(env, info, &argc, argv.data(), &holder, &data) != napi_ok) {
        return ThrowForwardError(env, "Failed to read UIAbilityContext method arguments.");
    }

    napi_value inner = nullptr;
    if (!GetInnerObject(env, holder, inner)) {
        return nullptr;
    }

    const auto* methodName = static_cast<const char*>(data);
    napi_value method = nullptr;
    napi_valuetype methodType = napi_undefined;
    if (napi_get_named_property(env, inner, methodName, &method) != napi_ok || method == nullptr ||
        napi_typeof(env, method, &methodType) != napi_ok || methodType != napi_function) {
        TAG_LOGE(AAFwkTag::CONTEXT, "inner UIAbilityContext method is unavailable: %{public}s", methodName);
        return ThrowForwardError(env, "The requested UIAbilityContext method is unavailable.");
    }

    napi_value result = nullptr;
    napi_status status = napi_call_function(env, inner, method, argc, argc == 0 ? nullptr : argv.data(), &result);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "forward UIAbilityContext method failed: %{public}s, %{public}d",
            methodName, status);
        return ThrowForwardError(env, "Failed to forward the UIAbilityContext method.");
    }
    return result;
}

napi_value GetForwardedProperty(napi_env env, napi_callback_info info)
{
    napi_value holder = nullptr;
    void* data = nullptr;
    if (env == nullptr || info == nullptr ||
        napi_get_cb_info(env, info, nullptr, nullptr, &holder, &data) != napi_ok ||
        holder == nullptr || data == nullptr) {
        return ThrowForwardError(env, "Failed to read the UIAbilityContext property.");
    }

    napi_value inner = nullptr;
    if (!GetInnerObject(env, holder, inner)) {
        return nullptr;
    }

    napi_value result = nullptr;
    const auto* propertyName = static_cast<const char*>(data);
    if (napi_get_named_property(env, inner, propertyName, &result) != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "forward UIAbilityContext property failed: %{public}s", propertyName);
        return ThrowForwardError(env, "Failed to forward the UIAbilityContext property.");
    }
    return result;
}

napi_value SetArea(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value argv[1] = { nullptr };
    napi_value holder = nullptr;
    if (env == nullptr || info == nullptr ||
        napi_get_cb_info(env, info, &argc, argv, &holder, nullptr) != napi_ok || argc != 1) {
        return ThrowForwardError(env, "Failed to read UIAbilityContext.area.");
    }

    napi_value inner = nullptr;
    if (!GetInnerObject(env, holder, inner)) {
        return nullptr;
    }

    napi_value switchArea = nullptr;
    napi_valuetype type = napi_undefined;
    if (napi_get_named_property(env, inner, "switchArea", &switchArea) != napi_ok || switchArea == nullptr ||
        napi_typeof(env, switchArea, &type) != napi_ok || type != napi_function) {
        return ThrowForwardError(env, "UIAbilityContext.switchArea is unavailable.");
    }

    napi_value result = nullptr;
    if (napi_call_function(env, inner, switchArea, 1, argv, &result) != napi_ok) {
        return ThrowForwardError(env, "Failed to set UIAbilityContext.area.");
    }
    return result;
}

napi_value GetArea(napi_env env, napi_callback_info info)
{
    napi_value holder = nullptr;
    if (env == nullptr || info == nullptr ||
        napi_get_cb_info(env, info, nullptr, nullptr, &holder, nullptr) != napi_ok || holder == nullptr) {
        return ThrowForwardError(env, "Failed to read UIAbilityContext.area.");
    }

    napi_value inner = nullptr;
    if (!GetInnerObject(env, holder, inner)) {
        return nullptr;
    }

    napi_value getArea = nullptr;
    napi_valuetype type = napi_undefined;
    if (napi_get_named_property(env, inner, "getArea", &getArea) != napi_ok || getArea == nullptr ||
        napi_typeof(env, getArea, &type) != napi_ok || type != napi_function) {
        return ThrowForwardError(env, "UIAbilityContext.getArea is unavailable.");
    }

    napi_value result = nullptr;
    if (napi_call_function(env, inner, getArea, 0, nullptr, &result) != napi_ok) {
        return ThrowForwardError(env, "Failed to get UIAbilityContext.area.");
    }
    return result;
}

napi_value GetStageMode(napi_env env, napi_callback_info)
{
    napi_value stageMode = nullptr;
    if (env == nullptr || napi_get_boolean(env, true, &stageMode) != napi_ok) {
        return ThrowForwardError(env, "Failed to get UIAbilityContext.stageMode.");
    }
    return stageMode;
}

bool ValidateInnerObject(napi_env env, napi_value inner)
{
    void* abilityContext = nullptr;
    if (napi_unwrap(env, inner, &abilityContext) != napi_ok || abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "unwrap inner JsAbilityContext failed");
        return false;
    }

    napi_value baseContextObject = nullptr;
    void* baseContext = nullptr;
    if (napi_get_named_property(env, inner, BASE_CONTEXT_PROPERTY, &baseContextObject) != napi_ok ||
        baseContextObject == nullptr || napi_unwrap(env, baseContextObject, &baseContext) != napi_ok ||
        baseContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "unwrap inner JsBaseContext failed");
        return false;
    }
    return true;
}

bool BindForwardedMethods(napi_env env, napi_value holder, napi_value inner)
{
    bool hasIsTerminating = false;
    for (const char* methodName : FORWARDED_METHODS) {
        napi_value method = nullptr;
        napi_valuetype type = napi_undefined;
        if (napi_get_named_property(env, inner, methodName, &method) != napi_ok || method == nullptr ||
            napi_typeof(env, method, &type) != napi_ok) {
            return false;
        }
        if (type != napi_function) {
            TAG_LOGD(AAFwkTag::CONTEXT, "skip unavailable inner method: %{public}s", methodName);
            continue;
        }

        napi_property_descriptor descriptor = {
            methodName, nullptr, ForwardMethod, nullptr, nullptr, nullptr, napi_default,
            const_cast<char*>(methodName)
        };
        if (napi_define_properties(env, holder, 1, &descriptor) != napi_ok) {
            TAG_LOGE(AAFwkTag::CONTEXT, "define forwarded method failed: %{public}s", methodName);
            return false;
        }
        hasIsTerminating = hasIsTerminating || std::strcmp(methodName, IS_TERMINATING_METHOD) == 0;
    }
    return hasIsTerminating;
}

bool BindForwardedProperties(napi_env env, napi_value holder)
{
    std::vector<napi_property_descriptor> descriptors;
    descriptors.reserve(sizeof(FORWARDED_PROPERTIES) / sizeof(FORWARDED_PROPERTIES[0]) + 2);
    for (const char* propertyName : FORWARDED_PROPERTIES) {
        descriptors.push_back({
            propertyName, nullptr, nullptr, GetForwardedProperty, nullptr, nullptr, napi_default,
            const_cast<char*>(propertyName)
        });
    }
    descriptors.push_back({
        "area", nullptr, nullptr, GetArea, SetArea, nullptr, napi_default, nullptr
    });
    descriptors.push_back({
        "stageMode", nullptr, nullptr, GetStageMode, nullptr, nullptr, napi_default, nullptr
    });
    return napi_define_properties(env, holder, descriptors.size(), descriptors.data()) == napi_ok;
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
    delete static_cast<std::weak_ptr<AbilityContext>*>(data);
}

napi_value CreateTagUIAbilityContextHolder(napi_env targetEnv,
    const std::shared_ptr<AbilityContext>& abilityContext);

void* DetachTagUIAbilityContext(napi_env, void* nativeObject, void*)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "detach TAG UIAbilityContext holder");
    // Match the initial ArkTS CreateNativeReference contract: NativeBindingInfo
    // borrows the payload whose lifetime is owned by the outer napi_wrap.
    return nativeObject;
}

napi_value AttachTagUIAbilityContext(napi_env targetEnv, void* nativeObject, void*)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "attach TAG UIAbilityContext holder");
    if (targetEnv == nullptr || nativeObject == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "attach TAG UIAbilityContext with null parameter");
        return ThrowTargetError(targetEnv, "Failed to attach the TAG UIAbilityContext holder.");
    }

    auto abilityContext = static_cast<std::weak_ptr<AbilityContext>*>(nativeObject)->lock();
    if (abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "attach TAG UIAbilityContext with expired context");
        return ThrowTargetError(targetEnv, "The UIAbilityContext has expired while attaching the TAG holder.");
    }
    return CreateTagUIAbilityContextHolder(targetEnv, abilityContext);
}

napi_value CreateTagUIAbilityContextHolder(napi_env targetEnv,
    const std::shared_ptr<AbilityContext>& abilityContext)
{
    if (targetEnv == nullptr || abilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "create TAG UIAbilityContext with null parameter");
        return ThrowTargetError(targetEnv, "Failed to create the TAG UIAbilityContext holder.");
    }

    napi_value inner = CreateJsAbilityContext(targetEnv, abilityContext);
    if (inner == nullptr || !ValidateInnerObject(targetEnv, inner)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "create inner TAG UIAbilityContext failed");
        return ThrowForwardError(targetEnv, "Failed to create the inner TAG UIAbilityContext.");
    }

    napi_value holder = nullptr;
    if (napi_create_object(targetEnv, &holder) != napi_ok || holder == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "create target holder failed");
        return ThrowTargetError(targetEnv, "Failed to create the target UIAbilityContext holder.");
    }

    if (napi_set_named_property(targetEnv, holder, CONTEXT_IMPL_PROPERTY, inner) != napi_ok ||
        !BindForwardedProperties(targetEnv, holder) || !BindForwardedMethods(targetEnv, holder, inner)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "initialize forwarded target holder failed");
        return ThrowForwardError(targetEnv, "Failed to initialize the target UIAbilityContext holder.");
    }

    auto* weakAbilityContext = new (std::nothrow) std::weak_ptr<AbilityContext>(abilityContext);
    if (weakAbilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "allocate target weak ability context failed");
        return ThrowTargetError(targetEnv, "Failed to allocate the target UIAbilityContext holder.");
    }

    napi_status status = napi_coerce_to_native_binding_object(targetEnv, holder, DetachTagUIAbilityContext,
        AttachTagUIAbilityContext, weakAbilityContext, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "coerce target holder to native binding object failed: %{public}d", status);
        delete weakAbilityContext;
        return ThrowTargetError(targetEnv, "Failed to bind the target UIAbilityContext holder.");
    }

    status = napi_wrap(targetEnv, holder, weakAbilityContext, FinalizeContextHolder, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "wrap target holder failed: %{public}d", status);
        delete weakAbilityContext;
        return ThrowTargetError(targetEnv, "Failed to wrap the target UIAbilityContext holder.");
    }

    void* wrappedHolder = nullptr;
    if (napi_unwrap(targetEnv, holder, &wrappedHolder) != napi_ok || wrappedHolder != weakAbilityContext) {
        TAG_LOGE(AAFwkTag::CONTEXT, "validate target weak ability context failed");
        return ThrowTargetError(targetEnv, "Failed to validate the target UIAbilityContext holder.");
    }

    TAG_LOGI(AAFwkTag::CONTEXT, "target UIAbilityContext holder created with inner native context");
    return holder;
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
    return CreateTagUIAbilityContextHolder(targetEnv, abilityContext);
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
