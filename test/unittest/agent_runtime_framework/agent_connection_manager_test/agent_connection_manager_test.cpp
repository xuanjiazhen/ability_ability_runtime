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
#include <gmock/gmock.h>

#define private public
#include "agent_connection_manager.h"
#undef private
#include "ability_connect_callback.h"
#include "ability_manager_errors.h"
#include "agent_extension_connection_constants.h"
#include "hilog_tag_wrapper.h"
#include "iremote_object.h"
#include "mock_my_flag.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AgentRuntime;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AAFwk;

namespace {
class MockAbilityConnectCallback : public AbilityConnectCallback {
public:
    MockAbilityConnectCallback() = default;
    ~MockAbilityConnectCallback() override = default;

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject,
        int resultCode) override
    {
        connectCalled = true;
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {
        disconnectCalled = true;
    }

    bool connectCalled = false;
    bool disconnectCalled = false;
};
}

namespace OHOS {
namespace AgentRuntime {
class AgentConnectionManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void AgentConnectionManagerTest::SetUpTestCase(void)
{}

void AgentConnectionManagerTest::TearDownTestCase(void)
{}

void AgentConnectionManagerTest::SetUp(void)
{
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    MyFlag::retDisconnectAgentExtensionAbility = ERR_OK;
}

void AgentConnectionManagerTest::TearDown(void)
{}

/**
* @tc.name  : GetInstance_ShouldReturnSameInstance_WhenCalledMultipleTimes
* @tc.number: GetInstance_001
* @tc.desc  : Test that GetInstance returns the same singleton instance
*/
HWTEST_F(AgentConnectionManagerTest, GetInstance_001, TestSize.Level1)
{
    auto &instance1 = AgentConnectionManager::GetInstance();
    auto &instance2 = AgentConnectionManager::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_ShouldReturnError_WhenCallbackIsNull
* @tc.number: ConnectAgentExtensionAbility_001
* @tc.desc  : Test ConnectAgentExtensionAbility returns ERR_INVALID_CALLER when callback is null
*/
HWTEST_F(AgentConnectionManagerTest, ConnectAgentExtensionAbility_001, TestSize.Level1)
{
    Want want;
    sptr<AbilityConnectCallback> callback = nullptr;
    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);
    EXPECT_EQ(result, AAFwk::ERR_INVALID_CALLER);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_ShouldReturnError_WhenAgentIdIsEmpty
* @tc.number: ConnectAgentExtensionAbility_002
* @tc.desc  : Test ConnectAgentExtensionAbility returns ERR_INVALID_VALUE when agentId is empty
*/
HWTEST_F(AgentConnectionManagerTest, ConnectAgentExtensionAbility_002, TestSize.Level1)
{
    Want want;
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_ShouldReturnError_WhenConnectFails
* @tc.number: ConnectAgentExtensionAbility_003
* @tc.desc  : Test ConnectAgentExtensionAbility returns error when AgentManagerClient connect fails
*/
HWTEST_F(AgentConnectionManagerTest, ConnectAgentExtensionAbility_003, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    MyFlag::retConnectAgentExtensionAbility = ERR_INVALID_VALUE;
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_ShouldReturnSuccess_WhenConnectSucceeds
* @tc.number: ConnectAgentExtensionAbility_004
* @tc.desc  : Test ConnectAgentExtensionAbility returns ERR_OK when connection succeeds
*/
HWTEST_F(AgentConnectionManagerTest, ConnectAgentExtensionAbility_004, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    auto result = AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_ShouldReturnError_WhenCallbackIsNull
* @tc.number: DisconnectAgentExtensionAbility_001
* @tc.desc  : Test DisconnectAgentExtensionAbility returns ERR_INVALID_VALUE when callback is null
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectAgentExtensionAbility_001, TestSize.Level1)
{
    sptr<AbilityConnectCallback> callback = nullptr;
    auto result = AgentConnectionManager::GetInstance().DisconnectAgentExtensionAbility(callback);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_ShouldReturnError_WhenCallbackNotFound
* @tc.number: DisconnectAgentExtensionAbility_002
* @tc.desc  : Test DisconnectAgentExtensionAbility returns CONNECTION_NOT_EXIST when callback not found
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectAgentExtensionAbility_002, TestSize.Level1)
{
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    auto result = AgentConnectionManager::GetInstance().DisconnectAgentExtensionAbility(callback);
    EXPECT_EQ(result, CONNECTION_NOT_EXIST);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_ShouldReturnError_WhenDisconnectFails
* @tc.number: DisconnectAgentExtensionAbility_003
* @tc.desc  : Test DisconnectAgentExtensionAbility returns error when disconnect fails
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectAgentExtensionAbility_003, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    MyFlag::retDisconnectAgentExtensionAbility = ERR_INVALID_VALUE;
    auto result = AgentConnectionManager::GetInstance().DisconnectAgentExtensionAbility(callback);
    EXPECT_EQ(result, ERR_INVALID_VALUE);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_ShouldReturnSuccess_WhenDisconnectSucceeds
* @tc.number: DisconnectAgentExtensionAbility_004
* @tc.desc  : Test DisconnectAgentExtensionAbility returns ERR_OK when disconnect succeeds
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectAgentExtensionAbility_004, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    MyFlag::retDisconnectAgentExtensionAbility = ERR_OK;
    auto result = AgentConnectionManager::GetInstance().DisconnectAgentExtensionAbility(callback);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_ShouldReturnSuccess_WhenOtherCallbacksExist
* @tc.number: DisconnectAgentExtensionAbility_005
* @tc.desc  : Test that disconnect works when multiple callbacks exist
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectAgentExtensionAbility_005, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback1 = new MockAbilityConnectCallback();
    sptr<MockAbilityConnectCallback> callback2 = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback1);
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback2);

    auto result = AgentConnectionManager::GetInstance().DisconnectAgentExtensionAbility(callback1);
    EXPECT_EQ(result, ERR_OK);
}

/**
* @tc.name  : RemoveConnection_ShouldReturnFalse_WhenConnectionNotFound
* @tc.number: RemoveConnection_001
* @tc.desc  : Test RemoveConnection returns false when connection not found
*/
HWTEST_F(AgentConnectionManagerTest, RemoveConnection_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    auto result = AgentConnectionManager::GetInstance().RemoveConnection(connection);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : DisconnectNonexistentService_ShouldReturnTrue_WhenServiceNotFound
* @tc.number: DisconnectNonexistentService_001
* @tc.desc  : Test DisconnectNonexistentService returns true and disconnects when service not found
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectNonexistentService_001, TestSize.Level1)
{
    AppExecFwk::ElementName element;
    element.SetBundleName("test.bundle");
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();

    MyFlag::retDisconnectAgentExtensionAbility = ERR_OK;
    auto result = AgentConnectionManager::GetInstance().DisconnectNonexistentService(element, connection);
    EXPECT_TRUE(result);
}

/**
* @tc.name  : DisconnectNonexistentService_ShouldReturnFalse_WhenServiceFound
* @tc.number: DisconnectNonexistentService_002
* @tc.desc  : Test DisconnectNonexistentService returns false when service is found
*/
HWTEST_F(AgentConnectionManagerTest, DisconnectNonexistentService_002, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("test.bundle", "test.module", "test.ability");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    sptr<AgentConnection> connection =
        AgentConnectionManager::GetInstance().agentConnections_.begin()->first.agentConnection;
    AppExecFwk::ElementName element;
    element.SetBundleName("test.bundle");

    auto result = AgentConnectionManager::GetInstance().DisconnectNonexistentService(element, connection);
    EXPECT_FALSE(result);
}
} // namespace AgentRuntime
} // namespace OHOS
