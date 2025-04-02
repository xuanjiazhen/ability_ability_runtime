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

#include "sts_app_manager.h"

#include "hilog_tag_wrapper.h"
#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "app_mgr_constants.h"
#include "app_mgr_interface.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AppManagerSts {
thread_local std::unique_ptr<AbilityRuntime::STSNativeReference> stsReference;

OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetAppManagerInstance()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(appObject);
}

OHOS::sptr<OHOS::AAFwk::IAbilityManager> GetAbilityManagerInstance()
{
    OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
        OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    OHOS::sptr<OHOS::IRemoteObject> abilityObject =
        systemAbilityManager->GetSystemAbility(OHOS::ABILITY_MGR_SERVICE_ID);
    return OHOS::iface_cast<OHOS::AAFwk::IAbilityManager>(abilityObject);
}

static void PreloadApplication(ani_env *env, [[maybe_unused]]ani_class aniClass,
    ani_string bundleName, ani_int userId, ani_int mode, ani_int appIndex)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "PreloadApplication");
    std::string bundleNameStr;
    ani_size sz {};
    env->String_GetUTF8Size(bundleName, &sz);
    bundleNameStr.resize(sz + 1);
    env->String_GetUTF8SubString(bundleName, 0, sz, bundleNameStr.data(), bundleNameStr.size(), &sz);
    TAG_LOGI(AAFwkTag::DELEGATOR, "PreloadApplication bundleName %{public}s", bundleNameStr.c_str());

    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "appMgr is null");
        return;
    }
    appMgr->PreloadApplication(bundleNameStr, userId, static_cast<AppExecFwk::PreloadMode>(mode), appIndex);
    TAG_LOGI(AAFwkTag::DELEGATOR, "PreloadApplication END");
}

static ani_boolean IsRunningInStabilityTest([[maybe_unused]] ani_env *env, [[maybe_unused]]ani_class aniClass)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "sts IsRunningInStabilityTest called");
    auto abilityManger = GetAbilityManagerInstance();
    if (!abilityManger) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "sts IsRunningInStabilityTest abilityManger is null");
        return 0;
    }
    bool ret = abilityManger->IsRunningInStabilityTest();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "sts IsRunningInStabilityTest ret : %{public}d", ret);
    return AppExecFwk::BoolToAniBoolean(ret);
}

static ani_int KillProcessWithAccount([[maybe_unused]] ani_env *env, ani_string bundleName, const ani_int accountId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccount called");
    std::string bundleNamestd;
    AppExecFwk::GetStdString(env, bundleName, bundleNamestd);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccount bundleName: %{public}s,\
        accountId: %{public}d", bundleNamestd.c_str(), accountId);
    auto appManger = GetAppManagerInstance();
    if (!appManger) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccount appmanger is null");
        return 0;
    }

    int ret = appManger->GetAmsMgr()->KillProcessWithAccount(bundleNamestd, accountId);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccount ret : %{public}d", ret);
    return ret;
}

static ani_int KillProcessWithAccountEx([[maybe_unused]] ani_env *env, ani_string bundleName,
    const ani_int accountId, ani_boolean clearPageStack, ani_object int_object)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccount called");
    std::string bundleNamestd;
    AppExecFwk::GetStdString(env, bundleName, bundleNamestd);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccount bundleName: %{public}s, accountId: %{public}d,\
        clearPageStack: %{public}d", bundleNamestd.c_str(), accountId, clearPageStack);
    
    ani_boolean isUndefined = false;
    env->Reference_IsUndefined(int_object, &isUndefined);
    if (!isUndefined) {
        ani_int result;
        if (ANI_OK !=env->Object_CallMethodByName_Int(int_object, "intValue", nullptr, &result)){
            TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccount Object_CallMethodByName_Int Fail");
        } else {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccount appIndex: %{public}d", result);
        }
    } 
    
    auto appManger = GetAppManagerInstance();
    if (!appManger) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccount appmanger is null");
        return 0;
    }
    int ret = appManger->GetAmsMgr()->KillProcessWithAccount(bundleNamestd, accountId, clearPageStack);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccount ret : %{public}d", ret);
    return ret;
}

static ani_int KillProcessesByBundleName([[maybe_unused]] ani_env *env, ani_string bundleName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts ExcuteiskillProcessesByBundleName called");
    std::string bundleNamestd;
    AppExecFwk::GetStdString(env, bundleName, bundleNamestd);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts ExcuteiskillProcessesByBundleName bundleName: %{public}s", bundleNamestd.c_str());
    auto abilityManger = GetAbilityManagerInstance();
    if (!abilityManger) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "sts ExcuteiskillProcessesByBundleName appmanger is null");
        return 0;
    }

    int ret = abilityManger->KillProcess(bundleNamestd);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts ExcuteiskillProcessesByBundleName ret : %{public}d", ret);
    return ret;
}

static ani_int KillProcessesByBundleNameEx([[maybe_unused]] ani_env *env, ani_string bundleName,
    ani_boolean clearPageStack, ani_object int_object)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessesByBundleNameEx called");
    std::string bundleNamestd;
    AppExecFwk::GetStdString(env, bundleName, bundleNamestd);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessesByBundleNameEx bundleName: %{public}s,\
        clearPageStack: %{public}d", bundleNamestd.c_str(), clearPageStack);
    
    ani_boolean isUndefined = false;
    env->Reference_IsUndefined(int_object, &isUndefined);
    if (!isUndefined) {
        ani_int result;
        if (ANI_OK !=env->Object_CallMethodByName_Int(int_object, "intValue", nullptr, &result)){
            TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessesByBundleNameEx Object_CallMethodByName_Int Fail");
        } else {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessesByBundleNameEx appIndex: %{public}d", result);
        }
    } 
    
    auto abilityManger = GetAbilityManagerInstance();
    if (!abilityManger) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessesByBundleNameEx abilityManger is null");
        return 0;
    }
    int ret = abilityManger->KillProcess(bundleNamestd, clearPageStack);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessesByBundleNameEx ret : %{public}d", ret);
    return ret;
}

static ani_int ClearUpApplicationData([[maybe_unused]] ani_env *env, ani_string bundleName)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts ExcuteclearUpApplicationData called");
    std::string bundleNamestd;
    AppExecFwk::GetStdString(env, bundleName, bundleNamestd);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts ExcuteclearUpApplicationData bundleName: %{public}s", bundleNamestd.c_str());

    auto appManger = GetAppManagerInstance();
    if (!appManger) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "sts ExcuteclearUpApplicationData appmanger is null");
        return 0;
    }

    int ret = appManger->ClearUpApplicationData(bundleNamestd, 0);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts ExcuteclearUpApplicationData ret : %{public}d", ret);
    return ret;
}

static ani_boolean IsRamConstrainedDevice([[maybe_unused]] ani_env *env)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts IsRamConstrainedDevice called");
    auto abilityManger = GetAbilityManagerInstance();
    if (!abilityManger) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "sts IsRamConstrainedDevice appmanger is null");
        return 0;
    }
    bool ret = abilityManger->IsRamConstrainedDevice();
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts IsRamConstrainedDevice ret : %{public}d", ret);
    return AppExecFwk::BoolToAniBoolean(ret);
}

static ani_ref GetForegroundApplications([[maybe_unused]] ani_env *env)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "sts GetForegroundApplications called");
    auto appManger = GetAppManagerInstance();
    if (!appManger) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "sts GetForegroundApplications appmanger is null");
        return nullptr;
    }
    std::vector<OHOS::AppExecFwk::AppStateData> vData;
    int ret = appManger->GetForegroundApplications(vData);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts GetForegroundApplications ret : %{public}d", ret);
    ani_object outAniObj = nullptr;
    if(!ConvertToAniAppStateDataArray(env, vData, outAniObj)) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "sts ConvertToAniAppStateDataArray fail");
        return nullptr;
    }
    return outAniObj;
}

void StsAppManagerRegistryInit(ani_env *env)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "StsAppManagerRegistryInit call");
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "ResetError failed");
    }

    ani_namespace ns;
    status = env->FindNamespace("L@ohos/app/ability/appManager/appManager;", &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "FindNamespace appManager failed status : %{public}d", status);
        return;
    }

    std::array kitFunctions = {
        ani_native_function {"nativePreloadApplication", nullptr, reinterpret_cast<void *>(PreloadApplication)},
        ani_native_function {"nativeIsRunningInStabilityTest", ":Z", reinterpret_cast<void *>(IsRunningInStabilityTest)},
        ani_native_function {"nativeKillProcessWithAccount", "Lstd/core/String;I:I", reinterpret_cast<void *>(KillProcessWithAccount)},
        ani_native_function {"nativeKillProcessWithAccountEx", nullptr, reinterpret_cast<void *>(KillProcessWithAccountEx)},
        ani_native_function {"nativeKillProcessesByBundleName", "Lstd/core/String;:I", reinterpret_cast<void *>(KillProcessesByBundleName)},
        ani_native_function {"nativeKillProcessesByBundleNameEx", nullptr, reinterpret_cast<void *>(KillProcessesByBundleNameEx)},
        ani_native_function {"nativeClearUpApplicationData", "Lstd/core/String;:I", reinterpret_cast<void *>(ClearUpApplicationData)},
        ani_native_function {"nativeIsRamConstrainedDevice", ":Z", reinterpret_cast<void *>(IsRamConstrainedDevice)},
        ani_native_function {"nativeGetForegroundApplications", ":Lescompat/Array;", reinterpret_cast<void *>(GetForegroundApplications)},
    };

    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "Namespace_BindNativeFunctions failed status : %{public}d", status);
    }

    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::STSRUNTIME, "ResetError failed");
    }

    TAG_LOGI(AAFwkTag::DELEGATOR, "StsAppManagerRegistryInit end");
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGI(AAFwkTag::DELEGATOR, "ANI_Constructor");
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGI(AAFwkTag::DELEGATOR, "GetEnv failed status : %{public}d", status);
        return ANI_NOT_FOUND;
    }

    StsAppManagerRegistryInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGI(AAFwkTag::DELEGATOR, "ANI_Constructor finish");
    return ANI_OK;
}
}

} // namespace AbilityDelegatorSts
} // namespace OHOS