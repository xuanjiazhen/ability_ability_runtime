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
#define protected public
#include "app_query_helper.h"
#include "app_running_manager.h"
#include "app_running_record.h"
#include "remote_client_manager.h"
#include "bundle_mgr_helper.h"
#include "global_constant.h"
#include "user_controller/user_controller.h"
#undef private
#undef protected
#include "mock_my_status.h"
#include "ability_manager_errors.h"
#include "app_mgr_constants.h"
#include "app_utils.h"
#include "permission_verification.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t FOUNDATION_UID = 5523;
constexpr int32_t FILE_SYSTEM_SERVICE_UID = 7100;
constexpr int32_t BASE_USER_RANGE = 200000;
constexpr int32_t TEST_USER_ID_100 = 100;
constexpr int32_t TEST_USER_ID_101 = 101;
constexpr uint64_t TEST_DEFAULT_DISPLAY_ID = 0;
constexpr const char* TEST_BUNDLE_NAME = "com.example.test";
constexpr int32_t INVALID_CLONE_INDEX_NEG = -1;
constexpr int32_t INVALID_CLONE_INDEX_MID = 5000;
constexpr int32_t VALID_CLONE_INDEX = 1;
constexpr int32_t PC_TABLET_INDEX_VAL = GlobalConstant::PC_TABLET_INDEX;
}

class AppQueryHelperTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AppQueryHelper> appQueryHelper_ = nullptr;
};

void AppQueryHelperTest::SetUpTestCase()
{
    auto& status = AAFwk::MyStatus::GetInstance();
    status.getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
}

void AppQueryHelperTest::TearDownTestCase()
{
    auto& status = AAFwk::MyStatus::GetInstance();
    status.getBundleManagerHelper_ = nullptr;
}

void AppQueryHelperTest::SetUp()
{
    appQueryHelper_ = std::make_shared<AppQueryHelper>();
    auto& status = AAFwk::MyStatus::GetInstance();
    status.verifyRunningInfoPerm_ = true;
    status.isSACall_ = true;
    status.judgeCallerIsAllowed_ = true;
    status.getCallingUid_ = FOUNDATION_UID;
    status.getBundleInfoV9_ = ERR_OK;
    status.getCloneBundleInfo_ = ERR_OK;
    status.checkAppClone_ = ERR_OK;
    status.isAppRunningByBundleName_ = ERR_OK;
    status.getDualModeBundleInfoCall_ = 0;
    status.getDualModeBundleInfoAppIndex_ = 0;

    auto appRunningManager = std::make_shared<AppRunningManager>();
    auto remoteClientManager = std::make_shared<RemoteClientManager>();
    appQueryHelper_->appRunningManager_ = appRunningManager;
    appQueryHelper_->remoteClientManager_ = remoteClientManager;
    appQueryHelper_->multiUserConfigurationMgr_ = nullptr;
    appQueryHelper_->appPreloader_ = nullptr;

    UserController::GetInstance().SetForegroundUserId(TEST_USER_ID_100, TEST_DEFAULT_DISPLAY_ID);
}

void AppQueryHelperTest::TearDown()
{
    appQueryHelper_ = nullptr;
    AAFwk::MyStatus::GetInstance().getDualModeBundleInfoCall_ = 0;
}

/**
 * @tc.name: GetValidUserId_001
 * @tc.desc: Test GetValidUserId with a valid userId (not DEFAULT_INVALID_VALUE).
 *           Should return the userId directly without modification.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, GetValidUserId_001, TestSize.Level1)
{
    int32_t userId = TEST_USER_ID_100;
    int32_t result = appQueryHelper_->GetValidUserId(userId);
    EXPECT_EQ(result, TEST_USER_ID_100);
}

/**
 * @tc.name: GetValidUserId_002
 * @tc.desc: Test GetValidUserId with DEFAULT_INVALID_VALUE (-1) and caller UID maps to a
 *           non-system user (e.g. user 100). Should return the derived userId.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, GetValidUserId_002, TestSize.Level1)
{
    AAFwk::MyStatus::GetInstance().getCallingUid_ = TEST_USER_ID_100 * BASE_USER_RANGE;
    int32_t userId = AAFwk::DEFAULT_INVALID_VALUE;
    int32_t result = appQueryHelper_->GetValidUserId(userId);
    EXPECT_EQ(result, TEST_USER_ID_100);
}

/**
 * @tc.name: GetValidUserId_003
 * @tc.desc: Test GetValidUserId with DEFAULT_INVALID_VALUE (-1) and caller UID maps to U0.
 *           Should resolve to the foreground user ID.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, GetValidUserId_003, TestSize.Level1)
{
    AAFwk::MyStatus::GetInstance().getCallingUid_ = 0;
    UserController::GetInstance().SetForegroundUserId(TEST_USER_ID_101, TEST_DEFAULT_DISPLAY_ID);
    int32_t userId = AAFwk::DEFAULT_INVALID_VALUE;
    int32_t result = appQueryHelper_->GetValidUserId(userId);
    EXPECT_EQ(result, TEST_USER_ID_101);
    UserController::GetInstance().SetForegroundUserId(TEST_USER_ID_100, TEST_DEFAULT_DISPLAY_ID);
}

/**
 * @tc.name: GetValidUserId_004
 * @tc.desc: Test GetValidUserId with DEFAULT_INVALID_VALUE (-1) and caller UID maps to U1.
 *           Should resolve to the foreground user ID.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, GetValidUserId_004, TestSize.Level1)
{
    AAFwk::MyStatus::GetInstance().getCallingUid_ = BASE_USER_RANGE;
    UserController::GetInstance().SetForegroundUserId(TEST_USER_ID_101, TEST_DEFAULT_DISPLAY_ID);
    int32_t userId = AAFwk::DEFAULT_INVALID_VALUE;
    int32_t result = appQueryHelper_->GetValidUserId(userId);
    EXPECT_EQ(result, TEST_USER_ID_101);
    UserController::GetInstance().SetForegroundUserId(TEST_USER_ID_100, TEST_DEFAULT_DISPLAY_ID);
}

/**
 * @tc.name: IsAppRunning_3Arg_001
 * @tc.desc: Test IsAppRunning (3-arg) with permission denied.
 *           Should return ERR_PERMISSION_DENIED.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_3Arg_001, TestSize.Level1)
{
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = false;
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, 0, isRunning);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: IsAppRunning_3Arg_002
 * @tc.desc: Test IsAppRunning (3-arg) with invalid appCloneIndex (negative).
 *           Should return ERR_APP_CLONE_INDEX_INVALID.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_3Arg_002, TestSize.Level1)
{
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, INVALID_CLONE_INDEX_NEG, isRunning);
    EXPECT_EQ(result, AAFwk::ERR_APP_CLONE_INDEX_INVALID);
}

/**
 * @tc.name: IsAppRunning_3Arg_003
 * @tc.desc: Test IsAppRunning (3-arg) with invalid appCloneIndex (mid-range, not a valid
 *           clone index, not PC_TABLET_INDEX, not 0). Should return ERR_APP_CLONE_INDEX_INVALID.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_3Arg_003, TestSize.Level1)
{
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, INVALID_CLONE_INDEX_MID, isRunning);
    EXPECT_EQ(result, AAFwk::ERR_APP_CLONE_INDEX_INVALID);
}

/**
 * @tc.name: IsAppRunning_3Arg_004
 * @tc.desc: Test IsAppRunning (3-arg) with remoteClientManager_ null.
 *           Should return ERR_INVALID_OPERATION.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_3Arg_004, TestSize.Level1)
{
    appQueryHelper_->remoteClientManager_ = nullptr;
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, 0, isRunning);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
}

/**
 * @tc.name: IsAppRunning_3Arg_005
 * @tc.desc: Test IsAppRunning (3-arg) with bundleMgrHelper null.
 *           Should return ERR_INVALID_OPERATION.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_3Arg_005, TestSize.Level1)
{
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, 0, isRunning);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
}

/**
 * @tc.name: IsAppRunning_3Arg_006
 * @tc.desc: Test IsAppRunning (3-arg) with appCloneIndex == 0 (normal app).
 *           Should call GetDualModeBundleInfo and GetBundleInfoV9, then succeed.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_3Arg_006, TestSize.Level1)
{
    AAFwk::MyStatus::GetInstance().getCallingUid_ = TEST_USER_ID_100 * BASE_USER_RANGE;
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, 0, isRunning);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getDualModeBundleInfoCall_, 1);
}

/**
 * @tc.name: IsAppRunning_3Arg_007
 * @tc.desc: Test IsAppRunning (3-arg) with appCloneIndex == 0 and GetBundleInfoV9 fails.
 *           Should return ERR_APP_CLONE_INDEX_INVALID.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_3Arg_007, TestSize.Level1)
{
    AAFwk::MyStatus::GetInstance().getCallingUid_ = TEST_USER_ID_100 * BASE_USER_RANGE;
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_INVALID_VALUE;
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, 0, isRunning);
    EXPECT_EQ(result, AAFwk::ERR_APP_CLONE_INDEX_INVALID);
    AAFwk::MyStatus::GetInstance().getBundleInfoV9_ = ERR_OK;
}

/**
 * @tc.name: IsAppRunning_3Arg_008
 * @tc.desc: Test IsAppRunning (3-arg) with a valid clone index (1).
 *           Should call GetCloneBundleInfo and succeed.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_3Arg_008, TestSize.Level1)
{
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, VALID_CLONE_INDEX, isRunning);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getDualModeBundleInfoCall_, 0);
}

/**
 * @tc.name: IsAppRunning_3Arg_009
 * @tc.desc: Test IsAppRunning (3-arg) with PC_TABLET_INDEX.
 *           Should pass validation and proceed.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_3Arg_009, TestSize.Level1)
{
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, PC_TABLET_INDEX_VAL, isRunning);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getDualModeBundleInfoCall_, 0);
}

/**
 * @tc.name: IsAppRunning_3Arg_010
 * @tc.desc: Test IsAppRunning (3-arg) with valid clone index and GetCloneBundleInfo fails.
 *           Should return ERR_APP_CLONE_INDEX_INVALID.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_3Arg_010, TestSize.Level1)
{
    AAFwk::MyStatus::GetInstance().getCloneBundleInfo_ = ERR_INVALID_VALUE;
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, VALID_CLONE_INDEX, isRunning);
    EXPECT_EQ(result, AAFwk::ERR_APP_CLONE_INDEX_INVALID);
    AAFwk::MyStatus::GetInstance().getCloneBundleInfo_ = ERR_OK;
}

/**
 * @tc.name: IsAppRunning_4Arg_001
 * @tc.desc: Test IsAppRunning (4-arg) with caller that is not foundation or file system service.
 *           Should return ERR_PERMISSION_DENIED.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_4Arg_001, TestSize.Level1)
{
    AAFwk::MyStatus::GetInstance().getCallingUid_ = TEST_USER_ID_100 * BASE_USER_RANGE;
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, 0, TEST_USER_ID_100, isRunning);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: IsAppRunning_4Arg_002
 * @tc.desc: Test IsAppRunning (4-arg) with file system service UID.
 *           Should pass caller check and proceed.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_4Arg_002, TestSize.Level1)
{
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FILE_SYSTEM_SERVICE_UID;
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, 0, TEST_USER_ID_100, isRunning);
    EXPECT_EQ(result, ERR_OK);
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
}

/**
 * @tc.name: IsAppRunning_4Arg_003
 * @tc.desc: Test IsAppRunning (4-arg) with permission denied (verifyRunningInfoPerm_ false).
 *           Should return ERR_PERMISSION_DENIED.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_4Arg_003, TestSize.Level1)
{
    AAFwk::MyStatus::GetInstance().verifyRunningInfoPerm_ = false;
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, 0, TEST_USER_ID_100, isRunning);
    EXPECT_EQ(result, ERR_PERMISSION_DENIED);
}

/**
 * @tc.name: IsAppRunning_4Arg_004
 * @tc.desc: Test IsAppRunning (4-arg) with invalid appCloneIndex (negative).
 *           Should return ERR_APP_CLONE_INDEX_INVALID.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_4Arg_004, TestSize.Level1)
{
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, INVALID_CLONE_INDEX_NEG,
        TEST_USER_ID_100, isRunning);
    EXPECT_EQ(result, AAFwk::ERR_APP_CLONE_INDEX_INVALID);
}

/**
 * @tc.name: IsAppRunning_4Arg_005
 * @tc.desc: Test IsAppRunning (4-arg) with invalid appCloneIndex (mid-range).
 *           Should return ERR_APP_CLONE_INDEX_INVALID.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_4Arg_005, TestSize.Level1)
{
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, INVALID_CLONE_INDEX_MID,
        TEST_USER_ID_100, isRunning);
    EXPECT_EQ(result, AAFwk::ERR_APP_CLONE_INDEX_INVALID);
}

/**
 * @tc.name: IsAppRunning_4Arg_006
 * @tc.desc: Test IsAppRunning (4-arg) with invalid userId (< 0).
 *           Should return INVALID_PARAMETERS_ERR.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_4Arg_006, TestSize.Level1)
{
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, 0, -1, isRunning);
    EXPECT_EQ(result, AAFwk::INVALID_PARAMETERS_ERR);
}

/**
 * @tc.name: IsAppRunning_4Arg_007
 * @tc.desc: Test IsAppRunning (4-arg) with appCloneIndex == 0 and bundleMgrHelper null.
 *           Should return ERR_INVALID_OPERATION.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_4Arg_007, TestSize.Level1)
{
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = nullptr;
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, 0, TEST_USER_ID_100, isRunning);
    EXPECT_EQ(result, ERR_INVALID_OPERATION);
    AAFwk::MyStatus::GetInstance().getBundleManagerHelper_ = std::make_shared<BundleMgrHelper>();
}

/**
 * @tc.name: IsAppRunning_4Arg_008
 * @tc.desc: Test IsAppRunning (4-arg) with appCloneIndex == 0 (normal app).
 *           Should call GetValidUserId and GetDualModeBundleInfo, then succeed.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_4Arg_008, TestSize.Level1)
{
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, 0, TEST_USER_ID_100, isRunning);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getDualModeBundleInfoCall_, 1);
}

/**
 * @tc.name: IsAppRunning_4Arg_009
 * @tc.desc: Test IsAppRunning (4-arg) with a valid clone index (1).
 *           Should not call GetDualModeBundleInfo and succeed.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_4Arg_009, TestSize.Level1)
{
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, VALID_CLONE_INDEX,
        TEST_USER_ID_100, isRunning);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getDualModeBundleInfoCall_, 0);
}

/**
 * @tc.name: IsAppRunning_4Arg_010
 * @tc.desc: Test IsAppRunning (4-arg) with PC_TABLET_INDEX.
 *           Should pass validation and succeed without calling GetDualModeBundleInfo.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_4Arg_010, TestSize.Level1)
{
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, PC_TABLET_INDEX_VAL,
        TEST_USER_ID_100, isRunning);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getDualModeBundleInfoCall_, 0);
}

/**
 * @tc.name: IsAppRunning_4Arg_011
 * @tc.desc: Test IsAppRunning (4-arg) with appCloneIndex == 0 and userId == DEFAULT_INVALID_VALUE.
 *           GetValidUserId should resolve to caller's userId via UID.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_4Arg_011, TestSize.Level1)
{
    AAFwk::MyStatus::GetInstance().getCallingUid_ = TEST_USER_ID_100 * BASE_USER_RANGE;
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, 0,
        AAFwk::DEFAULT_INVALID_VALUE, isRunning);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getDualModeBundleInfoCall_, 1);
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
}

/**
 * @tc.name: IsAppRunning_4Arg_012
 * @tc.desc: Test IsAppRunning (4-arg) with appCloneIndex == 0, userId == DEFAULT_INVALID_VALUE,
 *           and caller UID maps to U0. GetValidUserId should resolve to foreground user.
 * @tc.type: FUNC
 */
HWTEST_F(AppQueryHelperTest, IsAppRunning_4Arg_012, TestSize.Level1)
{
    AAFwk::MyStatus::GetInstance().getCallingUid_ = 0;
    UserController::GetInstance().SetForegroundUserId(TEST_USER_ID_101, TEST_DEFAULT_DISPLAY_ID);
    bool isRunning = false;
    int32_t result = appQueryHelper_->IsAppRunning(TEST_BUNDLE_NAME, 0,
        AAFwk::DEFAULT_INVALID_VALUE, isRunning);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(AAFwk::MyStatus::GetInstance().getDualModeBundleInfoCall_, 1);
    UserController::GetInstance().SetForegroundUserId(TEST_USER_ID_100, TEST_DEFAULT_DISPLAY_ID);
    AAFwk::MyStatus::GetInstance().getCallingUid_ = FOUNDATION_UID;
}
}  // namespace AppExecFwk
}  // namespace OHOS
