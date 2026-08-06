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

#ifndef OHOS_AGENT_RUNTIME_AGENT_PRE_INSTALL_EVENT_RECEIVER_H
#define OHOS_AGENT_RUNTIME_AGENT_PRE_INSTALL_EVENT_RECEIVER_H

#include <memory>

#include "common_event_data.h"
#include "common_event_subscribe_info.h"
#include "common_event_subscriber.h"

namespace OHOS {
namespace AgentRuntime {
// Subscribes to COMMON_EVENT_BUNDLE_SCAN_FINISHED so that AgentCards declared by pre-installed
// apps (whose install events fired before AgentManagerService started) are backfilled into the
// database following the existing HandleBundleInstall rules.
class AgentPreInstallEventReceiver : public EventFwk::CommonEventSubscriber {
public:
    explicit AgentPreInstallEventReceiver(const EventFwk::CommonEventSubscribeInfo &subscribeInfo);
    ~AgentPreInstallEventReceiver() = default;

    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;

    // Offloads a one-shot pre-install backfill to ffrt. Idempotent (HandleBundleInstall re-merges
    // to the same state). Called on COMMON_EVENT_BUNDLE_SCAN_FINISHED and also as an unconditional
    // first-run right after a successful subscribe, so a late-starting AgentManagerService that
    // missed the non-sticky, one-shot scan-finished event still backfills pre-installed cards.
    static void BackfillPreInstallCards();
};
} // namespace AgentRuntime
} // namespace OHOS
#endif // OHOS_AGENT_RUNTIME_AGENT_PRE_INSTALL_EVENT_RECEIVER_H
