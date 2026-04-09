/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef OHOS_ABILITY_RUNTIME_REMOTE_INTENT_RESULT_CALLBACK_STUB_H
#define OHOS_ABILITY_RUNTIME_REMOTE_INTENT_RESULT_CALLBACK_STUB_H

#include <iremote_object.h>
#include <iremote_stub.h>
#include "nocopyable.h"

#include "remote_intent_result_callback_interface.h"

namespace OHOS {
namespace DistributedSchedule {

class RemoteIntentResultCallbackStub : public IRemoteStub<IRemoteIntentResultCallback> {
public:
    RemoteIntentResultCallbackStub();
    virtual ~RemoteIntentResultCallbackStub() = default;

    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;

private:
    DISALLOW_COPY_AND_MOVE(RemoteIntentResultCallbackStub);
    int32_t OnIntentResultInner(MessageParcel &data, MessageParcel &reply);
    int32_t OnLinkDisconnectedInner(MessageParcel &data, MessageParcel &reply);
};

} // namespace DistributedSchedule
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_REMOTE_INTENT_RESULT_CALLBACK_STUB_H
