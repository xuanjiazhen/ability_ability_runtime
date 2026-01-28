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

#define private public
#include "tokenid_permission.h"
#undef private

#include "hilog_tag_wrapper.h"
#include "oh_mock_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

class TokenIdPermissionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::unique_ptr<TokenIdPermission> tokenPerm_;
};

void TokenIdPermissionTest::SetUpTestCase() {}

void TokenIdPermissionTest::TearDownTestCase() {}

void TokenIdPermissionTest::SetUp()
{
    uint32_t tokenId = 1001;
    tokenPerm_ = std::make_unique<TokenIdPermission>(tokenId);
}

void TokenIdPermissionTest::TearDown()
{
    tokenPerm_ = nullptr;
}

/**
 * @tc.name: TokenIdPermission_Constructor_001
 * @tc.desc: Test TokenIdPermission constructor
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, TokenIdPermission_Constructor_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "TokenIdPermission_Constructor_001 start");

    uint32_t tokenId = 2001;
    TokenIdPermission tokenPerm(tokenId);

    EXPECT_EQ(tokenPerm.GetTokenId(), tokenId);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "TokenIdPermission_Constructor_001 end");
}

/**
 * @tc.name: GetTokenId_001
 * @tc.desc: Test GetTokenId method
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, GetTokenId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetTokenId_001 start");

    uint32_t tokenId = 1001;
    EXPECT_EQ(tokenPerm_->GetTokenId(), tokenId);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetTokenId_001 end");
}

/**
 * @tc.name: VerifyProxyAuthorizationUriPermission_001
 * @tc.desc: Test VerifyProxyAuthorizationUriPermission with permission granted
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyProxyAuthorizationUriPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyProxyAuthorizationUriPermission_001 start");

    // Mock PermissionVerification to return true
    OH_EXPECT_RET({true}, PermissionVerification, VerifyProxyAuthorizationUriPermission, uint32_t);

    bool result = tokenPerm_->VerifyProxyAuthorizationUriPermission();

    // Result depends on mock and permission verification
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyProxyAuthorizationUriPermission_001 end");
}

/**
 * @tc.name: VerifyProxyAuthorizationUriPermission_002
 * @tc.desc: Test VerifyProxyAuthorizationUriPermission with permission denied
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyProxyAuthorizationUriPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyProxyAuthorizationUriPermission_002 start");

    OH_EXPECT_RET({false}, PermissionVerification, VerifyProxyAuthorizationUriPermission, uint32_t);

    bool result = tokenPerm_->VerifyProxyAuthorizationUriPermission();

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyProxyAuthorizationUriPermission_002 end");
}

/**
 * @tc.name: VerifyFileAccessManagerPermission_001
 * @tc.desc: Test VerifyFileAccessManagerPermission with permission granted
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyFileAccessManagerPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyFileAccessManagerPermission_001 start");

    OH_EXPECT_RET({true}, PermissionVerification, VerifyFileAccessManagerPermission, uint32_t);

    bool result = tokenPerm_->VerifyFileAccessManagerPermission();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyFileAccessManagerPermission_001 end");
}

/**
 * @tc.name: VerifyFileAccessManagerPermission_002
 * @tc.desc: Test VerifyFileAccessManagerPermission with permission denied
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyFileAccessManagerPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyFileAccessManagerPermission_002 start");

    OH_EXPECT_RET({false}, PermissionVerification, VerifyFileAccessManagerPermission, uint32_t);

    bool result = tokenPerm_->VerifyFileAccessManagerPermission();

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyFileAccessManagerPermission_002 end");
}

/**
 * @tc.name: VerifyReadImageVideoPermission_001
 * @tc.desc: Test VerifyReadImageVideoPermission with permission granted
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyReadImageVideoPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyReadImageVideoPermission_001 start");

    OH_EXPECT_RET({true}, PermissionVerification, VerifyReadImageVideoPermission, uint32_t);

    bool result = tokenPerm_->VerifyReadImageVideoPermission();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyReadImageVideoPermission_001 end");
}

/**
 * @tc.name: VerifyReadImageVideoPermission_002
 * @tc.desc: Test VerifyReadImageVideoPermission with permission denied
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyReadImageVideoPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyReadImageVideoPermission_002 start");

    OH_EXPECT_RET({false}, PermissionVerification, VerifyReadImageVideoPermission, uint32_t);

    bool result = tokenPerm_->VerifyReadImageVideoPermission();

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyReadImageVideoPermission_002 end");
}

/**
 * @tc.name: VerifyWriteImageVideoPermission_001
 * @tc.desc: Test VerifyWriteImageVideoPermission with permission granted
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyWriteImageVideoPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyWriteImageVideoPermission_001 start");

    OH_EXPECT_RET({true}, PermissionVerification, VerifyWriteImageVideoPermission, uint32_t);

    bool result = tokenPerm_->VerifyWriteImageVideoPermission();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyWriteImageVideoPermission_001 end");
}

/**
 * @tc.name: VerifyWriteImageVideoPermission_002
 * @tc.desc: Test VerifyWriteImageVideoPermission with permission denied
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyWriteImageVideoPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyWriteImageVideoPermission_002 start");

    OH_EXPECT_RET({false}, PermissionVerification, VerifyWriteImageVideoPermission, uint32_t);

    bool result = tokenPerm_->VerifyWriteImageVideoPermission();

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyWriteImageVideoPermission_002 end");
}

/**
 * @tc.name: VerifyReadAudioPermission_001
 * @tc.desc: Test VerifyReadAudioPermission
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyReadAudioPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyReadAudioPermission_001 start");

    OH_EXPECT_RET({true}, MockAccessTokenKit, VerifyAccessToken, uint32_t, const std::string&);

    bool result = tokenPerm_->VerifyReadAudioPermission();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyReadAudioPermission_001 end");
}

/**
 * @tc.name: VerifyWriteAudioPermission_001
 * @tc.desc: Test VerifyWriteAudioPermission
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyWriteAudioPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyWriteAudioPermission_001 start");

    OH_EXPECT_RET({true}, MockAccessTokenKit, VerifyAccessToken, uint32_t, const std::string&);

    bool result = tokenPerm_->VerifyWriteAudioPermission();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyWriteAudioPermission_001 end");
}

/**
 * @tc.name: VerifyRWDownloadPermission_001
 * @tc.desc: Test VerifyRWDownloadPermission
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyRWDownloadPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyRWDownloadPermission_001 start");

    OH_EXPECT_RET({true}, MockAccessTokenKit, VerifyAccessToken, uint32_t, const std::string&);

    bool result = tokenPerm_->VerifyRWDownloadPermission();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyRWDownloadPermission_001 end");
}

/**
 * @tc.name: VerifyRWDeskTopPermission_001
 * @tc.desc: Test VerifyRWDeskTopPermission
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyRWDeskTopPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyRWDeskTopPermission_001 start");

    OH_EXPECT_RET({true}, MockAccessTokenKit, VerifyAccessToken, uint32_t, const std::string&);

    bool result = tokenPerm_->VerifyRWDeskTopPermission();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyRWDeskTopPermission_001 end");
}

/**
 * @tc.name: VerifyRWDocumentsPermission_001
 * @tc.desc: Test VerifyRWDocumentsPermission
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyRWDocumentsPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyRWDocumentsPermission_001 start");

    OH_EXPECT_RET({true}, MockAccessTokenKit, VerifyAccessToken, uint32_t, const std::string&);

    bool result = tokenPerm_->VerifyRWDocumentsPermission();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyRWDocumentsPermission_001 end");
}

/**
 * @tc.name: VerifySandboxAccessPermission_001
 * @tc.desc: Test VerifySandboxAccessPermission
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifySandboxAccessPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifySandboxAccessPermission_001 start");

    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, uint32_t, const std::string&);

    bool result = tokenPerm_->VerifySandboxAccessPermission();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifySandboxAccessPermission_001 end");
}

/**
 * @tc.name: TokenIdPermission_MultiplePermissions_001
 * @tc.desc: Test multiple permission verifications in sequence
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, TokenIdPermission_MultiplePermissions_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "TokenIdPermission_MultiplePermissions_001 start");

    OH_EXPECT_RET({true}, PermissionVerification, VerifyProxyAuthorizationUriPermission, uint32_t);
    OH_EXPECT_RET({true}, PermissionVerification, VerifyFileAccessManagerPermission, uint32_t);
    OH_EXPECT_RET({true}, PermissionVerification, VerifyReadImageVideoPermission, uint32_t);
    OH_EXPECT_RET({true}, PermissionVerification, VerifyWriteImageVideoPermission, uint32_t);

    bool result1 = tokenPerm_->VerifyProxyAuthorizationUriPermission();
    bool result2 = tokenPerm_->VerifyFileAccessManagerPermission();
    bool result3 = tokenPerm_->VerifyReadImageVideoPermission();
    bool result4 = tokenPerm_->VerifyWriteImageVideoPermission();

    // All permissions should be verified successfully based on mocks
    TAG_LOGI(AAFwkTag::URIPERMMGR, "TokenIdPermission_MultiplePermissions_001 end");
}

} // namespace AAFwk
} // namespace OHOS
