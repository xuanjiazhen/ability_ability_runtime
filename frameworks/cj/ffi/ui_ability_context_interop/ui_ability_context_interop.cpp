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
#include <memory>
#include <new>

#include "cj_ability_context.h"
#include "ffi_remote_data.h"
#include "hilog_tag_wrapper.h"
#include "js_ability_context.h"
#include "js_data_struct_converter.h"
#include "native_engine/native_engine.h"

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

enum class TagContextProperty {
    CACHE_DIR,
    TEMP_DIR,
    RESOURCE_DIR,
    FILES_DIR,
    DISTRIBUTED_FILES_DIR,
    DATABASE_DIR,
    PREFERENCES_DIR,
    BUNDLE_CODE_DIR,
    CLOUD_FILE_DIR,
    LOG_FILE_DIR,
    PROCESS_NAME,
    AREA,
};

struct TagContextPropertySpec {
    const char* name;
    TagContextProperty property;
};

constexpr TagContextPropertySpec TAG_CONTEXT_PROPERTIES[] = {
    { "cacheDir", TagContextProperty::CACHE_DIR },
    { "tempDir", TagContextProperty::TEMP_DIR },
    { "resourceDir", TagContextProperty::RESOURCE_DIR },
    { "filesDir", TagContextProperty::FILES_DIR },
    { "distributedFilesDir", TagContextProperty::DISTRIBUTED_FILES_DIR },
    { "databaseDir", TagContextProperty::DATABASE_DIR },
    { "preferencesDir", TagContextProperty::PREFERENCES_DIR },
    { "bundleCodeDir", TagContextProperty::BUNDLE_CODE_DIR },
    { "cloudFileDir", TagContextProperty::CLOUD_FILE_DIR },
    { "logFileDir", TagContextProperty::LOG_FILE_DIR },
    { "processName", TagContextProperty::PROCESS_NAME },
    { "area", TagContextProperty::AREA },
};

constexpr const char* FORWARDED_PROPERTIES[] = {
    "applicationInfo",
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

class TagJsBaseContext final {
public:
    explicit TagJsBaseContext(const std::shared_ptr<Context>& context) : context_(context) {}
    ~TagJsBaseContext() = default;

    std::shared_ptr<Context> GetContext() const
    {
        return context_.lock();
    }

private:
    std::weak_ptr<Context> context_;
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

void FinalizeBaseContext(napi_env, void* data, void*)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "finalize TAG base context");
    delete static_cast<TagJsBaseContext*>(data);
}

void FinalizeContextHolder(napi_env, void* data, void*)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "finalize UIAbilityContext interop holder");
    delete static_cast<std::weak_ptr<AbilityContext>*>(data);
}

TagJsBaseContext* GetBaseContextPayload(napi_env env, napi_value inner)
{
    napi_value baseObject = nullptr;
    if (napi_get_named_property(env, inner, BASE_CONTEXT_NAME, &baseObject) != napi_ok || baseObject == nullptr) {
        return nullptr;
    }

    void* wrapped = nullptr;
    if (napi_unwrap(env, baseObject, &wrapped) != napi_ok) {
        return nullptr;
    }
    return static_cast<TagJsBaseContext*>(wrapped);
}

napi_value GetTagContextProperty(napi_env env, napi_callback_info info)
{
    napi_value inner = nullptr;
    void* data = nullptr;
    if (!GetThisAndData(env, info, inner, &data) || data == nullptr) {
        return ThrowTargetError(env, "Failed to read TAG Context property callback data.");
    }

    auto* baseContext = GetBaseContextPayload(env, inner);
    auto context = baseContext == nullptr ? nullptr : baseContext->GetContext();
    if (context == nullptr) {
        return ThrowTargetError(env, "The native Context has expired.");
    }

    auto* spec = static_cast<const TagContextPropertySpec*>(data);
    napi_value result = nullptr;
    if (spec->property == TagContextProperty::AREA) {
        if (napi_create_int32(env, context->GetArea(), &result) != napi_ok) {
            return ThrowTargetError(env, "Failed to create the TAG Context area value.");
        }
        return result;
    }

    std::string value;
    switch (spec->property) {
        case TagContextProperty::CACHE_DIR:
            value = context->GetCacheDir();
            break;
        case TagContextProperty::TEMP_DIR:
            value = context->GetTempDir();
            break;
        case TagContextProperty::RESOURCE_DIR:
            value = context->GetResourceDir();
            break;
        case TagContextProperty::FILES_DIR:
            value = context->GetFilesDir();
            break;
        case TagContextProperty::DISTRIBUTED_FILES_DIR:
            value = context->GetDistributedFilesDir();
            break;
        case TagContextProperty::DATABASE_DIR:
            value = context->GetDatabaseDir();
            break;
        case TagContextProperty::PREFERENCES_DIR:
            value = context->GetPreferencesDir();
            break;
        case TagContextProperty::BUNDLE_CODE_DIR:
            value = context->GetBundleCodeDir();
            break;
        case TagContextProperty::CLOUD_FILE_DIR:
            value = context->GetCloudFileDir();
            break;
        case TagContextProperty::LOG_FILE_DIR:
            value = context->GetLogFileDir();
            break;
        case TagContextProperty::PROCESS_NAME:
            value = context->GetProcessName();
            break;
        case TagContextProperty::AREA:
            break;
    }

    if (napi_create_string_utf8(env, value.c_str(), NAPI_AUTO_LENGTH, &result) != napi_ok) {
        return ThrowTargetError(env, "Failed to create a TAG Context string property.");
    }
    return result;
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

bool BindInnerContextProperties(napi_env env, napi_value inner)
{
    napi_property_descriptor descriptors[sizeof(TAG_CONTEXT_PROPERTIES) / sizeof(TAG_CONTEXT_PROPERTIES[0])] = {};
    for (size_t index = 0; index < sizeof(TAG_CONTEXT_PROPERTIES) / sizeof(TAG_CONTEXT_PROPERTIES[0]); ++index) {
        descriptors[index] = {
            TAG_CONTEXT_PROPERTIES[index].name,
            nullptr,
            nullptr,
            GetTagContextProperty,
            nullptr,
            nullptr,
            napi_default,
            const_cast<TagContextPropertySpec*>(&TAG_CONTEXT_PROPERTIES[index]),
        };
    }
    return napi_define_properties(env, inner,
        sizeof(descriptors) / sizeof(descriptors[0]), descriptors) == napi_ok;
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
            nullptr,
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

bool SetInnerDataProperties(napi_env env, napi_value inner, const std::shared_ptr<AbilityContext>& abilityContext)
{
    auto appInfo = abilityContext->GetApplicationInfo();
    if (appInfo != nullptr &&
        !SetNamedProperty(env, inner, "applicationInfo", CreateJsApplicationInfo(env, *appInfo))) {
        return false;
    }

    auto hapModuleInfo = abilityContext->GetHapModuleInfo();
    if (hapModuleInfo != nullptr &&
        !SetNamedProperty(env, inner, "currentHapModuleInfo", CreateJsHapModuleInfo(env, *hapModuleInfo))) {
        return false;
    }

    auto abilityInfo = abilityContext->GetAbilityInfo();
    if (abilityInfo != nullptr &&
        !SetNamedProperty(env, inner, "abilityInfo", CreateJsAbilityInfo(env, *abilityInfo))) {
        return false;
    }

    napi_value contextType = nullptr;
    if (napi_create_string_utf8(env, "UIAbilityContext", NAPI_AUTO_LENGTH, &contextType) != napi_ok ||
        !SetNamedProperty(env, inner, "contextType", contextType)) {
        return false;
    }
    return true;
}

bool CopyInnerPropertyIfPresent(napi_env env, napi_value outer, napi_value inner, const char* name)
{
    bool hasProperty = false;
    if (napi_has_named_property(env, inner, name, &hasProperty) != napi_ok) {
        return false;
    }
    if (!hasProperty) {
        return true;
    }

    napi_value value = nullptr;
    return napi_get_named_property(env, inner, name, &value) == napi_ok &&
        SetNamedProperty(env, outer, name, value);
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

napi_value CreateTagUIAbilityContextHolder(
    napi_env targetEnv, const std::shared_ptr<AbilityContext>& abilityContext)
{
    if (targetEnv == nullptr || abilityContext == nullptr) {
        return ThrowTargetError(targetEnv, "The native UIAbilityContext is invalid.");
    }

    napi_value baseObject = nullptr;
    if (napi_create_object(targetEnv, &baseObject) != napi_ok || baseObject == nullptr) {
        return ThrowTargetError(targetEnv, "Failed to create the TAG BaseContext object.");
    }
    auto* baseContext = new (std::nothrow) TagJsBaseContext(abilityContext);
    if (baseContext == nullptr) {
        return ThrowTargetError(targetEnv, "Failed to allocate the TAG BaseContext payload.");
    }
    if (napi_wrap(targetEnv, baseObject, baseContext, FinalizeBaseContext, nullptr, nullptr) != napi_ok) {
        delete baseContext;
        return ThrowTargetError(targetEnv, "Failed to wrap the TAG BaseContext object.");
    }
    if (!VerifyWrappedPayload(targetEnv, baseObject, baseContext) ||
        baseContext->GetContext() != abilityContext) {
        return ThrowTargetError(targetEnv, "The TAG BaseContext payload is inconsistent.");
    }

    napi_value inner = nullptr;
    if (napi_create_object(targetEnv, &inner) != napi_ok || inner == nullptr) {
        return ThrowTargetError(targetEnv, "Failed to create the TAG UIAbilityContext inner object.");
    }
    auto* jsAbilityContext = new (std::nothrow) JsAbilityContext(abilityContext);
    if (jsAbilityContext == nullptr) {
        return ThrowTargetError(targetEnv, "Failed to allocate the TAG JsAbilityContext payload.");
    }
    if (napi_wrap(targetEnv, inner, jsAbilityContext, JsAbilityContext::Finalizer, nullptr, nullptr) != napi_ok) {
        delete jsAbilityContext;
        return ThrowTargetError(targetEnv, "Failed to wrap the TAG UIAbilityContext inner object.");
    }
    if (!VerifyWrappedPayload(targetEnv, inner, jsAbilityContext) ||
        jsAbilityContext->GetAbilityContext() != abilityContext) {
        return ThrowTargetError(targetEnv, "The TAG JsAbilityContext payload is inconsistent.");
    }
    if (!SetNamedProperty(targetEnv, inner, BASE_CONTEXT_NAME, baseObject) ||
        !BindInnerContextProperties(targetEnv, inner) ||
        !SetInnerDataProperties(targetEnv, inner, abilityContext)) {
        return ThrowTargetError(targetEnv, "Failed to initialize the TAG UIAbilityContext inner object.");
    }

    napi_value outer = nullptr;
    if (napi_create_object(targetEnv, &outer) != napi_ok || outer == nullptr) {
        return ThrowTargetError(targetEnv, "Failed to create the TAG UIAbilityContext holder.");
    }
    napi_value stageMode = nullptr;
    if (!SetNamedProperty(targetEnv, outer, CONTEXT_IMPL_NAME, inner) ||
        napi_get_boolean(targetEnv, true, &stageMode) != napi_ok ||
        !SetNamedProperty(targetEnv, outer, "stageMode", stageMode) ||
        !BindOuterForwardedProperties(targetEnv, outer) ||
        !CopyInnerPropertyIfPresent(targetEnv, outer, inner, "abilityInfo") ||
        !CopyInnerPropertyIfPresent(targetEnv, outer, inner, "currentHapModuleInfo")) {
        return ThrowTargetError(targetEnv, "Failed to initialize the TAG UIAbilityContext holder.");
    }
    if (!HasSameNamedProperty(targetEnv, outer, inner, "applicationInfo") ||
        !HasSameNamedProperty(targetEnv, outer, inner, "abilityInfo") ||
        !HasSameNamedProperty(targetEnv, outer, inner, "currentHapModuleInfo")) {
        return ThrowTargetError(targetEnv, "The TAG UIAbilityContext data properties are inconsistent.");
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
