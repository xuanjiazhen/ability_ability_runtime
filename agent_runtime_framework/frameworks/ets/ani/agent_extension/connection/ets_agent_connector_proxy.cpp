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

#include "ets_agent_connector_proxy.h"

#include "ability_business_error.h"
#include "ani_common_want.h"
#include "ets_error_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AgentRuntime {
using namespace AbilityRuntime;

constexpr const char *AGENT_CONNECTOR_PROXY_CLASS_NAME = "application.AgentConnectorProxy.AgentConnectorProxyImpl";

ani_object EtsAgentConnectorProxy::CreateEtsAgentConnectorProxy(ani_env *env, const sptr<IRemoteObject> &impl)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "CreateEtsAgentConnectorProxy called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }
    ani_object object = nullptr;
    ani_class cls {};
    ani_status status = env->FindClass(AGENT_CONNECTOR_PROXY_CLASS_NAME, &cls);
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
    if ((status = env->Class_FindField(cls, "nativeAgentConnectorProxy", &field)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Class_FindField failed status: %{public}d", status);
        return nullptr;
    }
    std::unique_ptr<EtsAgentConnectorProxy> proxy = std::make_unique<EtsAgentConnectorProxy>(impl);

    ani_long nativeAgentConnectorProxyLong = reinterpret_cast<ani_long>(proxy.release());
    if ((status = env->Object_SetField_Long(object, field, nativeAgentConnectorProxyLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Object_SetField_Long failed status: %{public}d", status);
        return nullptr;
    }

    std::array functions = {
        ani_native_function {"nativeSendData", nullptr,
            reinterpret_cast<void *>(EtsAgentConnectorProxy::SendData)},
        ani_native_function {"nativeAuthorize", nullptr,
            reinterpret_cast<void *>(EtsAgentConnectorProxy::Authorize)}
    };

    if ((status = env->Class_BindNativeMethods(cls, functions.data(), functions.size())) != ANI_OK
        && status != ANI_ALREADY_BINDED) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Class_BindNativeMethods status: %{public}d", status);
        return nullptr;
    }
    return object;
}

ani_object EtsAgentConnectorProxy::CreateEmptyProxyObject(ani_env *env)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "CreateEmptyProxyObject called");
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env");
        return nullptr;
    }
    ani_object object = nullptr;
    ani_class cls {};
    ani_status status = env->FindClass(AGENT_CONNECTOR_PROXY_CLASS_NAME, &cls);
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

EtsAgentConnectorProxy::EtsAgentConnectorProxy(const sptr<IRemoteObject> &impl)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "EtsAgentConnectorProxy called");
    if (impl != nullptr) {
        proxy_ = iface_cast<OHOS::AgentRuntime::IAgentConnector>(impl);
    }
    if (proxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null proxy");
    }
}

EtsAgentConnectorProxy::~EtsAgentConnectorProxy()
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "EtsAgentConnectorProxy destroyed");
    proxy_ = nullptr;
}

EtsAgentConnectorProxy* EtsAgentConnectorProxy::GetEtsAgentConnectorProxy(ani_env *env, ani_object obj)
{
    if (env == nullptr || obj == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null env or obj");
        return nullptr;
    }
    EtsAgentConnectorProxy *connectorProxy = nullptr;
    ani_status status = ANI_ERROR;
    ani_long nativeAgentConnectorProxyLong = 0;
    if ((status = env->Object_GetFieldByName_Long(obj, "nativeAgentConnectorProxy",
        &nativeAgentConnectorProxyLong)) != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Object_GetFieldByName_Long failed status: %{public}d", status);
        return nullptr;
    }
    connectorProxy = reinterpret_cast<EtsAgentConnectorProxy *>(nativeAgentConnectorProxyLong);
    if (connectorProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "connectorProxy null");
        return nullptr;
    }
    return connectorProxy;
}

void EtsAgentConnectorProxy::SendData(ani_env *env, ani_object obj, ani_string data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "SendData called");
    EtsAgentConnectorProxy* etsConnectorProxy = GetEtsAgentConnectorProxy(env, obj);
    if (etsConnectorProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetEtsAgentConnectorProxy failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    // Convert ani_string to std::string
    ani_size length = 0;
    ani_status status = env->String_GetLength(data, &length);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "String_GetLength failed status: %{public}d", status);
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }

    std::vector<char> buffer(length + 1);
    status = env->String_GetUtf8(data, buffer.data(), buffer.size(), nullptr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "String_GetUtf8 failed status: %{public}d", status);
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }
    std::string dataStr(buffer.data());

    etsConnectorProxy->OnSendData(env, dataStr);
}

void EtsAgentConnectorProxy::Authorize(ani_env *env, ani_object obj, ani_string data)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "Authorize called");
    EtsAgentConnectorProxy* etsConnectorProxy = GetEtsAgentConnectorProxy(env, obj);
    if (etsConnectorProxy == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "GetEtsAgentConnectorProxy failed");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    // Convert ani_string to std::string
    ani_size length = 0;
    ani_status status = env->String_GetLength(data, &length);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "String_GetLength failed status: %{public}d", status);
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }

    std::vector<char> buffer(length + 1);
    status = env->String_GetUtf8(data, buffer.data(), buffer.size(), nullptr);
    if (status != ANI_OK) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "String_GetUtf8 failed status: %{public}d", status);
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INVALID_PARAM);
        return;
    }
    std::string dataStr(buffer.data());

    etsConnectorProxy->OnAuthorize(env, dataStr);
}

void EtsAgentConnectorProxy::OnSendData(ani_env *env, const std::string &data)
{
    if (proxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null proxy_");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnSendData, data length: %{public}zu", data.length());

    int32_t ret = proxy_->SendData(data);
    if (ret != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "proxy_->SendData failed: %{public}d", ret);
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
}

void EtsAgentConnectorProxy::OnAuthorize(ani_env *env, const std::string &data)
{
    if (proxy_ == nullptr) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "null proxy_");
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
        return;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAuthorize, data length: %{public}zu", data.length());

    int32_t ret = proxy_->Authorize(data);
    if (ret != static_cast<int32_t>(AbilityErrorCode::ERROR_OK)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "proxy_->Authorize failed: %{public}d", ret);
        AbilityRuntime::EtsErrorUtil::ThrowError(env, AbilityErrorCode::ERROR_CODE_INNER);
    }
}

}  // namespace AgentRuntime
}  // namespace OHOS
