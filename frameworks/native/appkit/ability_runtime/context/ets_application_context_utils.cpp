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

#include "ani_enum_convert.h"
#include "application_context_manager.h"
#include "ets_application_context_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr const char* ETS_APPLICATION_CONTEXT_CLASS_NAME = "Lapplication/ApplicationContext/ApplicationContext;";
constexpr double FOUNT_SIZE = 0.0;
}
void EtsApplicationContextUtils::RestartApp(ani_env *env, ani_object aniObj,
    ani_object wantObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "RestartApp Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = GeApplicationContext(env, aniObj);
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        return;
    }
    AAFwk::Want want;
    if (!AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse want failed");
        ThrowStsInvalidParamError(env, "Parse param want failed, want must be Want.");
        return;
    }
    auto errCode = applicationContext->RestartApp(want);
    switch (errCode) {
    case ERR_INVALID_VALUE:
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        break;
    case AAFwk::ERR_RESTART_APP_INCORRECT_ABILITY:
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_RESTART_APP_INCORRECT_ABILITY);
        break;
    case AAFwk::ERR_RESTART_APP_FREQUENT:
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_RESTART_APP_FREQUENT);
        break;
    case AAFwk::NOT_TOP_ABILITY:
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_NOT_TOP_ABILITY);
        break;
    default:
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
    TAG_LOGD(AAFwkTag::APPKIT, "RestartApp errCode is %{public}d", errCode);
}

void EtsApplicationContextUtils::SetFont(ani_env *env, ani_object aniObj,
    ani_string font)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetFont Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = GeApplicationContext(env, aniObj);
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        return;
    }
    std::string stdFont = "";
    if (!AppExecFwk::GetStdString(env, font, stdFont)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse font failed");
        ThrowStsInvalidParamError(env, "Parse param font failed, font must be string.");
    }
    TAG_LOGD(AAFwkTag::APPKIT, "SetFont font %{public}s", stdFont.c_str());
    applicationContext->SetFont(stdFont);
}

void EtsApplicationContextUtils::SetColorMode(ani_env *env, ani_object aniObj,
    ani_enum_item colorMode)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetColorMode Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = GeApplicationContext(env, aniObj);
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        return;
    }
    ani_int mode = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvertStsToNative(env, colorMode, mode)) {
        ThrowStsInvalidParamError(env, "Parse param colorMode failed, colorMode must be number.");
        TAG_LOGE(AAFwkTag::APPKIT, "Parse colorMode failed");
        return;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "colorMode is %{public}d", mode);
    applicationContext->SetColorMode(static_cast<int32_t>(mode));
}

void EtsApplicationContextUtils::SetLanguage(ani_env *env, ani_object aniObj,
    ani_string language)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetLanguage Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = GeApplicationContext(env, aniObj);
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        return;
    }
    std::string stdLanguage = "";
    if (!AppExecFwk::GetStdString(env, language, stdLanguage)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse language failed");
        ThrowStsInvalidParamError(env, "Parse param language failed, language must be string.");
        return;
    }
    applicationContext->SetLanguage(stdLanguage);
    TAG_LOGD(AAFwkTag::APPKIT, "stdLanguage language %{public}s", stdLanguage.c_str());
}

void EtsApplicationContextUtils::SetFontSizeScale(ani_env *env, ani_object aniObj,
    ani_double fontSizeScale)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetFontSizeScale Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = GeApplicationContext(env, aniObj);
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        return;
    }

    double stdFontSizeScale = static_cast<double>(fontSizeScale);
    TAG_LOGD(AAFwkTag::APPKIT, "fontSizeScale: %{public}f", stdFontSizeScale);
    if (fontSizeScale < FOUNT_SIZE) {
        TAG_LOGE(AAFwkTag::APPKIT, "invalid size");
        ThrowStsInvalidParamError(env, "Invalid font size.");
        return;
    }
    applicationContext->SetFontSizeScale(stdFontSizeScale);
}

void EtsApplicationContextUtils::ClearUpApplicationData(ani_env *env, ani_object aniObj,
    ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "ClearUpApplicationData Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = GeApplicationContext(env, aniObj);
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "nativeContext is null");
        AppExecFwk::AsyncCallback(env, callback, CreateStsError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT,
            "applicationContext if already released."), nullptr);
        return;
    }
    applicationContext->ClearUpApplicationData();
    AppExecFwk::AsyncCallback(env, callback, CreateStsError(env, AbilityErrorCode::ERROR_OK), nullptr);
}

void EtsApplicationContextUtils::GetRunningProcessInformation(ani_env *env,
    ani_object aniObj, ani_object callback)
{
    TAG_LOGD(AAFwkTag::APPKIT, "GetRunningProcessInformation Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    ani_object emptyArray = AppManagerSts::CreateEmptyAniArray(env);
    auto applicationContext = GeApplicationContext(env, aniObj);
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "nativeContext is null");
        AppExecFwk::AsyncCallback(env, callback, CreateStsError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT,
            "applicationContext if already released."), emptyArray);
        return;
    }
    std::vector<AppExecFwk::RunningProcessInfo> infos;
    AppExecFwk::RunningProcessInfo processInfo;
    ErrCode innerErrCode = applicationContext->GetProcessRunningInformation(processInfo);
    if (innerErrCode == ERR_OK) {
        infos.emplace_back(processInfo);
        ani_object aniInfosRef = AppManagerSts::CreateRunningProcessInfoArray(env, infos);
        if (aniInfosRef == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null array");
            AppExecFwk::AsyncCallback(env, callback, CreateStsError(env,
                (ani_int)AbilityErrorCode::ERROR_CODE_INNER, "Initiate array failed."), emptyArray);
        } else {
            AppExecFwk::AsyncCallback(env, callback, CreateStsError(env, AbilityErrorCode::ERROR_OK), aniInfosRef);
        }
    } else {
        AppExecFwk::AsyncCallback(env, callback, CreateStsError(env,
            (ani_int)AbilityErrorCode::ERROR_CODE_INNER, "Get process infos failed."), emptyArray);
    }
}

void EtsApplicationContextUtils::killAllProcesses(ani_env *env, ani_object aniObj,
    ani_boolean clearPageStack, ani_object call)
{
    TAG_LOGD(AAFwkTag::APPKIT, "killAllProcesses Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is nullptr");
        return;
    }
    ani_object aniObject = AbilityRuntime::CreateStsError(env, AbilityRuntime::AbilityErrorCode::ERROR_OK);
    ErrCode innerErrCode = ERR_OK;
    auto context = GeApplicationContext(env, aniObj);
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "nativeContextLong is nullptr");
        innerErrCode = AbilityRuntime::ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST;
        aniObject = AbilityRuntime::CreateStsError(env, innerErrCode, "applicationContext is already released.");
        AppExecFwk::AsyncCallback(env, call, aniObject, nullptr);
        return;
    }
    AppExecFwk::AsyncCallback(env, call, aniObject, nullptr);
    context->KillProcessBySelf(clearPageStack);
}

void EtsApplicationContextUtils::PreloadUIExtensionAbility(ani_env *env,
    ani_object aniObj, ani_object wantObj, ani_object call)
{
    TAG_LOGD(AAFwkTag::APPKIT, "PreloadUIExtensionAbility Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    AAFwk::Want want;
    if (!OHOS::AppExecFwk::UnwrapWant(env, wantObj, want)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse want failed");
        AppExecFwk::AsyncCallback(env, call, AbilityRuntime::CreateStsInvalidParamError(env,
            "Parse param want failed, want must be Want."), nullptr);
        return;
    }
    auto context = GeApplicationContext(env, aniObj);
    if (!context) {
        AppExecFwk::AsyncCallback(env, call, AbilityRuntime::CreateStsErrorByNativeErr(env,
            (int32_t)AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT), nullptr);
        return;
    }
    auto hostBundleName = context->GetBundleName();
    TAG_LOGD(AAFwkTag::APPKIT, "HostBundleName is %{public}s", hostBundleName.c_str());
    auto innerErrCode = AAFwk::AbilityManagerClient::GetInstance()->PreloadUIExtensionAbility(want, hostBundleName);
    if (innerErrCode == ERR_OK) {
        AppExecFwk::AsyncCallback(env, call, AbilityRuntime::CreateStsError(env,
            AbilityRuntime::AbilityErrorCode::ERROR_OK), nullptr);
    } else {
        TAG_LOGE(AAFwkTag::APPKIT, "OnPreloadUIExtensionAbility failed %{public}d", innerErrCode);
        AppExecFwk::AsyncCallback(env, call, AbilityRuntime::CreateStsErrorByNativeErr(env, innerErrCode), nullptr);
    }
}

void EtsApplicationContextUtils::SetSupportedProcessCacheSync(ani_env *env,
    ani_object aniObj, ani_boolean value)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetSupportedProcessCacheSync Call");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return;
    }
    auto applicationContext = GeApplicationContext(env, aniObj);
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INVALID_CONTEXT);
        return;
    }
    int32_t errCode = applicationContext->SetSupportedProcessCacheSelf(value);
    if (errCode == AAFwk::ERR_CAPABILITY_NOT_SUPPORT) {
        AbilityRuntime::EtsErrorUtil::ThrowError(env,
            AbilityRuntime::AbilityErrorCode::ERROR_CODE_CAPABILITY_NOT_SUPPORT);
    } else if (errCode != ERR_OK) {
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER);
    }
}

std::shared_ptr<ApplicationContext> EtsApplicationContextUtils::GeApplicationContext(ani_env *env, ani_object aniObj)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_class cls {};
    if ((status = env->FindClass(ETS_APPLICATION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return nullptr;
    }
    ani_field contextField = nullptr;
    if ((status = env->Class_FindField(cls, "nativeContext", &contextField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return nullptr;
    }
    ani_long nativeContextLong;
    if ((status = env->Object_GetField_Long(aniObj, contextField, &nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return nullptr;
    }
    auto weakContext = reinterpret_cast<std::weak_ptr<ApplicationContext>*>(nativeContextLong);
    return weakContext != nullptr ? weakContext->lock() : nullptr;
}

ani_object EtsApplicationContextUtils::SetApplicationContext(ani_env* aniEnv,
    const std::shared_ptr<ApplicationContext> &applicationContext)
{
    TAG_LOGD(AAFwkTag::APPKIT, "SetApplicationContext Call");
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniEnv");
        return nullptr;
    }
    if (applicationContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_class cls {};
    if ((status = aniEnv->FindClass(ETS_APPLICATION_CONTEXT_CLASS_NAME, &cls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return nullptr;
    }
    ani_method method {};
    if ((status = aniEnv->Class_FindMethod(cls, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return nullptr;
    }
    ani_object contextObj = nullptr;
    if ((status = aniEnv->Object_New(cls, method, &contextObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return nullptr;
    }
    ani_field field = nullptr;
    if ((status = aniEnv->Class_FindField(cls, "nativeContext", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return nullptr;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<ApplicationContext>(applicationContext);
    if (workContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "workContext nullptr");
        return nullptr;
    }
    ani_long nativeContextLong = (ani_long)workContext;
    if ((status = aniEnv->Object_SetField_Long(contextObj, field, nativeContextLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        delete workContext;
        workContext = nullptr;
        return nullptr;
    }
    return contextObj;
}

void EtsApplicationContextUtils::BindApplicationContextFunc(ani_env* aniEnv)
{
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null aniEnv");
        return;
    }
    ani_class contextClass = nullptr;
    ani_status status = ANI_ERROR;
    if ((status = aniEnv->FindClass(ETS_APPLICATION_CONTEXT_CLASS_NAME, &contextClass)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass ApplicationContext failed status: %{public}d", status);
        return;
    }
    std::array applicationContextFunctions = {
        ani_native_function {"setSupportedProcessCacheSync", "Z:V",
            reinterpret_cast<void *>(EtsApplicationContextUtils::SetSupportedProcessCacheSync)},
        ani_native_function {"nativekillAllProcessesSync", "ZLutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsApplicationContextUtils::killAllProcesses)},
        ani_native_function {"nativepreloadUIExtensionAbilitySync",
            "L@ohos/app/ability/Want/Want;Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsApplicationContextUtils::PreloadUIExtensionAbility)},
        ani_native_function {"nativegetRunningProcessInformation",
            "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsApplicationContextUtils::GetRunningProcessInformation)},
        ani_native_function {"nativeclearUpApplicationData",
            "Lutils/AbilityUtils/AsyncCallbackWrapper;:V",
            reinterpret_cast<void *>(EtsApplicationContextUtils::ClearUpApplicationData)},
        ani_native_function {"nativesetLanguage", "Lstd/core/String;:V",
            reinterpret_cast<void *>(EtsApplicationContextUtils::SetLanguage)},
        ani_native_function {"nativesetFontSizeScale", "D:V",
            reinterpret_cast<void *>(EtsApplicationContextUtils::SetFontSizeScale)},
        ani_native_function {"nativesetColorMode",
            "L@ohos/app/ability/ConfigurationConstant/ConfigurationConstant/ColorMode;:V",
            reinterpret_cast<void *>(EtsApplicationContextUtils::SetColorMode)},
        ani_native_function {"nativesetFont", "Lstd/core/String;:V",
            reinterpret_cast<void *>(EtsApplicationContextUtils::SetFont)},
        ani_native_function {"nativerestartApp", "L@ohos/app/ability/Want/Want;:V",
            reinterpret_cast<void *>(EtsApplicationContextUtils::RestartApp)},
    };
    status = aniEnv->Class_BindNativeMethods(contextClass, applicationContextFunctions.data(),
        applicationContextFunctions.size());
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Class_BindNativeMethods failed status: %{public}d", status);
    }
}

ani_object EtsApplicationContextUtils::CreateEtsApplicationContext(ani_env* aniEnv,
    const std::shared_ptr<ApplicationContext> &applicationContext)
{
    TAG_LOGD(AAFwkTag::APPKIT, "CreateEtsApplicationContext Call");
    if (applicationContext == nullptr || aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null applicationContext or aniEnv");
        return nullptr;
    }
    ani_object applicationContextObject = SetApplicationContext(aniEnv, applicationContext);
    ani_status status = ANI_ERROR;
    ani_ref applicationContextObjectRef = nullptr;
    if ((status = aniEnv->GlobalReference_Create(applicationContextObject, &applicationContextObjectRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "GlobalReference_Create failed status: %{public}d", status);
        return nullptr;
    }
    auto etsReference = std::make_shared<AbilityRuntime::ETSNativeReference>();
    etsReference->aniObj = applicationContextObject;
    AbilityRuntime::ApplicationContextManager::GetApplicationContextManager().SetEtsGlobalObject(etsReference);
    BindApplicationContextFunc(aniEnv);
    return applicationContextObject;
}
} // namespace AbilityRuntime
} // namespace OHOS