/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "sts_ui_extension_base.h"

#include <type_traits>
#include <vector>

#include "ability_info.h"
#include "ability_manager_client.h"
#include "array_wrapper.h"
#include "configuration_utils.h"
#include "connection_manager.h"
#include "context.h"
#include "hilog_tag_wrapper.h"
#include "hilog_wrapper.h"
#include "hitrace_meter.h"
#include "insight_intent_executor_mgr.h"
#include "int_wrapper.h"
#include "js_data_struct_converter.h"
#include "sts_extension_common.h"
#include "js_extension_context.h"
#include "sts_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_content_session.h"

#include "string_wrapper.h"
#include "ui_extension_window_command.h"
#include "want_params_wrapper.h"
#include "application_configuration_manager.h"
#include "ohos_application.h"

namespace OHOS {
namespace AbilityRuntime {

STSUIExtensionBase::STSUIExtensionBase(const std::unique_ptr<Runtime> &runtime)
    : stsRuntime_(static_cast<STSRuntime&>(*runtime))
{
    abilityResultListeners_ = std::make_shared<AbilityResultListeners>();
}

STSUIExtensionBase::~STSUIExtensionBase()
{
    TAG_LOGD(AAFwkTag::UI_EXT, "destructor");
    contentSessions_.clear();
}

std::shared_ptr<ExtensionCommon> STSUIExtensionBase::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null abilityInfo");
        return nullptr;
    }
    if (abilityInfo_->srcEntrance.empty()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "empty abilityInfo srcEntrance");
        return nullptr;
    }

    RegisterAbilityConfigUpdateCallback();

    if (record != nullptr) {
        token_ = record->GetToken();
    }
    std::string srcPath(abilityInfo_->moduleName + "/");
    srcPath.append(abilityInfo_->srcEntrance);
    srcPath.erase(srcPath.rfind('.'));
    srcPath.append(".abc");

    std::string moduleName(abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);

    stsObj_ = stsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE, false,
        abilityInfo_->srcEntrance);
    if (stsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null stsObj_");
        return nullptr;
    }

    BindContext();
    handler_ = handler;
    RegisterDisplayInfoChangedListener();
    return StsExtensionCommon::Create(stsRuntime_, static_cast<STSNativeReference&>(*stsObj_), shellContextRef_);
}

void STSUIExtensionBase::RegisterAbilityConfigUpdateCallback()
{
    auto uiExtensionAbility = std::static_pointer_cast<STSUIExtensionBase>(shared_from_this());
    std::weak_ptr<STSUIExtensionBase> abilityWptr = uiExtensionAbility;
    context_->RegisterAbilityConfigUpdateCallback(
        [abilityWptr, abilityContext = context_](AppExecFwk::Configuration &config) {
        std::shared_ptr<STSUIExtensionBase> abilitySptr = abilityWptr.lock();
        if (abilitySptr == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilitySptr");
            return;
        }
        if (abilityContext == nullptr || abilityContext->GetAbilityInfo() == nullptr) {
            TAG_LOGE(AAFwkTag::UIABILITY, "null abilityContext or null GetAbilityInfo");
            return;
        }
        if (abilityContext->GetAbilityConfiguration() == nullptr) {
            auto abilityModuleContext = abilityContext->CreateModuleContext(
                abilityContext->GetAbilityInfo()->moduleName);
            if (abilityModuleContext == nullptr) {
                TAG_LOGE(AAFwkTag::UIABILITY, "null abilityModuleContext");
                return;
            }
            auto abilityResourceMgr = abilityModuleContext->GetResourceManager();
            abilityContext->SetAbilityResourceManager(abilityResourceMgr);
            AbilityRuntime::ApplicationConfigurationManager::GetInstance().
                AddIgnoreContext(abilityContext, abilityResourceMgr);
            TAG_LOGD(AAFwkTag::UIABILITY, "%{public}zu",
                AbilityRuntime::ApplicationConfigurationManager::GetInstance().GetIgnoreContext().size());
        }
        abilityContext->SetAbilityConfiguration(config);
        if (config.GetItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE).
            compare(AppExecFwk::ConfigurationInner::COLOR_MODE_AUTO) == 0) {
            config.AddItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE,
                ApplicationConfigurationManager::GetInstance().GetColorMode());

            if (AbilityRuntime::ApplicationConfigurationManager::GetInstance().
                GetColorModeSetLevel() > AbilityRuntime::SetLevel::System) {
                config.AddItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP,
                    AppExecFwk::ConfigurationInner::IS_SET_BY_APP);
            }
            abilityContext->GetAbilityConfiguration()->
                RemoveItem(AAFwk::GlobalConfigurationKey::SYSTEM_COLORMODE);
            abilityContext->GetAbilityConfiguration()->
                RemoveItem(AAFwk::GlobalConfigurationKey::COLORMODE_IS_SET_BY_APP);
        }

        abilitySptr->OnAbilityConfigurationUpdated(config);
    });
}

void STSUIExtensionBase::BindContext()
{
}

void STSUIExtensionBase::OnStart(
    const AAFwk::Want &want, AAFwk::LaunchParam &launchParam, sptr<AAFwk::SessionInfo> sessionInfo)
{}

void STSUIExtensionBase::OnStop()
{
}

void STSUIExtensionBase::OnStop(AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
}

void STSUIExtensionBase::OnStopCallBack()
{
}

void STSUIExtensionBase::OnCommandWindow(
    const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{}

void STSUIExtensionBase::ForegroundWindowInitInsightIntentExecutorInfo(const AAFwk::Want &want,
    const sptr<AAFwk::SessionInfo> &sessionInfo, InsightIntentExecutorInfo &executorInfo)
{}

bool STSUIExtensionBase::ForegroundWindowWithInsightIntent(const AAFwk::Want &want,
    const sptr<AAFwk::SessionInfo> &sessionInfo, bool needForeground)
{
    return true;
}

void STSUIExtensionBase::ExecuteInsightIntentDone(uint64_t intentId, const InsightIntentExecuteResult &result)
{
}

void STSUIExtensionBase::PostInsightIntentExecuted(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const AppExecFwk::InsightIntentExecuteResult &result, bool needForeground)
{
}

void STSUIExtensionBase::OnCommandWindowDone(const sptr<AAFwk::SessionInfo> &sessionInfo, AAFwk::WindowCommand winCmd)
{
}

void STSUIExtensionBase::OnInsightIntentExecuteDone(const sptr<AAFwk::SessionInfo> &sessionInfo,
    const AppExecFwk::InsightIntentExecuteResult &result)
{
}

void STSUIExtensionBase::OnCommand(const AAFwk::Want &want, bool restart, int32_t startId)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
}

void STSUIExtensionBase::OnForeground(const Want &want, sptr<AAFwk::SessionInfo> sessionInfo)
{
}

void STSUIExtensionBase::OnBackground()
{

}

bool STSUIExtensionBase::CallStsOnSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo,
    const sptr<Rosen::Window> &uiWindow, const uint64_t &uiExtensionComponentId)
{
    return true;
}

sptr<Rosen::WindowOption> STSUIExtensionBase::CreateWindowOption(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    auto option = sptr<Rosen::WindowOption>::MakeSptr();
    if (option == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "make option failed");
        return nullptr;
    }

    option->SetWindowName(context_->GetBundleName() + context_->GetAbilityInfo()->name);
    option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_UI_EXTENSION);
    option->SetWindowSessionType(Rosen::WindowSessionType::EXTENSION_SESSION);
    option->SetParentId(sessionInfo->hostWindowId);
    option->SetRealParentId(sessionInfo->realHostWindowId);
    option->SetParentWindowType(static_cast<Rosen::WindowType>(sessionInfo->parentWindowType));
    option->SetUIExtensionUsage(static_cast<uint32_t>(sessionInfo->uiExtensionUsage));
    option->SetDensity(sessionInfo->density);
    option->SetIsDensityFollowHost(sessionInfo->isDensityFollowHost);
    if (context_->isNotAllow != -1) {
        bool isNotAllow = context_->isNotAllow == 1 ? true : false;
        TAG_LOGD(AAFwkTag::UI_EXT, "isNotAllow: %{public}d", isNotAllow);
        option->SetConstrainedModal(isNotAllow);
    }
    return option;
}

bool STSUIExtensionBase::HandleSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    if (sessionInfo == nullptr || sessionInfo->uiExtensionComponentId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return false;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64 ", element: %{public}s",
        sessionInfo->uiExtensionComponentId, want.GetElement().GetURI().c_str());
    if (sessionInfo == nullptr || sessionInfo->uiExtensionComponentId == 0) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return false;
    }
    std::shared_ptr<AAFwk::Want> sharedWant = std::make_shared<AAFwk::Want>(want);
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        if (context_ == nullptr || context_->GetAbilityInfo() == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null context");
            return false;
        }
        auto option = CreateWindowOption(sessionInfo);
        if (option == nullptr) {
            return false;
        }
        sptr<Rosen::Window> uiWindow;
        {
            HITRACE_METER_NAME(HITRACE_TAG_APP, "Rosen::Window::Create");
            option->SetDisplayId(sessionInfo->displayId);
            uiWindow = Rosen::Window::Create(option, context_, sessionInfo->sessionToken);
        }
        if (uiWindow == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow");
            return false;
        }
        uiWindow->UpdateExtensionConfig(sharedWant);
        if (!CallStsOnSessionCreate(*sharedWant, sessionInfo, uiWindow, componentId)) {
            return false;
        }
        uiWindowMap_[componentId] = uiWindow;
#ifdef SUPPORT_GRAPHICS
        if (context_->GetWindow() == nullptr) {
            context_->SetWindow(uiWindow);
        }
#endif // SUPPORT_GRAPHICS
    } else {
        auto uiWindow = uiWindowMap_[componentId];
        if (uiWindow == nullptr) {
            TAG_LOGE(AAFwkTag::UI_EXT, "null uiWindow");
            return false;
        }
        uiWindow->UpdateExtensionConfig(sharedWant);
    }
    return true;
}

void STSUIExtensionBase::ForegroundWindow(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (!HandleSessionCreate(want, sessionInfo)) {
        TAG_LOGE(AAFwkTag::UI_EXT, "HandleSessionCreate failed");
        return;
    }
    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64, sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    auto &uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        HITRACE_METER_NAME(HITRACE_TAG_APP, "Rosen::Window::show");
        uiWindow->Show();
        foregroundWindows_.emplace(componentId);
    }
}

void STSUIExtensionBase::BackgroundWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "Invalid sessionInfo");
        return;
    }

    TAG_LOGD(AAFwkTag::UI_EXT, "UIExtension component id: %{public}" PRId64, sessionInfo->uiExtensionComponentId);
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "not find uiWindow");
        return;
    }
    auto &uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        uiWindow->Hide();
        foregroundWindows_.erase(componentId);
    }
}

void STSUIExtensionBase::DestroyWindow(const sptr<AAFwk::SessionInfo> &sessionInfo)
{
}

void STSUIExtensionBase::OnConfigurationUpdated(const AppExecFwk::Configuration &configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }

    auto abilityConfig = context_->GetAbilityConfiguration();
    auto configUtils = std::make_shared<ConfigurationUtils>();

    if (abilityConfig != nullptr) {
        auto newConfig = configUtils->UpdateGlobalConfig(configuration, context_->GetConfiguration(),
            abilityConfig, context_->GetResourceManager());
        if (newConfig.GetItemSize() == 0) {
            return;
        }
        if (context_->GetWindow()) {
            TAG_LOGI(AAFwkTag::UIABILITY, "newConfig: %{public}s", newConfig.GetName().c_str());
            auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(newConfig);
            context_->GetWindow()->UpdateConfigurationForSpecified(diffConfiguration, context_->GetResourceManager());
        }
    } else {
        auto configUtils = std::make_shared<ConfigurationUtils>();
        configUtils->UpdateGlobalConfig(configuration, context_->GetConfiguration(), context_->GetResourceManager());
    }

    ConfigurationUpdated();
}

void STSUIExtensionBase::OnAbilityConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::UI_EXT, "called");

    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }

    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateAbilityConfig(configuration, context_->GetResourceManager());

    if (context_->GetWindow()) {
        TAG_LOGI(AAFwkTag::UIABILITY, "newConfig: %{public}s", configuration.GetName().c_str());
        auto diffConfiguration = std::make_shared<AppExecFwk::Configuration>(configuration);
        context_->GetWindow()->UpdateConfigurationForSpecified(diffConfiguration, context_->GetResourceManager());
    }

    ConfigurationUpdated();
}

void STSUIExtensionBase::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
}

void STSUIExtensionBase::OnAbilityResult(int32_t requestCode, int32_t resultCode, const Want &resultData)
{
    TAG_LOGD(AAFwkTag::UI_EXT, "called");
    if (context_ == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null context");
        return;
    }
    context_->OnAbilityResult(requestCode, resultCode, resultData);
    if (abilityResultListeners_ == nullptr) {
        TAG_LOGW(AAFwkTag::UI_EXT, "null abilityResultListeners");
        return;
    }
    abilityResultListeners_->OnAbilityResult(requestCode, resultCode, resultData);
}

void STSUIExtensionBase::SetAbilityInfo(const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo)
{
    abilityInfo_ = abilityInfo;
}

void STSUIExtensionBase::SetContext(const std::shared_ptr<UIExtensionContext> &context)
{
    context_ = context;
}

void STSUIExtensionBase::ConfigurationUpdated()
{

}

#ifdef SUPPORT_GRAPHICS
void STSUIExtensionBase::OnDisplayInfoChange(
    const sptr<IRemoteObject> &token, Rosen::DisplayId displayId, float density, Rosen::DisplayOrientation orientation)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "displayId: %{public}" PRIu64 "", displayId);
    if (context_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }

    auto contextConfig = context_->GetConfiguration();
    if (contextConfig == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null configuration");
        return;
    }

    TAG_LOGI(AAFwkTag::UI_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
    auto configUtils = std::make_shared<ConfigurationUtils>();
    auto result = configUtils->UpdateDisplayConfig(
        contextConfig, context_->GetResourceManager(), displayId, density, orientation);
    TAG_LOGI(AAFwkTag::UI_EXT, "Config dump after update: %{public}s", contextConfig->GetName().c_str());
    if (result) {
        auto jsUiExtension = std::static_pointer_cast<STSUIExtensionBase>(shared_from_this());
        auto task = [jsUiExtension]() {
            if (jsUiExtension) {
                jsUiExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "STSUIExtensionBase:OnChange");
        }
    }
}

void STSUIExtensionBase::RegisterDisplayInfoChangedListener()
{
    // register displayid change callback
    auto jsUiExtensionBase = std::static_pointer_cast<STSUIExtensionBase>(shared_from_this());
    stsUiExtensionBaseDisplayListener_ = sptr<STSUIExtensionBaseDisplayListener>::MakeSptr(jsUiExtensionBase);
    if (stsUiExtensionBaseDisplayListener_ == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null STSUIExtensionBaseDisplayListener");
        return;
    }
    if (context_ == nullptr || context_->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    TAG_LOGI(AAFwkTag::UI_EXT, "RegisterDisplayInfoChangedListener");
    Rosen::WindowManager::GetInstance().RegisterDisplayInfoChangedListener(
        context_->GetToken(), stsUiExtensionBaseDisplayListener_);
}

void STSUIExtensionBase::UnregisterDisplayInfoChangedListener()
{
    if (context_ == nullptr || context_->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::UI_EXT, "null context");
        return;
    }
    Rosen::WindowManager::GetInstance().UnregisterDisplayInfoChangedListener(
        context_->GetToken(), stsUiExtensionBaseDisplayListener_);
}
#endif // SUPPORT_GRAPHICS
} // namespace AbilityRuntime
} // namespace OHOS
