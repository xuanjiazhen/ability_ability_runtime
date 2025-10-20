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

#include "startup_manager.h"
#include "ability_stage_context.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr const char *ETS_STARTUP_MANAGER_CLASS_NAME = "L@ohos/app/appstartup/startupManager/startupManager;";
constexpr const char *SIGNATURE_STARTUP_MANAGER_RUN_ASYNCCALLBACK = 
    "Lescompat/Array;Lapplication/AbilityStageContext/AbilityStageContext;"
    "L@ohos/app/appstartup/StartupConfig/StartupConfig;Lutils/AbilityUtils/AsyncCallbackWrapper;:V";
constexpr const char *SIGNATURE_STARTUP_MANAGER_REMOVE_ALL_STARTUP_TASK_RESULTS =
    ":V";
constexpr const char *SIGNATURE_STARTUP_MANAGER_GET_STARTUP_TASK_RESULTS =
    "Lstd/core/String;:Lstd/core/Object;";
constexpr const char *SIGNATURE_STARTUP_MANAGER_IS_STARTUP_TASK_INITIALIZED =
    "Lstd/core/String;:Lstd/core/Boolean;";
constexpr const char *SIGNATURE_STARTUP_MANAGER_REMOVE_STARTUP_TASK_RESULT =
    "Lstd/core/String;:V";
}

void ETSStartupManager::NativeRun(ani_env *env, ani_object startupTasks,
    ani_object abilityStageContext, ani_object startupConfig, ani_object callback)
{
    std::shared_ptr<StartupTaskManager> startupTaskManager = nullptr;
    int32_t result = RunStartupTask(env, startupTasks, abilityStageContext, startupConfig, startupTaskManager);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "RunStartupTask failed");
        return;
    }
    auto onCompletedCallback = std::make_shared<OnCompletedCallback>(
        [env, callback](const std::shared_ptr<StartupTaskResult> &result) {
            if (result == nullptr || result->GetResultCode() != ERR_OK) {
                ani_object resultValue = ETSStartupConfig::BuildResult(env, result);
                AppExecFwk::AsyncCallback(env, callback, resultValue, nullptr);
                return;
            }
            ani_object resultValue = ETSStartupConfig::BuildResult(env, result);
            AppExecFwk::AsyncCallback(env, callback, resultValue, nullptr);
            return;
        }
    );
    const auto timeoutCallback = []() {
        auto startupManager = DelayedSingleton<StartupManager>::GetInstance();
        if (startupManager == nullptr) {
            TAG_LOGE(AAFwkTag::STARTUP, "null startupManager");
            return;
        }
        AAFwk::EventInfo eventInfo;
        eventInfo.errCode = ERR_STARTUP_TIMEOUT;
        eventInfo.errMsg = "Manual task timeout.";
        eventInfo.bundleName = startupManager->GetBundleName();
        eventInfo.appIndex = startupManager->GetAppIndex();
        eventInfo.userId = startupManager->GetUid() / AppExecFwk::Constants::BASE_USER_RANGE;
        AAFwk::EventReport::SendAppStartupErrorEvent(
            AAFwk::EventName::APP_STARTUP_ERROR, HiSysEvent::FAULT, eventInfo);
    };
    startupTaskManager->SetTimeoutCallback(timeoutCallback);
    result = startupTaskManager->Run(onCompletedCallback);
    if (result != ERR_OK) {
        if (!onCompletedCallback->IsCalled()) {
            AbilityRuntimeErrorUtil::Throw(env, result, StartupUtils::GetErrorMessage(result));
            return;
        }
    }
}

ani_object ETSStartupManager::NativeGetStartupTaskResult(ani_env *env, ani_string startupTask)
{
    TAG_LOGI(AAFwkTag::STARTUP, "called");
    std::string strStartupTask;
    if (!AppExecFwk::GetStdString(env, startupTask, strStartupTask)) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetStdString failed.");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param startupTask failed");
        return nullptr;
    }
    std::shared_ptr<StartupTaskResult> result;
    int32_t res = DelayedSingleton<StartupManager>::GetInstance()->GetResult(strStartupTask, result);
    if (res != ERR_OK || result == nullptr || result->GetResultCode() != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "get %{public}s result failed", strStartupTask.c_str());
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Failed to get result.");
        return nullptr;
    }
    if (result->GetResultType() != StartupTaskResult::ResultType::ETS) {
        TAG_LOGE(AAFwkTag::STARTUP, "result type not ets");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: result type not ets.");
        return nullptr;
    }
    std::shared_ptr<EtsStartupTaskResult> etsResult = std::static_pointer_cast<EtsStartupTaskResult>(result);
    if (etsResult == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "ets result is null");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: ets result is null.");
        return nullptr;
    }
    ani_ref etsResultRef = etsResult->GetEtsStartupResultRef();
    return reinterpret_cast<ani_object>(etsResultRef);
}

bool ETSStartupManager::NativeIsStartupTaskInitialized(ani_env *env, ani_string startupTask)
{
    bool isInitialized = false;
    std::string strStartupTask;
    if (!AppExecFwk::GetStdString(env, startupTask, strStartupTask)) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetStdString failed.");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param startupTask failed");
        return isInitialized;
    }
    int32_t res = DelayedSingleton<StartupManager>::GetInstance()->IsInitialized(strStartupTask, isInitialized);
    if (res != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "get %{public}s result failed", strStartupTask.c_str());
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Failed to get result.");
        return isInitialized;
    }
    return isInitialized;
}

void ETSStartupManager::NativeRemoveStartupTaskResult(ani_env *env, ani_string startupTask)
{
    TAG_LOGI(AAFwkTag::STARTUP, "called");
    std::string strStartupTask;
    if (!AppExecFwk::GetStdString(env, startupTask, strStartupTask)) {
        TAG_LOGE(AAFwkTag::STARTUP, "GetStdString failed.");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param startupTask failed");
        return;
    }
    int32_t res = DelayedSingleton<StartupManager>::GetInstance()->RemoveResult(strStartupTask);
    if (res != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "remove %{public}s result failed", strStartupTask.c_str());
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Failed to remove result.");
        return;
    }
    return;
}

int32_t ETSStartupManager::RunStartupTask(ani_env *env, ani_object startupTasks, ani_object startupConfig,
    ani_object abilityStageContext, std::shared_ptr<StartupTaskManager> &startupTaskManager)
{
    std::vector<std::string> dependencies;
    if (!AppExecFwk::UnwrapArrayString(env, reinterpret_cast<ani_object>(startupTasks), dependencies)) {
        TAG_LOGE(AAFwkTag::STARTUP, "get dependencies failed");
        return ERR_STARTUP_INVALID_VALUE;
    }
    std::shared_ptr<StartupConfig> config;
    int32_t result = GetConfig(env, startupConfig, config);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "get config failed");
        return result;
    }
    bool supportFeatureModule = abilityStageContext != nullptr;
    result = DelayedSingleton<StartupManager>::GetInstance()->BuildAppStartupTaskManager(dependencies,
        startupTaskManager, supportFeatureModule);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "build startup task manager failed");
        return result;
    }
    if (startupTaskManager == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "startup task manager is null");
        return ERR_STARTUP_INVALID_VALUE;
    }
    if (abilityStageContext != nullptr) {
        startupTaskManager->UpdateStartupTaskContextRef(reinterpret_cast<ani_ref>(abilityStageContext), true);
    }
    result = startupTaskManager->Prepare();
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "prepare startup task manager failed");
        return result;
    }
    if (config != nullptr) {
        startupTaskManager->SetConfig(config);
    }
    return ERR_OK;
}

int32_t ETSStartupManager::GetConfig(ani_env *env, ani_object configObj, std::shared_ptr<StartupConfig> &config)
{
    std::shared_ptr<ETSStartupConfig> startupConfig = std::make_shared<ETSStartupConfig>(env);
    if (startupConfig == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "create startup config failed");
        return ERR_STARTUP_INVALID_VALUE;
    }
    if (startupConfig->Init(configObj) != ERR_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "init startup config failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parameter error: Failed to init config.");
        return ERR_STARTUP_INVALID_VALUE;
    }
    config = startupConfig;
    return ERR_OK;
}

void ETSStartupManager::NativeRemoveAllStartupTaskResults(ani_env *env)
{
    TAG_LOGD(AAFwkTag::STARTUP, "called");
    DelayedSingleton<StartupManager>::GetInstance()->RemoveAllResults();
    return;
}

void ETSStartupManagerInit(ani_env *env)
{
    TAG_LOGI(AAFwkTag::STARTUP, "Init startup manager");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::STARTUP, "env is null");
        return;
    }
    ani_class cls = nullptr;
    auto status = env->FindClass(ETS_STARTUP_MANAGER_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "find class failed, status: %{public}d", status);
        return;
    }

    std::array nativeFuncs = {
        ani_native_function { "nativeRun", SIGNATURE_STARTUP_MANAGER_RUN_ASYNCCALLBACK,
            reinterpret_cast<void*>(ETSStartupManager::NativeRun) },
        ani_native_function { "nativeRemoveAllStartupTaskResults",
            SIGNATURE_STARTUP_MANAGER_REMOVE_ALL_STARTUP_TASK_RESULTS,
            reinterpret_cast<void*>(ETSStartupManager::NativeRemoveAllStartupTaskResults) },
        ani_native_function { "nativeGetStartupTaskResult", SIGNATURE_STARTUP_MANAGER_GET_STARTUP_TASK_RESULTS,
            reinterpret_cast<void*>(ETSStartupManager::NativeGetStartupTaskResult) },
        ani_native_function { "nativeIsStartupTaskInitialized", SIGNATURE_STARTUP_MANAGER_IS_STARTUP_TASK_INITIALIZED,
            reinterpret_cast<void*>(ETSStartupManager::NativeIsStartupTaskInitialized) },
        ani_native_function { "nativeRemoveStartupTaskResult", SIGNATURE_STARTUP_MANAGER_REMOVE_STARTUP_TASK_RESULT,
            reinterpret_cast<void*>(ETSStartupManager::NativeRemoveStartupTaskResult) },
    };
    status = env->ClassBindNativeMethods(cls, nativeFuncs.data(), static_cast<int32_t>(nativeFuncs.size()));
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::STARTUP, "bind native methods failed, status: %{public}d", status);
        return;
    }
    TAG_LOGI(AAFwkTag::STARTUP, "Init startup manager success");
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

    ETSStartupManagerInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::STARTUP, "ANI_Constructor finish");
    return ANI_OK;
}
}
}
}