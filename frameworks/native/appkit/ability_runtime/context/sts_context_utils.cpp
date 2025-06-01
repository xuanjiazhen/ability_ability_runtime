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

#include "sts_context_utils.h"

#include "ani_common_util.h"
#include "ani_enum_convert.h"
#include "application_context.h"
#include "application_context_manager.h"
#include "common_fun_ani.h"
#include "hilog_tag_wrapper.h"
#include "resourceManager.h"
#include "sts_error_utils.h"
#include "ani_common_util.h"
#include "ability_runtime_error_util.h"
#include "tokenid_kit.h"
#include "ipc_skeleton.h"

namespace OHOS {
namespace AbilityRuntime {
namespace ContextUtil {
namespace {
constexpr const char* AREA_MODE_ENUM_NAME = "L@ohos/app/ability/contextConstant/contextConstant/AreaMode;";
constexpr const char* CONTEXT_CLASS_NAME = "Lapplication/Context/Context;";
}
static std::weak_ptr<Context> context_;
void BindApplicationCtx(ani_env* aniEnv, ani_class contextClass, ani_object contextObj,
    void* applicationCtxRef)
{
    // bind parent context field:applicationContext
    ani_field applicationContextField;
    if (aniEnv->Class_FindField(contextClass, "applicationContext", &applicationContextField) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Class_FindField failed");
        return;
    }
    ani_ref applicationContextRef = reinterpret_cast<ani_ref>(applicationCtxRef);
    if (aniEnv->Object_SetField_Ref(contextObj, applicationContextField, applicationContextRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Ref failed");
        return;
    }
}

void BindApplicationInfo(ani_env* aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<Context> context)
{
    ani_field applicationInfoField;
    if (ANI_OK != aniEnv->Class_FindField(contextClass, "applicationInfo", &applicationInfoField)) {
        TAG_LOGE(AAFwkTag::APPKIT, "find applicationInfo failed");
        return;
    }
    auto appInfo = context->GetApplicationInfo();
    ani_object appInfoObj = AppExecFwk::CommonFunAni::ConvertApplicationInfo(aniEnv, *appInfo);
    if (aniEnv->Object_SetField_Ref(contextObj, applicationInfoField,
        reinterpret_cast<ani_ref>(appInfoObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Ref failed");
        return;
    }
}

void BindResourceManager(ani_env* aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<Context> context)
{
    ani_field resourceManagerField;
    if (ANI_OK != aniEnv->Class_FindField(contextClass, "resourceManager", &resourceManagerField)) {
        TAG_LOGE(AAFwkTag::APPKIT, "find resourceManager failed");
        return;
    }
    auto resourceManager = context->GetResourceManager();
    ani_object resourceMgrObj = Global::Resource::ResMgrAddon::CreateResMgr(aniEnv, "", resourceManager, context);
    if (aniEnv->Object_SetField_Ref(contextObj, resourceManagerField,
        reinterpret_cast<ani_ref>(resourceMgrObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Ref failed");
        return;
    }
}

void BindParentProperty(ani_env* aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<Context> context)
{
    BindApplicationInfo(aniEnv, contextClass, contextObj, context);
    BindResourceManager(aniEnv, contextClass, contextObj, context);

    // bind parent context property
    ani_field areaField;
    if (ANI_OK != aniEnv->Class_FindField(contextClass, "area", &areaField)) {
        TAG_LOGE(AAFwkTag::APPKIT, "find area failed");
        return;
    }
    auto area = context->GetArea();
    ani_enum_item areaModeItem {};
    OHOS::AAFwk::AniEnumConvertUtil::EnumConvertNativeToSts(aniEnv, AREA_MODE_ENUM_NAME, area, areaModeItem);

    if (aniEnv->Object_SetField_Ref(contextObj, areaField, (ani_ref)areaModeItem) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Int failed");
        return;
    }

    ani_field filesDirField;
    if (ANI_OK != aniEnv->Class_FindField(contextClass, "filesDir", &filesDirField)) {
        TAG_LOGE(AAFwkTag::APPKIT, "find filesDir failed");
        return;
    }
    auto filesDir = context->GetFilesDir();
    ani_string filesDir_string{};
    aniEnv->String_NewUTF8(filesDir.c_str(), filesDir.size(), &filesDir_string);
    if (aniEnv->Object_SetField_Ref(contextObj, filesDirField, reinterpret_cast<ani_ref>(filesDir_string)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Ref failed");
        return;
    }

    ani_field tempDirField;
    if (ANI_OK != aniEnv->Class_FindField(contextClass, "tempDir", &tempDirField)) {
        TAG_LOGE(AAFwkTag::APPKIT, "find find tempDir failed");
        return;
    }
    auto tempDir = context->GetTempDir();
    ani_string tempDir_string{};
    aniEnv->String_NewUTF8(tempDir.c_str(), tempDir.size(), &tempDir_string);
    if (aniEnv->Object_SetField_Ref(contextObj, tempDirField, reinterpret_cast<ani_ref>(tempDir_string)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Ref failed");
        return;
    }
}

void BindContextDir(ani_env* aniEnv, ani_class contextClass, ani_object contextObj,
    std::shared_ptr<Context> context)
{
    if (aniEnv == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "aniEnv or context is nullptr");
        return;
    }
    ani_status status = ANI_ERROR;
    ani_field preferencesDirField;
    if ((status = aniEnv->Class_FindField(contextClass, "preferencesDir", &preferencesDirField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "find preferencesDir failed, status: %{public}d", status);
        return;
    }
    auto preferencesDir = context->GetPreferencesDir();
    TAG_LOGI(AAFwkTag::APPKIT, "ani preferencesDir:%{public}s", preferencesDir.c_str());
    ani_string preferencesDirString{};
    aniEnv->String_NewUTF8(preferencesDir.c_str(), preferencesDir.size(), &preferencesDirString);
    if ((status = aniEnv->Object_SetField_Ref(contextObj, preferencesDirField,
        reinterpret_cast<ani_ref>(preferencesDirString))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Ref failed status: %{public}d", status);
        return;
    }

    ani_field databaseDirField;
    if ((status = aniEnv->Class_FindField(contextClass, "databaseDir", &databaseDirField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "find databaseDir failed status: %{public}d", status);
        return;
    }
    auto databaseDir = context->GetDatabaseDir();
    TAG_LOGI(AAFwkTag::APPKIT, "ani databaseDir:%{public}s", databaseDir.c_str());
    ani_string databaseDirString{};
    aniEnv->String_NewUTF8(databaseDir.c_str(), databaseDir.size(), &databaseDirString);
    if ((status = aniEnv->Object_SetField_Ref(contextObj, databaseDirField,
        reinterpret_cast<ani_ref>(databaseDirString))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Ref failed status: %{public}d", status);
        return;
    }

    ani_field cacheDirField;
    if ((status = aniEnv->Class_FindField(contextClass, "cacheDir", &cacheDirField)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "find cacheDir failed status: %{public}d", status);
        return;
    }
    auto cacheDir = context->GetCacheDir();
    TAG_LOGI(AAFwkTag::APPKIT, "ani cacheDir:%{public}s", cacheDir.c_str());
    ani_string cacheDirString{};
    aniEnv->String_NewUTF8(cacheDir.c_str(), cacheDir.size(), &cacheDirString);
    if ((status = aniEnv->Object_SetField_Ref(contextObj, cacheDirField,
        reinterpret_cast<ani_ref>(cacheDirString))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetField_Ref failed, status: %{public}d", status);
        return;
    }
}

bool SetHapModuleInfo(
    ani_env *env, ani_class cls, ani_object contextObj, const std::shared_ptr<OHOS::AbilityRuntime::Context> &context)
{
    if (env == nullptr || context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or context");
        return false;
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (hapModuleInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "hapModuleInfo is nullptr");
        return false;
    }
    ani_ref hapModuleInfoRef = AppExecFwk::CommonFunAni::ConvertHapModuleInfo(env, *hapModuleInfo);
    if (hapModuleInfoRef == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "hapModuleInfoRef is nullptr");
        return false;
    }
    ani_status status = ANI_OK;
    status = env->Object_SetPropertyByName_Ref(contextObj, "currentHapModuleInfo", hapModuleInfoRef);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Object_SetPropertyByName_Ref failed, status: %{public}d", status);
        return false;
    }
    return true;
}

void StsCreatContext(ani_env* aniEnv, ani_class contextClass, ani_object contextObj,
    void* applicationCtxRef, std::shared_ptr<Context> context)
{
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "aniEnv is nullptr");
        return;
    }
    BindApplicationCtx(aniEnv, contextClass, contextObj, applicationCtxRef);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context is nullptr");
        return;
    }
    if (!SetHapModuleInfo(aniEnv, contextClass, contextObj, context)) {
        TAG_LOGE(AAFwkTag::APPKIT, "SetHapModuleInfo fail");
        return;
    }
    context_ = context;
    BindParentProperty(aniEnv, contextClass, contextObj, context);
    BindContextDir(aniEnv, contextClass, contextObj, context);
}

ani_object CreatAniContext(ani_env* aniEnv, std::shared_ptr<Context> context)
{
    if (aniEnv == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "aniEnv is nullptr");
        return nullptr;
    }
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "context is nullptr");
        return nullptr;
    }
    ani_status status = ANI_ERROR;
    ani_ref undefRef;
    if ((status = aniEnv->GetUndefined(&undefRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "GetUndefined failed %{public}d", status);
        return nullptr;
    }
    ani_class contextClass = nullptr;
    if ((status = aniEnv->FindClass(CONTEXT_CLASS_NAME, &contextClass)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass ApplicationContext failed status: %{public}d", status);
        AbilityRuntime::ThrowStsInvalidParamError(aniEnv, "FindClass failed");
        return reinterpret_cast<ani_object>(undefRef);
    }
    ani_method method = nullptr;
    if ((status = aniEnv->Class_FindMethod(contextClass, "<ctor>", ":V", &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return reinterpret_cast<ani_object>(undefRef);
    }
    ani_object contextObj = nullptr;
    if ((status = aniEnv->Object_New(contextClass, method, &contextObj)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::UI_EXT, "status: %{public}d", status);
        return reinterpret_cast<ani_object>(undefRef);
    }
    BindParentProperty(aniEnv, contextClass, contextObj, context);
    BindContextDir(aniEnv, contextClass, contextObj, context);
    return contextObj;
}

bool CheckCallerIsSystemApp()
{
    auto selfToken = IPCSkeleton::GetSelfTokenID();
    if (!Security::AccessToken::TokenIdKit::IsSystemAppByFullTokenID(selfToken)) {
        return false;
    }
    return true;
}

ani_object CreateModuleResourceManagerSync([[maybe_unused]]ani_env *env, [[maybe_unused]]ani_object aniObj,
    ani_string bundleName, ani_string moduleName)
{
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is nullptr");
        return {};
    }
    std::string bundleName_ = "";
    AppExecFwk::AniStringToStdString(env, bundleName, bundleName_);
    std::string moduleName_ = "";
    AppExecFwk::AniStringToStdString(env, moduleName, moduleName_);
    auto context = context_.lock();
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return {};
    }
    if (!CheckCallerIsSystemApp()) {
        TAG_LOGE(AAFwkTag::APPKIT, "not system-app");
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_NOT_SYSTEM_APP);
        return {};
    }
    auto resourceManager = context->CreateModuleResourceManager(bundleName_, moduleName_);
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager");
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return {};
    }
    return Global::Resource::ResMgrAddon::CreateResMgr(env, "", resourceManager, context);
}

ani_object GetApplicationContextSync([[maybe_unused]]ani_env *env, [[maybe_unused]]ani_object aniObj)
{
    auto appContextObj = ApplicationContextManager::GetApplicationContextManager().GetStsGlobalObject(env);
    if (appContextObj != nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "appContextObj is not nullptr");
        return appContextObj->aniObj;
    }
    ThrowStsInvalidParamError(env, "appContextObj null");
    return {};
}

ani_object NativeCreateDisplayContext(ani_env *env, ani_object aniObj, ani_double displayId)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "NativeCreateDisplayContext");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is nullptr");
        return nullptr;
    }
#ifdef SUPPORT_GRAPHICS
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return nullptr;
    }
    ani_ref undefRef;
    ani_status status = ANI_ERROR;
    if ((status = env->GetUndefined(&undefRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "GetUndefined failed %{public}d", status);
        return nullptr;
    }
    if (displayId < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "displayId is invalid, less than 0");
        ThrowStsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        return reinterpret_cast<ani_object>(undefRef);
    }
    uint64_t validDisplayId = static_cast<uint64_t>(displayId);
    auto displayContext = context->CreateDisplayContext(validDisplayId);
    if (displayContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create displayContext");
        return reinterpret_cast<ani_object>(undefRef);
    }
    return CreatAniContext(env, displayContext);
#else
    ani_ref undefRef;
    if ((status = env->GetUndefined(&undefRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "GetUndefined failed %{public}d", status);
        return nullptr;
    }
    return reinterpret_cast<ani_object>(undefRef);
#endif
}

ani_object NativeCreateAreaModeContext(ani_env *env, ani_object aniObj, ani_object areaModeObj)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "NativeCreateAreaModeContext");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is nullptr");
        return nullptr;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return nullptr;
    }
    ani_ref undefRef;
    ani_status status = ANI_ERROR;
    if ((status = env->GetUndefined(&undefRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "GetUndefined failed %{public}d", status);
        return nullptr;
    }
    int areaMode = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvertStsToNative(env, areaModeObj, areaMode)) {
        TAG_LOGE(AAFwkTag::APPMGR, "param areaMode err");
        ThrowStsInvalidNumParametersError(env);
        return nullptr;
    }
    auto areaContext = context->CreateAreaModeContext(areaMode);
    if (areaContext == nullptr) {
        ThrowStsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create areaContext");
        return reinterpret_cast<ani_object>(undefRef);
    }
    return CreatAniContext(env, areaContext);
}

ani_object NativeCreateSystemHspModuleResourceManager(ani_env *env, ani_object aniObj,
    ani_string bundleNameObj, ani_string moduleNameObj)
{
    TAG_LOGD(AAFwkTag::UIABILITY, "NativeGetGroupDir");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "env is nullptr");
        return nullptr;
    }
    ani_ref undefRef;
    ani_status status = ANI_ERROR;
    if ((status = env->GetUndefined(&undefRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "GetUndefined failed %{public}d", status);
        return nullptr;
    }
    auto context = context_.lock();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return reinterpret_cast<ani_object>(undefRef);
    }
    std::string bundleName = "";
    if (!AppExecFwk::AniStringToStdString(env, bundleNameObj, bundleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse bundleName failed");
        ThrowStsInvalidParamError(env, "Parse param bundleName failed, bundleName must be string.");
        return reinterpret_cast<ani_object>(undefRef);
    }
    std::string moduleName = "";
    if (!AppExecFwk::AniStringToStdString(env, moduleNameObj, moduleName)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Parse moduleName failed");
        ThrowStsInvalidParamError(env, "Parse param moduleName failed, moduleName must be string.");
        return reinterpret_cast<ani_object>(undefRef);
    }
    std::shared_ptr<Global::Resource::ResourceManager> resourceManager = nullptr;
    int32_t retCode = context->CreateSystemHspModuleResourceManager(bundleName, moduleName, resourceManager);
    if (resourceManager == nullptr && retCode == ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SYSTEM_HSP) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager");
        ThrowStsError(env, ERR_ABILITY_RUNTIME_EXTERNAL_NOT_SYSTEM_HSP);
        return reinterpret_cast<ani_object>(undefRef);
    }
    if (resourceManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null resourceManager");
        ThrowStsError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return reinterpret_cast<ani_object>(undefRef);
    }
    return Global::Resource::ResMgrAddon::CreateResMgr(env, "", resourceManager, context);
}
}
} // namespace AbilityRuntime
} // namespace OHOS
