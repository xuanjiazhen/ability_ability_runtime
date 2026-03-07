/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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
 
#include "ani.h"
 
#include <cstdint>
#include <unistd.h>
#include <memory>
#include "ability_business_error.h"
#include "app_recovery.h"
#include "application_data_manager.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "ani_common_want.h"
#include "want.h"
#include "error_manager_ani_util.h"
#include "ierror_observer.h"
#include "ets_error_utils.h"
 
namespace OHOS {
namespace AbilityRuntime {
namespace {

static GlobalObserverItem defaultLeakObserver;
static std::mutex defaultLeakMtx;
static std::mutex defaultHandlerMtx;
static GlobalObserverItem defaultHandler;
}
 
class ErrorManagerAni final {
public:
    ErrorManagerAni() {}
    ~ErrorManagerAni() = default;
 
    static void DoErrorCallback(const std::string &summary, const AppExecFwk::ErrorObject &errorObj)
    {
        TAG_LOGW(AAFwkTag::APPKIT, "yhl -- DoErrorCallback Start");
        if (defaultHandler.vm == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "yhl -- DoErrorCallback defaultHandler.vm is nullptr");
            return;
        }
        ani_env* env = GetAniEnv(defaultHandler.vm);
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "yhl -- DoErrorCallback env is nullptr");
            return;
        }
        ani_object errorParam = nullptr;
        ani_class cls {};
        if (ANI_OK != env->FindClass(CLASS_NAME_EVENT_PACKAGE_HOLDER, &cls)) {
            TAG_LOGW(AAFwkTag::APPKIT, "yhl -- DoErrorCallback failed to find Class %{public}s", CLASS_NAME_EVENT_PACKAGE_HOLDER);
            return;
        }
        ani_method method {};
        ani_status status = env->Class_FindMethod(cls, "<ctor>", "iC{escompat.Error}:", &method);
        if (status != ANI_OK) {
            TAG_LOGW(AAFwkTag::APPKIT, "yhl -- DoErrorCallback failed to get method %{public}s constructor",
                CLASS_NAME_EVENT_PACKAGE_HOLDER);
            return;
        }
        if (ANI_OK != env->Object_New(cls, "<ctor>", &errorParam)) {
            TAG_LOGW(AAFwkTag::APPKIT, "yhl -- DoErrorCallback failed to create object %{public}s", CLASS_NAME_EVENT_PACKAGE_HOLDER);
            return;
        }
        ani_ref argv[INITITAL_REFCOUNT_ONE] = {errorParam};
        ani_ref result{};
        if (env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(defaultHandler.ref), INITITAL_REFCOUNT_ONE, argv,
            &result) != ANI_OK) {
            TAG_LOGW(AAFwkTag::APPKIT, "yhl -- DoErrorCallback failed to call SetDefaultErrorHandler function");
        }
        TAG_LOGW(AAFwkTag::APPKIT, "yhl -- DoErrorCallback end");
    }

    static ani_object SetDefaultErrorHandler(ani_env *env, ani_object function)
    {
        TAG_LOGW(AAFwkTag::APPKIT, "yhl -- SetDefaultErrorHandler begin");
        if (IsRefUndefined(env, function)) {
            TAG_LOGW(AAFwkTag::APPKIT, "yhl -- function is undefined");
            EtsErrorUtil::ThrowInvalidNumParametersError(env);
            return nullptr;
        }
        if (IsNull(env, function)) {
            function = nullptr;
            TAG_LOGW(AAFwkTag::APPKIT, "yhl -- function is null");
        }
        std::lock_guard<std::mutex> lock(defaultHandlerMtx);
        ani_object result{};
        if (!defaultHandler.ref) {
            TAG_LOGW(AAFwkTag::APPKIT, "yhl -- Delete defaultHandler ref");
            result = nullptr;
        } else {
            result = static_cast<ani_object>(defaultHandler.ref);
            env->GlobalReference_Delete(defaultHandler.ref);
            defaultHandler.ref = nullptr;
            TAG_LOGW(AAFwkTag::APPKIT, "yhl -- Delete defaultHandler ref");
        }
        if (function) {
            auto status = env->GlobalReference_Create(function, &defaultHandler.ref);
            if (status != ANI_OK) {
                TAG_LOGW(AAFwkTag::APPKIT, "yhl -- Get new defaultHandler function Failed");
                return result;
            }
        }
        TAG_LOGE(AAFwkTag::APPKIT, "yhl -- SetDefaultErrorHandler ref:%{public}p", defaultHandler.ref);
        defaultHandler.vm = GetAniVm(env);
        if (defaultHandler.vm == nullptr) {
            TAG_LOGW(AAFwkTag::APPKIT, "yhl -- SetDefaultErrorHandler !defaultHandler.vm");
        }
 
        TAG_LOGW(AAFwkTag::APPKIT, "yhl -- SetDefaultErrorHandler end");
        return result;
    }

    static ani_object CreateDetailInfoObject(AppExecFwk::LeakType leakType, const AppExecFwk::LeakDetailInfo &detailInfo)
    {
        ani_env* env = GetAniEnv(defaultHandler.vm);
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::ANI, "yhl -- env is nullptr");
            return nullptr;
        }
        ani_status status = ANI_ERROR;
        ani_class recordCls = nullptr;
        status = env->FindClass(RECORD_CLASS_NAME, &recordCls);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "FindClass failed status: %{public}d", status);
            return nullptr;
        }
        ani_method objectMethod = nullptr;
        status = env->Class_FindMethod(recordCls, "<ctor>", ":", &objectMethod);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "Class_FindMethod constructor failed: %{public}d", status);
            return nullptr;
        }
        ani_object recordObject = nullptr;
        status = env->Object_New(recordCls, objectMethod, &recordObject);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "Object_New failed: %{public}d", status);
            return nullptr;
        }
        ani_method recordSetMethod {};;
        status = env->Class_FindMethod(recordCls, "$_set",
            "X{C{std.core.Numeric}C{std.core.String}C{std.core.BaseEnum}}Y:", &recordSetMethod);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ANI, "Class_FindMethod failed status: %{public}d", status);
            return nullptr;
        }
        std::map<std::string, unsigned long> keyValueMap = {
            {"arkts", detailInfo.arktsSize},
            {"native", detailInfo.nativeSize},
            {"ion", detailInfo.ionSize},
            {"gpu", detailInfo.gpuSize},
            {"ashmem", detailInfo.ashmemSize},
            {"other", detailInfo.otherSize}
        };
        for (auto iter = keyValueMap.begin(); iter != keyValueMap.end(); ++iter) {
            std::string key = iter->first;
            ani_string aniKey = AppExecFwk::GetAniString(env, key);
            long value = iter->second;
            ani_object aniValueObj = AppExecFwk::CreateLong(env, value);
            status = env->Object_CallMethod_Void(recordObject, recordSetMethod, aniKey, aniValueObj);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::PROCESSMGR, "Object_CallMethod_Void failed status: %{public}d", status);
                return nullptr;
            }
        }    

        return recordObject;
    }

    static bool LeakObserverFunc(const AppExecFwk::LeakObject &obj)
    {
        TAG_LOGI(AAFwkTag::APPKIT, "xyyy ObserverFunc start");
        std::lock_guard<std::mutex> lock(defaultLeakMtx);
        ani_env* env = GetAniEnv(defaultLeakObserver.vm);
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "yhl -- LeakObserverFunc env is nullptr");
            return true;
        }
        if (defaultLeakObserver.ref == nullptr) {
            TAG_LOGW(AAFwkTag::APPKIT, "yhl -- LeakObserverFunc !defaultLeakObserver.ref");
            return true;
        }
        ani_ref argv[] = {
            AppExecFwk::CreateInt(env, static_cast<int>(obj.leakType)),
            AppExecFwk::CreateLong(env, obj.leakSize),
            CreateDetailInfoObject(obj.leakType, obj.detailInfo)
        };
        ani_ref result{};
        if (env->FunctionalObject_Call(reinterpret_cast<ani_fn_object>(defaultLeakObserver.ref), INITITAL_REFCOUNT_THREE,
            argv, &result) != ANI_OK) {
            TAG_LOGI(AAFwkTag::APPKIT, "yhl -- LeakObserverFunc failed to call function");
        }
        TAG_LOGW(AAFwkTag::APPKIT, "yhl -- LeakObserverFunc End");
        return true;
    }

    static ani_object SetDefaultLeakObserver(ani_env *env, ani_object function)
    {
       TAG_LOGW(AAFwkTag::APPKIT, "yhl -- SetDefaultErrorHandler end");
       if (!AppExecFwk::EventRunner::IsAppMainThread()) {
            TAG_LOGE(AAFwkTag::APPKIT, "yhl -- SetDefaultLeakObserver not mainThread");
            EtsErrorUtil::ThrowInvalidCallerError(env);
            return nullptr;
        }
        TAG_LOGW(AAFwkTag::APPKIT, "yhl -- SetDefaultErrorHandler begin");
        // 步骤1：参数校验 - 检查传入的 function 是否为 undefined
        if (IsRefUndefined(env, function) || IsNull(env, function)) {
            TAG_LOGE(AAFwkTag::APPKIT, "yhl -- function is undefined");
            EtsErrorUtil::ThrowInvalidNumParametersError(env);
            return nullptr;
        }
        std::lock_guard<std::mutex> lock(defaultLeakMtx);
        ani_object oldObserverFunc{};
        if (defaultLeakObserver.ref == nullptr) {
            // 旧回调不存在，返回 null 两个作用，注册新函数，返回旧的函数。
            oldObserverFunc = nullptr;
            TAG_LOGW(AAFwkTag::APPKIT, "yhl -- defaultLeakObserver.ref == nullptr)");
        } else {
            // 旧回调存在：1. 保存旧引用用于返回 2. 释放旧的全局引用 3. 清空旧引用
            oldObserverFunc = static_cast<ani_object>(defaultLeakObserver.ref);
            env->GlobalReference_Delete(defaultLeakObserver.ref);
            defaultLeakObserver.ref = nullptr;
            TAG_LOGW(AAFwkTag::APPKIT, "yhl -- defaultLeakObserver.ref = nullptr");
        }
        // 步骤5：处理新的观察者回调函数（创建全局引用）
        if (function) {
            auto status = env->GlobalReference_Create(function, &defaultLeakObserver.ref);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::APPKIT, "yhl -- Get new defaultLeakObserver function Failed");
                return oldObserverFunc;
            }
        }
        TAG_LOGE(AAFwkTag::APPKIT, "yhl -- SetDefaultErrorHandler ref:%{public}p", defaultLeakObserver.ref);
       // 步骤7：保存 ETS/JS 虚拟机实例（保证后续调用回调时能获取执行环境）
        defaultLeakObserver.vm = GetAniVm(env);
        if (defaultLeakObserver.vm == nullptr) {
            env->GlobalReference_Delete(defaultHandler.ref);
            defaultHandler.ref = nullptr;
            TAG_LOGW(AAFwkTag::APPKIT, "yhl -- ani_env::GetVM failed");
        }

        TAG_LOGW(AAFwkTag::APPKIT, "yhl -- SetDefaultErrorHandler end");
        return oldObserverFunc;
    }
};
 
static void EtsErrorManagerInit(ani_env *env)
{
    TAG_LOGI(AAFwkTag::RECOVERY, "yhl -- EtsErrorManagerInit Called.");
    if (env == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "yhl -- null env");
        return;
    }
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGW(AAFwkTag::APPKIT, "yhl -- ResetError failed");
    }

    ani_namespace ns;
    status = env->FindNamespace("@ohos.app.ability.errorManager.errorManager", &ns);
    if (status != ANI_OK) {
        TAG_LOGW(AAFwkTag::APPKIT, "yhl -- FindNamespace errorManager failed status: %{public}d", status);
        return;
    }
 
    std::array kitFunctions = {
        ani_native_function {"setDefaultErrorHandler", nullptr, reinterpret_cast<void *>(ErrorManagerAni::SetDefaultErrorHandler)},
        ani_native_function {"setDefaultLeakObserver", nullptr, reinterpret_cast<void *>(ErrorManagerAni::SetDefaultLeakObserver)},    
    };
 
    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGW(AAFwkTag::APPKIT, "yhl -- Namespace_BindNativeFunctions failed status: %{public}d", status);
    }
 
    if (env->ResetError() != ANI_OK) {
        TAG_LOGW(AAFwkTag::APPKIT, "yhl -- ResetError failed");
    }
    TAG_LOGE(AAFwkTag::RECOVERY, "yhl -- EtsErrorManagerInit end");
}
}  // namespace AbilityRuntime
}  // namespace OHOS
 
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGI(AAFwkTag::RECOVERY, "yhl -- ANI_Constructor start.");
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if (vm == nullptr) {
        TAG_LOGW(AAFwkTag::APPKIT, "yhl -- null vm");
        return ANI_ERROR;
    }
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGW(AAFwkTag::APPKIT, "yhl -- GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }
 
    OHOS::AbilityRuntime::EtsErrorManagerInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGW(AAFwkTag::APPKIT, "yhl -- SetNotifyETSExceptionFunc Start");
    OHOS::AppExecFwk::ApplicationDataManager::GetInstance().SetLeakObserver(
    OHOS::AbilityRuntime::ErrorManagerAni::LeakObserverFunc);
    // OHOS::AppExecFwk::ApplicationDataManager::GetInstance().SetNotifyETSExceptionFunc(OHOS::AbilityRuntime::ErrorManagerAni::DoErrorCallback);
    TAG_LOGW(AAFwkTag::APPKIT, "yhl -- NotifyETSExceptionFunc DoErrorCallback: %{public}p.", OHOS::AbilityRuntime::ErrorManagerAni::DoErrorCallback);
    TAG_LOGI(AAFwkTag::RECOVERY, "yhl -- ANI_Constructor finish");
    return ANI_OK;
}