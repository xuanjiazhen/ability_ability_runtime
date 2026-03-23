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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ability_connect_callback_interface.h"
#include "agent_card.h"
#include "agent_extension_connection_constants.h"

#define private public
#include "ability_manager_errors.h"
#include "agent_bundle_event_callback.h"
#include "agent_manager_service.h"
#include "agent_load_callback.h"
#include "iagent_receiver.h"
#include "hilog_tag_wrapper.h"
#undef private
#include "iremote_object.h"
#include "mock_my_flag.h"
#include "system_ability.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AgentRuntime;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AgentRuntime {
const int BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;

class AgentManagerServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void AgentManagerServiceTest::SetUpTestCase(void)
{}

void AgentManagerServiceTest::TearDownTestCase(void)
{}

void AgentManagerServiceTest::SetUp(void)
{
    MyFlag::isAddSystemAbilityListenerCalled = false;
    MyFlag::isRegisterBundleEventCallbackCalled = false;
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retVerifyConnectAgentPermission = true;
    MyFlag::retVerifyGetAgentCardPermission = true;
    MyFlag::retVerifyModifyAgentCardPermission = true;
    MyFlag::retRegisterAgentCard = ERR_OK;
    MyFlag::retUpdateAgentCard = ERR_OK;
    MyFlag::retDeleteAgentCard = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::agentCardBundleName = "test.bundle";
    MyFlag::agentCardModuleName = "test.module";
    MyFlag::agentCardAbilityName = "test.ability";
    MyFlag::connectAbilityWithExtensionTypeCallCount = 0;
    MyFlag::disconnectAbilityCallCount = 0;
    MyFlag::lastConnectAbilityConnection = nullptr;
    MyFlag::lastDisconnectAbilityConnection = nullptr;
    AgentManagerService::GetInstance()->workflowHostSessions_.clear();
    AgentManagerService::GetInstance()->workflowAgentOwners_.clear();
}

void AgentManagerServiceTest::TearDown(void)
{}

/**
* @tc.name  : GetInstance_ShouldReturnNewInstance_WhenCalledFirstTime
* @tc.number: GetInstance_001
* @tc.desc  : Test that GetInstance returns a new instance when called for the first time
*/
HWTEST_F(AgentManagerServiceTest, GetInstance_001, TestSize.Level1)
{
    sptr<AgentManagerService> instance = AgentManagerService::GetInstance();
    EXPECT_NE(instance, nullptr);
}

/**
* @tc.name  : GetInstance_ShouldReturnSameInstance_WhenCalledSubsequently
* @tc.number: GetInstance_002
* @tc.desc  : Test that GetInstance returns the same instance when called subsequently
*/
HWTEST_F(AgentManagerServiceTest, GetInstance_002, TestSize.Level1)
{
    sptr<AgentManagerService> instance1 = AgentManagerService::GetInstance();
    sptr<AgentManagerService> instance2 = AgentManagerService::GetInstance();
    EXPECT_EQ(instance1, instance2);
}

/**
* @tc.name  : GetInstance_ShouldReturnNullptr_WhenMemoryAllocationFails
* @tc.number: GetInstance_003
* @tc.desc  : Test that GetInstance returns nullptr when memory allocation fails
*/
HWTEST_F(AgentManagerServiceTest, GetInstance_003, TestSize.Level1)
{
    sptr<AgentManagerService> instance = sptr<AgentManagerService>::MakeSptr();
    AgentManagerService::instance_ = instance;
    auto outputInstance = AgentManagerService::GetInstance();
    EXPECT_EQ(instance, outputInstance);
}

/**
* @tc.name  : OnStart_ShouldLogPublishFailure_WhenPublishFails
* @tc.number: OnStart_001
* @tc.desc  : Test that OnStart logs a publish failure message when Publish fails.
*/
HWTEST_F(AgentManagerServiceTest, OnStart_001, TestSize.Level1)
{
    AgentManagerService::GetInstance()->OnStart();
    EXPECT_FALSE(MyFlag::isAddSystemAbilityListenerCalled);
}

/**
* @tc.name  : OnStart_ShouldLogAddBundleMgrFailure_WhenAddSystemAbilityListenerFails
* @tc.number: OnStart_002
* @tc.desc  : Test that OnStart logs an addBundleMgr failure message when AddSystemAbilityListener fails.
*/
HWTEST_F(AgentManagerServiceTest, OnStart_002, TestSize.Level1)
{
    MyFlag::retPublish = true;
    MyFlag::retAddSystemAbilityListener = false;
    AgentManagerService::GetInstance()->OnStart();
    EXPECT_TRUE(MyFlag::isAddSystemAbilityListenerCalled);
}

/**
* @tc.name  : OnStart_ShouldLogStartAndInitialize_WhenPublishAndAddSystemAbilityListenerSucceed
* @tc.number: OnStart_003
* @tc.desc  : Test when Publish and AddSystemAbilityListener succeed.
*/
HWTEST_F(AgentManagerServiceTest, OnStart_003, TestSize.Level1)
{
    MyFlag::retPublish = true;
    MyFlag::retAddSystemAbilityListener = true;
    AgentManagerService::GetInstance()->OnStart();
    EXPECT_TRUE(MyFlag::isAddSystemAbilityListenerCalled);
}

/**
* @tc.name  : OnAddSystemAbility_001
* @tc.number: OnAddSystemAbility_001
* @tc.desc  : OnAddSystemAbility_001
*/
HWTEST_F(AgentManagerServiceTest, OnAddSystemAbility_001, TestSize.Level1)
{
    int32_t invalidSystemAbilityId = BUNDLE_MGR_SERVICE_SYS_ABILITY_ID + 1;
    AgentManagerService::GetInstance()->OnAddSystemAbility(invalidSystemAbilityId, "123");
    EXPECT_FALSE(MyFlag::isRegisterBundleEventCallbackCalled);
}

/**
* @tc.name  : OnAddSystemAbility_002
* @tc.number: OnAddSystemAbility_002
* @tc.desc  : OnAddSystemAbility_002
*/
HWTEST_F(AgentManagerServiceTest, OnAddSystemAbility_002, TestSize.Level1)
{
    int32_t systemAbilityId = BUNDLE_MGR_SERVICE_SYS_ABILITY_ID;
    AgentManagerService::GetInstance()->bundleEventCallback_ = nullptr;
    AgentManagerService::GetInstance()->OnAddSystemAbility(systemAbilityId, "123");
    EXPECT_TRUE(MyFlag::isRegisterBundleEventCallbackCalled);
}

/**
* @tc.name  : RegisterBundleEventCallback_ShouldNotRegister_WhenCallbackAlreadyExists
* @tc.number: RegisterBundleEventCallback_001
* @tc.desc  : Test that the function returns immediately when the callback is already registered.
*/
HWTEST_F(AgentManagerServiceTest, RegisterBundleEventCallback_001, TestSize.Level1)
{
    // Set the bundleEventCallback_ to a non-null value
    AgentManagerService::GetInstance()->bundleEventCallback_ = sptr<AgentBundleEventCallback>::MakeSptr();

    // Call the function
    AgentManagerService::GetInstance()->RegisterBundleEventCallback();

    // Verify that the callback is not registered again
    EXPECT_NE(AgentManagerService::GetInstance()->bundleEventCallback_, nullptr);
    EXPECT_FALSE(MyFlag::isRegisterBundleEventCallbackCalled);
}

/**
* @tc.name  : RegisterBundleEventCallback_ShouldRegisterSuccessfully_WhenCallbackNotRegistered
* @tc.number: RegisterBundleEventCallback_002
* @tc.desc  : Test that the function successfully registers the callback when it is not already registered.
*/
HWTEST_F(AgentManagerServiceTest, RegisterBundleEventCallback_002, TestSize.Level1)
{
    // Ensure bundleEventCallback_ is nullptr
    AgentManagerService::GetInstance()->bundleEventCallback_ = nullptr;
    MyFlag::retRegisterBundleEventCallback = true;

    // Call the function
    AgentManagerService::GetInstance()->RegisterBundleEventCallback();

    // Verify that the callback is registered
    EXPECT_NE(AgentManagerService::GetInstance()->bundleEventCallback_, nullptr);
    EXPECT_TRUE(MyFlag::isRegisterBundleEventCallbackCalled);
}

/**
* @tc.name  : RegisterBundleEventCallback_ShouldNotRegister_WhenRegistrationFails
* @tc.number: RegisterBundleEventCallback_003
* @tc.desc  : Test that the function does not register the callback when the registration attempt fails.
*/
HWTEST_F(AgentManagerServiceTest, RegisterBundleEventCallback_003, TestSize.Level1)
{
    // Ensure bundleEventCallback_ is nullptr
    AgentManagerService::GetInstance()->bundleEventCallback_ = nullptr;
    MyFlag::retRegisterBundleEventCallback = false;

    // Call the function
    AgentManagerService::GetInstance()->RegisterBundleEventCallback();

    // Verify that the callback is registered
    EXPECT_EQ(AgentManagerService::GetInstance()->bundleEventCallback_, nullptr);
    EXPECT_TRUE(MyFlag::isRegisterBundleEventCallbackCalled);
}

/**
* @tc.name  : GetAllAgentCards
* @tc.number: GetAllAgentCards_001
* @tc.desc  : GetAllAgentCards_001
*/
HWTEST_F(AgentManagerServiceTest, GetAllAgentCards_001, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    AgentCardsRawData rawData;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAllAgentCards(rawData), ERR_OK);
}

/**
* @tc.name  : GetAllAgentCards_002
* @tc.number: GetAllAgentCards_002
* @tc.desc  : Test GetAllAgentCards when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, GetAllAgentCards_002, TestSize.Level1)
{
    MyFlag::retVerifyGetAgentCardPermission = false;
    AgentCardsRawData rawData;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAllAgentCards(rawData), ERR_PERMISSION_DENIED);
    MyFlag::retVerifyGetAgentCardPermission = true;
}

/**
* @tc.name  : GetAgentCardsByBundleName
* @tc.number: GetAgentCardsByBundleName_001
* @tc.desc  : GetAgentCardsByBundleName_001
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardsByBundleName_001, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetAgentCardsByBundleName = ERR_NAME_NOT_FOUND;
    MyFlag::retGetApplicationInfo = true;
    std::string bundleName = "bundle";
    std::vector<AgentCard> cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards), ERR_OK);
    MyFlag::retGetAgentCardsByBundleName = ERR_OK;
}

/**
* @tc.name  : GetAgentCardsByBundleName_002
* @tc.number: GetAgentCardsByBundleName_002
* @tc.desc  : Test GetAgentCardsByBundleName when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardsByBundleName_002, TestSize.Level1)
{
    MyFlag::retVerifyGetAgentCardPermission = false;
    std::string bundleName = "bundle";
    std::vector<AgentCard> cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards), ERR_PERMISSION_DENIED);
    MyFlag::retVerifyGetAgentCardPermission = true;
}

/**
* @tc.name  : GetAgentCardsByBundleName_003
* @tc.number: GetAgentCardsByBundleName_003
* @tc.desc  : Test GetAgentCardsByBundleName when GetAgentCardsByBundleName returns error
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardsByBundleName_003, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetAgentCardsByBundleName = ERR_INVALID_VALUE;
    std::string bundleName = "bundle";
    std::vector<AgentCard> cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards), ERR_INVALID_VALUE);
    MyFlag::retGetAgentCardsByBundleName = ERR_OK;
}

/**
* @tc.name  : GetAgentCardsByBundleName_004
* @tc.number: GetAgentCardsByBundleName_004
* @tc.desc  : Test GetAgentCardsByBundleName success case
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardsByBundleName_004, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetAgentCardsByBundleName = ERR_OK;
    std::string bundleName = "bundle";
    std::vector<AgentCard> cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards), ERR_OK);
}

/**
* @tc.name  : GetAgentCardsByBundleName_005
* @tc.number: GetAgentCardsByBundleName_005
* @tc.desc  : Test GetAgentCardsByBundleName when GetApplicationInfo returns false (bundle doesn't exist)
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardsByBundleName_005, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetAgentCardsByBundleName = ERR_NAME_NOT_FOUND;
    MyFlag::retGetApplicationInfo = false;
    std::string bundleName = "bundle";
    std::vector<AgentCard> cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards),
        AAFwk::ERR_BUNDLE_NOT_EXIST);
    MyFlag::retGetAgentCardsByBundleName = ERR_OK;
    MyFlag::retGetApplicationInfo = true;
}

/**
* @tc.name  : GetAgentCardByAgentId
* @tc.number: GetAgentCardByAgentId_001
* @tc.desc  : GetAgentCardByAgentId_001
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardByAgentId_001, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetBundleNameByPid = ERR_OK;
    MyFlag::retGetAgentCardByAgentId = ERR_NAME_NOT_FOUND;
    MyFlag::retGetApplicationInfo = true;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardByAgentId(bundleName, agentId, card),
        AAFwk::ERR_INVALID_AGENT_CARD_ID);
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
}

/**
* @tc.name  : GetAgentCardByAgentId_002
* @tc.number: GetAgentCardByAgentId_002
* @tc.desc  : Test GetAgentCardByAgentId when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardByAgentId_002, TestSize.Level1)
{
    MyFlag::retVerifyGetAgentCardPermission = false;
    MyFlag::retGetBundleNameByPid = ERR_OK;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardByAgentId(bundleName, agentId, card),
        ERR_PERMISSION_DENIED);
    MyFlag::retVerifyGetAgentCardPermission = true;
}

/**
* @tc.name  : GetAgentCardByAgentId_003
* @tc.number: GetAgentCardByAgentId_003
* @tc.desc  : Test GetAgentCardByAgentId when GetAgentCardByAgentId returns error
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardByAgentId_003, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetBundleNameByPid = ERR_OK;
    MyFlag::retGetAgentCardByAgentId = ERR_INVALID_VALUE;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardByAgentId(bundleName, agentId, card), ERR_INVALID_VALUE);
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
}

/**
* @tc.name  : GetAgentCardByAgentId_004
* @tc.number: GetAgentCardByAgentId_004
* @tc.desc  : Test GetAgentCardByAgentId success case
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardByAgentId_004, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetBundleNameByPid = ERR_OK;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardByAgentId(bundleName, agentId, card), ERR_OK);
}

/**
* @tc.name  : GetAgentCardByAgentId_005
* @tc.number: GetAgentCardByAgentId_005
* @tc.desc  : Test GetAgentCardByAgentId when GetApplicationInfo returns false (bundle doesn't exist)
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardByAgentId_005, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetBundleNameByPid = ERR_OK;
    MyFlag::retGetAgentCardByAgentId = ERR_NAME_NOT_FOUND;
    MyFlag::retGetApplicationInfo = false;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardByAgentId(bundleName, agentId, card),
        AAFwk::ERR_BUNDLE_NOT_EXIST);
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retGetApplicationInfo = true;
}

/**
* @tc.name  : UpdateAgentCard_001
* @tc.number: UpdateAgentCard_001
* @tc.desc  : Test UpdateAgentCard when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, UpdateAgentCard_001, TestSize.Level1)
{
    MyFlag::retVerifyModifyAgentCardPermission = false;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->UpdateAgentCard(card), ERR_PERMISSION_DENIED);
    MyFlag::retVerifyModifyAgentCardPermission = true;
}

/**
* @tc.name  : RegisterAgentCard_001
* @tc.number: RegisterAgentCard_001
* @tc.desc  : Test RegisterAgentCard when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, RegisterAgentCard_001, TestSize.Level1)
{
    MyFlag::retVerifyModifyAgentCardPermission = false;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->RegisterAgentCard(card), ERR_PERMISSION_DENIED);
    MyFlag::retVerifyModifyAgentCardPermission = true;
}

/**
* @tc.name  : RegisterAgentCard_002
* @tc.number: RegisterAgentCard_002
* @tc.desc  : Test RegisterAgentCard propagates manager error
*/
HWTEST_F(AgentManagerServiceTest, RegisterAgentCard_002, TestSize.Level1)
{
    MyFlag::retRegisterAgentCard = AAFwk::ERR_AGENT_CARD_DUPLICATE_REGISTER;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->RegisterAgentCard(card),
        AAFwk::ERR_AGENT_CARD_DUPLICATE_REGISTER);
}

/**
* @tc.name  : RegisterAgentCard_003
* @tc.number: RegisterAgentCard_003
* @tc.desc  : Test RegisterAgentCard success case
*/
HWTEST_F(AgentManagerServiceTest, RegisterAgentCard_003, TestSize.Level1)
{
    MyFlag::retRegisterAgentCard = ERR_OK;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->RegisterAgentCard(card), ERR_OK);
}

/**
* @tc.name  : UpdateAgentCard_002
* @tc.number: UpdateAgentCard_002
* @tc.desc  : Test UpdateAgentCard propagates manager error
*/
HWTEST_F(AgentManagerServiceTest, UpdateAgentCard_002, TestSize.Level1)
{
    MyFlag::retUpdateAgentCard = AAFwk::ERR_AGENT_CARD_VERSION_TOO_OLD;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->UpdateAgentCard(card),
        AAFwk::ERR_AGENT_CARD_VERSION_TOO_OLD);
}

/**
* @tc.name  : UpdateAgentCard_003
* @tc.number: UpdateAgentCard_003
* @tc.desc  : Test UpdateAgentCard success case
*/
HWTEST_F(AgentManagerServiceTest, UpdateAgentCard_003, TestSize.Level1)
{
    MyFlag::retUpdateAgentCard = ERR_OK;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->UpdateAgentCard(card), ERR_OK);
}

/**
* @tc.name  : DeleteAgentCard_001
* @tc.number: DeleteAgentCard_001
* @tc.desc  : Test DeleteAgentCard when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, DeleteAgentCard_001, TestSize.Level1)
{
    MyFlag::retVerifyModifyAgentCardPermission = false;
    EXPECT_EQ(AgentManagerService::GetInstance()->DeleteAgentCard("bundle", "agentId"), ERR_PERMISSION_DENIED);
    MyFlag::retVerifyModifyAgentCardPermission = true;
}

/**
* @tc.name  : DeleteAgentCard_002
* @tc.number: DeleteAgentCard_002
* @tc.desc  : Test DeleteAgentCard propagates manager error
*/
HWTEST_F(AgentManagerServiceTest, DeleteAgentCard_002, TestSize.Level1)
{
    MyFlag::retDeleteAgentCard = AAFwk::ERR_INVALID_AGENT_CARD_ID;
    EXPECT_EQ(AgentManagerService::GetInstance()->DeleteAgentCard("bundle", "agentId"),
        AAFwk::ERR_INVALID_AGENT_CARD_ID);
}

/**
* @tc.name  : DeleteAgentCard_003
* @tc.number: DeleteAgentCard_003
* @tc.desc  : Test DeleteAgentCard success case
*/
HWTEST_F(AgentManagerServiceTest, DeleteAgentCard_003, TestSize.Level1)
{
    MyFlag::retDeleteAgentCard = ERR_OK;
    EXPECT_EQ(AgentManagerService::GetInstance()->DeleteAgentCard("bundle", "agentId"), ERR_OK);
}

namespace {
class MockAbilityConnection : public IRemoteStub<AAFwk::IAbilityConnection> {
public:
    MockAbilityConnection() = default;
    ~MockAbilityConnection() override = default;

    int32_t connectDoneCount = 0;
    int32_t disconnectDoneCount = 0;
    int32_t lastConnectResultCode = ERR_OK;
    int32_t lastDisconnectResultCode = ERR_OK;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject,
        int32_t resultCode) override
    {
        connectDoneCount++;
        lastConnectResultCode = resultCode;
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override
    {
        disconnectDoneCount++;
        lastDisconnectResultCode = resultCode;
    }
};

class TestWorkflowReceiver : public IRemoteStub<IAgentReceiver> {
public:
    int32_t workflowInvokedCount = 0;
    std::vector<std::string> invokedAgentIds;

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

    int32_t WorkflowInvoked(const std::string &agentId) override
    {
        workflowInvokedCount++;
        invokedAgentIds.push_back(agentId);
        return ERR_OK;
    }
};
}

/**
* @tc.name  : ConnectAgentExtensionAbility_001
* @tc.number: ConnectAgentExtensionAbility_001
* @tc.desc  : Test ConnectAgentExtensionAbility when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_001, TestSize.Level1)
{
    MyFlag::retVerifyConnectAgentPermission = false;
    AAFwk::Want want;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        ERR_PERMISSION_DENIED);
    MyFlag::retVerifyConnectAgentPermission = true;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_002
* @tc.number: ConnectAgentExtensionAbility_002
* @tc.desc  : Test ConnectAgentExtensionAbility when process is not foreground
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_002, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_BACKGROUND;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::NOT_TOP_ABILITY);
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_003
* @tc.number: ConnectAgentExtensionAbility_003
* @tc.desc  : Test ConnectAgentExtensionAbility bypasses GetAgentCard permission path and fails on missing card
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_003, TestSize.Level1)
{
    MyFlag::retVerifyConnectAgentPermission = true;
    MyFlag::retVerifyGetAgentCardPermission = false;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_NAME_NOT_FOUND;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("nonExistentAgentId"));
    want.SetBundle("test.bundle");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    // ConnectAgentExtensionAbility should not rely on the extra GetAgentCard permission check.
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::ERR_INVALID_AGENT_CARD_ID);
    MyFlag::retVerifyGetAgentCardPermission = true;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_004
* @tc.number: ConnectAgentExtensionAbility_004
* @tc.desc  : Test ConnectAgentExtensionAbility when agentId is empty
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_004, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    AAFwk::Want want;
    // agentId is not set, so it will be empty
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_005
* @tc.number: ConnectAgentExtensionAbility_005
* @tc.desc  : Test ConnectAgentExtensionAbility when connection is null
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_005, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    sptr<AAFwk::IAbilityConnection> connection = nullptr;
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_006
* @tc.number: ConnectAgentExtensionAbility_006
* @tc.desc  : Test ConnectAgentExtensionAbility when ConnectAbilityWithExtensionType fails
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_006, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::retConnectAbilityWithExtensionType = ERR_INVALID_VALUE;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        ERR_INVALID_VALUE);
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_007
* @tc.number: ConnectAgentExtensionAbility_007
* @tc.desc  : Test ConnectAgentExtensionAbility when extension ability does not exist
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_007, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retQueryExtensionAbilityInfos = false;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::RESOLVE_ABILITY_ERR);
    MyFlag::retQueryExtensionAbilityInfos = true;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_008
* @tc.number: ConnectAgentExtensionAbility_008
* @tc.desc  : Test ConnectAgentExtensionAbility when GetRunningProcessInfoByPid fails
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_008, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_INVALID_VALUE;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        ERR_INVALID_VALUE);
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_009
* @tc.number: ConnectAgentExtensionAbility_009
* @tc.desc  : Test ConnectAgentExtensionAbility success case
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_009, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_010
* @tc.number: ConnectAgentExtensionAbility_010
* @tc.desc  : Test ConnectAgentExtensionAbility when extension ability type is not AGENT
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_010, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_011
* @tc.number: ConnectAgentExtensionAbility_011
* @tc.desc  : Test ConnectAgentExtensionAbility allows atomic-service agent connect without local extension metadata
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_011, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retQueryExtensionAbilityInfos = false;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::ATOMIC_SERVICE);
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_001
* @tc.number: DisconnectAgentExtensionAbility_001
* @tc.desc  : Test DisconnectAgentExtensionAbility when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_001, TestSize.Level1)
{
    MyFlag::retVerifyConnectAgentPermission = false;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection),
        ERR_PERMISSION_DENIED);
    MyFlag::retVerifyConnectAgentPermission = true;
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_002
* @tc.number: DisconnectAgentExtensionAbility_002
* @tc.desc  : Test DisconnectAgentExtensionAbility when connection is null
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_002, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    sptr<AAFwk::IAbilityConnection> connection = nullptr;
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection),
        AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_012
* @tc.number: ConnectAgentExtensionAbility_012
* @tc.desc  : Test workflow connect rejects mismatched target with INVALID_PARAMETERS_ERR
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_012, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::WORKFLOW);
    MyFlag::agentCardBundleName = "workflow.bundle";
    MyFlag::agentCardAbilityName = "WorkflowExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("workflowAgent"));
    want.SetElementName("", "other.bundle", "WorkflowExtAbility", "entry");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_013
* @tc.number: ConnectAgentExtensionAbility_013
* @tc.desc  : Test workflow connect skips module comparison when want module is empty
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_013, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::WORKFLOW);
    MyFlag::agentCardAgentId = "workflowAgent";
    MyFlag::agentCardBundleName = "workflow.bundle";
    MyFlag::agentCardAbilityName = "WorkflowExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("workflowAgent"));
    want.SetElementName("", "workflow.bundle", "WorkflowExtAbility", "");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_014
* @tc.number: ConnectAgentExtensionAbility_014
* @tc.desc  : Test workflow connect reuses one real host connection and notifies each workflow invocation
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_014, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::WORKFLOW);
    MyFlag::agentCardBundleName = "workflow.bundle";
    MyFlag::agentCardAbilityName = "WorkflowExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("agentA"));
    wantA.SetElementName("", "workflow.bundle", "WorkflowExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);

    auto service = AgentManagerService::GetInstance();
    ASSERT_NE(MyFlag::lastConnectAbilityConnection, nullptr);
    ASSERT_EQ(service->workflowHostSessions_.size(), 1);
    auto sessionIter = service->workflowHostSessions_.begin();
    auto hostKey = sessionIter->first;

    sptr<TestWorkflowReceiver> receiver = new TestWorkflowReceiver();
    AppExecFwk::ElementName element("", "workflow.bundle", "WorkflowExtAbility", "entry");
    service->HandleWorkflowConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);
    EXPECT_EQ(receiver->workflowInvokedCount, 1);
    ASSERT_EQ(receiver->invokedAgentIds.size(), 1);
    EXPECT_EQ(receiver->invokedAgentIds[0], "agentA");
    EXPECT_EQ(connectionA->connectDoneCount, 1);

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "workflow.bundle", "WorkflowExtAbility", "entry");
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    EXPECT_EQ(receiver->workflowInvokedCount, 2);
    ASSERT_EQ(receiver->invokedAgentIds.size(), 2);
    EXPECT_EQ(receiver->invokedAgentIds[1], "agentB");
    EXPECT_EQ(connectionB->connectDoneCount, 1);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_015
* @tc.number: ConnectAgentExtensionAbility_015
* @tc.desc  : Test workflow connect rejects duplicate active agentId for the same caller
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_015, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::WORKFLOW);
    MyFlag::agentCardBundleName = "workflow.bundle";
    MyFlag::agentCardAbilityName = "WorkflowExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("agentA"));
    want.SetElementName("", "workflow.bundle", "WorkflowExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connectionA), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connectionB),
        ERR_WORKFLOW_AGENT_ALREADY_ACTIVE);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
}

/**
* @tc.name  : NotifyWorkflowAgentComplete_001
* @tc.number: NotifyWorkflowAgentComplete_001
* @tc.desc  : Test notifyWorkflowAgentComplete validates empty agentId
*/
HWTEST_F(AgentManagerServiceTest, NotifyWorkflowAgentComplete_001, TestSize.Level1)
{
    EXPECT_EQ(AgentManagerService::GetInstance()->NotifyWorkflowAgentComplete(""),
        AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : NotifyWorkflowAgentComplete_002
* @tc.number: NotifyWorkflowAgentComplete_002
* @tc.desc  : Test notifyWorkflowAgentComplete keeps host alive until the last workflow completes
*/
HWTEST_F(AgentManagerServiceTest, NotifyWorkflowAgentComplete_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::WORKFLOW);
    MyFlag::agentCardBundleName = "workflow.bundle";
    MyFlag::agentCardAbilityName = "WorkflowExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("agentA"));
    wantA.SetElementName("", "workflow.bundle", "WorkflowExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);

    AppExecFwk::ElementName element("", "workflow.bundle", "WorkflowExtAbility", "entry");
    auto hostKey = service->workflowHostSessions_.begin()->first;
    sptr<TestWorkflowReceiver> receiver = new TestWorkflowReceiver();
    service->HandleWorkflowConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "workflow.bundle", "WorkflowExtAbility", "entry");
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);

    EXPECT_EQ(service->NotifyWorkflowAgentComplete("agentA"), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    ASSERT_EQ(service->workflowHostSessions_.size(), 1);
    EXPECT_TRUE(service->workflowHostSessions_.begin()->second->activeAgentIds.count("agentB") > 0);

    EXPECT_EQ(service->NotifyWorkflowAgentComplete("agentB"), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
}

/**
* @tc.name  : WorkflowHostConnection_OnAbilityConnectDone_001
* @tc.number: WorkflowHostConnection_OnAbilityConnectDone_001
* @tc.desc  : Test WorkflowHostConnection forwards connect completion to the owning workflow session
*/
HWTEST_F(AgentManagerServiceTest, WorkflowHostConnection_OnAbilityConnectDone_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::WORKFLOW);
    MyFlag::agentCardBundleName = "workflow.bundle";
    MyFlag::agentCardAbilityName = "WorkflowExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("agentA"));
    want.SetElementName("", "workflow.bundle", "WorkflowExtAbility", "entry");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection), ERR_OK);

    ASSERT_EQ(service->workflowHostSessions_.size(), 1);
    auto sessionIter = service->workflowHostSessions_.begin();
    auto hostConnection = sessionIter->second.hostConnection;
    ASSERT_NE(hostConnection, nullptr);

    sptr<TestWorkflowReceiver> receiver = new TestWorkflowReceiver();
    AppExecFwk::ElementName element("", "workflow.bundle", "WorkflowExtAbility", "entry");
    hostConnection->OnAbilityConnectDone(element, receiver->AsObject(), ERR_OK);

    EXPECT_EQ(receiver->workflowInvokedCount, 1);
    ASSERT_EQ(receiver->invokedAgentIds.size(), 1);
    EXPECT_EQ(receiver->invokedAgentIds[0], "agentA");
    EXPECT_EQ(connection->connectDoneCount, 1);
}

/**
* @tc.name  : WorkflowHostConnection_OnAbilityDisconnectDone_001
* @tc.number: WorkflowHostConnection_OnAbilityDisconnectDone_001
* @tc.desc  : Test WorkflowHostConnection forwards disconnect completion and clears workflow session state
*/
HWTEST_F(AgentManagerServiceTest, WorkflowHostConnection_OnAbilityDisconnectDone_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::WORKFLOW);
    MyFlag::agentCardBundleName = "workflow.bundle";
    MyFlag::agentCardAbilityName = "WorkflowExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("agentA"));
    want.SetElementName("", "workflow.bundle", "WorkflowExtAbility", "entry");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection), ERR_OK);

    ASSERT_EQ(service->workflowHostSessions_.size(), 1);
    auto sessionIter = service->workflowHostSessions_.begin();
    auto hostKey = sessionIter->first;
    auto hostConnection = sessionIter->second.hostConnection;
    ASSERT_NE(hostConnection, nullptr);

    sptr<TestWorkflowReceiver> receiver = new TestWorkflowReceiver();
    AppExecFwk::ElementName element("", "workflow.bundle", "WorkflowExtAbility", "entry");
    service->HandleWorkflowConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);
    ASSERT_EQ(connection->connectDoneCount, 1);

    hostConnection->OnAbilityDisconnectDone(element, ERR_OK);

    EXPECT_EQ(connection->disconnectDoneCount, 1);
    EXPECT_TRUE(service->workflowHostSessions_.empty());
    EXPECT_TRUE(service->workflowAgentOwners_.empty());
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_005
* @tc.number: DisconnectAgentExtensionAbility_005
* @tc.desc  : Test explicit workflow disconnect clears bookkeeping and disconnects shared host once
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_005, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::WORKFLOW);
    MyFlag::agentCardBundleName = "workflow.bundle";
    MyFlag::agentCardAbilityName = "WorkflowExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("agentA"));
    wantA.SetElementName("", "workflow.bundle", "WorkflowExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();

    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);
    auto hostKey = service->workflowHostSessions_.begin()->first;
    sptr<TestWorkflowReceiver> receiver = new TestWorkflowReceiver();
    AppExecFwk::ElementName element("", "workflow.bundle", "WorkflowExtAbility", "entry");
    service->HandleWorkflowConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "workflow.bundle", "WorkflowExtAbility", "entry");
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);

    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connectionB), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    EXPECT_TRUE(service->workflowHostSessions_.begin()->second->activeAgentIds.empty());
    EXPECT_TRUE(service->workflowAgentOwners_.empty());
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_003
* @tc.number: DisconnectAgentExtensionAbility_003
* @tc.desc  : Test DisconnectAgentExtensionAbility success case
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_003, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retDisconnectAbility = ERR_OK;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection), ERR_OK);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_004
* @tc.number: DisconnectAgentExtensionAbility_004
* @tc.desc  : Test DisconnectAgentExtensionAbility when DisconnectAbility fails
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_004, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection),
        ERR_INVALID_VALUE);
    MyFlag::retDisconnectAbility = ERR_OK;
}
} // namespace AgentRuntime
} // namespace OHOS
