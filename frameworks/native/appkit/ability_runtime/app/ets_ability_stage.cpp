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
#include "ets_ability_stage.h"

#include <algorithm>
#include <cstring>
#include <exception>
#include <fstream>
#include <iterator>
#include <memory>
#include <string>

#include "ability_delegator_registry.h"
#include "ani_common_configuration.h"
#include "configuration_convertor.h"
#include "ets_ability_stage_context.h"
#include "freeze_util.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "ohos_application.h"
#include "startup_manager.h"

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AbilityRuntime {

AbilityStage *ETSAbilityStage::Create(
    const std::unique_ptr<Runtime>& runtime, const AppExecFwk::HapModuleInfo& hapModuleInfo)
{
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null runtime");
        return nullptr;
    }
    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "ETSAbilityStage::Create");

    auto& etsRuntime = static_cast<ETSRuntime&>(*runtime);
    // RegisterStopPreloadSoCallback(etsRuntime); // todo RegisterStopPreloadSoCallback实现
    return new (std::nothrow) ETSAbilityStage(etsRuntime);
}

ETSAbilityStage::ETSAbilityStage(ETSRuntime &etsRuntime) : etsRuntime_(etsRuntime)
{}

ETSAbilityStage::~ETSAbilityStage()
{
    TAG_LOGI(AAFwkTag::APPKIT, "destructor");
    auto context = GetContext();
    if (context != nullptr) {
        context->Unbind();
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::UIABILITY, "null env");
        return;
    }
    if (shellContextRef_ && shellContextRef_->aniRef) {
        env->GlobalReference_Delete(shellContextRef_->aniRef);
    }
}

void ETSAbilityStage::Init(const std::shared_ptr<Context> &context,
    const std::weak_ptr<AppExecFwk::OHOSApplication> application)
{
    AbilityStage::Init(context, application);
    SetShellContextRef(context);
}

void ETSAbilityStage::LoadModule(const AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::APPKIT, "AbilityStage::LoadModule");
    std::string srcPath(hapModuleInfo.name);
    std::string moduleName(hapModuleInfo.moduleName);
    moduleName.append("::").append("AbilityStage");

    srcPath.append("/");
    if (!hapModuleInfo.srcEntrance.empty()) {
        srcPath.append(hapModuleInfo.srcEntrance);
        auto pos = srcPath.rfind(".");
        if (pos != std::string::npos) {
            srcPath.erase(pos);
            srcPath.append(".abc");
        }
    }
    if (hapModuleInfo.srcEntrance.empty()) {
        return;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "entry path: %{public}s", hapModuleInfo.srcEntrance.c_str());
    bool esModule = hapModuleInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    etsAbilityStageObj_ = etsRuntime_.LoadModule(moduleName, srcPath,
        hapModuleInfo.hapPath, esModule, false, hapModuleInfo.srcEntrance);
    SetEtsAbilityStage();
}

void ETSAbilityStage::OnCreate(const AAFwk::Want &want) const
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "OnCreate called");
    AbilityStage::OnCreate(want);

    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "ETSAbilityStage::OnCreate begin");
    CallObjectMethod(false, "onCreate", ":V");
    FreezeUtil::GetInstance().AddAppLifecycleEvent(0, "ETSAbilityStage::OnCreate end");

    auto delegator = AppExecFwk::AbilityDelegatorRegistry::GetAbilityDelegator(AbilityRuntime::Runtime::Language::ETS);
    if (delegator) {
        delegator->PostPerformStageStart(CreateStageProperty());
    }
}

void ETSAbilityStage::OnDestroy() const
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnDestroy called");
    AbilityStage::OnDestroy();
    CallObjectMethod(false, "onDestroy", ":V");
}

std::string ETSAbilityStage::OnAcceptWant(const AAFwk::Want &want)
{
    return std::string();
}

std::string ETSAbilityStage::OnNewProcessRequest(const AAFwk::Want &want)
{
    return std::string();
}

void ETSAbilityStage::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    TAG_LOGD(AAFwkTag::APPKIT, "OnConfigurationUpdated called");
    AbilityStage::OnConfigurationUpdated(configuration);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "env nullptr");
        return;
    }
    auto application = application_.lock();
    if (application == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "application is null");
        return;
    }
    auto fullConfig = application->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::APPKIT, "null fullConfig");
        return;
    }
    ETSAbilityStageContext::ConfigurationUpdated(env, fullConfig);
    ani_object configObj = OHOS::AppExecFwk::WrapConfiguration(env, configuration);
    if (configObj == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null configObj");
        return;
    }
    CallObjectMethod(false, "onConfigurationUpdate", "L@ohos/app/ability/Configuration/Configuration;:V", configObj);
}

void ETSAbilityStage::OnMemoryLevel(int32_t level)
{
}

bool ETSAbilityStage::CallObjectMethod(bool withResult, const char *name, const char *signature, ...) const
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGD(AAFwkTag::ABILITY, "CallObjectMethod: name:%{public}s", name);
    if (etsAbilityStageObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "etsAbilityStageObj_ nullptr");
        return false;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "env nullptr");
        return false;
    }
    ani_status status = ANI_OK;
    ani_method method = nullptr;
    if ((status = env->Class_FindMethod(etsAbilityStageObj_->aniCls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "status: %{public}d", status);
        return false;
    }
    env->ResetError();
    if (withResult) {
        ani_boolean res = ANI_FALSE;
        va_list args;
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Boolean(etsAbilityStageObj_->aniObj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::ABILITY, "status: %{public}d", status);
            etsRuntime_.HandleUncaughtError();
        }
        va_end(args);
        return res;
    }
    va_list args;
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsAbilityStageObj_->aniObj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "status: %{public}d", status);
        etsRuntime_.HandleUncaughtError();
        return false;
    }
    va_end(args);
    return false;
}

std::shared_ptr<AppExecFwk::EtsDelegatorAbilityStageProperty> ETSAbilityStage::CreateStageProperty() const
{
    auto property = std::make_shared<AppExecFwk::EtsDelegatorAbilityStageProperty>();
    property->moduleName_ = GetHapModuleProp("name");
    property->srcEntrance_ = GetHapModuleProp("srcEntrance");
    return property;
}

std::string ETSAbilityStage::GetHapModuleProp(const std::string &propName) const
{
    auto context = GetContext();
    if (!context) {
        TAG_LOGE(AAFwkTag::APPKIT, "null context");
        return std::string();
    }
    auto hapModuleInfo = context->GetHapModuleInfo();
    if (!hapModuleInfo) {
        TAG_LOGE(AAFwkTag::APPKIT, "null hapModuleInfo");
        return std::string();
    }
    if (propName.compare("name") == 0) {
        return hapModuleInfo->name;
    }
    if (propName.compare("srcEntrance") == 0) {
        return hapModuleInfo->srcEntrance;
    }
    TAG_LOGD(AAFwkTag::APPKIT, "name = %{public}s", propName.c_str());
    return std::string();
}

void ETSAbilityStage::SetShellContextRef(std::shared_ptr<Context> context)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "context nullptr");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "env nullptr");
        return;
    }

    ani_object contextObj = ETSAbilityStageContext::CreateEtsAbilityStageContext(env, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "CreateEtsAbilityStageContext failed");
        return;
    }
    ani_ref contextGlobalRef = nullptr;
    ani_status status = ANI_OK;
    if ((status = env->GlobalReference_Create(contextObj, &contextGlobalRef)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "GlobalReference_Create failed status: %{public}d", status);
        return;
    }
    ani_ref* heapRef = new ani_ref(contextGlobalRef);
    context->Bind(heapRef);
    shellContextRef_ = std::make_shared<AppExecFwk::ETSNativeReference>();
    shellContextRef_->aniObj = reinterpret_cast<ani_object>(contextGlobalRef);
    shellContextRef_->aniRef = contextGlobalRef;
}

void ETSAbilityStage::SetEtsAbilityStage()
{
    if (!etsAbilityStageObj_) {
        TAG_LOGE(AAFwkTag::APPKIT, "etsAbilityStageObj_ null");
        return;
    }
    if (shellContextRef_ == nullptr || shellContextRef_->aniRef == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "shellContextRef_ or shellContextRef_->aniRef null");
        return;
    }
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITY, "env nullptr");
        return;
    }

    ani_status status = ANI_OK;
    if (env->Object_SetFieldByName_Ref(etsAbilityStageObj_->aniObj, "context", shellContextRef_->aniRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::ABILITY, "Object_SetFieldByName_Ref context failed");
    }
}

int32_t EtsAbilityStage::RunAutoStartupTask(const std::function<void()> &callback, std::shared_ptr<AAFwk::Want> want,
    bool &isAsyncCallback, const std::shared_ptr<Context> &stageContext, bool preAbilityStageLoad)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "RunAutoStartupTask, pre:%{public}d", preAbilityStageLoad);
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
    if (hapModuleInfo->moduleType != AppExecFwk::ModuleType::ENTRY &&
        hapModuleInfo->moduleType != AppExecFwk::ModuleType::FEATURE) {
        TAG_LOGD(AAFwkTag::APPKIT, "not entry module or feature module");
        return ERR_INVALID_VALUE;
    }
    if (hapModuleInfo->appStartup.empty()) {
        TAG_LOGD(AAFwkTag::APPKIT, "module no app startup config");
        return ERR_INVALID_VALUE;
    }
    if (!shellContextRef_) {
        SetShellContextRef(stageContext);
    }
    int32_t result = RegisterAppStartupTask(hapModuleInfo, want);
    if (result != ERR_OK) {
        return result;
    }
    return RunAutoStartupTaskInner(callback, want, isAsyncCallback, hapModuleInfo->name, preAbilityStageLoad);
}

int32_t EtsAbilityStage::RegisterAppStartupTask(std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo,
    std::shared_ptr<AAFwk::Want> want)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    if (isStartupTaskRegistered_) {
        TAG_LOGD(AAFwkTag::APPKIT, "app startup task already registered");
        return ERR_OK;
    }
    isStartupTaskRegistered_ = true;
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
    result = startupManager->RunLoadModuleStartupConfigTask(needRunAutoStartupTask, hapModuleInfo);
    if (result != ERR_OK) {
        return result;
    }
    if (!needRunAutoStartupTask) {
        return ERR_OK;
    }

    const std::string &configEntry = startupManager->GetPendingConfigEntry();
    if (!LoadEtsStartupConfig(configEntry, want, hapModuleInfo->moduleName, hapModuleInfo->moduleType)) {
        TAG_LOGE(AAFwkTag::APPKIT, "load ets appStartup config failed.");
        return ERR_INVALID_VALUE;
    }
    return RegisterEtsStartupTask(hapModuleInfo);
}

bool EtsAbilityStage::LoadEtsStartupConfig(const std::string &srcEntry, std::shared_ptr<AAFwk::Want> want,
    const std::string &moduleName, AppExecFwk::ModuleType moduleType)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    std::unique_ptr<AppExecFwk::ETSNativeReference> startupConfigEntry = LoadStartupConfigSrcEntry(srcEntry);
    if (startupConfigEntry == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupConfigEntry");
        return false;
    }
    auto env = etsRuntime_.GetAniEnv();
    std::shared_ptr<EtsStartupConfig> startupConfig = std::make_shared<EtsStartupConfig>(env);
    if (startupConfig == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupConfig");
        return false;
    }
    if (startupConfig->Init(startupConfigEntry->aniRef, want) != ERR_OK) {
        return false;
    }
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
        return false;
    }
    startupManager->SetModuleConfig(startupConfig, moduleName, moduleType == AppExecFwk::ModuleType::ENTRY);
    return true;
}

std::unique_ptr<AppExecFwk::ETSNativeReference> EtsAbilityStage::LoadStartupConfigSrcEntry(const std::string &srcEntry)
{
    HITRACE_METER_NAME(HITRACE_TAG_APP, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::APPKIT, "LoadStartupConfigSrcEntry call");
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

    std::string moduleName(hapModuleInfo->moduleName);
    std::string srcPath(moduleName + "/" + srcEntry);
    auto pos = srcPath.rfind('.');
    if (pos == std::string::npos) {
        return nullptr;
    }
    srcPath.erase(pos);
    srcPath.append(".abc");
    bool esModule = hapModuleInfo->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    std::unique_ptr<AppExecFwk::ETSNativeReference> etsCode =
        etsRuntime_.LoadModule(moduleName, srcPath, hapModuleInfo->hapPath, esModule, false, ""); // todo srcEntrance
    return etsCode;
}

int32_t EtsAbilityStage::RegisterEtsStartupTask(std::shared_ptr<AppExecFwk::HapModuleInfo> hapModuleInfo)
{
    std::shared_ptr<StartupManager> startupManager = DelayedSingleton<StartupManager>::GetInstance();
    if (startupManager == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null startupManager");
        return ERR_INVALID_VALUE;
    }
    if (hapModuleInfo == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "null hapModuleInfo");
        return ERR_INVALID_VALUE;
    }
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "shellContextRef_ null");
        return ERR_INVALID_VALUE;
    }
    const std::vector<StartupTaskInfo> startupTaskInfos = startupManager->GetStartupTaskInfos(hapModuleInfo->name);
    for (const auto& item : startupTaskInfos) {
        std::shared_ptr<EtsStartupTask> etsStartupTask;
        if (startupManager->EnableLazyLoadingAppStartupTasks()) {
            etsStartupTask = std::make_shared<EtsStartupTask>(item.name, etsRuntime_, item, shellContextRef_->aniRef);
        } else {
            // todo LoadEtsOhmUrl移到ets_startup_task中
            std::unique_ptr<AppExecFwk::ETSNativeReference> startupEtsRef = LoadEtsOhmUrl(
                item.srcEntry, item.ohmUrl, item.moduleName, item.hapPath, item.esModule);
            if (startupEtsRef == nullptr) {
                TAG_LOGE(AAFwkTag::APPKIT, "load ets appStartup tasks failed.");
                continue;
            }
            etsStartupTask = std::make_shared<EtsStartupTask>(item.name, etsRuntime_, startupEtsRef,
                shellContextRef_->aniRef);
        }

        etsStartupTask->SetDependencies(item.dependencies);
        etsStartupTask->SetIsExcludeFromAutoStart(item.excludeFromAutoStart);
        etsStartupTask->SetCallCreateOnMainThread(item.callCreateOnMainThread);
        etsStartupTask->SetWaitOnMainThread(item.waitOnMainThread);
        etsStartupTask->SetModuleName(item.moduleName);
        etsStartupTask->SetModuleType(item.moduleType);
        etsStartupTask->SetMatchRules(std::move(item.matchRules));
        etsStartupTask->SetPreAbilityStageLoad(item.preAbilityStageLoad);
        startupManager->RegisterAppStartupTask(item.name, etsStartupTask);
    }
    return ERR_OK;
}

std::unique_ptr<AppExecFwk::ETSNativeReference> EtsAbilityStage::LoadEtsOhmUrl(const std::string &srcEntry,
    const std::string &ohmUrl, const std::string &moduleName, const std::string &hapPath, bool esmodule)
{
    TAG_LOGD(AAFwkTag::APPKIT, "call");
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
    std::unique_ptr<AppExecFwk::ETSNativeReference> etsCode(
        etsRuntime_.LoadModule(moduleNameWithStartupTask, srcPath, hapPath, esmodule, false, ohmUrl));
    return etsCode;
}

}  // namespace AbilityRuntime
}  // namespace OHOS

ETS_EXPORT extern "C" OHOS::AbilityRuntime::AbilityStage *OHOS_ETS_Ability_Stage_Create(
    const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime,
    const OHOS::AppExecFwk::HapModuleInfo &hapModuleInfo)
{
    return OHOS::AbilityRuntime::ETSAbilityStage::Create(runtime, hapModuleInfo);
}