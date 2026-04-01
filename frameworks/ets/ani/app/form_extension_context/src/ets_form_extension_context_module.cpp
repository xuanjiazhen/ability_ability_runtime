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

#include "ets_form_extension_context_module.h"

#include <cstdio>
#include <sys/syscall.h>
#include <unistd.h>

#include "ani_base_context.h"
#include "ani_common_util.h"
#include "context_transfer.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "ets_form_extension_context.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "interop_js/arkts_esvalue.h"
#include "interop_js/arkts_interop_js_api.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include "js_form_extension_context.h"
#include "js_runtime_utils.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *ETS_FORM_EXTENSION_CONTEXT_CLASS_NAME =
    "application.FormExtensionContext.FormExtensionContext";
} // namespace

ani_object EtsFormExtensionContextModule::NativeTransferStatic(ani_env *aniEnv, ani_object, ani_object input,
    ani_object type)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::CONTEXT, "transfer static FormExtensionContext");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null aniEnv");
        return nullptr;
    }

    void *unwrapResult = nullptr;
    bool success = arkts_esvalue_unwrap(aniEnv, input, &unwrapResult);
    if (!success) {
        TAG_LOGE(AAFwkTag::CONTEXT, "failed to unwrap");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }
    if (unwrapResult == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null unwrapResult");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto context = reinterpret_cast<std::weak_ptr<FormExtensionContext> *>(unwrapResult)->lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null FormExtensionContext");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto formExtensionContext = Context::ConvertTo<FormExtensionContext>(context);
    if (formExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "invalid formExtensionContext");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto &bindingObj = formExtensionContext->GetBindingObject();
    if (bindingObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null bindingObj");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto staticContext = bindingObj->Get<ani_ref>();
    if (staticContext != nullptr) {
        TAG_LOGI(AAFwkTag::CONTEXT, "there exist a staticContext");
        return reinterpret_cast<ani_object>(*staticContext);
    }

    // if not exist, create a new one
    auto newContext = CreateStaticObject(aniEnv, type, context);
    if (newContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "newContext invalid");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    return newContext;
}

ani_object EtsFormExtensionContextModule::CreateStaticObject(ani_env *aniEnv, ani_object type,
    std::shared_ptr<FormExtensionContext> formExtensionContext)
{
    std::string contextType;
    if (!AppExecFwk::GetStdString(aniEnv, reinterpret_cast<ani_string>(type), contextType)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetStdString failed");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "contextType %{public}s", contextType.c_str());

    auto newContext = ContextTransfer::GetInstance().GetStaticObject(contextType, aniEnv, formExtensionContext);
    if (newContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "create formExtensionContext failed");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    return newContext;
}

std::unique_ptr<NativeReference> EtsFormExtensionContextModule::CreateNativeReference(napi_env napiEnv,
    std::shared_ptr<FormExtensionContext> formExtensionContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (napiEnv == nullptr || formExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null param");
        return nullptr;
    }

    auto value = CreateJsFormExtensionContext(napiEnv, formExtensionContext);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(napiEnv, "application.FormExtensionContext", &value,
        1);
    if (systemModule == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null systemModule");
        return nullptr;
    }

    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(napiEnv, object, napi_object)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "check type failed");
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<FormExtensionContext>(formExtensionContext);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null workContext");
        return nullptr;
    }
    auto status = napi_coerce_to_native_binding_object(napiEnv, object, DetachCallbackFunc,
        AttachFormExtensionContext, workContext, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "coerce FormExtensionContext failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    status = napi_wrap(napiEnv, object, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::CONTEXT, "finalizer for weak_ptr FormExtensionContext");
            delete static_cast<std::weak_ptr<FormExtensionContext> *>(data);
        }, nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "wrap failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return systemModule;
}

napi_value EtsFormExtensionContextModule::GetOrCreateDynamicObject(napi_env napiEnv,
    std::shared_ptr<FormExtensionContext> formExtensionContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (napiEnv == nullptr || formExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null param");
        return nullptr;
    }

    // if sub-thread, create a new formExtensionContext and return
    if (getpid() != syscall(SYS_gettid)) {
        auto subThreadObj = static_cast<NativeReference *>(
            formExtensionContext->GetSubThreadObject(static_cast<void *>(napiEnv)));
        if (subThreadObj != nullptr) {
            return subThreadObj->Get();
        }
        auto subThreadRef = CreateNativeReference(napiEnv, formExtensionContext);
        if (subThreadRef == nullptr) {
            return nullptr;
        }
        auto newObject = subThreadRef->Get();
        formExtensionContext->BindSubThreadObject(
            static_cast<void *>(napiEnv), static_cast<void *>(subThreadRef.release()));
        return newObject;
    }

    // if main-thread, get bindingObj firstly
    auto &bindingObj = formExtensionContext->GetBindingObject();
    if (bindingObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null bindingObj");
        return nullptr;
    }

    // if main-thread bindingObj exist, return it directly
    auto dynamicContext = bindingObj->Get<NativeReference>();
    if (dynamicContext != nullptr) {
        TAG_LOGI(AAFwkTag::UIABILITY, "there exist a dynamicContext");
        return dynamicContext->Get();
    }

    // if main-thread bindingObj didn't exist, create and bind
    auto nativeRef = CreateNativeReference(napiEnv, formExtensionContext);
    if (nativeRef == nullptr) {
        return nullptr;
    }

    auto object = nativeRef->Get();
    formExtensionContext->Bind(nativeRef.release());
    return object;
}

ani_object EtsFormExtensionContextModule::NativeTransferDynamic(ani_env *aniEnv, ani_class aniCls,
    ani_object input)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::CONTEXT, "transfer dynamic FormExtensionContext");
    if (!IsInstanceOf(aniEnv, input)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "not FormExtensionContext");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto context = AbilityRuntime::GetStageModeContext(aniEnv, input);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null context");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    std::shared_ptr<FormExtensionContext> formExtensionContext =
        Context::ConvertTo<FormExtensionContext>(context);
    if (formExtensionContext == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "invalid formExtensionContext");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    ani_object object = CreateDynamicObject(aniEnv, aniCls, formExtensionContext);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "invalid object");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    return object;
}

ani_object EtsFormExtensionContextModule::CreateDynamicObject(ani_env *aniEnv, ani_class aniCls,
    std::shared_ptr<FormExtensionContext> formExtensionContext)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string contextType;
    if (!AppExecFwk::GetStaticFieldString(aniEnv, aniCls, "contextType", contextType)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "get context type failed");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::CONTEXT, "contextType %{public}s", contextType.c_str());

    // get napiEnv from aniEnv
    napi_env napiEnv = {};
    if (!arkts_napi_scope_open(aniEnv, &napiEnv)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "arkts_napi_scope_open failed");
        return nullptr;
    }

    // create normal FormExtensionContext
    auto contextObj = ContextTransfer::GetInstance().GetDynamicObject(contextType, napiEnv, formExtensionContext);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "create FormExtensionContext failed");
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    hybridgref ref = nullptr;
    bool success = hybridgref_create_from_napi(napiEnv, contextObj, &ref);
    if (!success) {
        TAG_LOGE(AAFwkTag::CONTEXT, "hybridgref_create_from_napi failed");
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    ani_object result = nullptr;
    success = hybridgref_get_esvalue(aniEnv, ref, &result);
    if (!success) {
        TAG_LOGE(AAFwkTag::CONTEXT, "hybridgref_get_esvalue failed");
        hybridgref_delete_from_napi(napiEnv, ref);
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    hybridgref_delete_from_napi(napiEnv, ref);

    if (!arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "arkts_napi_scope_close_n failed");
        return nullptr;
    }

    return result;
}

bool EtsFormExtensionContextModule::IsInstanceOf(ani_env *aniEnv, ani_object aniObj)
{
    ani_class cls {};
    ani_status status = ANI_ERROR;
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null aniEnv");
        return false;
    }
    if ((status = aniEnv->FindClass(ETS_FORM_EXTENSION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "FindClass status: %{public}d", status);
        return false;
    }
    ani_boolean isInstanceOf = false;
    if ((status = aniEnv->Object_InstanceOf(aniObj, cls, &isInstanceOf)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Object_InstanceOf status: %{public}d", status);
        return false;
    }
    return isInstanceOf;
}

void EtsFormExtensionContextModule::RegisterContextObjectCreator()
{
    ContextTransfer::GetInstance().RegisterStaticObjectCreator("FormExtensionContext",
        [](ani_env *aniEnv, std::shared_ptr<Context> context) -> ani_object {
            auto formExtensionContext = Context::ConvertTo<FormExtensionContext>(context);
            if (formExtensionContext == nullptr) {
                TAG_LOGE(AAFwkTag::CONTEXT, "invalid formExtensionContext");
                return nullptr;
            }
            auto newContext = CreateEtsFormExtensionContext(aniEnv, formExtensionContext);
            if (newContext == nullptr) {
                TAG_LOGE(AAFwkTag::CONTEXT, "create formExtensionContext failed");
                return nullptr;
            }
            return newContext;
    });

    ContextTransfer::GetInstance().RegisterDynamicObjectCreator("FormExtensionContext",
        [](napi_env napiEnv, std::shared_ptr<Context> context) -> napi_value {
            auto formExtensionContext = Context::ConvertTo<FormExtensionContext>(context);
            if (formExtensionContext == nullptr) {
                TAG_LOGE(AAFwkTag::CONTEXT, "invalid formExtensionContext");
                return nullptr;
            }

            auto object = EtsFormExtensionContextModule::GetOrCreateDynamicObject(napiEnv,
                formExtensionContext);
            if (object == nullptr) {
                TAG_LOGE(AAFwkTag::CONTEXT, "get or create object failed");
                return nullptr;
            }
            return object;
    });
}

napi_value EtsFormExtensionContextModule::AttachFormExtensionContext(napi_env env, void *value, void *hint)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "attach formExtensionContext");
    if (env == nullptr || value == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "invalid params");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<FormExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null ptr");
        return nullptr;
    }

    auto object = CreateJsFormExtensionContext(env, ptr);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null object");
        return nullptr;
    }
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(env, "application.FormExtensionContext", &object, 1);
    if (systemModule == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null systemModule");
        return nullptr;
    }
    auto contextObj = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::CONTEXT, "not napi object");
        return nullptr;
    }

    auto status = napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachFormExtensionContext, value, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "coerce formExtensionContext failed: %{public}d", status);
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<FormExtensionContext>(ptr);
    status = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::CONTEXT, "finalizer for weak_ptr formExtensionContext");
            delete static_cast<std::weak_ptr<FormExtensionContext> *>(data);
        },
        nullptr, nullptr);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::CONTEXT, "wrap formExtensionContext failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return contextObj;
}

void EtsFormExtensionContextModuleInit(ani_env *aniEnv)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "Init FormExtensionContext kit");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null ani env");
        return;
    }

    ani_class formExtensionContextCls = nullptr;
    auto status = aniEnv->FindClass(ETS_FORM_EXTENSION_CONTEXT_CLASS_NAME, &formExtensionContextCls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "FindClass FormExtensionContext failed status: %{public}d", status);
        return;
    }

    std::array nativeFuncs = {
        ani_native_function { "nativeTransferStatic", "C{std.interop.ESValue}C{std.core.String}:C{std.core.Object}",
            reinterpret_cast<void*>(EtsFormExtensionContextModule::NativeTransferStatic) },
        ani_native_function { "nativeTransferDynamic", "C{std.core.Object}:C{std.interop.ESValue}",
            reinterpret_cast<void*>(EtsFormExtensionContextModule::NativeTransferDynamic) },
    };
    status = aniEnv->Class_BindStaticNativeMethods(formExtensionContextCls, nativeFuncs.data(),
        nativeFuncs.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "Class_BindStaticNativeMethods failed status: %{public}d", status);
        return;
    }

    EtsFormExtensionContextModule::RegisterContextObjectCreator();
    TAG_LOGD(AAFwkTag::CONTEXT, "Init FormExtensionContext kit end");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::CONTEXT, "ANI_Constructor");
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::CONTEXT, "null vm");
        return ANI_ERROR;
    }

    ani_env *aniEnv = nullptr;
    ani_status status = vm->GetEnv(ANI_VERSION_1, &aniEnv);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::CONTEXT, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    EtsFormExtensionContextModuleInit(aniEnv);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::CONTEXT, "ANI_Constructor finish");
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS
