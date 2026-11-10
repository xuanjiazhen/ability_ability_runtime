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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CANCEL_LISTENER_STUB_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CANCEL_LISTENER_STUB_H

#include "insight_intent_cancel_listener_interface.h"
#include <iremote_stub.h>

namespace OHOS {
namespace AAFwk {
class InsightIntentCancelListenerStub : public IRemoteStub<IInsightIntentCancelListener> {
public:
    InsightIntentCancelListenerStub() = default;
    ~InsightIntentCancelListenerStub() override = default;
    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t OnCanceledInner(MessageParcel &data, MessageParcel &reply);
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CANCEL_LISTENER_STUB_H
