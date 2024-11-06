/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "js_embeddable_ui_ability_impl.h"

#include "configuration_utils.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "insight_intent_execute_param.h"
#include "js_data_struct_converter.h"
#include "js_embeddable_ui_ability_context.h"
#include "js_embeddable_window_stage.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_ui_extension_content_session.h"
#include "js_utils.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_want.h"
#include "ui_extension_context.h"

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
} // namespace

napi_value AttachEmbeddableUIAbilityContext(napi_env env, void *value, void *extValue)
{
    TAG_LOGD(AAFwkTag::EMBEDDABLE_UIABILITY, "AttachEmbeddableUIAbilityContext");
    if (value == nullptr || extValue == nullptr) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "invalid parameter");
        return nullptr;
    }

    auto ptr = reinterpret_cast<std::weak_ptr<UIExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "invalid context");
        return nullptr;
    }

    auto screenModePtr = reinterpret_cast<std::weak_ptr<int32_t> *>(extValue)->lock();
    if (screenModePtr == nullptr) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "invalid screenModePtr");
        return nullptr;
    }

    auto object = JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(env, nullptr, ptr, *screenModePtr);
    auto contextRef = JsRuntime::LoadSystemModuleByEngine(env, "application.EmbeddableUIAbilityContext",
        &object, 1);
    if (contextRef == nullptr) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "load module failed");
        return nullptr;
    }
    auto contextObj = contextRef->GetNapiValue();
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "get context failed");
        return nullptr;
    }

    auto status = napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc,
        AttachEmbeddableUIAbilityContext, value, extValue);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "coerce context failed: %{public}d", status);
        return nullptr;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(ptr);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::EMBEDDABLE_UIABILITY, "Finalizer for embeddableuiability context");
            delete static_cast<std::weak_ptr<UIExtensionContext> *>(data);
        }, nullptr, nullptr);
    return contextObj;
}

JsEmbeddableUIAbilityImpl::JsEmbeddableUIAbilityImpl(const std::unique_ptr<Runtime> &runtime)
    : JsUIExtensionBase(runtime)
{
    TAG_LOGD(AAFwkTag::EMBEDDABLE_UIABILITY, "JsEmbeddableUIAbilityImpl constructor");
}

JsEmbeddableUIAbilityImpl::~JsEmbeddableUIAbilityImpl()
{
    TAG_LOGD(AAFwkTag::EMBEDDABLE_UIABILITY, "JsEmbeddableUIAbilityImpl destructor");
}

void JsEmbeddableUIAbilityImpl::BindContext(const std::shared_ptr<AAFwk::Want> want)
{
    TAG_LOGD(AAFwkTag::EMBEDDABLE_UIABILITY, "BindContext");
    CHECK_POINTER(want);
    CHECK_POINTER(jsObj_);
    CHECK_POINTER(context_);
    screenMode_ = want->GetIntParam(AAFwk::SCREEN_MODE_KEY, AAFwk::IDLE_SCREEN_MODE);

    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value obj = jsObj_->GetNapiValue();
    napi_value contextObj = JsEmbeddableUIAbilityContext::CreateJsEmbeddableUIAbilityContext(env,
        nullptr, context_, screenMode_);
    CHECK_POINTER(contextObj);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.EmbeddableUIAbilityContext",
        &contextObj, ARGC_ONE);
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "load EmbeddableUIAbilityContext module failed");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "get context failed");
        return;
    }

    auto workContext = new (std::nothrow) std::weak_ptr<UIExtensionContext>(context_);
    CHECK_POINTER(workContext);
    screenModePtr_ = std::make_shared<int32_t>(screenMode_);
    auto workScreenMode = new (std::nothrow) std::weak_ptr<int32_t>(screenModePtr_);
    if (workScreenMode == nullptr) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "workScreenMode nullptr");
        delete workContext;
        return;
    }

    napi_coerce_to_native_binding_object(env, contextObj, DetachCallbackFunc, AttachEmbeddableUIAbilityContext,
        workContext, workScreenMode);
    context_->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);
    napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*hint) {
            TAG_LOGD(AAFwkTag::EMBEDDABLE_UIABILITY, "Finalizer for embeddableuiability context");
            delete static_cast<std::weak_ptr<UIExtensionContext>*>(data);
            delete static_cast<std::weak_ptr<int32_t> *>(hint);
        }, workScreenMode, nullptr);
    TAG_LOGD(AAFwkTag::EMBEDDABLE_UIABILITY, "BindContext end");
}

void JsEmbeddableUIAbilityImpl::OnStart(const AAFwk::Want &want, AAFwk::LaunchParam &launchParam,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::EMBEDDABLE_UIABILITY, "OnStart");
#ifdef SUPPORT_GRAPHICS
    if (context_ != nullptr && sessionInfo != nullptr) {
        auto configUtils = std::make_shared<ConfigurationUtils>();
        configUtils->InitDisplayConfig(context_->GetConfiguration(), context_->GetResourceManager(),
            sessionInfo->displayId, sessionInfo->density, sessionInfo->orientation);
    }
#endif // SUPPORT_GRAPHICS

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    if (context_ != nullptr) {
        JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, context_->GetConfiguration());
    }

    napi_value napiWant = AppExecFwk::WrapWant(env, want);
    if (InsightIntentExecuteParam::IsInsightIntentExecute(want)) {
        launchParam.launchReason = AAFwk::LaunchReason::LAUNCHREASON_INSIGHT_INTENT;
    }
    napi_value argv[] = { napiWant, CreateJsLaunchParam(env, launchParam) };
    CallObjectMethod("onCreate", argv, ARGC_TWO);
    TAG_LOGD(AAFwkTag::EMBEDDABLE_UIABILITY, "OnStart end");
}

bool JsEmbeddableUIAbilityImpl::HandleSessionCreate(const AAFwk::Want &want, const sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr || sessionInfo->uiExtensionComponentId == 0) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "Invalid sessionInfo");
        return false;
    }
    std::lock_guard<std::mutex> lock(uiWindowMutex_);
    TAG_LOGD(AAFwkTag::EMBEDDABLE_UIABILITY, "UIExtension component id: %{public}" PRId64 ", element: %{public}s",
        sessionInfo->uiExtensionComponentId, want.GetElement().GetURI().c_str());
    auto compId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(compId) == uiWindowMap_.end()) {
        auto uiWindow = CreateUIWindow(context_, sessionInfo);
        if (uiWindow == nullptr) {
            TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "create ui window error");
            return false;
        }
        HandleScope handleScope(jsRuntime_);
        napi_env env = jsRuntime_.GetNapiEnv();
        napi_value nativeContentSession = JsUIExtensionContentSession::CreateJsUIExtensionContentSession(
            env, sessionInfo, uiWindow, context_, abilityResultListeners_);
        napi_ref ref = nullptr;
        napi_create_reference(env, nativeContentSession, 1, &ref);
        contentSessions_.emplace(compId, std::shared_ptr<NativeReference>(reinterpret_cast<NativeReference*>(ref)));
        auto jsAppWindowStage = CreateAppWindowStage(uiWindow, sessionInfo);
        if (jsAppWindowStage == nullptr) {
            TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "JsAppWindowStage is nullptr");
            return false;
        }
        napi_value argv[] = {jsAppWindowStage->GetNapiValue()};
        CallObjectMethod("onWindowStageCreate", argv, ARGC_ONE);
        uiWindowMap_[compId] = uiWindow;
        if (context_->GetWindow() == nullptr) {
            context_->SetWindow(uiWindow);
        }
    }
    return true;
}

sptr<Rosen::Window> JsEmbeddableUIAbilityImpl::CreateUIWindow(const std::shared_ptr<UIExtensionContext> context,
    const sptr<AAFwk::SessionInfo> &sessionInfo)
{
    if (context == nullptr || context->GetAbilityInfo() == nullptr) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "Failed to get context");
        return nullptr;
    }
    auto option = sptr<Rosen::WindowOption>::MakeSptr();
    if (option == nullptr) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "make option failed");
        return nullptr;
    }
    option->SetWindowName(context->GetBundleName() + context->GetAbilityInfo()->name);
    option->SetWindowType(Rosen::WindowType::WINDOW_TYPE_UI_EXTENSION);
    option->SetWindowSessionType(Rosen::WindowSessionType::EXTENSION_SESSION);
    option->SetParentId(sessionInfo->hostWindowId);
    option->SetRealParentId(sessionInfo->realHostWindowId);
    option->SetParentWindowType(static_cast<Rosen::WindowType>(sessionInfo->parentWindowType));
    option->SetUIExtensionUsage(static_cast<uint32_t>(sessionInfo->uiExtensionUsage));
    HITRACE_METER_NAME(HITRACE_TAG_APP, "Rosen::Window::Create");
    return Rosen::Window::Create(option, context_, sessionInfo->sessionToken);
}

std::unique_ptr<NativeReference> JsEmbeddableUIAbilityImpl::CreateAppWindowStage(sptr<Rosen::Window> uiWindow,
    sptr<AAFwk::SessionInfo> sessionInfo)
{
    auto env = jsRuntime_.GetNapiEnv();
    napi_value jsWindowStage = Rosen::JsEmbeddableWindowStage::CreateJsEmbeddableWindowStage(
        env, uiWindow, sessionInfo);
    if (jsWindowStage == nullptr) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "Failed to create jsWindowSatge object");
        return nullptr;
    }
    return JsRuntime::LoadSystemModuleByEngine(env, "application.embeddablewindowstage", &jsWindowStage, 1);
}

void JsEmbeddableUIAbilityImpl::DestroyWindow(const sptr<AAFwk::SessionInfo> sessionInfo)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    if (sessionInfo == nullptr) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "Invalid sessionInfo");
        return;
    }
    std::lock_guard<std::mutex> lock(uiWindowMutex_);
    auto componentId = sessionInfo->uiExtensionComponentId;
    if (uiWindowMap_.find(componentId) == uiWindowMap_.end()) {
        TAG_LOGE(AAFwkTag::EMBEDDABLE_UIABILITY, "Wrong to find uiWindow");
        return;
    }
    if (contentSessions_.find(componentId) != contentSessions_.end() && contentSessions_[componentId] != nullptr) {
        HandleScope handleScope(jsRuntime_);
        CallObjectMethod("onWindowStageDestroy");
    }
    TAG_LOGI(AAFwkTag::EMBEDDABLE_UIABILITY, "Before window destory, UIExtcomponent id: %{public}" PRId64,
        sessionInfo->uiExtensionComponentId);
    auto uiWindow = uiWindowMap_[componentId];
    if (uiWindow) {
        uiWindow->Destroy();
    }
    uiWindowMap_.erase(componentId);
    foregroundWindows_.erase(componentId);
    contentSessions_.erase(componentId);
    if (abilityResultListeners_) {
        abilityResultListeners_->RemoveListener(componentId);
    }
    TAG_LOGD(AAFwkTag::EMBEDDABLE_UIABILITY, "DestroyWindow end");
}
} // namespace AbilityRuntime
} // namespace OHOS
