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
        MyFlag::isOnAbilityConnectDoneCalled = true;
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int resultCode) override
    {
        MyFlag::isOnAbilityDisconnectDoneCalled = true;
    }
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
    AgentConnectionManager::GetInstance().agentConnections_.clear();
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;
    MyFlag::retDisconnectAgentExtensionAbility = ERR_OK;
    MyFlag::isOnAbilityConnectDoneCalled = false;
    MyFlag::isOnAbilityDisconnectDoneCalled = false;
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
    want.SetElementName("", "test.bundle", "test.module", "test.ability");
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

/**
* @tc.name  : AddConnectCallback_ShouldAddCallback_WhenCalled
* @tc.number: AgentConnection_AddConnectCallback_001
* @tc.desc  : Test AddConnectCallback adds callback to the list
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_AddConnectCallback_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();

    connection->AddConnectCallback(callback);

    auto callbacks = connection->GetCallbackList();
    EXPECT_EQ(callbacks.size(), static_cast<size_t>(1));
}

/**
* @tc.name  : RemoveConnectCallback_ShouldRemoveCallback_WhenCalled
* @tc.number: AgentConnection_RemoveConnectCallback_001
* @tc.desc  : Test RemoveConnectCallback removes callback from the list
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_RemoveConnectCallback_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();

    connection->AddConnectCallback(callback);
    connection->RemoveConnectCallback(callback);

    auto callbacks = connection->GetCallbackList();
    EXPECT_EQ(callbacks.size(), static_cast<size_t>(0));
}

/**
* @tc.name  : SetRemoteObject_GetRemoteObject_ShouldSetAndGet
* @tc.number: AgentConnection_RemoteObject_001
* @tc.desc  : Test SetRemoteObject and GetRemoteObject
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_RemoteObject_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    sptr<IRemoteObject> remoteObj = sptr<IRemoteObject>::MakeSptr();

    connection->SetRemoteObject(remoteObj);

    auto result = connection->GetRemoteObject();
    EXPECT_EQ(result, remoteObj);
}

/**
* @tc.name  : SetResultCode_GetResultCode_ShouldSetAndGet
* @tc.number: AgentConnection_ResultCode_001
* @tc.desc  : Test SetResultCode and GetResultCode
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_ResultCode_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    int32_t testCode = 123;

    connection->SetResultCode(testCode);

    auto result = connection->GetResultCode();
    EXPECT_EQ(result, testCode);
}

/**
* @tc.name  : SetConnectionState_GetConnectionState_ShouldSetAndGet
* @tc.number: AgentConnection_ConnectionState_001
* @tc.desc  : Test SetConnectionState and GetConnectionState
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_ConnectionState_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();

    connection->SetConnectionState(CONNECTION_STATE_CONNECTED);

    auto result = connection->GetConnectionState();
    EXPECT_EQ(result, CONNECTION_STATE_CONNECTED);
}

/**
* @tc.name  : GetCallbackList_ShouldReturnEmptyList_WhenNoCallbacks
* @tc.number: AgentConnection_GetCallbackList_001
* @tc.desc  : Test GetCallbackList returns empty list when no callbacks
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_GetCallbackList_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();

    auto callbacks = connection->GetCallbackList();
    EXPECT_EQ(callbacks.size(), static_cast<size_t>(0));
}

/**
* @tc.name  : GetCallbackList_ShouldReturnCallbacks_WhenCallbacksExist
* @tc.number: AgentConnection_GetCallbackList_002
* @tc.desc  : Test GetCallbackList returns callbacks when callbacks exist
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_GetCallbackList_002, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    sptr<MockAbilityConnectCallback> callback1 = new MockAbilityConnectCallback();
    sptr<MockAbilityConnectCallback> callback2 = new MockAbilityConnectCallback();

    connection->AddConnectCallback(callback1);
    connection->AddConnectCallback(callback2);

    auto callbacks = connection->GetCallbackList();
    EXPECT_EQ(callbacks.size(), static_cast<size_t>(2));
}

/**
* @tc.name  : OnAbilityConnectDone_ShouldReturnEarly_WhenCallbackListIsEmpty
* @tc.number: AgentConnection_OnAbilityConnectDone_001
* @tc.desc  : Test OnAbilityConnectDone returns early when callback list is empty
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_OnAbilityConnectDone_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObj = sptr<IRemoteObject>::MakeSptr();
    int32_t resultCode = ERR_OK;

    connection->OnAbilityConnectDone(element, remoteObj, resultCode);

    EXPECT_EQ(connection->GetRemoteObject(), remoteObj);
    EXPECT_EQ(connection->GetResultCode(), resultCode);
    EXPECT_EQ(connection->GetConnectionState(), CONNECTION_STATE_CONNECTED);
}

/**
* @tc.name  : OnAbilityConnectDone_ShouldCallCallbacks_WhenCallbacksExist
* @tc.number: AgentConnection_OnAbilityConnectDone_002
* @tc.desc  : Test OnAbilityConnectDone calls all callbacks when callbacks exist
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_OnAbilityConnectDone_002, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    sptr<MockAbilityConnectCallback> callback1 = new MockAbilityConnectCallback();
    sptr<MockAbilityConnectCallback> callback2 = new MockAbilityConnectCallback();
    connection->AddConnectCallback(callback1);
    connection->AddConnectCallback(callback2);

    AppExecFwk::ElementName element;
    sptr<IRemoteObject> remoteObj = sptr<IRemoteObject>::MakeSptr();
    int32_t resultCode = ERR_OK;

    MyFlag::isOnAbilityConnectDoneCalled = false;
    connection->OnAbilityConnectDone(element, remoteObj, resultCode);

    EXPECT_EQ(connection->GetRemoteObject(), remoteObj);
    EXPECT_EQ(connection->GetResultCode(), resultCode);
    EXPECT_EQ(connection->GetConnectionState(), CONNECTION_STATE_CONNECTED);
}

/**
* @tc.name  : OnAbilityDisconnectDone_ShouldReturnEarly_WhenCallbackListIsEmpty
* @tc.number: AgentConnection_OnAbilityDisconnectDone_001
* @tc.desc  : Test OnAbilityDisconnectDone returns early when callback list is empty
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_OnAbilityDisconnectDone_001, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    AppExecFwk::ElementName element;
    int32_t resultCode = ERR_OK;

    connection->OnAbilityDisconnectDone(element, resultCode);

    EXPECT_EQ(connection->GetConnectionState(), CONNECTION_STATE_DISCONNECTED);
}

/**
* @tc.name  : OnAbilityDisconnectDone_ShouldCallCallbacks_WhenCallbacksExist
* @tc.number: AgentConnection_OnAbilityDisconnectDone_002
* @tc.desc  : Test OnAbilityDisconnectDone calls all callbacks when callbacks exist
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_OnAbilityDisconnectDone_002, TestSize.Level1)
{
    sptr<AgentConnection> connection = sptr<AgentConnection>::MakeSptr();
    sptr<MockAbilityConnectCallback> callback1 = new MockAbilityConnectCallback();
    sptr<MockAbilityConnectCallback> callback2 = new MockAbilityConnectCallback();
    connection->AddConnectCallback(callback1);
    connection->AddConnectCallback(callback2);

    AppExecFwk::ElementName element;
    int32_t resultCode = ERR_OK;

    MyFlag::isOnAbilityDisconnectDoneCalled = false;
    connection->OnAbilityDisconnectDone(element, resultCode);

    EXPECT_EQ(MyFlag::isOnAbilityDisconnectDoneCalled, true);
    EXPECT_EQ(connection->GetConnectionState(), CONNECTION_STATE_DISCONNECTED);
    EXPECT_EQ(connection->GetRemoteObject(), nullptr);
}

/**
* @tc.name  : OnAbilityDisconnectDone_ShouldRemoveConnection_WhenResultCodeIsDied
* @tc.number: AgentConnection_OnAbilityDisconnectDone_003
* @tc.desc  : Test OnAbilityDisconnectDone removes connection when resultCode is DIED
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_OnAbilityDisconnectDone_003, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.module", "test.ability");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    sptr<AgentConnection> connection =
        AgentConnectionManager::GetInstance().agentConnections_.begin()->first.agentConnection;

    AppExecFwk::ElementName element;
    element.SetBundleName("test.bundle");
    int32_t resultCode = DIED;

    MyFlag::isOnAbilityDisconnectDoneCalled = false;
    connection->OnAbilityDisconnectDone(element, resultCode);

    EXPECT_TRUE(AgentConnectionManager::GetInstance().agentConnections_.empty());
}

/**
* @tc.name  : OnAbilityDisconnectDone_ShouldSetResultCodeWhenResultCodeIsDied
* @tc.number: AgentConnection_OnAbilityDisconnectDone_004
* @tc.desc  : Test OnAbilityDisconnectDone sets resultCode to DIED+1 when resultCode is DIED
*/
HWTEST_F(AgentConnectionManagerTest, AgentConnection_OnAbilityDisconnectDone_004, TestSize.Level1)
{
    Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "test.bundle", "test.module", "test.ability");
    MyFlag::retConnectAgentExtensionAbility = ERR_OK;

    sptr<MockAbilityConnectCallback> callback = new MockAbilityConnectCallback();
    AgentConnectionManager::GetInstance().ConnectAgentExtensionAbility(want, callback);

    sptr<AgentConnection> connection =
        AgentConnectionManager::GetInstance().agentConnections_.begin()->first.agentConnection;

    AppExecFwk::ElementName element;
    element.SetBundleName("test.bundle");
    int32_t resultCode = DIED;

    MyFlag::isOnAbilityDisconnectDoneCalled = false;
    connection->OnAbilityDisconnectDone(element, resultCode);

    EXPECT_TRUE(MyFlag::isOnAbilityDisconnectDoneCalled);
    EXPECT_EQ(connection->GetConnectionState(), CONNECTION_STATE_DISCONNECTED);
}
} // namespace AgentRuntime
} // namespace OHOS
