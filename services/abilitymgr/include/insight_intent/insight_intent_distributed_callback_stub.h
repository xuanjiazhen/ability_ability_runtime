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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_DISTRIBUTED_CALLBACK_STUB_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_DISTRIBUTED_CALLBACK_STUB_H

#include "iremote_stub.h"
#include "insight_intent_execute_callback_interface.h"

namespace OHOS {
namespace AAFwk {
class InsightIntentDistributedCallbackStub : public IRemoteStub<IInsightIntentExecuteCallback> {
public:
    explicit InsightIntentDistributedCallbackStub(uint64_t key, uint64_t intentId)
        : key_(key), intentId_(intentId)
    {}

    ~InsightIntentDistributedCallbackStub() override = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        if (data.ReadInterfaceToken() != GetDescriptor()) {
            return ERR_INVALID_STATE;
        }
        switch (code) {
            case IInsightIntentExecuteCallback::ON_INSIGHT_INTENT_EXECUTE_DONE:
                return OnExecuteDoneInner(data, reply);
            default:
                return IRemoteStub<IInsightIntentExecuteCallback>::OnRemoteRequest(code, data, reply, option);
        }
    }

private:
    int32_t OnExecuteDoneInner(MessageParcel &data, MessageParcel &reply)
    {
        uint64_t key = data.ReadUint64();
        int32_t resultCode = data.ReadInt32();
        auto result = data.ReadParcelable<AppExecFwk::InsightIntentExecuteResult>();
        if (result == nullptr) {
            return ERR_FLATTEN_OBJECT;
        }
        OnExecuteDone(key, resultCode, *result);
        return ERR_OK;
    }

    uint64_t key_ = 0;
    uint64_t intentId_ = 0;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_D分布ED_CALLBACK_STUB_H