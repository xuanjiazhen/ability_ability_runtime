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
#include <thread>

#define private public
#include "uri_permission_manager_stub_impl.h"
#undef private

#include "hilog_tag_wrapper.h"
#include "oh_mock_utils.h"
#include "ability_manager_errors.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

class UriPermissionManagerStubImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<UriPermissionManagerStubImpl> stubImpl_ = nullptr;
};

void UriPermissionManagerStubImplTest::SetUpTestCase() {}

void UriPermissionManagerStubImplTest::TearDownTestCase() {}

void UriPermissionManagerStubImplTest::SetUp()
{
    stubImpl_ = std::make_shared<UriPermissionManagerStubImpl>();
}

void UriPermissionManagerStubImplTest::TearDown()
{
    stubImpl_ = nullptr;
}

/**
 * @tc.name: VerifyUriPermission_001
 * @tc.desc: Test VerifyUriPermission with valid file uri and read flag
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, VerifyUriPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermission_001 start");

    // Mock IsDFSCall to return true
    // Mock FUDUtils::IsDFSCall
    OH_EXPECT_RET({true}, FUDUtils, IsDFSCall);

    Uri testUri("file://com.example.app/data/storage/el2/base/test.txt");
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    bool funcResult = false;

    ErrCode ret = stubImpl_->VerifyUriPermission(testUri, flag, tokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);
    // Result depends on whether permission was granted before

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermission_001 end");
}

/**
 * @tc.name: VerifyUriPermission_002
 * @tc.desc: Test VerifyUriPermission with invalid flag
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, VerifyUriPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermission_002 start");

    OH_EXPECT_RET({true}, FUDUtils, IsDFSCall);

    Uri testUri("file://com.example.app/data/storage/el2/base/test.txt");
    uint32_t flag = 0; // Invalid flag
    uint32_t tokenId = 1001;
    bool funcResult = true; // Start with true

    ErrCode ret = stubImpl_->VerifyUriPermission(testUri, flag, tokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_FALSE(funcResult); // Should be false for invalid flag

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermission_002 end");
}

/**
 * @tc.name: VerifyUriPermission_003
 * @tc.desc: Test VerifyUriPermission with non-DFS call
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, VerifyUriPermission_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermission_003 start");

    // Mock IsDFSCall to return false
    OH_EXPECT_RET({false}, FUDUtils, IsDFSCall);

    Uri testUri("file://com.example.app/data/storage/el2/base/test.txt");
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    bool funcResult = true; // Start with true

    ErrCode ret = stubImpl_->VerifyUriPermission(testUri, flag, tokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_FALSE(funcResult); // Should be false for non-DFS call

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermission_003 end");
}

/**
 * @tc.name: VerifyUriPermission_004
 * @tc.desc: Test VerifyUriPermission with non-file scheme uri
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, VerifyUriPermission_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermission_004 start");

    OH_EXPECT_RET({true}, FUDUtils, IsDFSCall);
    OH_EXPECT_RET({"file"}, FUDConstants, FILE_SCHEME);

    Uri testUri("content://com.example.app/data/test.txt");
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    bool funcResult = true;

    ErrCode ret = stubImpl_->VerifyUriPermission(testUri, flag, tokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_FALSE(funcResult); // Should be false for non-file scheme

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermission_004 end");
}

/**
 * @tc.name: GrantUriPermission_Single_001
 * @tc.desc: Test GrantUriPermission with single valid uri
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermission_Single_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Single_001 start");

    // Mock system app call
    OH_EXPECT_RET({true}, FUDUtils, IsSystemAppCall);
    OH_EXPECT_RET({true}, FUDUtils, IsSAOrSystemAppCall);
    OH_EXPECT_RET({ERR_OK}, FUDUtils, CheckCalledBySandBox);
    OH_EXPECT_RET({true}, FUDUtils, IsPrivilegedSACall);

    Uri testUri("file://com.example.app/data/storage/el2/base/test.txt");
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t initiatorTokenId = 1001;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermission(testUri, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Single_001 end");
}

/**
 * @tc.name: GrantUriPermission_Single_002
 * @tc.desc: Test GrantUriPermission with invalid uri type for system app
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermission_Single_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Single_002 start");

    OH_EXPECT_RET({true}, FUDUtils, IsSystemAppCall);
    OH_EXPECT_RET({"file"}, FUDConstants, FILE_SCHEME);

    Uri testUri("content://com.example.app/data/test.txt"); // Non-file uri
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t initiatorTokenId = 1001;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermission(testUri, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(funcResult, ERR_CODE_INVALID_URI_TYPE);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Single_002 end");
}

/**
 * @tc.name: GrantUriPermission_Vector_001
 * @tc.desc: Test GrantUriPermission with vector of uris
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermission_Vector_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Vector_001 start");

    OH_EXPECT_RET({true}, FUDUtils, IsSAOrSystemAppCall);
    OH_EXPECT_RET({ERR_OK}, FUDUtils, CheckCalledBySandBox);
    OH_EXPECT_RET({ERR_OK}, FUDUtils, GetTokenIdByBundleName, const std::string&, int32_t, uint32_t&);
    OH_EXPECT_RET({false}, FUDUtils, IsPrivilegedSACall);
    OH_EXPECT_RET({1001}, IPCSkeleton, GetCallingTokenID);

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test1.txt",
        "file://com.example.app/data/storage/el2/base/test2.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t initiatorTokenId = 1001;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Vector_001 end");
}

/**
 * @tc.name: GrantUriPermission_Vector_002
 * @tc.desc: Test GrantUriPermission with empty uri vector
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermission_Vector_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Vector_002 start");

    OH_EXPECT_RET({true}, FUDUtils, IsSAOrSystemAppCall);
    OH_EXPECT_RET({ERR_OK}, FUDUtils, CheckCalledBySandBox);

    std::vector<std::string> uriVec; // Empty vector
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t initiatorTokenId = 1001;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(funcResult, ERR_URI_LIST_OUT_OF_RANGE);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Vector_002 end");
}

/**
 * @tc.name: GrantUriPermission_Vector_003
 * @tc.desc: Test GrantUriPermission with invalid flag
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermission_Vector_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Vector_003 start");

    OH_EXPECT_RET({true}, FUDUtils, IsSAOrSystemAppCall);
    OH_EXPECT_RET({ERR_OK}, FUDUtils, CheckCalledBySandBox);

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t flag = 0; // Invalid flag
    std::string targetBundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t initiatorTokenId = 1001;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(funcResult, ERR_CODE_INVALID_URI_FLAG);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Vector_003 end");
}

/**
 * @tc.name: GrantUriPermission_Vector_004
 * @tc.desc: Test GrantUriPermission with non-system app call
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermission_Vector_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Vector_004 start");

    OH_EXPECT_RET({false}, FUDUtils, IsSAOrSystemAppCall);

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t initiatorTokenId = 1001;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(funcResult, ERR_NOT_SYSTEM_APP);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Vector_004 end");
}

/**
 * @tc.name: GrantUriPermissionPrivileged_001
 * @tc.desc: Test GrantUriPermissionPrivileged with valid uris
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermissionPrivileged_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionPrivileged_001 start");

    OH_EXPECT_RET({true}, FUDUtils, IsSAOrSystemAppCall);
    OH_EXPECT_RET({ERR_OK}, FUDUtils, CheckCalledBySandBox);

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t initiatorTokenId = 1001;
    int32_t hideSensitiveType = 0;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermissionPrivileged(uriVec, flag, targetBundleName,
        appIndex, initiatorTokenId, hideSensitiveType, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionPrivileged_001 end");
}

/**
 * @tc.name: CheckUriAuthorization_001
 * @tc.desc: Test CheckUriAuthorization with valid uris
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, CheckUriAuthorization_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriAuthorization_001 start");

    OH_EXPECT_RET({true}, FUDUtils, IsDFSCall);

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    std::vector<bool> funcResult;

    ErrCode ret = stubImpl_->CheckUriAuthorization(uriVec, flag, tokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriAuthorization_001 end");
}

/**
 * @tc.name: CheckUriAuthorization_002
 * @tc.desc: Test CheckUriAuthorization with empty uri vector
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, CheckUriAuthorization_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriAuthorization_002 start");

    OH_EXPECT_RET({true}, FUDUtils, IsDFSCall);

    std::vector<std::string> uriVec; // Empty vector
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    std::vector<bool> funcResult;

    ErrCode ret = stubImpl_->CheckUriAuthorization(uriVec, flag, tokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(funcResult.empty());

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriAuthorization_002 end");
}

/**
 * @tc.name: RevokeAllUriPermissions_001
 * @tc.desc: Test RevokeAllUriPermissions
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, RevokeAllUriPermissions_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeAllUriPermissions_001 start");

    uint32_t tokenId = 1001;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->RevokeAllUriPermissions(tokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeAllUriPermissions_001 end");
}

/**
 * @tc.name: RevokeUriPermissionManually_001
 * @tc.desc: Test RevokeUriPermissionManually with valid uri
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, RevokeUriPermissionManually_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeUriPermissionManually_001 start");

    Uri testUri("file://com.example.app/data/storage/el2/base/test.txt");
    std::string bundleName = "com.example.target";
    int32_t appIndex = 0;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->RevokeUriPermissionManually(testUri, bundleName, appIndex, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeUriPermissionManually_001 end");
}

/**
 * @tc.name: GrantUriPermissionByKey_001
 * @tc.desc: Test GrantUriPermissionByKey
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermissionByKey_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionByKey_001 start");

    std::string key = "test_udmf_key";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t targetTokenId = 1001;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermissionByKey(key, flag, targetTokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionByKey_001 end");
}

/**
 * @tc.name: GrantUriPermissionByKeyAsCaller_001
 * @tc.desc: Test GrantUriPermissionByKeyAsCaller
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermissionByKeyAsCaller_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionByKeyAsCaller_001 start");

    std::string key = "test_udmf_key";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t callerTokenId = 1002;
    uint32_t targetTokenId = 1001;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermissionByKeyAsCaller(key, flag, callerTokenId, targetTokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionByKeyAsCaller_001 end");
}

/**
 * @tc.name: VerifyUriPermissionByPolicy_001
 * @tc.desc: Test VerifyUriPermissionByPolicy
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, VerifyUriPermissionByPolicy_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermissionByPolicy_001 start");

    std::vector<PolicyInfo> policyVec;
    PolicyInfo policy;
    policy.path = "/data/storage/el2/base/test.txt";
    policy.mode = 0;
    policyVec.push_back(policy);

    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;

    std::vector<bool> result = stubImpl_->VerifyUriPermissionByPolicy(policyVec, flag, tokenId);

    EXPECT_EQ(result.size(), 1u);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermissionByPolicy_001 end");
}

/**
 * @tc.name: VerifyUriPermissionByPolicy_002
 * @tc.desc: Test VerifyUriPermissionByPolicy with empty policy vector
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, VerifyUriPermissionByPolicy_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermissionByPolicy_002 start");

    std::vector<PolicyInfo> policyVec; // Empty vector
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;

    std::vector<bool> result = stubImpl_->VerifyUriPermissionByPolicy(policyVec, flag, tokenId);

    EXPECT_TRUE(result.empty());

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermissionByPolicy_002 end");
}

/**
 * @tc.name: GrantUriPermissionPrivileged_002
 * @tc.desc: Test GrantUriPermissionPrivileged with empty uri vector
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermissionPrivileged_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionPrivileged_002 start");

    OH_EXPECT_RET({true}, FUDUtils, IsSAOrSystemAppCall);
    OH_EXPECT_RET({ERR_OK}, FUDUtils, CheckCalledBySandBox);

    std::vector<std::string> uriVec; // Empty vector
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t initiatorTokenId = 1001;
    int32_t hideSensitiveType = 0;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermissionPrivileged(uriVec, flag, targetBundleName,
        appIndex, initiatorTokenId, hideSensitiveType, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionPrivileged_002 end");
}

/**
 * @tc.name: GrantUriPermissionPrivileged_003
 * @tc.desc: Test GrantUriPermissionPrivileged with write permission
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermissionPrivileged_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionPrivileged_003 start");

    OH_EXPECT_RET({true}, FUDUtils, IsSAOrSystemAppCall);
    OH_EXPECT_RET({ERR_OK}, FUDUtils, CheckCalledBySandBox);

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    std::string targetBundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t initiatorTokenId = 1001;
    int32_t hideSensitiveType = 0;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermissionPrivileged(uriVec, flag, targetBundleName,
        appIndex, initiatorTokenId, hideSensitiveType, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionPrivileged_003 end");
}

/**
 * @tc.name: GrantUriPermissionPrivileged_004
 * @tc.desc: Test GrantUriPermissionPrivileged with read write permission
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermissionPrivileged_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionPrivileged_004 start");

    OH_EXPECT_RET({true}, FUDUtils, IsSAOrSystemAppCall);
    OH_EXPECT_RET({ERR_OK}, FUDUtils, CheckCalledBySandBox);

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    std::string targetBundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t initiatorTokenId = 1001;
    int32_t hideSensitiveType = 1;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermissionPrivileged(uriVec, flag, targetBundleName,
        appIndex, initiatorTokenId, hideSensitiveType, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionPrivileged_004 end");
}

/**
 * @tc.name: CheckUriAuthorization_003
 * @tc.desc: Test CheckUriAuthorization with write permission
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, CheckUriAuthorization_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriAuthorization_003 start");

    OH_EXPECT_RET({true}, FUDUtils, IsDFSCall);

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    uint32_t tokenId = 1001;
    std::vector<bool> funcResult;

    ErrCode ret = stubImpl_->CheckUriAuthorization(uriVec, flag, tokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriAuthorization_003 end");
}

/**
 * @tc.name: CheckUriAuthorization_004
 * @tc.desc: Test CheckUriAuthorization with read write permission
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, CheckUriAuthorization_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriAuthorization_004 start");

    OH_EXPECT_RET({true}, FUDUtils, IsDFSCall);

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    uint32_t tokenId = 1001;
    std::vector<bool> funcResult;

    ErrCode ret = stubImpl_->CheckUriAuthorization(uriVec, flag, tokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriAuthorization_004 end");
}

/**
 * @tc.name: VerifyUriPermissionByPolicy_003
 * @tc.desc: Test VerifyUriPermissionByPolicy with multiple policies
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, VerifyUriPermissionByPolicy_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermissionByPolicy_003 start");

    std::vector<PolicyInfo> policyVec;
    for (int i = 0; i < 5; i++) {
        PolicyInfo policy;
        policy.path = "/data/storage/el2/base/test" + std::to_string(i) + ".txt";
        policy.mode = 0;
        policyVec.push_back(policy);
    }

    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;

    std::vector<bool> result = stubImpl_->VerifyUriPermissionByPolicy(policyVec, flag, tokenId);

    EXPECT_EQ(result.size(), 5u);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermissionByPolicy_003 end");
}

/**
 * @tc.name: VerifyUriPermissionByMap_001
 * @tc.desc: Test VerifyUriPermissionByMap with single uri
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, VerifyUriPermissionByMap_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermissionByMap_001 start");

    std::vector<Uri> uriVec;
    uriVec.push_back(Uri("file://com.example.app/data/storage/el2/base/test.txt"));

    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;

    std::vector<bool> result = stubImpl_->VerifyUriPermissionByMap(uriVec, flag, tokenId);

    EXPECT_EQ(result.size(), 1u);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermissionByMap_001 end");
}

/**
 * @tc.name: VerifyUriPermissionByMap_002
 * @tc.desc: Test VerifyUriPermissionByMap with multiple uris
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, VerifyUriPermissionByMap_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermissionByMap_002 start");

    std::vector<Uri> uriVec;
    for (int i = 0; i < 3; i++) {
        uriVec.push_back(Uri("file://com.example.app/data/storage/el2/base/test" + std::to_string(i) + ".txt"));
    }

    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;

    std::vector<bool> result = stubImpl_->VerifyUriPermissionByMap(uriVec, flag, tokenId);

    EXPECT_EQ(result.size(), 3u);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermissionByMap_002 end");
}

/**
 * @tc.name: RevokeAllUriPermissions_002
 * @tc.desc: Test RevokeAllUriPermissions with different token
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, RevokeAllUriPermissions_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeAllUriPermissions_002 start");

    uint32_t tokenId = 2002;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->RevokeAllUriPermissions(tokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeAllUriPermissions_002 end");
}

/**
 * @tc.name: RevokeUriPermissionManually_002
 * @tc.desc: Test RevokeUriPermissionManually with different app index
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, RevokeUriPermissionManually_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeUriPermissionManually_002 start");

    Uri testUri("file://com.example.app/data/storage/el2/base/test.txt");
    std::string bundleName = "com.example.target";
    int32_t appIndex = 1;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->RevokeUriPermissionManually(testUri, bundleName, appIndex, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeUriPermissionManually_002 end");
}

/**
 * @tc.name: GrantUriPermissionByKey_002
 * @tc.desc: Test GrantUriPermissionByKey with write permission
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermissionByKey_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionByKey_002 start");

    std::string key = "test_udmf_key_2";
    uint32_t flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    uint32_t targetTokenId = 1001;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermissionByKey(key, flag, targetTokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionByKey_002 end");
}

/**
 * @tc.name: GrantUriPermissionByKeyAsCaller_002
 * @tc.desc: Test GrantUriPermissionByKeyAsCaller with write permission
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermissionByKeyAsCaller_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionByKeyAsCaller_002 start");

    std::string key = "test_udmf_key_3";
    uint32_t flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    uint32_t callerTokenId = 1002;
    uint32_t targetTokenId = 1001;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermissionByKeyAsCaller(key, flag, callerTokenId, targetTokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermissionByKeyAsCaller_002 end");
}

/**
 * @tc.name: GrantUriPermission_Vector_MaxURI_001
 * @tc.desc: Test GrantUriPermission with maximum uri count
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermission_Vector_MaxURI_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Vector_MaxURI_001 start");

    OH_EXPECT_RET({true}, FUDUtils, IsSAOrSystemAppCall);
    OH_EXPECT_RET({ERR_OK}, FUDUtils, CheckCalledBySandBox);

    std::vector<std::string> uriVec;
    for (int i = 0; i < 100; i++) {
        uriVec.push_back("file://com.example.app/data/storage/el2/base/test" + std::to_string(i) + ".txt");
    }

    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t initiatorTokenId = 1001;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_Vector_MaxURI_001 end");
}

/**
 * @tc.name: VerifyUriPermission_DocsUri_001
 * @tc.desc: Test VerifyUriPermission with docs cloud uri
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, VerifyUriPermission_DocsUri_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermission_DocsUri_001 start");

    OH_EXPECT_RET({true}, FUDUtils, IsDFSCall);

    Uri testUri("file://com.example.app/data/storage/el2/base/docs/test.doc?networkid=test");
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    bool funcResult = false;

    ErrCode ret = stubImpl_->VerifyUriPermission(testUri, flag, tokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermission_DocsUri_001 end");
}

/**
 * @tc.name: GrantUriPermission_PersistableFlag_001
 * @tc.desc: Test GrantUriPermission with persistable flag
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermission_PersistableFlag_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_PersistableFlag_001 start");

    OH_EXPECT_RET({true}, FUDUtils, IsSAOrSystemAppCall);
    OH_EXPECT_RET({ERR_OK}, FUDUtils, CheckCalledBySandBox);
    OH_EXPECT_RET({true}, FUDUtils, IsPrivilegedSACall);

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    std::string targetBundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t initiatorTokenId = 1001;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_PersistableFlag_001 end");
}

/**
 * @tc.name: CheckUriAuthorization_MixedUris_001
 * @tc.desc: Test CheckUriAuthorization with mixed valid and invalid uris
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, CheckUriAuthorization_MixedUris_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriAuthorization_MixedUris_001 start");

    OH_EXPECT_RET({true}, FUDUtils, IsDFSCall);

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test1.txt",
        "file://media/image/test.jpg",
        "file://com.example.app/data/storage/el2/base/test2.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;
    std::vector<bool> funcResult;

    ErrCode ret = stubImpl_->CheckUriAuthorization(uriVec, flag, tokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(funcResult.size(), 3u);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriAuthorization_MixedUris_001 end");
}

/**
 * @tc.name: GrantUriPermission_MultipleCalls_001
 * @tc.desc: Test multiple GrantUriPermission calls
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermission_MultipleCalls_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_MultipleCalls_001 start");

    OH_EXPECT_RET({true}, FUDUtils, IsSAOrSystemAppCall);
    OH_EXPECT_RET({ERR_OK}, FUDUtils, CheckCalledBySandBox);
    OH_EXPECT_RET({false}, FUDUtils, IsPrivilegedSACall);
    OH_EXPECT_RET({1001}, IPCSkeleton, GetCallingTokenID);

    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t initiatorTokenId = 1001;
    int32_t funcResult = -1;

    // First call
    ErrCode ret1 = stubImpl_->GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);
    EXPECT_EQ(ret1, ERR_OK);

    // Second call with same params
    int32_t funcResult2 = -1;
    ErrCode ret2 = stubImpl_->GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, funcResult2);
    EXPECT_EQ(ret2, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_MultipleCalls_001 end");
}

/**
 * @tc.name: RevokeAllUriPermissions_MultipleTokens_001
 * @tc.desc: Test RevokeAllUriPermissions with multiple tokens
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, RevokeAllUriPermissions_MultipleTokens_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeAllUriPermissions_MultipleTokens_001 start");

    std::vector<uint32_t> tokenIds = {1001, 1002, 1003};

    for (auto tokenId : tokenIds) {
        int32_t funcResult = -1;
        ErrCode ret = stubImpl_->RevokeAllUriPermissions(tokenId, funcResult);
        EXPECT_EQ(ret, ERR_OK);
    }

    TAG_LOGI(AAFwkTag::URIPERMMGR, "RevokeAllUriPermissions_MultipleTokens_001 end");
}

/**
 * @tc.name: VerifyUriPermission_InvalidToken_001
 * @tc.desc: Test VerifyUriPermission with invalid token
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, VerifyUriPermission_InvalidToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermission_InvalidToken_001 start");

    OH_EXPECT_RET({true}, FUDUtils, IsDFSCall);

    Uri testUri("file://com.example.app/data/storage/el2/base/test.txt");
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 0; // Invalid token
    bool funcResult = true;

    ErrCode ret = stubImpl_->VerifyUriPermission(testUri, flag, tokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUriPermission_InvalidToken_001 end");
}

/**
 * @tc.name: GrantUriPermission_ContentUri_001
 * @tc.desc: Test GrantUriPermission with content uri
 * @tc.type: FUNC
 */
HWTEST_F(UriPermissionManagerStubImplTest, GrantUriPermission_ContentUri_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_ContentUri_001 start");

    OH_EXPECT_RET({true}, FUDUtils, IsSAOrSystemAppCall);
    OH_EXPECT_RET({ERR_OK}, FUDUtils, CheckCalledBySandBox);
    OH_EXPECT_RET({true}, FUDUtils, IsUdmfOrPasteboardCall);

    std::vector<std::string> uriVec = {
        "content://com.example.app/data/test.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string targetBundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t initiatorTokenId = 1001;
    int32_t funcResult = -1;

    ErrCode ret = stubImpl_->GrantUriPermission(uriVec, flag, targetBundleName, appIndex, initiatorTokenId, funcResult);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GrantUriPermission_ContentUri_001 end");
}

} // namespace AAFwk
} // namespace OHOS
