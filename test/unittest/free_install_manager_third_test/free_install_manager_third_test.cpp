/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ability_manager_service.h"
#include "ability_record.h"
#include "bundle_mgr_helper.h"
#include "distributed_client.h"
#include "hilog_tag_wrapper.h"
#include "insight_intent_execute_manager.h"
#include "mock_ability_token.h"
#include "mock_app_mgr_service.h"
#include "mock_bundle_manager_service.h"
#include "sa_mgr_client.h"
#include "task_handler_wrap.h"
#include "utils/app_mgr_util.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AAFwk {
int32_t DistributedClient::StartRemoteFreeInstall(const Want &, int32_t, int32_t, uint32_t,
    const sptr<IRemoteObject> &)
{
    return ERR_OK;
}

TaskHandle TaskHandlerWrap::SubmitTask(const std::function<void()> &task, const std::string &)
{
    if (task) {
        task();
    }
    return TaskHandle();
}

InsightIntentExecuteManager::InsightIntentExecuteManager() = default;

InsightIntentExecuteManager::~InsightIntentExecuteManager() = default;

int32_t InsightIntentExecuteManager::ExecuteIntentDone(uint64_t, int32_t,
    const AppExecFwk::InsightIntentExecuteResult &, int32_t, uint32_t)
{
    return ERR_OK;
}
}  // namespace AAFwk
namespace AppExecFwk {
namespace {
const int BUNDLE_MGR_SERVICE_SYS_ABILITY_ID = 401;
constexpr const char* LOCAL_BUNDLE_NAME = "com.test.demo";
constexpr const char* REMOTE_BUNDLE_NAME = "com.remote.demo";
constexpr const char* AGENT_ABILITY_NAME = "AgentAbility";
constexpr int32_t TEST_USER_ID = 100;

class FreeInstallAgentBundleMgrService : public OHOS::MockBundleManagerService {
public:
    explicit FreeInstallAgentBundleMgrService(const std::weak_ptr<FreeInstallManager> &freeInstallManager)
        : freeInstallManager_(freeInstallManager)
    {}

    ErrCode GetNameForUid(const int, std::string &bundleName) override
    {
        bundleName = LOCAL_BUNDLE_NAME;
        return ERR_OK;
    }

    bool QueryAbilityInfo(const Want &, int32_t, int32_t, AbilityInfo &) override
    {
        return queryAbilityResult_;
    }

    bool QueryExtensionAbilityInfos(const Want &, const int32_t &, const int32_t &,
        std::vector<ExtensionAbilityInfo> &extensionInfos) override
    {
        extensionInfos = extensionInfos_;
        return queryExtensionResult_;
    }

    bool QueryAbilityInfo(const Want &want, int32_t, int32_t, AbilityInfo &, const sptr<IRemoteObject> &) override
    {
        queryAbilityInfoWithCallbackCount_++;
        auto freeInstallManager = freeInstallManager_.lock();
        if (freeInstallManager == nullptr) {
            return false;
        }

        FreeInstallInfo taskInfo;
        if (freeInstallManager->GetFreeInstallTaskInfo(want.GetElement().GetBundleName(),
            want.GetElement().GetAbilityName(), want.GetStringParam(Want::PARAM_RESV_START_TIME), taskInfo)) {
            freeInstallManager->NotifyFreeInstallResult(-1, taskInfo.want, ERR_OK, false);
        } else {
            freeInstallManager->NotifyFreeInstallResult(-1, want, ERR_OK, false);
        }
        return false;
    }

    void SetQueryAbilityResult(bool queryAbilityResult)
    {
        queryAbilityResult_ = queryAbilityResult;
    }

    void SetQueryExtensionResult(bool queryExtensionResult, const std::vector<ExtensionAbilityInfo> &extensionInfos = {})
    {
        queryExtensionResult_ = queryExtensionResult;
        extensionInfos_ = extensionInfos;
    }

    int32_t GetQueryAbilityInfoWithCallbackCount() const
    {
        return queryAbilityInfoWithCallbackCount_;
    }

private:
    std::weak_ptr<FreeInstallManager> freeInstallManager_;
    bool queryAbilityResult_ = false;
    bool queryExtensionResult_ = false;
    std::vector<ExtensionAbilityInfo> extensionInfos_;
    int32_t queryAbilityInfoWithCallbackCount_ = 0;
};

class FreeInstallAgentAppMgrService : public MockAppMgrService {
public:
    bool GetAppRunningStateByBundleName(const std::string &) override
    {
        return false;
    }
};

class FreeInstallAgentMockScope {
public:
    explicit FreeInstallAgentMockScope(const std::shared_ptr<FreeInstallManager> &freeInstallManager)
    {
        bundleMgrHelper_ = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
        if (bundleMgrHelper_ != nullptr) {
            bundleMgrBackup_ = bundleMgrHelper_->bundleMgr_;
            mockBundleMgr_ = new (std::nothrow) FreeInstallAgentBundleMgrService(freeInstallManager);
            bundleMgrHelper_->bundleMgr_ = mockBundleMgr_;
        }

        appMgrBackup_ = AppMgrUtil::appMgr_;
        mockAppMgr_ = new (std::nothrow) FreeInstallAgentAppMgrService();
        AppMgrUtil::appMgr_ = mockAppMgr_;
    }

    ~FreeInstallAgentMockScope()
    {
        if (bundleMgrHelper_ != nullptr) {
            bundleMgrHelper_->bundleMgr_ = bundleMgrBackup_;
        }
        AppMgrUtil::appMgr_ = appMgrBackup_;
    }

    sptr<FreeInstallAgentBundleMgrService> GetBundleMgr() const
    {
        return mockBundleMgr_;
    }

    sptr<FreeInstallAgentAppMgrService> GetAppMgr() const
    {
        return mockAppMgr_;
    }

private:
    std::shared_ptr<AppExecFwk::BundleMgrHelper> bundleMgrHelper_;
    sptr<IBundleMgr> bundleMgrBackup_;
    sptr<IAppMgr> appMgrBackup_;
    sptr<FreeInstallAgentBundleMgrService> mockBundleMgr_;
    sptr<FreeInstallAgentAppMgrService> mockAppMgr_;
};
}
class FreeInstallTest : public testing::Test {
public:
    FreeInstallTest()
    {}
    ~FreeInstallTest()
    {}
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<Token> MockToken();
    std::shared_ptr<FreeInstallManager> freeInstallManager_ = nullptr;
};

void FreeInstallTest::SetUpTestCase(void) {}

void FreeInstallTest::TearDownTestCase(void) {}

void FreeInstallTest::SetUp(void) {}

void FreeInstallTest::TearDown(void)
{}

sptr<Token> FreeInstallTest::MockToken()
{
    sptr<Token> token = nullptr;
    AbilityRequest abilityRequest;
    abilityRequest.appInfo.bundleName = "com.test.demo";
    abilityRequest.abilityInfo.name = "MainAbility";
    abilityRequest.abilityInfo.type = AbilityType::PAGE;
    std::shared_ptr<AbilityRecord> abilityRecord = AbilityRecord::CreateAbilityRecord(abilityRequest);
    if (!abilityRecord) {
        return nullptr;
    }
    return abilityRecord->GetToken();
}

/**
 * @tc.number: FreeInstall_ConnectFreeInstall_003
 * @tc.name: ConnectFreeInstall
 * @tc.desc: Test ConnectFreeInstall returns directly when AGENT extension is already installed.
 */
HWTEST_F(FreeInstallTest, FreeInstall_ConnectFreeInstall_003, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    FreeInstallAgentMockScope mockScope(freeInstallManager_);
    ASSERT_NE(mockScope.GetBundleMgr(), nullptr);
    ASSERT_NE(mockScope.GetAppMgr(), nullptr);

    ExtensionAbilityInfo agentExtensionInfo;
    agentExtensionInfo.type = ExtensionAbilityType::AGENT;
    agentExtensionInfo.applicationInfo.bundleType = BundleType::ATOMIC_SERVICE;
    mockScope.GetBundleMgr()->SetQueryExtensionResult(true, { agentExtensionInfo });

    Want want;
    ElementName element("", REMOTE_BUNDLE_NAME, AGENT_ABILITY_NAME);
    want.SetElement(element);

    int agentRes = freeInstallManager_->ConnectFreeInstall(
        want, TEST_USER_ID, nullptr, "", ExtensionAbilityType::AGENT);

    EXPECT_EQ(agentRes, ERR_OK);
    EXPECT_EQ(mockScope.GetBundleMgr()->GetQueryAbilityInfoWithCallbackCount(), 0);
    EXPECT_TRUE(freeInstallManager_->freeInstallList_.empty());
}

/**
 * @tc.number: FreeInstall_ConnectFreeInstall_004
 * @tc.name: ConnectFreeInstall
 * @tc.desc: Test ConnectFreeInstall rejects AGENT connect when an ability target is resolved.
 */
HWTEST_F(FreeInstallTest, FreeInstall_ConnectFreeInstall_004, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    FreeInstallAgentMockScope mockScope(freeInstallManager_);
    ASSERT_NE(mockScope.GetBundleMgr(), nullptr);
    ASSERT_NE(mockScope.GetAppMgr(), nullptr);
    mockScope.GetBundleMgr()->SetQueryAbilityResult(true);

    Want want;
    ElementName element("", REMOTE_BUNDLE_NAME, "MainAbility");
    want.SetElement(element);

    int agentRes = freeInstallManager_->ConnectFreeInstall(
        want, TEST_USER_ID, nullptr, "", ExtensionAbilityType::AGENT);

    EXPECT_EQ(agentRes, ERR_WRONG_INTERFACE_CALL);
    EXPECT_EQ(mockScope.GetBundleMgr()->GetQueryAbilityInfoWithCallbackCount(), 0);
    EXPECT_TRUE(freeInstallManager_->freeInstallList_.empty());
}

/**
 * @tc.number: FreeInstall_ConnectFreeInstall_005
 * @tc.name: ConnectFreeInstall
 * @tc.desc: Test ConnectFreeInstall rejects AGENT connect when a non-AGENT extension target is resolved.
 */
HWTEST_F(FreeInstallTest, FreeInstall_ConnectFreeInstall_005, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    FreeInstallAgentMockScope mockScope(freeInstallManager_);
    ASSERT_NE(mockScope.GetBundleMgr(), nullptr);
    ASSERT_NE(mockScope.GetAppMgr(), nullptr);

    ExtensionAbilityInfo serviceExtensionInfo;
    serviceExtensionInfo.type = ExtensionAbilityType::SERVICE;
    mockScope.GetBundleMgr()->SetQueryExtensionResult(true, { serviceExtensionInfo });

    Want want;
    ElementName element("", REMOTE_BUNDLE_NAME, "ServiceAbility");
    want.SetElement(element);

    int agentRes = freeInstallManager_->ConnectFreeInstall(
        want, TEST_USER_ID, nullptr, "", ExtensionAbilityType::AGENT);

    EXPECT_EQ(agentRes, ERR_WRONG_INTERFACE_CALL);
    EXPECT_EQ(mockScope.GetBundleMgr()->GetQueryAbilityInfoWithCallbackCount(), 0);
    EXPECT_TRUE(freeInstallManager_->freeInstallList_.empty());
}

/**
 * @tc.number: FreeInstall_ConnectFreeInstall_006
 * @tc.name: ConnectFreeInstall
 * @tc.desc: Test ConnectFreeInstall starts free install for AGENT connect when no target is resolved.
 */
HWTEST_F(FreeInstallTest, FreeInstall_ConnectFreeInstall_006, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    FreeInstallAgentMockScope mockScope(freeInstallManager_);
    ASSERT_NE(mockScope.GetBundleMgr(), nullptr);
    ASSERT_NE(mockScope.GetAppMgr(), nullptr);

    Want want;
    ElementName element("", REMOTE_BUNDLE_NAME, AGENT_ABILITY_NAME);
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));

    int agentRes = freeInstallManager_->ConnectFreeInstall(
        want, TEST_USER_ID, nullptr, "", ExtensionAbilityType::AGENT);

    EXPECT_EQ(agentRes, ERR_OK);
    EXPECT_EQ(mockScope.GetBundleMgr()->GetQueryAbilityInfoWithCallbackCount(), 1);
    EXPECT_TRUE(freeInstallManager_->freeInstallList_.empty());
}

/**
 * @tc.number: RemoteFreeInstall_001
 * @tc.name: RemoteFreeInstall
 * @tc.desc: Test RemoteFreeInstall.
 */
HWTEST_F(FreeInstallTest, FreeInstall_RemoteFreeInstall_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    // mock callerToken
    const sptr<IRemoteObject> callerToken = MockToken();
    int res = 0;

    usleep(100000); // 100000 means us.
    // from freeInstallManager_->freeInstallList_ find startInstallTime
    for (auto it = freeInstallManager_->freeInstallList_.begin(); it != freeInstallManager_->freeInstallList_.end();) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        if (want.GetElement().GetBundleName().compare(bundleName) != 0 ||
            want.GetElement().GetAbilityName().compare(abilityName) != 0) {
            want.SetParam(Want::PARAM_RESV_START_TIME, (*it).want.GetStringParam(Want::PARAM_RESV_START_TIME));
            break;
        }
    }
    res = freeInstallManager_->RemoteFreeInstall(want, userId, requestCode, callerToken);
    EXPECT_EQ(res, NOT_TOP_ABILITY);
}

/**
 * @tc.number: RemoteFreeInstall_002
 * @tc.name: RemoteFreeInstall
 * @tc.desc: Test RemoteFreeInstall.
 */
HWTEST_F(FreeInstallTest, FreeInstall_RemoteFreeInstall_002, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    want.SetParam(FROM_REMOTE_KEY, true);
    // mock callerToken
    const sptr<IRemoteObject> callerToken = MockToken();
    int res = 0;

    usleep(100000); // 100000 means us.
    // from freeInstallManager_->freeInstallList_ find startInstallTime
    for (auto it = freeInstallManager_->freeInstallList_.begin(); it != freeInstallManager_->freeInstallList_.end();) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        if (want.GetElement().GetBundleName().compare(bundleName) != 0 ||
            want.GetElement().GetAbilityName().compare(abilityName) != 0) {
            want.SetParam(Want::PARAM_RESV_START_TIME, (*it).want.GetStringParam(Want::PARAM_RESV_START_TIME));
            break;
        }
    }
    res = freeInstallManager_->RemoteFreeInstall(want, userId, requestCode, callerToken);
    EXPECT_NE(res, ERR_OK);
}

/**
 * @tc.number: StartRemoteFreeInstall_001
 * @tc.name: StartRemoteFreeInstall
 * @tc.desc: Test StartRemoteFreeInstall.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartRemoteFreeInstall_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    want.SetParam(Want::PARAM_RESV_FOR_RESULT, true);
    // mock callerToken
    const sptr<IRemoteObject> callerToken = MockToken();
    int res = 0;

    usleep(100000); // 100000 means us.
    // from freeInstallManager_->freeInstallList_ find startInstallTime
    for (auto it = freeInstallManager_->freeInstallList_.begin(); it != freeInstallManager_->freeInstallList_.end();) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        if (want.GetElement().GetBundleName().compare(bundleName) != 0 ||
            want.GetElement().GetAbilityName().compare(abilityName) != 0) {
            want.SetParam(Want::PARAM_RESV_START_TIME, (*it).want.GetStringParam(Want::PARAM_RESV_START_TIME));
            break;
        }
    }
    res = freeInstallManager_->StartRemoteFreeInstall(want, userId, requestCode, callerToken);
    EXPECT_NE(res, ERR_OK);
}

/**
 * @tc.number: StartRemoteFreeInstall_002
 * @tc.name: StartRemoteFreeInstall
 * @tc.desc: Test StartRemoteFreeInstall.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartRemoteFreeInstall_002, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    want.SetParam(Want::PARAM_RESV_FOR_RESULT, false);
    // mock callerToken
    const sptr<IRemoteObject> callerToken = MockToken();
    int res = 0;
    usleep(100000); // 100000 means us.
    // from freeInstallManager_->freeInstallList_ find startInstallTime
    for (auto it = freeInstallManager_->freeInstallList_.begin(); it != freeInstallManager_->freeInstallList_.end();) {
        std::string bundleName = (*it).want.GetElement().GetBundleName();
        std::string abilityName = (*it).want.GetElement().GetAbilityName();
        if (want.GetElement().GetBundleName().compare(bundleName) != 0 ||
            want.GetElement().GetAbilityName().compare(abilityName) != 0) {
            want.SetParam(Want::PARAM_RESV_START_TIME, (*it).want.GetStringParam(Want::PARAM_RESV_START_TIME));
            break;
        }
    }
    res = freeInstallManager_->StartRemoteFreeInstall(want, userId, requestCode, callerToken);
    EXPECT_NE(res, ERR_OK);
}

/**
 * @tc.number: NotifyDmsCallback_001
 * @tc.name: NotifyDmsCallback
 * @tc.desc: Test NotifyDmsCallback.
 */
HWTEST_F(FreeInstallTest, FreeInstall_NotifyDmsCallback_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    freeInstallManager_->NotifyDmsCallback(want, requestCode);
    EXPECT_TRUE(freeInstallManager_->dmsFreeInstallCbs_.empty());
}

/**
 * @tc.number: NotifyDmsCallback_002
 * @tc.name: NotifyDmsCallback
 * @tc.desc: Test NotifyDmsCallback.
 */
HWTEST_F(FreeInstallTest, FreeInstall_NotifyDmsCallback_002, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    const int32_t userId = 100;
    const int requestCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    FreeInstallInfo info = {
        .userId = userId,
        .requestCode = requestCode,
        .dmsCallback = nullptr,
        .want = want
    };
    freeInstallManager_->dmsFreeInstallCbs_.push_back(info);
    ASSERT_FALSE(freeInstallManager_->dmsFreeInstallCbs_.empty());

    freeInstallManager_->NotifyDmsCallback(want, requestCode);
    EXPECT_TRUE(freeInstallManager_->dmsFreeInstallCbs_.empty());
}

/**
 * @tc.number: HandleOnFreeInstallSuccess_001
 * @tc.name: HandleOnFreeInstallSuccess
 * @tc.desc: Test HandleOnFreeInstallSuccess.
 */
HWTEST_F(FreeInstallTest, FreeInstall_HandleOnFreeInstallSuccess_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    int32_t recordId = 100;
    FreeInstallInfo freeInstallInfo;
    freeInstallInfo.want = want;
    bool isAsync = true;
    freeInstallInfo.isPreStartMissionCalled = true;
    freeInstallManager_->HandleOnFreeInstallSuccess(recordId, freeInstallInfo, isAsync);
    EXPECT_EQ(freeInstallInfo.isOpenAtomicServiceShortUrl, false);
}

/**
 * @tc.number: HandleOnFreeInstallSuccess_002
 * @tc.name: HandleOnFreeInstallSuccess
 * @tc.desc: Test HandleOnFreeInstallSuccess.
 */
HWTEST_F(FreeInstallTest, FreeInstall_HandleOnFreeInstallSuccess_002, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    int32_t recordId = 100;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    FreeInstallInfo freeInstallInfo;
    freeInstallInfo.want = want;
    bool isAsync = true;
    freeInstallInfo.isOpenAtomicServiceShortUrl = true;
    freeInstallManager_->HandleOnFreeInstallSuccess(recordId, freeInstallInfo, isAsync);
    EXPECT_EQ(freeInstallInfo.isPreStartMissionCalled, false);
}

/**
 * @tc.number: HandleOnFreeInstallFail_001
 * @tc.name: HandleOnFreeInstallFail
 * @tc.desc: Test HandleOnFreeInstallFail.
 */
HWTEST_F(FreeInstallTest, FreeInstall_HandleOnFreeInstallFail_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.ohos.param.sessionId", "MainAbility");
    want.SetElement(element);
    int32_t recordId = 100;
    int resultCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    FreeInstallInfo freeInstallInfo;
    freeInstallInfo.want = want;
    bool isAsync = true;
    freeInstallInfo.isPreStartMissionCalled = true;
    freeInstallInfo.isStartUIAbilityBySCBCalled = true;
    freeInstallManager_->HandleOnFreeInstallFail(recordId, freeInstallInfo, resultCode, isAsync);
    EXPECT_EQ(freeInstallInfo.isOpenAtomicServiceShortUrl, false);
}

/**
 * @tc.number: HandleOnFreeInstallFail_002
 * @tc.name: HandleOnFreeInstallFail
 * @tc.desc: Test HandleOnFreeInstallFail.
 */
HWTEST_F(FreeInstallTest, FreeInstall_HandleOnFreeInstallFail_002, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.ohos.param.sessionId", "MainAbility");
    want.SetElement(element);
    int32_t recordId = 100;
    int resultCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    FreeInstallInfo freeInstallInfo;
    freeInstallInfo.want = want;
    bool isAsync = true;
    freeInstallInfo.isOpenAtomicServiceShortUrl = true;
    freeInstallManager_->HandleOnFreeInstallFail(recordId, freeInstallInfo, resultCode, isAsync);
    EXPECT_EQ(freeInstallInfo.isPreStartMissionCalled, false);
}

/**
 * @tc.number: HandleOnFreeInstallFail_003
 * @tc.name: HandleOnFreeInstallFail
 * @tc.desc: Test HandleOnFreeInstallFail.
 */
HWTEST_F(FreeInstallTest, FreeInstall_HandleOnFreeInstallFail_003, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.example.aplication", "MainAbility");
    want.SetElement(element);
    int32_t recordId = 100;
    int resultCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    FreeInstallInfo freeInstallInfo;
    freeInstallInfo.want = want;
    bool isAsync = true;
    freeInstallInfo.isOpenAtomicServiceShortUrl = true;
    freeInstallInfo.startOptions = std::make_shared<StartOptions>();
    EXPECT_NE(freeInstallInfo.startOptions, nullptr);
    freeInstallInfo.startOptions->requestId_ = "1234567890";
    freeInstallManager_->HandleOnFreeInstallFail(recordId, freeInstallInfo, resultCode, isAsync);
    EXPECT_EQ(freeInstallInfo.isPreStartMissionCalled, false);
}

/**
 * @tc.number: HandleOnFreeInstallFail_004
 * @tc.name: HandleOnFreeInstallFail
 * @tc.desc: Test HandleOnFreeInstallFail.
 */
HWTEST_F(FreeInstallTest, FreeInstall_HandleOnFreeInstallFail_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "FreeInstall_HandleOnFreeInstallFail_004 start");
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.example.aplication", "MainAbility");
    want.SetElement(element);
    int32_t recordId = 100;
    int resultCode = 0;
    want.SetParam(Want::PARAM_RESV_START_TIME, std::string("0"));
    FreeInstallInfo freeInstallInfo;
    freeInstallInfo.want = want;
    bool isAsync = true;
    freeInstallInfo.isOpenAtomicServiceShortUrl = true;
    freeInstallInfo.startOptions = std::make_shared<StartOptions>();
    EXPECT_NE(freeInstallInfo.startOptions, nullptr);
    freeInstallInfo.startOptions->requestId_ = "1234567890";
    freeInstallInfo.callerToken = sptr<IRemoteObject>(new (std::nothrow) MockAbilityToken());
    freeInstallManager_->HandleOnFreeInstallFail(recordId, freeInstallInfo, resultCode, isAsync);
    EXPECT_EQ(freeInstallInfo.isPreStartMissionCalled, false);
    TAG_LOGI(AAFwkTag::TEST, "FreeInstall_HandleOnFreeInstallFail_004 start");
}

/**
 * @tc.number: StartAbilityByFreeInstall_001
 * @tc.name: StartAbilityByFreeInstall
 * @tc.desc: Test StartAbilityByFreeInstall.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartAbilityByFreeInstall_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    FreeInstallInfo freeInstallInfo;
    std::string bundleName = "com.test.demo";
    std::string abilityName = "MainAbility";
    std::string startTime = "2024-7-17 00:00:00";
    std::shared_ptr<FreeInstallParams> param = std::make_shared<FreeInstallParams>();
    freeInstallInfo.startOptions = param->startOptions;
    want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    freeInstallInfo.want = want;
    freeInstallManager_->StartAbilityByFreeInstall(freeInstallInfo, bundleName, abilityName, startTime);
    freeInstallInfo.startOptions = nullptr;
    freeInstallManager_->StartAbilityByFreeInstall(freeInstallInfo, bundleName, abilityName, startTime);
    EXPECT_EQ(freeInstallInfo.isPreStartMissionCalled, false);
}

/**
 * @tc.number: StartAbilityByFreeInstall_002
 * @tc.name: StartAbilityByFreeInstall
 * @tc.desc: Test StartAbilityByFreeInstall.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartAbilityByFreeInstall_002, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "", "");
    want.SetElement(element);
    FreeInstallInfo freeInstallInfo;
    std::string bundleName = "com.test.demo";
    std::string abilityName = "MainAbility";
    std::string startTime = "2024-7-17 00:00:00";
    want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    freeInstallInfo.want = want;
    freeInstallManager_->StartAbilityByFreeInstall(freeInstallInfo, bundleName, abilityName, startTime);
    EXPECT_EQ(freeInstallInfo.isPreStartMissionCalled, false);
}

/**
 * @tc.number: StartAbilityByPreInstall_001
 * @tc.name: StartAbilityByPreInstall
 * @tc.desc: Test StartAbilityByPreInstall.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartAbilityByPreInstall_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    FreeInstallInfo freeInstallInfo;
    std::string bundleName = "com.test.demo";
    std::string abilityName = "MainAbility";
    std::string startTime = "2024-7-17 00:00:00";
    std::shared_ptr<FreeInstallParams> param = std::make_shared<FreeInstallParams>();
    freeInstallInfo.startOptions = param->startOptions;
    int32_t recordId = 100;
    want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    freeInstallInfo.want = want;
    freeInstallInfo.isStartUIAbilityBySCBCalled = true;
    freeInstallInfo.isOpenAtomicServiceShortUrl = true;
    freeInstallManager_->StartAbilityByPreInstall(recordId, freeInstallInfo, bundleName, abilityName, startTime);
    freeInstallInfo.startOptions = nullptr;
    freeInstallManager_->StartAbilityByPreInstall(recordId, freeInstallInfo, bundleName, abilityName, startTime);
    EXPECT_EQ(freeInstallInfo.isPreStartMissionCalled, false);
}

/**
 * @tc.number: StartAbilityByPreInstall_002
 * @tc.name: StartAbilityByPreInstall
 * @tc.desc: Test StartAbilityByPreInstall.
 */
HWTEST_F(FreeInstallTest, FreeInstall_StartAbilityByPreInstall_002, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "", "");
    want.SetElement(element);
    int32_t recordId = 100;
    FreeInstallInfo freeInstallInfo;
    std::string bundleName = "com.test.demo";
    std::string abilityName = "MainAbility";
    std::string startTime = "2024-7-17 00:00:00";
    want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    freeInstallInfo.want = want;
    freeInstallManager_->StartAbilityByPreInstall(recordId, freeInstallInfo, bundleName, abilityName, startTime);
    EXPECT_EQ(freeInstallInfo.isPreStartMissionCalled, false);
}

/**
 * @tc.number: SetSCBCallStatus_001
 * @tc.name: SetSCBCallStatus
 * @tc.desc: Test SetSCBCallStatus.
 */
HWTEST_F(FreeInstallTest, FreeInstall_SetSCBCallStatus_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    std::string bundleName = "com.test.demo";
    std::string abilityName = "MainAbility";
    std::string startTime = "2024-7-17 00:00:00";
    want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    const int32_t userId = 1;
    const int requestCode = 0;
    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr);
    {
        std::lock_guard<ffrt::mutex> lock(freeInstallManager_->freeInstallListLock_);
        freeInstallManager_->freeInstallList_.push_back(info);
    }
    bool scbCallStatus = true;
    freeInstallManager_->SetSCBCallStatus(bundleName, abilityName, startTime, scbCallStatus);
    EXPECT_EQ(info.isStartUIAbilityBySCBCalled, false);
}

/**
 * @tc.number: SetPreStartMissionCallStatus_001
 * @tc.name: SetPreStartMissionCallStatus
 * @tc.desc: Test SetPreStartMissionCallStatus.
 */
HWTEST_F(FreeInstallTest, FreeInstall_SetPreStartMissionCallStatus_001, TestSize.Level1)
{
    freeInstallManager_ = std::make_shared<FreeInstallManager>();
    Want want;
    ElementName element("", "com.test.demo", "MainAbility");
    want.SetElement(element);
    std::string bundleName = "com.test.demo";
    std::string abilityName = "MainAbility";
    std::string startTime = "2024-7-17 00:00:00";
    want.SetParam(Want::PARAM_RESV_START_TIME, startTime);
    const int32_t userId = 1;
    const int requestCode = 0;
    FreeInstallInfo info = freeInstallManager_->BuildFreeInstallInfo(want, userId, requestCode, nullptr);
    {
        std::lock_guard<ffrt::mutex> lock(freeInstallManager_->freeInstallListLock_);
        freeInstallManager_->freeInstallList_.push_back(info);
    }
    bool preStartMissionCallStatus = true;
    freeInstallManager_->SetPreStartMissionCallStatus(bundleName, abilityName, startTime, preStartMissionCallStatus);
    EXPECT_EQ(info.isPreStartMissionCalled, false);
}
}  // namespace AppExecFwk
}  // namespace OHOS
