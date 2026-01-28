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
#include "tokenid_permission.h"
#include "permission_verification_mock.h"
#include "permission_constants.h"
#include "oh_mock_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

constexpr uint32_t TEST_TOKEN_ID = 1001;

class TokenIdPermissionTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void TokenIdPermissionTest::SetUpTestCase() {}

void TokenIdPermissionTest::TearDownTestCase() {}

void TokenIdPermissionTest::SetUp() {}

void TokenIdPermissionTest::TearDown() {}

/**
 * @tc.name: VerifyProxyAuthorizationUriPermission_Granted_001
 * @tc.desc: Verify VerifyProxyAuthorizationUriPermission returns true when permission granted.
 *           Branch: initProxyAuthorizationUriPermission_ == false, VerifyPermissionByTokenId returns true
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyProxyAuthorizationUriPermission_Granted_001, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyProxyAuthorizationUriPermission();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyProxyAuthorizationUriPermission_Denied_002
 * @tc.desc: Verify VerifyProxyAuthorizationUriPermission returns false when permission denied.
 *           Branch: initProxyAuthorizationUriPermission_ == false, VerifyPermissionByTokenId returns false
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyProxyAuthorizationUriPermission_Denied_002, TestSize.Level1)
{
    OH_EXPECT_RET({false}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyProxyAuthorizationUriPermission();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: VerifyProxyAuthorizationUriPermission_Cached_003
 * @tc.desc: Verify VerifyProxyAuthorizationUriPermission returns cached value on second call.
 *           Branch: initProxyAuthorizationUriPermission_ == true, return cached value
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyProxyAuthorizationUriPermission_Cached_003, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    tidp.VerifyProxyAuthorizationUriPermission();  // First call
    bool result = tidp.VerifyProxyAuthorizationUriPermission();  // Second call (cached)

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyFileAccessManagerPermission_Granted_001
 * @tc.desc: Verify VerifyFileAccessManagerPermission returns true when permission granted.
 *           Branch: initFileAccessManagerPermission_ == false, VerifyPermissionByTokenId returns true
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyFileAccessManagerPermission_Granted_001, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyFileAccessManagerPermission();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyFileAccessManagerPermission_Denied_002
 * @tc.desc: Verify VerifyFileAccessManagerPermission returns false when permission denied.
 *           Branch: initFileAccessManagerPermission_ == false, VerifyPermissionByTokenId returns false
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyFileAccessManagerPermission_Denied_002, TestSize.Level1)
{
    OH_EXPECT_RET({false}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyFileAccessManagerPermission();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: VerifyFileAccessManagerPermission_Cached_003
 * @tc.desc: Verify VerifyFileAccessManagerPermission returns cached value on second call.
 *           Branch: initFileAccessManagerPermission_ == true, return cached value
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyFileAccessManagerPermission_Cached_003, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    tidp.VerifyFileAccessManagerPermission();  // First call
    bool result = tidp.VerifyFileAccessManagerPermission();  // Second call (cached)

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyReadImageVideoPermission_Granted_001
 * @tc.desc: Verify VerifyReadImageVideoPermission returns true when permission granted.
 *           Branch: initReadImageVideoPermission_ == false, VerifyPermissionByTokenId returns true
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyReadImageVideoPermission_Granted_001, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyReadImageVideoPermission();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyReadImageVideoPermission_Denied_002
 * @tc.desc: Verify VerifyReadImageVideoPermission returns false when permission denied.
 *           Branch: initReadImageVideoPermission_ == false, VerifyPermissionByTokenId returns false
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyReadImageVideoPermission_Denied_002, TestSize.Level1)
{
    OH_EXPECT_RET({false}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyReadImageVideoPermission();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: VerifyReadImageVideoPermission_Cached_003
 * @tc.desc: Verify VerifyReadImageVideoPermission returns cached value on second call.
 *           Branch: initReadImageVideoPermission_ == true, return cached value
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyReadImageVideoPermission_Cached_003, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    tidp.VerifyReadImageVideoPermission();  // First call
    bool result = tidp.VerifyReadImageVideoPermission();  // Second call (cached)

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyWriteImageVideoPermission_Granted_001
 * @tc.desc: Verify VerifyWriteImageVideoPermission returns true when permission granted.
 *           Branch: initWriteImageVideoPermission_ == false, VerifyPermissionByTokenId returns true
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyWriteImageVideoPermission_Granted_001, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyWriteImageVideoPermission();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyWriteImageVideoPermission_Denied_002
 * @tc.desc: Verify VerifyWriteImageVideoPermission returns false when permission denied.
 *           Branch: initWriteImageVideoPermission_ == false, VerifyPermissionByTokenId returns false
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyWriteImageVideoPermission_Denied_002, TestSize.Level1)
{
    OH_EXPECT_RET({false}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyWriteImageVideoPermission();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: VerifyWriteImageVideoPermission_Cached_003
 * @tc.desc: Verify VerifyWriteImageVideoPermission returns cached value on second call.
 *           Branch: initWriteImageVideoPermission_ == true, return cached value
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyWriteImageVideoPermission_Cached_003, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    tidp.VerifyWriteImageVideoPermission();  // First call
    bool result = tidp.VerifyWriteImageVideoPermission();  // Second call (cached)

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyReadAudioPermission_Granted_001
 * @tc.desc: Verify VerifyReadAudioPermission returns true when permission granted.
 *           Branch: initReadAudioPermission_ == false, VerifyPermissionByTokenId returns true
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyReadAudioPermission_Granted_001, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyReadAudioPermission();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyReadAudioPermission_Denied_002
 * @tc.desc: Verify VerifyReadAudioPermission returns false when permission denied.
 *           Branch: initReadAudioPermission_ == false, VerifyPermissionByTokenId returns false
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyReadAudioPermission_Denied_002, TestSize.Level1)
{
    OH_EXPECT_RET({false}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyReadAudioPermission();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: VerifyReadAudioPermission_Cached_003
 * @tc.desc: Verify VerifyReadAudioPermission returns cached value on second call.
 *           Branch: initReadAudioPermission_ == true, return cached value
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyReadAudioPermission_Cached_003, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    tidp.VerifyReadAudioPermission();  // First call
    bool result = tidp.VerifyReadAudioPermission();  // Second call (cached)

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyWriteAudioPermission_Granted_001
 * @tc.desc: Verify VerifyWriteAudioPermission returns true when permission granted.
 *           Branch: initWriteAudioPermission_ == false, VerifyPermissionByTokenId returns true
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyWriteAudioPermission_Granted_001, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyWriteAudioPermission();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyWriteAudioPermission_Denied_002
 * @tc.desc: Verify VerifyWriteAudioPermission returns false when permission denied.
 *           Branch: initWriteAudioPermission_ == false, VerifyPermissionByTokenId returns false
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyWriteAudioPermission_Denied_002, TestSize.Level1)
{
    OH_EXPECT_RET({false}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyWriteAudioPermission();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: VerifyWriteAudioPermission_Cached_003
 * @tc.desc: Verify VerifyWriteAudioPermission returns cached value on second call.
 *           Branch: initWriteAudioPermission_ == true, return cached value
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyWriteAudioPermission_Cached_003, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    tidp.VerifyWriteAudioPermission();  // First call
    bool result = tidp.VerifyWriteAudioPermission();  // Second call (cached)

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyRWDownloadPermission_Granted_001
 * @tc.desc: Verify VerifyRWDownloadPermission returns true when permission granted.
 *           Branch: initRWDownloadPermission_ == false, VerifyPermissionByTokenId returns true
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyRWDownloadPermission_Granted_001, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyRWDownloadPermission();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyRWDownloadPermission_Denied_002
 * @tc.desc: Verify VerifyRWDownloadPermission returns false when permission denied.
 *           Branch: initRWDownloadPermission_ == false, VerifyPermissionByTokenId returns false
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyRWDownloadPermission_Denied_002, TestSize.Level1)
{
    OH_EXPECT_RET({false}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyRWDownloadPermission();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: VerifyRWDownloadPermission_Cached_003
 * @tc.desc: Verify VerifyRWDownloadPermission returns cached value on second call.
 *           Branch: initRWDownloadPermission_ == true, return cached value
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyRWDownloadPermission_Cached_003, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    tidp.VerifyRWDownloadPermission();  // First call
    bool result = tidp.VerifyRWDownloadPermission();  // Second call (cached)

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyRWDeskTopPermission_Granted_001
 * @tc.desc: Verify VerifyRWDeskTopPermission returns true when permission granted.
 *           Branch: initRWDeskTopPermission_ == false, VerifyPermissionByTokenId returns true
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyRWDeskTopPermission_Granted_001, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyRWDeskTopPermission();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyRWDeskTopPermission_Denied_002
 * @tc.desc: Verify VerifyRWDeskTopPermission returns false when permission denied.
 *           Branch: initRWDeskTopPermission_ == false, VerifyPermissionByTokenId returns false
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyRWDeskTopPermission_Denied_002, TestSize.Level1)
{
    OH_EXPECT_RET({false}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyRWDeskTopPermission();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: VerifyRWDeskTopPermission_Cached_003
 * @tc.desc: Verify VerifyRWDeskTopPermission returns cached value on second call.
 *           Branch: initRWDeskTopPermission_ == true, return cached value
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyRWDeskTopPermission_Cached_003, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    tidp.VerifyRWDeskTopPermission();  // First call
    bool result = tidp.VerifyRWDeskTopPermission();  // Second call (cached)

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyRWDocumentsPermission_Granted_001
 * @tc.desc: Verify VerifyRWDocumentsPermission returns true when permission granted.
 *           Branch: initRWDocumentsPermission_ == false, VerifyPermissionByTokenId returns true
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyRWDocumentsPermission_Granted_001, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyRWDocumentsPermission();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifyRWDocumentsPermission_Denied_002
 * @tc.desc: Verify VerifyRWDocumentsPermission returns false when permission denied.
 *           Branch: initRWDocumentsPermission_ == false, VerifyPermissionByTokenId returns false
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyRWDocumentsPermission_Denied_002, TestSize.Level1)
{
    OH_EXPECT_RET({false}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifyRWDocumentsPermission();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: VerifyRWDocumentsPermission_Cached_003
 * @tc.desc: Verify VerifyRWDocumentsPermission returns cached value on second call.
 *           Branch: initRWDocumentsPermission_ == true, return cached value
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifyRWDocumentsPermission_Cached_003, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    tidp.VerifyRWDocumentsPermission();  // First call
    bool result = tidp.VerifyRWDocumentsPermission();  // Second call (cached)

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifySandboxAccessPermission_Granted_001
 * @tc.desc: Verify VerifySandboxAccessPermission returns true when permission granted.
 *           Branch: initSandboxAccessPermission_ == false, VerifyPermissionByTokenId returns true
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifySandboxAccessPermission_Granted_001, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifySandboxAccessPermission();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: VerifySandboxAccessPermission_Denied_002
 * @tc.desc: Verify VerifySandboxAccessPermission returns false when permission denied.
 *           Branch: initSandboxAccessPermission_ == false, VerifyPermissionByTokenId returns false
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifySandboxAccessPermission_Denied_002, TestSize.Level1)
{
    OH_EXPECT_RET({false}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    bool result = tidp.VerifySandboxAccessPermission();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: VerifySandboxAccessPermission_Cached_003
 * @tc.desc: Verify VerifySandboxAccessPermission returns cached value on second call.
 *           Branch: initSandboxAccessPermission_ == true, return cached value
 * @tc.type: FUNC
 */
HWTEST_F(TokenIdPermissionTest, VerifySandboxAccessPermission_Cached_003, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    TokenIdPermission tidp(TEST_TOKEN_ID);
    tidp.VerifySandboxAccessPermission();  // First call
    bool result = tidp.VerifySandboxAccessPermission();  // Second call (cached)

    EXPECT_EQ(result, true);
}

}  // namespace AAFwk
}  // namespace OHOS
