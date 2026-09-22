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

#ifndef OHOS_ABILITY_RUNTIME_TEST_MOCK_MEM_MGR_CLIENT_H
#define OHOS_ABILITY_RUNTIME_TEST_MOCK_MEM_MGR_CLIENT_H

#include <cstdint>
#include <vector>

namespace OHOS {
namespace Memory {
// Shared UT mock shadowing the real memmgr mem_mgr_client.h: records NotifyProcessStatus/SetCritical
// calls so tests can assert the critical-refcount pairing without real memmgrservice IPC.
class MemMgrClient {
public:
    struct NotifyStatusCall {
        int32_t pid;
        int32_t type;
        int32_t status;
        int32_t saId;
    };

    struct SetCriticalCall {
        int32_t pid;
        bool critical;
        int32_t saId;
    };

    static MemMgrClient &GetInstance()
    {
        static MemMgrClient instance;
        return instance;
    }

    int32_t NotifyProcessStatus(int32_t pid, int32_t type, int32_t status, int saId = -1)
    {
        NotifyStatusCall call = { pid, type, status, saId };
        notifyStatusCalls_.push_back(call);
        return forcedReturn_;
    }

    int32_t SetCritical(int32_t pid, bool critical, int saId = -1)
    {
        SetCriticalCall call = { pid, critical, saId };
        setCriticalCalls_.push_back(call);
        return forcedReturn_;
    }

    static void SetForcedReturn(int32_t ret)
    {
        forcedReturn_ = ret;
    }

    static void Reset()
    {
        notifyStatusCalls_.clear();
        setCriticalCalls_.clear();
        forcedReturn_ = 0;
    }

    static std::vector<NotifyStatusCall> notifyStatusCalls_;
    static std::vector<SetCriticalCall> setCriticalCalls_;
    static int32_t forcedReturn_;
};
} // namespace Memory
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_TEST_MOCK_MEM_MGR_CLIENT_H
