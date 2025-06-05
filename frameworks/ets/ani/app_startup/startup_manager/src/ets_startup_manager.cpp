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

#include "ets_startup_manager.h"
#include "ets_startup_config.h"
#include "ets_startup_task_result.h"
#include "ability_runtime_error_util.h"
#include "ani_common_util.h"
#include "ets_runtime_utils.h"
#include "hilog_tag_wrapper.h"
#include "sts_error_utils.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace AppExecFwk;

ani_object EtsStartupManager::OnRun(ani_env *env, ani_object startupTasks,
    ani_object config, ani_object call)
{
    TAG_LOGD(AAFwkTag::STARTUP, "OnRun called");
    if(env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return {};
    }

    std::shared_ptr<StartupTaskManager> startupTaskManager = nullptr;
    int32_t result = RunStartupTask(env, startupTasks, config, startupTaskManager);
    if (result != ERR_OK) {
        ThrowStsError(env, result, StartupUtils::GetErrorMessage(result));
        return CreateEtsUndefined(env);
    }

    ani_vm *vm = nullptr;
    if (env->GetVM(&vm) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetVM failed");
        ThrowStsError(env, ERR_STARTUP_INTERNAL_ERROR, StartupUtils::GetErrorMessage(ERR_STARTUP_INTERNAL_ERROR));
        return CreateEtsUndefined(env);
    }

    ani_ref callRef = nullptr;
    if (env->GlobalReference_Create(call, &callRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "GlobalReference_Create failed");
        ThrowStsError(env, ERR_STARTUP_INTERNAL_ERROR, StartupUtils::GetErrorMessage(ERR_STARTUP_INTERNAL_ERROR));
        return CreateEtsUndefined(env);
    }

    auto callback = std::make_shared<OnCompletedCallback>(
        [vm, callRef](const std::shared_ptr<StartupTaskResult> &result) {

            ani_env *env = nullptr;
            ani_status status = vm->GetEnv(ANI_VERSION_1, &env);
            if (status != ANI_OK) {
                TAG_LOGE(AAFwkTag::STARTUP, "status: %{public}d", status);
                return;
            }

            if (result == nullptr || result->GetResultCode() != ERR_OK) {
                ani_object error = EtsStartupConfig::BuildResult(env, result);
                AppExecFwk::AsyncCallback(env, static_cast<ani_object>(callRef), error, CreateEtsNull(env));
                env->GlobalReference_Delete(callRef);
                return;
            }
            ani_object resolution = EtsStartupConfig::BuildResult(env, result);
            AppExecFwk::AsyncCallback(env, static_cast<ani_object>(callRef), resolution, CreateEtsNull(env));
            env->GlobalReference_Delete(callRef);
            return;
        });
    result = startupTaskManager->Run(callback);
    if (result != ERR_OK) {
        if (!callback->IsCalled()) {
            env->GlobalReference_Delete(callRef);
            ThrowStsError(env, result, StartupUtils::GetErrorMessage(result));
            return CreateEtsUndefined(env);
        }
    }
    return CreateEtsNull(env);
}

void EtsStartupManager::OnRemoveAllResult(ani_env *env)
{
    TAG_LOGD(AAFwkTag::STARTUP, "OnRemoveAllResult called");
    if(env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return;
    }
    DelayedSingleton<StartupManager>::GetInstance()->RemoveAllResult();
    return;
}

ani_object EtsStartupManager::OnGetResult(ani_env *env, ani_string startupTask)
{
    TAG_LOGD(AAFwkTag::STARTUP, "OnGetResult called");
    if(env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return {};
    }

    std::string startupTaskStd;
    if (!AniStringToStdString(env, startupTask, startupTaskStd)) {
        TAG_LOGE(AAFwkTag::STARTUP, "convert failed");
        ThrowStsInvalidParamError(env, "Parameter error: Failed to convert startupTask, must be a string.");
        return CreateEtsUndefined(env);
    }

    std::shared_ptr<StartupTaskResult> result;
    int32_t res = DelayedSingleton<StartupManager>::GetInstance()->GetResult(startupTaskStd, result);
    if (res != ERR_OK || result == nullptr || result->GetResultCode() != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "get %{public}s result failed", startupTaskStd.c_str());
        ThrowStsInvalidParamError(env, "Parameter error: Failed to get result.");
        return CreateEtsUndefined(env);
    }
    if (result->GetResultType() != StartupTaskResult::ResultType::ETS) {
        return CreateEtsUndefined(env);
    }
    std::shared_ptr<EtsStartupTaskResult> etsResult = std::static_pointer_cast<EtsStartupTaskResult>(result);
    if (etsResult == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null etsResult: %{public}s", startupTaskStd.c_str());
        ThrowStsInvalidParamError(env, "Parameter error: Failed to convert to js result.");
        return CreateEtsUndefined(env);
    }
    std::shared_ptr<STSNativeReference> etsResultRef = etsResult->GetEtsStartupResultRef();
    if (etsResultRef == nullptr) {
        return CreateEtsUndefined(env);
    }
    return static_cast<ani_object>(etsResultRef->aniRef);
}

ani_object EtsStartupManager::OnIsInitialized(ani_env *env, ani_string startupTask)
{
    TAG_LOGD(AAFwkTag::STARTUP, "OnIsInitialized called");
    if(env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return {};
    }

    std::string startupTaskStd;
    if (!AniStringToStdString(env, startupTask, startupTaskStd)) {
        TAG_LOGE(AAFwkTag::STARTUP, "convert failed");
        ThrowStsInvalidParamError(env, "Parameter error: Failed to convert startupTask,"
            "must be a string or startupTask name is not matched.");
        return CreateEtsUndefined(env);
    }

    bool isInitialized = false;
    int32_t res = DelayedSingleton<StartupManager>::GetInstance()->IsInitialized(startupTaskStd, isInitialized);
    if (res != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "get %{public}s if initialized failed:%{public}d", startupTaskStd.c_str(), res);
        ThrowStsInvalidParamError(env, "Parameter error: Startup manager not initialized.");
        return CreateEtsUndefined(env);
    }
    return createBoolean(env, isInitialized);
}

void EtsStartupManager::OnRemoveResult(ani_env *env, ani_string startupTask)
{
    TAG_LOGD(AAFwkTag::STARTUP, "OnRemoveResult called");
    if(env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return;
    }

    std::string startupTaskStd;
    if (!AniStringToStdString(env, startupTask, startupTaskStd)) {
        TAG_LOGE(AAFwkTag::STARTUP, "convert failed");
        ThrowStsInvalidParamError(env, "Parameter error: Failed to convert startupTask,"
            "must be a string or startupTask name is not matched.");
        return;
    }

    int32_t res = DelayedSingleton<StartupManager>::GetInstance()->RemoveResult(startupTaskStd);
    if (res != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "remove %{public}s result failed:%{public}d", startupTaskStd.c_str(), res);
        ThrowStsInvalidParamError(env, "Parameter error: Failed to remove result.");
        return;
    }
    return;
}

int32_t EtsStartupManager::GetConfig(ani_env *env, ani_object value, std::shared_ptr<StartupConfig> &config)
{
    std::shared_ptr<EtsStartupConfig> startupConfig = std::make_shared<EtsStartupConfig>(env);
    if (startupConfig == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null startupConfig");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    if (startupConfig->Init(value) != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "init config failed");
        ThrowStsInvalidParamError(env, "Parameter error: Failed to init config, must be a StartupConfig.");
        return ERR_STARTUP_INVALID_VALUE;
    }
    config = startupConfig;
    return ERR_OK;
}

int32_t EtsStartupManager::RunStartupTask(ani_env *env, ani_object startupTasks, ani_object config,
    std::shared_ptr<StartupTaskManager> &startupTaskManager)
{
    std::vector<std::string> dependencies;
    if (!UnWrapArrayString(env, startupTasks, dependencies)) {
        TAG_LOGE(AAFwkTag::STARTUP, "get dependencies failed");
        return ERR_STARTUP_INVALID_VALUE;
    }
    std::shared_ptr<StartupConfig> configCpp;
    ani_boolean isUndefined = true;
    ani_status status = env->Reference_IsUndefined(config, &isUndefined);
    if (status == ANI_OK) {
        if (!isUndefined) {
            int32_t result = GetConfig(env, config, configCpp);
            if (result != ANI_OK) {
                TAG_LOGE(AAFwkTag::STARTUP, "get config failed");
                return ERR_STARTUP_INVALID_VALUE;
            }
        }
    } else {
        TAG_LOGE(AAFwkTag::STARTUP, "Reference_IsUndefined failed status: %{public}d", status);
        return ERR_STARTUP_INTERNAL_ERROR;
    }

    int32_t result = DelayedSingleton<StartupManager>::GetInstance()->BuildAppStartupTaskManager(dependencies,
        startupTaskManager);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "build manager failed: %{public}d", result);
        return result;
    }
    if (startupTaskManager == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null startupTaskMgr");
        return ERR_STARTUP_INTERNAL_ERROR;
    }
    result = startupTaskManager->Prepare();
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "prepare startup failed: %{public}d", result);
        return result;
    }
    if (configCpp != nullptr) {
        startupTaskManager->SetConfig(configCpp);
    }
    return ERR_OK;
}
void EtsStartupManagerInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::STARTUP, "EtsStartupManagerInit called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "null env");
        return;
    }

    ani_namespace ns;
    ani_status status = env->FindNamespace("L@ohos/app/ability/startupManager/startupManager;", &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "FindNamespace startupManager failed status: %{public}d", status);
        return;
    }

    std::array kitFunctions = {
        ani_native_function {"nativeRunSync", nullptr,
            reinterpret_cast<void *>(EtsStartupManager::OnRun)},
        ani_native_function {"nativeRemoveAllStartupTaskResults", nullptr,
            reinterpret_cast<void *>(EtsStartupManager::OnRemoveAllResult)},
        ani_native_function {"nativeGetStartupTaskResult", nullptr,
            reinterpret_cast<void *>(EtsStartupManager::OnGetResult)},
        ani_native_function {"nativeIsStartupTaskInitialized", nullptr,
            reinterpret_cast<void *>(EtsStartupManager::OnIsInitialized)},
        ani_native_function {"nativeRemoveStartupTaskResult", nullptr,
            reinterpret_cast<void *>(EtsStartupManager::OnRemoveResult)},
    };

    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK &&  status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::STARTUP, "bind native functions failed status: %{public}d", status);
    }
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::STARTUP, "ANI_Constructor");
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGD(AAFwkTag::STARTUP, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    EtsStartupManagerInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::STARTUP, "ANI_Constructor finish");
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS
