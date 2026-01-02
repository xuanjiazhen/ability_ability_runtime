/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "agent_extension.h"
#include "agent_extension_context.h"
#include "configuration_utils.h"
#include "connection_manager.h"
#include "hilog_tag_wrapper.h"
#include "js_agent_extension.h"
#include "js_runtime_utils.h"
#include "runtime.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr size_t ARGC_ONE = 1;
constexpr size_t ARGC_TWO = 2;
}
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

AgentExtensionStubImpl::AgentExtensionStubImpl(std::weak_ptr<JsAgentExtension>& ext)
    :extension_(ext)
{
}

AgentExtensionStubImpl::~AgentExtensionStubImpl()
{
    TAG_LOGI(AAFwkTag::AGENT_EXT, "~");
}

int32_t AgentExtensionStubImpl::SendData(sptr<IRemoteObject> hostProxy, OHOS::AAFwk::WantParams &data)
{
    auto sptr = extension_.lock();
    if (sptr) {
        return sptr->OnSendData(hostProxy, data);
    }

    return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
}

napi_value AttachAgentExtensionContext(napi_env env, void *value, void *)
{
    if (value == nullptr) {
        TAG_LOGW(AAFwkTag::AGENT_EXT, "null value");
        return nullptr;
    }
    auto ptr = reinterpret_cast<std::weak_ptr<AgentExtensionContext> *>(value)->lock();
    if (ptr == nullptr) {
        TAG_LOGW(AAFwkTag::AGENT_EXT, "null ptr");
        return nullptr;
    }
    napi_value object = CreateJsAgentExtensionContext(env, ptr);
    auto sysModule = JsRuntime::LoadSystemModuleByEngine(env,
        "application.AgentExtensionContext", &object, 1);
    if (sysModule == nullptr) {
        TAG_LOGW(AAFwkTag::AGENT_EXT, "null sysModule");
        return nullptr;
    }
    auto contextObj = sysModule->GetNapiValue();
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAgentExtensionContext, value, nullptr);
    auto workContext = new (std::nothrow) std::weak_ptr<AgentExtensionContext>(ptr);
    auto res = napi_wrap(env, contextObj, workContext,
        [](napi_env, void *data, void *) {
            TAG_LOGD(AAFwkTag::AGENT_EXT, "Finalizer for weak_ptr app service extension context is called");
            delete static_cast<std::weak_ptr<AgentExtensionContext> *>(data);
        },
        nullptr, nullptr);
    if (res != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "napi_wrap failed:%{public}d", res);
        delete workContext;
        return nullptr;
    }
    return contextObj;
}

JsAgentExtension* JsAgentExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new JsAgentExtension(static_cast<JsRuntime&>(*runtime));
}

void JsAgentExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    //HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::AGENT_EXT, "Call");
    AgentExtension::Init(record, application, handler, token);
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "get srcPath failed");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    TAG_LOGD(AAFwkTag::AGENT_EXT, "called, moduleName:%{public}s,srcPath:%{public}s",
        moduleName.c_str(), srcPath.c_str());
    HandleScope handleScope(jsRuntime_);
    auto env = jsRuntime_.GetNapiEnv();

    jsObj_ = jsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE,
        false, abilityInfo_->srcEntrance);
    if (jsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "null jsObj_");
        return;
    }

    TAG_LOGD(AAFwkTag::AGENT_EXT, "ConvertNativeValueTo");
    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "get JsAgentExtension obj failed");
        return;
    }

    BindContext(env, obj);

    SetExtensionCommon(JsExtensionCommon::Create(jsRuntime_, static_cast<NativeReference&>(*jsObj_), shellContextRef_));

    auto context = GetContext();
    auto appContext = Context::GetApplicationContext();
    if (context != nullptr && appContext != nullptr) {
        auto appConfig = appContext->GetConfiguration();
        if (appConfig != nullptr) {
            TAG_LOGD(AAFwkTag::AGENT_EXT, "Original config dump: %{public}s", appConfig->GetName().c_str());
            context->SetConfiguration(std::make_shared<Configuration>(*appConfig));
        }
    }
    //ListenWMS();
}

void JsAgentExtension::OnStart(const AAFwk::Want &want)
{
    Extension::OnStart(want);
    TAG_LOGI(AAFwkTag::AGENT_EXT, "call");

    auto context = GetContext();
    if (context != nullptr) {
#ifdef SUPPORT_GRAPHICS
        int32_t displayId = AAFwk::DisplayUtil::GetDefaultDisplayId();
        TAG_LOGD(AAFwkTag::APP_SERVICE_EXT, "displayId %{public}d", displayId);
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
    TAG_LOGD(AAFwkTag::AGENT_EXT, "ok");
}

sptr<IRemoteObject> JsAgentExtension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    //HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::AGENT_EXT, "call");
    HandleScope handleScope(jsRuntime_);
    Extension::OnConnect(want);
    napi_env env = jsRuntime_.GetNapiEnv();
    sptr<IRemoteObject> hostProxy = GetHostProxyFromWant(want);
    if (hostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "null hostProxy");
        return nullptr;
    }
    napi_value napiWant = WrapWant(env, want);
    if (napiWant == nullptr) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "null napiWant");
        return nullptr;
    }
    if (extensionStub_ == nullptr) {
        std::weak_ptr<JsUIServiceExtension> weakThis =
            std::static_pointer_cast<JsAgentExtension>(shared_from_this());
        extensionStub_ = sptr<UIServiceStubImpl>::MakeSptr(weakThis);
    }
    sptr<IRemoteObject> stubObject = nullptr;
    if (extensionStub_ != nullptr) {
        stubObject = extensionStub_->AsObject();
    }
    if (hostProxyMap_.find(hostProxy) != hostProxyMap_.end()) {
        TAG_LOGI(AAFwkTag::AGENT_EXT, "hostProxy exist");
        return stubObject;
    }
    napi_ref hostProxyNref = AAFwk::JsUIServiceHostProxy::CreateJsUIServiceHostProxy(env, hostProxy);
    if (hostProxyNref == nullptr) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "null hostProxyNref");
        return nullptr;
    }
    napi_value jsHostProxy = reinterpret_cast<NativeReference*>(hostProxyNref)->GetNapiValue();
    napi_value argv[] = {napiWant, jsHostProxy};
    CallObjectMethod("onConnect", argv, ARGC_TWO);
    bool ret = CreateWindowIfNeeded();
    if (!ret) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "create window failed");
        delete reinterpret_cast<NativeReference*>(hostProxyNref);
        return nullptr;
    }
    hostProxyMap_[hostProxy] = std::unique_ptr<NativeReference>(reinterpret_cast<NativeReference*>(hostProxyNref));
    return stubObject;
}

int32_t JsAgentExtension::OnSendData(sptr<IRemoteObject> hostProxy, OHOS::AAFwk::WantParams &data)
{
    napi_env env = jsRuntime_.GetNapiEnv();
    std::unique_ptr<NapiAsyncTask::CompleteCallback> complete = std::make_unique<NapiAsyncTask::CompleteCallback>
        ([weak = weak_from_this(), hostProxy, wantParams = data](napi_env env, NapiAsyncTask &task, int32_t status) {
            auto extensionSptr = weak.lock();
            if (!extensionSptr) {
                TAG_LOGE(AAFwkTag::AGENT_EXT, "null extensionSptr");
                return;
            }
            auto sptrThis = std::static_pointer_cast<JsAgentExtension>(extensionSptr);
            if (!sptrThis) {
                TAG_LOGE(AAFwkTag::AGENT_EXT, "null sptrThis");
                return;
            }
            sptrThis->HandleSendData(hostProxy, wantParams);
        });

    napi_ref callback = nullptr;
    std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
    NapiAsyncTask::Schedule("JsAgentExtension::SendData",
        env, std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(complete)));
    return static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
}

void JsAgentExtension::HandleSendData(sptr<IRemoteObject> hostProxy, const OHOS::AAFwk::WantParams &data)
{
    if (hostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "null hostProxy");
        return;
    }
    napi_value jsHostProxy = nullptr;
    auto iter = hostProxyMap_.find(hostProxy);
    if (iter != hostProxyMap_.end()) {
        auto &hostProxyNref = iter->second;
        if (hostProxyNref != nullptr) {
            jsHostProxy = hostProxyNref->GetNapiValue();
        }
    }
    if (jsHostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "null jsHostProxy");
        return;
    }

    napi_env env = jsRuntime_.GetNapiEnv();
    napi_value argv[] = {jsHostProxy, AppExecFwk::CreateJsWantParams(env, data)};
    CallObjectMethod("onData", argv, ARGC_TWO);
}

napi_value JsAgentExtension::CallObjectMethod(const char* name, napi_value const* argv, size_t argc)
{
    TAG_LOGD(AAFwkTag::AGENT_EXT, "name:%{public}s", name);

    if (!jsObj_) {
        TAG_LOGW(AAFwkTag::AGENT_EXT, "Not found agent_extension_ability.js");
        return nullptr;
    }

    HandleScope handleScope(jsRuntime_);
    napi_env env = jsRuntime_.GetNapiEnv();

    napi_value obj = jsObj_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, obj, napi_object)) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "get object failed");
        return nullptr;
    }

    napi_value method = nullptr;
    napi_get_named_property(env, obj, name, &method);
    if (!CheckTypeForNapiValue(env, method, napi_function)) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "get '%{public}s' object failed", name);
        return nullptr;
    }
    TAG_LOGD(AAFwkTag::AGENT_EXT, "CallFunction(%{public}s) ok", name);
    napi_value result = nullptr;

    TryCatch tryCatch(env);
    napi_call_function(env, obj, method, argc, argv, &result);
    if (tryCatch.HasCaught()) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "HandleUncaughtException");
        reinterpret_cast<NativeEngine*>(env)->HandleUncaughtException();
    }
    return result;
}

sptr<IRemoteObject> JsAgentExtension::GetHostProxyFromWant(const AAFwk::Want &want)
{
    if (!want.HasParameter(AGENTEXTENSIONHOSTPROXY_KEY)) {
        TAG_LOGW(AAFwkTag::AGENT_EXT, "Not found AGENTEXTENSIONHOSTPROXY_KEY");
        return nullptr;
    }
    return want.GetRemoteObject(AGENTEXTENSIONHOSTPROXY_KEY);
}

void JsAgentExtension::BindContext(napi_env env, napi_value obj)
{
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "null context");
        return;
    }
    TAG_LOGD(AAFwkTag::AGENT_EXT, "call");
    napi_value contextObj = CreateJsAgentExtensionContext(env, context);
    shellContextRef_ = JsRuntime::LoadSystemModuleByEngine(env, "application.AgentExtensionContext",
        &contextObj, ARGC_ONE);
    if (shellContextRef_ == nullptr) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "null shellContextRef");
        return;
    }
    contextObj = shellContextRef_->GetNapiValue();
    if (!CheckTypeForNapiValue(env, contextObj, napi_object)) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "get context native obj failed");
        return;
    }
    auto workContext = new (std::nothrow) std::weak_ptr<AgentExtensionContext>(context);
    napi_coerce_to_native_binding_object(
        env, contextObj, DetachCallbackFunc, AttachAgentExtensionContext, workContext, nullptr);
    TAG_LOGD(AAFwkTag::AGENT_EXT, "Bind");
    context->Bind(jsRuntime_, shellContextRef_.get());
    napi_set_named_property(env, obj, "context", contextObj);

    auto res = napi_wrap(env, contextObj, workContext,
        [](napi_env, void* data, void*) {
            delete static_cast<std::weak_ptr<AgentExtensionContext>*>(data);
        },
        nullptr, nullptr);
    if (res != napi_ok && workContext != nullptr) {
        TAG_LOGE(AAFwkTag::AGENT_EXT, "napi_wrap failed:%{public}d", res);
        delete workContext;
        return;
    }
    TAG_LOGD(AAFwkTag::AGENT_EXT, "end");
}

void JsAgentExtension::GetSrcPath(std::string &srcPath)
{
    TAG_LOGD(AAFwkTag::AGENT_EXT, "called");
    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        srcPath.erase(srcPath.rfind('.'));
        srcPath.append(".abc");
    }
}
} // namespace AgentRuntime
} // namespace OHOS