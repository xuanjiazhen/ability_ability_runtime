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

#include "js_agent_host_proxy.h"

#include "ability_business_error.h"
#include "hilog_tag_wrapper.h"
#include "ipc_types.h"
#include "js_error_utils.h"

namespace OHOS {
namespace AgentRuntime {
using namespace AbilityRuntime;

static constexpr int32_t INDEX_ZERO = 0;
static constexpr int32_t ARGC_ONE = 1;

napi_value JsAgentHostProxy::CreateJsAgentHostProxy(napi_env env, const sptr<IRemoteObject> &hostProxy)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "CreateJsAgentHostProxy called");
    HandleEscape handleEscape(env);
    napi_value object = nullptr;
    napi_create_object(env, &object);
    if (object == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to create object");
        return CreateJsUndefined(env);
    }

    std::unique_ptr<JsAgentHostProxy> proxy = std::make_unique<JsAgentHostProxy>(hostProxy);
    napi_wrap(env, object, proxy.release(), Finalizer, nullptr, nullptr);

    const char *moduleName = "JsAgentHostProxy";
    BindNativeFunction(env, object, "sendData", moduleName, JsAgentHostProxy::SendData);
    BindNativeFunction(env, object, "authorize", moduleName, JsAgentHostProxy::Authorize);
    return handleEscape.Escape(object);
}

void JsAgentHostProxy::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "Finalizer called");
    std::unique_ptr<JsAgentHostProxy>(static_cast<JsAgentHostProxy*>(data));
}

JsAgentHostProxy::JsAgentHostProxy(const sptr<IRemoteObject> &hostProxy)
    : hostProxy_(hostProxy)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Constructor called");
    if (hostProxy_ == nullptr) {
        TAG_LOGW(AAFwkTag::SER_ROUTER, "hostProxy_ is null");
    }
}

JsAgentHostProxy::~JsAgentHostProxy()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Destructor called");
    hostProxy_ = nullptr;
}

napi_value JsAgentHostProxy::SendData(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SendData called from JS");
    GET_NAPI_INFO_AND_CALL(env, info, JsAgentHostProxy, OnSendData);
}

napi_value JsAgentHostProxy::Authorize(napi_env env, napi_callback_info info)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Authorize called from JS");
    GET_NAPI_INFO_AND_CALL(env, info, JsAgentHostProxy, OnAuthorize);
}

napi_value JsAgentHostProxy::OnSendData(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnSendData implementation");

    if (hostProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null hostProxy_");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid argc: %{public}zu", info.argc);
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // Extract string parameter
    std::string data;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], data)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to convert parameter to string");
        ThrowInvalidParamError(env, "Parameter must be a string");
        return CreateJsUndefined(env);
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "Sending data to host, length: %{public}zu", data.length());

    // Create IPC message to send to host
    MessageParcel parcelData;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!parcelData.WriteInterfaceToken(IAgentProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write interface token failed");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    if (!parcelData.WriteString(data)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write data failed");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    auto error = hostProxy_->SendRequest(static_cast<uint32_t>(IAgentProxy::ON_SEND_DATA),
        parcelData, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "SendRequest error %{public}d", error);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "SendData succeeded");
    return CreateJsUndefined(env);
}

napi_value JsAgentHostProxy::OnAuthorize(napi_env env, NapiCallbackInfo &info)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAuthorize implementation");

    if (hostProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null hostProxy_");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    if (info.argc < ARGC_ONE) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Invalid argc: %{public}zu", info.argc);
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // Extract string parameter
    std::string authData;
    if (!ConvertFromJsValue(env, info.argv[INDEX_ZERO], authData)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to convert parameter to string");
        ThrowInvalidParamError(env, "Parameter must be a string");
        return CreateJsUndefined(env);
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "Sending auth to host, length: %{public}zu", authData.length());

    // Create IPC message to send to host
    MessageParcel parcelData;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_ASYNC);

    if (!parcelData.WriteInterfaceToken(IAgentProxy::GetDescriptor())) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write interface token failed");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    if (!parcelData.WriteString(authData)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "write auth data failed");
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    auto error = hostProxy_->SendRequest(static_cast<uint32_t>(IAgentProxy::ON_AUTHORIZE),
        parcelData, reply, option);
    if (error != ERR_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "SendRequest error %{public}d", error);
        ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return CreateJsUndefined(env);
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "Authorize succeeded");
    return CreateJsUndefined(env);
}

} // namespace AgentRuntime
} // namespace OHOS
