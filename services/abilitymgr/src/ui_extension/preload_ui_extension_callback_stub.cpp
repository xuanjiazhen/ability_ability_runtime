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

#include "preload_ui_extension_callback_stub.h"
#include "preload_ui_extension_host_client.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {

PreloadUIExtensionCallbackStub::PreloadUIExtensionCallbackStub() {}

PreloadUIExtensionCallbackStub::~PreloadUIExtensionCallbackStub() {}

int32_t PreloadUIExtensionCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (data.ReadInterfaceToken() != IPreloadUIExtensionCallback::GetDescriptor()) {
        TAG_LOGE(AAFwkTag::UI_EXT, "InterfaceToken not equal");
        return ERR_INVALID_STATE;
    }

    switch (code) {
        case ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE:
            return HandleOnLoadedDone(data, reply);
        case ON_PRELOAD_UI_EXTENSION_ABILITY_DESTROY_DONE:
            return HandleOnDestroyDone(data, reply);
        default:
            TAG_LOGW(AAFwkTag::UI_EXT, "Invalid code");
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t PreloadUIExtensionCallbackStub::HandleOnLoadedDone(MessageParcel &data, MessageParcel &reply)
{
    TAG_LOGI(AAFwkTag::UI_EXT, "called");
    std::string bundleName = data.ReadString();
    int32_t resultCode = data.ReadInt32();
    int32_t extensionAbilityId = data.ReadInt32();
    OnLoadedDone(bundleName, resultCode, extensionAbilityId);
    return ERR_OK;
}

int32_t PreloadUIExtensionCallbackStub::HandleOnDestroyDone(MessageParcel &data, MessageParcel &reply)
{
    std::string bundleName = data.ReadString();
    int32_t resultCode = data.ReadInt32();
    int32_t extensionAbilityId = data.ReadInt32();
    OnDestroyDone(bundleName, resultCode, extensionAbilityId);
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS
