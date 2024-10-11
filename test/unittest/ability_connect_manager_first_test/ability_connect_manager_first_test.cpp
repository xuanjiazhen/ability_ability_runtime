/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include <memory>

#define private public
#define protected public
#include "ability_connect_manager.h"
#undef private
#undef protected

#include "ability_config.h"
#include "ability_manager_errors.h"
#include "ability_scheduler.h"
#include "ability_util.h"
#include "bundlemgr/mock_bundle_manager.h"
#include "hilog_tag_wrapper.h"
#include "mock_ability_connect_callback.h"
#include "mock_sa_call.h"
#include "mock_task_handler_wrap.h"
#include "sa_mgr_client.h"
#include "system_ability_definition.h"
#include "session/host/include/session.h"
#include <thread>
#include <chrono>

using namespace testing::ext;
using namespace OHOS::AppExecFwk;
using testing::_;
using testing::Invoke;
using testing::Return;
using testing::SetArgReferee;
using ::testing::DoAll;

namespace {
    const int32_t SLEEP_TIME = 10000;
}

namespace OHOS {
namespace AAFwk {
template<typename F>
static void WaitUntilTaskCalled(const F& f, const std::shared_ptr<TaskHandlerWrap>& handler,
    std::atomic<bool>& taskCalled)
{
    const uint32_t maxRetryCount = 1000;
    const uint32_t sleepTime = 1000;
    uint32_t count = 0;
    if (handler->SubmitTask(f)) {
        while (!taskCalled.load()) {
            ++count;
            // if delay more than 1 second, break
            if (count >= maxRetryCount) {
                break;
            }
            usleep(sleepTime);
        }
    }
}

static void WaitUntilTaskDone(const std::shared_ptr<TaskHandlerWrap>& handler)
{
    std::atomic<bool> taskCalled(false);
    auto f = [&taskCalled]() { taskCalled.store(true); };
    WaitUntilTaskCalled(f, handler, taskCalled);
}

class AbilityConnectManagerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    AbilityConnectManager* ConnectManager() const;
    std::shared_ptr<MockTaskHandlerWrap> TaskHandler() const;
    std::shared_ptr<EventHandlerWrap> EventHandler() const;

    AbilityRequest GenerateAbilityRequest(const std::string& deviceName, const std::string& abilityName,
        const std::string& appName, const std::string& bundleName, const std::string& moduleName);

    static constexpr int TEST_WAIT_TIME = 1000000;

    sptr<SessionInfo> MockSessionInfo(int32_t persistentId);
    std::shared_ptr<AbilityRecord> InitAbilityRecord();
    std::shared_ptr<MockTaskHandlerWrap> taskHandler_;
    //inline static MockAppMgrClient* mockAppMgrClient_ = nullptr;
protected:
    AbilityRequest abilityRequest_{};
    AbilityRequest abilityRequest1_{};
    AbilityRequest abilityRequest2_{};
    std::shared_ptr<AbilityRecord> serviceRecord_{ nullptr };
    std::shared_ptr<AbilityRecord> serviceRecord1_{ nullptr };
    std::shared_ptr<AbilityRecord> serviceRecord2_{ nullptr };
    OHOS::sptr<Token> serviceToken_{ nullptr };
    OHOS::sptr<Token> serviceToken1_{ nullptr };
    OHOS::sptr<Token> serviceToken2_{ nullptr };
    OHOS::sptr<IAbilityConnection> callbackA_{ nullptr };
    OHOS::sptr<IAbilityConnection> callbackB_{ nullptr };

private:
    std::shared_ptr<AbilityConnectManager> connectManager_;
    std::shared_ptr<EventHandlerWrap> eventHandler_;
};

AbilityRequest AbilityConnectManagerTest::GenerateAbilityRequest(const std::string& deviceName,
    const std::string& abilityName, const std::string& appName, const std::string& bundleName,
    const std::string& moduleName)
{
    ElementName element(deviceName, bundleName, abilityName, moduleName);
    Want want;
    want.SetElement(element);

    AbilityInfo abilityInfo;
    abilityInfo.visible = true;
    abilityInfo.applicationName = appName;
    abilityInfo.type = AbilityType::SERVICE;
    abilityInfo.name = abilityName;
    abilityInfo.bundleName = bundleName;
    abilityInfo.moduleName = moduleName;
    abilityInfo.deviceId = deviceName;
    ApplicationInfo appinfo;
    appinfo.name = appName;
    abilityInfo.applicationInfo = appinfo;
    AbilityRequest abilityRequest;
    abilityRequest.want = want;
    abilityRequest.abilityInfo = abilityInfo;
    abilityRequest.appInfo = appinfo;
    abilityInfo.process = bundleName;

    return abilityRequest;
}

sptr<SessionInfo> AbilityConnectManagerTest::MockSessionInfo(int32_t persistentId)
{
    sptr<SessionInfo> sessionInfo = new (std::nothrow) SessionInfo();
    if (!sessionInfo) {
        TAG_LOGE(AAFwkTag::TEST, "sessionInfo is nullptr");
        return nullptr;
    }
    sessionInfo->persistentId = persistentId;
    return sessionInfo;
}

std::shared_ptr<AbilityRecord> AbilityConnectManagerTest::InitAbilityRecord()
{
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.example.unittest";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    return abilityRecord;
}

void AbilityConnectManagerTest::SetUpTestCase(void)
{}
void AbilityConnectManagerTest::TearDownTestCase(void)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME));
}

void AbilityConnectManagerTest::SetUp(void)
{
    connectManager_ = std::make_unique<AbilityConnectManager>(0);
    taskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("AbilityConnectManagerTest");
    eventHandler_ = std::make_shared<EventHandlerWrap>(taskHandler_);
    // generate ability request
    std::string deviceName = "device";
    std::string abilityName = "ServiceAbility";
    std::string appName = "hiservcie";
    std::string bundleName = "com.ix.hiservcie";
    std::string moduleName = "entry";
    abilityRequest_ = GenerateAbilityRequest(deviceName, abilityName, appName, bundleName, moduleName);
    serviceRecord_ = AbilityRecord::CreateAbilityRecord(abilityRequest_);
    serviceToken_ = serviceRecord_->GetToken();
    std::string deviceName1 = "device";
    std::string abilityName1 = "musicServiceAbility";
    std::string appName1 = "musicservcie";
    std::string bundleName1 = "com.ix.musicservcie";
    std::string moduleName1 = "entry";
    abilityRequest1_ = GenerateAbilityRequest(deviceName1, abilityName1, appName1, bundleName1, moduleName1);
    serviceRecord1_ = AbilityRecord::CreateAbilityRecord(abilityRequest1_);
    std::string deviceName2 = "device";
    std::string abilityName2 = "residentServiceAbility";
    std::string appName2 = "residentservcie";
    std::string bundleName2 = "com.ix.residentservcie";
    std::string moduleName2 = "entry";
    abilityRequest2_ = GenerateAbilityRequest(deviceName2, abilityName2, appName2, bundleName2, moduleName2);
    serviceRecord2_ = AbilityRecord::CreateAbilityRecord(abilityRequest2_);
    serviceToken2_ = serviceRecord_->GetToken();
    serviceToken1_ = serviceRecord_->GetToken();
    callbackA_ = new AbilityConnectCallback();
    callbackB_ = new AbilityConnectCallback();
    // mock bms
    OHOS::DelayedSingleton<SaMgrClient>::GetInstance()->RegisterSystemAbility(
        OHOS::BUNDLE_MGR_SERVICE_SYS_ABILITY_ID, new BundleMgrService());
}

void AbilityConnectManagerTest::TearDown(void)
{
    // reset the callback count
    AbilityConnectCallback::onAbilityConnectDoneCount = 0;
    AbilityConnectCallback::onAbilityDisconnectDoneCount = 0;
    serviceRecord_ = nullptr;
}

AbilityConnectManager* AbilityConnectManagerTest::ConnectManager() const
{
    return connectManager_.get();
}

std::shared_ptr<MockTaskHandlerWrap> AbilityConnectManagerTest::TaskHandler() const
{
    return taskHandler_;
}

std::shared_ptr<EventHandlerWrap> AbilityConnectManagerTest::EventHandler() const
{
    return eventHandler_;
}

/*
 * Feature: AbilityConnectManager
 * Function: GetUIExtensionBySessionInfo
 */
HWTEST_F(AbilityConnectManagerTest, GetUIExtensionBySessionInfo_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionMap_.clear();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    auto service = connectManager->GetUIExtensionBySessionInfo(sessionInfo);
    std::string device = "device";
    std::string abilityName1 = "uiExtensionAbility1";
    std::string appName1 = "uiExtensionProvider1";
    std::string bundleName1 = "com.ix.uiExtensionProvider1";
    std::string moduleName1 = "entry";
    auto request1 = GenerateAbilityRequest(device, abilityName1, appName1, bundleName1, moduleName1);
    auto uiExtension1 = AbilityRecord::CreateAbilityRecord(request1);
    EXPECT_NE(uiExtension1, nullptr);
    uiExtension1->abilityInfo_.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    uiExtension1 = nullptr;
    connectManager->uiExtensionMap_.emplace(
        sessionInfo->sessionToken, AbilityConnectManager::UIExtWindowMapValType(uiExtension1, sessionInfo));
    service = connectManager->GetUIExtensionBySessionInfo(sessionInfo);
    EXPECT_EQ(service, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: GetUIExtensionBySessionInfo
 */
HWTEST_F(AbilityConnectManagerTest, GetUIExtensionBySessionInfo_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_002 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionMap_.clear();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->callerToken = new Rosen::Session(info);
    std::string device = "device";
    std::string abilityName1 = "uiExtensionAbility1";
    std::string appName1 = "uiExtensionProvider1";
    std::string bundleName1 = "com.ix.uiExtensionProvider1";
    std::string moduleName1 = "entry";
    auto request1 = GenerateAbilityRequest(device, abilityName1, appName1, bundleName1, moduleName1);
    auto uiExtension1 = AbilityRecord::CreateAbilityRecord(request1);
    EXPECT_NE(uiExtension1, nullptr);
    uiExtension1->abilityInfo_.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    Rosen::SessionInfo infos;
    sptr<SessionInfo> sessionInfo1(new SessionInfo());
    sessionInfo1 = nullptr;
    connectManager->uiExtensionMap_.emplace(
        sessionInfo->sessionToken, AbilityConnectManager::UIExtWindowMapValType(uiExtension1, sessionInfo1));
    auto service = connectManager->GetUIExtensionBySessionInfo(sessionInfo);
    EXPECT_EQ(service, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: GetUIExtensionBySessionInfo
 */
HWTEST_F(AbilityConnectManagerTest, GetUIExtensionBySessionInfo_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_003 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionMap_.clear();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    sessionInfo->callerToken = new Rosen::Session(info);
    std::string device = "device";
    std::string abilityName1 = "uiExtensionAbility1";
    std::string appName1 = "uiExtensionProvider1";
    std::string bundleName1 = "com.ix.uiExtensionProvider1";
    std::string moduleName1 = "entry";
    auto request1 = GenerateAbilityRequest(device, abilityName1, appName1, bundleName1, moduleName1);
    auto uiExtension1 = AbilityRecord::CreateAbilityRecord(request1);
    EXPECT_NE(uiExtension1, nullptr);
    uiExtension1->abilityInfo_.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;

    Rosen::SessionInfo infos;
    sptr<SessionInfo> sessionInfo1(new SessionInfo());
    sessionInfo1->sessionToken = new Rosen::Session(info);
    sessionInfo1->callerToken = uiExtension1->GetToken();
    connectManager->uiExtensionMap_.emplace(
        sessionInfo->sessionToken, AbilityConnectManager::UIExtWindowMapValType(uiExtension1, sessionInfo1));
    auto service = connectManager->GetUIExtensionBySessionInfo(sessionInfo);
    EXPECT_EQ(service, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_003 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: GetUIExtensionBySessionInfo
 */
HWTEST_F(AbilityConnectManagerTest, GetUIExtensionBySessionInfo_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_004 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);
    connectManager->uiExtensionMap_.clear();
    Rosen::SessionInfo info;
    sptr<SessionInfo> sessionInfo(new SessionInfo());
    sessionInfo->sessionToken = new Rosen::Session(info);
    std::string device = "device";
    std::string abilityName1 = "uiExtensionAbility1";
    std::string appName1 = "uiExtensionProvider1";
    std::string bundleName1 = "com.ix.uiExtensionProvider1";
    std::string moduleName1 = "entry";
    auto request1 = GenerateAbilityRequest(device, abilityName1, appName1, bundleName1, moduleName1);
    auto uiExtension1 = AbilityRecord::CreateAbilityRecord(request1);
    EXPECT_NE(uiExtension1, nullptr);
    uiExtension1->abilityInfo_.extensionAbilityType = ExtensionAbilityType::SYS_COMMON_UI;
    Rosen::SessionInfo infos;
    sptr<SessionInfo> sessionInfo1(new SessionInfo());
    sessionInfo1->sessionToken = uiExtension1->GetToken();
    sessionInfo1->callerToken = new Rosen::Session(info);
    connectManager->uiExtensionMap_.emplace(
        sessionInfo->sessionToken, AbilityConnectManager::UIExtWindowMapValType(uiExtension1, sessionInfo1));
    auto service = connectManager->GetUIExtensionBySessionInfo(sessionInfo);
    EXPECT_EQ(service, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "GetUIExtensionBySessionInfo_004 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleActiveAbility
 */
HWTEST_F(AbilityConnectManagerTest, HandleActiveAbility_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = serviceRecord_;
    AbilityRequest abilityRequest;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    OHOS::sptr<IAbilityConnection> callback1 = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connectRecord =
        std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callback1);
    std::shared_ptr<AbilityRecord> targetService = nullptr;
    connectManager->HandleActiveAbility(targetService, connectRecord);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI_SERVICE;
    connectManager->HandleActiveAbility(abilityRecord, connectRecord);
    connectRecord = nullptr;
    connectManager->HandleActiveAbility(abilityRecord, connectRecord);
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleActiveAbility
 */
HWTEST_F(AbilityConnectManagerTest, HandleActiveAbility_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_002 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);
    auto result = connectManager->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);
    auto connectMap = connectManager->connectMap_;
    auto connectRecordList = connectMap.at(callbackA_->AsObject());
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));
    auto elementName = abilityRequest_.want.GetElement();
    auto elementNameUri = elementName.GetURI();
    auto serviceMap = connectManager->GetServiceMap();
    std::shared_ptr<AbilityRecord> abilityRecord = serviceMap.at(elementNameUri);
    //AbilityRequest abilityRequest;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    OHOS::sptr<IAbilityConnection> callback1 = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connectRecord =
        std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callback1);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
    connectManager->HandleActiveAbility(abilityRecord, connectRecord);
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_002 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: HandleActiveAbility
 */
HWTEST_F(AbilityConnectManagerTest, HandleActiveAbility_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_003 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);
    auto result = connectManager->ConnectAbilityLocked(abilityRequest_, callbackA_, nullptr);
    EXPECT_EQ(0, result);

    result = connectManager->ConnectAbilityLocked(abilityRequest_, callbackB_, nullptr);
    EXPECT_EQ(0, result);

    auto connectMap = connectManager->connectMap_;
    auto connectRecordList = connectMap.at(callbackA_->AsObject());
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));

    connectRecordList = connectMap.at(callbackB_->AsObject());
    EXPECT_EQ(1, static_cast<int>(connectRecordList.size()));

    auto elementName = abilityRequest_.want.GetElement();
    std::string elementNameUri = elementName.GetURI();
    auto serviceMap = connectManager->GetServiceMap();
    auto abilityRecord = serviceMap.at(elementNameUri);
    connectRecordList = abilityRecord->GetConnectRecordList();
    EXPECT_EQ(2, static_cast<int>(connectRecordList.size()));
    //AbilityRequest abilityRequest;
    sptr<IRemoteObject> callerToken = abilityRecord->GetToken();
    OHOS::sptr<IAbilityConnection> callback1 = new AbilityConnectCallback();
    std::shared_ptr<ConnectionRecord> connectRecord =
        std::make_shared<ConnectionRecord>(callerToken, abilityRecord, callback1);
    abilityRecord->abilityInfo_.extensionAbilityType = AppExecFwk::ExtensionAbilityType::UI;
    connectManager->HandleActiveAbility(abilityRecord, connectRecord);
    connectManager->taskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("HandleActiveAbility003");
    abilityRecord->connRemoteObject_ = abilityRecord->GetToken();
    connectManager->HandleActiveAbility(abilityRecord, connectRecord);
    connectManager->taskHandler_ = nullptr;
    abilityRecord->connRemoteObject_ = nullptr;
    connectManager->HandleActiveAbility(abilityRecord, connectRecord);
    connectManager->taskHandler_ = nullptr;
    abilityRecord->connRemoteObject_ = abilityRecord->GetToken();
    connectManager->HandleActiveAbility(abilityRecord, connectRecord);
    connectManager->taskHandler_ = MockTaskHandlerWrap::CreateQueueHandler("HandleActiveAbility003");
    abilityRecord->connRemoteObject_ = nullptr;
    connectManager->HandleActiveAbility(abilityRecord, connectRecord);
    TAG_LOGI(AAFwkTag::TEST, "HandleActiveAbility_003 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: OnTimeOut
 */
HWTEST_F(AbilityConnectManagerTest, OnTimeOut_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnTimeOut_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);
    uint32_t msgId = 2;
    bool isHalf = true;
    int64_t abilityRecordIds = 1;
    std::shared_ptr<AbilityRecord> abilityRecord = nullptr;
    connectManager->OnTimeOut(msgId, abilityRecordIds, isHalf);
    std::shared_ptr<AbilityRecord> abilityRecords = serviceRecord_;
    connectManager->serviceMap_.emplace("first", abilityRecords);
    connectManager->serviceMap_.emplace("second", nullptr);
    int64_t abilityRecordId = abilityRecords->GetRecordId();
    connectManager->OnTimeOut(msgId, abilityRecordId, isHalf);
    isHalf = false;
    msgId = 0;
    connectManager->OnTimeOut(msgId, abilityRecordId, isHalf);
    msgId = 2;
    connectManager->OnTimeOut(msgId, abilityRecordId, isHalf);
    msgId = 5;
    connectManager->OnTimeOut(msgId, abilityRecordId, isHalf);
    msgId = 6;
    connectManager->OnTimeOut(msgId, abilityRecordId, isHalf);

    TAG_LOGI(AAFwkTag::TEST, "OnTimeOut_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: PrintTimeOutLog
 */
HWTEST_F(AbilityConnectManagerTest, PrintTimeOutLog_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "PrintTimeOutLog_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = serviceRecord_;
    bool isHalf = true;
    uint32_t msgId = 0;
    AppExecFwk::RunningProcessInfo processInfo = {};
    processInfo.pid_ = 1;
    /*EXPECT_CALL(*mockAppMgrClient_, GetRunningProcessInfoByToken(_, _))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<1>(processInfo)));*/
    connectManager->PrintTimeOutLog(abilityRecord, msgId, isHalf);
    TAG_LOGI(AAFwkTag::TEST, "PrintTimeOutLog_001 end");
}

/*
 * Feature: AbilityConnectManager
 * Function: KeepAbilityAlive
 */
HWTEST_F(AbilityConnectManagerTest, KeepAbilityAlive_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "KeepAbilityAlive_001 start");
    std::shared_ptr<AbilityConnectManager> connectManager = std::make_shared<AbilityConnectManager>(0);
    EXPECT_NE(connectManager, nullptr);
    std::shared_ptr<AbilityRecord> abilityRecord = serviceRecord_;
    EXPECT_NE(abilityRecord, nullptr);
    sptr<IRemoteObject> token = abilityRecord->GetToken();
    EXPECT_NE(token, nullptr);
    int32_t currentUserId = 0;
    connectManager->userId_ = 1;
    connectManager->KeepAbilityAlive(abilityRecord, currentUserId);
    EXPECT_NE(connectManager, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "KeepAbilityAlive_001 end");
}


}  // namespace AAFwk
}  // namespace OHOS
