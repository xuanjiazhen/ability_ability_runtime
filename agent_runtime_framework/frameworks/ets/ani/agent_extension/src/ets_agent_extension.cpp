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

#include "ets_agent_extension.h"

#include "ability_business_error.h"
#include "agent_extension.h"
#include "agent_extension_connection_constants.h"
#include "agent_extension_context.h"
#include "agent_extension_stub_impl.h"
#include "ani_common_configuration.h"
#include "ani_common_want.h"
#include "configuration_utils.h"
#include "ets_agent_extension_context.h"
#include "ets_extension_context.h"
#include "ets_runtime.h"
#include "hilog_tag_wrapper.h"
#include "hitrace_meter.h"
#include "remote_object_taihe_ani.h"

#ifdef WINDOWS_PLATFORM
#define ETS_EXPORT __declspec(dllexport)
#else
#define ETS_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AgentRuntime {
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace {
constexpr const char *AGENT_EXTENSION_CONTEXT_CLASS_NAME =
    "application.AgentExtensionContext.AgentExtensionContext";
constexpr const char *ON_CREATE_SIGNATURE = "C{@ohos.app.ability.Want.Want}:";
constexpr const char *ON_CONNECT_SIGNATURE = "C{@ohos.app.ability.Want.Want}C{:}:";
constexpr const char *ON_DISCONNECT_SIGNATURE = "C{@ohos.app.ability.Want.Want}C{:}:";
constexpr const char *ON_DATA_SIGNATURE = "C{:}S:";
constexpr const char *VOID_SIGNATURE = ":";
constexpr const char *ON_CONFIGURATION_UPDATE_SIGNATURE = "C{@ohos.app.ability.Configuration.Configuration}:";
} // namespace

EtsAgentExtension* EtsAgentExtension::Create(const std::unique_ptr<Runtime>& runtime)
{
    return new EtsAgentExtension(static_cast<ETSRuntime&>(*runtime));
}

EtsAgentExtension::EtsAgentExtension(ETSRuntime& etsRuntime) : etsRuntime_(etsRuntime) {}

EtsAgentExtension::~EtsAgentExtension()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "EtsAgentExtension destroy");
    auto context = GetContext();
    if (context) {
        context->Unbind();
    }
}

void EtsAgentExtension::Init(const std::shared_ptr<AbilityLocalRecord> &record,
    const std::shared_ptr<OHOSApplication> &application, std::shared_ptr<AbilityHandler> &handler,
    const sptr<IRemoteObject> &token)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Call");
    AgentExtension::Init(record, application, handler, token);
    std::string srcPath = "";
    GetSrcPath(srcPath);
    if (srcPath.empty()) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "get srcPath failed");
        return;
    }

    std::string moduleName(Extension::abilityInfo_->moduleName);
    moduleName.append("::").append(abilityInfo_->name);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "called, moduleName:%{public}s,srcPath:%{public}s",
        moduleName.c_str(), srcPath.c_str());

    etsObj_ = etsRuntime_.LoadModule(
        moduleName, srcPath, abilityInfo_->hapPath, abilityInfo_->compileMode == CompileMode::ES_MODULE,
        false, abilityInfo_->srcEntrance);
    if (etsObj_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null etsObj_");
        return;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "LoadModule success");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return;
    }
    BindContext(env);

    auto context = GetContext();
    auto appContext = Context::GetApplicationContext();
    if (context != nullptr && appContext != nullptr) {
        auto appConfig = appContext->GetConfiguration();
        if (appConfig != nullptr) {
            TAG_LOGD(AAFwkTag::SER_ROUTER, "Original config dump: %{public}s", appConfig->GetName().c_str());
            context->SetConfiguration(std::make_shared<Configuration>(*appConfig));
        }
    }
}

void EtsAgentExtension::OnStart(const AAFwk::Want &want)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::SER_ROUTER, "call");
    Extension::OnStart(want);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env not found");
        return;
    }
    ani_ref wantRef = OHOS::AppExecFwk::WrapWant(env, want);
    if (wantRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null wantRef");
        return;
    }

    CallObjectMethod(false, "onCreate", ON_CREATE_SIGNATURE, wantRef);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
}

void EtsAgentExtension::OnStop()
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "call");
    Extension::OnStop();
    CallObjectMethod(false, "onDestroy", VOID_SIGNATURE);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
}

sptr<IRemoteObject> EtsAgentExtension::OnConnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<sptr<IRemoteObject>> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::SER_ROUTER, "call");
    Extension::OnConnect(want);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }

    sptr<IRemoteObject> hostProxy = GetHostProxyFromWant(want);
    if (hostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null hostProxy");
        return nullptr;
    }

    ani_object aniWant = WrapWant(env, want);
    if (aniWant == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null aniWant");
        return nullptr;
    }

    if (extensionStub_ == nullptr) {
        std::weak_ptr<EtsAgentExtension> weakThis =
            std::static_pointer_cast<EtsAgentExtension>(shared_from_this());
        extensionStub_ = sptr<AgentExtensionStubImpl>::MakeSptr(weakThis);
    }
    sptr<IRemoteObject> stubObject = nullptr;
    if (extensionStub_ != nullptr) {
        stubObject = extensionStub_->AsObject();
    }

    if (hostProxyMap_.find(hostProxy) != hostProxyMap_.end()) {
        TAG_LOGI(AAFwkTag::SER_ROUTER, "hostProxy exist");
        return stubObject;
    }

    // Create ETS host proxy object
    ani_object jsHostProxy = nullptr;
    ani_status status = ANI_ERROR;
    ani_class hostProxyCls = nullptr;
    ani_method hostProxyCtor = nullptr;

    // Find RemoteObject wrapper class and create instance
    if ((status = env->FindClass("ohos.napi.RemoteObject", &hostProxyCls)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to find RemoteObject class, status: %{public}d", status);
        return nullptr;
    }

    ani_ref hostProxyRef = AniWrapNativeRemoteObject(env, hostProxy);
    if (hostProxyRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to wrap hostProxy");
        return nullptr;
    }
    jsHostProxy = reinterpret_cast<ani_object>(hostProxyRef);

    CallObjectMethod(false, "onConnect", ON_CONNECT_SIGNATURE, aniWant, jsHostProxy);

    // Store the host proxy reference
    ani_ref globalRef = nullptr;
    if (env->GlobalReference_Create(jsHostProxy, &globalRef) == ANI_OK) {
        auto etsRef = std::make_unique<ETSNativeReference>(env, globalRef);
        hostProxyMap_[hostProxy] = std::move(etsRef);
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
    return stubObject;
}

void EtsAgentExtension::OnDisconnect(const AAFwk::Want &want,
    AppExecFwk::AbilityTransactionCallbackInfo<> *callbackInfo, bool &isAsyncCallback)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, __PRETTY_FUNCTION__);
    TAG_LOGI(AAFwkTag::SER_ROUTER, "call");
    Extension::OnDisconnect(want);
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return;
    }

    sptr<IRemoteObject> hostProxy = GetHostProxyFromWant(want);
    if (hostProxy == nullptr) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "null hostProxy");
        return;
    }

    ani_object aniWant = WrapWant(env, want);
    if (aniWant == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null aniWant");
        return;
    }

    ani_object jsHostProxy = nullptr;
    auto iter = hostProxyMap_.find(hostProxy);
    if (iter != hostProxyMap_.end()) {
        auto &hostProxyRef = iter->second;
        if (hostProxyRef != nullptr && hostProxyRef->aniRef != nullptr) {
            jsHostProxy = reinterpret_cast<ani_object>(hostProxyRef->aniRef);
        }
    }

    if (jsHostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null jsHostProxy");
        return;
    }

    CallObjectMethod(false, "onDisconnect", ON_DISCONNECT_SIGNATURE, aniWant, jsHostProxy);
    hostProxyMap_.erase(iter);
    TAG_LOGD(AAFwkTag::SER_ROUTER, "end");
}

int32_t EtsAgentExtension::OnSendData(sptr<IRemoteObject> hostProxy, std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnSendData call");
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER);
    }

    HandleSendData(hostProxy, data);
    return static_cast<int32_t>(AbilityErrorCode::ERROR_OK);
}

void EtsAgentExtension::HandleSendData(sptr<IRemoteObject> hostProxy, const std::string &data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "HandleSendData call");
    if (hostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null hostProxy");
        return;
    }

    ani_object jsHostProxy = nullptr;
    auto iter = hostProxyMap_.find(hostProxy);
    if (iter != hostProxyMap_.end()) {
        auto &hostProxyRef = iter->second;
        if (hostProxyRef != nullptr && hostProxyRef->aniRef != nullptr) {
            jsHostProxy = reinterpret_cast<ani_object>(hostProxyRef->aniRef);
        }
    }

    if (jsHostProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null jsHostProxy");
        return;
    }

    auto env = etsRuntime_.GetAniEnv();
    // Create string from data
    ani_ref dataRef = nullptr;
    ani_string aniData = nullptr;
    if (env->String_NewUTF8(data.c_str(), &aniData) == ANI_OK) {
        dataRef = reinterpret_cast<ani_ref>(aniData);
    }

    if (dataRef == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null dataRef");
        return;
    }

    CallObjectMethod(false, "onData", ON_DATA_SIGNATURE, jsHostProxy, dataRef);
}

ani_ref EtsAgentExtension::CallObjectMethod(bool withResult, const char *name, const char *signature, ...)
{
    HITRACE_METER_NAME(HITRACE_TAG_ABILITY_MANAGER, std::string("CallObjectMethod:") + name);
    ani_status status = ANI_ERROR;
    ani_method method = nullptr;
    auto env = etsRuntime_.GetAniEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }
    if ((status = env->Class_FindMethod(etsObj_->aniCls, name, signature, &method)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Class_FindMethod failed for %{public}s, status: %{public}d", name, status);
        return nullptr;
    }
    if (method == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "method is null for %{public}s", name);
        return nullptr;
    }
    ani_ref res = nullptr;
    va_list args;
    if (withResult) {
        va_start(args, signature);
        if ((status = env->Object_CallMethod_Ref_V(etsObj_->aniObj, method, &res, args)) != ANI_OK) {
            TAG_LOGE(AAFwkTag::SER_ROUTER, "status : %{public}d", status);
            va_end(args);
            return nullptr;
        }
        va_end(args);
        return res;
    }
    va_start(args, signature);
    if ((status = env->Object_CallMethod_Void_V(etsObj_->aniObj, method, args)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "status : %{public}d", status);
    }
    va_end(args);
    return nullptr;
}

ani_object EtsAgentExtension::CreateETSContext(ani_env *env, std::shared_ptr<AgentExtensionContext> context)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CreateETSContext");
    return CreateEtsAgentExtensionContext(env, context);
}

ani_object EtsAgentExtension::WrapWant(ani_env *env, const AAFwk::Want &want)
{
    AAFwk::Want jsWant = want;
    jsWant.RemoveParam(AGENTEXTENSIONHOSTPROXY_KEY);
    ani_object aniWant = OHOS::AppExecFwk::WrapWantToObject(env, jsWant);
    return aniWant;
}

sptr<IRemoteObject> EtsAgentExtension::GetHostProxyFromWant(const AAFwk::Want &want)
{
    if (!want.HasParameter(AGENTEXTENSIONHOSTPROXY_KEY)) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "Not found AGENTEXTENSIONHOSTPROXY_KEY");
        return nullptr;
    }
    return want.GetRemoteObject(AGENTEXTENSIONHOSTPROXY_KEY);
}

void EtsAgentExtension::BindContext(ani_env *env)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "BindContext");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "env is null");
        return;
    }
    auto context = GetContext();
    if (context == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to get context");
        return;
    }
    ani_object contextObj = CreateETSContext(env, context);
    if (contextObj == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null contextObj");
        return;
    }
    ani_field contextField;
    auto status = env->Class_FindField(etsObj_->aniCls, "context", &contextField);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Class_FindField context failed");
        return;
    }
    ani_ref contextRef = nullptr;
    if (env->GlobalReference_Create(contextObj, &contextRef) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GlobalReference_Create contextObj failed");
        return;
    }
    if (env->Object_SetField_Ref(etsObj_->aniObj, contextField, contextRef) != ANI_OK) {
        TAG_LOGD(AAFwkTag::SER_ROUTER, "Object_SetField_Ref contextObj failed");
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "BindContext end");
}

void EtsAgentExtension::GetSrcPath(std::string &srcPath)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "called");
    if (!Extension::abilityInfo_->srcEntrance.empty()) {
        srcPath.append(Extension::abilityInfo_->moduleName + "/");
        srcPath.append(Extension::abilityInfo_->srcEntrance);
        auto pos = srcPath.rfind(".");
        if (pos != std::string::npos) {
            srcPath.erase(pos);
            srcPath.append(".abc");
        }
    }
}

} // namespace AgentRuntime
} // namespace OHOS

ETS_EXPORT extern "C" OHOS::AgentRuntime::AgentExtension *OHOS_ETS_Agent_Extension_Create(
    const std::unique_ptr<OHOS::AbilityRuntime::Runtime> &runtime)
{
    return OHOS::AgentRuntime::EtsAgentExtension::Create(runtime);
}
