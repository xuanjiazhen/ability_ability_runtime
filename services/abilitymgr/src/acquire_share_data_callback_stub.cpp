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

#include "acquire_share_data_callback_stub.h"

namespace OHOS {
namespace AAFwk {

AcquireShareDataCallbackStub::AcquireShareDataCallbackStub() {}

AcquireShareDataCallbackStub::~AcquireShareDataCallbackStub()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "call");
}

int32_t AcquireShareDataCallbackStub::OnRemoteRequest(
    uint32_t code,
    MessageParcel& data,
    MessageParcel& reply,
    MessageOption& option)
{
    std::u16string localDescriptor = GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (localDescriptor != remoteDescriptor) {
        return ERR_TRANSACTION_FAILED;
    }
    switch (static_cast<IAcquireShareDataCallbackIpcCode>(code)) {
        case IAcquireShareDataCallbackIpcCode::COMMAND_ACQUIRE_SHARE_DATA_DONE: {
            int32_t resultCode = data.ReadInt32();
            std::unique_ptr<WantParams> wantParam(data.ReadParcelable<WantParams>());
            if (!wantParam) {
                return ERR_INVALID_DATA;
            }

            int32_t result = 0;
            ErrCode errCode = AcquireShareDataDone(resultCode, *wantParam, result);
            if (SUCCEEDED(errCode)) {
                if (!reply.WriteInt32(errCode)) {
                    return ERR_INVALID_DATA;
                }
            }
            return ERR_NONE;
        }
        default:
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    return ERR_TRANSACTION_FAILED;
}
ErrCode AcquireShareDataCallbackStub::AcquireShareDataDone(int32_t resultCode, const WantParams &wantParam, int32_t &funcResult)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "resultCode:%{public}d, wantParam size:%{public}d", resultCode, wantParam.Size());
    if (resultCode || wantParam.IsEmpty()) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "invaild param");
    }
    auto task = [resultCode, wantParam, shareRuntimeTask = shareRuntimeTask_]() {
        if (shareRuntimeTask) {
            shareRuntimeTask(resultCode, wantParam);
        }
    };
    TAG_LOGI(AAFwkTag::ABILITYMGR, "acquireShareDataDone shareRuntimeTask start");
    if (!handler_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "null handler_ object");
        return funcResult = OBJECT_NULL;
    }
    handler_->PostTask(task, "AcquieShareDataDone.");
    TAG_LOGI(AAFwkTag::ABILITYMGR, "acquireShareDataDone shareRuntimeTask end");
    return funcResult = NO_ERROR;
}

void AcquireShareDataCallbackStub::SetHandler(std::shared_ptr<AppExecFwk::EventHandler> handler)
{
    handler_ = handler;
}

void AcquireShareDataCallbackStub::SetShareRuntimeTask(ShareRuntimeTask &shareRuntimeTask)
{
    shareRuntimeTask_ = shareRuntimeTask;
}
} // namespace AAFwk
} // namespace OHOS