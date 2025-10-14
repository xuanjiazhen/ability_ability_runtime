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

#include "ets_kiosk_manager.h"

#include "ability_manager_client.h"
#include "ani_base_context.h"
#include "ani_common_util.h"
#include "hilog_tag_wrapper.h"
#include "ability_context.h"
#include "ability_business_error.h"
#include "kiosk_status.h"
#include "ets_error_utils.h"

namespace OHOS {
namespace AbilityRuntime {
using AbilityManagerClient = AAFwk::AbilityManagerClient;

constexpr const char* KIOSK_STATUS_CLASS_NAME = "Lapplication/KioskStatus/KioskStatusInner";

bool IsNull(ani_env *env, ani_ref ref)
{
    ani_boolean isNull = ANI_FALSE;
    env->Reference_IsNull(ref, &isNull);
    return isNull;
}

bool IsRefUndefined(ani_env *env, ani_ref ref)
{
    ani_boolean isUndefined = ANI_FALSE;
    env->Reference_IsUndefined(ref, &isUndefined);
    return isUndefined;
}

void EnterKioskModeSyncCheck([[maybe_unused]]ani_env *env, ani_object contextObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, contextObj);
        if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        EtsErrorUtil::ThrowInvalidParamError(
            env, "Failed to get context, context must not be nullptr.");
    }
    auto uiAbilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    if (uiAbilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null UIAbilityContext");
        EtsErrorUtil::ThrowInvalidParamError(
            env, "Failed to get uiAbilityContext, uiAbilityContext must not be nullptr.");
    }
}

void EnterKioskModeSync([[maybe_unused]]ani_env *env, ani_object contextObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, contextObj);
    auto uiAbilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    auto token = uiAbilityContext->GetToken();
    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    *innerErrCode = AbilityManagerClient::GetInstance()->EnterKioskMode(token);
    if (*innerErrCode != ERR_OK) {
        AppExecFwk::AsyncCallback(
            env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(*innerErrCode)), nullptr);
    }
    AppExecFwk::AsyncCallback(
        env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(*innerErrCode)), nullptr);
}

void ExitKioskModeSyncCheck([[maybe_unused]]ani_env *env, ani_object contextObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, contextObj);
        if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        EtsErrorUtil::ThrowInvalidParamError(
            env, "Failed to get context, context must not be nullptr.");
    }
    auto uiAbilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    if (uiAbilityContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null UIAbilityContext");
        EtsErrorUtil::ThrowInvalidParamError(
            env, "Failed to get uiAbilityContext, uiAbilityContext must not be nullptr.");
    }
}

void ExitKioskModeSync([[maybe_unused]]ani_env *env, ani_object contextObj, ani_object callback)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto context = OHOS::AbilityRuntime::GetStageModeContext(env, contextObj);
    auto uiAbilityContext = AbilityRuntime::Context::ConvertTo<AbilityRuntime::AbilityContext>(context);
    auto token = uiAbilityContext->GetToken();
    auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    *innerErrCode = AbilityManagerClient::GetInstance()->ExitKioskMode(token);
    if (*innerErrCode != ERR_OK) {
        AppExecFwk::AsyncCallback(
            env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(*innerErrCode)), nullptr);
    }
    AppExecFwk::AsyncCallback(
        env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(*innerErrCode)), nullptr);
}

ani_object CreateKioskStatusRet(ani_env *env, std::shared_ptr<AAFwk::KioskStatus> kioskStatus)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }
    if (kioskStatus == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null kioskStatus");
        return nullptr;
    }
    ani_class cls = nullptr;
    ani_method method = nullptr;
    ani_object contextObj = nullptr;
    ani_status status = env->FindClass(KIOSK_STATUS_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Class_FindMethod status: %{public}d", status);
        return nullptr;
    }
    if ((status = env->Object_New(cls, method, &contextObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_New status: %{public}d", status);
        return nullptr;
    }
    if (!AppExecFwk::SetFieldBoolByName(env, cls, contextObj, "isKioskMode", kioskStatus->isKioskMode_)) {
        TAG_LOGE(AAFwkTag::APPKIT, "set isKioskMode failed");
        return nullptr;
    }
    if (!AppExecFwk::SetFieldStringByName(env, cls, contextObj, "kioskBundleName", kioskStatus->kioskBundleName_)) {
        TAG_LOGE(AAFwkTag::APPKIT, "set kioskBundleName failed");
        return nullptr;
    }
    if (!AppExecFwk::SetFieldIntByName(env, cls, contextObj, "kioskBundleUid", kioskStatus->kioskBundleUid_)) {
        TAG_LOGE(AAFwkTag::APPKIT, "set kioskBundleUid failed");
        return nullptr;
    }
    return contextObj;
}

void GetKioskStatusSync([[maybe_unused]]ani_env *env, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "Get KioskStatus");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    // auto innerErrCode = std::make_shared<ErrCode>(ERR_OK);
    std::shared_ptr<AAFwk::KioskStatus> kioskStatus = std::make_shared<AAFwk::KioskStatus>();
    // *innerErrCode = AbilityManagerClient::GetInstance()->GetKioskStatus(*kioskStatus);
    // if (*innerErrCode != ERR_OK) {
    //     TAG_LOGE(AAFwkTag::APPKIT, "innerErrCode=%{public}d", *innerErrCode);
    //     AppExecFwk::AsyncCallback(
    //         env, callback, EtsErrorUtil::CreateErrorByNativeErr(env, static_cast<int32_t>(*innerErrCode)), nullptr);
    //     return;
    // }
    kioskStatus->isKioskMode_ = true;
    kioskStatus->kioskBundleName_ = "zuogang.app.kiosk";
    kioskStatus->kioskBundleUid_ = 10086;
    auto ret = CreateKioskStatusRet(env, kioskStatus);
    AppExecFwk::AsyncCallback(env, callback, nullptr, ret);
}

void EtsKioskManagerInit(ani_env *env)
{
    TAG_LOGI(AAFwkTag::APPKIT, "EtsKioskManagerInit call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Invalid param");
        return;
    }
    ani_namespace ns;
    const char* targetNamespace = "L@ohos/app/ability/kioskManager/kioskManager;";
    if (env->FindNamespace(targetNamespace, &ns) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindNamespace failed");
    }
    if (ns == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "ns null");
        return;
    }
    std::array functions = {
        ani_native_function {
            "nativeEnterKioskMode",
            nullptr,
            reinterpret_cast<void*>(EnterKioskModeSync)
        },
        ani_native_function {
            "nativeEnterKioskModeCheck",
            nullptr,
            reinterpret_cast<void*>(EnterKioskModeSyncCheck)
        },
        ani_native_function {
            "nativeExitKioskMode",
            nullptr,
            reinterpret_cast<void*>(ExitKioskModeSync)
        },
        ani_native_function {
            "nativeExitKioskModeCheck",
            nullptr,
            reinterpret_cast<void*>(ExitKioskModeSyncCheck)
        },
        ani_native_function {
            "nativeGetKioskStatus",
            nullptr,
            reinterpret_cast<void*>(GetKioskStatusSync)
        },

    };
    TAG_LOGI(AAFwkTag::APPKIT, "EtsKioskManagerInit bind functions");
    if (env->Namespace_BindNativeFunctions(ns, functions.data(), functions.size()) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Namespace_BindNativeFunctions failed");
    };
    TAG_LOGI(AAFwkTag::APPKIT, "EtsKioskManagerInit end");
}

extern "C"{
ANI_EXPORT ani_status ANI_Constructor(ani_vm *vm, uint32_t *result)
{
    TAG_LOGI(AAFwkTag::APPKIT, "ANI_Constructor");
    if (vm == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "vm null");
        return ANI_ERROR;
    }
    if (result == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "result null");
        return ANI_ERROR;
    }
    ani_env *env = nullptr;
    ani_status status = ANI_ERROR;
    status = vm->GetEnv(ANI_VERSION_1, &env);
    if (status != ANI_OK) {
        TAG_LOGI(AAFwkTag::APPKIT, "GetEnv failed status: %{public}d", status);
        return ANI_NOT_FOUND;
    }

    EtsKioskManagerInit(env);
    *result = ANI_VERSION_1;
    TAG_LOGI(AAFwkTag::APPKIT, "ANI_Constructor finish");
    return ANI_OK;
}
}
} // namespace Ability
} // namespace OHOS