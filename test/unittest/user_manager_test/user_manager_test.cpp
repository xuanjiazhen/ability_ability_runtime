/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#define private public
#include "user_controller/user_manager.h"
#undef private
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class UserManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UserManagerTest::SetUpTestCase() {}
void UserManagerTest::TearDownTestCase() {}
void UserManagerTest::SetUp() {}
void UserManagerTest::TearDown() {}

/**
 * @tc.name: ClearUserId_0100
 * @tc.desc: ClearUserId Test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UserManagerTest, ClearUserId_0100, TestSize.Level1)
{
    int32_t userId = 201;
    uint64_t displayIdParam = 11;
    UserManager::GetInstance().SetCurrentUserId(userId, displayIdParam);
    int32_t mapSize = UserManager::GetInstance().displayIdMap_.size();
    UserManager::GetInstance().ClearUserId(userId);
    EXPECT_EQ(UserManager::GetInstance().displayIdMap_.size(), (mapSize - 1));
}

/**
 * @tc.name: IsCurrentUserTest_0100
 * @tc.desc: IsCurrentUser Test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UserManagerTest, IsCurrentUser_0100, TestSize.Level1)
{
    int32_t userId = 202;
    auto result = UserManager::GetInstance().IsCurrentUser(userId);
    EXPECT_FALSE(result);
}

/**
 * @tc.name: IsCurrentUserTest_0200
 * @tc.desc: IsCurrentUser Test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UserManagerTest, IsCurrentUser_0200, TestSize.Level1)
{
    int32_t userId = 201;
    uint64_t displayIdParam = 11;
    UserManager::GetInstance().SetCurrentUserId(userId, displayIdParam);
    auto result = UserManager::GetInstance().IsCurrentUser(userId);
    EXPECT_TRUE(result);
}

/**
 * @tc.name: GetCurrentUserId_0100
 * @tc.desc: GetCurrentUserId Test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UserManagerTest, GetCurrentUserId_0100, TestSize.Level1)
{
    int32_t userIdParam = 201;
    uint64_t displayId = 11;
    UserManager::GetInstance().SetCurrentUserId(userIdParam, displayId);
    auto useId = UserManager::GetInstance().GetCurrentUserId(displayId);
    EXPECT_EQ(useId, userIdParam);
}

/**
 * @tc.name: GetDisplayIdByUserIdTest_0100
 * @tc.desc: GetDisplayIdByUserId Test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UserManagerTest, GetDisplayIdByUserId_0100, TestSize.Level1)
{
    int32_t userId = 202;
    uint64_t displayId = 0;
    auto ret = UserManager::GetInstance().GetDisplayIdByUserId(userId, displayId);
    EXPECT_EQ(displayId, 0);
}

/**
 * @tc.name: GetDisplayIdByUserIdTest_0200
 * @tc.desc: GetDisplayIdByUserId Test
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(UserManagerTest, GetDisplayIdByUserId_0200, TestSize.Level1)
{
    int32_t userId = 201;
    uint64_t displayIdParam = 11;
    UserManager::GetInstance().SetCurrentUserId(userId, displayIdParam);
    uint64_t displayId = 0;
    auto ret = UserManager::GetInstance().GetDisplayIdByUserId(userId, displayId);
    EXPECT_EQ(displayId, displayIdParam);
}
}  // namespace AAFwk
}  // namespace OHOS
