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

#include "async_result.h"

#include "iremote_proxy.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
class AsyncResultProxy : public IRemoteProxy<IAsyncResult> {
public:
    explicit AsyncResultProxy(const sptr<IRemoteObject> &impl) : IRemoteProxy<IAsyncResult>(impl)
    {}

    virtual void OnResult(const PrepareTerminateResult &result) override
    {
        SendRequestCommon(ON_PREPARE_TERMINATE_RESULT, [result](MessageParcel &data) {
            data.WriteInt32(result.pid);
            data.WriteBool(result.isOk);
            });
    }
private:
    void SendRequestCommon(AsyncResultCmd cmd, const std::function<void(MessageParcel &)> &fillDataFunc)
    {
        sptr<IRemoteObject> remote = Remote();
        if (remote == nullptr) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "null remote object");
            return;
        }

        MessageParcel data;
        MessageParcel reply;
        MessageOption option(MessageOption::TF_ASYNC);
        if (!data.WriteInterfaceToken(IAsyncResult::GetDescriptor())) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "write interface token failed");
            return;
        }

        fillDataFunc(data);
        int error = remote->SendRequest(cmd, data, reply, option);
        if (error != NO_ERROR) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "SendRequest fail, error: %{public}d", error);
            return;
        }
    }

private:
    static inline BrokerDelegator<AsyncResultProxy> delegator_;
};

int32_t AsyncResultStub::OnPrepareTerminateResult(MessageParcel &data, MessageParcel &reply)
{
    OnResult({data.ReadInt32(), data.ReadBool()});
    return NO_ERROR;
}

int32_t AsyncResultStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string descriptor = AsyncResultStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (descriptor != remoteDescriptor) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "local descriptor invalid");
        return ERR_INVALID_STATE;
    }

    if (code < CMD_MAX && code >= 0) {
        switch (code) {
            case ON_PREPARE_TERMINATE_RESULT:
                return OnPrepareTerminateResult(data, reply);
                break;
        }
    }

    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}
}  // namespace AAFwk
}  // namespace OHOS
