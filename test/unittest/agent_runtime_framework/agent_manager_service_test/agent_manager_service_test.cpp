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
#include "ability_manager_errors.h"
#include "agent_card.h"
#include "agent_extension_connection_constants.h"
#include "agent_service_connection.h"

#define private public
#include "agent_bundle_event_callback.h"
#include "agent_manager_service.h"
#include "agent_load_callback.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "iagent_receiver.h"
#include "ipc_object_stub.h"
#include "ipc_skeleton.h"
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

AgentCard BuildServiceTestAgentCard(const std::string &agentId)
{
    AgentCard card;
    card.agentId = agentId;
    card.type = AgentCardType::APP;
    card.name = agentId;
    card.description = "desc";
    card.version = "1.0.0";
    card.category = "productivity";
    card.defaultInputModes = { "text/plain" };
    card.defaultOutputModes = { "text/plain" };
    card.iconUrl = "http://example.com/icon.png";
    auto skill = std::make_shared<AgentSkill>();
    skill->id = agentId + "_skill";
    skill->name = "skill";
    skill->description = "skill desc";
    skill->tags = { "tag" };
    card.skills = { skill };
    card.appInfo = std::make_shared<AgentAppInfo>();
    card.appInfo->bundleName = "bundle";
    card.appInfo->moduleName = "module";
    card.appInfo->abilityName = "ability";
    return card;
}

StandardAgentKey BuildTestStandardKey()
{
    StandardAgentKey key;
    key.callerUid = 100;
    key.agentId = "testAgent";
    key.bundleName = "test.bundle";
    key.abilityName = "TestAbility";
    return key;
}

std::shared_ptr<AgentStandardRecord> AddTestStandardSession(
    const sptr<AgentManagerService> &service, const StandardAgentKey &key,
    const sptr<AgentServiceConnection> &serviceConnection, const std::vector<sptr<IRemoteObject>> &callerRemotes,
    bool isDisconnecting = false)
{
    auto session = std::make_shared<AgentStandardRecord>();
    session->key = key;
    session->serviceConnection = serviceConnection;
    session->isDisconnecting = isDisconnecting;
    session->callerRemotes = callerRemotes;
    service->standardSessions_.emplace_back(session);
    return session;
}

void AddTestStandardTrackedConnection(const sptr<AgentManagerService> &service,
    const StandardAgentKey &key, const sptr<AAFwk::IAbilityConnection> &connection,
    const sptr<AgentServiceConnection> &serviceConnection, bool countTowardsCallerLimit)
{
    TrackedConnectionRecord record;
    record.callerUid = key.callerUid;
    record.callerIdentity = AgentCallerIdentity(key.callerUid);
    record.callerRemote = connection->AsObject();
    record.serviceConnection = serviceConnection;
    record.standardKey = key;
    record.countTowardsCallerLimit = countTowardsCallerLimit;
    service->trackedConnections_[connection->AsObject()] = record;
}

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
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = true;
    MyFlag::retVerifyModifyAgentCardPermission = true;
    MyFlag::retRegisterAgentCard = ERR_OK;
    MyFlag::retUpdateAgentCard = ERR_OK;
    MyFlag::retDeleteAgentCard = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    MyFlag::retGetBundleInfo = true;
    MyFlag::retGetResConfigFile = true;
    MyFlag::mockApplicationInfoIsSystemApp = true;
    MyFlag::mockExtensionInfos.clear();
    MyFlag::mockHapModuleInfos.clear();
    MyFlag::mockProfileInfos.clear();
    MyFlag::extensionAbilityUid = IPCSkeleton::GetCallingUid();
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::runningProcessInstanceKey = "";
    MyFlag::retGetBundleNameByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardsByBundleName.clear();
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::agentCardBundleName = "test.bundle";
    MyFlag::agentCardModuleName = "";
    MyFlag::agentCardAbilityName = "TestAbility";
    MyFlag::shouldCreateAgentCardAppInfo = true;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::lastConnectAbilityWant = Want();
    MyFlag::shouldFillExtensionAbilityInfos = true;
    MyFlag::retGetApplicationInfo = false;
    MyFlag::lastConnectAbilityConnection = nullptr;
    MyFlag::lastConnectAbilityCallerToken = nullptr;
    MyFlag::lastConnectAbilityExtensionType = AppExecFwk::ExtensionAbilityType::UNSPECIFIED;
    MyFlag::lastDisconnectAbilityConnection = nullptr;
    auto service = AgentManagerService::GetInstance();
    service->trackedConnections_.clear();
    service->callerConnectionCounts_.clear();
    service->standardSessions_.clear();
    MyFlag::connectAbilityWithExtensionTypeCallCount = 0;
    MyFlag::disconnectAbilityCallCount = 0;
    AgentManagerService::GetInstance()->agentHostSessions_.clear();
    AgentManagerService::GetInstance()->agentOwners_.clear();
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
* @tc.name  : OnStop_001
* @tc.number: OnStop_001
* @tc.desc  : Test OnStop clears tracked connections and caller counts
*/
HWTEST_F(AgentManagerServiceTest, OnStop_001, TestSize.Level1)
{
    auto connection = sptr<IRemoteObject>(new (std::nothrow) IPCObjectStub(u"tracked.remote"));
    TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection, record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    AgentManagerService::GetInstance()->OnStop();

    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
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
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    AgentCardsRawData rawData;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAllAgentCards(rawData), ERR_NOT_SYSTEM_APP);
}

/**
* @tc.name  : GetAllAgentCards_002
* @tc.number: GetAllAgentCards_002
* @tc.desc  : GetAllAgentCards success
*/
HWTEST_F(AgentManagerServiceTest, GetAllAgentCards_002, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    AgentCardsRawData rawData;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAllAgentCards(rawData), ERR_OK);
}

/**
* @tc.name  : GetAllAgentCards_003
* @tc.number: GetAllAgentCards_003
* @tc.desc  : Test GetAllAgentCards when permission verification fails
*/
HWTEST_F(AgentManagerServiceTest, GetAllAgentCards_003, TestSize.Level1)
{
    MyFlag::retVerifyGetAgentCardPermission = false;
    AgentCardsRawData rawData;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAllAgentCards(rawData), ERR_PERMISSION_DENIED);
    MyFlag::retVerifyGetAgentCardPermission = true;
}

/**
* @tc.name  : GetAllAgentCards_004
* @tc.number: GetAllAgentCards_004
* @tc.desc  : Test GetAllAgentCards propagates AgentCardMgr failure
*/
HWTEST_F(AgentManagerServiceTest, GetAllAgentCards_004, TestSize.Level1)
{
    MyFlag::retGetAllAgentCards = ERR_INVALID_VALUE;
    AgentCardsRawData rawData;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAllAgentCards(rawData), ERR_INVALID_VALUE);
    MyFlag::retGetAllAgentCards = ERR_OK;
}

/**
* @tc.name  : GetAgentCardsByBundleName
* @tc.number: GetAgentCardsByBundleName_000
* @tc.desc  : Test GetAgentCardsByBundleName when caller is not allowed to use system API
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardsByBundleName_000, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    std::string bundleName = "bundle";
    AgentCardsRawData cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards), ERR_NOT_SYSTEM_APP);
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
    AgentCardsRawData cards;
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
    AgentCardsRawData cards;
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
    AgentCardsRawData cards;
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
    MyFlag::agentCardsByBundleName = { BuildServiceTestAgentCard("agent1"), BuildServiceTestAgentCard("agent2") };
    std::string bundleName = "bundle";
    AgentCardsRawData cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards), ERR_OK);
    std::vector<AgentCard> cardVec;
    EXPECT_EQ(AgentCardsRawData::ToAgentCardVec(cards, cardVec), ERR_OK);
    ASSERT_EQ(cardVec.size(), 2);
    EXPECT_EQ(cardVec[0].agentId, "agent1");
    EXPECT_EQ(cardVec[1].agentId, "agent2");
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
    AgentCardsRawData cards;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardsByBundleName(bundleName, cards),
        AAFwk::ERR_BUNDLE_NOT_EXIST);
    MyFlag::retGetAgentCardsByBundleName = ERR_OK;
    MyFlag::retGetApplicationInfo = true;
}

/**
* @tc.name  : GetAgentCardByAgentId
* @tc.number: GetAgentCardByAgentId_000
* @tc.desc  : Test GetAgentCardByAgentId when caller is not allowed to use system API
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardByAgentId_000, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    std::string bundleName = "bundle";
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetAgentCardByAgentId(bundleName, agentId, card),
        ERR_NOT_SYSTEM_APP);
}

/**
* @tc.name  : GetAgentCardByAgentId
* @tc.number: GetAgentCardByAgentId_001
* @tc.desc  : GetAgentCardByAgentId_001
*/
HWTEST_F(AgentManagerServiceTest, GetAgentCardByAgentId_001, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
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
* @tc.name  : GetCallerAgentCardByAgentId_001
* @tc.number: GetCallerAgentCardByAgentId_001
* @tc.desc  : Test GetCallerAgentCardByAgentId when GetBundleNameByPid fails
*/
HWTEST_F(AgentManagerServiceTest, GetCallerAgentCardByAgentId_001, TestSize.Level1)
{
    MyFlag::retGetBundleNameByPid = ERR_INVALID_VALUE;
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetCallerAgentCardByAgentId(agentId, card), ERR_INVALID_VALUE);
    MyFlag::retGetBundleNameByPid = ERR_OK;
}

/**
* @tc.name  : GetCallerAgentCardByAgentId_002
* @tc.number: GetCallerAgentCardByAgentId_002
* @tc.desc  : Test GetCallerAgentCardByAgentId returns invalid card id when caller bundle exists but card is missing
*/
HWTEST_F(AgentManagerServiceTest, GetCallerAgentCardByAgentId_002, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_NAME_NOT_FOUND;
    MyFlag::retGetApplicationInfo = true;
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetCallerAgentCardByAgentId(agentId, card),
        AAFwk::ERR_INVALID_AGENT_CARD_ID);
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
}

/**
* @tc.name  : GetCallerAgentCardByAgentId_003
* @tc.number: GetCallerAgentCardByAgentId_003
 * @tc.desc : Test GetCallerAgentCardByAgentId returns bundle not exist when caller bundle lookup succeeds but app info
 * is missing
*/
HWTEST_F(AgentManagerServiceTest, GetCallerAgentCardByAgentId_003, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_NAME_NOT_FOUND;
    MyFlag::retGetApplicationInfo = false;
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetCallerAgentCardByAgentId(agentId, card),
        AAFwk::ERR_BUNDLE_NOT_EXIST);
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retGetApplicationInfo = true;
}

/**
* @tc.name  : GetCallerAgentCardByAgentId_004
* @tc.number: GetCallerAgentCardByAgentId_004
* @tc.desc  : Test GetCallerAgentCardByAgentId success case
*/
HWTEST_F(AgentManagerServiceTest, GetCallerAgentCardByAgentId_004, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetCallerAgentCardByAgentId(agentId, card), ERR_OK);
}

/**
* @tc.name  : GetCallerAgentCardByAgentId_005
* @tc.number: GetCallerAgentCardByAgentId_005
* @tc.desc  : Test GetCallerAgentCardByAgentId propagates non-name-not-found errors from AgentCardMgr
*/
HWTEST_F(AgentManagerServiceTest, GetCallerAgentCardByAgentId_005, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_INVALID_VALUE;
    std::string agentId = "agentId";
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->GetCallerAgentCardByAgentId(agentId, card), ERR_INVALID_VALUE);
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
}

/**
 * @tc.name  : UpdateAgentCard_004
 * @tc.number: UpdateAgentCard_004
 * @tc.desc  : Test UpdateAgentCard when caller is not allowed to use system API
*/
HWTEST_F(AgentManagerServiceTest, UpdateAgentCard_004, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->UpdateAgentCard(card), ERR_NOT_SYSTEM_APP);
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
 * @tc.name  : RegisterAgentCard_004
 * @tc.number: RegisterAgentCard_004
 * @tc.desc  : Test RegisterAgentCard when caller is not allowed to use system API
*/
HWTEST_F(AgentManagerServiceTest, RegisterAgentCard_004, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    AgentCard card;
    EXPECT_EQ(AgentManagerService::GetInstance()->RegisterAgentCard(card), ERR_NOT_SYSTEM_APP);
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
 * @tc.name  : DeleteAgentCard_004
 * @tc.number: DeleteAgentCard_004
 * @tc.desc  : Test DeleteAgentCard when caller is not allowed to use system API
*/
HWTEST_F(AgentManagerServiceTest, DeleteAgentCard_004, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    EXPECT_EQ(AgentManagerService::GetInstance()->DeleteAgentCard("bundle", "agentId"), ERR_NOT_SYSTEM_APP);
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
    std::vector<std::string> *eventOrder = nullptr;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    void OnAbilityConnectDone(
        const AppExecFwk::ElementName &element,
        const sptr<IRemoteObject> &remoteObject,
        int32_t resultCode) override
    {
        if (eventOrder != nullptr) {
            eventOrder->emplace_back("connect");
        }
        connectDoneCount++;
        lastConnectResultCode = resultCode;
    }

    void OnAbilityDisconnectDone(const AppExecFwk::ElementName &element, int32_t resultCode) override
    {
        disconnectDoneCount++;
        lastDisconnectResultCode = resultCode;
    }
};

class TestAgentReceiver : public IRemoteStub<IAgentReceiver> {
public:
    int32_t agentInvokedCount = 0;
    std::vector<std::string> invokedAgentIds;
    std::vector<std::string> *eventOrder = nullptr;
    bool isObjectDead = false;

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return 0;
    }

    bool IsObjectDead() const override
    {
        return isObjectDead;
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
        if (eventOrder != nullptr) {
            eventOrder->emplace_back("invoke");
        }
        agentInvokedCount++;
        invokedAgentIds.push_back(agentId);
        return ERR_OK;
    }
};
}

/**
* @tc.name  : ConnectAgentExtensionAbility_000
* @tc.number: ConnectAgentExtensionAbility_000
* @tc.desc  : Test ConnectAgentExtensionAbility when caller is not allowed to use system API
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_000, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    AAFwk::Want want;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_NOT_SYSTEM_APP);
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
        ERR_INVALID_VALUE);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
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
    want.SetElementName("test.bundle", "TestAbility");
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
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::retQueryExtensionAbilityInfos = false;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
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
    want.SetElementName("test.bundle", "TestAbility");
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
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_017
 * @tc.number: ConnectAgentExtensionAbility_017
 * @tc.desc  : Test ConnectAgentExtensionAbility rejects want target mismatch with card appInfo
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_017, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "OtherAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_018
 * @tc.number: ConnectAgentExtensionAbility_018
 * @tc.desc  : Test ConnectAgentExtensionAbility ignores module mismatch when want module is absent
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_018, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_EQ(MyFlag::lastConnectAbilityWant.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND, 0);
    EXPECT_TRUE(MyFlag::lastConnectAbilityWant.GetStringParam(Want::PARAM_RESV_START_TIME).empty());
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_019
 * @tc.number: ConnectAgentExtensionAbility_019
 * @tc.desc  : Test ConnectAgentExtensionAbility accepts explicit module when card module is empty
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_019, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::agentCardModuleName = "";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("", "test.bundle", "TestAbility", "entry");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_020
 * @tc.number: ConnectAgentExtensionAbility_020
 * @tc.desc  : Test ConnectAgentExtensionAbility accepts explicit module when it matches card module
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_020, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("", "test.bundle", "TestAbility", "entry");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_021
 * @tc.number: ConnectAgentExtensionAbility_021
 * @tc.desc  : Test ConnectAgentExtensionAbility rejects explicit module mismatch with card appInfo
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_021, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("", "test.bundle", "TestAbility", "feature");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_022
 * @tc.number: ConnectAgentExtensionAbility_022
 * @tc.desc  : Test ConnectAgentExtensionAbility rejects want bundle mismatch with card appInfo
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_022, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("other.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_023
 * @tc.number: ConnectAgentExtensionAbility_023
 * @tc.desc  : Test ConnectAgentExtensionAbility rejects card without appInfo
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_023, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::shouldCreateAgentCardAppInfo = false;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
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
* @tc.name  : DisconnectAgentExtensionAbility_000
* @tc.number: DisconnectAgentExtensionAbility_000
* @tc.desc  : Test DisconnectAgentExtensionAbility when caller is not allowed to use system API
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_000, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection), ERR_NOT_SYSTEM_APP);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_014
* @tc.number: ConnectAgentExtensionAbility_014
* @tc.desc  : Test ConnectAgentExtensionAbility allows atomic-service agent connect without local extension metadata
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_014, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retQueryExtensionAbilityInfos = false;
    MyFlag::retGetBundleInfo = false;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::ATOMIC_SERVICE);
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_NE(MyFlag::lastConnectAbilityWant.GetFlags() & Want::FLAG_INSTALL_ON_DEMAND, 0);
    EXPECT_FALSE(MyFlag::lastConnectAbilityWant.GetStringParam(Want::PARAM_RESV_START_TIME).empty());
}

/**
* @tc.name  : ConnectAgentExtensionAbility_015
* @tc.number: ConnectAgentExtensionAbility_015
* @tc.desc  : Test ConnectAgentExtensionAbility returns RESOLVE_ABILITY_ERR when extension query succeeds but is empty
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_015, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::shouldFillExtensionAbilityInfos = false;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::RESOLVE_ABILITY_ERR);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_016
* @tc.number: ConnectAgentExtensionAbility_016
* @tc.desc  : Test ConnectAgentExtensionAbility returns RESOLVE_ABILITY_ERR for atomic-service when bundle exists but
*             ability metadata is missing
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_016, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::ATOMIC_SERVICE);
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::shouldFillExtensionAbilityInfos = false;
    MyFlag::retGetBundleInfo = true;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::RESOLVE_ABILITY_ERR);
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
 * @tc.name  : ConnectAgentExtensionAbility_024
 * @tc.number: ConnectAgentExtensionAbility_024
 * @tc.desc  : Test low-code connect rejects mismatched target with INVALID_PARAMETERS_ERR
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_024, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("lowCodeAgent"));
    want.SetElementName("", "other.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::ERR_WRONG_INTERFACE_CALL);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_025
 * @tc.number: ConnectAgentExtensionAbility_025
 * @tc.desc  : Test low-code connect skips module comparison when want module is empty
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_025, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardAgentId = "lowCodeAgent";
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("lowCodeAgent"));
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_031
 * @tc.number: ConnectAgentExtensionAbility_031
 * @tc.desc  : Test low-code connect reaches the low-code-specific mismatch branch when generic target match passes
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_031, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardAgentId = "lowCodeAgent";
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("lowCodeAgent"));
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection),
        AAFwk::INVALID_PARAMETERS_ERR);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_026
 * @tc.number: ConnectAgentExtensionAbility_026
 * @tc.desc  : Test low-code connect reuses one real host connection and notifies each invocation
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_026, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("agentA"));
    wantA.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);

    auto service = AgentManagerService::GetInstance();
    ASSERT_NE(MyFlag::lastConnectAbilityConnection, nullptr);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    auto sessionIter = service->agentHostSessions_.begin();
    auto hostKey = sessionIter->key;

    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);
    EXPECT_EQ(receiver->agentInvokedCount, 1);
    ASSERT_EQ(receiver->invokedAgentIds.size(), 1);
    EXPECT_EQ(receiver->invokedAgentIds[0], "agentA");
    EXPECT_EQ(connectionA->connectDoneCount, 1);

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 2);
    auto hostConnectionB = MyFlag::lastConnectAbilityConnection;
    ASSERT_NE(hostConnectionB, nullptr);
    auto *rawHostConnectionB = static_cast<AgentHostConnection *>(hostConnectionB.GetRefPtr());
    rawHostConnectionB->OnAbilityConnectDone(element, receiver->AsObject(), ERR_OK);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
    EXPECT_EQ(receiver->agentInvokedCount, 2);
    ASSERT_EQ(receiver->invokedAgentIds.size(), 2);
    EXPECT_EQ(receiver->invokedAgentIds[1], "agentB");
    EXPECT_EQ(connectionB->connectDoneCount, 1);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_039
 * @tc.number: ConnectAgentExtensionAbility_039
 * @tc.desc  : Test low-code connect reuses host session when moduleName is empty then explicit
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_039, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("agentA"));
    wantA.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();

    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_EQ(service->agentHostSessions_.begin()->key.moduleName, "entry");

    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    EXPECT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_EQ(service->agentOwners_.size(), 2);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_027
 * @tc.number: ConnectAgentExtensionAbility_027
 * @tc.desc  : Test low-code connect rejects duplicate active agentId for the same caller
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_027, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("agentA"));
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connectionA), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connectionB),
        ERR_LOW_CODE_AGENT_ALREADY_ACTIVE);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_040
 * @tc.number: ConnectAgentExtensionAbility_040
 * @tc.desc  : Test low-code connect rejects duplicate active agentId across caller instances
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_040, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("agentA"));
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connectionA), ERR_OK);

    MyFlag::runningProcessInstanceKey = "cloneB";
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connectionB), ERR_LOW_CODE_AGENT_ALREADY_ACTIVE);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_EQ(service->agentHostSessions_.begin()->session->agents.size(), 1);
    EXPECT_EQ(service->agentOwners_.size(), 1);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_041
 * @tc.number: ConnectAgentExtensionAbility_041
 * @tc.desc  : Test low-code shared host counts quota per caller identity
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_041, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("agentA"));
    wantA.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);
    auto hostKey = service->agentHostSessions_.begin()->key;
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);

    MyFlag::runningProcessInstanceKey = "cloneB";
    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);

    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 2);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_EQ(service->agentOwners_.size(), 2);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 2);
    EXPECT_EQ(service->callerConnectionCounts_[AgentCallerIdentity(IPCSkeleton::GetCallingUid(), "")], 1);
    EXPECT_EQ(service->callerConnectionCounts_[AgentCallerIdentity(IPCSkeleton::GetCallingUid(), "cloneB")], 1);
    EXPECT_TRUE(service->trackedConnections_[connectionA->AsObject()].countTowardsCallerLimit);
    EXPECT_TRUE(service->trackedConnections_[connectionB->AsObject()].countTowardsCallerLimit);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_028
 * @tc.number: ConnectAgentExtensionAbility_028
 * @tc.desc  : Test low-code host connections consume MAX_CONNECTIONS_PER_CALLER slots per distinct host
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_028, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);

    for (size_t i = 0; i < AgentManagerService::MAX_CONNECTIONS_PER_CALLER; i++) {
        std::string index = std::to_string(i);
        std::string bundleName = "lowcode.bundle." + index;
        std::string abilityName = "LowCodeExtAbility" + index;
        std::string agentId = "agent" + index;
        MyFlag::agentCardBundleName = bundleName;
        MyFlag::agentCardAbilityName = abilityName;
        MyFlag::agentCardModuleName = "entry";

        AAFwk::Want want;
        want.SetParam(AGENTID_KEY, agentId);
        want.SetElementName("", bundleName, abilityName, "entry");
        auto connection = sptr<MockAbilityConnection>::MakeSptr();
        EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    }

    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, AgentManagerService::MAX_CONNECTIONS_PER_CALLER);

    MyFlag::agentCardBundleName = "lowcode.bundle.overflow";
    MyFlag::agentCardAbilityName = "LowCodeExtAbilityOverflow";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want overflowWant;
    overflowWant.SetParam(AGENTID_KEY, std::string("agentOverflow"));
    overflowWant.SetElementName("", "lowcode.bundle.overflow", "LowCodeExtAbilityOverflow", "entry");
    auto overflowConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(overflowWant, overflowConnection),
        ERR_MAX_AGENT_CONNECTIONS_REACHED);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_044
 * @tc.number: ConnectAgentExtensionAbility_044
 * @tc.desc  : Test different processes of one caller app share one low-code quota slot for the same host session
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_044, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentCallerIdentity callerIdentity(callingUid, "");
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto existingConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->AddHostConnection(existingConnection->AsObject(), session->hostConnection);
    session->isConnected = true;
    session->element = AppExecFwk::ElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    session->CacheConnectDone(session->element, receiver->AsObject(), ERR_OK);
    session->AddAgent("agentA", existingConnection->AsObject(), existingConnection,
        existingConnection->AsObject(), false);
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });
    service->SetAgentOwnerLocked(AgentOwnerKey { callerIdentity, "agentA" }, session);

    TrackedConnectionRecord existingRecord;
    existingRecord.callerUid = callingUid;
    existingRecord.callerIdentity = callerIdentity;
    existingRecord.callerRemote = existingConnection->AsObject();
    existingRecord.serviceConnection = session->hostConnection;
    existingRecord.hostKey = hostKey;
    existingRecord.isLowCode = true;
    existingRecord.countTowardsCallerLimit = true;
    service->trackedConnections_[existingConnection->AsObject()] = existingRecord;
    service->callerConnectionCounts_[callerIdentity] = AgentManagerService::MAX_CONNECTIONS_PER_CALLER;

    AgentConnectPlan plan;
    auto newConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->PrepareLowCodeConnectPlan(hostKey, callingUid, "agentB", newConnection, callerIdentity, plan),
        ERR_OK);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_[callerIdentity], AgentManagerService::MAX_CONNECTIONS_PER_CALLER);
    ASSERT_NE(service->trackedConnections_.find(newConnection->AsObject()), service->trackedConnections_.end());
    EXPECT_FALSE(service->trackedConnections_[newConnection->AsObject()].countTowardsCallerLimit);
    EXPECT_EQ(service->agentOwners_.size(), 2);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_029
 * @tc.number: ConnectAgentExtensionAbility_029
 * @tc.desc  : Test low-code shared host session rejects more than MAX_AGENTS_PER_HOST_SESSION agents
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_029, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = IPCSkeleton::GetCallingUid();
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->isConnected = true;
    session->element = AppExecFwk::ElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    session->CacheConnectDone(session->element, sptr<TestAgentReceiver>(new TestAgentReceiver())->AsObject(), ERR_OK);
    for (size_t i = 0; i < AgentManagerService::MAX_AGENTS_PER_HOST_SESSION; i++) {
        std::string agentId = "agent" + std::to_string(i);
        session->agents[agentId] = LowCodeAgentRecord { nullptr, false };
        service->SetAgentOwnerLocked(AgentOwnerKey { session->hostUid, agentId }, session);
    }
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });

    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_EQ(service->agentHostSessions_.begin()->session->agents.size(),
        AgentManagerService::MAX_AGENTS_PER_HOST_SESSION);

    AAFwk::Want overflowWant;
    overflowWant.SetParam(AGENTID_KEY, std::string("agentOverflow"));
    overflowWant.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto overflowConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(overflowWant, overflowConnection),
        ERR_MAX_AGENT_CONNECTIONS_REACHED);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_030
 * @tc.number: ConnectAgentExtensionAbility_030
 * @tc.desc  : Test low-code shared host allows MAX_AGENTS_PER_HOST_SESSION agents
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_030, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);

    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want firstWant;
    firstWant.SetParam(AGENTID_KEY, std::string("agent0"));
    firstWant.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto firstConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(firstWant, firstConnection), ERR_OK);

    auto hostKey = service->agentHostSessions_.begin()->key;
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);

    for (size_t i = 1; i < AgentManagerService::MAX_AGENTS_PER_HOST_SESSION; i++) {
        std::string index = std::to_string(i);
        std::string agentId = "agent" + index;
        AAFwk::Want want;
        want.SetParam(AGENTID_KEY, agentId);
        want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
        auto connection = sptr<MockAbilityConnection>::MakeSptr();
        EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    }

    EXPECT_EQ(service->agentOwners_.size(), AgentManagerService::MAX_AGENTS_PER_HOST_SESSION);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    EXPECT_EQ(receiver->agentInvokedCount, AgentManagerService::MAX_AGENTS_PER_HOST_SESSION);

    AAFwk::Want overflowWant;
    overflowWant.SetParam(AGENTID_KEY, std::string("agentExtra"));
    overflowWant.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto overflowConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(overflowWant, overflowConnection),
        ERR_MAX_AGENT_CONNECTIONS_REACHED);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_042
 * @tc.number: ConnectAgentExtensionAbility_042
 * @tc.desc  : Test low-code pending owners share one real host connect before connect done
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_042, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    std::vector<sptr<MockAbilityConnection>> connections;
    for (size_t i = 0; i < AgentManagerService::MAX_AGENTS_PER_HOST_SESSION; i++) {
        std::string agentId = "agent" + std::to_string(i);
        AAFwk::Want want;
        want.SetParam(AGENTID_KEY, agentId);
        want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
        auto connection = sptr<MockAbilityConnection>::MakeSptr();
        EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection), ERR_OK);
        connections.emplace_back(connection);
    }

    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    auto hostKey = service->agentHostSessions_.begin()->key;
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);

    EXPECT_EQ(receiver->agentInvokedCount, AgentManagerService::MAX_AGENTS_PER_HOST_SESSION);
    for (const auto &connection : connections) {
        EXPECT_EQ(connection->connectDoneCount, 1);
    }
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_043
 * @tc.number: ConnectAgentExtensionAbility_043
 * @tc.desc  : Test low-code connect clears a stale host before reconnecting the same AgentId
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_043, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("agentA"));
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto firstConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, firstConnection), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);

    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    auto hostKey = service->agentHostSessions_.begin()->key;
    auto receiver = sptr<TestAgentReceiver>::MakeSptr();
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);
    receiver->isObjectDead = true;

    auto secondConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, secondConnection), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 2);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_TRUE(service->trackedConnections_.count(firstConnection->AsObject()) == 0);
    EXPECT_TRUE(service->trackedConnections_.count(secondConnection->AsObject()) > 0);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_032
 * @tc.number: ConnectAgentExtensionAbility_032
 * @tc.desc  : Test reused low-code host session ignores caller-wide quota and uses the host session limit
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_032, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want firstWant;
    firstWant.SetParam(AGENTID_KEY, std::string("agent0"));
    firstWant.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto firstConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(firstWant, firstConnection), ERR_OK);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);

    int32_t callerUid = IPCSkeleton::GetCallingUid();
    service->callerConnectionCounts_[callerUid] = AgentManagerService::MAX_CONNECTIONS_PER_CALLER;

    AAFwk::Want reuseWant;
    reuseWant.SetParam(AGENTID_KEY, std::string("agent1"));
    reuseWant.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto reuseConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(reuseWant, reuseConnection), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    EXPECT_EQ(service->agentOwners_.size(), 2);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_[callerUid], AgentManagerService::MAX_CONNECTIONS_PER_CALLER);
}

/**
 * @tc.name  : NotifyLowCodeAgentComplete_001
 * @tc.number: NotifyLowCodeAgentComplete_001
 * @tc.desc  : Test notifyLowCodeAgentComplete rejects non-system-app callers on the service side
 */
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_001, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    EXPECT_EQ(AgentManagerService::GetInstance()->NotifyLowCodeAgentComplete("agentA"), ERR_NOT_SYSTEM_APP);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_002
* @tc.number: NotifyLowCodeAgentComplete_002
* @tc.desc  : Test notifyLowCodeAgentComplete validates empty agentId
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_002, TestSize.Level1)
{
    EXPECT_EQ(AgentManagerService::GetInstance()->NotifyLowCodeAgentComplete(""),
        AAFwk::INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_003
* @tc.number: NotifyLowCodeAgentComplete_003
* @tc.desc  : Test notifyLowCodeAgentComplete keeps host alive until the last low-code agent completes
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("agentA"));
    wantA.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);
    auto hostConnectionA = MyFlag::lastConnectAbilityConnection;
    ASSERT_NE(hostConnectionA, nullptr);
    auto *rawHostConnectionA = static_cast<AgentHostConnection *>(hostConnectionA.GetRefPtr());

    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto hostKey = service->agentHostSessions_.begin()->key;
    hostKey.hostProxyRemote = connectionA->AsObject();
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);
    auto hostConnectionB = MyFlag::lastConnectAbilityConnection;
    ASSERT_NE(hostConnectionB, nullptr);
    auto *rawHostConnectionB = static_cast<AgentHostConnection *>(hostConnectionB.GetRefPtr());
    rawHostConnectionB->OnAbilityConnectDone(element, receiver->AsObject(), ERR_OK);

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_NE(MyFlag::lastDisconnectAbilityConnection, nullptr);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection->AsObject(), hostConnectionA->AsObject());
    EXPECT_EQ(connectionA->disconnectDoneCount, 0);
    rawHostConnectionA->OnAbilityDisconnectDone(element, ERR_OK);
    EXPECT_EQ(connectionA->disconnectDoneCount, 1);

    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->session->agents.count("agentB") > 0);
    EXPECT_EQ(service->agentHostSessions_.begin()->session->callerConnections.size(), 1);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
    EXPECT_EQ(connectionB->disconnectDoneCount, 0);

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentB"), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 2);
    ASSERT_NE(MyFlag::lastDisconnectAbilityConnection, nullptr);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection->AsObject(), hostConnectionB->AsObject());
    EXPECT_EQ(connectionB->disconnectDoneCount, 0);

    rawHostConnectionB->OnAbilityDisconnectDone(element, ERR_OK);
    EXPECT_EQ(connectionB->disconnectDoneCount, 1);
    EXPECT_TRUE(service->agentHostSessions_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());

    sptr<MockAbilityConnection> connectionA2 = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantA, connectionA2), ERR_OK);
    auto hostConnectionA2 = MyFlag::lastConnectAbilityConnection;
    ASSERT_NE(hostConnectionA2, nullptr);
    auto *rawHostConnectionA2 = static_cast<AgentHostConnection *>(hostConnectionA2.GetRefPtr());
    sptr<TestAgentReceiver> receiver2 = new TestAgentReceiver();
    rawHostConnectionA2->OnAbilityConnectDone(element, receiver2->AsObject(), ERR_OK);
    EXPECT_EQ(connectionA2->connectDoneCount, 1);
    EXPECT_EQ(receiver2->agentInvokedCount, 1);

    sptr<MockAbilityConnection> connectionB2 = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB2), ERR_OK);
    auto hostConnectionB2 = MyFlag::lastConnectAbilityConnection;
    ASSERT_NE(hostConnectionB2, nullptr);
    auto *rawHostConnectionB2 = static_cast<AgentHostConnection *>(hostConnectionB2.GetRefPtr());
    rawHostConnectionB2->OnAbilityConnectDone(element, receiver2->AsObject(), ERR_OK);
    EXPECT_EQ(connectionB2->connectDoneCount, 1);
    EXPECT_EQ(receiver2->agentInvokedCount, 2);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_004
* @tc.number: NotifyLowCodeAgentComplete_004
* @tc.desc  : Test notifyLowCodeAgentComplete releases the finished caller slot without disconnecting the shared host
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("agentA"));
    wantA.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);

    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    auto hostKey = service->agentHostSessions_.begin()->key;
    hostKey.hostProxyRemote = connectionA->AsObject();
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connectionA), ERR_INVALID_VALUE);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->session->agents.count("agentB") > 0);
    EXPECT_EQ(service->agentHostSessions_.begin()->session->callerConnections.size(), 1);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    EXPECT_EQ(service->trackedConnections_.begin()->second.callerRemote, connectionB->AsObject());
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
 * @tc.name  : DisconnectAgentExtensionAbility_010
 * @tc.number: DisconnectAgentExtensionAbility_010
 * @tc.desc  : Test explicit low-code disconnect clears bookkeeping and disconnects shared host once
 */
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_010, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";
    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("agentA"));
    wantA.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();

    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);
    auto hostKey = service->agentHostSessions_.begin()->key;
    hostKey.hostProxyRemote = connectionA->AsObject();
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);

    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connectionB), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_FALSE(service->agentHostSessions_.begin()->session->isDisconnecting);
    EXPECT_EQ(service->agentOwners_.size(), 1);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
    EXPECT_EQ(connectionA->disconnectDoneCount, 0);
    EXPECT_EQ(connectionB->disconnectDoneCount, 1);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->session->agents.count("agentA") > 0);
    EXPECT_FALSE(service->agentOwners_.empty());
    EXPECT_FALSE(service->callerConnectionCounts_.empty());
}

/**
 * @tc.name  : DisconnectAgentExtensionAbility_015
 * @tc.number: DisconnectAgentExtensionAbility_015
 * @tc.desc  : Test low-code proxy disconnect tears down the backing host after all owned agents are removed
 */
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_015, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";
    auto connectionA = sptr<MockAbilityConnection>::MakeSptr();
    auto connectionB = sptr<MockAbilityConnection>::MakeSptr();
    auto sharedHostProxy = sptr<MockAbilityConnection>::MakeSptr();

    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("agentA"));
    wantA.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, sharedHostProxy->AsObject());
    wantA.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);
    auto hostKey = service->agentHostSessions_.begin()->key;
    auto receiver = sptr<TestAgentReceiver>::MakeSptr();
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    service->HandleAgentHostConnectDone(hostKey, element, receiver->AsObject(), ERR_OK);

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("agentB"));
    wantB.SetParam(AGENTEXTENSIONHOSTPROXY_KEY, sharedHostProxy->AsObject());
    wantB.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    ASSERT_EQ(service->agentHostSessions_.begin()->session->agents.size(), 2);

    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connectionB), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_NE(MyFlag::lastDisconnectAbilityConnection, nullptr);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection->AsObject(),
        service->agentHostSessions_.begin()->session->hostConnection->AsObject());
    EXPECT_TRUE(service->agentHostSessions_.begin()->session->isDisconnecting);
    EXPECT_TRUE(service->agentHostSessions_.begin()->session->agents.empty());

    service->HandleAgentHostDisconnectDone(hostKey, element, ERR_OK);
    EXPECT_TRUE(service->agentHostSessions_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_003
* @tc.number: DisconnectAgentExtensionAbility_003
* @tc.desc  : Test DisconnectAgentExtensionAbility success case
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_003, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
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
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection),
        ERR_INVALID_VALUE);
    MyFlag::retDisconnectAbility = ERR_OK;
}

/**
* @tc.name  : ConnectAgentExtensionAbility_011
* @tc.number: ConnectAgentExtensionAbility_011
* @tc.desc  : Test ConnectAgentExtensionAbility enforces max connections per caller on the service side
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_011, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");

    std::vector<sptr<MockAbilityConnection>> connections;
    for (size_t i = 0; i < AgentManagerService::MAX_CONNECTIONS_PER_CALLER; i++) {
        std::string abilityName = "TestAbility" + std::to_string(i);
        MyFlag::agentCardAbilityName = abilityName;
        want.SetElementName("test.bundle", abilityName);
        auto connection = sptr<MockAbilityConnection>::MakeSptr();
        connections.emplace_back(connection);
        EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    }

    MyFlag::agentCardAbilityName = "OverflowAbility";
    want.SetElementName("test.bundle", "OverflowAbility");
    auto overflowConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, overflowConnection),
        ERR_MAX_AGENT_CONNECTIONS_REACHED);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_012
* @tc.number: ConnectAgentExtensionAbility_012
* @tc.desc  : Test ConnectAgentExtensionAbility rollback on connect failure callback
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_012, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    ASSERT_NE(MyFlag::lastConnectAbilityConnection, nullptr);
    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.begin()->second, 1);

    AppExecFwk::ElementName element;
    MyFlag::lastConnectAbilityConnection->OnAbilityConnectDone(element, nullptr, ERR_INVALID_VALUE);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_005
* @tc.number: DisconnectAgentExtensionAbility_005
* @tc.desc  : Test DisconnectAgentExtensionAbility uses service wrapper and releases count before callback
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_005, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    ASSERT_NE(MyFlag::lastConnectAbilityConnection, nullptr);
    EXPECT_NE(MyFlag::lastConnectAbilityConnection->AsObject(), connection->AsObject());

    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection), ERR_OK);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection, MyFlag::lastConnectAbilityConnection);
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);

    AppExecFwk::ElementName element;
    MyFlag::lastConnectAbilityConnection->OnAbilityDisconnectDone(element, ERR_OK);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_007
* @tc.number: DisconnectAgentExtensionAbility_007
* @tc.desc  : Test DisconnectAgentExtensionAbility restores count when disconnect request fails immediately
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_007, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection), ERR_INVALID_VALUE);
    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.begin()->second, 1);
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);
    EXPECT_FALSE(AgentManagerService::GetInstance()->trackedConnections_.begin()->second.isDisconnecting);
    MyFlag::retDisconnectAbility = ERR_OK;
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_008
* @tc.number: DisconnectAgentExtensionAbility_008
* @tc.desc  : Test DisconnectAgentExtensionAbility frees quota for immediate reconnect
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_008, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");

    std::vector<sptr<MockAbilityConnection>> connections;
    for (size_t i = 0; i < AgentManagerService::MAX_CONNECTIONS_PER_CALLER; i++) {
        auto connection = sptr<MockAbilityConnection>::MakeSptr();
        connections.emplace_back(connection);
        EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    }

    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connections[0]), ERR_OK);
    auto newConnection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, newConnection), ERR_OK);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_009
* @tc.number: DisconnectAgentExtensionAbility_009
* @tc.desc  : Test DisconnectAgentExtensionAbility is idempotent while the connection is disconnecting
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_009, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection), ERR_OK);
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.begin()->second.isDisconnecting);
}

/**
* @tc.name  : ConnectAgentExtensionAbility_013
* @tc.number: ConnectAgentExtensionAbility_013
* @tc.desc  : Test ConnectAgentExtensionAbility rejects duplicate tracked connection registration
*/
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_013, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectAgentExtensionAbility(want, connection), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    ASSERT_EQ(AgentManagerService::GetInstance()->standardSessions_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->standardSessions_.front()->pendingCallbacks.size(), 1);
}

/**
* @tc.name  : DisconnectAgentExtensionAbility_006
* @tc.number: DisconnectAgentExtensionAbility_006
* @tc.desc  : Test DisconnectAgentExtensionAbility rejects untracked connection
*/
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_006, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectAgentExtensionAbility(connection), ERR_INVALID_VALUE);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_001
* @tc.number: ConnectServiceExtensionAbility_001
* @tc.desc  : Test ConnectServiceExtensionAbility rejects null caller token
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_001, TestSize.Level1)
{
    AAFwk::Want want;
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(nullptr, want, connection),
        ERR_INVALID_VALUE);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_002
* @tc.number: ConnectServiceExtensionAbility_002
* @tc.desc  : Test ConnectServiceExtensionAbility connects through AMS with explicit caller token and SERVICE type
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_002, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    EXPECT_EQ(MyFlag::lastConnectAbilityCallerToken, callerToken);
    EXPECT_EQ(MyFlag::lastConnectAbilityExtensionType, AppExecFwk::ExtensionAbilityType::SERVICE);
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);
    EXPECT_FALSE(AgentManagerService::GetInstance()->trackedConnections_.begin()->second.countTowardsCallerLimit);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_003
* @tc.number: ConnectServiceExtensionAbility_003
* @tc.desc  : Test ConnectServiceExtensionAbility ignores CONNECT_AGENT permission state
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_003, TestSize.Level1)
{
    MyFlag::retVerifyConnectAgentPermission = false;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_004
* @tc.number: ConnectServiceExtensionAbility_004
* @tc.desc  : Test ConnectServiceExtensionAbility rejects non-system-app callers
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_004, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_NOT_SYSTEM_APP);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_005
* @tc.number: ConnectServiceExtensionAbility_005
* @tc.desc  : Test ConnectServiceExtensionAbility rejects null connection
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_005, TestSize.Level1)
{
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, nullptr),
        ERR_INVALID_VALUE);
}

/**
* @tc.name  : ConnectServiceExtensionAbility_006
* @tc.number: ConnectServiceExtensionAbility_006
* @tc.desc  : Test ConnectServiceExtensionAbility rejects unresolved service target
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_006, TestSize.Level1)
{
    MyFlag::retQueryExtensionAbilityInfos = false;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        RESOLVE_ABILITY_ERR);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
}

/**
* @tc.name  : ConnectServiceExtensionAbility_007
* @tc.number: ConnectServiceExtensionAbility_007
* @tc.desc  : Test ConnectServiceExtensionAbility rejects non-service extension targets
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_007, TestSize.Level1)
{
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::AGENT;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_WRONG_INTERFACE_CALL);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 0);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
}

/**
* @tc.name  : ConnectServiceExtensionAbility_008
* @tc.number: ConnectServiceExtensionAbility_008
* @tc.desc  : Test ConnectServiceExtensionAbility rolls back tracked state on AMS failure
*/
HWTEST_F(AgentManagerServiceTest, ConnectServiceExtensionAbility_008, TestSize.Level1)
{
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::retConnectAbilityWithExtensionType = ERR_INVALID_VALUE;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_INVALID_VALUE);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_001
* @tc.number: DisconnectServiceExtensionAbility_001
* @tc.desc  : Test DisconnectServiceExtensionAbility disconnects without caller quota bookkeeping
*/
HWTEST_F(AgentManagerServiceTest, DisconnectServiceExtensionAbility_001, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_OK);
    ASSERT_NE(MyFlag::lastConnectAbilityConnection, nullptr);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, connection), ERR_OK);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection, MyFlag::lastConnectAbilityConnection);
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);

    AppExecFwk::ElementName element;
    MyFlag::lastConnectAbilityConnection->OnAbilityDisconnectDone(element, ERR_OK);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_002
* @tc.number: DisconnectServiceExtensionAbility_002
* @tc.desc  : Test DisconnectServiceExtensionAbility ignores CONNECT_AGENT permission state
*/
HWTEST_F(AgentManagerServiceTest, DisconnectServiceExtensionAbility_002, TestSize.Level1)
{
    MyFlag::retVerifyConnectAgentPermission = false;
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, connection), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_003
* @tc.number: DisconnectServiceExtensionAbility_003
* @tc.desc  : Test DisconnectServiceExtensionAbility rejects non-system-app callers
*/
HWTEST_F(AgentManagerServiceTest, DisconnectServiceExtensionAbility_003, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, connection),
        ERR_NOT_SYSTEM_APP);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_004
* @tc.number: DisconnectServiceExtensionAbility_004
* @tc.desc  : Test DisconnectServiceExtensionAbility rejects null connection
*/
HWTEST_F(AgentManagerServiceTest, DisconnectServiceExtensionAbility_004, TestSize.Level1)
{
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));

    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, nullptr),
        INVALID_PARAMETERS_ERR);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_005
* @tc.number: DisconnectServiceExtensionAbility_005
* @tc.desc  : Test DisconnectServiceExtensionAbility rejects untracked connection
*/
HWTEST_F(AgentManagerServiceTest, DisconnectServiceExtensionAbility_005, TestSize.Level1)
{
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, connection),
        ERR_INVALID_VALUE);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_006
* @tc.number: DisconnectServiceExtensionAbility_006
* @tc.desc  : Test DisconnectServiceExtensionAbility is idempotent while disconnecting
*/
HWTEST_F(AgentManagerServiceTest, DisconnectServiceExtensionAbility_006, TestSize.Level1)
{
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_OK;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, connection), ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, connection), ERR_OK);
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.begin()->second.isDisconnecting);
}

/**
* @tc.name  : DisconnectServiceExtensionAbility_007
* @tc.number: DisconnectServiceExtensionAbility_007
* @tc.desc  : Test DisconnectServiceExtensionAbility rolls back disconnecting state on AMS failure
*/
HWTEST_F(AgentManagerServiceTest, DisconnectServiceExtensionAbility_007, TestSize.Level1)
{
    MyFlag::retQueryExtensionAbilityInfos = true;
    MyFlag::extensionAbilityType = AppExecFwk::ExtensionAbilityType::SERVICE;
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    AAFwk::Want want;
    want.SetBundle("test.bundle");
    auto callerToken = sptr<IRemoteObject>(new IPCObjectStub(u"caller.token"));
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(AgentManagerService::GetInstance()->ConnectServiceExtensionAbility(callerToken, want, connection),
        ERR_OK);
    EXPECT_EQ(AgentManagerService::GetInstance()->DisconnectServiceExtensionAbility(callerToken, connection),
        ERR_INVALID_VALUE);
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);
    auto trackedIter = AgentManagerService::GetInstance()->trackedConnections_.find(connection->AsObject());
    ASSERT_NE(trackedIter, AgentManagerService::GetInstance()->trackedConnections_.end());
    EXPECT_FALSE(trackedIter->second.isDisconnecting);
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : ValidateConnectAgentRequest_001
* @tc.number: ValidateConnectAgentRequest_001
* @tc.desc  : Test ValidateConnectAgentRequest rejects non-system-app callers
*/
HWTEST_F(AgentManagerServiceTest, ValidateConnectAgentRequest_001, TestSize.Level1)
{
    MyFlag::retJudgeCallerIsAllowedToUseSystemAPI = false;
    int32_t callerUid = -1;
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->ValidateConnectAgentRequest(connection, callerUid),
        ERR_NOT_SYSTEM_APP);
}

/**
* @tc.name  : ValidateConnectAgentRequest_002
* @tc.number: ValidateConnectAgentRequest_002
* @tc.desc  : Test ValidateConnectAgentRequest rejects callers without connect permission
*/
HWTEST_F(AgentManagerServiceTest, ValidateConnectAgentRequest_002, TestSize.Level1)
{
    MyFlag::retVerifyConnectAgentPermission = false;
    int32_t callerUid = -1;
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->ValidateConnectAgentRequest(connection, callerUid),
        ERR_PERMISSION_DENIED);
}

/**
* @tc.name  : ValidateConnectAgentRequest_003
* @tc.number: ValidateConnectAgentRequest_003
* @tc.desc  : Test ValidateConnectAgentRequest rejects null connections
*/
HWTEST_F(AgentManagerServiceTest, ValidateConnectAgentRequest_003, TestSize.Level1)
{
    int32_t callerUid = -1;
    sptr<AAFwk::IAbilityConnection> connection = nullptr;
    EXPECT_EQ(AgentManagerService::GetInstance()->ValidateConnectAgentRequest(connection, callerUid),
        ERR_INVALID_VALUE);
}

/**
* @tc.name  : ValidateConnectAgentRequest_004
* @tc.number: ValidateConnectAgentRequest_004
* @tc.desc  : Test ValidateConnectAgentRequest leaves quota checks to the classified connect path
*/
HWTEST_F(AgentManagerServiceTest, ValidateConnectAgentRequest_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    service->callerConnectionCounts_[callerUid] = AgentManagerService::MAX_CONNECTIONS_PER_CALLER;
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    int32_t outCallerUid = -1;

    EXPECT_EQ(service->ValidateConnectAgentRequest(connection, outCallerUid), ERR_OK);
    EXPECT_EQ(outCallerUid, callerUid);
}

/**
* @tc.name  : ValidateConnectAgentRequest_005
* @tc.number: ValidateConnectAgentRequest_005
* @tc.desc  : Test ValidateConnectAgentRequest rejects background callers
*/
HWTEST_F(AgentManagerServiceTest, ValidateConnectAgentRequest_005, TestSize.Level1)
{
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_BACKGROUND;
    int32_t callerUid = -1;
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    EXPECT_EQ(AgentManagerService::GetInstance()->ValidateConnectAgentRequest(connection, callerUid),
        AAFwk::NOT_TOP_ABILITY);
}

/**
* @tc.name  : ResolveConnectAgentTarget_001
* @tc.number: ResolveConnectAgentTarget_001
* @tc.desc  : Test ResolveConnectAgentTarget fills agent target metadata on success
*/
HWTEST_F(AgentManagerServiceTest, ResolveConnectAgentTarget_001, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::LOW_CODE);
    MyFlag::agentCardBundleName = "lowcode.bundle";
    MyFlag::agentCardAbilityName = "LowCodeExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    AgentResolvedTarget target;

    EXPECT_EQ(AgentManagerService::GetInstance()->ResolveConnectAgentTarget(want, target), ERR_OK);
    EXPECT_EQ(target.agentId, "testAgent");
    EXPECT_EQ(target.card.type, AgentCardType::LOW_CODE);
    EXPECT_EQ(target.connectWant.GetElement().GetBundleName(), "lowcode.bundle");
    EXPECT_EQ(target.connectWant.GetElement().GetAbilityName(), "LowCodeExtAbility");
    EXPECT_EQ(target.callingUid, IPCSkeleton::GetCallingUid());
    EXPECT_EQ(target.targetUid, MyFlag::extensionAbilityUid);
}

/**
* @tc.name  : ResolveConnectAgentTarget_002
* @tc.number: ResolveConnectAgentTarget_002
* @tc.desc  : Test ResolveConnectAgentTarget rejects unknown AgentCards
*/
HWTEST_F(AgentManagerServiceTest, ResolveConnectAgentTarget_002, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_NAME_NOT_FOUND;
    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("missingAgent"));
    want.SetBundle("test.bundle");
    AgentResolvedTarget target;

    EXPECT_EQ(AgentManagerService::GetInstance()->ResolveConnectAgentTarget(want, target),
        AAFwk::ERR_INVALID_AGENT_CARD_ID);
}

/**
* @tc.name  : ResolveConnectAgentTarget_003
* @tc.number: ResolveConnectAgentTarget_003
* @tc.desc  : Test ResolveConnectAgentTarget normalizes module with the device-free SetElementName overload
*/
HWTEST_F(AgentManagerServiceTest, ResolveConnectAgentTarget_003, TestSize.Level1)
{
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::agentCardBundleName = "agent.bundle";
    MyFlag::agentCardAbilityName = "AgentExtAbility";
    MyFlag::agentCardModuleName = "entry";

    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetElementName("remoteDevice", "agent.bundle", "AgentExtAbility");
    AgentResolvedTarget target;

    EXPECT_EQ(AgentManagerService::GetInstance()->ResolveConnectAgentTarget(want, target), ERR_OK);
    EXPECT_EQ(target.connectWant.GetElement().GetDeviceID(), "remoteDevice");
    EXPECT_EQ(target.connectWant.GetElement().GetBundleName(), "agent.bundle");
    EXPECT_EQ(target.connectWant.GetElement().GetAbilityName(), "AgentExtAbility");
    EXPECT_EQ(target.connectWant.GetElement().GetModuleName(), "entry");
}

/**
* @tc.name  : FindTrackedConnectionLocked_001
* @tc.number: FindTrackedConnectionLocked_001
* @tc.desc  : Test FindTrackedConnectionLocked falls back to the single standard connection owned by the caller
*/
HWTEST_F(AgentManagerServiceTest, FindTrackedConnectionLocked_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto trackedConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto probeConnection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = trackedConnection->AsObject();
    service->trackedConnections_[trackedConnection->AsObject()] = record;

    auto iter = service->FindTrackedConnectionLocked(probeConnection, IPCSkeleton::GetCallingUid());
    ASSERT_NE(iter, service->trackedConnections_.end());
    EXPECT_EQ(iter->first, trackedConnection->AsObject());
}

/**
* @tc.name  : FindTrackedConnectionLocked_002
* @tc.number: FindTrackedConnectionLocked_002
* @tc.desc  : Test FindTrackedConnectionLocked refuses ambiguous callerUid fallback
*/
HWTEST_F(AgentManagerServiceTest, FindTrackedConnectionLocked_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto trackedConnectionA = sptr<MockAbilityConnection>::MakeSptr();
    auto trackedConnectionB = sptr<MockAbilityConnection>::MakeSptr();
    auto probeConnection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord recordA;
    recordA.callerUid = IPCSkeleton::GetCallingUid();
    recordA.callerRemote = trackedConnectionA->AsObject();
    TrackedConnectionRecord recordB = recordA;
    recordB.callerRemote = trackedConnectionB->AsObject();
    service->trackedConnections_[trackedConnectionA->AsObject()] = recordA;
    service->trackedConnections_[trackedConnectionB->AsObject()] = recordB;

    auto iter = service->FindTrackedConnectionLocked(probeConnection, IPCSkeleton::GetCallingUid());
    EXPECT_EQ(iter, service->trackedConnections_.end());
}

/**
* @tc.name  : FindTrackedConnectionLocked_003
* @tc.number: FindTrackedConnectionLocked_003
* @tc.desc  : Test FindTrackedConnectionLocked does not use callerUid fallback for low-code connections
*/
HWTEST_F(AgentManagerServiceTest, FindTrackedConnectionLocked_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto trackedConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto probeConnection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = trackedConnection->AsObject();
    record.isLowCode = true;
    service->trackedConnections_[trackedConnection->AsObject()] = record;

    auto iter = service->FindTrackedConnectionLocked(probeConnection, IPCSkeleton::GetCallingUid());
    EXPECT_EQ(iter, service->trackedConnections_.end());
}

/**
* @tc.name  : TryRegisterConnectionLocked_001
* @tc.number: TryRegisterConnectionLocked_001
* @tc.desc  : Test TryRegisterConnectionLocked rejects duplicate caller remotes
*/
HWTEST_F(AgentManagerServiceTest, TryRegisterConnectionLocked_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = connection->AsObject();
    service->trackedConnections_[connection->AsObject()] = record;

    EXPECT_EQ(service->TryRegisterConnectionLocked(connection, IPCSkeleton::GetCallingUid()), ERR_INVALID_VALUE);
}

/**
* @tc.name  : RegisterTrackedConnectionAndGetServiceConnection_001
* @tc.number: RegisterTrackedConnectionAndGetServiceConnection_001
* @tc.desc  : Test RegisterTrackedConnectionAndGetServiceConnection installs tracked wrapper state
*/
HWTEST_F(AgentManagerServiceTest, RegisterTrackedConnectionAndGetServiceConnection_001, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;

    EXPECT_EQ(AgentManagerService::GetInstance()->RegisterTrackedConnectionAndGetServiceConnection(
        connection, IPCSkeleton::GetCallingUid(), true, serviceConnection), ERR_OK);
    ASSERT_NE(serviceConnection, nullptr);
    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);
    auto trackedIter = AgentManagerService::GetInstance()->trackedConnections_.find(connection->AsObject());
    ASSERT_NE(trackedIter, AgentManagerService::GetInstance()->trackedConnections_.end());
    EXPECT_EQ(trackedIter->second.serviceConnection->AsObject(), serviceConnection->AsObject());
}

/**
* @tc.name  : RegisterTrackedConnectionAndGetServiceConnection_002
* @tc.number: RegisterTrackedConnectionAndGetServiceConnection_002
* @tc.desc  : Test RegisterTrackedConnectionAndGetServiceConnection skips caller quota bookkeeping when requested
*/
HWTEST_F(AgentManagerServiceTest, RegisterTrackedConnectionAndGetServiceConnection_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;
    int32_t callerUid = IPCSkeleton::GetCallingUid();

    EXPECT_EQ(service->RegisterTrackedConnectionAndGetServiceConnection(connection, callerUid, false,
        serviceConnection), ERR_OK);
    ASSERT_NE(serviceConnection, nullptr);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    auto trackedIter = service->trackedConnections_.find(connection->AsObject());
    ASSERT_NE(trackedIter, service->trackedConnections_.end());
    EXPECT_EQ(trackedIter->second.callerUid, callerUid);
    EXPECT_EQ(trackedIter->second.serviceConnection->AsObject(), serviceConnection->AsObject());
    EXPECT_FALSE(trackedIter->second.countTowardsCallerLimit);
    EXPECT_NE(trackedIter->second.deathRecipient, nullptr);
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : RegisterTrackedConnectionAndGetServiceConnection_003
* @tc.number: RegisterTrackedConnectionAndGetServiceConnection_003
* @tc.desc  : Test RegisterTrackedConnectionAndGetServiceConnection propagates duplicate registration failure
*/
HWTEST_F(AgentManagerServiceTest, RegisterTrackedConnectionAndGetServiceConnection_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = IPCSkeleton::GetCallingUid();
    record.callerRemote = connection->AsObject();
    service->trackedConnections_[connection->AsObject()] = record;
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;

    EXPECT_EQ(service->RegisterTrackedConnectionAndGetServiceConnection(
        connection, IPCSkeleton::GetCallingUid(), true, serviceConnection), ERR_INVALID_VALUE);
    EXPECT_EQ(serviceConnection, nullptr);
    EXPECT_EQ(service->trackedConnections_.size(), 1);
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : RegisterTrackedConnectionAndGetServiceConnection_004
* @tc.number: RegisterTrackedConnectionAndGetServiceConnection_004
* @tc.desc  : Test RegisterTrackedConnectionAndGetServiceConnection rejects callers that already hit the quota
*/
HWTEST_F(AgentManagerServiceTest, RegisterTrackedConnectionAndGetServiceConnection_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    service->callerConnectionCounts_[callerUid] = AgentManagerService::MAX_CONNECTIONS_PER_CALLER;
    sptr<AAFwk::IAbilityConnection> serviceConnection = nullptr;

    EXPECT_EQ(service->RegisterTrackedConnectionAndGetServiceConnection(
        connection, callerUid, true, serviceConnection),
        AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED);
    EXPECT_EQ(serviceConnection, nullptr);
    EXPECT_TRUE(service->trackedConnections_.empty());
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_[callerUid], AgentManagerService::MAX_CONNECTIONS_PER_CALLER);
}

/**
* @tc.name  : RegisterTrackedConnectionAndGetServiceConnection_005
* @tc.number: RegisterTrackedConnectionAndGetServiceConnection_005
* @tc.desc  : Test caller quota is counted by uid and exact instanceKey
*/
HWTEST_F(AgentManagerServiceTest, RegisterTrackedConnectionAndGetServiceConnection_005, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto connectionA = sptr<MockAbilityConnection>::MakeSptr();
    auto connectionB = sptr<MockAbilityConnection>::MakeSptr();
    sptr<AAFwk::IAbilityConnection> serviceConnectionA = nullptr;
    sptr<AAFwk::IAbilityConnection> serviceConnectionB = nullptr;
    int32_t callerUid = IPCSkeleton::GetCallingUid();
    AgentCallerIdentity cloneA { callerUid, "cloneA" };
    AgentCallerIdentity cloneB { callerUid, "cloneB" };

    service->callerConnectionCounts_[cloneA] = AgentManagerService::MAX_CONNECTIONS_PER_CALLER;

    EXPECT_EQ(service->RegisterTrackedConnectionAndGetServiceConnection(connectionA, cloneA, true,
        serviceConnectionA), AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED);
    EXPECT_EQ(service->RegisterTrackedConnectionAndGetServiceConnection(connectionB, cloneB, true,
        serviceConnectionB), ERR_OK);

    EXPECT_EQ(serviceConnectionA, nullptr);
    ASSERT_NE(serviceConnectionB, nullptr);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 2);
    EXPECT_EQ(service->callerConnectionCounts_[cloneA], AgentManagerService::MAX_CONNECTIONS_PER_CALLER);
    EXPECT_EQ(service->callerConnectionCounts_[cloneB], 1);
}

/**
* @tc.name  : TryRegisterConnectionLocked_002
* @tc.number: TryRegisterConnectionLocked_002
* @tc.desc  : Test TryRegisterConnectionLocked stores explicit low-code tracking state without quota bookkeeping
*/
HWTEST_F(AgentManagerServiceTest, TryRegisterConnectionLocked_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto serviceConnection = sptr<MockAbilityConnection>::MakeSptr();
    AgentHostKey hostKey;
    hostKey.userId = 100;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeAbility";
    int32_t callerUid = IPCSkeleton::GetCallingUid();

    EXPECT_EQ(service->TryRegisterConnectionLocked(connection, callerUid, serviceConnection, &hostKey, false), ERR_OK);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    auto trackedIter = service->trackedConnections_.find(connection->AsObject());
    ASSERT_NE(trackedIter, service->trackedConnections_.end());
    EXPECT_EQ(trackedIter->second.serviceConnection->AsObject(), serviceConnection->AsObject());
    EXPECT_TRUE(trackedIter->second.isLowCode);
    EXPECT_EQ(trackedIter->second.hostKey.userId, hostKey.userId);
    EXPECT_EQ(trackedIter->second.hostKey.bundleName, hostKey.bundleName);
    EXPECT_EQ(trackedIter->second.hostKey.moduleName, hostKey.moduleName);
    EXPECT_EQ(trackedIter->second.hostKey.abilityName, hostKey.abilityName);
    EXPECT_FALSE(trackedIter->second.countTowardsCallerLimit);
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : PrepareLowCodeConnectPlan_001
* @tc.number: PrepareLowCodeConnectPlan_001
* @tc.desc  : Test PrepareLowCodeConnectPlan creates a new agent host session and reserves one caller slot
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeConnectPlan_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectPlan plan;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    EXPECT_EQ(service->PrepareLowCodeConnectPlan(hostKey, callingUid, "agentA", connection, callingUid, plan),
        ERR_OK);
    EXPECT_TRUE(plan.needRealConnect);
    EXPECT_TRUE(plan.registeredTrackedConnection);
    ASSERT_NE(plan.hostConnection, nullptr);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    ASSERT_EQ(service->agentOwners_.size(), 1);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_005
* @tc.number: NotifyLowCodeAgentComplete_005
* @tc.desc  : Test notifyLowCodeAgentComplete rejects unknown low-code agents
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_005, TestSize.Level1)
{
    EXPECT_EQ(AgentManagerService::GetInstance()->NotifyLowCodeAgentComplete("missingAgent"),
        AAFwk::ERR_INVALID_AGENT_CARD_ID);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_006
* @tc.number: NotifyLowCodeAgentComplete_006
* @tc.desc  : Test notifyLowCodeAgentComplete removes stale null sessions from the owner map
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_006, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    service->SetAgentOwnerLocked(AgentOwnerKey { callingUid, "agentA" }, nullptr);

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), AAFwk::CONNECTION_NOT_EXIST);
    EXPECT_TRUE(service->agentOwners_.empty());
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_007
* @tc.number: NotifyLowCodeAgentComplete_007
* @tc.desc  : Test notifyLowCodeAgentComplete handles missing session agent bookkeeping
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_007, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    service->SetAgentOwnerLocked(AgentOwnerKey { callingUid, "agentA" }, session);
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), AAFwk::CONNECTION_NOT_EXIST);
    EXPECT_TRUE(service->agentOwners_.empty());
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_008
* @tc.number: NotifyLowCodeAgentComplete_008
* @tc.desc  : Test notifyLowCodeAgentComplete keeps the caller connection when the same caller still owns agents
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_008, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto remote = connection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[remote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { remote, false };
    session->agents["agentB"] = LowCodeAgentRecord { remote, false };
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });
    service->SetAgentOwnerLocked(AgentOwnerKey { callingUid, "agentA" }, session);
    service->SetAgentOwnerLocked(AgentOwnerKey { callingUid, "agentB" }, session);
    TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = remote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    service->trackedConnections_[remote] = record;
    service->callerConnectionCounts_[callingUid] = 1;

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_EQ(service->agentHostSessions_.begin()->session->callerConnections.size(), 1);
    EXPECT_EQ(service->agentHostSessions_.begin()->session->agents.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->session->agents.count("agentB") > 0);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
* @tc.name  : NotifyLowCodeAgentComplete_009
* @tc.number: NotifyLowCodeAgentComplete_009
* @tc.desc  : Test notifyLowCodeAgentComplete restores session state when shared-host disconnect fails
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeAgentComplete_009, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto remote = connection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[remote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { remote, false };
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });
    service->SetAgentOwnerLocked(AgentOwnerKey { callingUid, "agentA" }, session);
    TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = remote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    service->trackedConnections_[remote] = record;
    service->callerConnectionCounts_[callingUid] = 1;

    EXPECT_EQ(service->NotifyLowCodeAgentComplete("agentA"), ERR_INVALID_VALUE);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_FALSE(service->agentHostSessions_.begin()->session->isDisconnecting);
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
    MyFlag::retDisconnectAbility = ERR_OK;
}

/**
* @tc.name  : PrepareLowCodeConnectPlan_002
* @tc.number: PrepareLowCodeConnectPlan_002
* @tc.desc  : Test PrepareLowCodeConnectPlan reuses connected host sessions and enables immediate callback delivery
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeConnectPlan_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectPlan plan;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->AddHostConnection(connection->AsObject(), session->hostConnection);
    session->isConnected = true;
    session->resultCode = ERR_OK;
    session->element = AppExecFwk::ElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    session->CacheConnectDone(session->element, receiver->AsObject(), ERR_OK);
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });

    EXPECT_EQ(service->PrepareLowCodeConnectPlan(hostKey, callingUid, "agentA", connection, callingUid, plan),
        ERR_OK);
    EXPECT_FALSE(plan.needRealConnect);
    EXPECT_TRUE(plan.notifyExistingConnection);
    EXPECT_EQ(plan.cachedRemoteObject, receiver->AsObject());
    ASSERT_EQ(service->agentOwners_.size(), 1);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
}

/**
* @tc.name  : PrepareLowCodeConnectPlan_004
* @tc.number: PrepareLowCodeConnectPlan_004
* @tc.desc  : Test connected low-code host creates a real connect for a new host proxy owner
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeConnectPlan_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectPlan plan;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto existingConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto newConnection = sptr<MockAbilityConnection>::MakeSptr();
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->AddHostConnection(existingConnection->AsObject(), session->hostConnection);
    session->CacheConnectDone(
        AppExecFwk::ElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry"), receiver->AsObject(), ERR_OK);
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });

    EXPECT_EQ(service->PrepareLowCodeConnectPlan(hostKey, callingUid, "agentA", newConnection, callingUid, plan),
        ERR_OK);
    EXPECT_TRUE(plan.needRealConnect);
    EXPECT_FALSE(plan.notifyExistingConnection);
    ASSERT_NE(plan.hostConnection, nullptr);
    EXPECT_NE(plan.hostConnection, session->hostConnection);
    ASSERT_EQ(service->agentOwners_.size(), 1);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
}

/**
* @tc.name  : PrepareLowCodeConnectPlan_003
* @tc.number: PrepareLowCodeConnectPlan_003
* @tc.desc  : Test PrepareLowCodeConnectPlan rejects reconnect while the shared host session is disconnecting
*/
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeConnectPlan_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectPlan plan;
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->isDisconnecting = true;
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });

    EXPECT_EQ(service->PrepareLowCodeConnectPlan(hostKey, callingUid, "agentA", connection, callingUid, plan),
        ERR_INVALID_VALUE);
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
}

/**
* @tc.name  : NotifyExistingLowCodeConnection_001
* @tc.number: NotifyExistingLowCodeConnection_001
* @tc.desc  : Test NotifyExistingLowCodeConnection notifies the agent receiver and callback immediately
*/
HWTEST_F(AgentManagerServiceTest, NotifyExistingLowCodeConnection_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectPlan plan;
    plan.cachedElement = AppExecFwk::ElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    sptr<TestAgentReceiver> receiver = new TestAgentReceiver();
    plan.cachedRemoteObject = receiver->AsObject();
    plan.cachedResultCode = ERR_OK;
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    service->NotifyExistingLowCodeConnection(plan, "agentA", connection);
    EXPECT_EQ(receiver->agentInvokedCount, 1);
    ASSERT_EQ(receiver->invokedAgentIds.size(), 1);
    EXPECT_EQ(receiver->invokedAgentIds[0], "agentA");
    EXPECT_EQ(connection->connectDoneCount, 1);
    EXPECT_EQ(connection->lastConnectResultCode, ERR_OK);
}

/**
* @tc.name  : NotifyExistingLowCodeConnection_002
* @tc.number: NotifyExistingLowCodeConnection_002
* @tc.desc  : Test NotifyExistingLowCodeConnection still notifies the callback when no agent receiver is exposed
*/
HWTEST_F(AgentManagerServiceTest, NotifyExistingLowCodeConnection_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectPlan plan;
    plan.cachedElement = AppExecFwk::ElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    plan.cachedRemoteObject = sptr<MockAbilityConnection>::MakeSptr()->AsObject();
    plan.cachedResultCode = ERR_OK;
    auto connection = sptr<MockAbilityConnection>::MakeSptr();

    service->NotifyExistingLowCodeConnection(plan, "agentA", connection);

    EXPECT_EQ(connection->connectDoneCount, 1);
    EXPECT_EQ(connection->lastConnectResultCode, ERR_OK);
}

/**
* @tc.name  : CleanupLowCodeConnectPlan_001
* @tc.number: CleanupLowCodeConnectPlan_001
* @tc.desc  : Test CleanupLowCodeConnectPlan removes agent-host bookkeeping and releases the reserved caller slot
*/
HWTEST_F(AgentManagerServiceTest, CleanupLowCodeConnectPlan_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = connection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[callerRemote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, true };
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });
    service->SetAgentOwnerLocked(AgentOwnerKey { callingUid, "agentA" }, session);
    TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = callerRemote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    service->trackedConnections_[callerRemote] = record;
    service->callerConnectionCounts_[callingUid] = 1;

    AgentConnectPlan plan;
    plan.hostKey = hostKey;
    plan.hostUid = callingUid;
    plan.callerRemote = callerRemote;
    plan.registeredTrackedConnection = true;
    service->CleanupLowCodeConnectPlan(plan, "agentA");

    EXPECT_TRUE(service->agentHostSessions_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
}

/**
* @tc.name  : CleanupLowCodeConnectPlan_002
* @tc.number: CleanupLowCodeConnectPlan_002
* @tc.desc  : Test CleanupLowCodeConnectPlan preserves shared host state when other callers and agents remain
*/
HWTEST_F(AgentManagerServiceTest, CleanupLowCodeConnectPlan_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto connectionA = sptr<MockAbilityConnection>::MakeSptr();
    auto connectionB = sptr<MockAbilityConnection>::MakeSptr();
    auto remoteA = connectionA->AsObject();
    auto remoteB = connectionB->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[remoteA] = connectionA;
    session->callerConnections[remoteB] = connectionB;
    session->agents["agentA"] = LowCodeAgentRecord { remoteA, true };
    session->agents["agentB"] = LowCodeAgentRecord { remoteB, true };
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });
    service->SetAgentOwnerLocked(AgentOwnerKey { callingUid, "agentA" }, session);
    service->SetAgentOwnerLocked(AgentOwnerKey { callingUid, "agentB" }, session);
    TrackedConnectionRecord recordA;
    recordA.callerUid = callingUid;
    recordA.callerRemote = remoteA;
    recordA.serviceConnection = session->hostConnection;
    recordA.isLowCode = true;
    recordA.hostKey = hostKey;
    TrackedConnectionRecord recordB = recordA;
    recordB.callerRemote = remoteB;
    service->trackedConnections_[remoteA] = recordA;
    service->trackedConnections_[remoteB] = recordB;
    service->callerConnectionCounts_[callingUid] = 2;

    AgentConnectPlan plan;
    plan.hostKey = hostKey;
    plan.hostUid = callingUid;
    plan.callerRemote = remoteA;
    plan.registeredTrackedConnection = true;
    service->CleanupLowCodeConnectPlan(plan, "agentA");

    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_EQ(service->agentHostSessions_.begin()->session->agents.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->session->agents.count("agentB") > 0);
    ASSERT_EQ(service->agentOwners_.size(), 1);
    EXPECT_TRUE(service->FindAgentOwnerLocked(
        AgentOwnerKey { callingUid, "agentB" }) != service->agentOwners_.end());
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    EXPECT_TRUE(service->trackedConnections_.count(remoteB) > 0);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
* @tc.name  : CompleteAgentHostConnect_001
* @tc.number: CompleteAgentHostConnect_001
 * @tc.desc  : Test CompleteAgentHostConnect cleans agent-host state when AMS connect fails immediately
*/
HWTEST_F(AgentManagerServiceTest, CompleteAgentHostConnect_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    MyFlag::retConnectAbilityWithExtensionType = ERR_INVALID_VALUE;

    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = connection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[callerRemote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, true };
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });
    service->SetAgentOwnerLocked(AgentOwnerKey { callingUid, "agentA" }, session);
    TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = callerRemote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    service->trackedConnections_[callerRemote] = record;
    service->callerConnectionCounts_[callingUid] = 1;

    AgentConnectPlan plan;
    plan.hostKey = hostKey;
    plan.hostUid = callingUid;
    plan.hostConnection = session->hostConnection;
    plan.needRealConnect = true;
    plan.callerRemote = callerRemote;
    plan.registeredTrackedConnection = true;

    AAFwk::Want want;
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    EXPECT_EQ(service->CompleteAgentHostConnect(want, "agentA", plan), ERR_INVALID_VALUE);
    EXPECT_TRUE(service->agentHostSessions_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
}

/**
* @tc.name  : CompleteAgentHostConnect_002
* @tc.number: CompleteAgentHostConnect_002
* @tc.desc  : Test CompleteAgentHostConnect forwards the connect request without cleanup on success
*/
HWTEST_F(AgentManagerServiceTest, CompleteAgentHostConnect_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentConnectPlan plan;
    AgentHostKey hostKey;
    hostKey.userId = IPCSkeleton::GetCallingUid() / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    plan.hostKey = hostKey;
    plan.hostUid = IPCSkeleton::GetCallingUid();
    plan.hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    AAFwk::Want want;
    want.SetElementName("", "lowcode.bundle", "LowCodeExtAbility", "entry");

    EXPECT_EQ(service->CompleteAgentHostConnect(want, "agentA", plan), ERR_OK);
    ASSERT_NE(MyFlag::lastConnectAbilityConnection, nullptr);
    EXPECT_EQ(MyFlag::lastConnectAbilityConnection->AsObject(), plan.hostConnection->AsObject());
}

/**
* @tc.name  : NotifyAgentInvoked_001
* @tc.number: NotifyAgentInvoked_001
* @tc.desc  : Test NotifyAgentInvoked returns false when no remote object is cached
*/
HWTEST_F(AgentManagerServiceTest, NotifyAgentInvoked_001, TestSize.Level1)
{
    AgentHostSession session;
    EXPECT_FALSE(session.NotifyAgentInvoked("agentA"));
}

/**
* @tc.name  : NotifyAgentInvoked_002
* @tc.number: NotifyAgentInvoked_002
* @tc.desc  : Test NotifyAgentInvoked returns false when the remote object is not an IAgentReceiver
*/
HWTEST_F(AgentManagerServiceTest, NotifyAgentInvoked_002, TestSize.Level1)
{
    AgentHostSession session;
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    session.CacheConnectDone(element, sptr<MockAbilityConnection>::MakeSptr()->AsObject(), ERR_OK);
    EXPECT_FALSE(session.NotifyAgentInvoked("agentA"));
}

/**
* @tc.name  : NotifyLowCodeConnectDoneCallbacks_001
* @tc.number: NotifyLowCodeConnectDoneCallbacks_001
* @tc.desc  : Test low-code connect callback is notified before AgentInvoked
*/
HWTEST_F(AgentManagerServiceTest, NotifyLowCodeConnectDoneCallbacks_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    std::vector<std::string> eventOrder;
    sptr<MockAbilityConnection> connection = sptr<MockAbilityConnection>::MakeSptr();
    sptr<TestAgentReceiver> receiver = sptr<TestAgentReceiver>::MakeSptr();
    connection->eventOrder = &eventOrder;
    receiver->eventOrder = &eventOrder;

    std::vector<sptr<AAFwk::IAbilityConnection>> callbacks = { connection };
    std::vector<std::string> pendingAgentIds = { "agentA" };
    service->NotifyLowCodeConnectDoneCallbacks(element, receiver->AsObject(), ERR_OK, callbacks, pendingAgentIds);

    EXPECT_THAT(eventOrder, ElementsAre("connect", "invoke"));
    EXPECT_EQ(connection->connectDoneCount, 1);
    EXPECT_EQ(receiver->agentInvokedCount, 1);
}

/**
* @tc.name  : HandleAgentHostConnectDone_001
* @tc.number: HandleAgentHostConnectDone_001
* @tc.desc  : Test HandleAgentHostConnectDone clears failed sessions and releases tracked caller slots
*/
HWTEST_F(AgentManagerServiceTest, HandleAgentHostConnectDone_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto remote = connection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[remote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { remote, true };
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });
    service->SetAgentOwnerLocked(AgentOwnerKey { callingUid, "agentA" }, session);
    TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = remote;
    record.serviceConnection = session->hostConnection;
    record.isLowCode = true;
    record.hostKey = hostKey;
    service->trackedConnections_[remote] = record;
    service->callerConnectionCounts_[callingUid] = 1;

    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    service->HandleAgentHostConnectDone(hostKey, element, nullptr, ERR_INVALID_VALUE);

    EXPECT_EQ(connection->connectDoneCount, 1);
    EXPECT_EQ(connection->lastConnectResultCode, ERR_INVALID_VALUE);
    EXPECT_TRUE(service->agentHostSessions_.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : ReleaseTrackedConnection_001
* @tc.number: ReleaseTrackedConnection_001
* @tc.desc  : Test ReleaseTrackedConnection erases tracking even when caller count entry is absent
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_001, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);

    AgentManagerService::GetInstance()->ReleaseTrackedConnection(connection);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : ReleaseTrackedConnection_002
* @tc.number: ReleaseTrackedConnection_002
* @tc.desc  : Test ReleaseTrackedConnection decrements caller count when multiple connections remain
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_002, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 2;

    AgentManagerService::GetInstance()->ReleaseTrackedConnection(connection);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_[100], 1);
}

/**
* @tc.name  : ReleaseCallerConnectionCountLocked_001
* @tc.number: ReleaseCallerConnectionCountLocked_001
* @tc.desc  : Test ReleaseCallerConnectionCountLocked returns false for unknown connection
*/
HWTEST_F(AgentManagerServiceTest, ReleaseCallerConnectionCountLocked_001, TestSize.Level1)
{
    auto callerRemote = sptr<IRemoteObject>(new (std::nothrow) IPCObjectStub(u"unknown.remote"));
    EXPECT_FALSE(AgentManagerService::GetInstance()->ReleaseCallerConnectionCountLocked(callerRemote));
}

/**
* @tc.name  : ReleaseCallerConnectionCountLocked_002
* @tc.number: ReleaseCallerConnectionCountLocked_002
* @tc.desc  : Test ReleaseCallerConnectionCountLocked returns false when caller count entry is missing
*/
HWTEST_F(AgentManagerServiceTest, ReleaseCallerConnectionCountLocked_002, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);

    EXPECT_FALSE(AgentManagerService::GetInstance()->ReleaseCallerConnectionCountLocked(connection->AsObject()));
}

/**
* @tc.name  : ReleaseCallerConnectionCountLocked_003
* @tc.number: ReleaseCallerConnectionCountLocked_003
* @tc.desc  : Test ReleaseCallerConnectionCountLocked decrements remaining count
*/
HWTEST_F(AgentManagerServiceTest, ReleaseCallerConnectionCountLocked_003, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 2;

    EXPECT_TRUE(AgentManagerService::GetInstance()->ReleaseCallerConnectionCountLocked(connection->AsObject()));
    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_[100], 1);
}

/**
* @tc.name  : ReleaseCallerConnectionCountLocked_004
* @tc.number: ReleaseCallerConnectionCountLocked_004
* @tc.desc  : Test ReleaseCallerConnectionCountLocked erases the last caller count entry
*/
HWTEST_F(AgentManagerServiceTest, ReleaseCallerConnectionCountLocked_004, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    EXPECT_TRUE(AgentManagerService::GetInstance()->ReleaseCallerConnectionCountLocked(connection->AsObject()));
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : ReleaseCallerConnectionCountLocked_005
* @tc.number: ReleaseCallerConnectionCountLocked_005
* @tc.desc  : Test ReleaseCallerConnectionCountLocked ignores non-counted connections
*/
HWTEST_F(AgentManagerServiceTest, ReleaseCallerConnectionCountLocked_005, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    record.countTowardsCallerLimit = false;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 2;

    EXPECT_TRUE(AgentManagerService::GetInstance()->ReleaseCallerConnectionCountLocked(connection->AsObject()));
    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_[100], 2);
}

/**
* @tc.name  : ReleaseTrackedConnection_003
* @tc.number: ReleaseTrackedConnection_003
* @tc.desc  : Test ReleaseTrackedConnection ignores null connection
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_003, TestSize.Level1)
{
    sptr<AAFwk::IAbilityConnection> connection = nullptr;
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    AgentManagerService::GetInstance()->ReleaseTrackedConnection(connection);

    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_[100], 1);
}

/**
* @tc.name  : ReleaseTrackedConnection_004
* @tc.number: ReleaseTrackedConnection_004
* @tc.desc  : Test ReleaseTrackedConnection ignores untracked connection
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_004, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    AgentManagerService::GetInstance()->ReleaseTrackedConnection(connection);

    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_[100], 1);
}

/**
* @tc.name  : ReleaseTrackedConnection_005
* @tc.number: ReleaseTrackedConnection_005
* @tc.desc  : Test ReleaseTrackedConnection erases non-counted tracking without changing caller quota bookkeeping
*/
HWTEST_F(AgentManagerServiceTest, ReleaseTrackedConnection_005, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    record.countTowardsCallerLimit = false;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 2;

    AgentManagerService::GetInstance()->ReleaseTrackedConnection(connection);

    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_[100], 2);
}

/**
* @tc.name  : HandleCallerConnectionDied_001
* @tc.number: HandleCallerConnectionDied_001
* @tc.desc  : Test HandleCallerConnectionDied ignores null remote object
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_001, TestSize.Level1)
{
    wptr<IRemoteObject> remote;
    AgentManagerService::GetInstance()->HandleCallerConnectionDied(remote);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection, nullptr);
}

/**
* @tc.name  : HandleCallerConnectionDied_002
* @tc.number: HandleCallerConnectionDied_002
* @tc.desc  : Test HandleCallerConnectionDied ignores unknown remote object
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_002, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new (std::nothrow) IPCObjectStub(u"test.remote");
    AgentManagerService::GetInstance()->HandleCallerConnectionDied(wptr<IRemoteObject>(remoteObject));
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection, nullptr);
}

/**
* @tc.name  : HandleCallerConnectionDied_003
* @tc.number: HandleCallerConnectionDied_003
* @tc.desc  : Test HandleCallerConnectionDied releases tracking when DisconnectAbility fails
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_003, TestSize.Level1)
{
    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto serviceConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callerConnection->AsObject();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.serviceConnection = serviceConnection;
    record.callerRemote = callerRemote;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(callerRemote, record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    AgentManagerService::GetInstance()->HandleCallerConnectionDied(wptr<IRemoteObject>(callerRemote));
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection->AsObject(), serviceConnection->AsObject());
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
    MyFlag::retDisconnectAbility = ERR_OK;
}

/**
* @tc.name  : HandleCallerConnectionDied_004
* @tc.number: HandleCallerConnectionDied_004
* @tc.desc  : Test HandleCallerConnectionDied releases tracking even when no service connection is stored
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_004, TestSize.Level1)
{
    auto callerConnection = sptr<MockAbilityConnection>::MakeSptr();
    auto callerRemote = callerConnection->AsObject();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = callerRemote;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(callerRemote, record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    AgentManagerService::GetInstance()->HandleCallerConnectionDied(wptr<IRemoteObject>(callerRemote));

    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection, nullptr);
    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
* @tc.name  : HandleCallerConnectionDied_005
* @tc.number: HandleCallerConnectionDied_005
* @tc.desc  : Test HandleCallerConnectionDied removes only the dead low-code caller when other agents remain
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_005, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto connectionA = sptr<MockAbilityConnection>::MakeSptr();
    auto connectionB = sptr<MockAbilityConnection>::MakeSptr();
    auto remoteA = connectionA->AsObject();
    auto remoteB = connectionB->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[remoteA] = connectionA;
    session->callerConnections[remoteB] = connectionB;
    session->agents["agentA"] = LowCodeAgentRecord { remoteA, false };
    session->agents["agentB"] = LowCodeAgentRecord { remoteB, false };
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });
    service->SetAgentOwnerLocked(AgentOwnerKey { callingUid, "agentA" }, session);
    service->SetAgentOwnerLocked(AgentOwnerKey { callingUid, "agentB" }, session);

    TrackedConnectionRecord recordA;
    recordA.callerUid = callingUid;
    recordA.callerRemote = remoteA;
    recordA.serviceConnection = session->hostConnection;
    recordA.hostKey = hostKey;
    recordA.isLowCode = true;
    service->trackedConnections_[remoteA] = recordA;

    TrackedConnectionRecord recordB;
    recordB.callerUid = callingUid;
    recordB.callerRemote = remoteB;
    recordB.serviceConnection = session->hostConnection;
    recordB.hostKey = hostKey;
    recordB.isLowCode = true;
    service->trackedConnections_[remoteB] = recordB;
    service->callerConnectionCounts_[callingUid] = 2;

    service->HandleCallerConnectionDied(wptr<IRemoteObject>(remoteA));

    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_EQ(service->agentHostSessions_.begin()->session->callerConnections.size(), 1);
    EXPECT_EQ(service->agentHostSessions_.begin()->session->agents.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->session->agents.count("agentB") > 0);
    ASSERT_EQ(service->agentOwners_.size(), 1);
    EXPECT_TRUE(service->FindAgentOwnerLocked(
        AgentOwnerKey { callingUid, "agentB" }) != service->agentOwners_.end());
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    EXPECT_TRUE(service->trackedConnections_.count(remoteB) > 0);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
* @tc.name  : HandleCallerConnectionDied_006
* @tc.number: HandleCallerConnectionDied_006
* @tc.desc  : Test HandleCallerConnectionDied tears down the shared low-code host after the last agent is removed
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_006, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto remote = connection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[remote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { remote, false };
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });
    service->SetAgentOwnerLocked(AgentOwnerKey { callingUid, "agentA" }, session);

    TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = remote;
    record.serviceConnection = session->hostConnection;
    record.hostKey = hostKey;
    record.isLowCode = true;
    service->trackedConnections_[remote] = record;
    service->callerConnectionCounts_[callingUid] = 1;

    service->HandleCallerConnectionDied(wptr<IRemoteObject>(remote));

    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_TRUE(service->agentHostSessions_.begin()->session->isDisconnecting);
    EXPECT_TRUE(service->agentHostSessions_.begin()->session->callerConnections.empty());
    EXPECT_TRUE(service->agentHostSessions_.begin()->session->agents.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
    ASSERT_NE(MyFlag::lastDisconnectAbilityConnection, nullptr);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection->AsObject(), session->hostConnection->AsObject());

    AppExecFwk::ElementName element("", "lowcode.bundle", "LowCodeExtAbility", "entry");
    service->HandleAgentHostDisconnectDone(hostKey, element, ERR_OK);
    EXPECT_TRUE(service->agentHostSessions_.empty());
}

/**
* @tc.name  : HandleCallerConnectionDied_007
* @tc.number: HandleCallerConnectionDied_007
* @tc.desc  : Test HandleCallerConnectionDied removes only the dead standard owner when others survive
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_007, TestSize.Level1)
{
    auto connection1 = sptr<MockAbilityConnection>::MakeSptr();
    auto connection2 = sptr<MockAbilityConnection>::MakeSptr();
    auto service = AgentManagerService::GetInstance();
    auto serviceConnection1 = sptr<AgentServiceConnection>::MakeSptr(connection1);
    auto serviceConnection2 = sptr<AgentServiceConnection>::MakeSptr(connection2);
    auto key = BuildTestStandardKey();
    auto session = AddTestStandardSession(service, key, serviceConnection1,
        { connection1->AsObject(), connection2->AsObject() });
    session->MarkConnected();
    session->AddServiceConnection(serviceConnection1, connection1);
    session->AddServiceConnection(serviceConnection2, connection2);
    AddTestStandardTrackedConnection(service, key, connection1, serviceConnection1, true);
    AddTestStandardTrackedConnection(service, key, connection2, serviceConnection2, false);
    service->callerConnectionCounts_[key.callerUid] = 1;

    ASSERT_EQ(service->standardSessions_.size(), 1);

    service->HandleCallerConnectionDied(wptr<IRemoteObject>(connection1->AsObject()));

    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_NE(MyFlag::lastDisconnectAbilityConnection, nullptr);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection->AsObject(), serviceConnection1->AsObject());
    ASSERT_EQ(service->standardSessions_.size(), 1);
    EXPECT_EQ(session->callerRemotes.size(), 1);
    EXPECT_EQ(session->callerRemotes.front(), connection2->AsObject());
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    auto survivingIt = service->trackedConnections_.find(connection2->AsObject());
    ASSERT_NE(survivingIt, service->trackedConnections_.end());
    EXPECT_TRUE(survivingIt->second.countTowardsCallerLimit);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_[key.callerUid], 1);
}

/**
* @tc.name  : HandleCallerConnectionDied_009
* @tc.number: HandleCallerConnectionDied_009
* @tc.desc  : Test HandleCallerConnectionDied tears down the standard backing session after the last owner dies
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_009, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto service = AgentManagerService::GetInstance();
    auto serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    auto key = BuildTestStandardKey();
    auto session = AddTestStandardSession(service, key, serviceConnection, { connection->AsObject() });
    session->MarkConnected();
    session->AddServiceConnection(serviceConnection, connection);
    AddTestStandardTrackedConnection(service, key, connection, serviceConnection, true);
    service->callerConnectionCounts_[key.callerUid] = 1;

    service->HandleCallerConnectionDied(wptr<IRemoteObject>(connection->AsObject()));

    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_NE(MyFlag::lastDisconnectAbilityConnection, nullptr);
    EXPECT_EQ(MyFlag::lastDisconnectAbilityConnection->AsObject(), serviceConnection->AsObject());
    EXPECT_TRUE(service->standardSessions_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
* @tc.name  : HandleCallerConnectionDied_008
* @tc.number: HandleCallerConnectionDied_008
* @tc.desc  : Test HandleCallerConnectionDied rolls back low-code host disconnecting flag when AMS disconnect fails
*/
HWTEST_F(AgentManagerServiceTest, HandleCallerConnectionDied_008, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    int32_t callingUid = IPCSkeleton::GetCallingUid();
    AgentHostKey hostKey;
    hostKey.userId = callingUid / 200000;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    auto remote = connection->AsObject();
    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = callingUid;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    session->callerConnections[remote] = connection;
    session->agents["agentA"] = LowCodeAgentRecord { remote, false };
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });
    service->SetAgentOwnerLocked(AgentOwnerKey { callingUid, "agentA" }, session);

    TrackedConnectionRecord record;
    record.callerUid = callingUid;
    record.callerRemote = remote;
    record.serviceConnection = session->hostConnection;
    record.hostKey = hostKey;
    record.isLowCode = true;
    service->trackedConnections_[remote] = record;
    service->callerConnectionCounts_[callingUid] = 1;
    MyFlag::retDisconnectAbility = ERR_INVALID_VALUE;

    service->HandleCallerConnectionDied(wptr<IRemoteObject>(remote));

    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    ASSERT_EQ(service->agentHostSessions_.size(), 1);
    EXPECT_FALSE(service->agentHostSessions_.begin()->session->isDisconnecting);
    EXPECT_TRUE(service->agentHostSessions_.begin()->session->callerConnections.empty());
    EXPECT_TRUE(service->agentHostSessions_.begin()->session->agents.empty());
    EXPECT_TRUE(service->agentOwners_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
    MyFlag::retDisconnectAbility = ERR_OK;
}

/**
* @tc.name  : HandleConnectionDone_001
* @tc.number: HandleConnectionDone_001
* @tc.desc  : Test HandleConnectionDone keeps tracking on successful connect callback
*/
HWTEST_F(AgentManagerServiceTest, HandleConnectionDone_001, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    AgentManagerService::GetInstance()->HandleConnectionDone(connection, ERR_OK, false);

    ASSERT_EQ(AgentManagerService::GetInstance()->trackedConnections_.size(), 1);
    ASSERT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(AgentManagerService::GetInstance()->callerConnectionCounts_[100], 1);
}

/**
* @tc.name  : HandleConnectionDone_002
* @tc.number: HandleConnectionDone_002
* @tc.desc  : Test HandleConnectionDone releases tracking on disconnect callback
*/
HWTEST_F(AgentManagerServiceTest, HandleConnectionDone_002, TestSize.Level1)
{
    auto connection = sptr<MockAbilityConnection>::MakeSptr();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    AgentManagerService::GetInstance()->trackedConnections_.emplace(connection->AsObject(), record);
    AgentManagerService::GetInstance()->callerConnectionCounts_[100] = 1;

    AgentManagerService::GetInstance()->HandleConnectionDone(connection, ERR_OK, true);

    EXPECT_TRUE(AgentManagerService::GetInstance()->trackedConnections_.empty());
    EXPECT_TRUE(AgentManagerService::GetInstance()->callerConnectionCounts_.empty());
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_033
 * @tc.number: ConnectAgentExtensionAbility_033
 * @tc.desc  : Test standard agent duplicate connect reuses one session but connects each owner
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_033, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::agentCardModuleName = "entry";
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;

    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");

    sptr<MockAbilityConnection> connection1 = new MockAbilityConnection();
    sptr<MockAbilityConnection> connection2 = new MockAbilityConnection();

    auto service = AgentManagerService::GetInstance();

    // First connect
    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection1), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);

    // Second connect (same caller+agentId+target) reuses the session but still connects this owner.
    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection2), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 2);

    // Only one quota should be consumed
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);

    // Only one standard session should exist
    ASSERT_EQ(service->standardSessions_.size(), 1);
    auto &session = service->standardSessions_.front();
    ASSERT_EQ(session->pendingCallbacks.size(), 2);
    ASSERT_EQ(session->standardServiceConnections.size(), 2);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_034
 * @tc.number: ConnectAgentExtensionAbility_034
 * @tc.desc  : Test standard agent session notifies all pending callers on connect done
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_034, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::agentCardModuleName = "entry";
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;

    AAFwk::Want wantWithoutModule;
    wantWithoutModule.SetParam(AGENTID_KEY, std::string("testAgent"));
    wantWithoutModule.SetBundle("test.bundle");
    wantWithoutModule.SetElementName("", "test.bundle", "TestAbility", "");

    AAFwk::Want wantWithModule;
    wantWithModule.SetParam(AGENTID_KEY, std::string("testAgent"));
    wantWithModule.SetBundle("test.bundle");
    wantWithModule.SetElementName("", "test.bundle", "TestAbility", "entry");

    sptr<MockAbilityConnection> connection1 = new MockAbilityConnection();
    sptr<MockAbilityConnection> connection2 = new MockAbilityConnection();

    auto service = AgentManagerService::GetInstance();

    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantWithoutModule, connection1), ERR_OK);
    auto serviceConnection1 = MyFlag::lastConnectAbilityConnection;
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantWithModule, connection2), ERR_OK);
    auto serviceConnection2 = MyFlag::lastConnectAbilityConnection;

    ASSERT_EQ(service->standardSessions_.size(), 1);

    // Simulate AMS connect done
    AppExecFwk::ElementName element("", "test.bundle", "TestAbility");
    sptr<MockAbilityConnection> mockRemote = new MockAbilityConnection();
    auto *rawSvcConn1 = static_cast<AgentServiceConnection *>(serviceConnection1.GetRefPtr());
    rawSvcConn1->OnAbilityConnectDone(element, mockRemote->AsObject(), ERR_OK);
    auto *rawSvcConn2 = static_cast<AgentServiceConnection *>(serviceConnection2.GetRefPtr());
    rawSvcConn2->OnAbilityConnectDone(element, mockRemote->AsObject(), ERR_OK);

    // Both callers should be notified
    EXPECT_EQ(connection1->connectDoneCount, 1);
    EXPECT_EQ(connection2->connectDoneCount, 1);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_035
 * @tc.number: ConnectAgentExtensionAbility_035
 * @tc.desc  : Test standard agent duplicate connect after connected triggers owner connect
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_035, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;

    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");

    sptr<MockAbilityConnection> connection1 = new MockAbilityConnection();
    sptr<MockAbilityConnection> connection2 = new MockAbilityConnection();
    sptr<MockAbilityConnection> connection3 = new MockAbilityConnection();

    auto service = AgentManagerService::GetInstance();

    // First connect
    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection1), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);

    // Simulate connect done
    ASSERT_EQ(service->standardSessions_.size(), 1);
    auto svcConn = service->standardSessions_.front()->serviceConnection;
    AppExecFwk::ElementName element("", "test.bundle", "TestAbility");
    sptr<MockAbilityConnection> mockRemote = new MockAbilityConnection();
    auto *rawSvcConn = static_cast<AgentServiceConnection *>(svcConn.GetRefPtr());
    rawSvcConn->OnAbilityConnectDone(element, mockRemote->AsObject(), ERR_OK);
    EXPECT_EQ(connection1->connectDoneCount, 1);

    // Second connect reuses the session but still goes through AMS so target onConnect runs for this owner.
    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection2), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 2);
    auto serviceConnection2 = MyFlag::lastConnectAbilityConnection;
    auto *rawSvcConn2 = static_cast<AgentServiceConnection *>(serviceConnection2.GetRefPtr());
    rawSvcConn2->OnAbilityConnectDone(element, mockRemote->AsObject(), ERR_OK);
    EXPECT_EQ(connection2->connectDoneCount, 1);

    // Third connect gets the same behavior.
    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection3), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 3);
    auto serviceConnection3 = MyFlag::lastConnectAbilityConnection;
    auto *rawSvcConn3 = static_cast<AgentServiceConnection *>(serviceConnection3.GetRefPtr());
    rawSvcConn3->OnAbilityConnectDone(element, mockRemote->AsObject(), ERR_OK);
    EXPECT_EQ(connection3->connectDoneCount, 1);

    // Still only one quota
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_036
 * @tc.number: ConnectAgentExtensionAbility_036
 * @tc.desc  : Test standard agent connect with empty moduleName still deduplicates
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_036, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::agentCardModuleName = "entry";
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;

    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("testAgent"));
    wantA.SetBundle("test.bundle");
    wantA.SetElementName("", "test.bundle", "TestAbility", "");

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("testAgent"));
    wantB.SetBundle("test.bundle");
    wantB.SetElementName("", "test.bundle", "TestAbility", "entry");

    sptr<MockAbilityConnection> connection1 = new MockAbilityConnection();
    sptr<MockAbilityConnection> connection2 = new MockAbilityConnection();

    auto service = AgentManagerService::GetInstance();

    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantA, connection1), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    ASSERT_EQ(service->standardSessions_.size(), 1);
    ASSERT_NE(service->standardSessions_.front(), nullptr);
    EXPECT_EQ(service->standardSessions_.front()->key.moduleName, "entry");

    // Different moduleName still reuses one standard session after resolution.
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connection2), ERR_OK);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 2);
    EXPECT_EQ(service->standardSessions_.size(), 1);
}

/**
 * @tc.name  : DisconnectAgentExtensionAbility_011
 * @tc.number: DisconnectAgentExtensionAbility_011
 * @tc.desc  : Test standard agent disconnect removes only the selected caller from a shared session
 */
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_011, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;

    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");

    sptr<MockAbilityConnection> connection1 = new MockAbilityConnection();
    sptr<MockAbilityConnection> connection2 = new MockAbilityConnection();

    auto service = AgentManagerService::GetInstance();

    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection1), ERR_OK);
    auto serviceConnection1 = MyFlag::lastConnectAbilityConnection;
    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection2), ERR_OK);
    auto serviceConnection2 = MyFlag::lastConnectAbilityConnection;

    // Simulate connect done
    ASSERT_EQ(service->standardSessions_.size(), 1);
    AppExecFwk::ElementName element("", "test.bundle", "TestAbility");
    sptr<MockAbilityConnection> mockRemote = new MockAbilityConnection();
    auto *rawSvcConn1 = static_cast<AgentServiceConnection *>(serviceConnection1.GetRefPtr());
    rawSvcConn1->OnAbilityConnectDone(element, mockRemote->AsObject(), ERR_OK);
    auto *rawSvcConn2 = static_cast<AgentServiceConnection *>(serviceConnection2.GetRefPtr());
    rawSvcConn2->OnAbilityConnectDone(element, mockRemote->AsObject(), ERR_OK);

    // Disconnect with the duplicate caller callback; it should tear down only that logical proxy.
    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connection2), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 1);
    auto *rawDisconnectSvcConn = static_cast<AgentServiceConnection *>(
        MyFlag::lastDisconnectAbilityConnection.GetRefPtr());
    rawDisconnectSvcConn->OnAbilityDisconnectDone(element, ERR_OK);
    EXPECT_EQ(connection1->disconnectDoneCount, 0);
    EXPECT_EQ(connection2->disconnectDoneCount, 1);

    // Session should remain for the surviving caller.
    ASSERT_EQ(service->standardSessions_.size(), 1);
    EXPECT_EQ(service->standardSessions_.front()->callerRemotes.size(), 1);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    auto survivingIt = service->trackedConnections_.find(connection1->AsObject());
    ASSERT_NE(survivingIt, service->trackedConnections_.end());
    EXPECT_TRUE(survivingIt->second.countTowardsCallerLimit);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_037
 * @tc.number: ConnectAgentExtensionAbility_037
 * @tc.desc  : Test different targets create different standard sessions
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_037, TestSize.Level1)
{
    MyFlag::retVerifyCallingPermission = true;
    MyFlag::retGetProcessRunningInfoByPid = ERR_OK;
    MyFlag::processState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    MyFlag::retGetAgentCardByAgentId = ERR_OK;
    MyFlag::agentCardAgentId = "testAgent";
    MyFlag::agentCardType = static_cast<int32_t>(AgentCardType::APP);
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;

    AAFwk::Want wantA;
    wantA.SetParam(AGENTID_KEY, std::string("testAgent"));
    wantA.SetBundle("test.bundle");
    wantA.SetElementName("test.bundle", "AbilityA");

    AAFwk::Want wantB;
    wantB.SetParam(AGENTID_KEY, std::string("testAgent"));
    wantB.SetBundle("test.bundle");
    wantB.SetElementName("test.bundle", "AbilityB");

    sptr<MockAbilityConnection> connectionA = new MockAbilityConnection();
    sptr<MockAbilityConnection> connectionB = new MockAbilityConnection();

    auto service = AgentManagerService::GetInstance();

    MyFlag::agentCardAbilityName = "AbilityA";
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantA, connectionA), ERR_OK);
    MyFlag::agentCardAbilityName = "AbilityB";
    EXPECT_EQ(service->ConnectAgentExtensionAbility(wantB, connectionB), ERR_OK);

    // Two different targets should trigger two AMS connects
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 2);
    ASSERT_EQ(service->standardSessions_.size(), 2);

    // Two quotas consumed
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 2);
}

/**
 * @tc.name  : ConnectAgentExtensionAbility_038
 * @tc.number: ConnectAgentExtensionAbility_038
 * @tc.desc  : Test standard agent connect failure cleans failed owners and the empty session
 */
HWTEST_F(AgentManagerServiceTest, ConnectAgentExtensionAbility_038, TestSize.Level1)
{
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;

    AAFwk::Want want;
    want.SetParam(AGENTID_KEY, std::string("testAgent"));
    want.SetBundle("test.bundle");
    want.SetElementName("test.bundle", "TestAbility");

    sptr<MockAbilityConnection> connection1 = new MockAbilityConnection();
    sptr<MockAbilityConnection> connection2 = new MockAbilityConnection();
    auto service = AgentManagerService::GetInstance();

    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection1), ERR_OK);
    auto serviceConnection1 = MyFlag::lastConnectAbilityConnection;
    EXPECT_EQ(service->ConnectAgentExtensionAbility(want, connection2), ERR_OK);
    auto serviceConnection2 = MyFlag::lastConnectAbilityConnection;
    ASSERT_EQ(service->standardSessions_.size(), 1);

    AppExecFwk::ElementName element("", "test.bundle", "TestAbility");
    auto *rawSvcConn1 = static_cast<AgentServiceConnection *>(serviceConnection1.GetRefPtr());
    rawSvcConn1->OnAbilityConnectDone(element, nullptr, ERR_INVALID_VALUE);
    auto *rawSvcConn2 = static_cast<AgentServiceConnection *>(serviceConnection2.GetRefPtr());
    rawSvcConn2->OnAbilityConnectDone(element, nullptr, ERR_INVALID_VALUE);

    EXPECT_EQ(connection1->connectDoneCount, 1);
    EXPECT_EQ(connection1->lastConnectResultCode, ERR_INVALID_VALUE);
    EXPECT_EQ(connection2->connectDoneCount, 1);
    EXPECT_EQ(connection2->lastConnectResultCode, ERR_INVALID_VALUE);
    EXPECT_TRUE(service->standardSessions_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
 * @tc.name  : DisconnectAgentExtensionAbility_012
 * @tc.number: DisconnectAgentExtensionAbility_012
 * @tc.desc  : Test standard agent disconnect returns OK without AMS call when session already disconnecting
 */
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_012, TestSize.Level1)
{
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    auto service = AgentManagerService::GetInstance();
    auto serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    auto key = BuildTestStandardKey();
    AddTestStandardSession(service, key, serviceConnection, { connection->AsObject() }, true);
    AddTestStandardTrackedConnection(service, key, connection, serviceConnection, true);
    service->callerConnectionCounts_[key.callerUid] = 1;

    ASSERT_EQ(service->standardSessions_.size(), 1);

    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connection), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
 * @tc.name  : DisconnectAgentExtensionAbility_013
 * @tc.number: DisconnectAgentExtensionAbility_013
 * @tc.desc  : Test shared standard agent owner disconnect is local and keeps the backing session
 */
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_013, TestSize.Level1)
{
    sptr<MockAbilityConnection> connection1 = new MockAbilityConnection();
    sptr<MockAbilityConnection> connection2 = new MockAbilityConnection();
    auto service = AgentManagerService::GetInstance();
    auto svcConn = sptr<AgentServiceConnection>::MakeSptr(connection1);
    auto key = BuildTestStandardKey();
    AddTestStandardSession(service, key, svcConn, { connection1->AsObject(), connection2->AsObject() });
    AddTestStandardTrackedConnection(service, key, connection1, svcConn, true);
    AddTestStandardTrackedConnection(service, key, connection2, svcConn, false);
    service->callerConnectionCounts_[key.callerUid] = 1;

    ASSERT_EQ(service->standardSessions_.size(), 1);

    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connection2), ERR_OK);
    EXPECT_EQ(MyFlag::disconnectAbilityCallCount, 0);
    EXPECT_EQ(connection1->disconnectDoneCount, 0);
    EXPECT_EQ(connection2->disconnectDoneCount, 1);

    ASSERT_EQ(service->standardSessions_.size(), 1);
    EXPECT_FALSE(service->standardSessions_.front()->isDisconnecting);
    auto trackedIt = service->trackedConnections_.find(connection2->AsObject());
    ASSERT_EQ(trackedIt, service->trackedConnections_.end());
    EXPECT_EQ(service->standardSessions_.front()->callerRemotes.size(), 1);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_.begin()->second, 1);
}

/**
 * @tc.name  : DisconnectAgentExtensionAbility_014
 * @tc.number: DisconnectAgentExtensionAbility_014
 * @tc.desc  : Test async standard disconnect failure restores active state without notifying disconnect done
 */
HWTEST_F(AgentManagerServiceTest, DisconnectAgentExtensionAbility_014, TestSize.Level1)
{
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    auto service = AgentManagerService::GetInstance();
    auto svcConn = sptr<AgentServiceConnection>::MakeSptr(connection);
    svcConn->SetStandardSessionMode();
    auto key = BuildTestStandardKey();
    auto session = AddTestStandardSession(service, key, svcConn, { connection->AsObject() });
    session->state = StandardAgentState::CONNECTED;
    AddTestStandardTrackedConnection(service, key, connection, svcConn, true);
    service->callerConnectionCounts_[key.callerUid] = 1;

    EXPECT_EQ(service->DisconnectAgentExtensionAbility(connection), ERR_OK);
    EXPECT_TRUE(session->isDisconnecting);
    EXPECT_EQ(session->state, StandardAgentState::DISCONNECTING_BACKING);
    EXPECT_TRUE(service->callerConnectionCounts_.empty());

    AppExecFwk::ElementName element("", "test.bundle", "TestAbility");
    svcConn->OnAbilityDisconnectDone(element, ERR_INVALID_VALUE);

    EXPECT_EQ(connection->disconnectDoneCount, 0);
    ASSERT_EQ(service->standardSessions_.size(), 1);
    EXPECT_FALSE(session->isDisconnecting);
    EXPECT_EQ(session->state, StandardAgentState::ACTIVE);
    auto trackedIt = service->trackedConnections_.find(connection->AsObject());
    ASSERT_NE(trackedIt, service->trackedConnections_.end());
    EXPECT_FALSE(trackedIt->second.isDisconnecting);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_[key.callerUid], 1);
}

/**
 * @tc.name  : HandleStandardAgentConnectDone_001
 * @tc.number: HandleStandardAgentConnectDone_001
 * @tc.desc  : Test unknown standard service remote is ignored
 */
HWTEST_F(AgentManagerServiceTest, HandleStandardAgentConnectDone_001, TestSize.Level1)
{
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    StandardAgentKey key;
    key.callerUid = IPCSkeleton::GetCallingUid();
    key.agentId = "testAgent";
    key.bundleName = "test.bundle";
    key.abilityName = "TestAbility";

    auto service = AgentManagerService::GetInstance();
    auto session = std::make_shared<AgentStandardRecord>();
    session->key = key;
    session->pendingCallbacks.push_back(connection);
    session->serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    service->standardSessions_.emplace_back(session);

    AppExecFwk::ElementName element("", "test.bundle", "TestAbility");
    sptr<MockAbilityConnection> unknownConnection = new MockAbilityConnection();
    service->HandleStandardAgentConnectDone(unknownConnection->AsObject(), element, unknownConnection->AsObject(),
        ERR_OK);

    EXPECT_EQ(connection->connectDoneCount, 0);
    ASSERT_EQ(service->standardSessions_.size(), 1);
    EXPECT_EQ(service->standardSessions_.front()->state,
        StandardAgentState::CONNECTING);
}

/**
 * @tc.name  : FindStandardSessionLocked_001
 * @tc.number: FindStandardSessionLocked_001
 * @tc.desc  : Test terminal and disconnecting standard sessions are not reusable
 */
HWTEST_F(AgentManagerServiceTest, FindStandardSessionLocked_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto key = BuildTestStandardKey();
    auto session = std::make_shared<AgentStandardRecord>(key);
    service->standardSessions_.push_back(session);

    session->state = StandardAgentState::CONNECTED;
    EXPECT_NE(service->FindStandardSessionLocked(key), nullptr);

    session->state = StandardAgentState::DISCONNECT_REQUESTED;
    EXPECT_EQ(service->FindStandardSessionLocked(key), nullptr);

    session->state = StandardAgentState::DISCONNECTING_BACKING;
    session->isDisconnecting = true;
    EXPECT_EQ(service->FindStandardSessionLocked(key), nullptr);

    session->isDisconnecting = false;
    session->state = StandardAgentState::FAILED;
    EXPECT_EQ(service->FindStandardSessionLocked(key), nullptr);

    session->state = StandardAgentState::DISCONNECTED;
    EXPECT_EQ(service->FindStandardSessionLocked(key), nullptr);
}

/**
 * @tc.name  : StandardAgentKey_001
 * @tc.number: StandardAgentKey_001
 * @tc.desc  : Test standard session key matching handles explicit and empty moduleName correctly
 */
HWTEST_F(AgentManagerServiceTest, StandardAgentKey_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    StandardAgentKey keyModuleA;
    keyModuleA.callerUid = IPCSkeleton::GetCallingUid();
    keyModuleA.agentId = "testAgent";
    keyModuleA.bundleName = "test.bundle";
    keyModuleA.moduleName = "moduleA";
    keyModuleA.abilityName = "TestAbility";

    StandardAgentKey keyModuleB = keyModuleA;
    keyModuleB.moduleName = "moduleB";
    StandardAgentKey keyWithoutModule = keyModuleA;
    keyWithoutModule.moduleName = "";

    EXPECT_FALSE(service->IsStandardAgentKeyEqual(keyModuleA, keyModuleB));
    EXPECT_FALSE(service->IsStandardAgentKeyMatched(keyModuleA, keyModuleB));
    EXPECT_TRUE(service->IsStandardAgentKeyEqual(keyModuleA, keyWithoutModule));
    EXPECT_TRUE(service->IsStandardAgentKeyMatched(keyModuleA, keyWithoutModule));
    EXPECT_TRUE(service->IsStandardAgentKeyMatched(keyWithoutModule, keyModuleA));
}

/**
 * @tc.name  : LowCodeHostKey_001
 * @tc.number: LowCodeHostKey_001
 * @tc.desc  : Test low-code host key matching handles moduleName and target identity branches
 */
HWTEST_F(AgentManagerServiceTest, LowCodeHostKey_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentHostKey key;
    key.userId = 100;
    key.bundleName = "test.bundle";
    key.moduleName = "moduleA";
    key.abilityName = "TestAbility";

    AgentHostKey sameTarget = key;
    EXPECT_TRUE(service->IsLowCodeHostKeyMatched(key, sameTarget));

    AgentHostKey otherModule = key;
    otherModule.moduleName = "moduleB";
    EXPECT_FALSE(service->IsLowCodeHostKeyMatched(key, otherModule));

    AgentHostKey emptyModule = key;
    emptyModule.moduleName = "";
    EXPECT_TRUE(service->IsLowCodeHostKeyMatched(key, emptyModule));
    EXPECT_TRUE(service->IsLowCodeHostKeyMatched(emptyModule, key));

    AgentHostKey otherAbility = key;
    otherAbility.abilityName = "OtherAbility";
    EXPECT_FALSE(service->IsLowCodeHostKeyMatched(key, otherAbility));

    AgentHostKey otherBundle = key;
    otherBundle.bundleName = "other.bundle";
    EXPECT_FALSE(service->IsLowCodeHostKeyMatched(key, otherBundle));

    AgentHostKey otherUser = key;
    otherUser.userId = 101;
    EXPECT_FALSE(service->IsLowCodeHostKeyMatched(key, otherUser));
}

/**
 * @tc.name  : AgentHostSessionIndex_001
 * @tc.number: AgentHostSessionIndex_001
 * @tc.desc  : Test low-code host session lookup and owner index helper branches
 */
HWTEST_F(AgentManagerServiceTest, AgentHostSessionIndex_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentHostKey hostKey;
    hostKey.userId = 100;
    hostKey.bundleName = "test.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "TestAbility";

    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostUid = 200;
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });

    AgentHostKey findKey = hostKey;
    findKey.moduleName = "";
    auto sessionIter = service->FindAgentHostSessionLocked(findKey);
    ASSERT_NE(sessionIter, service->agentHostSessions_.end());
    EXPECT_EQ(sessionIter->session, session);

    AgentHostKey missingKey = hostKey;
    missingKey.abilityName = "MissingAbility";
    EXPECT_EQ(service->FindAgentHostSessionLocked(missingKey), service->agentHostSessions_.end());

    AgentOwnerKey ownerKey { session->hostUid, "agentA" };
    AgentOwnerKey sameOwnerKey { session->hostUid, "agentA" };
    AgentOwnerKey otherOwnerKey { session->hostUid, "agentB" };
    EXPECT_TRUE(service->IsAgentOwnerKeyEqual(ownerKey, sameOwnerKey));
    EXPECT_FALSE(service->IsAgentOwnerKeyEqual(ownerKey, otherOwnerKey));

    service->SetAgentOwnerLocked(ownerKey, session);
    auto ownerIter = service->FindAgentOwnerLocked(ownerKey);
    ASSERT_NE(ownerIter, service->agentOwners_.end());
    EXPECT_EQ(ownerIter->session, session);

    auto replacementSession = std::make_shared<AgentHostSession>();
    service->SetAgentOwnerLocked(ownerKey, replacementSession);
    EXPECT_EQ(service->agentOwners_.size(), 1);
    ownerIter = service->FindAgentOwnerLocked(ownerKey);
    ASSERT_NE(ownerIter, service->agentOwners_.end());
    EXPECT_EQ(ownerIter->session, replacementSession);

    service->EraseAgentOwnerLocked(ownerKey);
    EXPECT_EQ(service->FindAgentOwnerLocked(ownerKey), service->agentOwners_.end());
}

/**
 * @tc.name  : TrackedConnectionHelpers_001
 * @tc.number: TrackedConnectionHelpers_001
 * @tc.desc  : Test tracked connection helper validation and record creation branches
 */
HWTEST_F(AgentManagerServiceTest, TrackedConnectionHelpers_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    size_t currentCount = 0;
    EXPECT_EQ(service->ValidateTrackedConnectionRegistrationLocked(nullptr, 100, true, currentCount),
        ERR_INVALID_VALUE);

    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    auto callerRemote = connection->AsObject();
    EXPECT_EQ(service->ValidateTrackedConnectionRegistrationLocked(callerRemote, 100, true, currentCount), ERR_OK);
    EXPECT_EQ(currentCount, 0);

    auto serviceConnection = service->GetOrCreateServiceConnection(connection, nullptr);
    ASSERT_NE(serviceConnection, nullptr);
    auto providedConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    EXPECT_EQ(service->GetOrCreateServiceConnection(connection, providedConnection)->AsObject(),
        providedConnection->AsObject());

    AgentHostKey hostKey;
    hostKey.bundleName = "test.bundle";
    auto record = service->BuildTrackedConnectionRecord(callerRemote, 100, serviceConnection, &hostKey, true);
    EXPECT_EQ(record.callerRemote, callerRemote);
    EXPECT_EQ(record.serviceConnection, serviceConnection);
    EXPECT_TRUE(record.isLowCode);
    EXPECT_TRUE(record.countTowardsCallerLimit);
    EXPECT_NE(record.deathRecipient, nullptr);
}

/**
 * @tc.name  : TrackedConnectionHelpers_002
 * @tc.number: TrackedConnectionHelpers_002
 * @tc.desc  : Test tracked connection validation detects duplicate and max-count branches
 */
HWTEST_F(AgentManagerServiceTest, TrackedConnectionHelpers_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    auto callerRemote = connection->AsObject();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = callerRemote;
    service->trackedConnections_[callerRemote] = record;

    size_t currentCount = 0;
    EXPECT_EQ(service->ValidateTrackedConnectionRegistrationLocked(callerRemote, 100, true, currentCount),
        ERR_INVALID_VALUE);

    sptr<MockAbilityConnection> otherConnection = new MockAbilityConnection();
    service->callerConnectionCounts_[100] = AgentManagerService::MAX_CONNECTIONS_PER_CALLER;
    EXPECT_EQ(service->ValidateTrackedConnectionRegistrationLocked(otherConnection->AsObject(), 100, true,
        currentCount), AAFwk::ERR_MAX_AGENT_CONNECTIONS_REACHED);
}

/**
 * @tc.name  : LowCodeConnectPlanHelpers_001
 * @tc.number: LowCodeConnectPlanHelpers_001
 * @tc.desc  : Test low-code connect plan validation and session creation helpers
 */
HWTEST_F(AgentManagerServiceTest, LowCodeConnectPlanHelpers_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AgentHostKey hostKey;
    hostKey.userId = 100;
    hostKey.bundleName = "test.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "TestAbility";
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();

    AgentConnectPlan plan;
    sptr<IRemoteObject> hostProxyRemote = nullptr;
    EXPECT_EQ(service->ValidateLowCodeConnectPlanLocked(connection, hostKey, plan, hostProxyRemote), ERR_OK);
    EXPECT_EQ(hostProxyRemote, connection->AsObject());

    std::shared_ptr<AgentHostSession> session;
    EXPECT_EQ(service->GetOrCreateLowCodeSessionLocked(hostKey, 200, hostProxyRemote, plan, session), ERR_OK);
    ASSERT_NE(session, nullptr);
    EXPECT_TRUE(plan.needRealConnect);
    EXPECT_EQ(service->EnsureLowCodeHostProxyConnectionLocked(hostKey, hostProxyRemote, plan, session), ERR_OK);
    EXPECT_FALSE(session->hostProxyConnections.empty());
}

/**
 * @tc.name  : LowCodeCompleteHelpers_001
 * @tc.number: LowCodeCompleteHelpers_001
 * @tc.desc  : Test low-code completion helper detects sibling agents for the same caller proxy
 */
HWTEST_F(AgentManagerServiceTest, LowCodeCompleteHelpers_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    auto session = std::make_shared<AgentHostSession>();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    auto callerRemote = connection->AsObject();
    session->agents["agentA"] = LowCodeAgentRecord { callerRemote, false, callerRemote };
    session->agents["agentB"] = LowCodeAgentRecord { callerRemote, false, callerRemote };

    EXPECT_TRUE(session->HasOtherAgentsForCaller("agentA", callerRemote, callerRemote));
    EXPECT_FALSE(session->HasOtherAgentsForCaller("agentB", nullptr, callerRemote));
    service->callerConnectionCounts_[100] = 1;
    session->RemoveCaller(callerRemote, callerRemote, true);
    EXPECT_TRUE(session->callerConnections.empty());
}

/**
 * @tc.name  : StandardSessionLookup_001
 * @tc.number: StandardSessionLookup_001
 * @tc.desc  : Test standard key build and session lookup helpers
 */
HWTEST_F(AgentManagerServiceTest, StandardSessionLookup_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AAFwk::Want want;
    want.SetElementName("", "test.bundle", "TestAbility", "entry");
    auto key = service->BuildStandardAgentKey(100, "testAgent", want);

    EXPECT_EQ(key.callerUid, 100);
    EXPECT_EQ(key.agentId, "testAgent");
    EXPECT_EQ(key.bundleName, "test.bundle");
    EXPECT_EQ(key.moduleName, "entry");
    EXPECT_EQ(key.abilityName, "TestAbility");

    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    auto session = std::make_shared<AgentStandardRecord>();
    session->key = key;
    session->serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    service->standardSessions_.push_back(nullptr);
    service->standardSessions_.push_back(session);

    auto findKey = key;
    findKey.moduleName = "";
    EXPECT_EQ(service->FindStandardSessionLocked(findKey), session);
    EXPECT_EQ(service->FindStandardSessionByServiceRemoteLocked(session->serviceConnection->AsObject()),
        std::next(service->standardSessions_.begin()));
    sptr<MockAbilityConnection> unknownConnection = new MockAbilityConnection();
    EXPECT_EQ(service->FindStandardSessionByServiceRemoteLocked(unknownConnection->AsObject()),
        service->standardSessions_.end());
}

/**
 * @tc.name  : RegisterStandardSessionCallerLocked_001
 * @tc.number: RegisterStandardSessionCallerLocked_001
 * @tc.desc  : Test standard session caller registration validates input and records connected callbacks
 */
HWTEST_F(AgentManagerServiceTest, RegisterStandardSessionCallerLocked_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    StandardAgentKey key;
    key.callerUid = IPCSkeleton::GetCallingUid();
    key.agentId = "testAgent";
    key.bundleName = "test.bundle";
    key.abilityName = "TestAbility";

    EXPECT_EQ(service->RegisterStandardSessionCallerLocked(nullptr, connection, false), ERR_INVALID_VALUE);
    auto session = std::make_shared<AgentStandardRecord>();
    session->key = key;
    EXPECT_EQ(service->RegisterStandardSessionCallerLocked(session, connection, false), ERR_INVALID_VALUE);

    session->serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    session->state = StandardAgentState::CONNECTED;
    EXPECT_EQ(service->RegisterStandardSessionCallerLocked(session, connection, false), ERR_OK);
    EXPECT_EQ(service->RegisterStandardSessionCallerLocked(session, connection, false), ERR_OK);

    ASSERT_EQ(session->callerRemotes.size(), 1);
    ASSERT_EQ(session->connectedCallbacks.size(), 1);
    EXPECT_TRUE(session->pendingCallbacks.empty());
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    EXPECT_FALSE(service->trackedConnections_.begin()->second.countTowardsCallerLimit);
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
 * @tc.name  : RegisterStandardSessionCallerLocked_002
 * @tc.number: RegisterStandardSessionCallerLocked_002
 * @tc.desc  : Test standard session caller registration rejects a connection from another standard session
 */
HWTEST_F(AgentManagerServiceTest, RegisterStandardSessionCallerLocked_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    StandardAgentKey existingKey;
    existingKey.callerUid = 100;
    existingKey.agentId = "agentA";
    existingKey.bundleName = "test.bundle";
    existingKey.abilityName = "AbilityA";

    TrackedConnectionRecord record;
    record.callerUid = existingKey.callerUid;
    record.callerRemote = connection->AsObject();
    record.standardKey = existingKey;
    service->trackedConnections_[connection->AsObject()] = record;

    auto session = std::make_shared<AgentStandardRecord>();
    session->key = existingKey;
    session->key.agentId = "agentB";
    session->serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    EXPECT_EQ(service->RegisterStandardSessionCallerLocked(session, connection, false), ERR_INVALID_VALUE);
}

/**
 * @tc.name  : CreateAgentStandardRecord_001
 * @tc.number: CreateAgentStandardRecord_001
 * @tc.desc  : Test standard session creation rolls back tracking when AMS connect fails immediately
 */
HWTEST_F(AgentManagerServiceTest, CreateAgentStandardRecord_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    AAFwk::Want want;
    want.SetElementName("", "test.bundle", "TestAbility", "entry");
    auto key = service->BuildStandardAgentKey(IPCSkeleton::GetCallingUid(), "testAgent", want);
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    MyFlag::retConnectAbilityWithExtensionType = ERR_INVALID_VALUE;

    EXPECT_EQ(service->CreateAgentStandardRecord(want, "testAgent", key, connection), ERR_INVALID_VALUE);
    EXPECT_EQ(MyFlag::connectAbilityWithExtensionTypeCallCount, 1);
    EXPECT_TRUE(service->standardSessions_.empty());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
    MyFlag::retConnectAbilityWithExtensionType = ERR_OK;
}

/**
 * @tc.name  : HandleStandardAgentConnectDone_002
 * @tc.number: HandleStandardAgentConnectDone_002
 * @tc.desc  : Test standard connect done caches proxy and moves pending callbacks to connected callbacks
 */
HWTEST_F(AgentManagerServiceTest, HandleStandardAgentConnectDone_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    auto session = std::make_shared<AgentStandardRecord>();
    session->key.callerUid = IPCSkeleton::GetCallingUid();
    session->key.agentId = "testAgent";
    session->key.bundleName = "test.bundle";
    session->key.abilityName = "TestAbility";
    session->serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    session->pendingCallbacks.push_back(connection);
    service->standardSessions_.push_back(session);

    AppExecFwk::ElementName element("", "test.bundle", "TestAbility");
    sptr<MockAbilityConnection> proxy = new MockAbilityConnection();
    service->HandleStandardAgentConnectDone(session->serviceConnection->AsObject(), element, proxy->AsObject(),
        ERR_OK);

    EXPECT_EQ(connection->connectDoneCount, 1);
    EXPECT_EQ(connection->lastConnectResultCode, ERR_OK);
    EXPECT_EQ(session->state, StandardAgentState::CONNECTED);
    EXPECT_TRUE(session->pendingCallbacks.empty());
    ASSERT_EQ(session->connectedCallbacks.size(), 1);
    EXPECT_EQ(session->cachedRemoteObject, proxy->AsObject());
    ASSERT_EQ(service->standardSessions_.size(), 1);
}

/**
 * @tc.name  : HandleStandardAgentDisconnectDone_001
 * @tc.number: HandleStandardAgentDisconnectDone_001
 * @tc.desc  : Test standard disconnect done ignores unknown service remote
 */
HWTEST_F(AgentManagerServiceTest, HandleStandardAgentDisconnectDone_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    auto session = std::make_shared<AgentStandardRecord>();
    session->serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    session->connectedCallbacks.push_back(connection);
    service->standardSessions_.push_back(session);

    sptr<MockAbilityConnection> unknownConnection = new MockAbilityConnection();
    AppExecFwk::ElementName element("", "test.bundle", "TestAbility");
    service->HandleStandardAgentDisconnectDone(unknownConnection->AsObject(), element, ERR_OK);

    EXPECT_EQ(connection->disconnectDoneCount, 0);
    ASSERT_EQ(service->standardSessions_.size(), 1);
}

/**
 * @tc.name  : HandleStandardAgentDisconnectDone_002
 * @tc.number: HandleStandardAgentDisconnectDone_002
 * @tc.desc  : Test standard partial disconnect done notifies only the selected callback
 */
HWTEST_F(AgentManagerServiceTest, HandleStandardAgentDisconnectDone_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> pendingConnection = new MockAbilityConnection();
    sptr<MockAbilityConnection> connectedConnection = new MockAbilityConnection();
    auto session = std::make_shared<AgentStandardRecord>();
    session->serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connectedConnection);
    session->pendingCallbacks.push_back(pendingConnection);
    session->connectedCallbacks.push_back(connectedConnection);
    session->callerRemotes.push_back(pendingConnection->AsObject());
    session->callerRemotes.push_back(connectedConnection->AsObject());
    session->standardServiceConnections[session->serviceConnection->AsObject()] = session->serviceConnection;
    session->pendingDisconnectServiceRemotes.insert(session->serviceConnection->AsObject());
    service->standardSessions_.push_back(session);

    TrackedConnectionRecord pendingRecord;
    pendingRecord.callerUid = 100;
    pendingRecord.callerRemote = pendingConnection->AsObject();
    pendingRecord.serviceConnection = sptr<AgentServiceConnection>::MakeSptr(pendingConnection);
    pendingRecord.isDisconnecting = true;
    service->trackedConnections_[pendingConnection->AsObject()] = pendingRecord;
    TrackedConnectionRecord connectedRecord;
    connectedRecord.callerUid = 100;
    connectedRecord.callerRemote = connectedConnection->AsObject();
    connectedRecord.serviceConnection = session->serviceConnection;
    connectedRecord.isDisconnecting = true;
    service->trackedConnections_[connectedConnection->AsObject()] = connectedRecord;

    AppExecFwk::ElementName element("", "test.bundle", "TestAbility");
    service->HandleStandardAgentDisconnectDone(session->serviceConnection->AsObject(), element, ERR_OK);

    EXPECT_EQ(pendingConnection->disconnectDoneCount, 0);
    EXPECT_EQ(connectedConnection->disconnectDoneCount, 1);
    ASSERT_EQ(service->standardSessions_.size(), 1);
    ASSERT_EQ(service->trackedConnections_.size(), 1);
    EXPECT_TRUE(service->trackedConnections_.count(pendingConnection->AsObject()) > 0);
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
 * @tc.name  : PrepareLowCodeDisconnectLocked_001
 * @tc.number: PrepareLowCodeDisconnectLocked_001
 * @tc.desc  : Test low-code disconnect preparation handles missing host session and quota rollback
 */
HWTEST_F(AgentManagerServiceTest, PrepareLowCodeDisconnectLocked_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    AgentHostKey hostKey;
    hostKey.userId = 0;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    record.hostKey = hostKey;
    record.isLowCode = true;
    service->trackedConnections_[connection->AsObject()] = record;
    auto trackedIt = service->trackedConnections_.find(connection->AsObject());
    AgentDisconnectPlan plan;

    EXPECT_EQ(service->PrepareLowCodeDisconnectLocked(trackedIt, plan), ERR_INVALID_VALUE);

    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->hostConnection = sptr<AgentHostConnection>::MakeSptr(hostKey);
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });
    EXPECT_EQ(service->PrepareLowCodeDisconnectLocked(trackedIt, plan), ERR_INVALID_VALUE);
    EXPECT_FALSE(session->isDisconnecting);
    EXPECT_FALSE(trackedIt->second.isDisconnecting);
}

/**
 * @tc.name  : PrepareStandardDisconnectLocked_001
 * @tc.number: PrepareStandardDisconnectLocked_001
 * @tc.desc  : Test standard disconnect preparation falls back when no standard key exists
 */
HWTEST_F(AgentManagerServiceTest, PrepareStandardDisconnectLocked_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    auto serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    record.serviceConnection = serviceConnection;
    service->trackedConnections_[connection->AsObject()] = record;
    service->callerConnectionCounts_[record.callerUid] = 1;
    auto trackedIt = service->trackedConnections_.find(connection->AsObject());
    AgentDisconnectPlan plan;
    plan.callerRemote = connection->AsObject();

    EXPECT_EQ(service->PrepareStandardDisconnectLocked(trackedIt, plan), ERR_OK);

    EXPECT_TRUE(trackedIt->second.isDisconnecting);
    EXPECT_EQ(plan.serviceConnection.GetRefPtr(), serviceConnection.GetRefPtr());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
 * @tc.name  : PrepareStandardDisconnectLocked_002
 * @tc.number: PrepareStandardDisconnectLocked_002
 * @tc.desc  : Test standard disconnect preparation reports quota release failure
 */
HWTEST_F(AgentManagerServiceTest, PrepareStandardDisconnectLocked_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    record.serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    service->trackedConnections_[connection->AsObject()] = record;
    auto trackedIt = service->trackedConnections_.find(connection->AsObject());
    AgentDisconnectPlan plan;
    plan.callerRemote = connection->AsObject();

    EXPECT_EQ(service->PrepareStandardDisconnectLocked(trackedIt, plan), ERR_INVALID_VALUE);

    EXPECT_TRUE(trackedIt->second.isDisconnecting);
    EXPECT_EQ(plan.serviceConnection, nullptr);
}

/**
 * @tc.name  : PrepareStandardDisconnectLocked_003
 * @tc.number: PrepareStandardDisconnectLocked_003
 * @tc.desc  : Test standard disconnect preparation falls back when no matching standard session exists
 */
HWTEST_F(AgentManagerServiceTest, PrepareStandardDisconnectLocked_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    auto serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    auto key = BuildTestStandardKey();
    TrackedConnectionRecord record;
    record.callerUid = key.callerUid;
    record.callerRemote = connection->AsObject();
    record.serviceConnection = serviceConnection;
    record.standardKey = key;
    service->trackedConnections_[connection->AsObject()] = record;
    service->callerConnectionCounts_[key.callerUid] = 1;
    service->standardSessions_.push_back(nullptr);
    auto nonMatchingSession = std::make_shared<AgentStandardRecord>();
    nonMatchingSession->key = key;
    nonMatchingSession->key.agentId = "anotherAgent";
    service->standardSessions_.push_back(nonMatchingSession);
    auto trackedIt = service->trackedConnections_.find(connection->AsObject());
    AgentDisconnectPlan plan;
    plan.callerRemote = connection->AsObject();

    EXPECT_EQ(service->PrepareStandardDisconnectLocked(trackedIt, plan), ERR_OK);

    EXPECT_TRUE(trackedIt->second.isDisconnecting);
    EXPECT_EQ(plan.serviceConnection.GetRefPtr(), serviceConnection.GetRefPtr());
    EXPECT_FALSE(plan.isStandard);
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
 * @tc.name  : PrepareStandardDisconnectLocked_004
 * @tc.number: PrepareStandardDisconnectLocked_004
 * @tc.desc  : Test standard disconnect preparation no-ops when session is already disconnecting
 */
HWTEST_F(AgentManagerServiceTest, PrepareStandardDisconnectLocked_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    auto serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    auto key = BuildTestStandardKey();
    AddTestStandardSession(service, key, serviceConnection, { connection->AsObject() }, true);
    AddTestStandardTrackedConnection(service, key, connection, serviceConnection, true);
    service->callerConnectionCounts_[key.callerUid] = 1;
    auto trackedIt = service->trackedConnections_.find(connection->AsObject());
    AgentDisconnectPlan plan;
    plan.callerRemote = connection->AsObject();

    EXPECT_EQ(service->PrepareStandardDisconnectLocked(trackedIt, plan), ERR_OK);

    EXPECT_FALSE(trackedIt->second.isDisconnecting);
    EXPECT_EQ(plan.serviceConnection, nullptr);
    EXPECT_FALSE(plan.isStandard);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_[key.callerUid], 1);
}

/**
 * @tc.name  : CleanupDeadStandardConnectionLocked_001
 * @tc.number: CleanupDeadStandardConnectionLocked_001
 * @tc.desc  : Test dead standard cleanup releases a record without standard session metadata
 */
HWTEST_F(AgentManagerServiceTest, CleanupDeadStandardConnectionLocked_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    auto serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    record.serviceConnection = serviceConnection;
    service->trackedConnections_[connection->AsObject()] = record;
    service->callerConnectionCounts_[record.callerUid] = 1;
    auto trackedIt = service->trackedConnections_.find(connection->AsObject());
    sptr<AAFwk::IAbilityConnection> disconnectedConnection = nullptr;

    service->CleanupDeadStandardConnectionLocked(trackedIt, connection->AsObject(), disconnectedConnection);

    EXPECT_EQ(disconnectedConnection.GetRefPtr(), serviceConnection.GetRefPtr());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

/**
 * @tc.name  : CleanupDeadStandardConnectionLocked_002
 * @tc.number: CleanupDeadStandardConnectionLocked_002
 * @tc.desc  : Test dead standard cleanup falls back when no matching standard session exists
 */
HWTEST_F(AgentManagerServiceTest, CleanupDeadStandardConnectionLocked_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    auto serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    auto key = BuildTestStandardKey();
    AddTestStandardTrackedConnection(service, key, connection, serviceConnection, true);
    service->callerConnectionCounts_[key.callerUid] = 1;
    service->standardSessions_.push_back(nullptr);
    auto nonMatchingSession = std::make_shared<AgentStandardRecord>();
    nonMatchingSession->key = key;
    nonMatchingSession->key.abilityName = "AnotherAbility";
    service->standardSessions_.push_back(nonMatchingSession);
    auto trackedIt = service->trackedConnections_.find(connection->AsObject());
    sptr<AAFwk::IAbilityConnection> disconnectedConnection = nullptr;

    service->CleanupDeadStandardConnectionLocked(trackedIt, connection->AsObject(), disconnectedConnection);

    EXPECT_EQ(disconnectedConnection.GetRefPtr(), serviceConnection.GetRefPtr());
    EXPECT_TRUE(service->trackedConnections_.empty());
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
    ASSERT_EQ(service->standardSessions_.size(), 2);
}

/**
 * @tc.name  : RollbackAgentDisconnectLocked_001
 * @tc.number: RollbackAgentDisconnectLocked_001
 * @tc.desc  : Test disconnect rollback restores low-code and normal tracked state
 */
HWTEST_F(AgentManagerServiceTest, RollbackAgentDisconnectLocked_001, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    AgentHostKey hostKey;
    hostKey.userId = 0;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";

    auto session = std::make_shared<AgentHostSession>();
    session->key = hostKey;
    session->isDisconnecting = true;
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, session });

    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    record.isDisconnecting = true;
    service->trackedConnections_[connection->AsObject()] = record;

    AgentDisconnectPlan plan;
    plan.callerRemote = connection->AsObject();
    plan.hostKey = hostKey;
    plan.hasHostKey = true;
    service->RollbackAgentDisconnectLocked(plan);

    EXPECT_FALSE(session->isDisconnecting);
    EXPECT_FALSE(service->trackedConnections_[connection->AsObject()].isDisconnecting);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_[100], 1);
}

/**
 * @tc.name  : RollbackAgentDisconnectLocked_002
 * @tc.number: RollbackAgentDisconnectLocked_002
 * @tc.desc  : Test standard rollback handles missing session, missing tracked record, and zero caller uid
 */
HWTEST_F(AgentManagerServiceTest, RollbackAgentDisconnectLocked_002, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    service->standardSessions_.push_back(nullptr);
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    AgentDisconnectPlan plan;
    plan.callerRemote = connection->AsObject();
    plan.isStandard = true;
    plan.standardKey = BuildTestStandardKey();
    plan.standardCallerUid = 0;

    service->RollbackAgentDisconnectLocked(plan);

    EXPECT_TRUE(service->callerConnectionCounts_.empty());
    ASSERT_EQ(service->standardSessions_.size(), 1);
}

/**
 * @tc.name  : RollbackAgentDisconnectLocked_003
 * @tc.number: RollbackAgentDisconnectLocked_003
 * @tc.desc  : Test standard rollback restores session and tracked state without normal quota increment
 */
HWTEST_F(AgentManagerServiceTest, RollbackAgentDisconnectLocked_003, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    auto serviceConnection = sptr<AgentServiceConnection>::MakeSptr(connection);
    auto key = BuildTestStandardKey();
    auto session = AddTestStandardSession(service, key, serviceConnection, { connection->AsObject() }, true);
    AddTestStandardTrackedConnection(service, key, connection, serviceConnection, true);
    service->trackedConnections_[connection->AsObject()].isDisconnecting = true;

    AgentDisconnectPlan plan;
    plan.callerRemote = connection->AsObject();
    plan.isStandard = true;
    plan.standardKey = key;
    plan.standardCallerUid = key.callerUid;

    service->RollbackAgentDisconnectLocked(plan);

    EXPECT_FALSE(session->isDisconnecting);
    EXPECT_FALSE(service->trackedConnections_[connection->AsObject()].isDisconnecting);
    ASSERT_EQ(service->callerConnectionCounts_.size(), 1);
    EXPECT_EQ(service->callerConnectionCounts_[key.callerUid], 1);
}

/**
 * @tc.name  : RollbackAgentDisconnectLocked_004
 * @tc.number: RollbackAgentDisconnectLocked_004
 * @tc.desc  : Test rollback tolerates null low-code host session and non-disconnecting tracked record
 */
HWTEST_F(AgentManagerServiceTest, RollbackAgentDisconnectLocked_004, TestSize.Level1)
{
    auto service = AgentManagerService::GetInstance();
    sptr<MockAbilityConnection> connection = new MockAbilityConnection();
    AgentHostKey hostKey;
    hostKey.userId = 0;
    hostKey.bundleName = "lowcode.bundle";
    hostKey.moduleName = "entry";
    hostKey.abilityName = "LowCodeExtAbility";
    service->agentHostSessions_.push_back(AgentHostSessionRecord { hostKey, nullptr });
    TrackedConnectionRecord record;
    record.callerUid = 100;
    record.callerRemote = connection->AsObject();
    record.isDisconnecting = false;
    service->trackedConnections_[connection->AsObject()] = record;

    AgentDisconnectPlan plan;
    plan.callerRemote = connection->AsObject();
    plan.hostKey = hostKey;
    plan.hasHostKey = true;

    service->RollbackAgentDisconnectLocked(plan);

    EXPECT_FALSE(service->trackedConnections_[connection->AsObject()].isDisconnecting);
    EXPECT_TRUE(service->callerConnectionCounts_.empty());
}

} // namespace AgentRuntime
} // namespace OHOS
