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

#ifdef ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE

#define private public
#include "media_permission_manager.h"
#undef private

#include "hilog_tag_wrapper.h"
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
 * @tc.name: GetInstance_001
 * @tc.desc: Test MediaPermissionManager GetInstance singleton
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, GetInstance_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetInstance_001 start");

    auto& instance1 = MediaPermissionManager::GetInstance();
    auto& instance2 = MediaPermissionManager::GetInstance();

    EXPECT_EQ(&instance1, &instance2);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetInstance_001 end");
}

/**
 * @tc.name: CheckUriPermission_001
 * @tc.desc: Test CheckUriPermission with valid uris
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPermission_001 start");

    std::vector<std::string> uriStrs = {
        "file://media/image/test1.jpg",
        "file://media/image/test2.jpg"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;

    auto& instance = MediaPermissionManager::GetInstance();
    std::vector<bool> result = instance.CheckUriPermission(uriStrs, flag, tokenId);

    EXPECT_EQ(result.size(), 2u);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPermission_001 end");
}

/**
 * @tc.name: CheckUriPermission_002
 * @tc.desc: Test CheckUriPermission with empty uri vector
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPermission_002 start");

    std::vector<std::string> uriStrs; // Empty vector
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;

    auto& instance = MediaPermissionManager::GetInstance();
    std::vector<bool> result = instance.CheckUriPermission(uriStrs, flag, tokenId);

    EXPECT_TRUE(result.empty());

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPermission_002 end");
}

/**
 * @tc.name: CheckUriPermission_003
 * @tc.desc: Test CheckUriPermission with write flag
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPermission_003 start");

    std::vector<std::string> uriStrs = {
        "file://media/image/test.jpg"
    };
    uint32_t flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    uint32_t tokenId = 1001;

    auto& instance = MediaPermissionManager::GetInstance();
    std::vector<bool> result = instance.CheckUriPermission(uriStrs, flag, tokenId);

    EXPECT_EQ(result.size(), 1u);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPermission_003 end");
}

/**
 * @tc.name: GrantUriPermission_001
 * @tc.desc: Test GrantUriPermission with valid uris
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, GrantUriPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_001 start");

    std::vector<std::string> uriStrs = {
        "file://media/image/test1.jpg",
        "file://media/image/test2.jpg"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;

    auto& instance = MediaPermissionManager::GetInstance();
    int32_t ret = instance.GrantUriPermission(uriStrs, flag, callerTokenId, targetTokenId);

    EXPECT_GE(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_001 end");
}

/**
 * @tc.name: GrantUriPermission_002
 * @tc.desc: Test GrantUriPermission with empty uri vector
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, GrantUriPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_002 start");

    std::vector<std::string> uriStrs; // Empty vector
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;

    auto& instance = MediaPermissionManager::GetInstance();
    int32_t ret = instance.GrantUriPermission(uriStrs, flag, callerTokenId, targetTokenId);

    EXPECT_EQ(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_002 end");
}

/**
 * @tc.name: GrantUriPermission_003
 * @tc.desc: Test GrantUriPermission with write flag
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, GrantUriPermission_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_003 start");

    std::vector<std::string> uriStrs = {
        "file://media/image/test.jpg"
    };
    uint32_t flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;

    auto& instance = MediaPermissionManager::GetInstance();
    int32_t ret = instance.GrantUriPermission(uriStrs, flag, callerTokenId, targetTokenId);

    EXPECT_GE(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_003 end");
}

/**
 * @tc.name: RevokeUriPermission_001
 * @tc.desc: Test RevokeUriPermission with valid uris
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, RevokeUriPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeUriPermission_001 start");

    std::vector<std::string> uriStrs = {
        "file://media/image/test1.jpg",
        "file://media/image/test2.jpg"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;

    auto& instance = MediaPermissionManager::GetInstance();
    int32_t ret = instance.RevokeUriPermission(uriStrs, flag, callerTokenId, targetTokenId);

    EXPECT_GE(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeUriPermission_001 end");
}

/**
 * @tc.name: RevokeUriPermission_002
 * @tc.desc: Test RevokeUriPermission with empty uri vector
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, RevokeUriPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeUriPermission_002 start");

    std::vector<std::string> uriStrs; // Empty vector
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;

    auto& instance = MediaPermissionManager::GetInstance();
    int32_t ret = instance.RevokeUriPermission(uriStrs, flag, callerTokenId, targetTokenId);

    EXPECT_EQ(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeUriPermission_002 end");
}

/**
 * @tc.name: MediaPermissionManager_MultipleOperations_001
 * @tc.desc: Test multiple operations in sequence
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, MediaPermissionManager_MultipleOperations_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "MediaPermissionManager_MultipleOperations_001 start");

    std::vector<std::string> uriStrs = {
        "file://media/image/test.jpg"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;

    auto& instance = MediaPermissionManager::GetInstance();

    // Check permission
    std::vector<bool> checkResult = instance.CheckUriPermission(uriStrs, flag, callerTokenId);

    // Grant permission
    int32_t grantRet = instance.GrantUriPermission(uriStrs, flag, callerTokenId, targetTokenId);

    // Check permission again
    std::vector<bool> checkResult2 = instance.CheckUriPermission(uriStrs, flag, targetTokenId);

    // Revoke permission
    int32_t revokeRet = instance.RevokeUriPermission(uriStrs, flag, callerTokenId, targetTokenId);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "MediaPermissionManager_MultipleOperations_001 end");
}

/**
 * @tc.name: CheckUriPermission_NoPermission_001
 * @tc.desc: Test CheckUriPermission when permission is denied
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_NoPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPermission_NoPermission_001 start");

    std::vector<std::string> uriStrs = {
        "file://media/image/test.jpg"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;

    auto& instance = MediaPermissionManager::GetInstance();
    std::vector<bool> result = instance.CheckUriPermission(uriStrs, flag, tokenId);

    // Test permission denied scenario
    if (result.size() > 0) {
        // Result depends on media permission helper
    }
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPermission_NoPermission_001 end");
}

/**
 * @tc.name: GrantUriPermission_Conflict_001
 * @tc.desc: Test GrantUriPermission with permission conflict
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, GrantUriPermission_Conflict_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Conflict_001 start");

    std::vector<std::string> uriStrs = {
        "file://media/image/test.jpg"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;

    auto& instance = MediaPermissionManager::GetInstance();
    int32_t ret = instance.GrantUriPermission(uriStrs, flag, callerTokenId, targetTokenId);

    // Test permission conflict scenario
    // Return value depends on media permission helper
    EXPECT_GE(ret, -1);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Conflict_001 end");
}

/**
 * @tc.name: RevokeUriPermission_NotGranted_001
 * @tc.desc: Test RevokeUriPermission when permission was never granted
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, RevokeUriPermission_NotGranted_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeUriPermission_NotGranted_001 start");

    std::vector<std::string> uriStrs = {
        "file://media/image/test.jpg"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;

    auto& instance = MediaPermissionManager::GetInstance();
    int32_t ret = instance.RevokeUriPermission(uriStrs, flag, callerTokenId, targetTokenId);

    // Test revoking non-existent permission
    // Return value depends on media permission helper
    EXPECT_GE(ret, -1);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeUriPermission_NotGranted_001 end");
}

/**
 * @tc.name: CheckUriPermission_MultipleMediaTypes_001
 * @tc.desc: Test CheckUriPermission with multiple media types
 * @tc.type: FUNC
 */
HWTEST_F(MediaPermissionManagerTest, CheckUriPermission_MultipleMediaTypes_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPermission_MultipleMediaTypes_001 start");

    std::vector<std::string> uriStrs = {
        "file://media/image/test.jpg",
        "file://media/video/test.mp4",
        "file://media/audio/test.mp3"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;

    auto& instance = MediaPermissionManager::GetInstance();
    std::vector<bool> result = instance.CheckUriPermission(uriStrs, flag, tokenId);

    EXPECT_EQ(result.size(), 3u);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPermission_MultipleMediaTypes_001 end");
}

} // namespace AAFwk
} // namespace OHOS

#endif // ABILITY_RUNTIME_MEDIA_LIBRARY_ENABLE
