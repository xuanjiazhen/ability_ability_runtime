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

#include "agent_extension_host_stub.h"

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AgentRuntime {

AgentExtensionHostStub::AgentExtensionHostStub()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "called");
    requestFuncMap_[ON_SEND_DATA] = &AgentExtensionHostStub::OnSendData;
    requestFuncMap_[ON_AUTHORIZE] = &AgentExtensionHostStub::OnAuthorize;
}

AgentExtensionHostStub::~AgentExtensionHostStub()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "called");
    requestFuncMap_.clear();
}

int AgentExtensionHostStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnRemoteRequest, code: %{public}u", code);

    std::u16string descriptor = AgentExtensionHostStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Descriptor mismatch");
        return ERR_INVALID_STATE;
    }

    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }

    TAG_LOGW(AAFwkTag::SER_ROUTER, "Unknown code: %{public}u", code);
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t AgentExtensionHostStub::OnSendData(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnSendData called");

    std::string dataStr;
    if (!data.ReadString(dataStr)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to read string from parcel");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnSendData, data length: %{public}zu", dataStr.length());

    int32_t result = OnSendData(dataStr);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to write result to reply parcel");
        return IPC_STUB_ERR;
    }

    return NO_ERROR;
}

int32_t AgentExtensionHostStub::OnAuthorize(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAuthorize called");

    std::string authStr;
    if (!data.ReadString(authStr)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to read string from parcel");
        return ERR_INVALID_VALUE;
    }

    TAG_LOGD(AAFwkTag::SER_ROUTER, "OnAuthorize, auth length: %{public}zu", authStr.length());

    int32_t result = OnAuthorize(authStr);
    if (!reply.WriteInt32(result)) {
        TAG_LOGE(AAFwkTag::SER_ROUTER, "Failed to write result to reply parcel");
        return IPC_STUB_ERR;
    }

    return NO_ERROR;
}

} // namespace AgentRuntime
} // namespace OHOS
