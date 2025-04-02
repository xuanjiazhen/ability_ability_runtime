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
    sptr<AniAppStateObserver> pObserver = GetAppStateObserver(env);
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr is null");
        return -1;
    }
    int32_t ret = appMgr->RegisterApplicationStateObserver(pObserver, bundleNameList);
    if (ret != 0) {
        TAG_LOGD(AAFwkTag::APPMGR, "RegisterApplicationStateObserver faild. ret %{public}d", ret);
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
        return;
    }
    sptr<AniAppForegroundStateObserver> pObserver = GetObserverForeground(env);
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr is null");
        return;
    }
    if (observerForeground_->IsEmpty()) {
        int ret = appMgr->RegisterAppForegroundStateObserver(pObserver);
        if (ret != NO_ERROR) {
            TAG_LOGE(AAFwkTag::APPMGR, "RegisterAppForegroundStateObserver faild");
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
        return;
    }
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr is null");
        return;
    }
    int32_t ret = appMgr->UnregisterApplicationStateObserver(pObserver);
    if (ret == 0 && pObserver->RemoveAniObserverObject(observerId)) {
        TAG_LOGD(AAFwkTag::APPMGR, "success size:%{public}zu", pObserver->GetAniObserverMapSize());
    } else {
        TAG_LOGE(AAFwkTag::APPMGR, "UnregisterApplicationStateObserver err:%{public}d", ret);
    }
}

void StsObserverManager::OnOffAppForegroundStateObserver(ani_env *env, ani_object observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOffAppForegroundStateObserver called");
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observer is null");
        return;
    }
    sptr<AniAppForegroundStateObserver> pObserver = GetObserverForeground(env);
    pObserver->RemoveAniObserverObject(observer);
    sptr<OHOS::AppExecFwk::IAppMgr> appMgr = GetAppManagerInstance();
    if (appMgr == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "appMgr is null");
        return;
    }
    if (pObserver->IsEmpty()) {
        int32_t ret = appMgr->UnregisterAppForegroundStateObserver(pObserver);
        if (ret != NO_ERROR) {
            TAG_LOGE(AAFwkTag::APPMGR, "err: %{public}d.", ret);
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
        return;
    }
    if (pObserver->IsEmpty()) {
        int32_t ret = appMgr->UnregisterAppForegroundStateObserver(pObserver);
        if (ret != NO_ERROR) {
            TAG_LOGE(AAFwkTag::APPMGR, "err: %{public}d.", ret);
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
        return;
    }
    STSAbilityFirstFrameStateObserverManager::GetInstance()->Init(env);
    if (observerObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observerObj is null");
        return;
    }
    std::string bundleNameStd;
    if (!OHOS::AppExecFwk::GetStdString(env, bundleName, bundleNameStd)) {
        TAG_LOGE(AAFwkTag::APPMGR, "Invalid param");
        return;
    }
    OHOS::sptr<OHOS::AAFwk::IAbilityManager> abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "abilityManager nullptr");
        return;
    }

    sptr<STSAbilityFirstFrameStateObserver> observer = new (std::nothrow) STSAbilityFirstFrameStateObserver(env);
    if (observer == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "null AbilityManager_ or observer");
        return;
    }

    if (STSAbilityFirstFrameStateObserverManager::GetInstance()->IsObserverObjectExist(observerObj)) {
        TAG_LOGE(AAFwkTag::APPMGR, "observerObj exist");
        return;
    }

    int32_t ret = abilityManager->RegisterAbilityFirstFrameStateObserver(observer, bundleNameStd);
    if (ret != NO_ERROR) {
        TAG_LOGE(AAFwkTag::APPMGR, "err: %{public}d", ret);
        return;
    }
    observer->SetStsObserverObject(observerObj);
    STSAbilityFirstFrameStateObserverManager::GetInstance()->AddSTSAbilityFirstFrameStateObserver(observer);
}

void StsObserverManager::OnOffAbilityFirstFrameStateObserver(ani_env *env, ani_object observerObj)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnOffAbilityFirstFrameStateObserver called");
#ifdef SUPPORT_SCREEN
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPMGR, "Non-system app");
        return;
    }
#endif
    STSAbilityFirstFrameStateObserverManager::GetInstance()->Init(env);
    if (observerObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "observerObj is null");
        return;
    }
    OHOS::sptr<OHOS::AAFwk::IAbilityManager> abilityManager = GetAbilityManagerInstance();
    if (!abilityManager) {
        TAG_LOGW(AAFwkTag::APPMGR, "abilityManager nullptr");
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
        return;
    }
    STSAbilityFirstFrameStateObserverManager::GetInstance()->Init(env);
    OHOS::sptr<OHOS::AAFwk::IAbilityManager> abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGW(AAFwkTag::APPMGR, "abilityManager nullptr");
        return;
    }
    STSAbilityFirstFrameStateObserverManager::GetInstance()->RemoveAllStsObserverObjects(abilityManager);
}
#endif

static ani_int OnApplicationStateObserver(ani_env *env, ani_object observer)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnApplicationStateObserver called");
    std::vector<std::string> bundleNameList;
    return StsObserverManager::GetInstance()->OnOnApplicationStateObserver(env, observer, bundleNameList);
}

static ani_int OnApplicationStateObserverByArray(ani_env *env, ani_object observer, ani_object bundleNameList)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OnApplicationStateObserverByArray called");
    std::vector<std::string> stdBundleNameList;
    ConvertToStdVectorString(env, bundleNameList, stdBundleNameList);
    return StsObserverManager::GetInstance()->OnOnApplicationStateObserver(env, observer, stdBundleNameList);
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

static void OffApplicationStateObserver(ani_env *env, ani_int observerId)
{
    TAG_LOGD(AAFwkTag::APPMGR, "OffApplicationStateObserver called");
    StsObserverManager::GetInstance()->OnOffApplicationStateObserver(env, observerId);
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

static ani_boolean IsSharedBundleRunning(ani_env *env, ani_string bundleName, ani_double versionCode)
{
    TAG_LOGD(AAFwkTag::APPMGR, "IsSharedBundleRunning called");
    ani_boolean ret = BoolToAniBoolean(false);
    std::string mBundleName;
    if (!OHOS::AppExecFwk::GetStdString(env, bundleName, mBundleName)) {
        TAG_LOGE(AAFwkTag::APPMGR, "GetStdString faild");
        return false;
    } 
    TAG_LOGD(AAFwkTag::APPMGR, "IsSharedBundleRunning bundleName: %{public}s, versionCode: %{public}f",
        mBundleName.c_str(), versionCode);
    OHOS::sptr<OHOS::AppExecFwk::IAppMgr> appManager = GetAppManagerInstance();
    if (!appManager) {
        TAG_LOGW(AAFwkTag::APPMGR, "appManager nullptr");
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
        return ret;
    }
    ret = static_cast<ani_double>(abilityManager->GetAppMemorySize());
    return ret;
}

static ani_object GetRunningProcessInformation(ani_env *env)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningProcessInformation called");
    ani_object aniInfos = nullptr;
    do {
        OHOS::sptr<OHOS::AppExecFwk::IAppMgr> appManager = GetAppManagerInstance();
        if (!appManager) {
            TAG_LOGW(AAFwkTag::APPMGR, "appManager nullptr");
            break;
        }

        std::vector<AppExecFwk::RunningProcessInfo> infos;
        auto ret = appManager->GetAllRunningProcesses(infos);
        if(ret != 0) {
            TAG_LOGW(AAFwkTag::APPMGR, "GetAllRunningProcesses error %{public}d", ret);
            break;
        }
        
        if (!ConvertToAniArrayProcessInfo(env, infos, aniInfos)) {
            TAG_LOGE(AAFwkTag::APPMGR, "ConvertToAniArrayProcessInfo faild %{public}d", ret);
            break;
        }
    } while(0);
    return aniInfos;
}

static ani_object GetRunningProcessInformationByBundleType(ani_env *env, ani_int enumIndex)
{
    TAG_LOGD(AAFwkTag::APPMGR, "GetRunningProcessInformationByBundleType called");
    ani_object aniInfos = nullptr;
    do {
        if( enumIndex < 0 ) {
            TAG_LOGW(AAFwkTag::APPMGR, "enumIndex faild. enumIndex %{public}d", enumIndex);
            break;
        }
        OHOS::sptr<OHOS::AppExecFwk::IAppMgr> appManager = GetAppManagerInstance();
        if (!appManager) {
            TAG_LOGW(AAFwkTag::APPMGR, "appManager nullptr");
            break;
        }
        std::vector<AppExecFwk::RunningProcessInfo> infos;
        auto ret = appManager->GetRunningProcessesByBundleType(
            static_cast<AppExecFwk::BundleType>(enumIndex), infos);
        if(ret != 0) {
            TAG_LOGW(AAFwkTag::APPMGR, "GetRunningProcessesByBundleType faild. %{public}d", ret);
            break;
        }
        if (!ConvertToAniArrayProcessInfo(env, infos, aniInfos)) {
            TAG_LOGE(AAFwkTag::APPMGR, "ConvertToAniArrayProcessInfo faild");
            break;
        }
    } while(0);
    return aniInfos;
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
            "I:V", reinterpret_cast<void *>(OffApplicationStateObserver)},
        ani_native_function {"nativeOff",
            offAppForegroundStateObserverSignature, reinterpret_cast<void *>(OffAppForegroundStateObserver)},
        ani_native_function {"nativeOff",
            offAbilityFirstFrameStateObserverSignature, reinterpret_cast<void *>(OffAbilityFirstFrameStateObserver)},
        ani_native_function {"nativeOff",
            offAllByType, reinterpret_cast<void *>(OffAllByType)},
        ani_native_function {"nativePreloadApplication", nullptr, reinterpret_cast<void *>(PreloadApplication)},
        ani_native_function {"nativeIsRunningInStabilityTest", ":Z", reinterpret_cast<void *>(IsRunningInStabilityTest)},
        ani_native_function {"nativeKillProcessWithAccount", "Lstd/core/String;I:I", reinterpret_cast<void *>(KillProcessWithAccount)},
        ani_native_function {"nativeKillProcessWithAccountEx", nullptr, reinterpret_cast<void *>(KillProcessWithAccountEx)},
        ani_native_function {"nativeKillProcessesByBundleName", "Lstd/core/String;:I", reinterpret_cast<void *>(KillProcessesByBundleName)},
        ani_native_function {"nativeKillProcessesByBundleNameEx", nullptr, reinterpret_cast<void *>(KillProcessesByBundleNameEx)},
        ani_native_function {"nativeClearUpApplicationData", "Lstd/core/String;:I", reinterpret_cast<void *>(ClearUpApplicationData)},
        ani_native_function {"nativeIsRamConstrainedDevice", ":Z", reinterpret_cast<void *>(IsRamConstrainedDevice)},
        ani_native_function {"nativeGetForegroundApplications", ":Lescompat/Array;", reinterpret_cast<void *>(GetForegroundApplications)},
        ani_native_function {"nativeGetAppMemorySize", ":D", reinterpret_cast<void *>(GetAppMemorySize)},
        ani_native_function {"nativeIsSharedBundleRunning", "Lstd/core/String;D:Z", reinterpret_cast<void *>(IsSharedBundleRunning)},
        ani_native_function {"nativeGetRunningProcessInformation",
            ":Lescompat/Array;", reinterpret_cast<void *>(GetRunningProcessInformation)},
        ani_native_function {"nativeGetRunningProcessInformationByBundleType",
            nullptr, reinterpret_cast<void *>(GetRunningProcessInformationByBundleType)},
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