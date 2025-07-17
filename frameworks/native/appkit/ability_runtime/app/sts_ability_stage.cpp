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

#include "sts_ability_stage.h"
#include "ability_delegator_registry.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "configuration_convertor.h"
#include "sts_ability_stage_context.h"
#include "ani_common_configuration.h"
#include "ohos_application.h"
#include "startup_manager.h"
#include "hitrace_meter.h"
#include <algorithm>
#include <cstring>
#include <exception>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>
#include "ani_enum_convert.h"

namespace OHOS {
namespace AbilityRuntime {
constexpr const char* PROFILE_FILE_PREFIX = "$profile:";
constexpr const char* STARTUP_TASKS = "startupTasks";
constexpr const char* NAME = "name";
constexpr const char* SRC_ENTRY = "srcEntry";
constexpr const char* DEPENDENCIES = "dependencies";
constexpr const char* EXCLUDE_FROM_AUTO_START = "excludeFromAutoStart";
constexpr const char* RUN_ON_THREAD = "runOnThread";
constexpr const char* WAIT_ON_MAIN_THREAD = "waitOnMainThread";
constexpr const char* CONFIG_ENTRY = "configEntry";
constexpr const char *TASKPOOL = "taskPool";
constexpr const char *TASKPOOL_LOWER = "taskpool";
constexpr const char* ABILITY_STAGE_CLASS_NAME = "L@ohos/app/ability/AbilityStage/AbilityStage;";

namespace {
void RegisterStopPreloadSoCallback(STSRuntime& stsRuntime)
{
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
        return;
    }
}

void OnPrepareTerminationPromiseCallback(ani_env* env, ani_object aniObj, ani_object dataObj)
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnDestroyPromiseCallback called");

    if (env == nullptr || aniObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env or null aniObj");
        return;
    }
    ani_long callbackPoint = 0;
    ani_status status = ANI_ERROR;
    if ((status = env->Object_GetFieldByName_Long(aniObj, "prepareTerminationCallbackPoint", &callbackPoint)) !=
        ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status: %{public}d", status);
        return;
    }
    auto *callbackInfo =
        reinterpret_cast<AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *>(
            callbackPoint);
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null callbackInfo");
        return;
    }
    ani_boolean isUndefined = false;
    if ((status = env->Reference_IsUndefined(dataObj, &isUndefined)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to check undefined status : %{public}d", status);
        return;
    }
    if (isUndefined) {
        TAG_LOGE(AAFwkTag::APPKIT, "onPrepareTermination unimplemented");
        return;
    }
    ani_int result = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvertStsToNative(env, dataObj, result)) {
        TAG_LOGE(AAFwkTag::APPKIT, "EnumConvertStsToNative param err");
        return;
    }
    AppExecFwk::OnPrepareTerminationResult prepareTerminationResult = { result, true };
    callbackInfo->Call(prepareTerminationResult);
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult>::Destroy(callbackInfo);

    if ((status = env->Object_SetFieldByName_Long(aniObj, "prepareTerminationCallbackPoint",
        static_cast<ani_long>(0))) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status : %{public}d", status);
        return;
    }
}
} // namespace

bool STSAbilityStage::UseCommonChunk(const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    for (auto &md: hapModuleInfo.metadata) {
        if (md.name == "USE_COMMON_CHUNK") {
            if (md.value != "true") {
                TAG_LOGE(AAFwkTag::APPKIT, "USE_COMMON_CHUNK = %s{public}s", md.value.c_str());
                return false;
            }
            return true;
        }
    }
    return false;
}

bool STSAbilityStage::BindNativeMethods()
{
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return false;
    }
    std::array functions = {
        ani_native_function{
            "nativeOnPrepareTerminationCallback", ":V", reinterpret_cast<void *>(OnPrepareTerminationPromiseCallback)},
    };
    ani_class cls {};
    ani_status status = env->FindClass(ABILITY_STAGE_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "FindClass failed status: %{public}d", status);
        return false;
    }
    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "Class_BindNativeMethods status: %{public}d", status);
        return false;
    }
    return true;
}

std::shared_ptr<AbilityStage> STSAbilityStage::Create(
    const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime");
        return nullptr;
    }
    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "STSAbilityStage::Create");

    auto& stsRuntime = static_cast<STSRuntime&>(*runtime);

    std::string srcPath(hapModuleInfo.name);
    std::string moduleName(hapModuleInfo.moduleName);
    moduleName.append("::").append("AbilityStage");
    bool commonChunkFlag = UseCommonChunk(hapModuleInfo);

    RegisterStopPreloadSoCallback(stsRuntime);

    srcPath.append("/");
    if (!hapModuleInfo.srcEntrance.empty()) {
        srcPath.append(hapModuleInfo.srcEntrance);
        auto pos = srcPath.rfind(".");
        if (pos != std::string::npos) {
            srcPath.erase(pos);
            srcPath.append(".abc");
        }
    }

    std::unique_ptr<STSNativeReference> moduleObj;
    if (!hapModuleInfo.srcEntrance.empty()) {
        moduleObj = stsRuntime.LoadModule(moduleName, srcPath, hapModuleInfo.hapPath,
            hapModuleInfo.compileMode == AppExecFwk::CompileMode::ES_MODULE, commonChunkFlag,
            hapModuleInfo.srcEntrance);
    }
    return std::make_shared<STSAbilityStage>(stsRuntime, std::move(moduleObj));
}

STSAbilityStage::STSAbilityStage(STSRuntime & stsRuntime, std::unique_ptr<STSNativeReference>&& stsAbilityStageObj)
    : stsRuntime_(stsRuntime), stsAbilityStageObj_(std::move(stsAbilityStageObj))
{}

STSAbilityStage::~STSAbilityStage()
{
    TAG_LOGI(AAFwkTag::APPKIT, "destructor");
    auto context = GetContext();
    if (context != nullptr) {
        context->Unbind();
    }
}

void STSAbilityStage::Init(const std::shared_ptr<Context> &context,
    const std::weak_ptr<AppExecFwk::OHOSApplication> application)
{
    AbilityStage::Init(context, application);
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return;
    }
    if (!stsAbilityStageObj_) {
        TAG_LOGE(AAFwkTag::APPKIT, "null stage");
        return;
    }
    if(!BindNativeMethods()) {
        TAG_LOGE(AAFwkTag::APPKIT, "BindNativeMethods failed");
    }
    SetJsAbilityStage(context);
}

void STSAbilityStage::OnCreate(const AAFwk::Want &want) const
{
    AbilityStage::OnCreate(want);

    CallObjectMethod(false, "onCreate", ":V");

    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "STSAbilityStage::OnCreate end");
    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::STS);
    if (delegator) {
        delegator->PostPerformStageStart(CreateStageProperty());
    }
}

void STSAbilityStage::OnDestroy() const
{
    AbilityStage::OnDestroy();
    CallObjectMethod(false, "onDestroy", ":V");
}

std::string STSAbilityStage::OnAcceptWant(const AAFwk::Want &want)
{
    return std::string();
}

std::string STSAbilityStage::OnNewProcessRequest(const AAFwk::Want &want)
{
    return std::string();
}

void STSAbilityStage::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnConfigurationUpdated called");
    AbilityStage::OnConfigurationUpdated(configuration);
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "env nullptr");
        return;
    }
    ani_object configObj = OHOS::AppExecFwk::WrapConfiguration(env, configuration);
    if (configObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null configObj");
        return;
    }
    CallObjectMethod(false, "onConfigurationUpdate", "L@ohos/app/ability/Configuration/Configuration;:V", configObj);
}

void STSAbilityStage::OnMemoryLevel(int32_t level)
{
}

int32_t STSAbilityStage::RunAutoStartupTask(const std::function<void()> &callback, bool &isAsyncCallback,
    const std::shared_ptr<Context> &stageContext)
{
    isAsyncCallback = false;
    return RunAutoStartupTaskInner(callback, isAsyncCallback);
}

int32_t STSAbilityStage::RegisterAppStartupTask(const std::shared_ptr<AppExecFwk::HapModuleInfo>& hapModuleInfo)
{
    return ERR_OK;
}

int32_t STSAbilityStage::RunAutoStartupTaskInner(const std::function<void()> &callback, bool &isAsyncCallback)
{
    return ERR_OK;
}

std::unique_ptr<STSNativeReference> STSAbilityStage::LoadJsOhmUrl(const std::string &srcEntry,
    const std::string &ohmUrl, const std::string &moduleName, const std::string &hapPath, bool esmodule)
{
    return nullptr;
}

std::unique_ptr<STSNativeReference> STSAbilityStage::LoadJsSrcEntry(const std::string &srcEntry)
{
    return nullptr;
}

bool STSAbilityStage::LoadJsStartupConfig(const std::string &srcEntry)
{
    return true;
}

bool STSAbilityStage::CallObjectMethod(bool withResult, const char *name, const char *signature, ...) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    if (stsAbilityStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "stsAbilityStageObj_ nullptr");
        return false;
    }

    auto env = stsRuntime_.GetAniEnv();
    STSAbilityStageContext::ResetEnv(env);
    ani_status status = ANI_OK;
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(stsAbilityStageObj_->aniCls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "status: %{public}d", status);
        return false;
    }
    env->ResetError();
    if (withResult) {
        ani_boolean res = 0;
        va_list args;
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Boolean(stsAbilityStageObj_->aniObj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::ABILITY, "status: %{public}d", status);
            stsRuntime_.HandleUncaughtError();
        }
        va_end(args);
        return res;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(stsAbilityStageObj_->aniObj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "status: %{public}d", status);
        stsRuntime_.HandleUncaughtError();
        return false;
    }
    va_end(args);
    return false;
}

std::shared_ptr<AppExecFwk::ETSDelegatorAbilityStageProperty> STSAbilityStage::CreateStageProperty() const
{
    auto property = std::make_shared<AppExecFwk::ETSDelegatorAbilityStageProperty>();
    return property;
}

std::string STSAbilityStage::GetHapModuleProp(const std::string &propName) const
{
    return std::string();
}


void STSAbilityStage::SetJsAbilityStage(const std::shared_ptr<Context> &context)
{
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "context nullptr");
        return;
    }

    if (!stsAbilityStageObj_) {
        TAG_LOGE(AAFwkTag::APPKIT, "Not found AbilityStage.js");
        return;
    }

    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "env nullptr");
        return;
    }

    STSAbilityStageContext::ResetEnv(env);

    ani_object stageCtxObj = STSAbilityStageContext::CreateStsAbilityStageContext(env, context);
    if (stageCtxObj == nullptr) {
        STSAbilityStageContext::ResetEnv(env);
        TAG_LOGE(AAFwkTag::ABILITY, "CreateStsAbilityStageContext failed");
        return;
    }

    ani_status status = ANI_OK;
    ani_field contextField;
    status = env->Class_FindField(stsAbilityStageObj_->aniCls, "context", &contextField);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "Class_GetField context failed");
        STSAbilityStageContext::ResetEnv(env);
        return;
    }
    ani_ref stageCtxObjRef = nullptr;
    if (env->GlobalReference_Create(stageCtxObj, &stageCtxObjRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "GlobalReference_Create stageCtxObj failed");
        return;
    }
    if (env->Object_SetField_Ref(stsAbilityStageObj_->aniObj, contextField, stageCtxObjRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "Object_SetField_Ref stageCtxObj failed");
        STSAbilityStageContext::ResetEnv(env);
    }
}

bool STSAbilityStage::OnPrepareTerminate(
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *callbackInfo,
    bool &isAsync) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnPrepareTerminate called");
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "callbackInfo nullptr");
        return false;
    }
    if (CallOnPrepareTerminateAsync(callbackInfo, isAsync)) {
        TAG_LOGI(AAFwkTag::APPKIT, "onPrepareTerminationAsync is implemented");
        return true;
    }
    isAsync = false;
    return CallOnPrepareTerminate(callbackInfo);
}

bool STSAbilityStage::CallOnPrepareTerminateAsync(
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *callbackInfo,
    bool &isAsync) const
{
    isAsync = false;
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "callbackInfo nullptr");
        return false;
    }
    isAsync = CallObjectMethod(true, "callOnPrepareTermination", ":Z");
    return isAsync;
}

ani_object STSAbilityStage::CallObjectMethod(const char *name, const char *signature, ...) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGD(AAFwkTag::APPKIT, "StsUIAbility call sts, name: %{public}s", name);
    if (stsAbilityStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null stsAbilityObj");
        return nullptr;
    }
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return nullptr;
    }
    auto obj = stsAbilityStageObj_->aniObj;
    auto cls = stsAbilityStageObj_->aniCls;
    ani_status status = ANI_ERROR;

    ani_method method {};
    if ((status = env->Class_FindMethod(cls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status : %{public}d", status);
        env->ResetError();
        return nullptr;
    }
    ani_ref res {};
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Ref(obj, method, &res, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "status : %{public}d", status);
        stsRuntime_.HandleUncaughtError();
        return nullptr;
    }
    va_end(args);
    return reinterpret_cast<ani_object>(res);
}

bool STSAbilityStage::CallOnPrepareTerminate(
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult> *callbackInfo) const
{
    TAG_LOGD(AAFwkTag::APPKIT, "CallOnPrepareTerminate call");
    if (callbackInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "callbackInfo nullptr");
        return false;
    }
    ani_env *env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return false;
    }
    ani_object resObj =
        CallObjectMethod("onPrepareTermination", ":L@ohos/app/ability/AbilityConstant/PrepareTermination;");
    if (resObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "onPrepareTermination unimplemented");
        return false;
    }
    ani_int result = 0;
    if (!AAFwk::AniEnumConvertUtil::EnumConvertStsToNative(env, resObj, result)) {
        TAG_LOGE(AAFwkTag::APPKIT, "EnumConvertStsToNative param err");
        return false;
    }
    AppExecFwk::OnPrepareTerminationResult prepareTerminationResult = { result, true };
    callbackInfo->Call(prepareTerminationResult);
    AppExecFwk::AbilityTransactionCallbackInfo<AppExecFwk::OnPrepareTerminationResult>::Destroy(callbackInfo);
    return true;
}

}  // namespace AbilityRuntime
}  // namespace OHOS
