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

#include "ets_agent_receiver_proxy.h"

#include "ability_business_error.h"
#include "ani_common_util.h"
#include "ani_common_want.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
constexpr int32_t INVALID_PARAM = static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
} // namespace
using namespace AbilityRuntime;

constexpr const char *AGENT_RECEIVER_PROXY_CLASS_NAME = "application.AgentReceiverProxy.AgentReceiverProxyImpl";

ani_object EtsAgentReceiverProxy::CreateEtsAgentReceiverProxy(ani_env *env, const sptr<IRemoteObject> &impl,
                                                              const sptr<IRemoteObject> &connectorProxy)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "CreateEtsAgentReceiverProxy called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }
    ani_object object = nullptr;
    ani_class cls {};
    ani_status status = env->FindClass(AGENT_RECEIVER_PROXY_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FindClass failed status: %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    status = env->Class_FindMethod(cls, "<ctor>", ":", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "call Class_FindMethod ctor failed");
        return nullptr;
    }
    status = env->Object_New(cls, method, &object);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "call Object_New failed");
        return nullptr;
    }
    ani_field field = nullptr;
    if ((status = env->Class_FindField(cls, "nativeAgentReceiverProxy", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Class_FindField failed status: %{public}d", status);
        return nullptr;
    }
    std::unique_ptr<EtsAgentReceiverProxy> proxy =
        std::make_unique<EtsAgentReceiverProxy>(impl, connectorProxy);

    ani_long nativeAgentReceiverProxyLong = reinterpret_cast<ani_long>(proxy.release());
    if ((status = env->Object_SetField_Long(object, field, nativeAgentReceiverProxyLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Object_SetField_Long failed status: %{public}d", status);
        return nullptr;
    }

    std::array functions = {
        ani_native_function {"nativeSendData", nullptr,
            reinterpret_cast<void *>(EtsAgentReceiverProxy::SendData)},
        ani_native_function {"nativeAuthorize", nullptr,
            reinterpret_cast<void *>(EtsAgentReceiverProxy::Authorize)}
    };

    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Class_BindNativeMethods status: %{public}d", status);
        return nullptr;
    }
    return object;
}

ani_object EtsAgentReceiverProxy::CreateEmptyProxyObject(ani_env *env)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CreateEmptyProxyObject called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }
    ani_object object = nullptr;
    ani_class cls {};
    ani_status status = env->FindClass(AGENT_RECEIVER_PROXY_CLASS_NAME, &cls);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "FindClass failed status: %{public}d", status);
        return nullptr;
    }
    ani_method method = nullptr;
    status = env->Class_FindMethod(cls, "<ctor>", ":", &method);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "call Class_FindMethod ctor failed");
        return nullptr;
    }
    status = env->Object_New(cls, method, &object);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "call Object_New failed");
        return nullptr;
    }
    return object;
}

EtsAgentReceiverProxy::EtsAgentReceiverProxy(const sptr<IRemoteObject> &impl,
                                             const sptr<IRemoteObject> &connectorProxy)
    : connectorProxy_(connectorProxy)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "EtsAgentReceiverProxy called");
    if (impl != nullptr) {
        proxy_ = iface_cast<OHOS::AgentRuntime::IAgentReceiver>(impl);
    }
    if (proxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null proxy");
    }
}

EtsAgentReceiverProxy::~EtsAgentReceiverProxy()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "EtsAgentReceiverProxy destroyed");
    proxy_ = nullptr;
    connectorProxy_ = nullptr;
}

EtsAgentReceiverProxy* EtsAgentReceiverProxy::GetEtsAgentReceiverProxy(ani_env *env, ani_object obj)
{
    if (env == nullptr || obj == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env or obj");
        return nullptr;
    }
    EtsAgentReceiverProxy *receiverProxy = nullptr;
    ani_status status = ANI_ERROR;
    ani_long nativeAgentReceiverProxyLong = 0;
    if ((status = env->Object_GetFieldByName_Long(obj, "nativeAgentReceiverProxy",
        &nativeAgentReceiverProxyLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Object_GetFieldByName_Long failed status: %{public}d", status);
        return nullptr;
    }
    receiverProxy = reinterpret_cast<EtsAgentReceiverProxy *>(nativeAgentReceiverProxyLong);
    if (receiverProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "receiverProxy null");
        return nullptr;
    }
    return receiverProxy;
}

void EtsAgentReceiverProxy::SendData(ani_env *env, ani_object obj, ani_string aniData)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SendData called");
    EtsAgentReceiverProxy* etsReceiverProxy = GetEtsAgentReceiverProxy(env, obj);
    if (etsReceiverProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetEtsAgentReceiverProxy failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    // Convert ani_string to std::string
    std::string data;
    if (!AppExecFwk::GetStdString(env, aniData, data)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "param data err");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, INVALID_PARAM, "Parameter error. Convert data fail.");
        return;
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "data: %{public}s", data.c_str());

    etsReceiverProxy->OnSendData(env, data);
}

void EtsAgentReceiverProxy::Authorize(ani_env *env, ani_object obj, ani_string aniData)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Authorize called");
    EtsAgentReceiverProxy* etsReceiverProxy = GetEtsAgentReceiverProxy(env, obj);
    if (etsReceiverProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetEtsAgentReceiverProxy failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    // Convert ani_string to std::string
    std::string data;
    if (!AppExecFwk::GetStdString(env, aniData, data)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "param data err");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, INVALID_PARAM, "Parameter error. Convert data fail.");
        return;
    }
    TAG_LOGD(AAFwkTag::SER_ROUTER, "data: %{public}s", data.c_str());

    etsReceiverProxy->OnAuthorize(env, data);
}

void EtsAgentReceiverProxy::OnSendData(ani_env *env, const std::string &data)
{
    if (proxy_ == nullptr || connectorProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null proxy_ or connectorProxy_");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnSendData, data length: %{public}zu", data.length());

    int32_t ret = proxy_->SendData(connectorProxy_, data);
    if (ret != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "proxy_->SendData failed: %{public}d", ret);
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
}

void EtsAgentReceiverProxy::OnAuthorize(ani_env *env, const std::string &data)
{
    if (proxy_ == nullptr || connectorProxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null proxy_ or connectorProxy_");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAuthorize, data length: %{public}zu", data.length());

    int32_t ret = proxy_->Authorize(connectorProxy_, data);
    if (ret != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "proxy_->Authorize failed: %{public}d", ret);
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
}

}  // namespace AgentRuntime
}  // namespace OHOS
