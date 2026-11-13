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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CANCEL_LISTENER_PROXY_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CANCEL_LISTENER_PROXY_H

#include "iremote_proxy.h"
#include "insight_intent_cancel_listener_interface.h"

namespace OHOS {
namespace AAFwk {
class InsightIntentCancelListenerProxy : public IRemoteProxy<IInsightIntentCancelListener> {
public:
    explicit InsightIntentCancelListenerProxy(const sptr<IRemoteObject> &impl)
        : IRemoteProxy<IInsightIntentCancelListener>(impl)
    {}
    virtual ~InsightIntentCancelListenerProxy() = default;

    void OnCanceled(int32_t reason) override;

private:
    static inline BrokerDelegator<InsightIntentCancelListenerProxy> delegator_;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CANCEL_LISTENER_PROXY_H
