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

#include "agent_connection_ledger_types.h"

namespace OHOS {
namespace AgentRuntime {
bool operator<(const AgentQuotaKey &left, const AgentQuotaKey &right)
{
    if (left.isLowCode != right.isLowCode) {
        return left.isLowCode < right.isLowCode;
    }
    if (left.hostKey < right.hostKey) {
        return true;
    }
    if (right.hostKey < left.hostKey) {
        return false;
    }
    return left.agentId < right.agentId;
}
}  // namespace AgentRuntime
}  // namespace OHOS
