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

int32_t ETSStartupTaskExecutor::RunOnTaskPool(ani_env *env, ani_object startup, 
    ani_object context, ani_object asyncTaskExecutor, ani_object asyncTaskCallback, 
    const std::string &startupName)
{
    TAG_LOGD(AAFwkTag::STARTUP, "RunOnTaskPool called for task: %s", startupName.c_str());
    
    if (env == nullptr || startup == nullptr || context == nullptr || 
        asyncTaskExecutor == nullptr || asyncTaskCallback == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null args in RunOnTaskPool");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    // 创建startupName的ANI字符串
    ani_object startupNameObj = nullptr;
    ani_status status = env->CreateString(startupName.c_str(), &startupNameObj);
    if (status != ANI_OK || startupNameObj == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to create startup name string");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    // 调用AsyncTaskExecutor的asyncPushTask方法
    // 参数顺序: startup, asyncCallback, context, startupName
    ani_object args[4] = { startup, asyncTaskCallback, context, startupNameObj };
    
    status = env->CallObjectMethodByName_Void(asyncTaskExecutor, "asyncPushTask", args, 4);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "failed to call asyncPushTask, status: %d", status);
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }

    TAG_LOGD(AAFwkTag::STARTUP, "RunOnTaskPool completed successfully for task: %s", startupName.c_str());
    return ERR_OK;
}

int32_t ETSStartupTaskExecutor::RunOnMainThread(ani_env *env, ani_object startup, 
    ani_object context, std::unique_ptr<StartupTaskResultCallback> callback)
{
    TAG_LOGD(AAFwkTag::STARTUP, "RunOnMainThread called");
    
    if (env == nullptr || startup == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null args in RunOnMainThread");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    ani_object returnVal = nullptr;
    int32_t resultCode = CallStartupInit(env, startup, context, callback, returnVal);
    if (resultCode != ERR_OK) {
        return resultCode;
    }
    return HandleReturnVal(env, returnVal, callback);
}

int32_t ETSStartupTaskExecutor::CallStartupInit(ani_env *env, ani_object startup, 
    ani_object context, std::unique_ptr<StartupTaskResultCallback> &callback, ani_object &returnVal)
{
    if (startup == nullptr || context == nullptr) {
        ReplyFailed(std::move(callback), ERR_STARTUP_INTERNAL_ERROR,
            "startup task is null or context is null.");
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    // 调用ETS StartupTask的init方法
    ani_object args[1] = { context };
    ani_status status = env->CallObjectMethodByName_Ref(startup, "init", &returnVal, args, 1);
    if (status != ANI_OK) {
        ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
            "failed to call init method.");
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }

    return ERR_OK;
}

int32_t ETSStartupTaskExecutor::HandleReturnVal(ani_env *env, ani_object returnVal,
    std::unique_ptr<StartupTaskResultCallback> &callback)
{
    if (returnVal == nullptr) {
        ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
            "the return value of startup task init is null.");
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }

    // 检查是否为Promise
    bool isPromise = false;
    ani_status status = env->IsPromise(returnVal, &isPromise);
    if (status != ANI_OK || !isPromise) {
        ReplyFailed(std::move(callback), ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
            "the return value of startup task init is not promise.");
        return ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP;
    }

    auto *callbackPtr = callback.release();
    
    // 设置Promise的resolve回调
    ani_object thenFunc = nullptr;
    status = env->GetPropertyByName(returnVal, "then", &thenFunc);
    if (status == ANI_OK && thenFunc != nullptr) {
        ani_object resolveCallback = nullptr;
        status = env->CreateFunction(ResolveResultCallback, callbackPtr, &resolveCallback);
        if (status == ANI_OK && resolveCallback != nullptr) {
            ani_object argvThen[1] = { resolveCallback };
            env->CallFunction(returnVal, thenFunc, 1, argvThen, nullptr);
        }
    }

    // 设置Promise的reject回调  
    ani_object catchFunc = nullptr;
    status = env->GetPropertyByName(returnVal, "catch", &catchFunc);
    if (status == ANI_OK && catchFunc != nullptr) {
        ani_object rejectCallback = nullptr;
        status = env->CreateFunction(RejectResultCallback, callbackPtr, &rejectCallback);
        if (status == ANI_OK && rejectCallback != nullptr) {
            ani_object argvCatch[1] = { rejectCallback };
            env->CallFunction(returnVal, catchFunc, 1, argvCatch, nullptr);
        }
    }

    return ERR_OK;
}

ani_object ETSStartupTaskExecutor::ResolveResultCallback(ani_env *env, ani_object thisObj, 
    ani_object *args, int32_t argc, void *data)
{
    TAG_LOGD(AAFwkTag::STARTUP, "ResolveResultCallback called");
    
    if (argc >= 1 && data != nullptr) {
        auto *callback = static_cast<StartupTaskResultCallback *>(data);
        ani_object resultObj = args[0];
        
        if (resultObj != nullptr) {
            ani_ref resultRef = nullptr;
            env->CreateReference(resultObj, 1, &resultRef);
            
            if (resultRef != nullptr) {
                std::shared_ptr<StartupTaskResult> result = std::make_shared<ETSStartupTaskResult>(resultRef);
                callback->Call(result);
            }
        }
        
        delete callback;
    }
    
    return nullptr;
}

ani_object ETSStartupTaskExecutor::RejectResultCallback(ani_env *env, ani_object thisObj, 
    ani_object *args, int32_t argc, void *data)
{
    TAG_LOGD(AAFwkTag::STARTUP, "RejectResultCallback called");
    
    if (data != nullptr) {
        auto *callback = static_cast<StartupTaskResultCallback *>(data);
        ReplyFailed(callback, ERR_STARTUP_FAILED_TO_EXECUTE_STARTUP,
            "the promise of startup task init is reject.");
    }
    
    return nullptr;
}

void ETSStartupTaskExecutor::ReplyFailed(StartupTaskResultCallback *callback,
    int32_t resultCode, const std::string &resultMessage)
{
    if (callback == nullptr) {
        return;
    }
    std::shared_ptr<StartupTaskResult> result = std::make_shared<ETSStartupTaskResult>(resultCode, resultMessage);
    callback->Call(result);
    delete callback;
}

void ETSStartupTaskExecutor::ReplyFailed(std::unique_ptr<StartupTaskResultCallback> callback,
    int32_t resultCode, const std::string &resultMessage)
{
    if (callback == nullptr) {
        return;
    }
    std::shared_ptr<StartupTaskResult> result = std::make_shared<ETSStartupTaskResult>(resultCode, resultMessage);
    callback->Call(result);
}

} // namespace AbilityRuntime
} // namespace OHOS