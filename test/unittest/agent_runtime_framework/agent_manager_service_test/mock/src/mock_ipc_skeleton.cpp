/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <unistd.h>
#include "ipc_skeleton.h"
#include "mock_my_flag.h"

namespace OHOS {
#ifdef CONFIG_IPC_SINGLE
using namespace IPC_SINGLE;
#endif

// The unit-test process has no active binder invoker, so the real
// IPCSkeleton::ResetCallingIdentity() returns "" (IPCSkeleton::ResetCallingIdentity in
// ipc_skeleton.cpp returns "" when IPCThreadSkeleton::GetActiveInvoker() is null). That
// makes AgentManagerCallerIdentityScope capture an empty caller identity, which the
// low-code connect/complete/disconnect verification guards reject. Override it with a
// stable non-empty token so the end-to-end low-code bookkeeping can be exercised
// in-process. SetCallingIdentity is left to the real ipc_core implementation, which is
// already a no-op (returns true) without an active invoker — except that we track the
// active-identity state so the ForCli anti-spoof test seams can distinguish the calling
// uid before vs after SetCallingIdentity (see GetCallingAppUid below).
std::string IPCSkeleton::ResetCallingIdentity()
{
    OHOS::AgentRuntime::MyFlag::setIdentityActive = false;  // identity reset to self
    return "mock-caller-identity";
}

// Keep OS caller UID and application identity independently configurable.
pid_t IPCSkeleton::GetCallingUid()
{
    if (!OHOS::AgentRuntime::MyFlag::overrideCallingUid) {
        return static_cast<pid_t>(getuid());
    }
    return static_cast<pid_t>(OHOS::AgentRuntime::MyFlag::setIdentityActive ? OHOS::AgentRuntime::MyFlag::identityUid
                                                         : OHOS::AgentRuntime::MyFlag::cliToolUid);
}

pid_t IPCSkeleton::GetCallingAppUid()
{
    if (!OHOS::AgentRuntime::MyFlag::overrideCallingAppUid) {
        return static_cast<pid_t>(getappuid());
    }
    return static_cast<pid_t>(OHOS::AgentRuntime::MyFlag::setIdentityActive ? OHOS::AgentRuntime::MyFlag::identityAppUid
                                                         : OHOS::AgentRuntime::MyFlag::cliToolAppUid);
}

bool IPCSkeleton::SetCallingIdentity(std::string &identity, bool flag)
{
    // Mirrors the real no-op (returns true without an active invoker) while toggling the
    // active-identity state the GetCallingAppUid seam relies on; retSetCallingIdentity models a
    // malformed identity string (the ForCli fail-closed branch).
    (void)identity;
    (void)flag;
    OHOS::AgentRuntime::MyFlag::setIdentityActive = true;
    return OHOS::AgentRuntime::MyFlag::retSetCallingIdentity;
}
}  // namespace OHOS
