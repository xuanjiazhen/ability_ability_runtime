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

#include "ani_common_child_process_param.h"
#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "child_process_manager.h"
#include "hilog_tag_wrapper.h"
#include "ets_error_utils.h"

namespace OHOS {
namespace ChildProcessManagerEts {
namespace {
constexpr const char* CHILD_PROCESS_MANAGER_NAME_SPACE = "L@ohos/app/ability/childProcessManager/childProcessManager;";
enum {
    MODE_SELF_FORK = 0,
    MODE_APP_SPAWN_FORK = 1,
};
struct ChildProcessAniParam {
    std::string srcEntry;
    AppExecFwk::ChildProcessArgs args;
    AppExecFwk::ChildProcessOptions options;
    int32_t childProcessType;
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

    static void StartArkChildProcess(ani_env *env, ani_string etsSrcEntry, ani_object ChildProcessArgs,
        ani_object callback)
    {
        GetInstance().OnStartArkChildProcess(env, etsSrcEntry, ChildProcessArgs, nullptr, callback);
    }

    static void StartArkChildProcessWithOptions(ani_env *env, ani_string etsSrcEntry, ani_object ChildProcessArgs,
        ani_object ChildProcessOptions, ani_object callback)
    {
        GetInstance().OnStartArkChildProcess(env, etsSrcEntry, ChildProcessArgs, ChildProcessOptions, callback);
    }

    static void StartNativeChildProcess(ani_env *env, ani_string etsSrcEntry, ani_object ChildProcessArgs,
        ani_object callback)
    {
        GetInstance().OnStartNativeChildProcess(env, etsSrcEntry, ChildProcessArgs, nullptr, callback);
    }

    static void StartNativeChildProcessWithOptions(ani_env *env, ani_string etsSrcEntry, ani_object ChildProcessArgs,
        ani_object ChildProcessOptions, ani_object callback)
    {
        GetInstance().OnStartNativeChildProcess(env, etsSrcEntry, ChildProcessArgs, ChildProcessOptions, callback);
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
            AbilityRuntime::EtsErrorUtil::ThrowError(env,
                AbilityRuntime::AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
            return;
        }
        std::string srcEntry;
        if (!AppExecFwk::GetStdString(env, etsSrcEntry, srcEntry)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse srcEntry failed");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param srcEntry failed, must be a valid string.");
            return;
        }
        TAG_LOGD(AAFwkTag::PROCESSMGR, "srcEntry %{public}s", srcEntry.c_str());
        int32_t startMode;
        if (!AAFwk::AniEnumConvertUtil::EnumConvert_EtsToNative(env, etsStartMode, startMode)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse startMode failed");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Unsupported startMode, must be StartMode.SELF_FORK or StartMode.APP_SPAWN_FORK.");
            return;
        }
        TAG_LOGD(AAFwkTag::PROCESSMGR, "startMode %{public}d", startMode);
        if (startMode != MODE_SELF_FORK && startMode != MODE_APP_SPAWN_FORK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Invalid StartMode");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
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
        TAG_LOGD(AAFwkTag::PROCESSMGR, "StartChildProcessBySelfFork errorCode: %{public}d, pid: %{public}d",
            errorCode, pid);
        if (errorCode != AbilityRuntime::ChildProcessManagerErrorCode::ERR_OK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "StartChildProcessBySelfFork failed, errorCode is %{public}d", errorCode);
            AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env,
                AbilityRuntime::ChildProcessManagerErrorUtil::GetAbilityErrorCode(errorCode)), nullptr);
            return;
        }
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env,
            AbilityRuntime::ChildProcessManagerErrorUtil::GetAbilityErrorCode(errorCode)),
            AppExecFwk::CreateDouble(env, static_cast<ani_double>(pid)));
    }

    static void StartChildProcessAppSpawnForkTask(ani_env *env, std::string &etsSrcEntry, ani_object callback)
    {
        pid_t pid = ERR_INVALID_VALUE;
        AbilityRuntime::ChildProcessManagerErrorCode innerErrorCode =
            AbilityRuntime::ChildProcessManager::GetInstance().StartChildProcessByAppSpawnFork(etsSrcEntry, pid);
        TAG_LOGD(AAFwkTag::PROCESSMGR, "StartChildProcessByAppSpawnFork innerErrorCode: %{public}d, pid: %{public}d",
            innerErrorCode, pid);
        if (innerErrorCode != AbilityRuntime::ChildProcessManagerErrorCode::ERR_OK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "StartChildProcessByAppSpawnFork failed, innerErrorCode is %{public}d",
                innerErrorCode);
            AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env,
                AbilityRuntime::ChildProcessManagerErrorUtil::GetAbilityErrorCode(innerErrorCode)), nullptr);
            return;
        }
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env,
            AbilityRuntime::ChildProcessManagerErrorUtil::GetAbilityErrorCode(innerErrorCode)),
            AppExecFwk::CreateDouble(env, static_cast<ani_double>(pid)));
    }

    void OnStartArkChildProcess(ani_env *env, ani_string etsSrcEntry, ani_object childProcessArgs,
        ani_object childProcessOptions, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::PROCESSMGR, "OnStartArkChildProcess Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "env is null");
            return;
        }
        if (AbilityRuntime::ChildProcessManager::GetInstance().IsChildProcessBySelfFork()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
            AbilityRuntime::EtsErrorUtil::ThrowError(env,
                AbilityRuntime::AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
            return;
        }
        std::string srcEntry;
        if (!AppExecFwk::GetStdString(env, etsSrcEntry, srcEntry)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse srcEntry failed");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param srcEntry failed, must be a valid string.");
            return;
        }
        if (srcEntry.empty()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "param srcEntry cannot be empty");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Param srcEntry cannot be empty.");
            return;
        }
        TAG_LOGD(AAFwkTag::PROCESSMGR, "srcEntry: %{public}s", srcEntry.c_str());
        AppExecFwk::ChildProcessArgs args;
        AppExecFwk::ChildProcessOptions options;
        if (!ParseArgsAndOptions(env, childProcessArgs, childProcessOptions, args, options)) {
            return;
        }
        ChildProcessAniParam param;
        param.srcEntry = srcEntry;
        param.args = args;
        param.options = options;
        param.childProcessType = AppExecFwk::CHILD_PROCESS_TYPE_ARK;
        StartChildProcessWithArgsTask(env, param, callback);
    }

    void OnStartNativeChildProcess(ani_env *env, ani_string etsEntryPoint, ani_object childProcessArgs,
        ani_object childProcessOptions, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::PROCESSMGR, "OnStartNativeChildProcess Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "env is null");
            return;
        }
        if (AbilityRuntime::ChildProcessManager::GetInstance().IsChildProcessBySelfFork()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Already in child process");
            AbilityRuntime::EtsErrorUtil::ThrowError(env,
                AbilityRuntime::AbilityErrorCode::ERROR_CODE_OPERATION_NOT_SUPPORTED);
            return;
        }
        std::string entryPoint;
        if (!AppExecFwk::GetStdString(env, etsEntryPoint, entryPoint)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Parse param entryPoint failed");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Parse param entryPoint failed, must be a valid string.");
            return;
        }
        if (entryPoint.empty()) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "Param entryPoint cannot be empty");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Param entryPoint cannot be empty.");
            return;
        }
        if (entryPoint.find(":") == std::string::npos) {
            TAG_LOGE(AAFwkTag::PROCESSMGR,
                "Param entryPoint must contain a colon to separate library name and entry function.");
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env,
                "Param entryPoint must contain a colon to separate library name and entry function.");
            return;
        }
        AppExecFwk::ChildProcessArgs args;
        AppExecFwk::ChildProcessOptions options;
        if (!ParseArgsAndOptions(env, childProcessArgs, childProcessOptions, args, options)) {
            return;
        }
        ChildProcessAniParam param;
        param.srcEntry = entryPoint;
        param.args = args;
        param.options = options;
        param.childProcessType = AppExecFwk::CHILD_PROCESS_TYPE_NATIVE_ARGS;
        StartChildProcessWithArgsTask(env, param, callback);
    }

    static bool ParseArgsAndOptions(ani_env *env, ani_object childProcessArgs, ani_object childProcessOptions,
        AppExecFwk::ChildProcessArgs &args, AppExecFwk::ChildProcessOptions &options)
    {
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "env is null");
            return false;
        }
        std::string errorMsg;
        if (!AppExecFwk::UnwrapChildProcessArgs(env, childProcessArgs, args, errorMsg)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "parse param args failed: %{private}s", errorMsg.c_str());
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, errorMsg);
            return false;
        }
        if (childProcessOptions &&
            !AppExecFwk::UnwrapChildProcessOptions(env, childProcessOptions, options, errorMsg)) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "parse param options failed: %{private}s", errorMsg.c_str());
            AbilityRuntime::EtsErrorUtil::ThrowInvalidParamError(env, errorMsg);
            return false;
        }
        return true;
    }

    static void StartChildProcessWithArgsTask(ani_env *env, const ChildProcessAniParam &param, ani_object callback)
    {
        pid_t pid = 0;
        TAG_LOGD(AAFwkTag::PROCESSMGR,
            "StartChildProcessWithArgs, childProcessType:%{public}d, srcEntry:%{private}s, "
            "args.entryParams size:%{public}zu, args.fds size:%{public}zu, options.isolationMode:%{public}d",
            param.childProcessType, param.srcEntry.c_str(), param.args.entryParams.length(),
            param.args.fds.size(), param.options.isolationMode);
        AbilityRuntime::ChildProcessManagerErrorCode errorCode =
            AbilityRuntime::ChildProcessManager::GetInstance().StartChildProcessWithArgs(
                param.srcEntry, pid, param.childProcessType, param.args, param.options);
        if (errorCode != AbilityRuntime::ChildProcessManagerErrorCode::ERR_OK) {
            TAG_LOGE(AAFwkTag::PROCESSMGR, "StartChildProcessWithArgs failed, errorCode: %{public}d", errorCode);
            AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env,
                AbilityRuntime::ChildProcessManagerErrorUtil::GetAbilityErrorCode(errorCode)), nullptr);
            return;
        }
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(env,
            AbilityRuntime::AbilityErrorCode::ERROR_OK), AppExecFwk::CreateDouble(env, static_cast<ani_double>(pid)));
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
            reinterpret_cast<void *>(EtsChildProcessManager::StartChildProcess)},
        ani_native_function {"nativeStartArkChildProcess",
            "Lstd/core/String;L@ohos/app/ability/ChildProcessArgs/ChildProcessArgs;"
            "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsChildProcessManager::StartArkChildProcess)},
        ani_native_function {"nativeStartArkChildProcess",
            "Lstd/core/String;L@ohos/app/ability/ChildProcessArgs/ChildProcessArgs;"
            "L@ohos/app/ability/ChildProcessOptions/ChildProcessOptions;"
            "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsChildProcessManager::StartArkChildProcessWithOptions)},
        ani_native_function {"nativeStartNativeChildProcess",
            "Lstd/core/String;L@ohos/app/ability/ChildProcessArgs/ChildProcessArgs;"
            "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsChildProcessManager::StartNativeChildProcess)},
        ani_native_function {"nativeStartNativeChildProcess",
            "Lstd/core/String;L@ohos/app/ability/ChildProcessArgs/ChildProcessArgs;"
            "L@ohos/app/ability/ChildProcessOptions/ChildProcessOptions;"
            "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsChildProcessManager::StartNativeChildProcessWithOptions)}
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