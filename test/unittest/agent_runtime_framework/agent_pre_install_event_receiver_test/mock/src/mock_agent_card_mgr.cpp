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

#include "agent_card_mgr.h"
#include "mock_my_flag.h"

namespace OHOS {
namespace AgentRuntime {
std::atomic<bool> MyFlag::preInstallBackfillCalled{false};
std::atomic<int32_t> MyFlag::preInstallBackfillUserId{-1};
std::atomic<int32_t> MyFlag::preInstallBackfillCallCount{0};

AgentCardMgr &AgentCardMgr::GetInstance()
{
    static AgentCardMgr instance;
    return instance;
}

AgentCardMgr::AgentCardMgr() {}

AgentCardMgr::~AgentCardMgr() {}

// Only the method the receiver dispatches to is mocked; other AgentCardMgr members are not
// referenced by agent_pre_install_event_receiver.cpp, so they need not be defined here.
int32_t AgentCardMgr::HandlePreInstallBackfill(int32_t userId)
{
    MyFlag::preInstallBackfillUserId.store(userId);
    MyFlag::preInstallBackfillCalled.store(true);
    MyFlag::preInstallBackfillCallCount.fetch_add(1);
    return 0;
}
}  // namespace AgentRuntime
}  // namespace OHOS
