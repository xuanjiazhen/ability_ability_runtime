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
#include "app_mgr_service.h"
#include "ability_manager_errors.h"
#include "mock_ipc_skeleton.h"
#include "mock_my_flag.h"
#include "mock_permission_verification.h"


using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceSecondTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppMgrServiceSecondTest::SetUpTestCase(void) {}

void AppMgrServiceSecondTest::TearDownTestCase(void) {}

void AppMgrServiceSecondTest::SetUp() {}

void AppMgrServiceSecondTest::TearDown() {}

/**
 * @tc.name: CheckCallingIsUserTestMode_0100
 * @tc.desc: CheckCallingIsUserTestMode.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, CheckCallingIsUserTestMode_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int pid = 1;
    bool isUserTest = false;
    appMgrService->appMgrServiceInner_ = nullptr;
    auto ret = appMgrService->CheckCallingIsUserTestMode(pid, isUserTest);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    ret = appMgrService->CheckCallingIsUserTestMode(pid, isUserTest);
    EXPECT_NE(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: NotifyProcessDependedOnWeb_0100
 * @tc.desc: NotifyProcessDependedOnWeb.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, NotifyProcessDependedOnWeb_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    appMgrService->appMgrServiceInner_ = nullptr;
    auto ret = appMgrService->NotifyProcessDependedOnWeb();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    ret = appMgrService->NotifyProcessDependedOnWeb();
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: KillAppSelfWithInstanceKey_0100
 * @tc.desc: KillAppSelfWithInstanceKey.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, KillAppSelfWithInstanceKey_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::string instanceKey;
    bool clearPageStack = false;
    std::string reason;
    appMgrService->appMgrServiceInner_ = nullptr;
    auto ret = appMgrService->KillAppSelfWithInstanceKey(instanceKey, clearPageStack, reason);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);

    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    ret = appMgrService->KillAppSelfWithInstanceKey(instanceKey, clearPageStack, reason);
    EXPECT_NE(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.name: IsSpecifiedModuleLoaded_0100
 * @tc.desc: IsSpecifiedModuleLoaded.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, IsSpecifiedModuleLoaded_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    AAFwk::Want want;
    AbilityInfo abilityInfo;
    bool result;
    auto ret = appMgrService->IsSpecifiedModuleLoaded(want, abilityInfo, result);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(appMgrService->taskHandler_,
            appMgrService->appMgrServiceInner_);
    IPCSkeleton::pid_ = 0;
    ret = appMgrService->IsSpecifiedModuleLoaded(want, abilityInfo, result);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    IPCSkeleton::pid_ = getprocpid();
    ret = appMgrService->IsSpecifiedModuleLoaded(want, abilityInfo, result);
    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: UpdateProcessMemoryState_0100
 * @tc.desc: UpdateProcessMemoryState.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, UpdateProcessMemoryState_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    std::vector<ProcessMemoryState> procMemState;
    auto ret = appMgrService->UpdateProcessMemoryState(procMemState);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);

    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(appMgrService->taskHandler_,
            appMgrService->appMgrServiceInner_);
    AAFwk::MyFlag::flag_ = 1;
    ret = appMgrService->UpdateProcessMemoryState(procMemState);
    EXPECT_EQ(ret, ERR_PERMISSION_DENIED);

    AAFwk::MyFlag::flag_ = 0;
    ret = appMgrService->UpdateProcessMemoryState(procMemState);
    EXPECT_NE(ret, ERR_INVALID_OPERATION);
    EXPECT_NE(ret, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: GetKilledProcessInfo_0100
 * @tc.desc: GetKilledProcessInfo.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceSecondTest, GetKilledProcessInfo_0100, TestSize.Level1)
{
    auto appMgrService = std::make_shared<AppMgrService>();
    int pid = 1;
    int uid = 0;
    KilledProcessInfo info;
    auto ret = appMgrService->GetKilledProcessInfo(pid, uid, info);
    EXPECT_EQ(ret, AAFwk::ERR_APP_MGR_SERVICE_NOT_READY);

    appMgrService->taskHandler_ = AAFwk::TaskHandlerWrap::CreateQueueHandler(Constants::APP_MGR_SERVICE_NAME);
    appMgrService->appMgrServiceInner_ = std::make_shared<AppMgrServiceInner>();
    appMgrService->eventHandler_ = std::make_shared<AMSEventHandler>(appMgrService->taskHandler_,
            appMgrService->appMgrServiceInner_);
    IPCSkeleton::pid_ = 0;
    ret = appMgrService->GetKilledProcessInfo(pid, uid, info);
    EXPECT_EQ(ret, AAFwk::ERR_NO_ALLOW_OUTSIDE_CALL);

    IPCSkeleton::pid_ = getprocpid();
    ret = appMgrService->GetKilledProcessInfo(pid, uid, info);
    EXPECT_NE(ret, AAFwk::ERR_APP_MGR_SERVICE_NOT_READY);
    EXPECT_NE(ret, AAFwk::ERR_NO_ALLOW_OUTSIDE_CALL);
}

} // namespace AppExecFwk
} // namespace OHOS
