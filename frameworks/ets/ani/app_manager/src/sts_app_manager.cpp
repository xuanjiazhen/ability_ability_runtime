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
		/*
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
		*/
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

		static ani_int ClearUpAppData([[maybe_unused]] ani_env *env, ani_string bundleName, ani_object appIndex)
		{
			int index = 0;
		    ani_boolean isUndefined = false;

		    std::cout << "clearUpAppData  enter"<< std::endl;
		    if (!env || !bundleName) {
		        std::cerr << "clearUpAppData: 非法输入参数" << std::endl;
		        return -1;
		    }

		    std::cout << "clearUpAppData  222"<< std::endl;

		    env->Reference_IsUndefined(appIndex, &isUndefined);
		    if(isUndefined) {
		        std::cout << "clearUpAppData NOT Pass Value " << std::endl; 
		    } else { 
			    if (ANI_OK !=env->Object_CallMethodByName_Int(appIndex, "intValue", nullptr, &index)){
			        std::cerr << "clearUpAppData Object_CallMethodByName_Int Fail" << std::endl;
			    }
			    std::cout << "clearUpAppData get int is " << index << std::endl;
			}

		    std::string bundleNameStr = OHOS::AppManagerSts::AniStringToStdString(env, bundleName);
		    std::cout << "clearUpAppData  bundleName: " << bundleNameStr << std::endl;
		    std::cout << "clearUpAppData  appIndex  : " << index << std::endl;

		    auto appManger =  OHOS::AppManagerSts::GetAppManagerInstance();
		    if (!appManger) {
		        std::cout << "  clearUpAppData appmanger is null" << std::endl;
		        return -1;
		    }

		    std::cout << "clearUpAppData  call ClearUpApplicationData  : "<< std::endl;
			auto ret = appManger->ClearUpApplicationData(bundleNameStr, index);
		    if (ret != 0) {
			    std::cout << "ClearUpApplicationData fail!!  return " << ret << std::endl;
		    }

			std::cout << "clearUpAppData   END " << std::endl;

			return 0;
		}

		static ani_int GetProcessMemoryByPid([[maybe_unused]] ani_env *env, ani_int pid)
		{
		    int memoryUsage = 0;
		    int result = 0;

		    std::cout << " ======== getProcessMemoryByPid  pid: " << pid << std::endl;

		    if (pid <= 0) {
		        std::cout << "getProcessMemoryByPid:: Parameter error" << std::endl;
		        return 401;
		    }

		    auto appManger =  OHOS::AppManagerSts::GetAppManagerInstance();
		    if (!appManger) {
		        std::cout << "	getProcessMemoryByPid appmanger is null" << std::endl;
		        return -1;
		    }

		    result = appManger->GetProcessMemoryByPid(pid, memoryUsage);
		    if (result != OHOS::ERR_OK) {
		        std::cout << "getProcessMemoryByPid::Failed to get memory usage, error code: " << result << std::endl;
		        return false;
		    }

		    std::cout << "getProcessMemoryByPid  memoryUsage: " << memoryUsage << "KB" << std::endl;
		    return memoryUsage;

		}

		static ani_object GetRunningMultiAppInfo([[maybe_unused]] ani_env *env, ani_string bundleName)
		{
		    ani_object person_obj = {};
			
		    std::string bundleNameStr = OHOS::AppManagerSts::AniStringToStdString(env, bundleName);
		    std::cout << " ======== getRunningMultiAppInfo  bundleName: " << bundleNameStr << std::endl;

			auto info = std::make_shared<OHOS::AppExecFwk::RunningMultiAppInfo>();
			auto innerErrorCode = std::make_shared<int32_t>(OHOS::ERR_OK);

			auto appManger =  OHOS::AppManagerSts::GetAppManagerInstance();
			if (!appManger) {
				std::cout << "	getRunningMultiAppInfo appmanger is null" << std::endl;
				return person_obj;
			}

			std::cout << "getRunningMultiAppInfo  call GetRunningMultiAppInfoByBundleName "<< std::endl;
			*innerErrorCode = appManger->GetRunningMultiAppInfoByBundleName(bundleNameStr, *info);
			if (*innerErrorCode != ANI_OK) {
				std::cout << "getRunningMultiAppInfo call GetRunningMultiAppInfoByBundleName fail, innerErrorCode is "<< *innerErrorCode << std::endl;
				return person_obj;
			} 
			std::cout << "getRunningMultiAppInfo  call GetRunningMultiAppInfoByBundleName END" << std::endl;

			//TODO::  组装 ani_object
			
		    return person_obj;
		}

		static ani_object GetRunningProcessInfoByBundleName([[maybe_unused]] ani_env *env, ani_string bundleName, ani_object userId)
		{
		    ani_object personObj = {};
			ani_int    iUserId = 0;

		 	if (bundleName == nullptr) {
				std::cout << "getRunningProcessInfoByBundleNameUserId::Parameter error: Invalid bundleName or userId." << std::endl;
				return personObj;//401;
			} 
		    std::string bundle_name = OHOS::AppManagerSts::AniStringToStdString(env, bundleName);
		    std::cout << " ======== getRunningProcessInfoByBundleNameUserId  bundleName: " << bundle_name << std::endl;

		    ani_boolean isUndefined;
		    env->Reference_IsUndefined(userId, &isUndefined);
		    if (isUndefined) {
		        std::cout << "CallMeWithOptionalDouble NOT Pass Value " << std::endl; 
		    } else {
			    if (ANI_OK !=env->Object_CallMethodByName_Int(userId, "intValue", nullptr, &iUserId)) {
			        std::cerr << "Object_CallMethodByName_Int Fail" << std::endl;
			        iUserId = -1;
			    }
			    std::cout << "Object_CallMethodByName_Int get double " << iUserId << std::endl;
			}


			auto appMgr = OHOS::AppManagerSts::GetAppManagerInstance();
			if (appMgr == nullptr) {
		        std::cout << "getRunningProcessInfoByBundleNameUserId::Internal error: Failed to get AppManager instance." << std::endl;
		        return personObj;//16000050;
			}

		    std::vector<OHOS::AppExecFwk::RunningProcessInfo> processInfoList;
			
			std::cout << "getRunningProcessInfoByBundleNameUserId:: call GetRunningProcessInformation." << std::endl;
			int result = appMgr->GetRunningProcessInformation(bundle_name, iUserId, processInfoList);
			std::cout << "getRunningProcessInfoByBundleNameUserId:: call GetRunningProcessInformation. end result"<< result << std::endl;
			if (result != OHOS::ERR_OK) {
				switch (result) {
					case OHOS::ERR_PERMISSION_DENIED:
				        std::cout << "getRunningProcessInfoByBundleNameUserId::Permission denied, non-system app called system api." << std::endl;
				        return personObj;//202;
		        
					case OHOS::ERR_INVALID_DATA:
				        std::cout << "getRunningProcessInfoByBundleNameUserId::Parameter error: Invalid bundleName or userId." << std::endl;
				        return personObj;//401;
					default:
				        std::cout << "getRunningProcessInfoByBundleNameUserId::Internal error: Failed to get AppManager instance." << std::endl;
				        return personObj;//16000050;
				}
			}
			
			//TODO::  组装 ani_object
			return personObj;
		}

		static ani_object GetSupportedProcessCachePids([[maybe_unused]] ani_env *env, ani_string bundleName)
		{
		    ani_object arrayObj = nullptr;
			int32_t ret = 0;
		    std::string bundleNameStr = OHOS::AppManagerSts::AniStringToStdString(env, bundleName);
		    std::cout << "getSupportedProcessCachePids  bundleName: " << bundleNameStr << std::endl;

			auto appManger = OHOS::AppManagerSts::GetAppManagerInstance();	
			std::cout << "getSupportedProcessCachePids  appMgr	: " << appManger << std::endl; 
			if (!appManger) {
				std::cout << "	getSupportedProcessCachePids appmanger is null" << std::endl;
				return nullptr;
			}

			std::vector<int32_t> list;
			std::cout << "getSupportedProcessCachePids  call GetSupportedProcessCachePids  : "<< std::endl;
			ret = appManger->GetSupportedProcessCachePids(bundleNameStr, list);
		    if (ret != 0) {
			     std::cout << "getSupportedProcessCachePids fail, ret is " << ret << std::endl;
			     return nullptr;
			}
			std::cout << "getSupportedProcessCachePids   END " << std::endl;
			//TODO::  组装 ani_object
			return arrayObj;
		}

		static ani_boolean IsApplicationRunning([[maybe_unused]] ani_env *env, ani_string bundleName)
		{
		    std::string bundleNameStr = OHOS::AppManagerSts::AniStringToStdString(env, bundleName);
		    std::cout << "isApplicationRunning  bundleName: " << bundleNameStr << std::endl;

			auto appMgr = OHOS::AppManagerSts::GetAppManagerInstance();
			if (appMgr == nullptr) {
			    std::cout << "isApplicationRunning:: error" << std::endl;
			    return false;
			}

			auto innerErrorCode = std::make_shared<int32_t>(ERR_OK);
			auto isRunning = std::make_shared<bool>(false);

		    std::cerr << "isApplicationRunning call IsApplicationRunning " << std::endl;

		    *innerErrorCode = appMgr->IsApplicationRunning(bundleNameStr, *isRunning);

		    std::cerr << "isApplicationRunning isRunning "<< *isRunning << std::endl;
		    std::cerr << "isApplicationRunning innerErrorCode "<< *innerErrorCode << std::endl;

			return *isRunning;
		}

		static ani_boolean IsAppRunning([[maybe_unused]] ani_env *env, ani_string bundleName, ani_object appIndex)
		{
			int index = -1;
		    ani_boolean isUndefined = false;
			
			std::cout << "isAppRunning:: enter" << std::endl;

		    env->Reference_IsUndefined(appIndex, &isUndefined);
		    if(isUndefined) {
		        std::cout << "isAppRunning NOT Pass Value " << std::endl; 
		    } else { 
			    if (ANI_OK !=env->Object_CallMethodByName_Int(appIndex, "intValue", nullptr, &index)){
			        std::cerr << "isAppRunning Object_CallMethodByName_Int Fail" << std::endl;
			        index = -1;
			    }
			    std::cout << "isAppRunning get int is " << index << std::endl;
			}

		    std::string bundleNameStr = OHOS::AppManagerSts::AniStringToStdString(env, bundleName);
		    std::cout << "isAppRunning  bundleName: " << bundleNameStr << std::endl;
		    std::cout << "isAppRunning  appIndex  : " << index  << std::endl;

			if (bundleName == nullptr) {
		        std::cout << "isAppRunning::Parameter error: Invalid bundleName" << std::endl;
		        return  false; //401;
			}

			auto isRunningtest = std::make_shared<bool>(false);

			auto appManger =  OHOS::AppManagerSts::GetAppManagerInstance();
			std::cout << "isAppRunning  appMgr	: " << appManger << std::endl; 
			if (!appManger) {
				std::cout << "	isAppRunning appmanger is null" << std::endl;
				return false;
			}

			auto isRunning = std::make_shared<bool>(false);

			std::cout << "isAppRunning  call IsApplicationRunning  : "<< std::endl;
			int32_t result = appManger->IsAppRunning(bundleNameStr, index, *isRunning);

			std::cerr << "isAppRunning  result = " << result << std::endl;
			std::cerr << "isAppRunning  isRunning = " << *isRunning << std::endl;

			return *isRunning;
		}

		static ani_int PreloadApplication([[maybe_unused]] ani_env *env, ani_string bundleName, ani_int userId, ani_int mode, ani_object appIndex)
		{
			int index = 0;

		    std::string bundle_name = OHOS::AppManagerSts::AniStringToStdString(env, bundleName);
		    std::cout << " ======== preloadApplication  bundleName: " << bundle_name << std::endl;
		    std::cout << " ======== preloadApplication  userId    : " << userId << std::endl;
		    std::cout << " ======== preloadApplication  mode      : " << mode << std::endl;

		    ani_boolean isUndefined = false;
		    env->Reference_IsUndefined(appIndex, &isUndefined);
		    if(isUndefined) {
		        std::cout << "preloadApplication NOT Pass Value " << std::endl; 
		    } else { 
			    if (ANI_OK !=env->Object_CallMethodByName_Int(appIndex, "intValue", nullptr, &index)){
			        std::cerr << "preloadApplication Object_CallMethodByName_Int Fail" << std::endl;
			        index = 0;
			    }
			    std::cout << "preloadApplication get int is " << index << std::endl;
			}

		    std::cout << " ======== preloadApplication  appIndex  : " << index << std::endl;

		    AppExecFwk::PreloadMode preloadMode = static_cast<AppExecFwk::PreloadMode>(mode);

			auto appMgr = OHOS::AppManagerSts::GetAppManagerInstance();
			if (appMgr == nullptr) {
		        std::cout << "preloadApplication::Internal error: Failed to get AppManager instance." << std::endl;
		        return -1;
			}

			auto innerErrorCode = std::make_shared<int32_t>(OHOS::ERR_OK);
			try {
				std::cout << "preloadApplication:: call PreloadApplication." << std::endl;
				*innerErrorCode = appMgr->PreloadApplication(bundle_name, userId, preloadMode, index);
			    std::cout << "preloadApplication:: end *innerErrorCode is "<< *innerErrorCode << std::endl;
			} catch (const std::exception& e) {	
				std::cout << "preloadApplication::Exception: " << e.what() << std::endl;
				return -1;
			} catch (...) {
				std::cout << "preloadApplication::Unknown exception occurred." << std::endl;
				return -1;
			}
		    return 0;
		}

		static ani_int TerminateMission([[maybe_unused]] ani_env *env, ani_int missionId)
		{
		    std::cout << "terminateMission  appIndex  : " << missionId << std::endl;

		    auto abilityManger = OHOS::AppManagerSts::GetAbilityManagerInstance();
			if (abilityManger == nullptr) {
		        std::cout << "terminateMission::Internal error: Failed to get abilityManger instance." << std::endl;
		        return -1; //16000050;
			}

		    std::cout << "terminateMission  call   TerminateMission " << std::endl;

			abilityManger->TerminateMission(missionId);

		    std::cout << "terminateMission  end " << std::endl;

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
            "I:V", reinterpret_cast<void *>(OffApplicationStateObserver)},
        ani_native_function {"nativeOff",
            offAppForegroundStateObserverSignature, reinterpret_cast<void *>(OffAppForegroundStateObserver)},
        ani_native_function {"nativeOff",
            offAbilityFirstFrameStateObserverSignature, reinterpret_cast<void *>(OffAbilityFirstFrameStateObserver)},
        ani_native_function {"nativeOff",
            offAllByType, reinterpret_cast<void *>(OffAllByType)},
        ani_native_function {"nativePreloadApplication", nullptr, reinterpret_cast<void *>(PreloadApplication)},
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
        ani_native_function {"nativeGetRunningProcessInformationByBundleType", nullptr,
            reinterpret_cast<void *>(GetRunningProcessInformationByBundleType)},
        ani_native_function {"nativeClearUpAppData", "Lstd/core/String;Lstd/core/Object;:I", reinterpret_cast<void *>(ClearUpAppData)},
        ani_native_function {"nativeGetProcessMemoryByPid", "I:I", reinterpret_cast<void *>(GetProcessMemoryByPid)},

        ani_native_function {"nativeIsAppRunning", nullptr, reinterpret_cast<void *>(IsAppRunning)},
        ani_native_function {"nativeIsApplicationRunning", nullptr, reinterpret_cast<void *>(IsApplicationRunning)},
        ani_native_function {"nativePreloadApplication", nullptr, reinterpret_cast<void *>(PreloadApplication)},
        ani_native_function {"nativeGetRunningProcessInfoByBundleName", nullptr, reinterpret_cast<void *>(GetRunningProcessInfoByBundleName)},
        ani_native_function {"nativeGetSupportedProcessCachePids", nullptr, reinterpret_cast<void *>(GetSupportedProcessCachePids)},
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