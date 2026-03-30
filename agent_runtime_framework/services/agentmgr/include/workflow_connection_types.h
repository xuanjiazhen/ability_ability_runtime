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

#ifndef OHOS_AGENT_RUNTIME_FRAMEWORK_WORKFLOW_CONNECTION_TYPES_H
#define OHOS_AGENT_RUNTIME_FRAMEWORK_WORKFLOW_CONNECTION_TYPES_H

#include <memory>
#include <set>
#include <string>
#include <vector>

#include "workflow_host_connection.h"

namespace OHOS {
namespace AgentRuntime {
struct WorkflowAgentOwnerKey {
    int32_t callingUid = 0;
    std::string agentId;

    bool operator<(const WorkflowAgentOwnerKey &that) const
    {
        if (callingUid != that.callingUid) {
            return callingUid < that.callingUid;
        }
        return agentId < that.agentId;
    }
};

struct WorkflowHostSession {
    WorkflowHostKey key;
    sptr<WorkflowHostConnection> hostConnection;
    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    std::set<std::string> activeAgentIds;
    std::set<std::string> pendingAgentIds;
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObject;
    int32_t resultCode = ERR_OK;
    bool isConnected = false;
    bool isDisconnecting = false;
};

struct WorkflowConnectPlan {
    WorkflowHostKey hostKey;
    sptr<WorkflowHostConnection> hostConnection;
    AppExecFwk::ElementName cachedElement;
    sptr<IRemoteObject> cachedRemoteObject;
    int32_t cachedResultCode = ERR_OK;
    bool notifyExistingConnection = false;
    bool needRealConnect = false;
    bool reservedCallerConnectionSlot = false;
};
}  // namespace AgentRuntime
}  // namespace OHOS
#endif  // OHOS_AGENT_RUNTIME_FRAMEWORK_WORKFLOW_CONNECTION_TYPES_H
