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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <mutex>

#define private public
#include "ability_manager_service.h"
#include "user_callback.h"
#include "user_controller.h"
#undef private

#include "refbase.h"
#include "hilog_tag_wrapper.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class UserControllerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UserControllerTest::SetUpTestCase()
{}

void UserControllerTest::TearDownTestCase()
{}

void UserControllerTest::SetUp()
{}

void UserControllerTest::TearDown()
{}

/**
 * @tc.name: UserItem_GetUserId_001
 * @tc.desc: Verify UserItem GetUserId call.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, UserItem_GetUserId_001, TestSize.Level1)
{
    int32_t expectUserId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(expectUserId);
    EXPECT_NE(userItem->GetUserId(), expectUserId);
}

/**
 * @tc.name: UserItem_SetState_001
 * @tc.desc: Verify UserItem SetState call.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, UserItem_SetState_001, TestSize.Level1)
{
    AAFwk::UserState userState = AAFwk::UserState::STATE_BOOTING;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    userItem->SetState(userState);
    EXPECT_EQ(userItem->curState_, AAFwk::UserState::STATE_BOOTING);
    EXPECT_EQ(userItem->lastState_, AAFwk::UserState::STATE_BOOTING);
}

/**
 * @tc.name: UserItem_SetState_002
 * @tc.desc: Verify UserItem SetState call.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, UserItem_SetState_002, TestSize.Level1)
{
    AAFwk::UserState userState = AAFwk::UserState::STATE_STARTED;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    userItem->SetState(userState);
    EXPECT_EQ(userItem->curState_, AAFwk::UserState::STATE_STARTED);
    EXPECT_EQ(userItem->lastState_, AAFwk::UserState::STATE_BOOTING);
}

/**
 * @tc.name: UserItem_GetState_001
 * @tc.desc: Verify UserItem GetState call.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, UserItem_GetState_001, TestSize.Level1)
{
    AAFwk::UserState userState = AAFwk::UserState::STATE_STARTED;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    userItem->SetState(userState);
    EXPECT_EQ(userItem->GetState(), AAFwk::UserState::STATE_STARTED);
}

/**
 * @tc.name: Init_001
 * @tc.desc: Verify Init call.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, Init_001, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    std::shared_ptr<AAFwk::TaskHandlerWrap> handler = nullptr;
    auto service = DelayedSingleton<AAFwk::AbilityManagerService>::GetInstance();
    EXPECT_CALL(*service, GetTaskHandler()).WillOnce(testing::Return(nullptr));
    controller->Init();
    EXPECT_EQ(controller->eventHandler_, nullptr);
}

/**
 * @tc.name: Init_002
 * @tc.desc: Verify Init call.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, Init_002, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    std::shared_ptr<AAFwk::TaskHandlerWrap> handler = std::make_shared<AAFwk::TaskHandlerWrap>();
    auto service = DelayedSingleton<AAFwk::AbilityManagerService>::GetInstance();
    EXPECT_CALL(*service, GetTaskHandler()).WillOnce(testing::Return(handler));
    controller->Init();
    EXPECT_NE(controller->eventHandler_, nullptr);
}

/**
 * @tc.name: Init_003
 * @tc.desc: Verify Init call.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, Init_003, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    std::shared_ptr<AAFwk::TaskHandlerWrap> handler = std::make_shared<AAFwk::TaskHandlerWrap>();
    auto service = DelayedSingleton<AAFwk::AbilityManagerService>::GetInstance();
    EXPECT_CALL(*service, GetTaskHandler()).WillOnce(testing::Return(handler));
    controller->eventHandler_ = std::make_shared<AAFwk::UserEventHandler>(handler, controller);
    controller->Init();
    EXPECT_NE(controller->eventHandler_, nullptr);
}

/**
 * @tc.name: ClearAbilityUserItems_001
 * @tc.desc: Verify ClearAbilityUserItems call.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, ClearAbilityUserItems_001, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    controller->userItems_.insert({0, userItem});
    controller->ClearAbilityUserItems(0);
    EXPECT_EQ(controller->userItems_.size(), 0);
}

/**
 * @tc.name: ClearAbilityUserItems_002
 * @tc.desc: Verify ClearAbilityUserItems call.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, ClearAbilityUserItems_002, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    controller->userItems_.insert({0, userItem});
    controller->ClearAbilityUserItems(1);
    EXPECT_EQ(controller->userItems_.size(), 1);
}

/**
 * @tc.name: StartUser_001
 * @tc.desc: Verify StartUser call.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_001, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    int32_t userId = -1;
    sptr<AAFwk::IUserCallback> callback = nullptr;
    bool isAppRecovery = 0;
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), AAFwk::INVALID_USERID_VALUE);
}

/**
 * @tc.name: StartUser_002
 * @tc.desc: Verify StartUser call.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_002, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    int32_t userId = -1;
    sptr<AAFwk::IUserCallback> callback = sptr<AAFwk::IUserCallback>::MakeSptr();
    bool isAppRecovery = 0;
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), AAFwk::INVALID_USERID_VALUE);
}

/**
 * @tc.name: StartUser_003
 * @tc.desc: Verify StartUser call.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_003, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    int32_t userId = AAFwk::USER_ID_NO_HEAD;
    sptr<AAFwk::IUserCallback> callback = nullptr;
    bool isAppRecovery = 0;
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), AAFwk::INVALID_USERID_VALUE);
}

/**
 * @tc.name: StartUser_004
 * @tc.desc: Verify StartUser call.
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_004, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    int32_t userId = AAFwk::USER_ID_NO_HEAD;
    sptr<AAFwk::IUserCallback> callback = sptr<AAFwk::IUserCallback>::MakeSptr();
    bool isAppRecovery = 0;
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), AAFwk::INVALID_USERID_VALUE);
}

/**
 * @tc.name: StartUser_005
 * @tc.desc: Verify StartUser call.
 *           Branch IsCurrentUser == true
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_005, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 101;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    controller->userItems_.insert({101, userItem});
    sptr<AAFwk::IUserCallback> callback = nullptr;
    bool isAppRecovery = 0;
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), ERR_OK);
}

/**
 * @tc.name: StartUser_006
 * @tc.desc: Verify StartUser call.
 *           Branch IsCurrentUser == true
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_006, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 101;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    controller->userItems_.insert({101, userItem});
    sptr<AAFwk::IUserCallback> callback = sptr<AAFwk::IUserCallback>::MakeSptr();
    bool isAppRecovery = 0;
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), ERR_OK);
}
} // namespace AppExecFwk
} // namespace OHOS