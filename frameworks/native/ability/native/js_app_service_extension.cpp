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

#include "js_app_service_extension.h"

#include "ability_business_error.h"
#include "ability_handler.h"
#include "ability_info.h"
#include "ability_manager_client.h"
#include "configuration_utils.h"
#include "display_util.h"
#include "hitrace_meter.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_param.h"
#include "insight_intent_execute_result.h"
#include "insight_intent_executor_info.h"
#include "insight_intent_executor_mgr.h"
#include "js_extension_common.h"
#include "js_extension_context.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "js_app_service_extension_context.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "napi_common_configuration.h"
#include "napi_common_want.h"
#include "napi_remote_object.h"
#ifdef SUPPORT_GRAPHICS
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "window_scene.h"
#endif

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
}

namespace {
sptr<IRemoteObject> GetNativeRemoteObject(napi_env env, napi_value obj)
{
    if (env == nullptr || obj == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null obj");
        return nullptr;
    }
    napi_valuetype type;
    napi_typeof(env, obj, &type);
    if (type == napi_undefined || type == napi_null) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "obj type invalid");
        return nullptr;
    }
    if (type != napi_object) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "obj not object");
        return nullptr;
    }
    return NAPI_ohos_rpc_getNativeRemoteObject(env, obj);
}
}

using namespace OHOS::AppExecFwk;

napi_value AttachAppServiceExtensionContext(napi_env env, void *value, void *)
{
    if (value == nullptr) {
        TAG_LOGW(AAFwkTag::SERVICE_EXT, "null value");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AppServiceExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::SERVICE_EXT, "null ptr");
        return nullptr;
    }
    napi_value object = CreateJsAppServiceExtensionContext(env, ptr);
    auto sysModule = JsRuntime::LoadSystemModuleByEngine(env,
        "application.AppServiceExtensionContext", &object, 1);
    if (sysModule == nullptr) {
        TAG_LOGW(AAFwkTag::SERVICE_EXT, "null sysModule");
        return nullptr;
    }
    auto contextObj = sysModule->GetNapiValue();
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAppServiceExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AppServiceExtensionContext>(ptr);
    auto res = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::SERVICE_EXT, "Finalizer for weak_ptr app service extension context is called");
            delete static_cast<std::weak_ptr<AppServiceExtensionContext> *>(data);
        },
        nullptr, nullptr);
    if (res != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "napi_wrap failed:%{public}d", res);
        delete workContext;
        return nullptr;
    }
    return contextObj;
}

JsAppServiceExtension* JsAppServiceExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new JsAppServiceExtension(static_cast<JsRuntime&>(*runtime));
}

JsAppServiceExtension::JsAppServiceExtension(JsRuntime& jsRuntime) : jsRuntime_(jsRuntime) {}
JsAppServiceExtension::~JsAppServiceExtension()
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }

    jsRuntime_.FreeNativeReference(std::move(jsObj_));
    jsRuntime_.FreeNativeReference(std::move(shellContextRef_));
}

void JsAppServiceExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppServiceExtension::Init(record, application, handler, token);
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "get srcPath failed");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called, moduleName:%{public}s,srcPath:%{public}s",
        moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE,
        false, abilityInfo_->srcEntrance);
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null jsObj_");
        return;
    }

    TAG_LOGD(AAFwkTag::SERVICE_EXT, "ConvertNativeValueTo");
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "get JsAppServiceExtension obj failed");
        return;
    }

    BindContext(env, obj);

    SetExtensionCommon(JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_));

    handler_ = handler;
    auto context = GetContext();
    auto appContext = Context::GetApplicationContext();
    if (context != nullptr && appContext != nullptr) {
        auto appConfig = appContext->GetConfiguration();
        if (appConfig != nullptr) {
            TAG_LOGD(AAFwkTag::SERVICE_EXT, "Original config dump: %{public}s", appConfig->GetName().c_str());
            context->SetConfiguration(std::make_shared<Configuration>(*appConfig));
        }
    }
}

void JsAppServiceExtension::BindContext(napi_env env, napi_value obj)
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return;
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "call");
    napi_value contextObj = CreateJsAppServiceExtensionContext(env, context);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.AppServiceExtensionContext",
        &contextObj, ARGC_ONE);
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null shellContextRef");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "get context native obj failed");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AppServiceExtensionContext>(context);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAppServiceExtensionContext, workContext, nullptr);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Bind");
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);

    auto res = napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            delete static_cast<std::weak_ptr<AppServiceExtensionContext>*>(data);
        },
        nullptr, nullptr);
    if (res != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "napi_wrap failed:%{public}d", res);
        delete workContext;
        return;
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
}

void JsAppServiceExtension::OnStart(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnStart(want);
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "call");

    auto context = GetContext();
    if (context != nullptr) {
#ifdef SUPPORT_GRAPHICS
        int32_t displayId = AAFwk::DisplayUtil::GetDefaultDisplayId();
        displayId = want.GetIntParam(Want::PARAM_RESV_DISPLAY_ID, displayId);
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "displayId %{public}d", displayId);
        auto configUtils = std::make_shared<ConfigurationUtils>();
        configUtils->InitDisplayConfig(displayId, context->GetConfiguration(), context->GetResourceManager());
#endif //SUPPORT_GRAPHICS
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // display config has changed, need update context.config
    if (context != nullptr) {
        JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, context->GetConfiguration());
    }

    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    CallObjectMethod("onCreate", argv, ARGC_ONE);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "ok");
}

void JsAppServiceExtension::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppServiceExtension::OnStop();
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "call");
    CallObjectMethod("onDestroy");
    bool ret = ConnectionManager::GetInstance().DisconnectCaller(GetContext()->GetToken());
    if (ret) {
        ConnectionManager::GetInstance().ReportConnectionLeakEvent(getpid(), gettid());
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "app service extension connection not disconnected");
    }
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "UnregisterDisplayInfoChangedListener");
    auto context = GetContext();
    if (context == nullptr || context->GetToken() == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return;
    }
#ifdef SUPPORT_GRAPHICS
    Rosen::WindowManager::GetInstance()
        .UnregisterDisplayInfoChangedListener(context->GetToken(), displayListener_);
    if (saStatusChangeListener_) {
        auto saMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (saMgr) {
            saMgr->UnSubscribeSystemAbility(WINDOW_MANAGER_SERVICE_ID, saStatusChangeListener_);
        } else {
            TAG_LOGW(AAFwkTag::SERVICE_EXT, "OnStop SaMgr null");
        }
    }
#endif //SUPPORT_GRAPHICS
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "ok");
}

sptr<IRemoteObject> JsAppServiceExtension::OnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    napi_value result = CallOnConnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    auto remoteObj = GetNativeRemoteObject(env, result);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null remoteObj");
    }
    return remoteObj;
}

sptr<IRemoteObject> JsAppServiceExtension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value result = CallOnConnect(want);
    isAsyncCallback = false;
    sptr<IRemoteObject> remoteObj = GetNativeRemoteObject(env, result);
    if (remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null remoteObj");
    }
    return remoteObj;
}

void JsAppServiceExtension::OnDisconnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    Extension::OnDisconnect(want);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");
    CallOnDisconnect(want, false);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
}

void JsAppServiceExtension::OnDisconnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleScope handleScope(jsRuntime_);
    Extension::OnDisconnect(want);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");
    CallOnDisconnect(want, false);
    isAsyncCallback = false;
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "end");
}

void JsAppServiceExtension::OnCommand(const AAFwk::Want &want, bool restart, int startId)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnCommand(want, restart, startId);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "restart=%{public}s,startId=%{public}d",
        restart ? "true" : "false",
        startId);
    // wrap want
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    // wrap startId
    napi_value napiStartId = nullptr;
    napi_create_int32(env, startId, &napiStartId);
    napi_value argv[] = {napiWant, napiStartId};
    CallObjectMethod("onRequest", argv, ARGC_TWO);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "ok");
}

bool JsAppServiceExtension::HandleInsightIntent(const AAFwk::Want &want)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");
    auto callback = std::make_unique<InsightIntentExecutorAsyncCallback>();
    callback.reset(InsightIntentExecutorAsyncCallback::Create());
    if (callback == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null callback");
        return false;
    }
    auto executeParam = std::make_shared<AppExecFwk::InsightIntentExecuteParam>();
    bool ret = AppExecFwk::InsightIntentExecuteParam::GenerateFromWant(want, *executeParam);
    if (!ret) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Generate execute param failed");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return false;
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Insight bundleName: %{public}s, moduleName: %{public}s, abilityName: %{public}s"
        "insightIntentName: %{public}s, executeMode: %{public}d, intentId: %{public}" PRIu64 "",
        executeParam->bundleName_.c_str(), executeParam->moduleName_.c_str(), executeParam->abilityName_.c_str(),
        executeParam->insightIntentName_.c_str(), executeParam->executeMode_, executeParam->insightIntentId_);
    auto asyncCallback = [weak = weak_from_this(), intentId = executeParam->insightIntentId_]
        (AppExecFwk::InsightIntentExecuteResult result) {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "intentId %{public}" PRIu64"", intentId);
        auto extension = weak.lock();
        if (extension == nullptr) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "null extension");
            return;
        }
        auto ret = extension->OnInsightIntentExecuteDone(intentId, result);
        if (!ret) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "OnInsightIntentExecuteDone failed");
        }
    };
    callback->Push(asyncCallback);
    InsightIntentExecutorInfo executorInfo;
    ret = GetInsightIntentExecutorInfo(want, executeParam, executorInfo);
    if (!ret) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Failed to retrieve intent executor");
        InsightIntentExecutorMgr::TriggerCallbackInner(std::move(callback),
            static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM));
        return false;
    }
    ret = DelayedSingleton<InsightIntentExecutorMgr>::GetInstance()->ExecuteInsightIntent(
        jsRuntime_, executorInfo, std::move(callback));
    if (!ret) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Execution of insight intent failed");
        return false;
    }
    return true;
}

bool JsAppServiceExtension::GetInsightIntentExecutorInfo(const Want &want,
    const std::shared_ptr<AppExecFwk::InsightIntentExecuteParam> &executeParam,
    InsightIntentExecutorInfo &executorInfo)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");
    auto context = GetContext();
    if (executeParam == nullptr || context == nullptr || abilityInfo_ == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Param invalid");
        return false;
    }

    const WantParams &wantParams = want.GetParams();
    executorInfo.srcEntry = wantParams.GetStringParam(AppExecFwk::INSIGHT_INTENT_SRC_ENTRY);
    executorInfo.hapPath = abilityInfo_->hapPath;
    executorInfo.esmodule = abilityInfo_->compileMode == AppExecFwk::CompileMode::ES_MODULE;
    executorInfo.token = context->GetToken();
    executorInfo.executeParam = executeParam;
    return true;
}

bool JsAppServiceExtension::OnInsightIntentExecuteDone(uint64_t intentId,
    const AppExecFwk::InsightIntentExecuteResult &result)
{
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "Notify execute done, intentId %{public}" PRIu64"", intentId);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return false;
    }
    auto token = context->GetToken();
    if (token == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null token");
        return false;
    }
    auto ret = AAFwk::AbilityManagerClient::GetInstance()->ExecuteInsightIntentDone(token, intentId, result);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "Notify execute done failed");
        return false;
    }
    return true;
}

napi_value JsAppServiceExtension::CallObjectMethod(const char* name, napi_value const* argv, size_t argc)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "name:%{public}s", name);

    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::SERVICE_EXT, "Not found AppServiceExtension.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "get AppServiceExtension obj failed");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "get '%{public}s' from AppServiceExtension obj failed", name);
        return nullptr;
    }
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "CallFunction(%{public}s) ok", name);
    napi_value result = nullptr;
    napi_status status = napi_call_function(env, obj, method, argc, argv, &result);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "call js func failed: %{public}d", status);
    }
    return result;
}

void JsAppServiceExtension::GetSrcPath(std::string &srcPath)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");
    if (!Extension::abilityInfo_->isModuleJson) {
        /* temporary compatibility api8 + config.json */
        srcPath.append(Extension::abilityInfo_->package);
        srcPath.append("/assets/js/");
        if (!Extension::abilityInfo_->srcPath.empty()) {
            srcPath.append(Extension::abilityInfo_->srcPath);
        }
        srcPath.append("/").append(Extension::abilityInfo_->name).append(".abc");
        return;
    }

    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}

napi_value JsAppServiceExtension::CallOnConnect(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    Extension::OnConnect(want);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "call");
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = {napiWant};
    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::SERVICE_EXT, "Not found AppServiceExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "get AppServiceExtension obj failed");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onConnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null method");
        return nullptr;
    }
    napi_value remoteNative = nullptr;
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "Call onConnect");
    napi_status status = napi_call_function(env, obj, method, ARGC_ONE, argv, &remoteNative);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "call js func failed %{public}d", status);
    }
    if (remoteNative == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null remoteNative");
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "ok");
    return remoteNative;
}

napi_value JsAppServiceExtension::CallOnDisconnect(const AAFwk::Want &want, bool withResult)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    HandleEscape handleEscape(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value napiWant = OHOS::AppExecFwk::WrapWant(env, want);
    napi_value argv[] = { napiWant };
    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::SERVICE_EXT, "Not found AppServiceExtension.js");
        return nullptr;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "get AppServiceExtension obj failed");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDisconnect", &method);
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null method");
        return nullptr;
    }
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "Call onDisconnect");
    if (withResult) {
        napi_value result = nullptr;
        napi_status status = napi_call_function(env, obj, method, ARGC_ONE, argv, &result);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "call js func failed %{public}d", status);
        }
        return handleEscape.Escape(result);
    } else {
        napi_status status = napi_call_function(env, obj, method, ARGC_ONE, argv, nullptr);
        if (status != napi_ok) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "call js func failed %{public}d", status);
        }
        return nullptr;
    }
}

void JsAppServiceExtension::OnConfigurationUpdated(const AppExecFwk::Configuration& configuration)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    AppServiceExtension::OnConfigurationUpdated(configuration);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "call");
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig != nullptr) {
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
        std::vector<std::string> changeKeyV;
        contextConfig->CompareDifferent(changeKeyV, configuration);
        if (!changeKeyV.empty()) {
            contextConfig->Merge(changeKeyV, configuration);
        }
        TAG_LOGD(AAFwkTag::SERVICE_EXT, "Config dump after merge: %{public}s", contextConfig->GetName().c_str());
    }
    ConfigurationUpdated();
}

void JsAppServiceExtension::ConfigurationUpdated()
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "called");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    // Notify extension context
    auto fullConfig = GetContext()->GetConfiguration();
    if (!fullConfig) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null configuration");
        return;
    }

    napi_value napiConfiguration = OHOS::AppExecFwk::WrapConfiguration(env, *fullConfig);
    CallObjectMethod("onConfigurationUpdated", &napiConfiguration, ARGC_ONE);
    CallObjectMethod("onConfigurationUpdate", &napiConfiguration, ARGC_ONE);
    JsExtensionContext::ConfigurationUpdated(env, shellContextRef_, fullConfig);
}

void JsAppServiceExtension::Dump(const std::vector<std::string> &params, std::vector<std::string> &info)
{
    Extension::Dump(params, info);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "call");
    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();
    // create js array object of params
    napi_value argv[] = { CreateNativeArray(env, params) };

    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::SERVICE_EXT, "Not found AppServiceExtension.js");
        return;
    }

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "get AppServiceExtension obj failed");
        return;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, "onDump", &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        method = nullptr;
        napi_get_named_property(env, obj, "dump", &method);
        if (!CheckTypeForNapiValue(env, method, napi_function)) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "get onConnect from AppServiceExtension obj failed");
            return;
        }
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "success");
    napi_value dumpInfo = nullptr;
    napi_call_function(env, obj, method, ARGC_ONE, argv, &dumpInfo);
    if (dumpInfo == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null dumpInfo");
        return;
    }
    uint32_t len = 0;
    napi_get_array_length(env, dumpInfo, &len);
    for (uint32_t i = 0; i < len; i++) {
        std::string dumpInfoStr;
        napi_value element = nullptr;
        napi_get_element(env, dumpInfo, i, &element);
        if (!ConvertFromJsValue(env, element, dumpInfoStr)) {
            TAG_LOGE(AAFwkTag::SERVICE_EXT, "Parse dumpInfoStr failed");
            return;
        }
        info.push_back(dumpInfoStr);
    }
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Dump info size: %{public}zu", info.size());
}

#ifdef SUPPORT_GRAPHICS
void JsAppServiceExtension::OnCreate(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "enter");
}

void JsAppServiceExtension::OnDestroy(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "exit");
}

void JsAppServiceExtension::OnDisplayInfoChange(const sptr<IRemoteObject>& token, Rosen::DisplayId displayId,
    float density, Rosen::DisplayOrientation orientation)
{
    TAG_LOGI(AAFwkTag::SERVICE_EXT, "displayId: %{public}" PRIu64, displayId);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null contextConfig");
        return;
    }

    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
    bool configChanged = false;
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateDisplayConfig(displayId, contextConfig, context->GetResourceManager(), configChanged);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Config dump after update: %{public}s", contextConfig->GetName().c_str());

    if (configChanged) {
        auto jsAppServiceExtension = std::static_pointer_cast<JsAppServiceExtension>(shared_from_this());
        auto task = [jsAppServiceExtension]() {
            if (jsAppServiceExtension) {
                jsAppServiceExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "JsAppServiceExtension:OnChange");
        }
    }

    TAG_LOGD(AAFwkTag::SERVICE_EXT, "finished");
}

void JsAppServiceExtension::OnChange(Rosen::DisplayId displayId)
{
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "displayId: %{public}" PRIu64"", displayId);
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null context");
        return;
    }

    auto contextConfig = context->GetConfiguration();
    if (contextConfig == nullptr) {
        TAG_LOGE(AAFwkTag::SERVICE_EXT, "null contextConfig");
        return;
    }

    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Config dump: %{public}s", contextConfig->GetName().c_str());
    bool configChanged = false;
    auto configUtils = std::make_shared<ConfigurationUtils>();
    configUtils->UpdateDisplayConfig(displayId, contextConfig, context->GetResourceManager(), configChanged);
    TAG_LOGD(AAFwkTag::SERVICE_EXT, "Config dump after update: %{public}s", contextConfig->GetName().c_str());

    if (configChanged) {
        auto jsAppServiceExtension = std::static_pointer_cast<JsAppServiceExtension>(shared_from_this());
        auto task = [jsAppServiceExtension]() {
            if (jsAppServiceExtension) {
                jsAppServiceExtension->ConfigurationUpdated();
            }
        };
        if (handler_ != nullptr) {
            handler_->PostTask(task, "JsAppServiceExtension:OnChange");
        }
    }

    TAG_LOGD(AAFwkTag::SERVICE_EXT, "finished");
}
#endif
} // AbilityRuntime
} // OHOS
