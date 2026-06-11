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

#include "ability_connect_callback_interface.h"
#include "ability_manager_errors.h"
#include "agent_base_record.h"
#include "agent_host_connection_types.h"
#include "agent_low_code_record.h"
#include "agent_standard_record.h"
#include "ipc_object_stub.h"
#include "iremote_object.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AAFwk;
using namespace OHOS::AgentRuntime;

namespace OHOS {
namespace AgentRuntime {
namespace {
class MockAbilityConnection : public IRemoteStub<AAFwk::IAbilityConnection> {
public:
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return ERR_OK;
    }

    void OnAbilityConnectDone(const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject, int32_t resultCode) override
    {}

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override
    {}
};

AgentHostKey BuildHostKey(const std::string &moduleName)
{
    AgentHostKey key;
    key.userId = 100;
    key.bundleName = "test.bundle";
    key.moduleName = moduleName;
    key.abilityName = "TestAbility";
    return key;
}

StandardAgentKey BuildStandardKey(const std::string &moduleName)
{
    StandardAgentKey key;
    key.callerUid = 200;
    key.agentId = "agentA";
    key.bundleName = "test.bundle";
    key.moduleName = moduleName;
    key.abilityName = "TestAbility";
    return key;
}
}

class AgentRecordTest : public testing::Test {};

/**
 * @tc.name  : AgentBaseRecord_001
 * @tc.number: AgentBaseRecord_001
 * @tc.desc  : Test target matching treats empty module name as the same resolved ability target
 */
HWTEST_F(AgentRecordTest, AgentBaseRecord_001, TestSize.Level1)
{
    AgentTargetInfo withModule { 100, "test.bundle", "entry", "TestAbility" };
    AgentTargetInfo withoutModule { 100, "test.bundle", "", "TestAbility" };
    AgentTargetInfo otherAbility { 100, "test.bundle", "entry", "OtherAbility" };

    EXPECT_TRUE(AgentBaseRecord::IsSameTarget(withModule, withoutModule));
    EXPECT_FALSE(AgentBaseRecord::IsSameTarget(withModule, otherAbility));
}

/**
 * @tc.name  : AgentLowCodeRecord_001
 * @tc.number: AgentLowCodeRecord_001
 * @tc.desc  : Test low-code record tracks agents and pending callbacks by host proxy
 */
HWTEST_F(AgentRecordTest, AgentLowCodeRecord_001, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto hostProxyRemote = connection->AsObject();
    AgentLowCodeRecord record(BuildHostKey("entry"));

    record.AddAgent("agentA", hostProxyRemote, connection, hostProxyRemote, true);

    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks;
    std::vector<std::string> agentIds;
    record.CollectPendingCallbacks(hostProxyRemote, callbacks, agentIds);

    ASSERT_EQ(callbacks.size(), 1);
    ASSERT_EQ(agentIds.size(), 1);
    EXPECT_EQ(callbacks[0]->AsObject(), connection->AsObject());
    EXPECT_EQ(agentIds[0], "agentA");
    EXPECT_TRUE(record.HasAgent("agentA"));
}

/**
 * @tc.name  : AgentLowCodeRecord_002
 * @tc.number: AgentLowCodeRecord_002
 * @tc.desc  : Test low-code record removes caller-owned agents without removing sibling agents
 */
HWTEST_F(AgentRecordTest, AgentLowCodeRecord_002, TestSize.Level1)
{
    auto firstConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto secondConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentLowCodeRecord record(BuildHostKey("entry"));

    record.AddAgent("agentA", firstConnection->AsObject(), firstConnection, firstConnection->AsObject(), false);
    record.AddAgent("agentB", firstConnection->AsObject(), firstConnection, firstConnection->AsObject(), false);
    record.AddAgent("agentC", secondConnection->AsObject(), secondConnection, secondConnection->AsObject(), false);

    sptr<IRemoteObject> hostProxyRemote = nullptr;
    std::vector<std::string> removedAgentIds;
    record.RemoveAgentsByCaller(firstConnection->AsObject(), removedAgentIds, hostProxyRemote);

    EXPECT_EQ(removedAgentIds.size(), 2);
    EXPECT_FALSE(record.HasAgent("agentA"));
    EXPECT_FALSE(record.HasAgent("agentB"));
    EXPECT_TRUE(record.HasAgent("agentC"));
}

/**
 * @tc.name  : AgentStandardRecord_001
 * @tc.number: AgentStandardRecord_001
 * @tc.desc  : Test standard record matching and connect-done callback promotion
 */
HWTEST_F(AgentRecordTest, AgentStandardRecord_001, TestSize.Level1)
{
    auto callback = sptr<MockAbilityConnection>::MakeSptr();
    auto serviceConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentStandardRecord record(BuildStandardKey("entry"));

    record.AddCallback(callback);
    record.AddServiceConnection(serviceConnection, callback);
    record.CollectConnectDoneCallbacks(serviceConnection->AsObject(), record.connectedCallbacks);

    EXPECT_TRUE(record.Matches(BuildStandardKey("")));
    ASSERT_EQ(record.connectedCallbacks.size(), 1);
    EXPECT_EQ(record.connectedCallbacks[0]->AsObject(), callback->AsObject());
}

/**
 * @tc.name  : AgentStandardRecord_002
 * @tc.number: AgentStandardRecord_002
 * @tc.desc  : Test standard record matching separates same uid callers by exact instanceKey
 */
HWTEST_F(AgentRecordTest, AgentStandardRecord_002, TestSize.Level1)
{
    auto cloneAKey = BuildStandardKey("entry");
    cloneAKey.callerIdentity = AgentCallerIdentity(200, "cloneA");
    cloneAKey.targetIdentity.type = static_cast<AgentCardType>(0);
    cloneAKey.targetIdentity.agentId = "agentA";
    cloneAKey.targetIdentity.bundleName = "test.bundle";
    cloneAKey.targetIdentity.moduleName = "entry";
    cloneAKey.targetIdentity.abilityName = "TestAbility";

    auto cloneBKey = cloneAKey;
    cloneBKey.callerIdentity = AgentCallerIdentity(200, "cloneB");

    AgentStandardRecord record(cloneAKey);

    EXPECT_TRUE(record.Matches(cloneAKey));
    EXPECT_FALSE(record.Matches(cloneBKey));
}
}  // namespace AgentRuntime
}  // namespace OHOS
