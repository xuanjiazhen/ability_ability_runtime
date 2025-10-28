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

#include "ets_ability_manager.h"

#include "ability_business_error.h"
#include "ability_context.h"
#include "ability_manager_client.h"
#include "ability_manager_errors.h"
#include "ability_manager_interface.h"
#include "accesstoken_kit.h"
#include "ani_base_context.h"
#include "ani_common_ability_state_data.h"
#include "ani_common_util.h"
#include "application_context.h" 
#include "ani_common_want.h"
#include "app_mgr_interface.h"
#include "ets_ability_foreground_state_observer.h"
#include "ets_ability_manager_utils.h"
#include "ets_error_utils.h"
#include "ets_preload_ui_extension_callback_client.h"
#include "hilog_tag_wrapper.h"
#include "permission_constants.h"
#include "preload_ui_extension_host_client.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char *ETS_ABILITY_MANAGER_NAMESPACE = "L@ohos/app/ability/abilityManager/abilityManager;";
constexpr const char *ETS_ABILITY_MANAGER_SIGNATURE_ARRAY = ":Lescompat/Array;";
constexpr const char *ETS_ABILITY_MANAGER_SIGNATURE_CALLBACK = "Lutils/AbilityUtils/AsyncCallbackWrapper;:V";
constexpr const char *ETS_ABILITY_MANAGER_SIGNATURE_VOID = ":V";
constexpr const char *ON_OFF_TYPE_ABILITY_FOREGROUND_STATE = "abilityForegroundState";
constexpr const char *ON_OFF_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE = "C{std.core.Function1}:";
constexpr const char *PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE =
    "L@ohos/app/ability/Want/Want;Lutils/AbilityUtils/AsyncCallbackWrapper;:V";
constexpr const char *CLEAR_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE = "ILutils/AbilityUtils/AsyncCallbackWrapper;:V";
constexpr const char *CLEAR_PRELOAD_UI_EXTENSION_ABILITIES_SIGNATURE = "Lutils/AbilityUtils/AsyncCallbackWrapper;:V";
constexpr int32_t ERR_FAILURE = -1;
}

class EtsAbilityManager final {
public:
    static ani_object GetForegroundUIAbilities(ani_env *env);
    static void GetForegroundUIAbilitiesCallBack(ani_env *env, ani_object callbackObj);
    static void GetTopAbility(ani_env *env, ani_object callback);
    static void CheckSystemApp(ani_env *env);
    static void CheckPerloadUIExtensionAbilityPermission(ani_env *env);
    static void GetAbilityRunningInfos(ani_env *env, ani_object callback);
    static void IsEmbeddedOpenAllowed(ani_env *env, ani_object contextObj, ani_string aniAppId, ani_object callbackObj);
    static void NativeOn(ani_env *env, ani_string aniType, ani_object aniObserver);
    static void NativeOff(ani_env *env, ani_string aniType, ani_object aniObserver);
    static void GetExtensionRunningInfos(ani_env *env, ani_int upperLimit, ani_object callback);
    static void NativeOnPreloadUIExtensionAbilityLoaded(ani_env *env, ani_fn_object callback);
    static void NativeOffPreloadUIExtensionAbilityLoaded(ani_env *env, ani_fn_object callback);
    static void NativeOnPreloadUIExtensionAbilityDestroy(ani_env *env, ani_fn_object callback);
    static void NativeOffPreloadUIExtensionAbilityDestroy(ani_env *env, ani_fn_object callback);
    static void NativePreloadUIExtensionAbility(ani_env *env, ani_object aniWant, ani_object callback);
    static void NativeClearPreloadUIExtensionAbility(ani_env *env, ani_int preloadId, ani_object callback);
    static void NativeClearPreloadUIExtensionAbilities(ani_env *env, ani_object callback);
private:
    static sptr<AppExecFwk::IAbilityManager> GetAbilityManagerInstance();
    static sptr<AppExecFwk::IAppMgr> GetAppManagerInstance();
    static sptr<AbilityRuntime::ETSAbilityForegroundStateObserver> observerForeground_;
    static std::vector<std::pair<ani_ref, int32_t>> loadedCallback_;
    static std::vector<std::pair<ani_ref, int32_t>> destroyCallback_;
};


sptr<AbilityRuntime::ETSAbilityForegroundStateObserver> EtsAbilityManager::observerForeground_ = nullptr;
std::vector<std::pair<ani_ref, int32_t>> EtsAbilityManager::loadedCallback_ = {};
std::vector<std::pair<ani_ref, int32_t>> EtsAbilityManager::destroyCallback_ = {};

sptr<AppExecFwk::IAbilityManager> EtsAbilityManager::GetAbilityManagerInstance()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> abilityManagerObj = systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    return iface_cast<AppExecFwk::IAbilityManager>(abilityManagerObj);
}

sptr<AppExecFwk::IAppMgr> EtsAbilityManager::GetAppManagerInstance()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> appObject = systemAbilityManager->GetSystemAbility(APP_MGR_SERVICE_ID);
    return iface_cast<AppExecFwk::IAppMgr>(appObject);
}

ani_object EtsAbilityManager::GetForegroundUIAbilities(ani_env *env)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call GetForegroundUIAbilities");

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return nullptr;
    }

    sptr<AppExecFwk::IAbilityManager> abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityManager is null");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }
    std::vector<AppExecFwk::AbilityStateData> list;
    int32_t ret = abilityManager->GetForegroundUIAbilities(list);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed: ret=%{public}d", ret);
        AbilityRuntime::AbilityErrorCode code = AbilityRuntime::GetJsErrorCodeByNativeError(ret);
        AbilityRuntime::EtsErrorUtil::ThrowError(env, code);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetForegroundUIAbilities succeeds, list.size=%{public}zu", list.size());
    ani_object aniArray = AppExecFwk::CreateAniAbilityStateDataArray(env, list);
    if (aniArray == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null aniArray");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }
    return aniArray;
}

void EtsAbilityManager::GetForegroundUIAbilitiesCallBack(ani_env *env, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call GetForegroundUIAbilitiesCallBack");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }

    sptr<AppExecFwk::IAbilityManager> abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityManager is null");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }
    std::vector<AppExecFwk::AbilityStateData> list;
    int32_t ret = abilityManager->GetForegroundUIAbilities(list);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed: ret=%{public}d", ret);
        AbilityRuntime::AbilityErrorCode code = AbilityRuntime::GetJsErrorCodeByNativeError(ret);
        AppExecFwk::AsyncCallback(env, callbackObj, EtsErrorUtil::CreateError(env, code), nullptr);
        return;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetForegroundUIAbilities succeeds, list.size=%{public}zu", list.size());
    ani_object aniArray = AppExecFwk::CreateAniAbilityStateDataArray(env, list);
    if (aniArray == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null aniArray");
        AppExecFwk::AsyncCallback(env, callbackObj,
            EtsErrorUtil::CreateError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callbackObj, EtsErrorUtil::CreateErrorByNativeErr(env, ERR_OK), aniArray);
}

void EtsAbilityManager::CheckSystemApp(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
}

void EtsAbilityManager::CheckPerloadUIExtensionAbilityPermission(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(
        callerToken, AAFwk::PermissionConstants::PERMISSION_PRELOAD_UI_EXTENSION_ABILITY);
    if (result != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "VerifyAccessToken fail");
        EtsErrorUtil::ThrowNoPermissionError(env, AAFwk::PermissionConstants::PERMISSION_PRELOAD_UI_EXTENSION_ABILITY);
    }
}

void EtsAbilityManager::GetTopAbility(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call GetTopAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not system app");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
    AppExecFwk::ElementName elementName = AAFwk::AbilityManagerClient::GetInstance()->GetTopAbility();
    int resultCode = 0;
    ani_object elementNameobj = AppExecFwk::WrapElementName(env, elementName);
    if (elementNameobj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null elementNameobj");
        resultCode = ERR_FAILURE;
    }
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, resultCode),
        elementNameobj);
    return;
}

void EtsAbilityManager::GetAbilityRunningInfos(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetAbilityRunningInfos");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    std::vector<AAFwk::AbilityRunningInfo> infos;
    auto errcode = AAFwk::AbilityManagerClient::GetInstance()->GetAbilityRunningInfos(infos);
    ani_object retObject = nullptr;
    AbilityManagerEts::WrapAbilityRunningInfoArray(env, retObject, infos);
    AppExecFwk::AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, errcode), retObject);
}

void EtsAbilityManager::IsEmbeddedOpenAllowed(ani_env *env, ani_object contextObj,
    ani_string aniAppId, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "IsEmbeddedOpenAllowed");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, contextObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null context");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param context failed, must not be nullptr.");
        return;
    }
    auto uiAbilityContext = OHOS::AbilityRuntime::Context::ConvertTo<OHOS::AbilityRuntime::AbilityContext>(context);
    if (uiAbilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null UIAbilityContext");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param context failed, must be UIAbilityContext.");
        return;
    }
    std::string appId;
    if (!AppExecFwk::GetStdString(env, aniAppId, appId)) {
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param appId failed, must be a string.");
        return;
    }
    auto token = uiAbilityContext->GetToken();
    ani_boolean ret = AAFwk::AbilityManagerClient::GetInstance()->IsEmbeddedOpenAllowed(token, appId);
    AppExecFwk::AsyncCallback(env, callbackObj,
        EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK),  AppExecFwk::CreateBoolean(env, ret));
}

void EtsAbilityManager::NativeOn(ani_env *env, ani_string aniType, ani_object aniObserver)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "nativeOn called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "env null ptr");
        return;
    }
    std::string strType;
    if (!AppExecFwk::GetStdString(env, aniType, strType) || strType != ON_OFF_TYPE_ABILITY_FOREGROUND_STATE) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetStdString failed");
        EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param observer failed, must be a AbilityForegroundStateObserver.");
        return;
    }
    ani_vm *aniVM = nullptr;
    if (env->GetVM(&aniVM) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "get aniVM failed");
        EtsErrorUtil::ThrowInvalidParamError(env, "Get aniVm failed.");
        return;
    }
    if (observerForeground_ == nullptr) {
        observerForeground_ = sptr<ETSAbilityForegroundStateObserver>::MakeSptr(aniVM);
        if (observerForeground_ == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null observerForeground_");
            EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
            return;
        }
    }
    if (observerForeground_->IsEmpty()) {
        auto appManager = GetAppManagerInstance();
        if (appManager == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "appManager null ptr");
            EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
            return;
        }
        int32_t ret = appManager->RegisterAbilityForegroundStateObserver(observerForeground_);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ret: %{public}d", ret);
        if (ret != NO_ERROR) {
            EtsErrorUtil::ThrowErrorByNativeErr(env, static_cast<int32_t>(ret));
            return;
        }
    }
    observerForeground_->AddEtsObserverObject(env, aniObserver);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "nativeOn end");
}

void EtsAbilityManager::NativeOff(ani_env *env, ani_string aniType, ani_object aniObserver)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "nativeOff called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "env null ptr");
        return;
    }
    std::string strType;
    if (!AppExecFwk::GetStdString(env, aniType, strType)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetStdString failed");
        EtsErrorUtil::ThrowInvalidParamError(env,
            "Parse param observer failed, must be a AbilityForegroundStateObserver.");
        return;
    }
    if (observerForeground_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null observer");
        EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    ani_status status = ANI_OK;
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(aniObserver, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to check undefined status : %{public}d", status);
        return;
    }
    if (!isUndefined) {
        observerForeground_->RemoveEtsObserverObject(aniObserver);
    } else {
        observerForeground_->RemoveAllEtsObserverObject();
    }
    if (observerForeground_->IsEmpty()) {
        auto appManager = GetAppManagerInstance();
        if (appManager == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "appManager null ptr");
            EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
            return;
        }
        int32_t ret = appManager->UnregisterAbilityForegroundStateObserver(observerForeground_);
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ret: %{public}d", ret);
        if (ret != NO_ERROR) {
            EtsErrorUtil::ThrowErrorByNativeErr(env, static_cast<int32_t>(ret));
        }
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "nativeOff end");
}

void EtsAbilityManager::GetExtensionRunningInfos(ani_env *env, ani_int upperLimit, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call GetExtensionRunningInfos");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    std::vector<AAFwk::ExtensionRunningInfo> infos;
    auto errcode = AAFwk::AbilityManagerClient::GetInstance()->GetExtensionRunningInfos(upperLimit, infos);
    if (errcode != ERR_OK) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateErrorByNativeErr(env, errcode), nullptr);
        return;
    }

    ani_object extensionArray = nullptr;
    AbilityManagerEts::WrapExtensionRunningInfoArray(env, extensionArray, infos);
    if (extensionArray == nullptr) {
        AppExecFwk::AsyncCallback(env, callback,
            EtsErrorUtil::CreateErrorByNativeErr(env,
                static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER)), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback,
        EtsErrorUtil::CreateErrorByNativeErr(env, errcode), extensionArray);
}

void EtsAbilityManager::NativeOnPreloadUIExtensionAbilityLoaded(ani_env *env, ani_fn_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeOnPreloadUIExtensionAbilityLoaded");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    ani_ref callbackRef = nullptr;
    ani_status status = env->GlobalReference_Create(callback, &callbackRef);
    if (status != ANI_OK || callbackRef == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GlobalReference_Create failed or null callbackRef, status: %{public}d", status);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    ani_vm *vm = nullptr;
    if (env->GetVM(&vm) != ANI_OK || vm == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "GetVM failed or null vm");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    std::shared_ptr<EtsPreloadUIExtensionLoadedCallbackClient> client =
        std::make_shared<EtsPreloadUIExtensionLoadedCallbackClient>(vm, callbackRef);
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    std::string bundleName = context->GetBundleName();
    int32_t key = PreloadUIExtensionHostClient::GetInstance()->AddLoadedCallback(client, bundleName);
    TAG_LOGE(AAFwkTag::ABILITYMGR, "context bundleName %{public}s", bundleName.c_str());
    TAG_LOGE(AAFwkTag::ABILITYMGR, "key %{public}d", key);
    loadedCallback_.push_back({ callbackRef, key });
}

void EtsAbilityManager::NativeOffPreloadUIExtensionAbilityLoaded(ani_env *env, ani_fn_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeOffPreloadUIExtensionAbilityLoaded");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    std::string bundleName = context->GetBundleName();
    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(callback, &isUndefined);
    if (isUndefined) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "callback is undefined");
        PreloadUIExtensionHostClient::GetInstance()->RemoveAllLoadedCallback(bundleName);
        return;
    }

    for (auto it = loadedCallback_.begin(); it != loadedCallback_.end(); ++it) {
        ani_boolean isEquals = ANI_FALSE;
        ani_status status = env->Reference_StrictEquals(callback, it->first, &isEquals);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Reference_StrictEquals failed status: %{public}d", status);
        }
        if (isEquals) {
            PreloadUIExtensionHostClient::GetInstance()->RemoveLoadedCallback(it->second, bundleName);
            TAG_LOGE(AAFwkTag::ABILITYMGR, "context bundleName %{public}s", bundleName.c_str());
            TAG_LOGE(AAFwkTag::ABILITYMGR, "key %{public}d", it->second);
            loadedCallback_.erase(it);
            break;
        }
    }
}

void EtsAbilityManager::NativeOnPreloadUIExtensionAbilityDestroy(ani_env *env, ani_fn_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeOnPreloadUIExtensionAbilityDestroy");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    ani_ref callbackRef = nullptr;
    ani_status status = env->GlobalReference_Create(callback, &callbackRef);
    if (status != ANI_OK || callbackRef == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GlobalReference_Create failed or null callbackRef, status: %{public}d", status);
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    ani_vm *vm = nullptr;
    if (env->GetVM(&vm) != ANI_OK || vm == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "GetVM failed or null vm");
        EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    std::shared_ptr<EtsPreloadUIExtensionDestroyCallbackClient> client =
        std::make_shared<EtsPreloadUIExtensionDestroyCallbackClient>(vm, callbackRef);
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    std::string bundleName = context->GetBundleName();
    int32_t key = PreloadUIExtensionHostClient::GetInstance()->AddDestroyCallback(client, bundleName);
    TAG_LOGE(AAFwkTag::ABILITYMGR, "context bundleName %{public}s", bundleName.c_str());
    TAG_LOGE(AAFwkTag::ABILITYMGR, "key %{public}d", key);
    destroyCallback_.push_back({ callbackRef, key });
}

void EtsAbilityManager::NativeOffPreloadUIExtensionAbilityDestroy(ani_env *env, ani_fn_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeOffPreloadUIExtensionAbilityDestroy");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    std::string bundleName = context->GetBundleName();
    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(callback, &isUndefined);
    if (isUndefined) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "callback is undefined");
        PreloadUIExtensionHostClient::GetInstance()->RemoveAllDestroyCallback(bundleName);
        return;
    }

    for (auto it = destroyCallback_.begin(); it != destroyCallback_.end(); ++it) {
        ani_boolean isEquals = ANI_FALSE;
        ani_status status = env->Reference_StrictEquals(callback, it->first, &isEquals);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Reference_StrictEquals failed status: %{public}d", status);
        }
        if (isEquals) {
            PreloadUIExtensionHostClient::GetInstance()->RemoveDestroyCallback(it->second, bundleName);
            TAG_LOGE(AAFwkTag::ABILITYMGR, "context bundleName %{public}s", bundleName.c_str());
            TAG_LOGE(AAFwkTag::ABILITYMGR, "key %{public}d", it->second);
            destroyCallback_.erase(it);
            break;
        }
    }
}

void EtsAbilityManager::NativePreloadUIExtensionAbility(ani_env *env, ani_object aniWant, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativePreloadUIExtensionAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, aniWant, want)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to parse want");
        EtsErrorUtil::ThrowInvalidParamError(env, "Parse param want failed, must be a Want");
        return;
    }
    sptr<AppExecFwk::IAbilityManager> abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null abilityManager");
        AppExecFwk::AsyncCallback(
            env, callback, EtsErrorUtil::CreateError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER), nullptr);
        return;
    }
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    TAG_LOGE(AAFwkTag::ABILITYMGR, "context bundleName %{public}s", context->GetBundleName().c_str());
    std::string hostBundleName = context->GetBundleName();
    int32_t preloadId = 0;
    ErrCode result = abilityManager->PreloadUIExtensionAbilityWithId(want, hostBundleName, preloadId);
    TAG_LOGE(AAFwkTag::ABILITYMGR, "preloadId: %{public}d", preloadId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "PreloadUIExtensionAbilityWithId failed, result: %{public}d", result);
        AppExecFwk::AsyncCallback(
            env, callback, EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(result)), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, nullptr, AppExecFwk::CreateInt(env, preloadId));
}

void EtsAbilityManager::NativeClearPreloadUIExtensionAbility(ani_env *env, ani_int preloadId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeClearPreloadUIExtensionAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, nullptr, nullptr);
}

void EtsAbilityManager::NativeClearPreloadUIExtensionAbilities(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeClearPreloadUIExtensionAbilities");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, nullptr, nullptr);
}

void EtsAbilityManagerRegistryInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call EtsAbilityManagerRegistryInit");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ResetError failed");
    }
    ani_namespace ns = nullptr;
    status = env->FindNamespace(ETS_ABILITY_MANAGER_NAMESPACE, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "FindNamespace abilityManager failed status : %{public}d", status);
        return;
    }
    std::array methods = {
        ani_native_function { "nativeGetForegroundUIAbilities", ETS_ABILITY_MANAGER_SIGNATURE_ARRAY,
            reinterpret_cast<void *>(EtsAbilityManager::GetForegroundUIAbilities) },
        ani_native_function { "getForegroundUIAbilitiesCallback", "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsAbilityManager::GetForegroundUIAbilitiesCallBack) },
        ani_native_function { "nativeGetTopAbility", ETS_ABILITY_MANAGER_SIGNATURE_CALLBACK,
            reinterpret_cast<void *>(EtsAbilityManager::GetTopAbility) },
        ani_native_function { "nativeCheckSystemApp", ETS_ABILITY_MANAGER_SIGNATURE_VOID,
            reinterpret_cast<void *>(EtsAbilityManager::CheckSystemApp) },
        ani_native_function { "nativeCheckPerloadUIExtensionAbilityPermission", ETS_ABILITY_MANAGER_SIGNATURE_VOID,
            reinterpret_cast<void *>(EtsAbilityManager::CheckPerloadUIExtensionAbilityPermission) },
        ani_native_function { "nativeGetAbilityRunningInfos", "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsAbilityManager::GetAbilityRunningInfos) },
        ani_native_function { "nativeIsEmbeddedOpenAllowed",
            "Lapplication/Context/Context;Lstd/core/String;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsAbilityManager::IsEmbeddedOpenAllowed) },
        ani_native_function { "nativeOn", nullptr, reinterpret_cast<void *>(EtsAbilityManager::NativeOn) },
        ani_native_function { "nativeOff", nullptr, reinterpret_cast<void *>(EtsAbilityManager::NativeOff) },
        ani_native_function { "nativeGetExtensionRunningInfos", "ILutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsAbilityManager::GetExtensionRunningInfos) },
        ani_native_function { "nativeOnPreloadUIExtensionAbilityLoaded", ON_OFF_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeOnPreloadUIExtensionAbilityLoaded) },
        ani_native_function { "nativeOffPreloadUIExtensionAbilityLoaded", ON_OFF_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeOffPreloadUIExtensionAbilityLoaded) },
        ani_native_function { "nativeOnPreloadUIExtensionAbilityDestroy", ON_OFF_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeOnPreloadUIExtensionAbilityDestroy) },
        ani_native_function { "nativeOffPreloadUIExtensionAbilityDestroy",
            ON_OFF_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeOffPreloadUIExtensionAbilityDestroy) },
        ani_native_function { "nativePreloadUIExtensionAbility", PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativePreloadUIExtensionAbility) },
        ani_native_function { "nativeClearPreloadUIExtensionAbility", CLEAR_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeClearPreloadUIExtensionAbility) },
        ani_native_function { "nativeClearPreloadUIExtensionAbilities", CLEAR_PRELOAD_UI_EXTENSION_ABILITIES_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeClearPreloadUIExtensionAbilities) },

    };
    status = env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Namespace_BindNativeFunctions failed status : %{public}d", status);
    }
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ResetError failed");
    }
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "in AbilityManagerEts.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null vm or result");
        return ANI_INVALID_ARGS;
    }

    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetEnv failed, status=%{public}d", status);
        return ANI_NOT_FOUND;
    }
    EtsAbilityManagerRegistryInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityManagerEts.ANI_Constructor finished");
    return ANI_OK;
}
}
} // namespace AbilityManagerEts
} // namespace OHOS