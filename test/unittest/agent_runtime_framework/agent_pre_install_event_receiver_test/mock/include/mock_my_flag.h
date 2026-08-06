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

#ifndef MOCK_AGENT_RUNTIME_MY_FLAG_H
#define MOCK_AGENT_RUNTIME_MY_FLAG_H

#include <atomic>
#include <cstdint>

namespace OHOS {
namespace AgentRuntime {
// Records invocations of AgentCardMgr::HandlePreInstallBackfill. Reads/writes happen across threads
// (the test thread reads; the ffrt worker thread writes inside the mock), hence atomics.
class MyFlag {
public:
    static std::atomic<bool> preInstallBackfillCalled;
    static std::atomic<int32_t> preInstallBackfillUserId;
    static std::atomic<int32_t> preInstallBackfillCallCount;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // MOCK_AGENT_RUNTIME_MY_FLAG_H
