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

#include "ets_modular_object_manager.h"

#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "ani_modular_object_extension_info.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* MODULAR_OBJECT_MANAGER_SPACE_NAME = "@ohos.app.ability.modularObjectManager.modularObjectManager";
} // namespace
class EtsModularObjectManager final {
    struct AinStringArgs {
        ani_string bundleName;
        ani_string moduleName;
        ani_string abilityName;
    };
    struct StringArgs {
        std::string bundleName;
        std::string moduleName;
        std::string abilityName;
    };

public:
    static void QueryAllModularObjectExtension(ani_env *env, ani_object callback);
    static void SetModularObjectDisabledState(ani_env *env, ani_string aniBundleName, ani_string aniModuleName,
        ani_string aniAbilityName, ani_int aniAppIndex, ani_boolean aniIsDisabled, ani_object callback);
    static void SetModularObjectGlobalDisabledState(ani_env *env, ani_boolean aniIsDisabled, ani_object callback);
    static void GetModularObjectGlobalDisabledState(ani_env *env, ani_object callback);

private:
    static bool SetModularObjectDisabledStateGetStringArgs(ani_env *env, const AinStringArgs &aniStringArgs,
        StringArgs &stringArgs, ani_object callback);
    static sptr<AAFwk::IAbilityManager> GetAbilityManagerInstance();
};

void EtsModularObjectManager::QueryAllModularObjectExtension(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ANI, "QueryAllModularObjectExtension called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "env null ptr");
        return;
    }

    auto abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "abilityManager null ptr");
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(
            env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }

    ani_object emptyArray = CreateEmptyAniArray(env);
    std::vector<AAFwk::ModularObjectExtensionInfo> infos;
    auto ret = abilityManager->QueryAllModularObjectExtensionInfo(infos);
    if (ret != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), emptyArray);
        return;
    }
    ani_object aniInfosRef = WrapModularObjectExtensionInfoArray(env, infos);
    if (aniInfosRef == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(
                env, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), emptyArray);
    } else {
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), aniInfosRef);
    }
    TAG_LOGD(AAFwkTag::ANI, "QueryAllModularObjectExtension finished");
}

void EtsModularObjectManager::SetModularObjectDisabledState(
    ani_env *env, ani_string aniBundleName, ani_string aniModuleName,
    ani_string aniAbilityName, ani_int aniAppIndex, ani_boolean aniIsDisabled, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ANI, "SetModularObjectDisabledState called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "env null ptr");
        return;
    }
    auto abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "abilityManager null ptr");
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(
            env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }
    AinStringArgs aniStringArgs {
        .bundleName = aniBundleName,
        .moduleName = aniModuleName,
        .abilityName = aniAbilityName
    };
    StringArgs stringArgs;
    if (!SetModularObjectDisabledStateGetStringArgs(env, aniStringArgs, stringArgs, callback)) {
        return;
    }

    auto ret = abilityManager->SetModularObjectDisabledState(stringArgs.bundleName, stringArgs.moduleName,
        stringArgs.abilityName, aniAppIndex, aniIsDisabled);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
    TAG_LOGD(AAFwkTag::ANI, "SetModularObjectDisabledState finished");
}

void EtsModularObjectManager::SetModularObjectGlobalDisabledState(ani_env *env, ani_boolean aniIsDisabled,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::ANI, "SetModularObjectGlobalDisabledState called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "env null ptr");
        return;
    }
    auto abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "abilityManager null ptr");
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(
            env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }

    auto ret = abilityManager->SetModularObjectGlobalDisabledState(aniIsDisabled);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
    TAG_LOGD(AAFwkTag::ANI, "SetModularObjectGlobalDisabledState finished");
}

void EtsModularObjectManager::GetModularObjectGlobalDisabledState(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ANI, "GetModularObjectGlobalDisabledState called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "env null ptr");
        return;
    }
    auto abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "abilityManager null ptr");
        AppExecFwk::AsyncCallback(env, callback, AbilityRuntime::EtsErrorUtil::CreateError(
            env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }

    bool disabledState = false;
    auto ret = abilityManager->GetModularObjectGlobalDisabledState(disabledState);
    AppExecFwk::AsyncCallback(env, callback,
        AbilityRuntime::EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(ret)),
        AppExecFwk::CreateBoolean(env, static_cast<ani_boolean>(disabledState)));
    TAG_LOGD(AAFwkTag::ANI, "GetModularObjectGlobalDisabledState finished");
}

bool EtsModularObjectManager::SetModularObjectDisabledStateGetStringArgs(ani_env *env,
    const AinStringArgs &aniStringArgs, StringArgs &stringArgs, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "env null ptr");
        return false;
    }

    if (!AppExecFwk::GetStdString(env, aniStringArgs.bundleName, stringArgs.bundleName)) {
        TAG_LOGE(AAFwkTag::ANI, "GetStdString Failed");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param bundleName failed, must be a string."), nullptr);
        return false;
    }
    if (!AppExecFwk::GetStdString(env, aniStringArgs.moduleName, stringArgs.moduleName)) {
        TAG_LOGE(AAFwkTag::ANI, "GetStdString Failed");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param moduleName failed, must be a string."), nullptr);
        return false;
    }
    if (!AppExecFwk::GetStdString(env, aniStringArgs.abilityName, stringArgs.abilityName)) {
        TAG_LOGE(AAFwkTag::ANI, "GetStdString Failed");
        AppExecFwk::AsyncCallback(env, callback,
            AbilityRuntime::EtsErrorUtil::CreateInvalidParamError(
                env, "Parse param abilityName failed, must be a string."), nullptr);
        return false;
    }
    return true;
}

sptr<AAFwk::IAbilityManager> EtsModularObjectManager::GetAbilityManagerInstance()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> abilityObject =
        systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    return iface_cast<AAFwk::IAbilityManager>(abilityObject);
}

void EtsModularObjectManagerRegistryInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::ANI, "EtsModularObjectManagerRegistryInit call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "env null ptr");
        return;
    }
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "ResetError failed");
    }
    ani_namespace ns;
    status = env->FindNamespace(MODULAR_OBJECT_MANAGER_SPACE_NAME, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI,
            "FindNamespace modularObjectManager failed status : %{public}d", status);
        return;
    }
    std::array kitFunctions = {
        ani_native_function{"nativeQueryAllModularObjectExtension", nullptr,
            reinterpret_cast<void *>(EtsModularObjectManager::QueryAllModularObjectExtension)},
        ani_native_function{"nativeSetModularObjectDisabledState", nullptr,
            reinterpret_cast<void *>(EtsModularObjectManager::SetModularObjectDisabledState)},
        ani_native_function{"nativeSetModularObjectGlobalDisabledState", nullptr,
            reinterpret_cast<void *>(EtsModularObjectManager::SetModularObjectGlobalDisabledState)},
        ani_native_function{"nativeGetModularObjectGlobalDisabledState", nullptr,
            reinterpret_cast<void *>(EtsModularObjectManager::GetModularObjectGlobalDisabledState)},
    };
    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "Namespace_BindNativeFunctions failed status : %{public}d", status);
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "ResetError failed");
    }
    TAG_LOGD(AAFwkTag::ANI, "EtsModularObjectManagerRegistryInit end");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::ANI, "in EtsModularObjectManager.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::ANI, "null vm or result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ANI, "GetEnv failed, status=%{public}d", status);
        return ANI_NOT_FOUND;
    }
    EtsModularObjectManagerRegistryInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::ANI, "EtsModularObjectManager.ANI_Constructor finished");
    return ANI_OK;
}
}  // extern "C"
}  // namespace AbilityRuntime
}  // namespace OHOS
