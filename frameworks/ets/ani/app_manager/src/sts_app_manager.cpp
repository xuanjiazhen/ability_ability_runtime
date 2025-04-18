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
#include "ipc_skeleton.h"
#include "hilog_tag_wrapper.h"
#include "ability_manager_client.h"
#include "ability_manager_interface.h"
#include "app_mgr_constants.h"
#include "app_mgr_interface.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "sts_app_manager_utils.h"
#include "sts_app_state_observer.h"
#include "sts_app_foreground_state_observer.h"
#include "ani_common_util.h"
#include "sts_error_utils.h"
#ifdef SUPPORT_GRAPHICS
#include "sts_ability_first_frame_state_observer.h"
#ifdef SUPPORT_SCREEN
#include "tokenid_kit.h"
#endif
#endif
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

#ifdef SUPPORT_SCREEN
    static bool CheckCallerIsSystemApp()
    {
        auto selfToken = OHOS::IPCSkeleton::GetSelfTokenID();
        if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
            return false;
        }
        return true;
    }
#endif

class StsObserverManager final {
public:
    static StsObserverManager* GetInstance() {
        static StsObserverManager instance;
        return &instance;
    }
    ~StsObserverManager() {}

    ani_int OnOnApplicationStateObserver(ani_env *env, ani_object observer, std::vector<std::string> &bundleNameList);
    void OnOnAppForegroundStateObserver(ani_env *env, ani_object observer);
    void OnOffApplicationStateObserver(ani_env *env, ani_int observerId);
    void OnOffAppForegroundStateObserver(ani_env *env, ani_object observer);
    void OnOffAllAppForegroundStateObserver(ani_env *env);
#ifdef SUPPORT_SCREEN
    void OnOnAbilityFirstFrameStateObserver(ani_env *env, ani_object observer, ani_string bundleName);
    void OnOffAbilityFirstFrameStateObserver(ani_env *env, ani_object observer);
    void OnOffAllAbilityFirstFrameStateObserver(ani_env *env);
#endif

private:
    StsObserverManager(){}
    DISALLOW_COPY_AND_MOVE(StsObserverManager);
    sptr<AniAppStateObserver> GetAppStateObserver(ani_env *env) {
        std::lock_guard<std::mutex> lock(aniObserverLock_);
        if (observer_ == nullptr) {
            observer_ = new AniAppStateObserver(env);
        }
        return observer_;
    }
    sptr<AniAppForegroundStateObserver> GetObserverForeground(ani_env *env) {
        std::lock_guard<std::mutex> lock(aniForegroundObserverLock_);
        if (observerForeground_ == nullptr) {
            observerForeground_ = new AniAppForegroundStateObserver(env);
        }
        return observerForeground_;
    }
private:
    sptr<AniAppStateObserver> observer_ = nullptr;
    sptr<AniAppForegroundStateObserver> observerForeground_ = nullptr;
    int32_t serialNumber_ = 0;
    std::mutex aniObserverLock_;
    std::mutex aniForegroundObserverLock_;
};

ani_int StsObserverManager::OnOnApplicationStateObserver(ani_env *env, ani_object observer, std::vector<std::string> &bundleNameList)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOnApplicationStateObserver called");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer is null");
        AbilityRuntime::ThrowStsInvalidParamError(env, "observer is null");
        return -1;
    }
    sptr<AniAppStateObserver> pObserver = GetAppStateObserver(env);
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr is null");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return -1;
    }
    int32_t ret = appMgr->RegisterApplicationStateObserver(pObserver, bundleNameList);
    if (ret != 0) {
        TAG_LOGD(AAFwkTag::APPMGR, "RegisterApplicationStateObserver faild. ret %{public}d", ret);
        AbilityRuntime::ThrowStsErrorByNativeErr(env, ret);
        return -1;
    }
    int64_t observerId = serialNumber_;
    pObserver->AddAniObserverObject(observerId, observer);
    if (serialNumber_ < INT32_MAX) {
        serialNumber_++;
    } else {
        serialNumber_ = 0;
    }
    return observerId;
}

void StsObserverManager::OnOnAppForegroundStateObserver(ani_env *env, ani_object observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOnAppForegroundStateObserver called");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer is null");
        AbilityRuntime::ThrowStsInvalidParamError(env, "observer is null");
        return;
    }
    sptr<AniAppForegroundStateObserver> pObserver = GetObserverForeground(env);
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr is null");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    if (observerForeground_->IsEmpty()) {
        int ret = appMgr->RegisterAppForegroundStateObserver(pObserver);
        if (ret != NO_ERROR) {
            TAG_LOGE(AAFwkTag::APPMGR, "RegisterAppForegroundStateObserver faild");
            AbilityRuntime::ThrowStsErrorByNativeErr(env, ret);
            return;
        }
    }
    pObserver->AddAniObserverObject(observer);
}

void StsObserverManager::OnOffApplicationStateObserver(ani_env *env, ani_int observerId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOffApplicationStateObserver called");
    sptr<AniAppStateObserver> pObserver = GetAppStateObserver(env);
    if (!pObserver->FindObserverByObserverId(observerId)) {
        TAG_LOGE(AAFwkTag::APPMGR, "not find observer:%{public}d", static_cast<int32_t>(observerId));
        AbilityRuntime::ThrowStsInvalidParamError(env, "not find observerId.");
        return;
    }
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr is null");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    int32_t ret = appMgr->UnregisterApplicationStateObserver(pObserver);
    TAG_LOGD(AAFwkTag::APPMGR, "success size:%{public}zu", pObserver->GetAniObserverMapSize());
    if (ret == 0 && pObserver->RemoveAniObserverObject(observerId)) {
        TAG_LOGD(AAFwkTag::APPMGR, "success size:%{public}zu", pObserver->GetAniObserverMapSize());
    } else {
        AbilityRuntime::ThrowStsErrorByNativeErr(env, ret);
        TAG_LOGE(AAFwkTag::APPMGR, "UnregisterApplicationStateObserver err:%{public}d", ret);
    }
}

void StsObserverManager::OnOffAppForegroundStateObserver(ani_env *env, ani_object observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOffAppForegroundStateObserver called");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer is null");
        AbilityRuntime::ThrowStsInvalidParamError(env, "observer is null");
        return;
    }
    sptr<AniAppForegroundStateObserver> pObserver = GetObserverForeground(env);
    pObserver->RemoveAniObserverObject(observer);
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr is null");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    if (pObserver->IsEmpty()) {
        int32_t ret = appMgr->UnregisterAppForegroundStateObserver(pObserver);
        if (ret != NO_ERROR) {
            TAG_LOGE(AAFwkTag::APPMGR, "err: %{public}d.", ret);
            AbilityRuntime::ThrowStsErrorByNativeErr(env, ret);
            return;
        }
    }
}

void StsObserverManager::OnOffAllAppForegroundStateObserver(ani_env *env)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOffAllAppForegroundStateObserver called");
    sptr<AniAppForegroundStateObserver> pObserver = GetObserverForeground(env);
    pObserver->RemoveAllAniObserverObjects();
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr is null");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    if (pObserver->IsEmpty()) {
        int32_t ret = appMgr->UnregisterAppForegroundStateObserver(pObserver);
        if (ret != NO_ERROR) {
            TAG_LOGE(AAFwkTag::APPMGR, "err: %{public}d.", ret);
            AbilityRuntime::ThrowStsErrorByNativeErr(env, ret);
            return;
        }
    }
}

#ifdef SUPPORT_SCREEN
void StsObserverManager::OnOnAbilityFirstFrameStateObserver(ani_env *env, ani_object observerObj, ani_string bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOnAbilityFirstFrameStateObserver called");
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Non-system app");
        AbilityRuntime::ThrowStsNotSystemAppError(env);
        return;
    }
    STSAbilityFirstFrameStateObserverManager::GetInstance()->Init(env);
    if (observerObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observerObj is null");
        AbilityRuntime::ThrowStsInvalidParamError(env, "observerObj is null");
        return;
    }
    std::string bundleNameStd;
    if (!OHOS::AppExecFwk::GetStdString(env, bundleName, bundleNameStd)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid param");
        AbilityRuntime::ThrowStsInvalidParamError(env, "Invalid param");
        return;
    }
    OHOS::sptr<OHOS::AAFwk::IAbilityManager> abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "abilityManager nullptr");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    sptr<STSAbilityFirstFrameStateObserver> observer = new (std::nothrow) STSAbilityFirstFrameStateObserver(env);
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null AbilityManager_ or observer");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    if (STSAbilityFirstFrameStateObserverManager::GetInstance()->IsObserverObjectExist(observerObj)) {
        TAG_LOGE(AAFwkTag::APPMGR, "observerObj exist");
        return;
    }

    int32_t ret = abilityManager->RegisterAbilityFirstFrameStateObserver(observer, bundleNameStd);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "err: %{public}d", ret);
        AbilityRuntime::ThrowStsErrorByNativeErr(env, ret);
        return;
    }
    observer->SetStsObserverObject(observerObj);
    STSAbilityFirstFrameStateObserverManager::GetInstance()->AddSTSAbilityFirstFrameStateObserver(observer);
}

void StsObserverManager::OnOffAbilityFirstFrameStateObserver(ani_env *env, ani_object observerObj)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOffAbilityFirstFrameStateObserver called");
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Non-system app");
        AbilityRuntime::ThrowStsNotSystemAppError(env);
        return;
    }
    if (observerObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observerObj is null");
        AbilityRuntime::ThrowStsInvalidParamError(env, "observerObj is null");
        return;
    }
    STSAbilityFirstFrameStateObserverManager::GetInstance()->Init(env);
    OHOS::sptr<OHOS::AAFwk::IAbilityManager> abilityManager = GetAbilityManagerInstance();
    if (!abilityManager) {
        TAG_LOGW(AAFwkTag::APPMGR, "abilityManager nullptr");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    STSAbilityFirstFrameStateObserverManager::GetInstance()->RemoveStsObserverObject(abilityManager,
        observerObj);
}

void StsObserverManager::OnOffAllAbilityFirstFrameStateObserver(ani_env *env)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOffAllAbilityFirstFrameStateObserver called");
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Non-system app");
        AbilityRuntime::ThrowStsNotSystemAppError(env);
        return;
    }
    STSAbilityFirstFrameStateObserverManager::GetInstance()->Init(env);
    OHOS::sptr<OHOS::AAFwk::IAbilityManager> abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "abilityManager nullptr");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    STSAbilityFirstFrameStateObserverManager::GetInstance()->RemoveAllStsObserverObjects(abilityManager);
}
#endif

static ani_double OnApplicationStateObserver(ani_env *env, ani_object observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnApplicationStateObserver called");
    std::vector<std::string> bundleNameList;
    int32_t id = StsObserverManager::GetInstance()->OnOnApplicationStateObserver(env, observer, bundleNameList);
    return static_cast<ani_double>(id);
}

static ani_double OnApplicationStateObserverByArray(ani_env *env, ani_object observer, ani_object bundleNameList)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnApplicationStateObserverByArray called");
    std::vector<std::string> stdBundleNameList;
    ConvertToStdVectorString(env, bundleNameList, stdBundleNameList);
    int32_t id = StsObserverManager::GetInstance()->OnOnApplicationStateObserver(env, observer, stdBundleNameList);
    return static_cast<ani_double>(id);
}

static void OnAppForegroundStateObserver(ani_env *env, ani_object observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnAppForegroundStateObserver called");
    return StsObserverManager::GetInstance()->OnOnAppForegroundStateObserver(env, observer);
}

static void OnAbilityFirstFrameStateObserver(ani_env *env, ani_object observer, ani_string bundleName)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnAbilityFirstFrameStateObserver called");
#ifdef SUPPORT_SCREEN
    return StsObserverManager::GetInstance()->OnOnAbilityFirstFrameStateObserver(env, observer, bundleName);
#else
    TAG_LOGE(AAFwkTag::APPMGR, "not support");
#endif
}

static void OffApplicationStateObserver(ani_env *env, ani_double observerId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OffApplicationStateObserver called");
    StsObserverManager::GetInstance()->OnOffApplicationStateObserver(env, static_cast<ani_int>(observerId));
}

static void OffAppForegroundStateObserver(ani_env *env, ani_object observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OffAppForegroundStateObserver called");
    StsObserverManager::GetInstance()->OnOffAppForegroundStateObserver(env, observer);
}

static void OffAbilityFirstFrameStateObserver(ani_env *env, ani_object observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OffAbilityFirstFrameStateObserver called");
#ifdef SUPPORT_SCREEN
    return StsObserverManager::GetInstance()->OnOffAbilityFirstFrameStateObserver(env, observer);
#else
    TAG_LOGE(AAFwkTag::APPMGR, "not support");
#endif
}

static void OffAllByType(ani_env *env, ani_string type)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OffAllByType call");
    std::string stdType;
    if (!OHOS::AppExecFwk::GetStdString(env, type, stdType)) {
        TAG_LOGD(AAFwkTag::APPMGR, "get type value faild");
        return;
    }

    if (stdType == "appForegroundState") {
        return StsObserverManager::GetInstance()->OnOffAllAppForegroundStateObserver(env);
    } else if (stdType == "abilityFirstFrameState") {
#ifdef SUPPORT_SCREEN
        return StsObserverManager::GetInstance()->OnOffAllAbilityFirstFrameStateObserver(env);
#else
        TAG_LOGE(AAFwkTag::APPMGR, "not support");
#endif 
    }
}

static ani_int PreloadApplication(ani_env *env, [[maybe_unused]]ani_class aniClass,
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
        return -1;
    }
    appMgr->PreloadApplication(bundleNameStr, userId, static_cast<AppExecFwk::PreloadMode>(mode), appIndex);
    TAG_LOGI(AAFwkTag::DELEGATOR, "PreloadApplication END");
    return 0;
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
    return BoolToAniBoolean(ret);
}

static ani_int KillProcessWithAccount([[maybe_unused]] ani_env *env, ani_string bundleName, const ani_double accountId)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccount called");
    std::string bundleNamestd;
    AppExecFwk::GetStdString(env, bundleName, bundleNamestd);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccount bundleName: %{public}s,\
        accountId: %{public}lf", bundleNamestd.c_str(), accountId);
    auto appManger = GetAppManagerInstance();
    if (!appManger) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccount appmanger is null");
        return 0;
    }

    int ret = appManger->GetAmsMgr()->KillProcessWithAccount(bundleNamestd, static_cast<int64_t>(accountId));
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccount ret : %{public}d", ret);
    return ret;
}

static ani_int KillProcessWithAccountEx([[maybe_unused]] ani_env *env, ani_string bundleName,
    const ani_double accountId, ani_boolean clearPageStack, ani_object int_object)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccountEx called");
    std::string bundleNamestd;
    AppExecFwk::GetStdString(env, bundleName, bundleNamestd);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccountEx bundleName: %{public}s, accountId: %{public}lf,\
        clearPageStack: %{public}d", bundleNamestd.c_str(), accountId, clearPageStack);
    
    ani_boolean isUndefined = false;
    env->Reference_IsUndefined(int_object, &isUndefined);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccountEx isUndefined: %{public}d", isUndefined);
    if (!isUndefined) {
        ani_double result;
        if (ANI_OK !=env->Object_CallMethodByName_Double(int_object, "doubleValue", nullptr, &result)){
            TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccountEx Object_CallMethodByName_Int Fail");
        } else {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccountEx appIndex: %{public}lf", result);
        }
    } 
    
    auto appManger = GetAppManagerInstance();
    if (!appManger) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccountEx appmanger is null");
        return 0;
    }
    int ret = appManger->GetAmsMgr()->KillProcessWithAccount(bundleNamestd, static_cast<int64_t>(accountId), clearPageStack);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessWithAccountEx ret : %{public}d", ret);
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
    TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessesByBundleNameEx isUndefined: %{public}d", isUndefined);
    if (!isUndefined) {
        ani_double result;
        if (ANI_OK !=env->Object_CallMethodByName_Double(int_object, "doubleValue", nullptr, &result)){
            TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessesByBundleNameEx Object_CallMethodByName_Int Fail");
        } else {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "sts KillProcessesByBundleNameEx appIndex: %{public}lf", result);
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
    return BoolToAniBoolean(ret);
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

static ani_boolean IsSharedBundleRunning(ani_env *env, ani_string bundleName, ani_double versionCode)
{
    TAG_LOGD(AAFwkTag::APPMGR, "IsSharedBundleRunning called");
    ani_boolean ret = BoolToAniBoolean(false);
    std::string mBundleName;
    if (!OHOS::AppExecFwk::GetStdString(env, bundleName, mBundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString faild");
        AbilityRuntime::ThrowStsInvalidParamError(env, "GetStdString faild");
        return false;
    } 
    TAG_LOGD(AAFwkTag::APPMGR, "IsSharedBundleRunning bundleName: %{public}s, versionCode: %{public}f",
        mBundleName.c_str(), versionCode);
    OHOS::sptr<OHOS::AppExecFwk::IAppMgr> appManager = GetAppManagerInstance();
    if (!appManager) {
        TAG_LOGW(AAFwkTag::APPMGR, "appManager nullptr");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return ret;
    }
    ret = BoolToAniBoolean(appManager->IsSharedBundleRunning(mBundleName, versionCode));
    return ret;
}

static ani_double GetAppMemorySize(ani_env *env)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetAppMemorySize called");
    ani_double ret = -1;
    OHOS::sptr<OHOS::AAFwk::IAbilityManager> abilityManager = GetAbilityManagerInstance();
    if (!abilityManager) {
        TAG_LOGW(AAFwkTag::APPMGR, "abilityManager nullptr");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return ret;
    }
    ret = static_cast<ani_double>(abilityManager->GetAppMemorySize());
    return ret;
}

static ani_object GetRunningProcessInformation(ani_env *env)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningProcessInformation called");
    ani_object aniInfos = nullptr;
    OHOS::sptr<OHOS::AppExecFwk::IAppMgr> appManager = GetAppManagerInstance();
    if (!appManager) {
        TAG_LOGW(AAFwkTag::APPMGR, "appManager nullptr");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return aniInfos;
    }
    std::vector<AppExecFwk::RunningProcessInfo> infos;
    auto ret = appManager->GetAllRunningProcesses(infos);
    if(ret != 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "GetAllRunningProcesses error %{public}d", ret);
        AbilityRuntime::ThrowStsErrorByNativeErr(env, ret);
        return aniInfos;
    }
    if (!ConvertToAniArrayProcessInfo(env, infos, aniInfos)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ConvertToAniArrayProcessInfo faild.");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return aniInfos;
    }
    return aniInfos;
}

static ani_object GetRunningProcessInformationByBundleType(ani_env *env, ani_int enumIndex)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningProcessInformationByBundleType called");
    ani_object aniInfos = nullptr;
    if( enumIndex < 0 ) {
        TAG_LOGW(AAFwkTag::APPMGR, "enumIndex faild. enumIndex %{public}d", enumIndex);
        AbilityRuntime::ThrowStsInvalidParamError(env, "enumIndex faild.");
        return nullptr;
    }
    OHOS::sptr<OHOS::AppExecFwk::IAppMgr> appManager = GetAppManagerInstance();
    if (!appManager) {
        TAG_LOGW(AAFwkTag::APPMGR, "appManager nullptr");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }
    std::vector<AppExecFwk::RunningProcessInfo> infos;
    auto ret = appManager->GetRunningProcessesByBundleType(
        static_cast<AppExecFwk::BundleType>(enumIndex), infos);
    if(ret != 0) {
        TAG_LOGW(AAFwkTag::APPMGR, "GetRunningProcessesByBundleType faild. %{public}d", ret);
        AbilityRuntime::ThrowStsErrorByNativeErr(env, ret);
        return nullptr;
    }
    if (!ConvertToAniArrayProcessInfo(env, infos, aniInfos)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ConvertToAniArrayProcessInfo faild");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }
    return aniInfos;
}

static ani_int ClearUpAppData([[maybe_unused]] ani_env *env, ani_string bundleName, ani_object appIndex)
{
    int index = 0;
    ani_boolean isUndefined = false;

    TAG_LOGD(AAFwkTag::ABILITYMGR, "clearUpAppData  enter");
    if (!env || !bundleName) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "clearUpAppData: input parameter error!");
        return -1;
    }

    env->Reference_IsUndefined(appIndex, &isUndefined);
    if(isUndefined) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "clearUpAppData NOT Pass Value ");
    } else { 
        if (ANI_OK !=env->Object_CallMethodByName_Int(appIndex, "intValue", nullptr, &index)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "clearUpAppData Object_CallMethodByName_Int Fail");
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "clearUpAppData get int is  %{public}d", index);
    }

    std::string bundleNameStr;
    AppExecFwk::GetStdString(env, bundleName, bundleNameStr);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "clearUpAppData  bundleName:  %{public}s", bundleNameStr.c_str());
    TAG_LOGD(AAFwkTag::ABILITYMGR, "clearUpAppData  appIndex  :  %{public}d", index);

    auto appManger =  OHOS::AppManagerSts::GetAppManagerInstance();
    if (!appManger) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "  clearUpAppData appmanger is null");
        return -1;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "clearUpAppData  call ClearUpApplicationData");
    auto ret = appManger->ClearUpApplicationData(bundleNameStr, index);
    if (ret != 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "ClearUpApplicationData fail!!  return %{public}d", ret);
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "clearUpAppData  END");
    return 0;
}

static ani_int GetProcessMemoryByPid([[maybe_unused]] ani_env *env, ani_int pid)
{
    int memoryUsage = 0;
    int result = 0;

    if (pid <= 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "getProcessMemoryByPid:: Parameter error");
        return 401;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "getProcessMemoryByPid  %{public}d", pid);

    auto appManger =  OHOS::AppManagerSts::GetAppManagerInstance();
    if (!appManger) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "getProcessMemoryByPid appmanger is null");
        return -1;
    }

    result = appManger->GetProcessMemoryByPid(pid, memoryUsage);
    if (result != OHOS::ERR_OK) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "getProcessMemoryByPid::Failed to get memory usage, error code %{public}d", result);
        return false;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "getProcessMemoryByPid  memoryUsage: %{public}d", memoryUsage);
    return memoryUsage;

}

static ani_object GetRunningMultiAppInfo([[maybe_unused]] ani_env *env, ani_string bundleName)
{
    ani_object multiAppInfoObj = nullptr;

    std::string bundleNameStr;
    AppExecFwk::GetStdString(env, bundleName, bundleNameStr);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "getRunningMultiAppInfo bundleName: %{public}s", bundleNameStr.c_str());

    auto runningMultiAppInfo = std::make_shared<OHOS::AppExecFwk::RunningMultiAppInfo>();
    auto innerErrorCode = std::make_shared<int32_t>(OHOS::ERR_OK);

    auto appManger =  OHOS::AppManagerSts::GetAppManagerInstance();
    if (!appManger) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "getRunningMultiAppInfo appmanger is null");
        return nullptr;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "getRunningMultiAppInfo  call GetRunningMultiAppInfoByBundleName");
    *innerErrorCode = appManger->GetRunningMultiAppInfoByBundleName(bundleNameStr, *runningMultiAppInfo);
    if (*innerErrorCode != ANI_OK) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "GetRunningMultiAppInfoByBundleName fail, return %{public}d  %{public}s",
            *innerErrorCode,  strerror(*innerErrorCode));
        return nullptr;
    }

    if (!ConvertToAniRunningMultiAppInfo(env, *runningMultiAppInfo, multiAppInfoObj)) {
        TAG_LOGE(AAFwkTag::APPMGR, "CreateToAniRunningMultiAppInfo faild ");
        return nullptr;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "getRunningMultiAppInfo  call GetRunningMultiAppInfoByBundleName END");
    return multiAppInfoObj;
}

static ani_object GetRunningProcessInfoByBundleName([[maybe_unused]] ani_env *env, ani_string bundleName, ani_object userId)
{
    ani_object processInfo = nullptr;
    ani_int    iUserId = 0;

    if (bundleName == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "GetRunningProcessInfoByBundleName::bundleName is NULL.");
        return nullptr;//401;
    }
    std::string bundleNameStr;
    AppExecFwk::GetStdString(env, bundleName, bundleNameStr);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetRunningProcessInfoByBundleName::bundleName is %{public}s.", bundleNameStr.c_str());

    ani_boolean isUndefined;
    env->Reference_IsUndefined(userId, &isUndefined);
    if (isUndefined) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "CallMeWithOptionalDouble NOT Pass Value ");
    } else {
        if (ANI_OK !=env->Object_CallMethodByName_Int(userId, "intValue", nullptr, &iUserId)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "Object_CallMethodByName_Int Fail");
            iUserId = -1;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "Object_CallMethodByName_Int get double  %{public}d.", iUserId);
    }


    auto appMgr = OHOS::AppManagerSts::GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "getRunningProcessInfoByBundleNameUserId:: get appMgr error.");
        return nullptr;//16000050;
    }

    std::vector<OHOS::AppExecFwk::RunningProcessInfo> processInfoList;

    TAG_LOGD(AAFwkTag::ABILITYMGR, "getRunningProcessInfoByBundleNameUserId:: call GetRunningProcessInformation.");
    int result = appMgr->GetRunningProcessInformation(bundleNameStr, iUserId, processInfoList);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetRunningProcessInformation.result %{public}d", result);
    if (result != OHOS::ERR_OK) {
        switch (result) {
            case OHOS::ERR_PERMISSION_DENIED:
                TAG_LOGD(AAFwkTag::ABILITYMGR, "getRunningProcessInfoByBundleNameUserId:: non-system app called system api.");
                return nullptr;//202;

            case OHOS::ERR_INVALID_DATA:
                TAG_LOGD(AAFwkTag::ABILITYMGR, "getRunningProcessInfoByBundleNameUserId:: Invalid bundleName or userId.");
                return nullptr;//401;
            default:
                TAG_LOGD(AAFwkTag::ABILITYMGR, "getRunningProcessInfoByBundleNameUserId:: Failed to get AppManager instance.");
                return nullptr;//16000050;
        }
    }
    
    if (!ConvertToAniArrayProcessInfo(env, processInfoList, processInfo)) {
        TAG_LOGE(AAFwkTag::APPMGR, "ConvertToAniArrayProcessInfo faild");
        return NULL;
    }

    return processInfo;
}

static ani_object GetSupportedProcessCachePids([[maybe_unused]] ani_env *env, ani_string bundleName)
{
    ani_object arrayObj = nullptr;
    int32_t ret = 0;
    std::string bundleNameStr;
    AppExecFwk::GetStdString(env, bundleName, bundleNameStr);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "getSupportedProcessCachePids  bundleName: %{public}s", bundleNameStr.c_str());

    auto appManger = OHOS::AppManagerSts::GetAppManagerInstance();
    if (!appManger) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "getSupportedProcessCachePids appmanger is null");
        return nullptr;
    }

    std::vector<int32_t> list;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "getSupportedProcessCachePids  call GetSupportedProcessCachePids ");
    ret = appManger->GetSupportedProcessCachePids(bundleNameStr, list);
    if (ret != 0) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "getSupportedProcessCachePids fail, ret is %{public}d", ret);
            return nullptr;
    }

    if (!ConvertIntVectorToAniArray(env, list, arrayObj)) {
            TAG_LOGE(AAFwkTag::APPMGR, "ConvertIntVectorToAniArray faild");
            return NULL;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "getSupportedProcessCachePids end");
    return arrayObj;
}

static ani_boolean IsApplicationRunning([[maybe_unused]] ani_env *env, ani_string bundleName)
{
    std::string bundleNameStr;
    AppExecFwk::GetStdString(env, bundleName, bundleNameStr);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "isApplicationRunning  bundleName: %{public}s", bundleNameStr.c_str());

    auto appMgr = OHOS::AppManagerSts::GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "isApplicationRunning:: error");
        return false;
    }

    auto isRunning = std::make_shared<bool>(false);

    TAG_LOGD(AAFwkTag::ABILITYMGR, "isApplicationRunning call IsApplicationRunning ");
    int32_t ret = appMgr->IsApplicationRunning(bundleNameStr, *isRunning);
    if (ret != 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "IsApplicationRunning fail, ret is %{public}d", ret);
        return false;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "isApplicationRunning isRunning is %{public}d", *isRunning);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "isApplicationRunning ret is %{public}d", ret);
    return *isRunning;
}

static ani_boolean IsAppRunning([[maybe_unused]] ani_env *env, ani_string bundleName, ani_object appIndex)
{
    int index = -1;
    ani_boolean isUndefined = false;

    TAG_LOGD(AAFwkTag::ABILITYMGR, "isAppRunning:: enter");

    env->Reference_IsUndefined(appIndex, &isUndefined);
    if(isUndefined) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "isAppRunning NOT Pass Value ");
    } else { 
        if (ANI_OK !=env->Object_CallMethodByName_Int(appIndex, "intValue", nullptr, &index)) {
            TAG_LOGD(AAFwkTag::ABILITYMGR, "isAppRunning Object_CallMethodByName_Int Fail");
            index = -1;
        }
        TAG_LOGD(AAFwkTag::ABILITYMGR, "isAppRunning get int is %{public}d", index);
    }

    std::string bundleNameStr;
    AppExecFwk::GetStdString(env, bundleName, bundleNameStr);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "isAppRunning  bundleName: %{public}s", bundleNameStr.c_str());
    TAG_LOGD(AAFwkTag::ABILITYMGR, "isAppRunning  appIndex  : %{public}d", index);

    if (bundleName == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "isAppRunning::Parameter error: Invalid bundleName");
        return  false; //401;
    }

    auto isRunningtest = std::make_shared<bool>(false);
    auto appManger =  OHOS::AppManagerSts::GetAppManagerInstance();
    if (!appManger) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "isAppRunning appManger is null");
        return false;
    }

    auto isRunning = std::make_shared<bool>(false);

    TAG_LOGD(AAFwkTag::ABILITYMGR, "isAppRunning  call IsApplicationRunning ");
    int32_t result = appManger->IsAppRunning(bundleNameStr, index, *isRunning);
    if (result != 0) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "IsAppRunning fail, ret is %{public}d", result);
        return false;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "isAppRunning    result = %{public}d", result);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "isAppRunning isRunning = %{public}d", *isRunning);

    return *isRunning;
}

static ani_int TerminateMission([[maybe_unused]] ani_env *env, ani_int missionId)
{

    TAG_LOGD(AAFwkTag::ABILITYMGR, "terminateMission  appIndex = %{public}d", missionId);

    auto abilityManger = OHOS::AppManagerSts::GetAbilityManagerInstance();
    if (abilityManger == nullptr) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "terminateMission:: Failed to get AppManagerSts.");
        return -1; //16000050;
    }

    TAG_LOGD(AAFwkTag::ABILITYMGR, "terminateMission  call   TerminateMission ");
    abilityManger->TerminateMission(missionId);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "terminateMission  end ");

    return 0;
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
        ani_native_function {"nativeOn",
            onApplicationStateObserverSignature, reinterpret_cast<void *>(OnApplicationStateObserver)},
        ani_native_function {"nativeOn",
            onApplicationStateObserverByArraySignature, reinterpret_cast<void *>(OnApplicationStateObserverByArray)},
        ani_native_function {"nativeOn",
            onAppForegroundStateObserverSignature, reinterpret_cast<void *>(OnAppForegroundStateObserver)},
        ani_native_function {"nativeOn",
            onAbilityFirstFrameStateObserverSignature, reinterpret_cast<void *>(OnAbilityFirstFrameStateObserver)},
        ani_native_function {"nativeOff",
            "D:V", reinterpret_cast<void *>(OffApplicationStateObserver)},
        ani_native_function {"nativeOff",
            offAppForegroundStateObserverSignature, reinterpret_cast<void *>(OffAppForegroundStateObserver)},
        ani_native_function {"nativeOff",
            offAbilityFirstFrameStateObserverSignature, reinterpret_cast<void *>(OffAbilityFirstFrameStateObserver)},
        ani_native_function {"nativeOff",
            offAllByType, reinterpret_cast<void *>(OffAllByType)},
        ani_native_function {"nativeIsRunningInStabilityTest", ":Z",
            reinterpret_cast<void *>(IsRunningInStabilityTest)},
        ani_native_function {"nativeKillProcessWithAccount", "Lstd/core/String;I:I",
            reinterpret_cast<void *>(KillProcessWithAccount)},
        ani_native_function {"nativeKillProcessWithAccountEx", nullptr,
            reinterpret_cast<void *>(KillProcessWithAccountEx)},
        ani_native_function {"nativeKillProcessesByBundleName", "Lstd/core/String;:I",
            reinterpret_cast<void *>(KillProcessesByBundleName)},
        ani_native_function {"nativeKillProcessesByBundleNameEx", nullptr,
            reinterpret_cast<void *>(KillProcessesByBundleNameEx)},
        ani_native_function {"nativeClearUpApplicationData", "Lstd/core/String;:I",
            reinterpret_cast<void *>(ClearUpApplicationData)},
        ani_native_function {"nativeIsRamConstrainedDevice", ":Z", reinterpret_cast<void *>(IsRamConstrainedDevice)},
        ani_native_function {"nativeGetForegroundApplications", ":Lescompat/Array;",
            reinterpret_cast<void *>(GetForegroundApplications)},
        ani_native_function {"nativeGetAppMemorySize", ":D", reinterpret_cast<void *>(GetAppMemorySize)},
        ani_native_function {"nativeIsSharedBundleRunning", "Lstd/core/String;D:Z",
            reinterpret_cast<void *>(IsSharedBundleRunning)},
        ani_native_function {"nativeGetRunningProcessInformation", ":Lescompat/Array;",
            reinterpret_cast<void *>(GetRunningProcessInformation)},
        ani_native_function {"nativeGetRunningProcessInformationByBundleType", getProcessInfoByBundleTypeSignature,
            reinterpret_cast<void *>(GetRunningProcessInformationByBundleType)},
        ani_native_function {"nativeClearUpAppData", nullptr, reinterpret_cast<void *>(ClearUpAppData)},
        ani_native_function {"nativeGetProcessMemoryByPid", "I:I", reinterpret_cast<void *>(GetProcessMemoryByPid)},
        ani_native_function {"nativeIsAppRunning", nullptr,
            reinterpret_cast<void *>(IsAppRunning)},
        ani_native_function {"nativeIsApplicationRunning", "Lstd/core/String;:Z",
            reinterpret_cast<void *>(IsApplicationRunning)},
        ani_native_function {"nativePreloadApplication", nullptr, reinterpret_cast<void *>(PreloadApplication)},
        ani_native_function {"nativeGetRunningProcessInfoByBundleName", nullptr,
            reinterpret_cast<void *>(GetRunningProcessInfoByBundleName)},
        ani_native_function {"nativeGetSupportedProcessCachePids", nullptr,
            reinterpret_cast<void *>(GetSupportedProcessCachePids)},
        ani_native_function {"nativeTerminateMission", nullptr, reinterpret_cast<void *>(TerminateMission)},
        ani_native_function {"nativeGetRunningMultiAppInfo", nullptr, reinterpret_cast<void *>(GetRunningMultiAppInfo)},
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
} // namespace AbilityDelegatorSts
} // namespace OHOS

ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGI(AAFwkTag::APPMGR, "StsAbilityDelegatorRegistryInit call");
    ani_env *env;
    if (ANI_OK != vm->GetEnv(ANI_VERSION_1, &env)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Unsupported ANI_VERSION_1");
        std::cout << "Unsupported ANI_VERSION_1" << std::endl;
        return ANI_INVALID_VERSION;
    }

    ani_status status = ANI_OK;
    OHOS::AppManagerSts::StsAppManagerRegistryInit(env);
    TAG_LOGI(AAFwkTag::APPMGR, "StsAbilityDelegatorRegistryInit call end");
    std::cout << "StsAbilityDelegatorRegistryInit call end" << status << std::endl;
    *result = ANI_VERSION_1;
    return status;
}