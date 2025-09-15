/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_JS_SERVICE_EXTENSION_CONTEXT_H
#define OHOS_ABILITY_RUNTIME_JS_SERVICE_EXTENSION_CONTEXT_H

#include <memory>

#include "ability_connect_callback.h"
#include "service_extension_context.h"
#include "event_handler.h"
#include "js_free_install_observer.h"
#include "native_engine/native_engine.h"
#include "open_link_options.h"

namespace OHOS {
namespace AbilityRuntime {
napi_value CreateJsServiceExtensionContext(napi_env env, std::shared_ptr<ServiceExtensionContext> context);

class JSServiceExtensionConnection : public AbilityConnectCallback {
public:
    explicit JSServiceExtensionConnection(napi_env env);
    virtual ~JSServiceExtensionConnection();
    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode) override;
    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override;
    void HandleOnAbilityConnectDone(
        const AppExecFwk::ElementName &element, const sptr<IRemoteObject> &remoteObject, int resultCode);
    void HandleOnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode);
    void SetJsConnectionObject(napi_value jsConnectionObject);
    void RemoveConnectionObject();
    void CallJsFailed(int32_t errorCode);
    void SetConnectionId(int64_t id);
    int64_t GetConnectionId();
private:
    napi_env env_;
    std::unique_ptr<NativeReference> jsConnectionObject_ = nullptr;
    int64_t connectionId_ = -1;
};

struct ConnectionKey {
    AAFwk::Want want;
    int64_t id;
    int32_t accountId;
};

struct key_compare {
    bool operator()(const ConnectionKey &key1, const ConnectionKey &key2) const
    {
        if (key1.id < key2.id) {
            return true;
        }
        return false;
    }
};

static std::map<ConnectionKey, sptr<JSServiceExtensionConnection>, key_compare> connects_;
static int64_t serialNumber_ = 0;
static std::shared_ptr<AppExecFwk::EventHandler> handler_ = nullptr;

class StartAbilityByCallParameters {
public:
    int err = 0;
    sptr<IRemoteObject> remoteCallee = nullptr;
    std::shared_ptr<CallerCallBack> callerCallBack = nullptr;
    std::mutex mutexlock;
    std::condition_variable condition;
};

class JsServiceExtensionContext final {
public:
    explicit JsServiceExtensionContext(const std::shared_ptr<ServiceExtensionContext>& context);
    ~JsServiceExtensionContext();

    static void Finalizer(napi_env env, void* data, void* hint);
    static napi_value StartAbility(napi_env env, napi_callback_info info);
    static napi_value OpenLink(napi_env env, napi_callback_info info);
    static napi_value StartAbilityAsCaller(napi_env env, napi_callback_info info);
    static napi_value StartRecentAbility(napi_env env, napi_callback_info info);
    static napi_value StartAbilityByCall(napi_env env, napi_callback_info info);
    static napi_value StartAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value ConnectAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value TerminateAbility(napi_env env, napi_callback_info info);
    static napi_value ConnectAbility(napi_env env, napi_callback_info info);
    static napi_value DisconnectAbility(napi_env env, napi_callback_info info);
    static napi_value StartServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value StartUIServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value StartServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value StopServiceExtensionAbility(napi_env env, napi_callback_info info);
    static napi_value StopServiceExtensionAbilityWithAccount(napi_env env, napi_callback_info info);
    static napi_value RequestModalUIExtension(napi_env env, napi_callback_info info);
    static napi_value PreStartMission(napi_env env, napi_callback_info info);
    static napi_value OpenAtomicService(napi_env env, napi_callback_info info);
    std::weak_ptr<ServiceExtensionContext> GetContext()
    {
        return context_;
    }

private:
    std::weak_ptr<ServiceExtensionContext> context_;
    sptr<JsFreeInstallObserver> freeInstallObserver_ = nullptr;

    // helper functions (private methods) declarations
    static void ClearFailedCallConnection(const std::weak_ptr<ServiceExtensionContext>& serviceContext,
        const std::shared_ptr<CallerCallBack> &callback);
    void AddFreeInstallObserver(napi_env env, const AAFwk::Want &want, napi_value callback,
        napi_value* result, bool isOpenLink = false);
    napi_value OnStartAbility(napi_env env, NapiCallbackInfo& info, bool isStartRecent = false);
    bool CheckUrl(std::string &urlValue);
    bool ParseOpenLinkParams(const napi_env &env, const NapiCallbackInfo &info, std::string &linkValue,
        AAFwk::OpenLinkOptions &openLinkOptions, AAFwk::Want &want);
    napi_value OnOpenLink(napi_env env, NapiCallbackInfo& info);
    napi_value OnOpenLinkInner(napi_env env, const AAFwk::Want& want, const std::string& startTime, const std::string& url);
    void UnwrapCompletionHandlerInStartOptions(napi_env env, napi_value param, AAFwk::StartOptions &options);
    napi_value OnOpenAtomicService(napi_env env, NapiCallbackInfo &info);
    napi_value OpenAtomicServiceInner(napi_env env, const AAFwk::Want &want, const AAFwk::StartOptions &options,
        std::string startTime);
    napi_value OnStartRecentAbility(napi_env env, NapiCallbackInfo& info, bool isStartRecent = false);
    napi_value OnStartAbilityAsCaller(napi_env env, NapiCallbackInfo& info);
    bool CheckStartAbilityInputParam(napi_env env, NapiCallbackInfo& info,
        AAFwk::Want& want, AAFwk::StartOptions& startOptions, size_t& unwrapArgc) const;
    napi_value OnStartAbilityByCall(napi_env env, NapiCallbackInfo& info);
    bool CheckStartAbilityByCallInputParam(napi_env env, NapiCallbackInfo& info, AAFwk::Want& want, int32_t& accountId);
    NapiAsyncTask::CompleteCallback GetCallComplete(std::shared_ptr<StartAbilityByCallParameters> calls);
    NapiAsyncTask::ExecuteCallback GetCallExecute(std::shared_ptr<StartAbilityByCallParameters> calls);
    CallerCallBack::CallBackClosure GetCallBackDone(std::shared_ptr<StartAbilityByCallParameters> calls);
    CallerCallBack::OnReleaseClosure GetReleaseListen();
    napi_value OnStartAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    bool CheckStartAbilityWithAccountInputParam(napi_env env, NapiCallbackInfo& info,
        AAFwk::Want& want, int32_t& accountId, size_t& unwrapArgc) const;
    bool CheckConnectAbilityWithAccountInputParam(napi_env env, NapiCallbackInfo& info,
        AAFwk::Want& want, int32_t& accountId, sptr<JSServiceExtensionConnection>& connection) const;
    napi_value OnTerminateAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnConnectAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnConnectAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    bool CheckConnectionParam(napi_env env, napi_value value,
        sptr<JSServiceExtensionConnection>& connection, AAFwk::Want& want, int32_t accountId = -1) const;
    napi_value OnDisconnectAbility(napi_env env, NapiCallbackInfo& info);
    void FindConnection(AAFwk::Want& want, sptr<JSServiceExtensionConnection>& connection, int64_t& connectId,
        int32_t &accountId) const;
    napi_value OnStartExtensionAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartUIServiceExtension(napi_env env, NapiCallbackInfo& info);
    napi_value OnStartExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnStopExtensionAbility(napi_env env, NapiCallbackInfo& info);
    napi_value OnStopExtensionAbilityWithAccount(napi_env env, NapiCallbackInfo& info);
    napi_value OnRequestModalUIExtension(napi_env env, NapiCallbackInfo& info);
    bool ParsePreStartMissionArgs(const napi_env &env, const NapiCallbackInfo &info, std::string& bundleName,
        std::string& moduleName, std::string& abilityName, std::string& startTime);
    napi_value OnPreStartMission(napi_env env, NapiCallbackInfo& info);
    NapiAsyncTask::ExecuteCallback GetStartAbilityExecFunc(const AAFwk::Want &want,
        const AAFwk::StartOptions &startOptions, int32_t userId, bool useOption, std::shared_ptr<int> retCode);
    NapiAsyncTask::CompleteCallback GetSimpleCompleteFunc(std::shared_ptr<int> retCode);
    NapiAsyncTask::ExecuteCallback GetConnectAbilityExecFunc(const AAFwk::Want &want,
        sptr<JSServiceExtensionConnection> connection, int64_t connectId, std::shared_ptr<int> innerErrorCode);
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JS_SERVICE_EXTENSION_CONTEXT_H
