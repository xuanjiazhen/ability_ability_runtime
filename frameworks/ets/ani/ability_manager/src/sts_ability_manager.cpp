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

#include "ability_context.h"
#include "ability_manager_client.h"
#include "ability_business_error.h"
#include "ability_manager_errors.h"
#include "ability_runtime_error_util.h"
#include "ani_base_context.h"
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
using AtomicServiceStartupRule = AbilityRuntime::AtomicServiceStartupRule;
namespace AbilityManagerSts {

constexpr int32_t ERR_FAILURE = -1;

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

static void NativeQueryAtomicServiceStartupRule(ani_env *env, ani_object contextObj, ani_string aniAppId,
    ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "NativeQueryAtomicServiceStartupRule");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
    }
    StsAbilityManager::GetInstance().QueryAtomicServiceStartupRule(env, contextObj, aniAppId, callbackObj);
}

void StsAbilityManager::GetAbilityRunningInfos(ani_env *env, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
    }
    std::vector<AAFwk::AbilityRunningInfo> infos;
    auto errcode = AAFwk::AbilityManagerClient::GetInstance()->GetAbilityRunningInfos(infos);
    AppExecFwk::AsyncCallback(env, callback, OHOS::AbilityRuntime::CreateStsErrorByNativeErr(env, errcode), nullptr);
}

void StsAbilityManager::NotifyDebugAssertResult(ani_env *env, ani_string sessionIdObj, ani_object userStatusObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
    }
    std::string SessionIdStr = "";
    if (!OHOS::AppExecFwk::GetStdString(env, sessionIdObj, SessionIdStr)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "convert sessionId failed");
        AbilityRuntime::ThrowStsInvalidParamError(env, "Parse param appId failed, must be a string.");
        return;
    }
    uint64_t assertSessionId = std::stoull(SessionIdStr);
    if (assertSessionId == 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "convert sessionId failed");
        AbilityRuntime::ThrowStsInvalidParamError(env, "Parse param sessionId failed, value must not be equal to zero.");
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
        AppExecFwk::AsyncCallback(env, callback,
            OHOS::AbilityRuntime::CreateStsErrorByNativeErr(env, AAFwk::INNER_ERR), nullptr);
        return;
    }
    auto ret = amsClient->NotifyDebugAssertResult(assertSessionId, static_cast<AAFwk::UserStatus>(userStatus));
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "NotifyDebugAssertResult failed %{public}d", ret);
    }
    AppExecFwk::AsyncCallback(env, callback,
        OHOS::AbilityRuntime::CreateStsErrorByNativeErr(env, ret), nullptr);
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
        ani_native_function {"nativeGetAbilityRunningInfos", nullptr, reinterpret_cast<void *>(StsAbilityManager::GetAbilityRunningInfos)},
        ani_native_function {"nativeQueryAtomicServiceStartupRule", nullptr, reinterpret_cast<void *>(NativeQueryAtomicServiceStartupRule)},
        ani_native_function {"nativeNotifyDebugAssertResult", nullptr, reinterpret_cast<void *>(StsAbilityManager::NotifyDebugAssertResult)},
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
void StsAbilityManager::QueryAtomicServiceStartupRule(ani_env *env, ani_object contextObj, ani_string aniAppId,
    ani_object callbackObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null env");
    }
    ani_boolean stageMode = false;
    ani_status status = OHOS::AbilityRuntime::IsStageContext(env, contextObj, stageMode);
    if (status != ANI_OK || !stageMode) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "not stageMode");
        AbilityRuntime::ThrowStsInvalidParamError(env, "Parse param context failed, must be a context of stageMode.");
        return;
    }
    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, contextObj);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null context");
        AbilityRuntime::ThrowStsInvalidParamError(env, "Parse param context failed, must not be nullptr.");
        return;
    }
    auto uiAbilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    if (uiAbilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null UIAbilityContext");
        AbilityRuntime::ThrowStsInvalidParamError(env, "Parse param context failed, must be UIAbilityContext.");
        return;
    }
    std::string appIdStr = "";
    if (!OHOS::AppExecFwk::GetStdString(env, aniAppId, appIdStr)) {
        TAG_LOGE(AAFwkTag::DELEGATOR, "GetStdString Failed");
        AbilityRuntime::ThrowStsInvalidParamError(env, "Parse param appId failed, must be a string.");
        return;
    }
    auto token = uiAbilityContext->GetToken();
    QueryAtomicServiceStartupRuleInner(env, token, appIdStr, callbackObj);
}

void StsAbilityManager::QueryAtomicServiceStartupRuleInner(ani_env *env, sptr<IRemoteObject> token, std::string appId,
    ani_object callbackObj) {
    int32_t errorCode = ERR_OK;
    auto rule = std::make_shared<AtomicServiceStartupRule>();
    std::string startTime = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::
        system_clock::now().time_since_epoch()).count());
    auto ret = AddQueryERMSObserver(env, token, appId, startTime, callbackObj);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "AddQueryERMSObserver failed, ret=%{public}d", ret);
        return;
    }
    errorCode = AAFwk::AbilityManagerClient::GetInstance()->QueryAtomicServiceStartupRule(
        token, appId, startTime, *rule);
    if (queryERMSObserver_ == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "null observer");
        return;
    }
    if (errorCode == AAFwk::ERR_ECOLOGICAL_CONTROL_STATUS) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "openning dialog to confirm");
        return;
    }
    if (errorCode != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "query failed: %{public}d", errorCode);
        queryERMSObserver_->OnQueryFinished(appId, startTime, *rule, AAFwk::INNER_ERR);
        return;
    }
    queryERMSObserver_->OnQueryFinished(appId, startTime, *rule, ERR_OK);
}

int32_t StsAbilityManager::AddQueryERMSObserver(ani_env *env, sptr<IRemoteObject> token, const std::string &appId,
    const std::string &startTime, ani_object callbackObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "AddQueryERMSObserver");
    int32_t ret = 0;
    if (queryERMSObserver_ == nullptr) {
        ani_vm *etsVm = nullptr;
        ani_status status = ANI_ERROR;
        if ((status = env->GetVM(&etsVm)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "status : %{public}d", status);
        }
        queryERMSObserver_ = new AbilityRuntime::StsQueryERMSObserver(etsVm);
    }
    queryERMSObserver_->AddStsObserverObject(appId, startTime, callbackObj);
    ret = AAFwk::AbilityManagerClient::GetInstance()->AddQueryERMSObserver(token, queryERMSObserver_);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "addQueryERMSObserver error");
        AtomicServiceStartupRule rule;
        queryERMSObserver_->OnQueryFinished(appId, startTime, rule, AAFwk::INNER_ERR);
        return ret;
    }
    return ERR_OK;
}
} // namespace AbilityManagerSts
} // namespace OHOS