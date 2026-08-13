/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "want_sender_stub.h"

#include "ability_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"

namespace OHOS {
namespace AAFwk {
WantSenderStub::WantSenderStub() {}

WantSenderStub::~WantSenderStub() {}

int WantSenderStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    if (code == IWantSender::WANT_SENDER_SEND) {
        TAG_LOGW(AAFwkTag::WANTAGENT,
            "IWantSender::Send IPC deprecated (would bypass CheckCallerPermission), reject. callerUid=%{public}d",
            IPCSkeleton::GetCallingUid());
        return ERR_NO_PERMISSION_CALLER;
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int WantSenderStub::SendInner(MessageParcel &data, MessageParcel &reply)
{
    SenderInfo *senderInfo = data.ReadParcelable<SenderInfo>();
    if (senderInfo == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null senderInfo");
        return ERR_INVALID_VALUE;
    }
    Send(*senderInfo);
    delete senderInfo;
    return NO_ERROR;
}
}  // namespace AAFwk
}  // namespace OHOS
