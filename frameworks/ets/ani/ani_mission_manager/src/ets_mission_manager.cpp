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

#include "ets_mission_manager.h"

#include "ability_manager_client.h"
#include "ani.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ets_error_utils.h"
#include "ets_mission_info_utils.h"
#include "hilog_tag_wrapper.h"
#include "permission_constants.h"

namespace OHOS {
namespace AbilityRuntime {
using namespace OHOS::AppExecFwk;
using AbilityManagerClient = AAFwk::AbilityManagerClient;
namespace {
constexpr const char* ETS_MISSION_INFO_NAME = "Lapplication/MissionInfo/MissionInfoInner;";
constexpr const char* ETS_MISSION_MANAGER_NAME = "L@ohos/app/ability/missionManager/missionManager;";
constexpr const char* ETS_MISSION_LISTENER_NAME = "Lapplication/MissionListener/MissionListenerInner;";
}
class EtsMissionManager {
public:
    EtsMissionManager(const EtsMissionManager&) = delete;
    EtsMissionManager& operator=(const EtsMissionManager&) = delete;

    static EtsMissionManager& GetInstance()
    {
        return instance;
    }

    static void ClearAllMissions(ani_env* env, ani_object callback)
    {
        instance.OnClearAllMissions(env, callback);
    }

    static void GetMissionInfo(ani_env* env, ani_string deviceId, ani_int missionId, ani_object callback)
    {
        instance.OnGetMissionInfo(env, deviceId, missionId, callback);
    }

     static void GetMissionInfos(ani_env* env, ani_string deviceId, ani_int numMax, ani_object callback)
    {
        instance.OnGetMissionInfos(env, deviceId, numMax, callback);
    }

    static void ClearMission(ani_env* env, ani_int missionId, ani_object callback)
    {
        instance.OnClearMission(env, missionId, callback);
    }

    static void LockMission(ani_env* env, ani_int missionId, ani_object callback)
    {
        instance.OnLockMission(env, missionId, callback);
    }

    static void UnlockMission(ani_env* env, ani_int missionId, ani_object callback)
    {
        instance.OnUnlockMission(env, missionId, callback);
    }

    static void GetMissionSnapShot(ani_env* env, ani_string deviceId, ani_int missionId, ani_object callback)
    {
        instance.OnGetMissionSnapShot(env, deviceId, missionId, callback, false);
    }

    static void GetLowResolutionMissionSnapShot(ani_env* env, ani_string deviceId, ani_int missionId, ani_object callback)
    {
        instance.OnGetMissionSnapShot(env, deviceId, missionId, callback, true);
    }
/*    
    
    static void GetMissionSnapShot(ani_env* env, ani_string deviceId, ani_int missionId, ani_object callback)
    {
        instance.OnGetMissionSnapShot(env, deviceId, missionId, callback);
    }

    static void MoveMissionsToBackground(ani_env* env, ani_int missionIds, ani_object callback)    //注意：ani_int missionIds，这个的类型？
    {
        instance.OnMoveMissionsToBackground(env, missionIds, callback);
    }

    static void MoveMissionsToForeground(ani_env* env, ani_int missionIds, ani_int topMission, ani_object callback)    //ani_int missionIds，这个的类型？
    {
        instance.OnMoveMissionsToForeground(env, missionIds, topMission, callback);
    }

    static void MoveMissionToFront(ani_env* env, ani_int missionId, ani_object options, ani_object callback)   //注意：ani_object options，这个的类型？
    {
        instance.OnMoveMissionToFront(env, missionId, missionId, callback);
    }

    static void On(ani_env* env, ani_object type, ani_object listener)   //注意：ani_object type，ani_object listener,这两个的类型？
    {
        instance.OnOn(env, type, listener);
    }

    static void Off(ani_env* env, ani_object type, ani_int listenerId, ani_object callback)   //注意：ani_object type，这个的类型？
    {
        instance.OnOff(env, type, listenerId, callback);
    }
*/

private:
    EtsMissionManager() = default;
    ~EtsMissionManager() = default;

    static EtsMissionManager instance;

    void OnClearAllMissions(ani_env* env, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::MISSION, "OnClearAllMissions Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "null env");
            return;
        }
        auto ret = AbilityManagerClient::GetInstance()->CleanAllMissions();
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::MISSION, "OnClearAllMissions is failed. ret = %{public}d.", ret);
            AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env,
                ret, PermissionConstants::PERMISSION_MANAGE_MISSION), nullptr);
            return;
        }
        AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
    }

    void OnGetMissionInfo(ani_env* env, ani_string deviceId, ani_int missionId, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::MISSION, "OnGetMissionInfo Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "null env");
            return;
        }
        auto emptyObject = GetEmptyMissionInfo(env);
        std::string stdDeviceId = "";
        if (!GetStdString(env, deviceId, stdDeviceId)) {
            TAG_LOGE(AAFwkTag::MISSION, "GetStdString failed");
            AsyncCallback(env, callback, EtsErrorUtil::CreateInvalidParamError(env,
                "Parse param deviceId failed, must be a string."), emptyObject);
            return;
        }
        AAFwk::MissionInfo missionInfo;
        auto ret = AbilityManagerClient::GetInstance()->GetMissionInfo(stdDeviceId,
            missionId, missionInfo);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::MISSION, "GetMissionInfo is failed. ret = %{public}d.", ret);
            AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env,
                ret, PermissionConstants::PERMISSION_MANAGE_MISSION), emptyObject);
            return;
        }
        auto aniMissionInfo = CreateEtsMissionInfo(env, missionInfo);
        AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), aniMissionInfo);
    }

    ani_object GetEmptyMissionInfo(ani_env* env)
    {
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "null env");
            return nullptr;
        }
        ani_class cls = nullptr;
        ani_status status = env->FindClass(ETS_MISSION_INFO_NAME, &cls);
        if (status != ANI_OK || cls == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "find Context failed status: %{public}d", status);
            return nullptr;
        }
        ani_method method = nullptr;
        status = env->Class_FindMethod(cls, "<ctor>", ":V", &method);
        if (status != ANI_OK || method == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "Class_FindMethod ctor failed status: %{public}d", status);
            return nullptr;
        }
        ani_object objValue = nullptr;
        status = env->Object_New(cls, method, &objValue);
        if (status != ANI_OK || objValue == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "Object_New failed status: %{public}d", status);
            return nullptr;
        }
        return objValue;
    }

    void OnGetMissionInfos(ani_env* env, ani_string deviceId, ani_int numMax, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::MISSION, "OnGetMissionInfos Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "null env");
            return;
        }
        auto emptyObject = GetEmptyMissionInfo(env);
        std::string stdDeviceId = "";
        if (!GetStdString(env, deviceId, stdDeviceId) || stdDeviceId.empty()) {
            TAG_LOGE(AAFwkTag::MISSION, "GetStdString failed");
            AsyncCallback(env, callback, EtsErrorUtil::CreateInvalidParamError(env,
                "Parse param deviceId failed, must be a string."), emptyObject);
            return;
        }
        std::vector<AAFwk::MissionInfo> missionInfos;
        auto ret = AbilityManagerClient::GetInstance()->GetMissionInfos(stdDeviceId,
            numMax, missionInfos);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::MISSION, "GetMissionInfos is failed. ret = %{public}d.", ret);
            AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env,
                ret, PermissionConstants::PERMISSION_MANAGE_MISSION), emptyObject);
            return;
        }
        auto aniMissionInfos = CreateEtsMissionInfos(env, missionInfos);     //这个函数在missionInfo文件里面写的，注意看看
        AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), aniMissionInfos);
    }

    void OnClearMission(ani_env* env, ani_int missionId, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::MISSION, "OnClearMission Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "null env");
            return;
        }

        auto ret = AbilityManagerClient::GetInstance()->CleanMission(missionId);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::MISSION, "OnClearMission is failed. ret = %{public}d.", ret);
            AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env,
                ret, PermissionConstants::PERMISSION_MANAGE_MISSION), nullptr);
            return;
        }

        AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
    }

    
    void OnLockMission(ani_env* env, ani_int missionId, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::MISSION, "OnLockMission Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "null env");
            return;
        }

        auto ret = AbilityManagerClient::GetInstance()->LockMissionForCleanup(missionId); 
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::MISSION, "OnLockMission is failed. ret = %{public}d.", ret);
            AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env,
                ret, PermissionConstants::PERMISSION_MANAGE_MISSION), nullptr);
            return;
        }

        AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
    }
    void OnUnlockMission(ani_env* env, ani_int missionId, ani_object callback)
    {
        TAG_LOGD(AAFwkTag::MISSION, "OnUnlockMission Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "null env");
            return;
        }

        auto ret = AbilityManagerClient::GetInstance()->UnlockMissionForCleanup(missionId);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::MISSION, "OnUnlockMission is failed. ret = %{public}d.", ret);
            AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env,
                ret, PermissionConstants::PERMISSION_MANAGE_MISSION), nullptr);
            return;
        }

        AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), nullptr);
    }

    void OnGetMissionSnapShot(ani_env* env, ani_string deviceId, ani_int missionId, ani_object callback, bool isLowResolution)
    {
        TAG_LOGD(AAFwkTag::MISSION, "OnGetMissionSnapShot Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "null env");
            return;
        }

        auto emptyObject = GetEmptyMissionSnapShot(env);   //可能需要新增这个函数

        std::string stdDeviceId = "";
        if (!GetStdString(env, deviceId, stdDeviceId) || stdDeviceId.empty()) {
            TAG_LOGE(AAFwkTag::MISSION, "GetStdString failed");
            AsyncCallback(env, callback, EtsErrorUtil::CreateInvalidParamError(env,
                "Parse param deviceId failed, must be a string."), emptyObject);
            return;
        }
        AAFwk::MissionSnapShot missionSnapShot;   //这个文件新增？
        auto ret = AbilityManagerClient::GetInstance()->GetMissionSnapShot(stdDeviceId,
            missionId, missionSnapShot, isLowResolution);
        if (ret != 0) {
            TAG_LOGE(AAFwkTag::MISSION, "OnGetMissionSnapShot is failed. ret = %{public}d.", ret);
            AsyncCallback(env, callback, EtsErrorUtil::CreateErrorByNativeErr(env,
                ret, PermissionConstants::PERMISSION_MANAGE_MISSION), emptyObject);
            return;
        }
        auto aniMissionSnapShot = CreateEtsMissionSnapShot(env, MissionSnapShot);
        AsyncCallback(env, callback, EtsErrorUtil::CreateError(env, AbilityErrorCode::ERROR_OK), aniMissionSnapShot);
    }
    
    ani_object CreateEtsMissionSnapShot(ani_env* env, const AAFwk::MissionSnapShot &missionSnapShot)
    {
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "env is null");
            return nullptr;
        }
        ani_class cls = nullptr;
        ani_status status = ANI_ERROR;
        status = env->FindClass(env, "Lapplication/MissionSnapShot/MissionSnapShotInner;", cls);
        if (status != ANI_OK || cls == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "FindClass failed");
            return nullptr;
        }
        ani_method method = nullptr;
        status = env->Class_FindMethod(cls, "<ctor>", ":V", &method);
        if (status != ANI_OK || method == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "Class_FindMethod failed");
            return nullptr;
        }
        ani_object object = nullptr;
        status = env->Object_New(cls, method, &object);
        if (status != ANI_OK || object == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "Object_New failed");
            return nullptr;
        }
        ani_object abilityObj = WrapElementName(env, missionSnapShot.topAbility);
        if (!SetRefProperty(env, object, "ability", abilityObj)) {
            TAG_LOGE(AAFwkTag::MISSION, "Set ability failed");
            return nullptr;
        }
#ifdef SUPPORT_SCREEN
        auto snapshotValue =
            OHOS::Media::PixelMapTaiheAni::CreateEtsPixelMap(env, missionSnapShot.snapshot);
        if (!SetRefProperty(env, object, "snapshot", snapshotValue)) {
            TAG_LOGE(AAFwkTag::MISSION, "Set snapshot failed");
            return nullptr;
        }
#endif
        return object;
    }

    ani_object GetEmptyMissionSnapShot(ani_env* env)
    {
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "null env");
            return nullptr;
        }
        ani_class cls = nullptr;
        ani_status status = env->FindClass("Lapplication/MissionSnapShot/MissionSnapShotInner;", &cls);
        if (status != ANI_OK || cls == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "find Context failed status: %{public}d", status);
            return nullptr;
        }
        ani_method method = nullptr;
        status = env->Class_FindMethod(cls, "<ctor>", ":V", &method);
        if (status != ANI_OK || method == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "Class_FindMethod ctor failed status: %{public}d", status);
            return nullptr;
        }
        ani_object objValue = nullptr;
        status = env->Object_New(cls, method, &objValue);
        if (status != ANI_OK || objValue == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "Object_New failed status: %{public}d", status);
            return nullptr;
        }
        return objValue;
    } 
    /*
    void OnMoveMissionsToBackground(ani_env* env, ani_int missionIds, ani_object callback)    //注意：ani_int missionIds，这个的类型？
    {
        TAG_LOGD(AAFwkTag::MISSION, "OnMoveMissionsToBackground Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "null env");
            return;
        }
    }

    void OnMoveMissionsToForeground(ani_env* env, ani_int missionIds, ani_int topMission, ani_object callback)    //ani_int missionIds，这个的类型？
    {
        TAG_LOGD(AAFwkTag::MISSION, "OnMoveMissionsToForeground Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "null env");
            return;
        }
    }

    void OnMoveMissionToFront(ani_env* env, ani_int missionId, ani_object options, ani_object callback)   //注意：ani_object options，这个的类型？
    {
        TAG_LOGD(AAFwkTag::MISSION, "OnMoveMissionToFront Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "null env");
            return;
        } 
    }

    void OnOn(ani_env* env, ani_object type, ani_object listener)   //注意：ani_object type，ani_object listener,这两个的类型？
    {
       TAG_LOGD(AAFwkTag::MISSION, "OnOn Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "null env");
            return;
        }  
    }

    void OnOff(ani_env* env, ani_object type, ani_int listenerId, ani_object callback)   //注意：ani_object type，这个的类型？
    {
        TAG_LOGD(AAFwkTag::MISSION, "OnOff Call");
        if (env == nullptr) {
            TAG_LOGE(AAFwkTag::MISSION, "null env");
            return;
        } 
    }
    */
};

EtsMissionManager EtsMissionManager::instance;

void EtsMissionManagerInit(ani_env* env)
{
    TAG_LOGD(AAFwkTag::MISSION, "EtsMissionManagerInit Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_namespace ns;
    status = env->FindNamespace(ETS_MISSION_MANAGER_NAME, &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::MISSION, "FindNamespace application failed status: %{public}d", status);
        return;
    }
    std::array methods = {
        ani_native_function {
            "nativeClearAllMissions", "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsMissionManager::ClearAllMissions)
        },
        ani_native_function {
            "nativeGetMissionInfo",
            "Lstd/core/String;ILutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsMissionManager::GetMissionInfo)
        },
         ani_native_function {
            "nativeGetMissionInfos",
            "Lstd/core/String;ILutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsMissionManager::GetMissionInfos)
        },
        ani_native_function {
            "nativeClearMission","ILutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsMissionManager::ClearMission)
        },
        ani_native_function {
            "nativeLockMission","ILutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsMissionManager::LockMission)
        },
        ani_native_function {
            "nativeUnlockMission","ILutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsMissionManager::UnlockMission)
        },
        ani_native_function {
            "nativeGetMissionSnapShot",
            "Lstd/core/String;ILutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsMissionManager::GetMissionSnapShot)
        },
        ani_native_function {
            "nativeMoveMissionsToBackground",
            "Lstd/core/String;ILutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsMissionManager::MoveMissionsToBackground)
        },
        ani_native_function {
            "nativeMoveMissionsToForeground",
            "Lstd/core/String;ILutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsMissionManager::MoveMissionsToForeground)
        },
        ani_native_function {
            "nativeMoveMissionToFront",
            "Lstd/core/String;ILutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsMissionManager::MoveMissionToFront)
        },
        ani_native_function {
            "nativeOn",
            "Lstd/core/String;ILutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsMissionManager::On)
        },
         ani_native_function {
            "nativeOff",
            "Lstd/core/String;ILutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsMissionManager::Off)
        }
    };
    status = env->Namespace_BindNativeFunctions(ns, methods.data(), methods.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::MISSION, "Namespace_BindNativeFunctions failed status: %{public}d", status);
    }
}

extern "C" {
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGD(AAFwkTag::MISSION, "in MissionManagerETS.ANI_Constructor");
    if (vm == nullptr || result == nullptr) {
        TAG_LOGE(AAFwkTag::MISSION, "null vm or result");
        return ANI_INVALID_ARGS;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::MISSION, "GetEnv failed, status: %{public}d", status);
        return ANI_NOT_FOUND;
    }
    EtsMissionManagerInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::MISSION, "MissionManagerETS.ANI_Constructor finished");
    return ANI_OK;
}
}
} // namespace AbilityRuntime
} // namespace OHOS