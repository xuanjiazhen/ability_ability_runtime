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

#include "ets_child_process_manager.h"

#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "child_process_manager.h"
#include "hilog_tag_wrapper.h"
#include "sts_error_utils.h"

namespace OHOS {
namespace ChildProcessManagerEts {
namespace {
constexpr const char* CHILD_PROCESS_MANAGER_NAME_SPACE = "L@ohos/app/ability/childProcessManager/childProcessManager;";
enum {
    MODE_SELF_FORK = 0,
    MODE_APP_SPAWN_FORK = 1,
};
}

class EtsChildProcessManager {
public:
    EtsChildProcessManager() = default;
    ~EtsChildProcessManager() = default;

    static EtsChildProcessManager &GetInstance()
    {
        static EtsChildProcessManager instance;
        return instance;
    }

    static void StartChildProcess(ani_env *env, ani_string etsSrcEntry, ani_enum_item etsStartMode,
        ani_object callback)
    {
        GetInstance().OnStartChildProcess(env, etsSrcEntry, etsStartMode, callback);
    }

private:
    void OnStartChildProcess(ani_env *env, ani_string etsSrcEntry, ani_enum_item etsStartMode, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::PROCESSMGR, "called");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "env is null");
            return;
        }

        if (AbilityRuntime::ChildProcessManager::GetInstance().IsChildProcess()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
            AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
            return;
        }

        std::string srcEntry;
        if (!AppExecFwk::GetStdString(env, etsSrcEntry, srcEntry)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse srcEntry failed");
            AbilityRuntime::ThrowStsInvalidParamError(env, "Parse param srcEntry failed, must be a valid string.");
            return;
        }
        TAG_LOGD(AAFwkTag::PROCESSMGR, "srcEntry %{public}s", srcEntry.c_str());

        ani_int startMode;
        if (!AAFwk::AniEnumConvertUtil::EnumConvertStsToNative(env, etsStartMode, startMode)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse startMode failed");
            AbilityRuntime::ThrowStsInvalidParamError(env,
                "Unsupported startMode, must be StartMode.SELF_FORK or StartMode.APP_SPAWN_FORK.");
            return;
        }
        TAG_LOGD(AAFwkTag::PROCESSMGR, "startMode %{public}d", startMode);

        if (startMode != MODE_SELF_FORK && startMode != MODE_APP_SPAWN_FORK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Invalid StartMode");
            AbilityRuntime::ThrowStsInvalidParamError(env, 
                "Unsupported startMode, must be StartMode.SELF_FORK or StartMode.APP_SPAWN_FORK.");
            return;
        }

        if (startMode == MODE_SELF_FORK) {
            StartChildProcessSelfForkTask(env, srcEntry, callback);
        } else {
            StartChildProcessAppSpawnForkTask(env, srcEntry, callback);
        }
    }

    static void StartChildProcessSelfForkTask(ani_env *env, std::string &etsSrcEntry, ani_object callback)
    {
        pid_t pid = 0;
        AbilityRuntime::ChildProcessManagerErrorCode errorCode =
            AbilityRuntime::ChildProcessManager::GetInstance().StartChildProcessBySelfFork(etsSrcEntry, pid);
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::CreateStsErrorByNativeErr(env, static_cast<int32_t>(errorCode)), nullptr);
    }

    static void StartChildProcessAppSpawnForkTask(ani_env *env, std::string &etsSrcEntry, ani_object callback)
    {
        pid_t pid = -1;
        AbilityRuntime::ChildProcessManagerErrorCode errorCode =
            AbilityRuntime::ChildProcessManager::GetInstance().StartChildProcessByAppSpawnFork(etsSrcEntry, pid);
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::CreateStsErrorByNativeErr(env, static_cast<int32_t>(errorCode)), nullptr);
    }
};

void EtsChildProcessManagerInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "EtsChildProcessManagerInit call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null env");
        return;
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "ResetError failed");
    }
    ani_status status = ANI_ERROR;
    ani_namespace ns;
    status = env->FindNamespace(CHILD_PROCESS_MANAGER_NAME_SPACE, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "FindNamespace childProcessManager failed status : %{public}d", status);
        return;
    }
    std::array kitFunctions = {
        ani_native_function {"nativeStartChildProcess",
            "Lstd/core/String;L@ohos/app/ability/childProcessManager/childProcessManager/StartMode;"
            "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsChildProcessManager::StartChildProcess)}
    };
    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "Namespace_BindNativeFunctions failed status : %{public}d", status);
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "ResetError failed");
    }
    TAG_LOGD(AAFwkTag::PROCESSMGR, "EtsChildProcessManagerInit end");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::PROCESSMGR, "in ChildProcessManagerEts.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "null vm or result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::PROCESSMGR, "GetEnv failed, status=%{public}d", status);
        return ANI_NOT_FOUND;
    }
    EtsChildProcessManagerInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::PROCESSMGR, "ChildProcessManagerEts.ANI_Constructor finished");
    return ANI_OK;
}
}
} // namespace ChildProcessManagerEts
} // namespace OHOS