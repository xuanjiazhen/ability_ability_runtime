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
    EXPECT_EQ(userItem->GetUserId(), expectUserId);
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
    OH_EXPECT_RET({handler}, AbilityManagerService, GetTaskHandler);
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
    OH_EXPECT_RET({handler}, AbilityManagerService, GetTaskHandler);
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
    OH_EXPECT_RET({handler}, AbilityManagerService, GetTaskHandler);
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
 *           Branch userId < 0 && callback == nullptr
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
 *           Branch userId < 0 && callback != nullptr
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
 *           Branch userId == USER_ID_NO_HEAD && callback == nullptr
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
 *           Branch userId == USER_ID_NO_HEAD && callback != nullptr
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
 *           Branch IsCurrentUser == true && callback == nullptr
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
 *           Branch IsCurrentUser == true && callback != nullptr
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

/**
 * @tc.name: StartUser_007
 * @tc.desc: Verify StartUser call.
 *           Branch IsExistOsAccount == false && callback == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_007, TestSize.Level1)
{
    TAG_LOGE(AAFwkTag::CONNECTION, "StartUser_007 start");
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 100;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    controller->userItems_.insert({101, userItem});
    sptr<AAFwk::IUserCallback> callback = nullptr;
    bool isAppRecovery = 0;
    std::vector<vector<bool>> expectOuter = {{false}};
    OH_EXPECT_RET_AND_OUTPUT({0}, expectOuter, OsAccountManagerWrapper,
        IsOsAccountExists, const int, bool& isOsAccountExists);
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), AAFwk::INVALID_USERID_VALUE);
}

/**
 * @tc.name: StartUser_008
 * @tc.desc: Verify StartUser call.
 *           Branch IsExistOsAccount == false && callback != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_008, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 100;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    controller->userItems_.insert({101, userItem});
    sptr<AAFwk::IUserCallback> callback = sptr<AAFwk::IUserCallback>::MakeSptr();
    bool isAppRecovery = 0;
    std::vector<vector<bool>> expectOuter = {{false}};
    OH_EXPECT_RET_AND_OUTPUT({0}, expectOuter, OsAccountManagerWrapper,
        IsOsAccountExists, const int, bool& isOsAccountExists);
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), AAFwk::INVALID_USERID_VALUE);
}

/**
 * @tc.name: StartUser_009
 * @tc.desc: Verify StartUser call.
 *           Branch GetCurrentUserId() != USER_ID_NO_HEAD && IsSceneBoardEnabled() == false
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_009, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 100;
    controller->freezingNewUserId_ = 0;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    controller->userItems_.insert({userId, userItem});
    sptr<AAFwk::IUserCallback> callback = sptr<AAFwk::IUserCallback>::MakeSptr();
    bool isAppRecovery = 0;
    std::vector<vector<bool>> expectOuter = {{true}};
    OH_EXPECT_RET_AND_OUTPUT({0}, expectOuter, OsAccountManagerWrapper,
        IsOsAccountExists, const int, bool& isOsAccountExists);
    OH_EXPECT_RET({false}, SceneBoardJudgement, IsSceneBoardEnabled);
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), ERR_OK);
    EXPECT_EQ(controller->freezingNewUserId_, 101);
}

/**
 * @tc.name: StartUser_010
 * @tc.desc: Verify StartUser call.
 *           Branch GetCurrentUserId() == USER_ID_NO_HEAD && IsSceneBoardEnabled() == false
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_010, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 0;
    controller->freezingNewUserId_ = 0;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    controller->userItems_.insert({userId, userItem});
    sptr<AAFwk::IUserCallback> callback = sptr<AAFwk::IUserCallback>::MakeSptr();
    bool isAppRecovery = 0;
    std::vector<vector<bool>> expectOuter = {{true}};
    OH_EXPECT_RET_AND_OUTPUT({0}, expectOuter, OsAccountManagerWrapper,
        IsOsAccountExists, const int, bool& isOsAccountExists);
    OH_EXPECT_RET({false}, SceneBoardJudgement, IsSceneBoardEnabled);
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), ERR_OK);
    EXPECT_EQ(controller->freezingNewUserId_, 0);
}

/**
 * @tc.name: StartUser_011
 * @tc.desc: Verify StartUser call.
 *           Branch GetCurrentUserId() != USER_ID_NO_HEAD && IsSceneBoardEnabled() == true
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_011, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 100;
    controller->freezingNewUserId_ = 0;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    controller->userItems_.insert({userId, userItem});
    sptr<AAFwk::IUserCallback> callback = sptr<AAFwk::IUserCallback>::MakeSptr();
    bool isAppRecovery = 0;
    std::vector<vector<bool>> expectOuter = {{true}};
    OH_EXPECT_RET_AND_OUTPUT({0}, expectOuter, OsAccountManagerWrapper,
        IsOsAccountExists, const int, bool& isOsAccountExists);
    OH_EXPECT_RET({true}, SceneBoardJudgement, IsSceneBoardEnabled);
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), ERR_OK);
    EXPECT_EQ(controller->freezingNewUserId_, 0);
}

/**
 * @tc.name: StartUser_012
 * @tc.desc: Verify StartUser call.
 *           Branch GetCurrentUserId() == USER_ID_NO_HEAD && IsSceneBoardEnabled() == true
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_012, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 0;
    controller->freezingNewUserId_ = 0;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    controller->userItems_.insert({userId, userItem});
    sptr<AAFwk::IUserCallback> callback = sptr<AAFwk::IUserCallback>::MakeSptr();
    bool isAppRecovery = 0;
    std::vector<vector<bool>> expectOuter = {{true}};
    OH_EXPECT_RET_AND_OUTPUT({0}, expectOuter, OsAccountManagerWrapper,
        IsOsAccountExists, const int, bool& isOsAccountExists);
    OH_EXPECT_RET({true}, SceneBoardJudgement, IsSceneBoardEnabled);
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), ERR_OK);
    EXPECT_EQ(controller->freezingNewUserId_, 0);
}

/**
 * @tc.name: StartUser_013
 * @tc.desc: Verify StartUser call.
 *           Branch state == STATE_STOPPING && callback != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_013, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 100;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    userItem->SetState(AAFwk::STATE_STOPPING);
    controller->userItems_.insert({userId, userItem});
    sptr<AAFwk::IUserCallback> callback = sptr<AAFwk::IUserCallback>::MakeSptr();
    bool isAppRecovery = 0;
    std::vector<vector<bool>> expectOuter = {{true}};
    OH_EXPECT_RET_AND_OUTPUT({0}, expectOuter, OsAccountManagerWrapper,
        IsOsAccountExists, const int, bool& isOsAccountExists);
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), ERR_DEAD_OBJECT);
}

/**
 * @tc.name: StartUser_014
 * @tc.desc: Verify StartUser call.
 *           Branch state == STATE_STOPPING && callback == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_014, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 100;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    userItem->SetState(AAFwk::STATE_STOPPING);
    controller->userItems_.insert({userId, userItem});
    sptr<AAFwk::IUserCallback> callback = nullptr;
    bool isAppRecovery = 0;
    std::vector<vector<bool>> expectOuter = {{true}};
    OH_EXPECT_RET_AND_OUTPUT({0}, expectOuter, OsAccountManagerWrapper,
        IsOsAccountExists, const int, bool& isOsAccountExists);
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), ERR_DEAD_OBJECT);
}

/**
 * @tc.name: StartUser_015
 * @tc.desc: Verify StartUser call.
 *           Branch state == STATE_SHUTDOWN && callback != nullptr
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_015, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 100;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    userItem->SetState(AAFwk::STATE_SHUTDOWN);
    controller->userItems_.insert({userId, userItem});
    sptr<AAFwk::IUserCallback> callback = sptr<AAFwk::IUserCallback>::MakeSptr();
    bool isAppRecovery = 0;
    std::vector<vector<bool>> expectOuter = {{true}};
    OH_EXPECT_RET_AND_OUTPUT({0}, expectOuter, OsAccountManagerWrapper,
        IsOsAccountExists, const int, bool& isOsAccountExists);
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), ERR_DEAD_OBJECT);
}

/**
 * @tc.name: StartUser_016
 * @tc.desc: Verify StartUser call.
 *           Branch state == STATE_SHUTDOWN && callback == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_016, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 100;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    userItem->SetState(AAFwk::STATE_SHUTDOWN);
    controller->userItems_.insert({userId, userItem});
    sptr<AAFwk::IUserCallback> callback = nullptr;
    bool isAppRecovery = 0;
    std::vector<vector<bool>> expectOuter = {{true}};
    OH_EXPECT_RET_AND_OUTPUT({0}, expectOuter, OsAccountManagerWrapper,
        IsOsAccountExists, const int, bool& isOsAccountExists);
    EXPECT_EQ(controller->StartUser(userId, callback, isAppRecovery), ERR_DEAD_OBJECT);
}

/**
 * @tc.name: StartUser_017
 * @tc.desc: Verify StartUser call.
 *           Branch state == STATE_BOOTING
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_017, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 100;
    int32_t userId = 101;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    userItem->SetState(AAFwk::STATE_BOOTING);
    controller->userItems_.insert({userId, userItem});
    sptr<AAFwk::IUserCallback> callback = nullptr;
    bool isAppRecovery = 0;
    std::vector<vector<bool>> expectOuter = {{true}};
    OH_EXPECT_RET_AND_OUTPUT({0}, expectOuter, OsAccountManagerWrapper,
        IsOsAccountExists, const int, bool& isOsAccountExists);
    EXPECT_NE(controller->StartUser(userId, callback, isAppRecovery), ERR_DEAD_OBJECT);
    EXPECT_EQ(controller->currentUserId_, 101);
}

/**
 * @tc.name: StopUser_001
 * @tc.desc: Verify StopUser call.
 *           Branch userId < 0
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_001, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    int32_t userId = -1;
    EXPECT_EQ(controller->StopUser(userId), -1);
}

/**
 * @tc.name: StopUser_002
 * @tc.desc: Verify StopUser call.
 *           Branch userId == USER_ID_NO_HEAD
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_002, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    int32_t userId = AAFwk::USER_ID_NO_HEAD;
    EXPECT_EQ(controller->StopUser(userId), -1);
}

/**
 * @tc.name: StopUser_003
 * @tc.desc: Verify StopUser call.
 *           Branch userId == USER_ID_DEFAULT
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_003, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    int32_t userId = AAFwk::USER_ID_DEFAULT;
    EXPECT_EQ(controller->StopUser(userId), -1);
}

/**
 * @tc.name: StopUser_004
 * @tc.desc: Verify StopUser call.
 *           Branch IsCurrentUser == true

 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_004, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 100;
    int32_t userId = 100;
    std::shared_ptr<AAFwk::UserItem> userItem = std::make_shared<AAFwk::UserItem>(userId);
    controller->userItems_.insert({userId, userItem});
    EXPECT_EQ(controller->StopUser(userId), 0);
}

/**
 * @tc.name: StopUser_005
 * @tc.desc: Verify StopUser call.
 *           Branch IsExistOsAccount == false
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_005, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 100;
    int32_t userId = 101;
    std::vector<vector<bool>> expectOuter = {{false}};
    OH_EXPECT_RET_AND_OUTPUT({0}, expectOuter, OsAccountManagerWrapper,
        IsOsAccountExists, const int, bool& isOsAccountExists);
    EXPECT_EQ(controller->StopUser(userId), -1);
}

/**
 * @tc.name: StopUser_006
 * @tc.desc: Verify StopUser call.
 *           Branch IsCurrentUser == false
 * @tc.type: FUNC
 */
HWTEST_F(UserControllerTest, StartUser_006, TestSize.Level1)
{
    std::shared_ptr<AAFwk::UserController> controller = std::make_shared<AAFwk::UserController>();
    controller->currentUserId_ = 100;
    int32_t userId = 101;
    std::vector<vector<bool>> expectOuter = {{true}};
    OH_EXPECT_RET_AND_OUTPUT({0}, expectOuter, OsAccountManagerWrapper,
        IsOsAccountExists, const int, bool& isOsAccountExists);
    EXPECT_EQ(controller->StopUser(userId), -1);
}
} // namespace AppExecFwk
} // namespace OHOS