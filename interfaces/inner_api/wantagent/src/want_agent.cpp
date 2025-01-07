/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "hilog_tag_wrapper.h"
#include "want_agent.h"
#include "want_agent_helper.h"

namespace OHOS::AbilityRuntime::WantAgent {
bool WantAgent::isMultithreadingSupported_ = false;

WantAgent::WantAgent(const std::shared_ptr<PendingWant> &pendingWant, AAFwk::WantTriggerInfo wantTriggerInfo, bool free_flag)
{
    pendingWant_ = pendingWant;
    wantTriggerInfo_ = wantTriggerInfo;
    free_flag_ = free_flag;
}

WantAgent::~WantAgent()
{
    if (free_flag_) {
        std::shared_ptr<WantAgent> wantAgent = std::make_shared<WantAgent>(pendingWant_, wantTriggerInfo_);
        WantAgentHelper::Cancel(wantAgent);
        TAG_LOGI(AAFwkTag::WANTAGENT, "ljj:free server want record");
    }
}

std::shared_ptr<PendingWant> WantAgent::GetPendingWant()
{
    return pendingWant_;
}

void WantAgent::SetPendingWant(const std::shared_ptr<PendingWant> &pendingWant)
{
    pendingWant_ = pendingWant;
}

void WantAgent::SetWantTriggerInfo(AAFwk::WantTriggerInfo wantTriggerInfo)
{
    wantTriggerInfo_ = wantTriggerInfo;
}

AAFwk::WantTriggerInfo *WantAgent::GetWantTriggerInfo()
{
    return &wantTriggerInfo_;
}

bool WantAgent::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteParcelable(pendingWant_.get())) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "parcel WriteString failed");
        return false;
    }

    if (!parcel.WriteParcelable(&wantTriggerInfo_)) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "Failed to write wantTriggerInfo_");
        return false;
    }

    return true;
}

WantAgent *WantAgent::Unmarshalling(Parcel &parcel)
{
    WantAgent *agent = new (std::nothrow) WantAgent();
    if (agent == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "read from parcel failed");
        return nullptr;
    }
    std::shared_ptr<PendingWant> pendingWant(parcel.ReadParcelable<PendingWant>());

    std::unique_ptr<AAFwk::WantTriggerInfo> WantTriggerRequest(parcel.ReadParcelable<AAFwk::WantTriggerInfo>());
    if (WantTriggerRequest == nullptr) {
        TAG_LOGE(AAFwkTag::WANTAGENT, "null WantTriggerRequest");
        return nullptr;
    }

    agent->SetPendingWant(pendingWant);
    agent->SetWantTriggerInfo(*WantTriggerRequest);

    return agent;
}

bool WantAgent::GetIsMultithreadingSupported()
{
    return isMultithreadingSupported_;
}

void WantAgent::SetIsMultithreadingSupported(bool isMultithreadingSupported)
{
    isMultithreadingSupported_ = isMultithreadingSupported;
}

}  // namespace OHOS::AbilityRuntime::WantAgent
