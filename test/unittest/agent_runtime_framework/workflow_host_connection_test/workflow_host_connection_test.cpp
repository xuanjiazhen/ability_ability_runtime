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

#include <gtest/gtest.h>

#define private public
#include "agent_manager_service.h"
#include "workflow_host_connection.h"
#undef private
#include "iagent_receiver.h"
#include "ipc_skeleton.h"
#include "mock_my_flag.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AgentRuntime;

namespace OHOS {
namespace AgentRuntime {
namespace {
class MockAbilityConnection : public IRemoteStub<AAFwk::IAbilityConnection> {
public:
    int connectDoneCount = 0;
    int disconnectDoneCount = 0;
    int32_t lastConnectResultCode = ERR_OK;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override
    {
        connectDoneCount++;
        lastConnectResultCode = resultCode;
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override
    {
        disconnectDoneCount++;
    }
};

class TestWorkflowReceiver : public IRemoteStub<IAgentReceiver> {
public:
    std::vector<std::string> invokedAgentIds;
    int workflowInvokedCount = 0;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    int32_t SendData(const sptr<IRemoteObject> &connectorProxy, const std::string &data) override
    {
        return ERR_OK;
    }

    int32_t Authorize(const sptr<IRemoteObject> &connectorProxy, const std::string &data) override
    {
        return ERR_OK;
    }

    int32_t AgentInvoked(const std::string &agentId) override
    {
        workflowInvokedCount++;
        invokedAgentIds.push_back(agentId);
        return ERR_OK;
    }
};
}  // namespace

class WorkflowHostConnectionTest : public testing::Test {
public:
    void SetUp() override
    {
        MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
        MyFlag::retDisconnectAbility = ERR_OK;
        auto service = AgentManagerService::GetInstance();
        service->trackedConnections_.clear();
        service->callerConnectionCounts_.clear();
        service->workflowHostSessions_.clear();
        service->workflowAgentOwners_.clear();
    }
};

/**
* @tc.name  : OnAbilityConnectDone_001
* @tc.number: OnAbilityConnectDone_001
* @tc.desc  : Test WorkflowHostConnection forwards connect completion to the owning workflow session
*/
HWTEST_F(WorkflowHostConnectionTest, OnAbilityConnectDone_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    WorkflowHostKey hostKey;
    hostKey.callingUid = IPCSkeleton::GetCallingUid();
    hostKey.bundleName = "workflow.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "WorkflowExtAbility";

    auto callback = sptr<MockAbilityConnection>::MakeSptr();
    auto session = std::make_shared<WorkflowHostSession>();
    session->key = hostKey;
    session->hostConnection = sptr<WorkflowHostConnection>::MakeSptr(hostKey);
    session->callbacks.push_back(callback);
    session->activeAgentIds.insert("agentA");
    session->pendingAgentIds.insert("agentA");
    service->workflowHostSessions_[hostKey] = session;
    service->workflowAgentOwners_[{hostKey.callingUid, "agentA"}] = session;
    service->callerConnectionCounts_[hostKey.callingUid] = 1;

    auto hostConnection = session->hostConnection;
    ASSERT_NE(hostConnection, nullptr);
    sptr<TestWorkflowReceiver> receiver = new TestWorkflowReceiver();
    AppExecFwk::ElementName element("", "workflow.bundle", "WorkflowExtAbility", "entry");

    hostConnection->OnAbilityConnectDone(element, receiver->AsObject(), ERR_OK);

    EXPECT_EQ(receiver->workflowInvokedCount, 1);
    ASSERT_EQ(receiver->invokedAgentIds.size(), 1);
    EXPECT_EQ(receiver->invokedAgentIds[0], "agentA");
    EXPECT_EQ(callback->connectDoneCount, 1);
    ASSERT_EQ(service->workflowHostSessions_.size(), 1);
    EXPECT_TRUE(service->workflowHostSessions_.begin()->second->isConnected);
}

/**
* @tc.name  : OnAbilityDisconnectDone_001
* @tc.number: OnAbilityDisconnectDone_001
* @tc.desc  : Test WorkflowHostConnection forwards disconnect completion and clears workflow session state
*/
HWTEST_F(WorkflowHostConnectionTest, OnAbilityDisconnectDone_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    WorkflowHostKey hostKey;
    hostKey.callingUid = IPCSkeleton::GetCallingUid();
    hostKey.bundleName = "workflow.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "WorkflowExtAbility";

    auto callback = sptr<MockAbilityConnection>::MakeSptr();
    auto session = std::make_shared<WorkflowHostSession>();
    session->key = hostKey;
    session->hostConnection = sptr<WorkflowHostConnection>::MakeSptr(hostKey);
    session->callbacks.push_back(callback);
    session->activeAgentIds.insert("agentA");
    service->workflowHostSessions_[hostKey] = session;
    service->workflowAgentOwners_[{hostKey.callingUid, "agentA"}] = session;
    service->callerConnectionCounts_[hostKey.callingUid] = 1;

    auto hostConnection = session->hostConnection;
    ASSERT_NE(hostConnection, nullptr);
    AppExecFwk::ElementName element("", "workflow.bundle", "WorkflowExtAbility", "entry");
    service->HandleWorkflowConnectDone(hostKey, element, nullptr, ERR_OK);

    hostConnection->OnAbilityDisconnectDone(element, ERR_OK);

    EXPECT_EQ(callback->disconnectDoneCount, 1);
    EXPECT_TRUE(service->workflowHostSessions_.empty());
    EXPECT_TRUE(service->workflowAgentOwners_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}
}  // namespace AgentRuntime
}  // namespace OHOS
