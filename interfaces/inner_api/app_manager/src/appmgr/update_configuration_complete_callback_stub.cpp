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

#include "update_configuration_complete_callback_stub.h"

#include "hilog_tag_wrapper.h"
#include "appexecfwk_errors.h"
#include "ipc_types.h"
#include "iremote_object.h"

namespace OHOS {
namespace AppExecFwk {
UpdateConfigurationCompleteCallbackStub::UpdateConfigurationCompleteCallbackStub()
{
}

UpdateConfigurationCompleteCallbackStub::~UpdateConfigurationCompleteCallbackStub()
{
}

int32_t UpdateConfigurationCompleteCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "Called.");
    std::u16string descriptor = UpdateConfigurationCompleteCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Local descriptor is not equal to remote.");
        return ERR_INVALID_STATE;
    }

    if (code == static_cast<uint32_t>(IUpdateConfigurationCompleteCallback::Message::ON_NOTIFY_COMPELTE)) {
        return HandleOnComplete(data, reply);
    }

    TAG_LOGI(AAFwkTag::APPMGR, "CompleteCallbackStub::OnRemoteRequest end");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t UpdateConfigurationCompleteCallbackStub::HandleOnComplete(MessageParcel &data,
    MessageParcel &reply)
{
    /*
     std::unique_ptr<Configuration> configuration(data.ReadParcelable<Configuration>());
    if (!configuration) {
        TAG_LOGE(AAFwkTag::APPMGR, "ReadParcelable<Configuration> failed");
        return ERR_APPEXECFWK_PARCEL_ERROR;
    }
    */
    TAG_LOGE(AAFwkTag::ABILITYMGR, "UpdateConfigurationCompleteCallbackStub::HandleOnFinish.............");
    OnComplete();
    return NO_ERROR;
}

// UpdateConfigurationCompleteCallbackRecipient::UpdateConfigurationCompleteCallbackRecipient(RemoteDiedHandler handler)
//     : handler_(handler)
// {}

// void UpdateConfigurationCompleteCallbackRecipient::OnRemoteDied(const wptr<IRemoteObject> &__attribute__((unused)) remote)
// {
//     TAG_LOGE(AAFwkTag::ABILITYMGR, "Remote died.");
//     if (handler_) {
//         handler_(remote);
//     }
// }
} // namespace AppExecFwk
} // namespace OHOS