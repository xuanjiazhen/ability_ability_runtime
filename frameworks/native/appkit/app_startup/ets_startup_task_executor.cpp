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

// ets_startup_task_executor.cpp

#include "ets_startup_task_executor.h"
#include "hilog_tag_wrapper.h"
#include "ets_startup_task_result.h"

namespace OHOS {
namespace AbilityRuntime {

int32_t ETSStartupTaskExecutor::RunOnTaskPool(ani_env *env, ani_ref startup, 
    ani_ref context, std::unique_ptr<StartupTaskResultCallback> callback)
{
    TAG_LOGD(AAFwkTag::STARTUP, "RunOnTaskPool called for task: %s", startupName.c_str());
    
    if (env == nullptr || startup == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null args in RunOnTaskPool");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    ani_status status = ANI_ERROR;
    ani_class cls = nullptr;
    ani_method ctor = nullptr;
    ani_object object = nullptr;
    
    if ((status = env->FindClass("Lappstartup/StartupTaskExecutor/StartupTaskExecutor", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "FindClass failed, status: %{public}d", status);
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null cls");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "findMethod failed, status: %{public}d", status);
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    if ((status = env->Object_New(cls, ctor, &object)) != ANI_OK || object == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "Object_New failed, status: %{public}d", status);
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    ani_object startupObj = reinterpret_cast<ani_object>(startup);
    ani_object contextObj = reinterpret_cast<ani_object>(context);

    auto sharedCallback = std::shared_ptr<StartupTaskResultCallback>(callback.release());
    auto workCallback = new (std::nothrow) std::weak_ptr<StartupTaskResultCallback>(sharedCallback);
    if (workCallback == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null workCallback");
        return nullptr;
    }
    ani_long nativeCallbackLong = reinterpret_cast<ani_long>(workCallback);
    if ((status = env->Object_CallMethodByName_Void(object, "asyncPushTask", 
        "L@ohos/app/appstartup/StartupTask/StartupTask;Lapplication/AbilityStageContext/AbilityStageContext;"
        "Lstd/core/Long;:V", startupTask, context, nativeCallbackLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to call asyncPushTask, status: %{public}d", status);
        delete weakContext;
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }

    TAG_LOGD(AAFwkTag::STARTUP, "RunOnTaskPool completed successfully for task: %s", startupName.c_str());
    return ERR_OK;
}

int32_t ETSStartupTaskExecutor::RunOnMainThread(ani_env *env, ani_ref startupTask, 
    ani_ref context, std::unique_ptr<StartupTaskResultCallback> callback)
{
    TAG_LOGD(AAFwkTag::STARTUP, "RunOnMainThread called");
    
    if (env == nullptr || startupTask == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null args in RunOnMainThread");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    ani_status status = ANI_ERROR;
    ani_class cls = nullptr;
    ani_method ctor = nullptr;
    ani_object object = nullptr;
    if ((status = env->FindClass("Lappstartup/StartupTaskExecutor/StartupTaskExecutor", &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "status : %{public}d", status);
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null cls");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &ctor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "findMethod failed, status : %{public}d", status);
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    if ((status = env->Object_New(cls, ctor, &object)) != ANI_OK || object == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Object_New status : %{public}d or null object", status);
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    auto sharedCallback = std::shared_ptr<StartupTaskResultCallback>(callback.release());

    auto workCallback = new (std::nothrow) std::weak_ptr<StartupTaskResultCallback>(sharedCallback);
    if (workCallback == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "null workCallback");
        return nullptr;
    }
    ani_long nativeCallbackLong = reinterpret_cast<ani_long>(workCallback);
    if ((status = env->Object_CallMethodByName_Void(object, "executeStartupTaskOnMainThread", 
        "L@ohos/app/appstartup/StartupTask/StartupTask;Lapplication/AbilityStageContext/AbilityStageContext;"
        "Lstd/core/Long;:V", startupTask, context, nativeCallbackLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to call executeStartupTaskOnMainThread");
        delete weakContext;
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }

    return ani_status;
}

void ETSStartupTaskExecutor::NativeOnTaskSuccess(ani_env *env, ani_object obj, 
    ani_long callbackLong, ani_object result)
{
    TAG_LOGD(AAFwkTag::STARTUP, "NativeOnTaskSuccess called");
    
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "invalid arguments in NativeOnTaskSuccess");
        return;
    }

    auto *callback = reinterpret_cast<StartupTaskResultCallback *>(callbackLong);
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null callback pointer");
        return;
    }

    ani_ref resultRef = nullptr;
    if (result != nullptr) {
        env->CreateReference(result, 1, &resultRef);
    }

    std::shared_ptr<StartupTaskResult> taskResult = std::make_shared<ETSStartupTaskResult>(resultRef);
    callback->Call(taskResult);
    
    delete callback;
}

void ETSStartupTaskExecutor::NativeOnTaskFailure(ani_env *env, ani_object thisObj, 
    ani_long callbackLong, ani_string errorMessage)
{
    TAG_LOGD(AAFwkTag::STARTUP, "NativeOnTaskFailure called");
    
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "invalid arguments in NativeOnTaskFailure");
        return;
    }

    auto *callback = reinterpret_cast<StartupTaskResultCallback *>(callbackLong);
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null callback pointer");
        return;
    }

    std::string strErrorMessage;
    if (!AppExecFwk::GetStdString(env, errorMessage, strErrorMessage)) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetStdString failed.");
        // 即使获取错误信息失败，我们仍然需要回调并传递一个错误
        strErrorMessage = "Unknown error";
    }

    std::shared_ptr<StartupTaskResult> taskResult = std::make_shared<ETSStartupTaskResult>(
        ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP, strErrorMessage);
    callback->Call(taskResult);
    
    delete callback;
}

void ETSStartupTaskExecutorInit(ani_env *env)
{
    TAG_LOGI(AAFwkTag::STARTUP, "Init startup task executor");

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "env is null");
        return;
    }
    ani_class startupTaskExecutorCls = nullptr;
    auto status = env->FindClass("Lappstartup/StartupTaskExecutor/StartupTaskExecutor", &startupTaskExecutorCls);
    if (status != ANI_OK || startupTaskExecutorCls == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "find StartupTaskExecutor class failed");
        return;
    }

    std::array startupTaskExecutorNativeFuncs = {
        ani_native_function { "nativeOnTaskSuccess", "Lstd/core/Long;Lstd/core/Object;:V",
            reinterpret_cast<void*>(ETSStartupTaskExecutor::NativeOnTaskSuccess) },
        ani_native_function { "nativeOnTaskFailure", "Lstd/core/Long;Lstd/core/String;:V",
            reinterpret_cast<void*>(ETSStartupTaskExecutor::NativeOnTaskFailure) },
    };

    status = env->Class_BindNativeMethods(startupTaskExecutorCls, 
        startupTaskExecutorNativeFuncs.data(), 
        static_cast<int32_t>(startupTaskExecutorNativeFuncs.size()));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "bind StartupTaskExecutor native methods failed");
        return;
    }

    TAG_LOGI(AAFwkTag::STARTUP, "StartupTaskExecutor native methods bound successfully");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::STARTUP, "ANI_Constructor");
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null vm");
        return ANI_ERROR;
    }
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    ETSStartupTaskExecutorInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::STARTUP, "ANI_Constructor finish");
    return ANI_OK;
}
} // namespace AbilityRuntime
} // namespace OHOS