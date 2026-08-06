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

#include "agent_pre_install_event_receiver.h"

#include "agent_card_mgr.h"
#include "common_event_support.h"
#include "ffrt.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AgentRuntime {
namespace {
// Pre-installed apps live under the main user; mirror the InsightIntent/ModularObject precedent.
constexpr int32_t MAIN_USER_ID = 100;
}

AgentPreInstallEventReceiver::AgentPreInstallEventReceiver(const EventFwk::CommonEventSubscribeInfo &subscribeInfo)
    : EventFwk::CommonEventSubscriber(subscribeInfo)
{}

void AgentPreInstallEventReceiver::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    std::string action = data.GetWant().GetAction();
    if (action != EventFwk::CommonEventSupport::COMMON_EVENT_BUNDLE_SCAN_FINISHED) {
        return;
    }
    TAG_LOGI(AAFwkTag::SER_ROUTER, "bundle scan finished, trigger pre-install agent card backfill");
    // Offload to ffrt so the common-event delivery thread is not blocked by BMS + DB work.
    ffrt::submit([] { AgentCardMgr::GetInstance().HandlePreInstallBackfill(MAIN_USER_ID); });
}
} // namespace AgentRuntime
} // namespace OHOS
