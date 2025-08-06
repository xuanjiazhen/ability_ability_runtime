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

#include "ets_context_module.h"

#include <cstdio>
#include <sys/syscall.h>
#include <unistd.h>

#ifndef LINUX_PREVIEW
#include "ani_base_context.h"
#include "ani_common_util.h"
#include "application_context_manager.h"
#include "context_transfer.h"
#include "ets_ability_stage_context.h"
#include "ets_application_context_utils.h"
#include "ets_context_utils.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "interop_js/arkts_esvalue.h"
#include "interop_js/arkts_interop_js_api.h"
#include "interop_js/hybridgref_ani.h"
#include "interop_js/hybridgref_napi.h"
#include "js_ability_stage_context.h"
#include "js_application_context_utils.h"
#include "js_context_utils.h"
#include "js_runtime_utils.h"
#include "native_engine/native_engine.h"
#endif // LINUX_PREVIEW

namespace OHOS {
namespace AbilityRuntime {
namespace {
#ifndef LINUX_PREVIEW
constexpr const char *ETS_CONTEXT_CLASS_NAME = "Lapplication/Context/Context;";

std::string GetClassNameByContextType(const std::string &contextType)
{
    std::string className;
    static const std::unordered_map<std::string, std::string> mapping = {
        {"Context", "Lapplication/Context/Context;"},
        {"ApplicationContext", "Lapplication/ApplicationContext/ApplicationContext;"},
        {"AbilityStageContext", "Lapplication/AbilityStageContext/AbilityStageContext;"},
        {"UIAbilityContext", "Lapplication/UIAbilityContext/UIAbilityContext;"}
    };
    auto it = mapping.find(contextType);
    if (it != mapping.end()) {
        className = it->second;
    }
    return className;
}
#endif // LINUX_PREVIEW
} // namespace

ani_object EtsContextModule::NativeTransferStatic(ani_env *aniEnv, ani_object self, ani_object input, ani_object type)
{
#ifndef LINUX_PREVIEW
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "transfer static Context");
    if (aniEnv == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "null aniEnv");
        return nullptr;
    }
    void *unwrapResult = nullptr;
    bool success = arkts_esvalue_unwrap(aniEnv, input, &unwrapResult);
    if (!success) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "failed to unwrap");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }
    if (unwrapResult == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "null unwrapResult");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto context = reinterpret_cast<std::weak_ptr<Context> *>(unwrapResult)->lock();
    if (context == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "null Context");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto &bindingObj = context->GetBindingObject();
    if (bindingObj == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "null bindingObj");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    auto staticContext = bindingObj->Get<ani_ref>();
    if (staticContext != nullptr) {
        TAG_LOGI(AAFwkTag::ETSRUNTIME, "there exist a staticContext");
        return reinterpret_cast<ani_object>(*staticContext);
    }

    auto contextObj = CreateStaticObject(aniEnv, type, context);
    if (contextObj == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "contextObj invalid");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    return contextObj;
#else
    //TAG_LOGE(AAFwkTag::ETSRUNTIME, "EtsContextModule::NativeTransferStatic not implemented");
    return nullptr;
#endif // LINUX_PREVIEW
}

ani_object EtsContextModule::CreateStaticObject(ani_env *aniEnv, ani_object type, std::shared_ptr<Context> context)
{
#ifndef LINUX_PREVIEW
    std::string contextType;
    if (!AppExecFwk::GetStdString(aniEnv, reinterpret_cast<ani_string>(type), contextType)) {
        //TAG_LOGE(AAFwkTag::JSNAPI, "GetStdString failed");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "contextType %{public}s", contextType.c_str());

    if (!ContextTransfer::GetInstance().IsStaticCreatorExist(contextType)) {
        std::string className = GetClassNameByContextType(contextType);
        if (!LoadTargetModule(aniEnv, className)) {
            return nullptr;
        }
    }

    auto contextObj = ContextTransfer::GetInstance().GetStaticObject(contextType, aniEnv, context);
    if (contextObj == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "contextObj invalid");
        return nullptr;
    }

    return contextObj;
#else
    //TAG_LOGE(AAFwkTag::ETSRUNTIME, "EtsContextModule::CreateStaticObject not implemented");
    return nullptr;
#endif // LINUX_PREVIEW
}

bool EtsContextModule::LoadTargetModule(ani_env *aniEnv, const std::string &className)
{
#ifndef LINUX_PREVIEW
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    ani_class contextCls = nullptr;
    auto status = aniEnv->FindClass(className.c_str(), &contextCls);
    if (status != ANI_OK) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "FindClass %{public}s failed status: %{public}d", className.c_str(), status);
        return false;
    }

    std::string gotContextType;
    if (!AppExecFwk::GetStaticFieldString(aniEnv, contextCls, "contextType", gotContextType)) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "get context type failed");
        return false;
    }

    TAG_LOGD(AAFwkTag::ETSRUNTIME, "gotContext %{public}s", gotContextType.c_str());
    return true;
#else
    //TAG_LOGE(AAFwkTag::ETSRUNTIME, "EtsContextModule::LoadTargetModule not implemented");
    return false;
#endif // LINUX_PREVIEW
}

std::unique_ptr<NativeReference> EtsContextModule::CreateNativeReference(napi_env napiEnv,
    std::shared_ptr<Context> context)
{
#ifndef LINUX_PREVIEW
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (napiEnv == nullptr || context == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "null param");
        return nullptr;
    }

    auto value = CreateJsBaseContext(napiEnv, context);
    auto systemModule = JsRuntime::LoadSystemModuleByEngine(napiEnv, "application.Context", &value, 1);
    if (systemModule == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "null system module");
        return nullptr;
    }

    napi_value object = systemModule->GetNapiValue();
    if (!CheckTypeForNapiValue(napiEnv, object, napi_object)) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "check type failed");
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<Context>(context);
    if (workContext == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "null workContext");
        return nullptr;
    }
    auto status = napi_coerce_to_native_binding_object(napiEnv, object, DetachCallbackFunc, AttachBaseContext,
        workContext, nullptr);
    if (status != napi_ok) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "coerce context failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    status = napi_wrap(napiEnv, object, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::ETSRUNTIME, "finalizer for weak_ptr context");
            delete static_cast<std::weak_ptr<Context> *>(data);
        }, nullptr, nullptr);
    if (status != napi_ok) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "wrap failed: %{public}d", status);
        delete workContext;
        return nullptr;
    }

    return systemModule;
#else
    //TAG_LOGE(AAFwkTag::ETSRUNTIME, "EtsContextModule::CreateNativeReference not implemented");
    return nullptr;
#endif // LINUX_PREVIEW
}

napi_value EtsContextModule::GetOrCreateDynamicObject(napi_env napiEnv, std::shared_ptr<Context> context)
{
#ifndef LINUX_PREVIEW
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (napiEnv == nullptr || context == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "null param");
        return nullptr;
    }

    // if sub-thread, create a new context and return
    if (getpid() != syscall(SYS_gettid)) {
        auto subThreadObj = static_cast<NativeReference *>(context->GetSubThreadObject(static_cast<void *>(napiEnv)));
        if (subThreadObj != nullptr) {
            return subThreadObj->Get();
        }
        auto subThreadRef = CreateNativeReference(napiEnv, context);
        if (subThreadRef == nullptr) {
            return nullptr;
        }
        auto newObject = subThreadRef->Get();
        context->BindSubThreadObject(static_cast<void *>(napiEnv), static_cast<void *>(subThreadRef.release()));
        return newObject;
    }

    // if main-thread, get bindingObj firstly
    auto &bindingObj = context->GetBindingObject();
    if (bindingObj == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "null bindingObj");
        return nullptr;
    }

    // if main-thread bindingObj exist, return it directly
    auto dynamicContext = bindingObj->Get<NativeReference>();
    if (dynamicContext != nullptr) {
        TAG_LOGI(AAFwkTag::UIABILITY, "there exist a dynamicContext");
        return dynamicContext->Get();
    }

    // if main-thread bindingObj didn't exist, create and bind
    std::unique_ptr<NativeReference> nativeRef = CreateNativeReference(napiEnv, context);
    if (nativeRef == nullptr) {
        return nullptr;
    }

    auto object = nativeRef->Get();
    context->Bind(nativeRef.release());
    return object;
#else
    //TAG_LOGE(AAFwkTag::ETSRUNTIME, "EtsContextModule::GetOrCreateDynamicObject not implemented");
    return nullptr;
#endif // LINUX_PREVIEW
}

ani_object EtsContextModule::NativeTransferDynamic(ani_env *aniEnv, ani_class aniCls, ani_object input)
{
#ifndef LINUX_PREVIEW
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "transfer dynamic Context");
    if (aniEnv == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "null aniEnv");
        return nullptr;
    }

    auto context = AbilityRuntime::GetStageModeContext(aniEnv, input);
    if (context == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "null context");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    ani_object object = CreateDynamicObject(aniEnv, aniCls, context);
    if (object == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "invalid object");
        EtsErrorUtil::ThrowEtsTransferClassError(aniEnv);
        return nullptr;
    }

    return object;
#else
    //TAG_LOGE(AAFwkTag::ETSRUNTIME, "EtsContextModule::NativeTransferDynamic not implemented");
    return nullptr;
#endif // LINUX_PREVIEW
}

ani_object EtsContextModule::CreateDynamicObject(ani_env *aniEnv, ani_class aniCls,
    std::shared_ptr<Context> contextPtr)
{
#ifndef LINUX_PREVIEW
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    std::string contextType;
    if (!AppExecFwk::GetStaticFieldString(aniEnv, aniCls, "contextType", contextType)) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "get context type failed");
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "contextType %{public}s", contextType.c_str());

    if (!ContextTransfer::GetInstance().IsDynamicCreatorExist(contextType)) {
        std::string className = GetClassNameByContextType(contextType);
        if (!LoadTargetModule(aniEnv, className)) {
            return nullptr;
        }
    }

    // get napiEnv from aniEnv
    napi_env napiEnv = {};
    if (!arkts_napi_scope_open(aniEnv, &napiEnv)) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "arkts_napi_scope_open failed");
        return nullptr;
    }

    // create normal context
    auto contextObj = ContextTransfer::GetInstance().GetDynamicObject(contextType, napiEnv, contextPtr);
    if (contextObj == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "create Context failed");
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    hybridgref ref = nullptr;
    bool success = hybridgref_create_from_napi(napiEnv, contextObj, &ref);
    if (!success) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "hybridgref_create_from_napi failed");
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    ani_object result = nullptr;
    success = hybridgref_get_esvalue(aniEnv, ref, &result);
    if (!success) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "hybridgref_get_esvalue failed");
        hybridgref_delete_from_napi(napiEnv, ref);
        arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr);
        return nullptr;
    }

    hybridgref_delete_from_napi(napiEnv, ref);

    if (!arkts_napi_scope_close_n(napiEnv, 0, nullptr, nullptr)) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "arkts_napi_scope_close_n failed");
        return nullptr;
    }

    return result;
#else
    //TAG_LOGE(AAFwkTag::ETSRUNTIME, "EtsContextModule::CreateDynamicObject not implemented");
    return nullptr;
#endif // LINUX_PREVIEW
}

void EtsContextModuleInit(ani_env *aniEnv)
{
#ifndef LINUX_PREVIEW
    TAG_LOGD(AAFwkTag::ETSRUNTIME, "Init Context kit");
    if (aniEnv == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "null ani env");
        return;
    }

    ani_class contextCls = nullptr;
    auto status = aniEnv->FindClass(ETS_CONTEXT_CLASS_NAME, &contextCls);
    if (status != ANI_OK) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "FindClass Context failed status: %{public}d", status);
        return;
    }

    std::array nativeFuncs = {
        ani_native_function { "nativeTransferStatic", "Lstd/interop/ESValue;Lstd/core/String;:Lstd/core/Object;",
            reinterpret_cast<void*>(EtsContextModule::NativeTransferStatic) },
        ani_native_function { "nativeTransferDynamic", "Lstd/core/Object;:Lstd/interop/ESValue;",
            reinterpret_cast<void*>(EtsContextModule::NativeTransferDynamic) },
    };
    status = aniEnv->Class_BindStaticNativeMethods(contextCls, nativeFuncs.data(), nativeFuncs.size());
    if (status != ANI_OK) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "Class_BindStaticNativeMethods failed status: %{public}d", status);
        return;
    }

    ContextTransfer::GetInstance().RegisterStaticObjectCreator("Context",
        [](ani_env *aniEnv, std::shared_ptr<Context> context) -> ani_object {
            ani_class cls {};
            ani_status status = ANI_ERROR;
            if ((status = aniEnv->FindClass("Lapplication/Context/Context;", &cls)) != ANI_OK) {
                //TAG_LOGE(AAFwkTag::ETSRUNTIME, "status: %{public}d", status);
                return nullptr;
            }
            return ContextUtil::CreateContextObject(aniEnv, cls, context);
    });

    ContextTransfer::GetInstance().RegisterDynamicObjectCreator("Context",
        [](napi_env napiEnv, std::shared_ptr<Context> context) -> napi_value {
            auto object = EtsContextModule::GetOrCreateDynamicObject(napiEnv, context);
            if (object == nullptr) {
            //TAG_LOGE(AAFwkTag::ETSRUNTIME, "get or create object failed");
                return nullptr;
            }
            return object;
    });

    TAG_LOGD(AAFwkTag::ETSRUNTIME, "Init Context kit end");
#else
    //TAG_LOGE(AAFwkTag::ETSRUNTIME, "EtsContextModuleInit not implemented");
#endif // LINUX_PREVIEW
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
//    TAG_LOGD(AAFwkTag::ETSRUNTIME, "ANI_Constructor");
    if (vm == nullptr) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "null vm");
        return ANI_ERROR;
    }

    ani_env *aniEnv = nullptr;
    ani_status status = vm->GetEnv(ANI_VERSION_1, &aniEnv);
    if (status != ANI_OK) {
        //TAG_LOGE(AAFwkTag::ETSRUNTIME, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    EtsContextModuleInit(aniEnv);
    *result = ANI_VERSION_1;
//    TAG_LOGD(AAFwkTag::ETSRUNTIME, "ANI_Constructor finish");
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS
