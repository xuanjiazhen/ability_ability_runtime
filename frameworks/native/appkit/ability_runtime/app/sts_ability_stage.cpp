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

namespace {
void RegisterStopPreloadSoCallback(STSRuntime& stsRuntime)
{
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
        return;
    }
    if (!startupManager->HasAppStartupConfig()) {
        // no app startup config, no need to register stop preload so callback.
        return;
    }
    stsRuntime.SetStopPreloadSoCallback([]()-> void {
        std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
        if (startupManager == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
            return;
        }
        startupManager->StopAutoPreloadSoTask();
    });
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

    SetJsAbilityStage(context, application);
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
    AbilityStage::OnConfigurationUpdated(configuration);
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "env nullptr");
        return;
    }

    ani_object configObj = OHOS::AppExecFwk::WrapConfiguration(env, configuration);

    CallObjectMethod(false, "onConfigurationUpdate", "L@ohos/app/ability/Configuration/Configuration;:V",
        &configuration);
}

void STSAbilityStage::OnMemoryLevel(int32_t level)
{
}

int32_t STSAbilityStage::RunAutoStartupTask(const std::function<void()> &callback, bool &isAsyncCallback,
    const std::shared_ptr<Context> &stageContext)
{
    TAG_LOGD(AAFwkTag::APPKIT, "RunAutoStartupTask called");
    isAsyncCallback = false;
    auto context = GetContext();
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return ERR_INVALID_VALUE;
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        TAG_LOGE(AAFwkTag::APPKIT, "null hapModuleInfo");
        return ERR_INVALID_VALUE;
    }
    if (hapModuleInfo->moduleType != AppExecFwk::ModuleType::ENTRY) {
        TAG_LOGD(AAFwkTag::APPKIT, "not entry module");
        return ERR_INVALID_VALUE;
    }
    if (hapModuleInfo->appStartup.empty()) {
        TAG_LOGD(AAFwkTag::APPKIT, "entry module no app startup config");
        return ERR_INVALID_VALUE;
    }

    SetJsAbilityStage(stageContext);

    int32_t result = RegisterAppStartupTask(hapModuleInfo);
    if (result != ERR_OK) {
        return result;
    }
    return RunAutoStartupTaskInner(callback, isAsyncCallback);
}

int32_t STSAbilityStage::RegisterAppStartupTask(const std::shared_ptr<AppExecFwk::HapModuleInfo>& hapModuleInfo)
{
   std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
        return ERR_INVALID_VALUE;
    }
    bool needRunAutoStartupTask = false;
    int32_t result = startupManager->LoadAppStartupTaskConfig(needRunAutoStartupTask);
    if (result != ERR_OK) {
        return result;
    }
    if (!needRunAutoStartupTask) {
        return ERR_OK;
    }

    //TODO dxp
    //stsRuntime_.UpdateModuleNameAndAssetPath(hapModuleInfo->moduleName);

    const std::string &configEntry = startupManager->GetPendingConfigEntry();
    if (!LoadEtsStartupConfig(configEntry)) {
        TAG_LOGE(AAFwkTag::APPKIT, "load js appStartup config failed.");
        return ERR_INVALID_VALUE;
    }

    const std::vector<StartupTaskInfo> &startupTaskInfos = startupManager->GetStartupTaskInfos();
    for (const auto& item : startupTaskInfos) {
        std::unique_ptr<STSNativeReference> startupStsRef = LoadEtsOhmUrl(
            item.srcEntry, item.ohmUrl, item.moduleName, item.hapPath, item.esModule);
        if (startupStsRef == nullptr) {
            TAG_LOGE(AAFwkTag::APPKIT, "load ets appStartup tasks failed.");
            continue;
        }

        auto etsStartupTask = std::make_shared<EtsStartupTask>(item.name, stsRuntime_, startupJsRef, shellContextRef_);
        etsStartupTask->SetDependencies(item.dependencies);
        etsStartupTask->SetIsExcludeFromAutoStart(item.excludeFromAutoStart);
        etsStartupTask->SetCallCreateOnMainThread(item.callCreateOnMainThread);
        etsStartupTask->SetWaitOnMainThread(item.waitOnMainThread);
        startupManager->RegisterAppStartupTask(item.name, etsStartupTask);
    }
    return ERR_OK;
}

int32_t STSAbilityStage::RunAutoStartupTaskInner(const std::function<void()> &callback, bool &isAsyncCallback)
{
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
        return ERR_INVALID_VALUE;
    }
    std::shared_ptr<StartupTaskManager> startupTaskManager = nullptr;
    int32_t result = startupManager->BuildAutoAppStartupTaskManager(startupTaskManager);
    if (result != ERR_OK) {
        return result;
    }
    result = startupTaskManager->Prepare();
    if (result != ERR_OK) {
        return result;
    }
    auto runAutoStartupCallback = std::make_shared<OnCompletedCallback>(
        [callback](const std::shared_ptr<StartupTaskResult> &) {
            TAG_LOGI(AAFwkTag::APPKIT, "RunAutoStartupCallback");
            callback();
        });
    result = startupTaskManager->Run(runAutoStartupCallback);
    if (result != ERR_OK) {
        isAsyncCallback = runAutoStartupCallback->IsCalled();
        return result;
    }
    isAsyncCallback = true;
    return ERR_OK;
}

std::unique_ptr<STSNativeReference> STSAbilityStage::LoadEtsOhmUrl(const std::string &srcEntry,
    const std::string &ohmUrl, const std::string &moduleName, const std::string &hapPath, bool esmodule)
{
    TAG_LOGD(AAFwkTag::APPKIT, "LoadEtsOhmUrl call");
    if (srcEntry.empty() && ohmUrl.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "srcEntry and ohmUrl empty");
        return nullptr;
    }

    std::string moduleNameWithStartupTask = moduleName + "::startupTask";
    std::string srcPath(moduleName + "/" + srcEntry);
    auto pos = srcPath.rfind('.');
    if (pos == std::string::npos) {
        return nullptr;
    }
    srcPath.erase(pos);
    srcPath.append(".abc");

    return stsRuntime_.LoadModule(moduleNameWithStartupTask, srcPath, hapPath, esmodule, false, ohmUrl);
}

std::unique_ptr<STSNativeReference> STSAbilityStage::LoadEtsSrcEntry(const std::string &srcEntry)
{
   TAG_LOGD(AAFwkTag::APPKIT, "LoadEtsSrcEntry call");
    if (srcEntry.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "srcEntry invalid");
        return nullptr;
    }
    auto context = GetContext();
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return nullptr;
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        TAG_LOGE(AAFwkTag::APPKIT, "null hapModuleInfo");
        return nullptr;
    }

    bool esmodule = hapModuleInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    std::string moduleName(hapModuleInfo->moduleName);
    std::string srcPath(moduleName + "/" + srcEntry);

    auto pos = srcPath.rfind('.');
    if (pos == std::string::npos) {
        return nullptr;
    }
    srcPath.erase(pos);
    srcPath.append(".abc");

    //TODO dxp srcEntry
    return stsRuntime_.LoadModule(moduleName, srcPath, hapModuleInfo->hapPath, esmodule);
}

bool STSAbilityStage::LoadEtsStartupConfig(const std::string &srcEntry)
{
    std::unique_ptr<STSNativeReference> startupConfigEntry = LoadEtsSrcEntry(srcEntry);
    if (startupConfigEntry == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupConfigEntry");
        return false;
    }
    auto env = stsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null env");
        return false;
    }
    std::shared_ptr<EtsStartupConfig> startupConfig = std::make_shared<EtsStartupConfig>(env);
    if (startupConfig == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupConfig");
        return false;
    }
    if (startupConfig->Init(startupConfigEntry) != ERR_OK) {
        return false;
    }
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
        return false;
    }
    startupManager->SetDefaultConfig(startupConfig);
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


void STSAbilityStage::SetJsAbilityStage(const std::shared_ptr<Context> &context,
    const std::weak_ptr<AppExecFwk::OHOSApplication> application)
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

    ani_object stageCtxObj = STSAbilityStageContext::CreateStsAbilityStageContext(env, context, application);
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

}  // namespace AbilityRuntime
}  // namespace OHOS
