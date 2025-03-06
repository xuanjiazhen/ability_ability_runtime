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

#ifndef OHOS_ABILITY_RUNTIME_ASYNC_RESULT_H
#define OHOS_ABILITY_RUNTIME_ASYNC_RESULT_H

#include <functional>

#include "iremote_broker.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AAFwk {
struct PrepareTerminateResult {
    int32_t pid = 0;
    bool isOk = false;
};

class IAsyncResult : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.AsyncResult");
    virtual void OnResult(const PrepareTerminateResult &result) {};

    enum AsyncResultCmd {
        ON_PREPARE_TERMINATE_RESULT = 0,
        CMD_MAX
    };
};

class AsyncResultStub : public IRemoteStub<IAsyncResult> {
public:
    virtual int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    DISALLOW_COPY_AND_MOVE(AsyncResultStub);

    int OnPrepareTerminateResult(MessageParcel &data, MessageParcel &reply);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ASYNC_RESULT_H
