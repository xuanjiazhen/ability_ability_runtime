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

#ifndef OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CANCEL_LISTENER_INTERFACE_H
#define OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CANCEL_LISTENER_INTERFACE_H

#include <cstdint>

#include "iremote_broker.h"

namespace OHOS {
namespace AAFwk {
/**
 * @enum InsightIntentCancelReason
 * The reason an in-flight insight intent execution was cancelled.
 *
 * Aligned with the v0.5 design doc: the caller-facing reason is zero-arg, so the
 * "caller cancelled" bucket is CALLER_CANCELLED, while TIMEOUT and SYSTEM are
 * reserved for framework-driven cancellations.
 */
enum class InsightIntentCancelReason : int32_t {
    // The end user cancelled the execution.
    USER_CANCELLED = 0,
    // The execution exceeded its time budget.
    TIMEOUT = 1,
    // The caller (App direct or Agent via functionManager) explicitly requested the cancellation.
    CALLER_CANCELLED = 2,
    // The system cancelled the execution (resource/policy driven).
    SYSTEM = 3,
};

/**
 * @class IInsightIntentCancelListener
 * Used to notify the target ability process that an in-flight insight intent
 * has been cancelled. One listener is bound to exactly one intentId, so
 * OnCanceled only carries the reason; the target side keeps the intentId as
 * its own member.
 */
class IInsightIntentCancelListener : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.AAFwk.InsightIntentCancelListener");

    virtual void OnCanceled(int32_t reason) = 0;

    enum {
        // ipc id for OnCanceled
        ON_CANCELED = 1,
    };
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INSIGHT_INTENT_CANCEL_LISTENER_INTERFACE_H
