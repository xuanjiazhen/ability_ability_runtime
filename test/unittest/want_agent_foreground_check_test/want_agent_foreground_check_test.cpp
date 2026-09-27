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
#include "mock_native_token.h"
#include "ability_manager_errors.h"
#define private public
#define protected public
#include "pending_want_record.h"
#include "pending_want_manager.h"
#include "ability_manager_service.h"
#include "user_controller/user_controller.h"
#undef private
#undef protected
#include "sender_info.h"
#include "want_receiver_stub.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace AAFwk {

class WantAgentForegroundCheckTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    sptr<IWantSender> CreateWantSender(int32_t userId);
};

void WantAgentForegroundCheckTest::SetUpTestCase() {}
void WantAgentForegroundCheckTest::TearDownTestCase() {}
void WantAgentForegroundCheckTest::SetUp() {}
void WantAgentForegroundCheckTest::TearDown() {}

sptr<IWantSender> WantAgentForegroundCheckTest::CreateWantSender(int32_t userId)
{
    auto want = std::make_shared<Want>();
    ElementName element("device", "com.test.bundle", "MainAbility");
    want->SetElement(element);
    WantSenderInfo wantSenderInfo;
    wantSenderInfo.type = static_cast<int32_t>(OperationType::START_ABILITY);
    wantSenderInfo.userId = userId;
    auto pendingWantManager = std::make_shared<PendingWantManager>();
    return pendingWantManager->GetWantSender(wantSenderInfo, nullptr);
}

/**
 * @tc.name: WantAgentForegroundCheck_001
 * @tc.desc: VM-7 - Target user in foreground, wantAgent triggers normally (R-7/AC-4.1)
 * @tc.type: FUNC
 * @tc.require: I9Q7QJ
 */
HWTEST_F(WantAgentForegroundCheckTest, WantAgentForegroundCheck_001, TestSize.Level1)
{
    auto abilityMgrService = DelayedSingleton<AbilityManagerService>::GetInstance();
    ASSERT_NE(abilityMgrService, nullptr);

    int32_t testUserId = 100;
    auto target = CreateWantSender(testUserId);
    ASSERT_NE(target, nullptr);

    SenderInfo senderInfo;
    senderInfo.code = 0;

    auto result = abilityMgrService->SendWantSender(target, senderInfo);
    EXPECT_NE(result, ERR_INVALID_VALUE);
}

/**
 * @tc.name: WantAgentForegroundCheck_002
 * @tc.desc: VM-8 - Target user not in foreground, wantAgent returns error code (R-8/AC-4.2)
 * @tc.type: FUNC
 * @tc.require: I9Q7QJ
 */
HWTEST_F(WantAgentForegroundCheckTest, WantAgentForegroundCheck_002, TestSize.Level1)
{
    auto abilityMgrService = DelayedSingleton<AbilityManagerService>::GetInstance();
    ASSERT_NE(abilityMgrService, nullptr);

    int32_t nonForegroundUserId = 999;
    auto target = CreateWantSender(nonForegroundUserId);
    ASSERT_NE(target, nullptr);

    SenderInfo senderInfo;
    senderInfo.code = 0;

    auto result = abilityMgrService->SendWantSender(target, senderInfo);
    EXPECT_EQ(result, ERR_OPERATION_NOT_SUPPORTED_ON_CURRENT_DEVICE);
}

/**
 * @tc.name: WantAgentForegroundCheck_003
 * @tc.desc: VM-9 - UserController unavailable (no foreground users), degrade to direct trigger (R-9/AC-4.3)
 * @tc.type: FUNC
 * @tc.require: I9Q7QJ
 */
HWTEST_F(WantAgentForegroundCheckTest, WantAgentForegroundCheck_003, TestSize.Level1)
{
    auto abilityMgrService = DelayedSingleton<AbilityManagerService>::GetInstance();
    ASSERT_NE(abilityMgrService, nullptr);

    auto& uc = AbilityRuntime::UserController::GetInstance();
    auto displayIdMapCopy = uc.displayIdMap_;
    uc.displayIdMap_.clear();

    int32_t testUserId = 100;
    auto target = CreateWantSender(testUserId);
    ASSERT_NE(target, nullptr);

    SenderInfo senderInfo;
    senderInfo.code = 0;

    auto result = abilityMgrService->SendWantSender(target, senderInfo);
    EXPECT_NE(result, ERR_OPERATION_NOT_SUPPORTED_ON_CURRENT_DEVICE);

    uc.displayIdMap_ = displayIdMapCopy;
}
}  // namespace AAFwk
}  // namespace OHOS
