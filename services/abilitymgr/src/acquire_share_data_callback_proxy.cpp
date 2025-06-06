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

#include "acquire_share_data_callback_proxy.h"

namespace OHOS {
namespace AAFwk {

ErrCode AcquireShareDataCallbackProxy::AcquireShareDataDone(
    int32_t resultCode,
    const WantParams& wantParam,
    int32_t& funcResult)
{
    MessageParcel data;
    MessageParcel reply;
    MessageOption option(MessageOption::TF_SYNC);

    if (!data.WriteInterfaceToken(GetDescriptor())) {
        return ERR_INVALID_VALUE;
    }

    if (!data.WriteInt32(resultCode)) {
        return ERR_INVALID_DATA;
    }
    if (!data.WriteParcelable(&wantParam)) {
        return ERR_INVALID_DATA;
    }

    sptr<IRemoteObject> remote = Remote();
    if (!remote) {
        return ERR_INVALID_DATA;
    }
    int32_t result = remote->SendRequest(
        static_cast<uint32_t>(IAcquireShareDataCallbackIpcCode::COMMAND_ACQUIRE_SHARE_DATA_DONE), data, reply, option);
    if (FAILED(result)) {
        return result;
    }

    ErrCode errCode = reply.ReadInt32();
    if (FAILED(errCode)) {
        return errCode;
    }

    funcResult = reply.ReadInt32();
    return ERR_OK;
}
} // namespace AAFwk
} // namespace OHOS