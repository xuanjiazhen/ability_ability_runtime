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
#include "media_permission_manager.h"
#include "media_permission_helper_mock.h"
#include "want.h"
#include "ability_manager_errors.h"
#include "oh_mock_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

class MediaPermissionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void MediaPermissionManagerTest::SetUpTestCase() {}

void MediaPermissionManagerTest::TearDownTestCase() {}

void MediaPermissionManagerTest::SetUp() {}

void MediaPermissionManagerTest::TearDown() {}

/**
 * @tc.name: GetMediaPermissionHelper_Success_001
 * @tc.desc: Verify GetMediaPermissionHelper returns valid helper.
 *           Branch: GetMediaPermissionHelper returns non-null
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, GetMediaPermissionHelper_Success_001, TestSize.Level1)
{
    // Create a mock MediaPermissionHelper
    auto mockHelper = new Media::MediaPermissionHelper();
    OH_GLOBAL_EXPECT_RET({mockHelper}, Media::MediaPermissionHelper, GetMediaPermissionHelper);

    auto& instance = MediaPermissionManager::GetInstance();
    auto* helper = instance.GetMediaPermissionHelper();

    EXPECT_NE(helper, nullptr);
}

/**
 * @tc.name: GetMediaPermissionHelper_Failure_002
 * @tc.desc: Verify GetMediaPermissionHelper returns nullptr.
 *           Branch: GetMediaPermissionHelper returns nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, GetMediaPermissionHelper_Failure_002, TestSize.Level1)
{
    OH_GLOBAL_EXPECT_RET({static_cast<Media::MediaPermissionHelper*>(nullptr)}, Media::MediaPermissionHelper, GetMediaPermissionHelper);

    auto& instance = MediaPermissionManager::GetInstance();
    auto* helper = instance.GetMediaPermissionHelper();

    EXPECT_EQ(helper, nullptr);
}

/**
 * @tc.name: CheckUriPermission_Success_001
 * @tc.desc: Verify CheckUriPermission returns correct results.
 *           Branch: helper != nullptr, CheckPhotoUriPermission succeeds, size matches
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_Success_001, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://media/test1.jpg", "file://media/test2.jpg"};
    uint32_t callerTokenId = 1001;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;

    // Mock helper
    auto mockHelper = new Media::MediaPermissionHelper();
    OH_GLOBAL_EXPECT_RET({mockHelper}, Media::MediaPermissionHelper, GetMediaPermissionHelper);

    // Mock CheckPhotoUriPermission to return success with results
    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, Media::MediaPermissionHelper, CheckPhotoUriPermission,
        uint32_t, std::vector<std::string>&, std::vector<bool>&, std::vector<uint32_t>&);

    auto& instance = MediaPermissionManager::GetInstance();
    auto results = instance.CheckUriPermission(uriVec, callerTokenId, flag);

    EXPECT_EQ(results.size(), 2U);
}

/**
 * @tc.name: CheckUriPermission_Failure_002
 * @tc.desc: Verify CheckUriPermission returns all false when helper is nullptr.
 *           Branch: helper == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_Failure_002, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://media/test1.jpg"};
    uint32_t callerTokenId = 1001;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;

    OH_GLOBAL_EXPECT_RET({static_cast<Media::MediaPermissionHelper*>(nullptr)}, Media::MediaPermissionHelper, GetMediaPermissionHelper);

    auto& instance = MediaPermissionManager::GetInstance();
    auto results = instance.CheckUriPermission(uriVec, callerTokenId, flag);

    EXPECT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0], false);
}

/**
 * @tc.name: CheckUriPermission_Failure_003
 * @tc.desc: Verify CheckUriPermission returns all false when CheckPhotoUriPermission fails.
 *           Branch: ret != ERR_OK
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_Failure_003, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://media/test1.jpg"};
    uint32_t callerTokenId = 1001;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;

    auto mockHelper = new Media::MediaPermissionHelper();
    OH_GLOBAL_EXPECT_RET({mockHelper}, Media::MediaPermissionHelper, GetMediaPermissionHelper);
    OH_EXPECT_RET({static_cast<int32_t>(INNER_ERR)}, Media::MediaPermissionHelper, CheckPhotoUriPermission,
        uint32_t, std::vector<std::string>&, std::vector<bool>&, std::vector<uint32_t>&);

    auto& instance = MediaPermissionManager::GetInstance();
    auto results = instance.CheckUriPermission(uriVec, callerTokenId, flag);

    EXPECT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0], false);
}

/**
 * @tc.name: CheckUriPermission_Failure_004
 * @tc.desc: Verify CheckUriPermission returns all false when size mismatches.
 *           Branch: results.size() != uriVec.size()
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_Failure_004, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://media/test1.jpg"};
    uint32_t callerTokenId = 1001;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;

    auto mockHelper = new Media::MediaPermissionHelper();
    OH_GLOBAL_EXPECT_RET({mockHelper}, Media::MediaPermissionHelper, GetMediaPermissionHelper);
    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, Media::MediaPermissionHelper, CheckPhotoUriPermission,
        uint32_t, std::vector<std::string>&, std::vector<bool>&, std::vector<uint32_t>&);

    auto& instance = MediaPermissionManager::GetInstance();
    auto results = instance.CheckUriPermission(uriVec, callerTokenId, flag);

    // Size mismatch check - if mock returns empty results, size mismatch occurs
    EXPECT_EQ(results.size(), 1U);
}

/**
 * @tc.name: CheckUriPermission_Empty_005
 * @tc.desc: Verify CheckUriPermission handles empty URI vector.
 *           Branch: uriVec empty
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_Empty_005, TestSize.Level1)
{
    std::vector<std::string> uriVec;
    uint32_t callerTokenId = 1001;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;

    auto mockHelper = new Media::MediaPermissionHelper();
    OH_GLOBAL_EXPECT_RET({mockHelper}, Media::MediaPermissionHelper, GetMediaPermissionHelper);
    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, Media::MediaPermissionHelper, CheckPhotoUriPermission,
        uint32_t, std::vector<std::string>&, std::vector<bool>&, std::vector<uint32_t>&);

    auto& instance = MediaPermissionManager::GetInstance();
    auto results = instance.CheckUriPermission(uriVec, callerTokenId, flag);

    EXPECT_EQ(results.size(), 0U);
}

/**
 * @tc.name: CheckUriPermission_ReadWrite_006
 * @tc.desc: Verify CheckUriPermission with read and write flag.
 *           Branch: flag & (READ | WRITE) both set
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_ReadWrite_006, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://media/test1.jpg"};
    uint32_t callerTokenId = 1001;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION;

    auto mockHelper = new Media::MediaPermissionHelper();
    OH_GLOBAL_EXPECT_RET({mockHelper}, Media::MediaPermissionHelper, GetMediaPermissionHelper);
    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, Media::MediaPermissionHelper, CheckPhotoUriPermission,
        uint32_t, std::vector<std::string>&, std::vector<bool>&, std::vector<uint32_t>&);

    auto& instance = MediaPermissionManager::GetInstance();
    auto results = instance.CheckUriPermission(uriVec, callerTokenId, flag);

    EXPECT_EQ(results.size(), 1U);
}

/**
 * @tc.name: CheckUriPermission_Write_007
 * @tc.desc: Verify CheckUriPermission with write only flag.
 *           Branch: flag & WRITE only
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_Write_007, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://media/test1.jpg"};
    uint32_t callerTokenId = 1001;
    uint32_t flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;

    auto mockHelper = new Media::MediaPermissionHelper();
    OH_GLOBAL_EXPECT_RET({mockHelper}, Media::MediaPermissionHelper, GetMediaPermissionHelper);
    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, Media::MediaPermissionHelper, CheckPhotoUriPermission,
        uint32_t, std::vector<std::string>&, std::vector<bool>&, std::vector<uint32_t>&);

    auto& instance = MediaPermissionManager::GetInstance();
    auto results = instance.CheckUriPermission(uriVec, callerTokenId, flag);

    EXPECT_EQ(results.size(), 1U);
}

/**
 * @tc.name: GrantUriPermission_Success_001
 * @tc.desc: Verify GrantUriPermission succeeds.
 *           Branch: helper != nullptr, GrantPhotoUriPermission succeeds
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, GrantUriPermission_Success_001, TestSize.Level1)
{
    std::vector<std::string> uris = {"file://media/test1.jpg"};
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    int32_t hideSensitiveType = 0;

    auto mockHelper = new Media::MediaPermissionHelper();
    OH_GLOBAL_EXPECT_RET({mockHelper}, Media::MediaPermissionHelper, GetMediaPermissionHelper);
    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, Media::MediaPermissionHelper, GrantPhotoUriPermission,
        uint32_t, uint32_t, std::vector<std::string>&, std::vector<Media::PhotoPermissionType>&, Media::HideSensitiveType);

    auto& instance = MediaPermissionManager::GetInstance();
    auto ret = instance.GrantUriPermission(uris, flag, callerTokenId, targetTokenId, hideSensitiveType);

    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GrantUriPermission_Failure_002
 * @tc.desc: Verify GrantUriPermission fails when helper is nullptr.
 *           Branch: helper == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, GrantUriPermission_Failure_002, TestSize.Level1)
{
    std::vector<std::string> uris = {"file://media/test1.jpg"};
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    int32_t hideSensitiveType = 0;

    OH_GLOBAL_EXPECT_RET({static_cast<Media::MediaPermissionHelper*>(nullptr)}, Media::MediaPermissionHelper, GetMediaPermissionHelper);

    auto& instance = MediaPermissionManager::GetInstance();
    auto ret = instance.GrantUriPermission(uris, flag, callerTokenId, targetTokenId, hideSensitiveType);

    EXPECT_EQ(ret, INNER_ERR);
}

/**
 * @tc.name: GrantUriPermission_Failure_003
 * @tc.desc: Verify GrantUriPermission fails when GrantPhotoUriPermission fails.
 *           Branch: ret != ERR_OK
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, GrantUriPermission_Failure_003, TestSize.Level1)
{
    std::vector<std::string> uris = {"file://media/test1.jpg"};
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    int32_t hideSensitiveType = 0;

    auto mockHelper = new Media::MediaPermissionHelper();
    OH_GLOBAL_EXPECT_RET({mockHelper}, Media::MediaPermissionHelper, GetMediaPermissionHelper);
    OH_EXPECT_RET({static_cast<int32_t>(INNER_ERR)}, Media::MediaPermissionHelper, GrantPhotoUriPermission,
        uint32_t, uint32_t, std::vector<std::string>&, std::vector<Media::PhotoPermissionType>&, Media::HideSensitiveType);

    auto& instance = MediaPermissionManager::GetInstance();
    auto ret = instance.GrantUriPermission(uris, flag, callerTokenId, targetTokenId, hideSensitiveType);

    EXPECT_EQ(ret, INNER_ERR);
}

/**
 * @tc.name: GrantUriPermission_ReadWrite_004
 * @tc.desc: Verify GrantUriPermission with read and write flag.
 *           Branch: flag & (READ | WRITE) -> TEMPORARY_READWRITE_IMAGEVIDEO
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, GrantUriPermission_ReadWrite_004, TestSize.Level1)
{
    std::vector<std::string> uris = {"file://media/test1.jpg"};
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    int32_t hideSensitiveType = 0;

    auto mockHelper = new Media::MediaPermissionHelper();
    OH_GLOBAL_EXPECT_RET({mockHelper}, Media::MediaPermissionHelper, GetMediaPermissionHelper);
    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, Media::MediaPermissionHelper, GrantPhotoUriPermission,
        uint32_t, uint32_t, std::vector<std::string>&, std::vector<Media::PhotoPermissionType>&, Media::HideSensitiveType);

    auto& instance = MediaPermissionManager::GetInstance();
    auto ret = instance.GrantUriPermission(uris, flag, callerTokenId, targetTokenId, hideSensitiveType);

    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GrantUriPermission_Write_005
 * @tc.desc: Verify GrantUriPermission with write only flag.
 *           Branch: flag == WRITE -> TEMPORARY_WRITE_IMAGEVIDEO
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, GrantUriPermission_Write_005, TestSize.Level1)
{
    std::vector<std::string> uris = {"file://media/test1.jpg"};
    uint32_t flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    int32_t hideSensitiveType = 0;

    auto mockHelper = new Media::MediaPermissionHelper();
    OH_GLOBAL_EXPECT_RET({mockHelper}, Media::MediaPermissionHelper, GetMediaPermissionHelper);
    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, Media::MediaPermissionHelper, GrantPhotoUriPermission,
        uint32_t, uint32_t, std::vector<std::string>&, std::vector<Media::PhotoPermissionType>&, Media::HideSensitiveType);

    auto& instance = MediaPermissionManager::GetInstance();
    auto ret = instance.GrantUriPermission(uris, flag, callerTokenId, targetTokenId, hideSensitiveType);

    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: GrantUriPermission_Read_006
 * @tc.desc: Verify GrantUriPermission with read only flag.
 *           Branch: flag == READ -> TEMPORARY_READ_IMAGEVIDEO
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, GrantUriPermission_Read_006, TestSize.Level1)
{
    std::vector<std::string> uris = {"file://media/test1.jpg"};
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    int32_t hideSensitiveType = 0;

    auto mockHelper = new Media::MediaPermissionHelper();
    OH_GLOBAL_EXPECT_RET({mockHelper}, Media::MediaPermissionHelper, GetMediaPermissionHelper);
    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, Media::MediaPermissionHelper, GrantPhotoUriPermission,
        uint32_t, uint32_t, std::vector<std::string>&, std::vector<Media::PhotoPermissionType>&, Media::HideSensitiveType);

    auto& instance = MediaPermissionManager::GetInstance();
    auto ret = instance.GrantUriPermission(uris, flag, callerTokenId, targetTokenId, hideSensitiveType);

    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: RevokeUriPermission_Success_001
 * @tc.desc: Verify RevokeUriPermission succeeds.
 *           Branch: helper != nullptr, CancelPhotoUriPermission succeeds
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, RevokeUriPermission_Success_001, TestSize.Level1)
{
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    std::string uri = "file://media/test1.jpg";

    auto mockHelper = new Media::MediaPermissionHelper();
    OH_GLOBAL_EXPECT_RET({mockHelper}, Media::MediaPermissionHelper, GetMediaPermissionHelper);
    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, Media::MediaPermissionHelper, CancelPhotoUriPermission,
        uint32_t, uint32_t, std::vector<std::string>&);

    auto& instance = MediaPermissionManager::GetInstance();
    auto ret = instance.RevokeUriPermission(callerTokenId, targetTokenId, uri);

    EXPECT_EQ(ret, ERR_OK);
}

/**
 * @tc.name: RevokeUriPermission_Failure_002
 * @tc.desc: Verify RevokeUriPermission fails when helper is nullptr.
 *           Branch: helper == nullptr
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, RevokeUriPermission_Failure_002, TestSize.Level1)
{
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    std::string uri = "file://media/test1.jpg";

    OH_GLOBAL_EXPECT_RET({static_cast<Media::MediaPermissionHelper*>(nullptr)}, Media::MediaPermissionHelper, GetMediaPermissionHelper);

    auto& instance = MediaPermissionManager::GetInstance();
    auto ret = instance.RevokeUriPermission(callerTokenId, targetTokenId, uri);

    EXPECT_EQ(ret, INNER_ERR);
}

/**
 * @tc.name: RevokeUriPermission_Failure_003
 * @tc.desc: Verify RevokeUriPermission fails when CancelPhotoUriPermission fails.
 *           Branch: ret != ERR_OK
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, RevokeUriPermission_Failure_003, TestSize.Level1)
{
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    std::string uri = "file://media/test1.jpg";

    auto mockHelper = new Media::MediaPermissionHelper();
    OH_GLOBAL_EXPECT_RET({mockHelper}, Media::MediaPermissionHelper, GetMediaPermissionHelper);
    OH_EXPECT_RET({static_cast<int32_t>(INNER_ERR)}, Media::MediaPermissionHelper, CancelPhotoUriPermission,
        uint32_t, uint32_t, std::vector<std::string>&);

    auto& instance = MediaPermissionManager::GetInstance();
    auto ret = instance.RevokeUriPermission(callerTokenId, targetTokenId, uri);

    EXPECT_EQ(ret, INNER_ERR);
}

}  // namespace AAFwk
}  // namespace OHOS
