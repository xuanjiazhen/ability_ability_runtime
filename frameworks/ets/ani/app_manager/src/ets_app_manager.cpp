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

#include "ets_app_manager.h"

#include "hilog_tag_wrapper.h"
#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "app_mgr_constants.h"
#include "app_mgr_interface.h"
#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "app_mgr_constants.h"
#include "app_mgr_interface.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "ets_error_utils.h"

namespace OHOS {
namespace AppManagerEts {

const char* APP_MANAGER_CLASS_NAME = "L@ohos/app/ability/appManager/appManager;";
class EtsAppManager final {
public:
    static OHOS::sptr<OHOS::AppExecFwk::IAppMgr> GetAppManagerInstance()
    {
        OHOS::sptr<OHOS::ISystemAbilityManager> systemAbilityManager =
            OHOS::SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        OHOS::sptr<OHOS::IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(OHOS::APP_MGR_SERVICE_ID);
        return OHOS::iface_cast<OHOS::AppExecFwk::IAppMgr>(appObject);
    }

    static void PreloadApplication(ani_env *env, ani_object callback, ani_string etsBundleName,
        ani_double etsUserId, ani_enum_item etsMode, ani_object etsAppIndex)
    {
    TAG_LOGD(AAFwkTag::APPMGR, "PreloadApplication");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "env is null");
            return;
        }
        std::string bundleName;
        if (!OHOS::AppExecFwk::GetStdString(env, etsBundleName, bundleName)) {
            TAG_LOGE(AAFwkTag::APPMGR, "param bundlename err");
            AppExecFwk::AsyncCallback(env, callback,
                OHOS::AbilityRuntime::CreateEtsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM)), nullptr);
            return;
        }
        TAG_LOGD(AAFwkTag::APPMGR, "PreloadApplication userId:%{public}f, bundleName %{public}s",
            etsUserId, bundleName.c_str());
        int32_t userId = static_cast<int32_t>(etsUserId);

        ani_int mode;
        if (!AAFwk::AniEnumConvertUtil::EnumConvertEtsToNative(env, etsMode, mode)) {
            TAG_LOGE(AAFwkTag::APPMGR, "param mode err");
            AppExecFwk::AsyncCallback(env, callback,
                OHOS::AbilityRuntime::CreateEtsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM)), nullptr);
            return;
        }
        ani_status status = ANI_OK;
        int32_t appIndex = 0;
        ani_boolean isUndefined = false;
        if ((status = env->Reference_IsUndefined(etsAppIndex, &isUndefined)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::APPMGR, "Failed to check undefined status : %{public}d", status);
            return;
        }
        ani_double dval = 0.0;
        if (!isUndefined) {
            if ((status = env->Object_CallMethodByName_Double(etsAppIndex,
                "doubleValue", nullptr, &dval)) != ANI_OK) {
                TAG_LOGE(AAFwkTag::APPMGR, "Object_CallMethodByName_Double status : %{public}d", status);
                return;
            }
            TAG_LOGD(AAFwkTag::APPMGR, "etsAppIndex: %{public}f", dval);
            appIndex = static_cast<int32_t>(dval);
        }
        TAG_LOGD(AAFwkTag::APPMGR, "PreloadApplication userId:%{public}d, mode:%{public}d, appIndex:%{public}d",
            userId, mode, appIndex);

        sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
        if (appMgr == nullptr) {
            TAG_LOGE(AAFwkTag::APPMGR, "appManager null ptr");
            AppExecFwk::AsyncCallback(env, callback,
                OHOS::AbilityRuntime::CreateEtsErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), nullptr);
            return;
        }
        auto ret = appMgr->PreloadApplication(bundleName, userId, static_cast<AppExecFwk::PreloadMode>(mode),
            appIndex);
        TAG_LOGD(AAFwkTag::APPMGR, "PreloadApplication ret %{public}d", ret);
        AppExecFwk::AsyncCallback(env, callback,
            OHOS::AbilityRuntime::CreateEtsErrorByNativeErr(env, static_cast<int32_t>(ret)), nullptr);
        TAG_LOGD(AAFwkTag::APPMGR, "PreloadApplication END");
    }
};

void EtsAppManagerRegistryInit(ani_env *env)
{
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ResetError failed");
    }

    ani_namespace ns;
    status = env->FindNamespace(APP_MANAGER_CLASS_NAME, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindNamespace appManager failed status : %{public}d", status);
        return;
    }

    std::array kitFunctions = {
        ani_native_function {
            "nativePreloadApplication", nullptr, reinterpret_cast<void *>(EtsAppManager::PreloadApplication)},
    };

    status = env->Namespace_BindNativeFunctions(ns, kitFunctions.data(), kitFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Namespace_BindNativeFunctions failed status : %{public}d", status);
    }

    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "ResetError failed");
    }
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::APPMGR, "in AppManagerEts.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null vm or result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetEnv failed, status=%{public}d", status);
        return ANI_NOT_FOUND;
    }
    EtsAppManagerRegistryInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::APPMGR, "AppManagerEts.ANI_Constructor finished");
    return ANI_OK;
}
} // extern "C"
} // namespace AppManagerEts
} // namespace OHOS