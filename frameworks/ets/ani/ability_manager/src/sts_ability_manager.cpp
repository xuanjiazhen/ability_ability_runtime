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

#include "sts_ability_manager.h"

#include "ability_manager_client.h"
#include "ability_business_error.h"
#include "ability_manager_errors.h"
#include "ability_runtime_error_util.h"
#include "ani_common_ability_state_data.h"
#include "ani_common_want.h"
#include "ani_enum_convert.h"
#include "hilog_tag_wrapper.h"
#include "ability_manager_interface.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "sts_error_utils.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AbilityManagerSts {

constexpr int32_t ERR_FAILURE = -1;
constexpr const char *CLASSNAME_ARRAY = "Lescompat/Array;";
constexpr const char *SET_OBJECT_VOID_SIGNATURE = "ILstd/core/Object;:V";
constexpr const char *CLASSNAME_ABILITY_RRUNNINGINFO = "Lapplication/AbilityRunningInfo/AbilityRunningInfoImpl;";
constexpr const char *ABILITY_STATE_ENUM_NAME = "Lability/abilityManager/AbilityState;";
sptr<AppExecFwk::IAbilityManager> GetAbilityManagerInstance()
{
    sptr<ISystemAbilityManager> systemAbilityManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> abilityManagerObj =
        systemAbilityManager->GetSystemAbility(ABILITY_MGR_SERVICE_ID);
    return iface_cast<AppExecFwk::IAbilityManager>(abilityManagerObj);
}

static ani_object GetForegroundUIAbilities(ani_env *env)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call GetForegroundUIAbilities");

    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return nullptr;
    }

    sptr<AppExecFwk::IAbilityManager> abilityManager = GetAbilityManagerInstance();
    if (abilityManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "abilityManager is null");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }
    std::vector<AppExecFwk::AbilityStateData> list;
    int32_t ret = abilityManager->GetForegroundUIAbilities(list);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed: ret=%{public}d", ret);
        AbilityRuntime::AbilityErrorCode code = AbilityRuntime::GetJsErrorCodeByNativeError(ret);
        AbilityRuntime::ThrowStsError(env, code);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetForegroundUIAbilities succeeds, list.size=%{public}zu", list.size());
    ani_object aniArray = AppExecFwk::CreateAniAbilityStateDataArray(env, list);
    if (aniArray == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null aniArray");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return nullptr;
    }
    return aniArray;
}

static void GetTopAbility(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call GetTopAbility");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
#ifdef ENABLE_ERRCODE
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not system app");
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return;
    }
#endif
    AppExecFwk::ElementName elementName = AAFwk::AbilityManagerClient::GetInstance()->GetTopAbility();
    int resultCode = 0;
    ani_object elementNameobj = AppExecFwk::WrapElementName(env, elementName);
    if (elementNameobj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null elementNameobj");
        resultCode = ERR_FAILURE;
    }
    ani_ref callbackRef = nullptr;
    auto status = env->GlobalReference_Create(callback, &callbackRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Create Gloabl ref for abilitymanager failed %{public}d", status);
        AbilityRuntime::ThrowStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }
    AppExecFwk::AsyncCallback(env, reinterpret_cast<ani_object>(callbackRef),
        OHOS::AbilityRuntime::CreateStsErrorByNativeErr(env, resultCode),
        elementNameobj);
    return;
}

void StsAbilityManagerRegistryInit(ani_env *env)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "call StsAbilityManagerRegistryInit");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    ani_status status = ANI_ERROR;
    if (env->ResetError() != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "ResetError failed");
    }
    ani_namespace ns;
    status = env->FindNamespace("L@ohos/app/ability/abilityManager/abilityManager;", &ns);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "FindNamespace abilityManager failed status : %{public}d", status);
        return;
    }
    std::array methods = {
        ani_native_function {
            "nativeGetForegroundUIAbilities",
            ":Lescompat/Array;",
            reinterpret_cast<void *>(GetForegroundUIAbilities)
        },
        ani_native_function {"nativeGetTopAbility", nullptr, reinterpret_cast<void *>(GetTopAbility)},
        ani_native_function { "nativeGetAbilityRunningInfos", nullptr,
            reinterpret_cast<void *>(StsAbilityManager::GetAbilityRunningInfos) },
        ani_native_function { "nativeNotifyDebugAssertResult", nullptr,
            reinterpret_cast<void *>(StsAbilityManager::NotifyDebugAssertResult) },
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
    TAG_LOGD(AAFwkTag::ABILITYMGR, "in AbilityManagerSts.ANI_Constructor");
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
    StsAbilityManagerRegistryInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AbilityManagerSts.ANI_Constructor finished");
    return ANI_OK;
}
}

void StsAbilityManager::GetAbilityRunningInfos(ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "GetAbilityRunningInfos");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    std::vector<AAFwk::AbilityRunningInfo> infos;
    auto errcode = AAFwk::AbilityManagerClient::GetInstance()->GetAbilityRunningInfos(infos);
    ani_object retObject = nullptr;
    WrapAbilityRunningInfoArray(env, retObject, infos);
    AppExecFwk::AsyncCallback(env, callback, OHOS::AbilityRuntime::CreateStsErrorByNativeErr(env, errcode), retObject);
}

void StsAbilityManager::NotifyDebugAssertResult(
    ani_env *env, ani_string sessionIdObj, ani_object userStatusObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "NotifyDebugAssertResult");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return;
    }
    std::string SessionIdStr = "";
    if (!OHOS::AppExecFwk::GetStdString(env, sessionIdObj, SessionIdStr)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "convert sessionId failed");
        AbilityRuntime::ThrowStsInvalidParamError(env, "Parse param  sessionId failed, must be a string.");
        return;
    }
    uint64_t assertSessionId = std::stoull(SessionIdStr);
    if (assertSessionId == 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "convert sessionId failed");
        AbilityRuntime::ThrowStsInvalidParamError(
            env, "Parse param sessionId failed, value must not be equal to zero.");
        return;
    }
    int32_t userStatus;
    if (!AAFwk::AniEnumConvertUtil::EnumConvert_StsToNative(env, userStatusObj, userStatus)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "convert status failed");
        AbilityRuntime::ThrowStsInvalidParamError(env, "Parse param status failed, must be a UserStatus.");
        AppExecFwk::AsyncCallback(env, callback,
            OHOS::AbilityRuntime::CreateStsErrorByNativeErr(env,
            static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_PARAM)), nullptr);
        return;
    }
    auto amsClient = AAFwk::AbilityManagerClient::GetInstance();
    if (amsClient == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null amsClient");
        AppExecFwk::AsyncCallback(
            env, callback, OHOS::AbilityRuntime::CreateStsErrorByNativeErr(env, AAFwk::INNER_ERR), nullptr);
        return;
    }
    auto ret = amsClient->NotifyDebugAssertResult(assertSessionId, static_cast<AAFwk::UserStatus>(userStatus));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyDebugAssertResult failed, ret: %{public}d", ret);
    }
    AppExecFwk::AsyncCallback(env, callback, OHOS::AbilityRuntime::CreateStsErrorByNativeErr(env, ret), nullptr);
}

bool StsAbilityManager::WrapAbilityRunningInfoArray(
    ani_env *env, ani_object &arrayObj, const std::vector<AAFwk::AbilityRunningInfo> &infos)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "WrapAbilityRunningInfoArray");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
        return false;
    }
    ani_class arrayCls = nullptr;
    ani_method arrayCtor;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(CLASSNAME_ARRAY, &arrayCls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "status : %{public}d", status);
        return false;
    }
    if (arrayCls == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null arrayCls");
        return false;
    }
    if ((status = env->Class_FindMethod(arrayCls, "<ctor>", "I:V", &arrayCtor)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "status : %{public}d", status);
        return false;
    }
    if (arrayCtor == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null arrayCtor");
        return false;
    }
    if ((status = env->Object_New(arrayCls, arrayCtor, &arrayObj, infos.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "status : %{public}d", status);
        return false;
    }
    for (size_t i = 0; i < infos.size(); i++) {
        ani_object infoObj = nullptr;
        if (!WrapAbilityRunningInfo(env, infoObj, infos[i])) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "WrapAbilityRunningInfo failed");
        }
        status = env->Object_CallMethodByName_Void(arrayObj, "$_set", SET_OBJECT_VOID_SIGNATURE, i, infoObj);
        if (status != ANI_OK) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "status : %{public}d", status);
            return false;
        }
    }
    return true;
}

bool StsAbilityManager::WrapAbilityRunningInfo(ani_env *env, ani_object &infoObj,
    const AAFwk::AbilityRunningInfo &info)
{
    ani_object elementNameObj = WrapElementName(env, info.ability);
    if (elementNameObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "WrapElementName failed");
        return false;
    }
    ani_class cls;
    ani_status status = ANI_ERROR;
    if ((status = env->FindClass(CLASSNAME_ABILITY_RRUNNINGINFO, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "findClass failed, status: %{public}d", status);
        return false;
    }
    if (cls == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null cls");
        return false;
    }
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "findMethod failed, status: %{public}d", status);
        return false;
    }
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null method");
        return false;
    }
    if ((status = env->Object_New(cls, method, &infoObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Object_New failed, status: %{public}d", status);
        return false;
    }
    if ((status = env->Object_SetPropertyByName_Ref(infoObj, "ability", elementNameObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "set ability failed, status: %{public}d", status);
        return false;
    }
    return WrapAbilityRunningInfoInner(env, infoObj, info, cls);
}

bool StsAbilityManager::WrapAbilityRunningInfoInner(
    ani_env *env, ani_object &infoObj, const AAFwk::AbilityRunningInfo &info, ani_class cls)
{
    ani_status status = ANI_ERROR;
    if ((status = env->Object_SetPropertyByName_Double(infoObj, "pid", info.pid)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "set pid failed, status: %{public}d", status);
        return false;
    }
    if ((status = env->Object_SetPropertyByName_Double(infoObj, "uid", info.uid)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "set uid failed, status: %{public}d", status);
        return false;
    }
    if (!AppExecFwk::SetFieldString(env, cls, infoObj, "processName", info.processName)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "set processName failed");
        return false;
    }
    if ((status = env->Object_SetPropertyByName_Double(infoObj, "startTime", info.startTime)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "set processName failed, status: %{public}d", status);
        return false;
    }
    ani_enum_item abilityStateItem {};
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvertNativeToSts(
        env, ABILITY_STATE_ENUM_NAME, info.abilityState, abilityStateItem);
    if ((status = env->Object_SetPropertyByName_Ref(infoObj, "abilityState", abilityStateItem)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "set abilityState failed, status: %{public}d", status);
        return false;
    }
    return true;
}
} // namespace AbilityManagerSts
} // namespace OHOS