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
#include "ani_common_want.h"
#include "app_mgr_interface.h"
#include "application_context.h"
#include "ets_ability_foreground_state_observer.h"
#include "ets_ability_manager_utils.h"
#include "ets_error_utils.h"
#include "ets_preload_ui_extension_callback_client.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "permission_constants.h"
#include "preload_ui_extension_host_client.h"
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
} // namespace

class EtsAbilityManager final {
public:
    static ani_object GetForegroundUIAbilities(ani_env *env);
    static void GetForegroundUIAbilitiesCallBack(ani_env *env, ani_object callbackObj);
    static void GetTopAbility(ani_env *env, ani_object callback);
    static void CheckSystemApp(ani_env *env);
    static void CheckPreloadUIExtensionAbilityPermission(ani_env *env);
    static void GetAbilityRunningInfos(ani_env *env, ani_object callback);
    static void IsEmbeddedOpenAllowed(ani_env *env, ani_object contextObj, ani_string aniAppId, ani_object callbackObj);
    static void NativeOn(ani_env *env, ani_string aniType, ani_object aniObserver);
    static void NativeOff(ani_env *env, ani_string aniType, ani_object aniObserver);
    static void GetExtensionRunningInfos(ani_env *env, ani_int upperLimit, ani_object callback);
    static void NativeOnPreloadedUIExtensionAbilityLoaded(ani_env *env, ani_fn_object callback);
    static void NativeOffPreloadedUIExtensionAbilityLoaded(ani_env *env, ani_fn_object callback);
    static void NativeOnPreloadedUIExtensionAbilityDestroyed(ani_env *env, ani_fn_object callback);
    static void NativeOffPreloadedUIExtensionAbilityDestroyed(ani_env *env, ani_fn_object callback);
    static void NativePreloadUIExtensionAbility(ani_env *env, ani_object aniWant, ani_object callback);
    static void NativeClearPreloadedUIExtensionAbility(ani_env *env, ani_int preloadId, ani_object callback);
    static void NativeClearPreloadedUIExtensionAbilities(ani_env *env, ani_object callback);

private:
    static sptr<AppExecFwk::IAbilityManager> GetAbilityManagerInstance();
    static sptr<AppExecFwk::IAppMgr> GetAppManagerInstance();
    static sptr<AbilityRuntime::ETSAbilityForegroundStateObserver> observerForeground_;
    static std::map<int32_t, std::vector<std::pair<ani_ref, int32_t>>> loadedCallbackMap_;
    static std::map<int32_t, std::vector<std::pair<ani_ref, int32_t>>> destroyCallbackMap_;
};

sptr<AbilityRuntime::ETSAbilityForegroundStateObserver> EtsAbilityManager::observerForeground_ = nullptr;
std::map<int32_t, std::vector<std::pair<ani_ref, int32_t>>> EtsAbilityManager::loadedCallbackMap_ = {};
std::map<int32_t, std::vector<std::pair<ani_ref, int32_t>>> EtsAbilityManager::destroyCallbackMap_ = {};

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

void EtsAbilityManager::CheckPreloadUIExtensionAbilityPermission(ani_env *env)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto callerToken = IPCSkeleton::GetCallingTokenID();
    int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(
        callerToken, AAFwk::PermissionConstants::PERMISSION_PRELOAD_UI_EXTENSION_ABILITY);
    if (result != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "VerifyAccessToken fail");
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

void EtsAbilityManager::NativeOnPreloadedUIExtensionAbilityLoaded(ani_env *env, ani_fn_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeOnPreloadedUIExtensionAbilityLoaded");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    auto &callbackVec = loadedCallbackMap_[callerPid];
    for (const auto &cb : callbackVec) {
        ani_boolean isEquals = ANI_FALSE;
        env->Reference_StrictEquals(callback, cb.first, &isEquals);
        if (isEquals) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "callback already exists");
            return;
        }
    }
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    std::string bundleName = context->GetBundleName();
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
    auto client = std::make_shared<EtsPreloadUIExtensionLoadedCallbackClient>(vm, callbackRef);
    int32_t key = PreloadUIExtensionHostClient::GetInstance()->AddLoadedCallback(client, bundleName, callerPid);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->RegisterPreloadUIExtensionHostClient(
        PreloadUIExtensionHostClient::GetInstance(), bundleName, callerPid);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "RegisterPreloadUIExtensionHostClient failed, ret: %{public}d", ret);
        AAFwk::AbilityManagerClient::GetInstance()->UnRegisterPreloadUIExtensionHostClient(bundleName, callerPid);
        EtsErrorUtil::ThrowErrorByNativeErr(env, ret);
    }
    callbackVec.push_back({ callbackRef, key });
}

void EtsAbilityManager::NativeOffPreloadedUIExtensionAbilityLoaded(ani_env *env, ani_fn_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeOffPreloadedUIExtensionAbilityLoaded");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    std::string bundleName = context->GetBundleName();
    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(callback, &isUndefined);
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    auto &callbackVec = loadedCallbackMap_[callerPid];
    if (isUndefined) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "callback is undefined");
        if (callbackVec.empty()) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null callback");
            return;
        }
        PreloadUIExtensionHostClient::GetInstance()->RemoveAllLoadedCallback(bundleName, callerPid);
        AAFwk::AbilityManagerClient::GetInstance()->UnRegisterPreloadUIExtensionHostClient(bundleName, callerPid);
        callbackVec.clear();
        return;
    }
    auto it = std::find_if(callbackVec.begin(), callbackVec.end(), [&](const auto &cb) {
        ani_boolean isEquals = ANI_FALSE;
        env->Reference_StrictEquals(callback, cb.first, &isEquals);
        return isEquals;
    });
    if (it == callbackVec.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callback not found");
        return;
    }
    PreloadUIExtensionHostClient::GetInstance()->RemoveLoadedCallback(it->second, bundleName, callerPid);
    AAFwk::AbilityManagerClient::GetInstance()->UnRegisterPreloadUIExtensionHostClient(bundleName, callerPid);
    callbackVec.erase(it);
}

void EtsAbilityManager::NativeOnPreloadedUIExtensionAbilityDestroyed(ani_env *env, ani_fn_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeOnPreloadedUIExtensionAbilityDestroyed");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    auto &callbackVec = destroyCallbackMap_[callerPid];
    for (const auto &cb : callbackVec) {
        ani_boolean isEquals = ANI_FALSE;
        env->Reference_StrictEquals(callback, cb.first, &isEquals);
        if (isEquals) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "callback already exists");
            return;
        }
    }
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    std::string bundleName = context->GetBundleName();
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
    auto client = std::make_shared<EtsPreloadUIExtensionDestroyCallbackClient>(vm, callbackRef);
    int32_t key = PreloadUIExtensionHostClient::GetInstance()->AddDestroyCallback(client, bundleName, callerPid);
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->RegisterPreloadUIExtensionHostClient(
        PreloadUIExtensionHostClient::GetInstance(), bundleName, callerPid);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "RegisterPreloadUIExtensionHostClient failed, ret: %{public}d", ret);
        AAFwk::AbilityManagerClient::GetInstance()->UnRegisterPreloadUIExtensionHostClient(bundleName, callerPid);
        EtsErrorUtil::ThrowErrorByNativeErr(env, ret);
    }
    callbackVec.push_back({ callbackRef, key });
}

void EtsAbilityManager::NativeOffPreloadedUIExtensionAbilityDestroyed(ani_env *env, ani_fn_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeOffPreloadedUIExtensionAbilityDestroyed");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    std::string bundleName = context->GetBundleName();
    ani_boolean isUndefined = true;
    env->Reference_IsUndefined(callback, &isUndefined);
    int32_t callerPid = IPCSkeleton::GetCallingPid();
    auto &callbackVec = destroyCallbackMap_[callerPid];
    if (isUndefined) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "callback is undefined");
        if (callbackVec.empty()) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "null callback");
            return;
        }
        PreloadUIExtensionHostClient::GetInstance()->RemoveAllDestroyCallback(bundleName, callerPid);
        AAFwk::AbilityManagerClient::GetInstance()->UnRegisterPreloadUIExtensionHostClient(bundleName, callerPid);
        callbackVec.clear();
        return;
    }
    auto it = std::find_if(callbackVec.begin(), callbackVec.end(), [&](const auto &cb) {
        ani_boolean isEquals = ANI_FALSE;
        env->Reference_StrictEquals(callback, cb.first, &isEquals);
        return isEquals;
    });
    if (it == callbackVec.end()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "callback not found");
        return;
    }
    PreloadUIExtensionHostClient::GetInstance()->RemoveDestroyCallback(it->second, bundleName, callerPid);
    AAFwk::AbilityManagerClient::GetInstance()->UnRegisterPreloadUIExtensionHostClient(bundleName, callerPid);
    callbackVec.erase(it);
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
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    TAG_LOGE(AAFwkTag::ABILITYMGR, "context bundleName %{public}s", context->GetBundleName().c_str());
    std::string hostBundleName = context->GetBundleName();
    int32_t preloadId = 0;
    ErrCode result =
        AAFwk::AbilityManagerClient::GetInstance()->PreloadUIExtensionAbilityWithId(want, hostBundleName, preloadId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "PreloadUIExtensionAbilityWithId failed, result: %{public}d", result);
        AppExecFwk::AsyncCallback(
            env, callback, EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(result)), nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, callback, nullptr, AppExecFwk::CreateInt(env, preloadId));
}

void EtsAbilityManager::NativeClearPreloadedUIExtensionAbility(ani_env *env, ani_int preloadId, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeClearPreloadedUIExtensionAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    ErrCode result = AAFwk::AbilityManagerClient::GetInstance()->UnPreloadUIExtensionAbility(preloadId);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "UnPreloadUIExtensionAbility failed, result: %{public}d", result);
        AppExecFwk::AsyncCallback(
            env, callback, EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(result)), nullptr);
    }
}

void EtsAbilityManager::NativeClearPreloadedUIExtensionAbilities(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call NativeClearPreloadedUIExtensionAbilities");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    auto context = OHOS::AbilityRuntime::Context::GetApplicationContext();
    std::string hostBundleName = context->GetBundleName();
    ErrCode result = AAFwk::AbilityManagerClient::GetInstance()->ClearAllPreloadUIExtensionAbility(hostBundleName);
    if (result != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ClearAllPreloadUIExtensionAbility failed, result: %{public}d", result);
        AppExecFwk::AsyncCallback(
            env, callback, EtsErrorUtil::CreateError(env, static_cast<AbilityErrorCode>(result)), nullptr);
    }
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
        ani_native_function { "nativeCheckPreloadUIExtensionAbilityPermission", ETS_ABILITY_MANAGER_SIGNATURE_VOID,
            reinterpret_cast<void *>(EtsAbilityManager::CheckPreloadUIExtensionAbilityPermission) },
        ani_native_function { "nativeGetAbilityRunningInfos", "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsAbilityManager::GetAbilityRunningInfos) },
        ani_native_function { "nativeIsEmbeddedOpenAllowed",
            "Lapplication/Context/Context;Lstd/core/String;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsAbilityManager::IsEmbeddedOpenAllowed) },
        ani_native_function { "nativeOn", nullptr, reinterpret_cast<void *>(EtsAbilityManager::NativeOn) },
        ani_native_function { "nativeOff", nullptr, reinterpret_cast<void *>(EtsAbilityManager::NativeOff) },
        ani_native_function { "nativeGetExtensionRunningInfos", "ILutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsAbilityManager::GetExtensionRunningInfos) },
        ani_native_function { "nativeOnPreloadedUIExtensionAbilityLoaded",
            ON_OFF_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeOnPreloadedUIExtensionAbilityLoaded) },
        ani_native_function { "nativeOffPreloadedUIExtensionAbilityLoaded",
            ON_OFF_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeOffPreloadedUIExtensionAbilityLoaded) },
        ani_native_function { "nativeOnPreloadedUIExtensionAbilityDestroyed",
            ON_OFF_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeOnPreloadedUIExtensionAbilityDestroyed) },
        ani_native_function { "nativeOffPreloadedUIExtensionAbilityDestroyed",
            ON_OFF_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeOffPreloadedUIExtensionAbilityDestroyed) },
        ani_native_function { "nativePreloadUIExtensionAbility", PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativePreloadUIExtensionAbility) },
        ani_native_function { "nativeClearPreloadedUIExtensionAbility", CLEAR_PRELOAD_UI_EXTENSION_ABILITY_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeClearPreloadedUIExtensionAbility) },
        ani_native_function { "nativeClearPreloadedUIExtensionAbilities",
            CLEAR_PRELOAD_UI_EXTENSION_ABILITIES_SIGNATURE,
            reinterpret_cast<void *>(EtsAbilityManager::NativeClearPreloadedUIExtensionAbilities) },
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