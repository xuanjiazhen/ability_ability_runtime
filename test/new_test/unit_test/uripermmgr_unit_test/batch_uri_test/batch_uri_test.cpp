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
#include "batch_uri.h"
#include "file_permission_manager.h"
#include "fud_constants.h"
#include "check_result.h"
#include "policy_info.h"
#include "oh_mock_utils.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AccessControl::SandboxManager;

namespace OHOS {
namespace AAFwk {

class BatchUriTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void BatchUriTest::SetUpTestCase() {}

void BatchUriTest::TearDownTestCase() {}

void BatchUriTest::SetUp() {}

void BatchUriTest::TearDown() {}

/**
 * @tc.name: Init_Empty_001
 * @tc.desc: Verify Init handles empty URI vector.
 *           Branch: uriVec.empty() == true
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, Init_Empty_001, TestSize.Level1)
{
    std::vector<std::string> uriVec;
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "com.example.target";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    int32_t result = batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: Init_InvalidScheme_002
 * @tc.desc: Verify Init rejects URIs with invalid scheme.
 *           Branch: scheme != FILE_SCHEME && scheme != CONTENT_SCHEME
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, Init_InvalidScheme_002, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"http://example.com/file.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "com.example.target";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    int32_t result = batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    EXPECT_EQ(result, 0);
}

/**
 * @tc.name: Init_ContentUri_003
 * @tc.desc: Verify Init handles content URIs.
 *           Branch: scheme == CONTENT_SCHEME
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, Init_ContentUri_003, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"content://media/example"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "com.example.target";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    int32_t result = batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    EXPECT_EQ(result, 1);
}

/**
 * @tc.name: Init_MediaUri_004
 * @tc.desc: Verify Init handles media URIs.
 *           Branch: authority == MEDIA_AUTHORITY
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, Init_MediaUri_004, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://media/test.jpg"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "com.example.target";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    int32_t result = batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    EXPECT_EQ(result, 1);
}

/**
 * @tc.name: Init_DocsUri_005
 * @tc.desc: Verify Init handles docs URIs.
 *           Branch: authority == DOCS_AUTHORITY
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, Init_DocsUri_005, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://docs/test.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "com.example.target";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    int32_t result = batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    EXPECT_EQ(result, 1);
}

/**
 * @tc.name: Init_TargetBundleUri_006
 * @tc.desc: Verify Init handles target bundle URIs.
 *           Branch: authority == targetAlterBundleName
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, Init_TargetBundleUri_006, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://com.example.target/test.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "com.example.target";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    int32_t result = batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    EXPECT_EQ(result, 1);
}

/**
 * @tc.name: SetMediaUriCheckResult_AllTrue_001
 * @tc.desc: Verify SetMediaUriCheckResult with all true results.
 *           Branch: Loop sets checkResult
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, SetMediaUriCheckResult_AllTrue_001, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://media/test1.jpg", "file://media/test2.jpg"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<bool> mediaUriResult = {true, true};
    batchUri.SetMediaUriCheckResult(mediaUriResult);

    EXPECT_EQ(batchUri.GetPermissionedUriCount(), 2);
}

/**
 * @tc.name: SetMediaUriCheckResult_Mixed_002
 * @tc.desc: Verify SetMediaUriCheckResult with mixed results.
 *           Branch: Some true, some false
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, SetMediaUriCheckResult_Mixed_002, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://media/test1.jpg", "file://media/test2.jpg"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<bool> mediaUriResult = {true, false};
    batchUri.SetMediaUriCheckResult(mediaUriResult);

    EXPECT_EQ(batchUri.GetPermissionedUriCount(), 1);
}

/**
 * @tc.name: SetOtherUriCheckResult_AllTrue_001
 * @tc.desc: Verify SetOtherUriCheckResult with all true results.
 *           Branch: All results true, some target bundle
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, SetOtherUriCheckResult_AllTrue_001, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://docs/test1.txt", "file://docs/test2.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "com.example.target";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<bool> otherUriResult = {true, true};
    batchUri.SetOtherUriCheckResult(otherUriResult);

    EXPECT_EQ(batchUri.GetPermissionedUriCount(), 2);
}

/**
 * @tc.name: GetMediaUriToGrant_SomePermissioned_001
 * @tc.desc: Verify GetMediaUriToGrant returns only permissioned URIs.
 *           Branch: Only add if result true
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, GetMediaUriToGrant_SomePermissioned_001, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://media/test1.jpg", "file://media/test2.jpg"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<bool> mediaUriResult = {true, false};
    batchUri.SetMediaUriCheckResult(mediaUriResult);

    std::vector<std::string> uriVecToGrant;
    int32_t count = batchUri.GetMediaUriToGrant(uriVecToGrant);

    EXPECT_EQ(count, 1);
}

/**
 * @tc.name: GetMediaUriToGrant_NonePermissioned_002
 * @tc.desc: Verify GetMediaUriToGrant returns empty when none permissioned.
 *           Branch: No URIs have permission
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, GetMediaUriToGrant_NonePermissioned_002, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://media/test1.jpg"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<bool> mediaUriResult = {false};
    batchUri.SetMediaUriCheckResult(mediaUriResult);

    std::vector<std::string> uriVecToGrant;
    int32_t count = batchUri.GetMediaUriToGrant(uriVecToGrant);

    EXPECT_EQ(count, 0);
}

/**
 * @tc.name: GetNeedCheckProxyPermissionURI_SomeNeedCheck_001
 * @tc.desc: Verify GetNeedCheckProxyPermissionURI filters URIs needing proxy check.
 *           Branch: result == false
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, GetNeedCheckProxyPermissionURI_SomeNeedCheck_001, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://docs/test1.txt", "file://docs/test2.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<bool> otherUriResult = {false, true};
    batchUri.SetOtherUriCheckResult(otherUriResult);

    std::vector<PolicyInfo> proxyUrisByPolicy;
    batchUri.GetNeedCheckProxyPermissionURI(proxyUrisByPolicy);

    EXPECT_GT(proxyUrisByPolicy.size(), 0U);
}

/**
 * @tc.name: SetCheckProxyByPolicyResult_Success_001
 * @tc.desc: Verify SetCheckProxyByPolicyResult with matching size.
 *           Branch: size match, results set
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, SetCheckProxyByPolicyResult_Success_001, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://docs/test1.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<bool> otherUriResult = {false};
    batchUri.SetOtherUriCheckResult(otherUriResult);

    std::vector<bool> proxyResultByPolicy = {true};
    bool result = batchUri.SetCheckProxyByPolicyResult(proxyResultByPolicy);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: SetCheckProxyByPolicyResult_SizeMismatch_002
 * @tc.desc: Verify SetCheckProxyByPolicyResult fails with size mismatch.
 *           Branch: size mismatch
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, SetCheckProxyByPolicyResult_SizeMismatch_002, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://docs/test1.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<bool> otherUriResult = {false};
    batchUri.SetOtherUriCheckResult(otherUriResult);

    std::vector<bool> proxyResultByPolicy = {true, false};  // Size mismatch
    bool result = batchUri.SetCheckProxyByPolicyResult(proxyResultByPolicy);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetUriToGrantByPolicy_HasSelfBundle_001
 * @tc.desc: Verify GetUriToGrantByPolicy with selfBundlePolicyInfos.
 *           Branch: Has selfBundlePolicyInfos
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, GetUriToGrantByPolicy_HasSelfBundle_001, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://com.example.caller/test.txt"};
    uint32_t mode = 1;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    // Mock FilePermissionManager::GetPathPolicyInfoFromUri
    PolicyInfo policyInfo;
    policyInfo.path = "/test";
    policyInfo.mode = mode;
    policyInfo.type = PolicyType::SELF_PATH;
    OH_EXPECT_RET({policyInfo}, FilePermissionManager, GetPathPolicyInfoFromUri, Uri&, uint32_t, const std::string&);

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<PolicyInfo> docsPolicyInfoVec;
    std::vector<PolicyInfo> bundlePolicyInfoVec;
    int32_t count = batchUri.GetUriToGrantByPolicy(docsPolicyInfoVec, bundlePolicyInfoVec);

    EXPECT_GT(count, 0);
}

/**
 * @tc.name: GetUriToGrantByPolicy_SkipsTargetBundle_002
 * @tc.desc: Verify GetUriToGrantByPolicy skips target bundle URIs.
 *           Branch: isTargetBundleUri == true
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, GetUriToGrantByPolicy_SkipsTargetBundle_002, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://com.example.target/test.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "com.example.target";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<bool> otherUriResult = {true};
    batchUri.SetOtherUriCheckResult(otherUriResult);

    std::vector<PolicyInfo> docsPolicyInfoVec;
    std::vector<PolicyInfo> bundlePolicyInfoVec;
    int32_t count = batchUri.GetUriToGrantByPolicy(docsPolicyInfoVec, bundlePolicyInfoVec);

    EXPECT_EQ(count, 0);  // Target bundle skipped
}

/**
 * @tc.name: GetUriToGrantByPolicy_Bool_ReturnAllTrue_001
 * @tc.desc: Verify GetUriToGrantByPolicy(bool) returns true when all permissioned.
 *           Branch: All otherPolicyInfos have permission
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, GetUriToGrantByPolicy_Bool_ReturnAllTrue_001, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://docs/test.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<bool> otherUriResult = {true};
    batchUri.SetOtherUriCheckResult(otherUriResult);

    std::vector<PolicyInfo> policyVec;
    bool result = batchUri.GetUriToGrantByPolicy(policyVec);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: GetUriToGrantByPolicy_Bool_ReturnSomeFalse_002
 * @tc.desc: Verify GetUriToGrantByPolicy(bool) returns false when some not permissioned.
 *           Branch: Some otherPolicyInfos don't have permission
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, GetUriToGrantByPolicy_Bool_ReturnSomeFalse_002, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://docs/test.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<bool> otherUriResult = {false};
    batchUri.SetOtherUriCheckResult(otherUriResult);

    std::vector<PolicyInfo> policyVec;
    bool result = batchUri.GetUriToGrantByPolicy(policyVec);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: SetCheckUriAuthorizationResult_Success_001
 * @tc.desc: Verify SetCheckUriAuthorizationResult with matching size.
 *           Branch: size match
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, SetCheckUriAuthorizationResult_Success_001, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://docs/test.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<bool> funcResult = {false};
    bool result = batchUri.SetCheckUriAuthorizationResult(funcResult);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: SetCheckUriAuthorizationResult_SizeMismatch_002
 * @tc.desc: Verify SetCheckUriAuthorizationResult fails with size mismatch.
 *           Branch: size mismatch
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, SetCheckUriAuthorizationResult_SizeMismatch_002, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://docs/test.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<bool> funcResult = {true, false};  // Size mismatch
    bool result = batchUri.SetCheckUriAuthorizationResult(funcResult);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetPermissionedUriCount_SomePermissioned_001
 * @tc.desc: Verify GetPermissionedUriCount returns correct count.
 *           Branch: Some URIs permissioned
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, GetPermissionedUriCount_SomePermissioned_001, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://media/test1.jpg", "file://media/test2.jpg"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    std::vector<bool> mediaUriResult = {true, false};
    batchUri.SetMediaUriCheckResult(mediaUriResult);

    int32_t count = batchUri.GetPermissionedUriCount();

    EXPECT_EQ(count, 1);
}

/**
 * @tc.name: IsAllUriValid_AllValid_001
 * @tc.desc: Verify IsAllUriValid returns true when all URIs valid.
 *           Branch: validUriCount == totalUriCount
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, IsAllUriValid_AllValid_001, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://media/test.jpg"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    bool result = batchUri.IsAllUriValid();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsAllUriValid_NotAllValid_002
 * @tc.desc: Verify IsAllUriValid returns false when some URIs invalid.
 *           Branch: validUriCount != totalUriCount
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, IsAllUriValid_NotAllValid_002, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"http://example.com/file.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    bool result = batchUri.IsAllUriValid();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: IsAllUriPermissioned_AllPermissioned_001
 * @tc.desc: Verify IsAllUriPermissioned returns true when all permissioned.
 *           Branch: All checkResult.result == true
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, IsAllUriPermissioned_AllPermissioned_001, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://com.example.caller/test.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    bool result = batchUri.IsAllUriPermissioned();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsAllUriPermissioned_NotAllPermissioned_002
 * @tc.desc: Verify IsAllUriPermissioned returns false when some not permissioned.
 *           Branch: Some checkResult.result == false
 * @tc.type: FUNC
 */
HWTEST_F(BatchUriTest, IsAllUriPermissioned_NotAllPermissioned_002, TestSize.Level1)
{
    std::vector<std::string> uriVec = {"file://docs/test.txt"};
    uint32_t mode = 0;
    std::string callerAlterBundleName = "com.example.caller";
    std::string targetAlterBundleName = "";
    bool haveSandboxAccessPermission = false;

    BatchUri batchUri;
    batchUri.Init(uriVec, mode, callerAlterBundleName, targetAlterBundleName, haveSandboxAccessPermission);

    bool result = batchUri.IsAllUriPermissioned();

    EXPECT_EQ(result, false);
}

}  // namespace AAFwk
}  // namespace OHOS
