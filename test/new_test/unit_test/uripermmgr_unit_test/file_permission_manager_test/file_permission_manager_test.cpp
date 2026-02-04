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

#define private public
#define protected public

#include <gtest/gtest.h>
#include "file_permission_manager.h"
#include "file_uri.h"
#include "uri.h"
#include "tokenid_permission.h"
#include "policy_info.h"
#include "oh_mock_utils.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AAFwk;
using namespace OHOS::AccessControl::SandboxManager;

namespace OHOS {
namespace AAFwk {

class FilePermissionManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void FilePermissionManagerTest::SetUpTestCase() {}

void FilePermissionManagerTest::TearDownTestCase() {}

void FilePermissionManagerTest::SetUp() {}

void FilePermissionManagerTest::TearDown() {}

/**
 * @tc.name: GetPathPolicyInfoFromUri_Success_001
 * @tc.desc: Verify GetPathPolicyInfoFromUri returns correct PolicyInfo.
 *           Branch: Normal URI with valid path
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetPathPolicyInfoFromUri_Success_001, TestSize.Level1)
{
    // ===== Arrange：准备数据和 Mock =====
    std::string uriString = "file://docs/test/path/file.txt";
    Uri uri(uriString);
    uint32_t flag = OperationMode::READ_MODE;
    std::string bundleName = "com.example.test";

    std::string expectedPath = "/storage/Users/currentUser/appdata/test/path/file.txt";

    // Mock GetRealPathBySA
    OH_EXPECT_RET({expectedPath}, FileUri, GetRealPathBySA, const std::string&);

    // ===== Act：调用被测代码 =====
    PolicyInfo policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uri, flag, bundleName);

    // ===== Assert：验证结果 =====
    EXPECT_EQ(policyInfo.path, expectedPath);
    EXPECT_EQ(policyInfo.mode, static_cast<uint64_t>(OperationMode::READ_MODE));
}

/**
 * @tc.name: GetPathPolicyInfoFromUri_WriteMode_002
 * @tc.desc: Verify GetPathPolicyInfoFromUri with WRITE_MODE flag.
 *           Branch: flag = WRITE_MODE
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetPathPolicyInfoFromUri_WriteMode_002, TestSize.Level1)
{
    std::string uriString = "file://docs/test/path/file.txt";
    Uri uri(uriString);
    uint32_t flag = OperationMode::WRITE_MODE;
    std::string bundleName = "com.example.test";

    std::string expectedPath = "/storage/Users/currentUser/appdata/test/path/file.txt";
    OH_EXPECT_RET({expectedPath}, FileUri, GetRealPathBySA, const std::string&);

    PolicyInfo policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uri, flag, bundleName);

    EXPECT_EQ(policyInfo.path, expectedPath);
    EXPECT_EQ(policyInfo.mode, static_cast<uint64_t>(OperationMode::WRITE_MODE));
}

/**
 * @tc.name: GetPathPolicyInfoFromUri_ReadWriteMode_003
 * @tc.desc: Verify GetPathPolicyInfoFromUri with READ_MODE | WRITE_MODE flag.
 *           Branch: flag = READ_MODE | WRITE_MODE
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetPathPolicyInfoFromUri_ReadWriteMode_003, TestSize.Level1)
{
    std::string uriString = "file://docs/test/path/file.txt";
    Uri uri(uriString);
    uint32_t flag = OperationMode::READ_MODE | OperationMode::WRITE_MODE;
    std::string bundleName = "com.example.test";

    std::string expectedPath = "/storage/Users/currentUser/appdata/test/path/file.txt";
    OH_EXPECT_RET({expectedPath}, FileUri, GetRealPathBySA, const std::string&);

    PolicyInfo policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uri, flag, bundleName);

    EXPECT_EQ(policyInfo.path, expectedPath);
    EXPECT_EQ(policyInfo.mode, static_cast<uint64_t>(OperationMode::READ_MODE | OperationMode::WRITE_MODE));
}

/**
 * @tc.name: GetPathPolicyInfoFromUri_DefaultBundleName_004
 * @tc.desc: Verify GetPathPolicyInfoFromUri with default bundleName.
 *           Branch: bundleName = "" (default parameter)
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetPathPolicyInfoFromUri_DefaultBundleName_004, TestSize.Level1)
{
    std::string uriString = "file://docs/test/path/file.txt";
    Uri uri(uriString);
    uint32_t flag = OperationMode::READ_MODE;

    std::string expectedPath = "/storage/Users/currentUser/appdata/test/path/file.txt";
    OH_EXPECT_RET({expectedPath}, FileUri, GetRealPathBySA, const std::string&);

    PolicyInfo policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uri, flag);

    EXPECT_EQ(policyInfo.path, expectedPath);
    EXPECT_EQ(policyInfo.mode, static_cast<uint64_t>(OperationMode::READ_MODE));
}

/**
 * @tc.name: CheckUriPersistentPermission_EmptyUri_001
 * @tc.desc: Verify CheckUriPersistentPermission handles empty URI vector.
 *           Branch: uriVec.size() == 0
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_EmptyUri_001, TestSize.Level1)
{
    std::vector<Uri> uriVec;
    uint32_t callerTokenId = 1001;
    uint32_t flag = OperationMode::READ_MODE;
    std::string bundleName = "com.example.test";
    std::vector<PolicyInfo> pathPolicies;

    auto results = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(results.size(), 0U);
    EXPECT_TRUE(pathPolicies.empty());
}

/**
 * @tc.name: CheckUriPersistentPermission_FileManagerGranted_002
 * @tc.desc: Verify CheckUriPersistentPermission with file manager URI granted.
 *           Branch: authority == FILE_MANAGER_AUTHORITY, CheckFileManagerUriPermission returns true
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_FileManagerGranted_002, TestSize.Level1)
{
    std::string uriString = "file://docs/storage/Users/currentUser/Download/com.example.test/file.txt";
    Uri uri(uriString);
    std::vector<Uri> uriVec = {uri};
    uint32_t callerTokenId = 1001;
    uint32_t flag = OperationMode::READ_MODE;
    std::string bundleName = "com.example.test";
    std::vector<PolicyInfo> pathPolicies;

    std::string expectedPath = "/storage/Users/currentUser/Download/com.example.test/file.txt";
    OH_EXPECT_RET({expectedPath}, FileUri, GetRealPathBySA, const std::string&);

    auto results = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0], true);
}

/**
 * @tc.name: CheckUriPersistentPermission_DocsUriSandboxGranted_003
 * @tc.desc: Verify CheckUriPersistentPermission with docs appdata URI and sandbox permission.
 *           Branch: path.find(APPDATA_URI) == 0, VerifySandboxAccessPermission returns true
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_DocsUriSandboxGranted_003, TestSize.Level1)
{
    std::string uriString = "file://docs/storage/Users/currentUser/appdata/test/file.txt";
    Uri uri(uriString);
    std::vector<Uri> uriVec = {uri};
    uint32_t callerTokenId = 1001;
    uint32_t flag = OperationMode::READ_MODE;
    std::string bundleName = "com.example.test";
    std::vector<PolicyInfo> pathPolicies;

    std::string expectedPath = "/storage/Users/currentUser/appdata/test/file.txt";
    OH_EXPECT_RET({expectedPath}, FileUri, GetRealPathBySA, const std::string&);

    auto mockTokenPermission = new TokenIdPermission(callerTokenId);
    OH_EXPECT_RET({true}, TokenIdPermission, VerifySandboxAccessPermission);

    auto results = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0], true);
}

/**
 * @tc.name: CheckUriPersistentPermission_DocsUriSandboxDenied_004
 * @tc.desc: Verify CheckUriPersistentPermission with docs appdata URI and no sandbox permission.
 *           Branch: path.find(APPDATA_URI) == 0, VerifySandboxAccessPermission returns false
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_DocsUriSandboxDenied_004, TestSize.Level1)
{
    std::string uriString = "file://docs/storage/Users/currentUser/appdata/test/file.txt";
    Uri uri(uriString);
    std::vector<Uri> uriVec = {uri};
    uint32_t callerTokenId = 1001;
    uint32_t flag = OperationMode::READ_MODE;
    std::string bundleName = "com.example.test";
    std::vector<PolicyInfo> pathPolicies;

    std::string expectedPath = "/storage/Users/currentUser/appdata/test/file.txt";
    OH_EXPECT_RET({expectedPath}, FileUri, GetRealPathBySA, const std::string&);

    OH_EXPECT_RET({false}, TokenIdPermission, VerifySandboxAccessPermission);

    auto results = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0], false);
}

/**
 * @tc.name: CheckUriPersistentPermission_DocsUriStorageGranted_005
 * @tc.desc: Verify CheckUriPersistentPermission with docs storage URI and file access permission.
 *           Branch: path.find(STORAGE_URI) == 0, VerifyFileAccessManagerPermission returns true
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_DocsUriStorageGranted_005, TestSize.Level1)
{
    std::string uriString = "file://docs/storage/Users/currentUser/test/file.txt";
    Uri uri(uriString);
    std::vector<Uri> uriVec = {uri};
    uint32_t callerTokenId = 1001;
    uint32_t flag = OperationMode::READ_MODE;
    std::string bundleName = "com.example.test";
    std::vector<PolicyInfo> pathPolicies;

    std::string expectedPath = "/storage/Users/currentUser/test/file.txt";
    OH_EXPECT_RET({expectedPath}, FileUri, GetRealPathBySA, const std::string&);

    OH_EXPECT_RET({false}, TokenIdPermission, VerifySandboxAccessPermission);
    OH_EXPECT_RET({true}, TokenIdPermission, VerifyFileAccessManagerPermission);

    auto results = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0], true);
}

/**
 * @tc.name: CheckUriPersistentPermission_DocsUriStorageDenied_006
 * @tc.desc: Verify CheckUriPersistentPermission with docs storage URI and no file access permission.
 *           Branch: path.find(STORAGE_URI) == 0, VerifyFileAccessManagerPermission returns false
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_DocsUriStorageDenied_006, TestSize.Level1)
{
    std::string uriString = "file://docs/storage/Users/currentUser/test/file.txt";
    Uri uri(uriString);
    std::vector<Uri> uriVec = {uri};
    uint32_t callerTokenId = 1001;
    uint32_t flag = OperationMode::READ_MODE;
    std::string bundleName = "com.example.test";
    std::vector<PolicyInfo> pathPolicies;

    std::string expectedPath = "/storage/Users/currentUser/test/file.txt";
    OH_EXPECT_RET({expectedPath}, FileUri, GetRealPathBySA, const std::string&);

    OH_EXPECT_RET({false}, TokenIdPermission, VerifySandboxAccessPermission);
    OH_EXPECT_RET({false}, TokenIdPermission, VerifyFileAccessManagerPermission);

    auto results = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0], false);
}

/**
 * @tc.name: CheckUriPersistentPermission_NonFileManagerUri_007
 * @tc.desc: Verify CheckUriPersistentPermission with non-file-manager URI.
 *           Branch: authority != FILE_MANAGER_AUTHORITY, goes to SandboxManager check
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_NonFileManagerUri_007, TestSize.Level1)
{
    std::string uriString = "file://com.example.test/storage/test/file.txt";
    Uri uri(uriString);
    std::vector<Uri> uriVec = {uri};
    uint32_t callerTokenId = 1001;
    uint32_t flag = OperationMode::READ_MODE;
    std::string bundleName = "com.example.test";
    std::vector<PolicyInfo> pathPolicies;

    std::string expectedPath = "/storage/test/file.txt";
    OH_EXPECT_RET({expectedPath}, FileUri, GetRealPathBySA, const std::string&);

    std::vector<bool> persistResultCodes = {true};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int32_t>(0)}, persistResultCodes,
        SandboxManagerKit, CheckPersistPolicy,
        uint32_t, std::vector<PolicyInfo>&, std::vector<bool>&);

    auto results = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0], true);
}

/**
 * @tc.name: CheckUriPersistentPermission_MultiUrisGranted_008
 * @tc.desc: Verify CheckUriPersistentPermission with multiple URIs, all granted.
 *           Branch: Multiple URIs, all paths match file manager conditions
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_MultiUrisGranted_008, TestSize.Level1)
{
    std::string uriString1 = "file://docs/storage/Users/currentUser/Download/com.example.test/file1.txt";
    std::string uriString2 = "file://docs/storage/Users/currentUser/Download/com.example.test/file2.txt";
    Uri uri1(uriString1);
    Uri uri2(uriString2);
    std::vector<Uri> uriVec = {uri1, uri2};
    uint32_t callerTokenId = 1001;
    uint32_t flag = OperationMode::READ_MODE;
    std::string bundleName = "com.example.test";
    std::vector<PolicyInfo> pathPolicies;

    std::string expectedPath1 = "/storage/Users/currentUser/Download/com.example.test/file1.txt";
    std::string expectedPath2 = "/storage/Users/currentUser/Download/com.example.test/file2.txt";
    auto paths = std::vector<std::string>{expectedPath1, expectedPath2};
    OH_EXPECT_RET(paths, FileUri, GetRealPathBySA, const std::string&);

    auto results = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(results.size(), 2U);
    EXPECT_EQ(results[0], true);
    EXPECT_EQ(results[1], true);
    EXPECT_EQ(pathPolicies.size(), 2U);
}

/**
 * @tc.name: CheckUriPersistentPermission_MultiUrisMixed_009
 * @tc.desc: Verify CheckUriPersistentPermission with multiple URIs, mixed results.
 *           Branch: Some URIs granted, some need persist check
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_MultiUrisMixed_009, TestSize.Level1)
{
    std::string uriString1 = "file://docs/storage/Users/currentUser/Download/com.example.test/file1.txt";
    std::string uriString2 = "file://com.example.test/storage/test/file2.txt";
    Uri uri1(uriString1);
    Uri uri2(uriString2);
    std::vector<Uri> uriVec = {uri1, uri2};
    uint32_t callerTokenId = 1001;
    uint32_t flag = OperationMode::READ_MODE;
    std::string bundleName = "com.example.test";
    std::vector<PolicyInfo> pathPolicies;

    std::string expectedPath1 = "/storage/Users/currentUser/Download/com.example.test/file1.txt";
    std::string expectedPath2 = "/storage/test/file2.txt";
    auto paths = std::vector<std::string>{expectedPath1, expectedPath2};
    OH_EXPECT_RET(paths, FileUri, GetRealPathBySA, const std::string&);

    std::vector<bool> persistResultCodes = {true};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int32_t>(0)}, persistResultCodes,
        SandboxManagerKit, CheckPersistPolicy,
        uint32_t, std::vector<PolicyInfo>&, std::vector<bool>&);

    auto results = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(results.size(), 2U);
    EXPECT_EQ(results[0], true);
    EXPECT_EQ(results[1], true);
}

/**
 * @tc.name: CheckUriPersistentPermission_SandboxManagerFail_010
 * @tc.desc: Verify CheckUriPersistentPermission when SandboxManager check fails.
 *           Branch: CheckPersistPolicy returns error code
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_SandboxManagerFail_010, TestSize.Level1)
{
    std::string uriString = "file://com.example.test/storage/test/file.txt";
    Uri uri(uriString);
    std::vector<Uri> uriVec = {uri};
    uint32_t callerTokenId = 1001;
    uint32_t flag = OperationMode::READ_MODE;
    std::string bundleName = "com.example.test";
    std::vector<PolicyInfo> pathPolicies;

    std::string expectedPath = "/storage/test/file.txt";
    OH_EXPECT_RET({expectedPath}, FileUri, GetRealPathBySA, const std::string&);

    std::vector<bool> persistResultCodes = {true};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int32_t>(-1)}, persistResultCodes,
        SandboxManagerKit, CheckPersistPolicy,
        uint32_t, std::vector<PolicyInfo>&, std::vector<bool>&);

    auto results = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0], false);
}

/**
 * @tc.name: CheckUriPersistentPermission_PolicyInfoFilled_011
 * @tc.desc: Verify CheckUriPersistentPermission fills pathPolicies correctly.
 *           Branch: Verify pathPolicies contains correct PolicyInfo for each URI
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_PolicyInfoFilled_011, TestSize.Level1)
{
    std::string uriString = "file://docs/storage/test/file.txt";
    Uri uri(uriString);
    std::vector<Uri> uriVec = {uri};
    uint32_t callerTokenId = 1001;
    uint32_t flag = OperationMode::READ_MODE;
    std::string bundleName = "com.example.test";
    std::vector<PolicyInfo> pathPolicies;

    std::string expectedPath = "/storage/test/file.txt";
    OH_EXPECT_RET({expectedPath}, FileUri, GetRealPathBySA, const std::string&);

    OH_EXPECT_RET({false}, TokenIdPermission, VerifySandboxAccessPermission);
    OH_EXPECT_RET({false}, TokenIdPermission, VerifyFileAccessManagerPermission);

    auto results = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(pathPolicies.size(), 1U);
    EXPECT_EQ(pathPolicies[0].path, expectedPath);
    EXPECT_EQ(pathPolicies[0].mode, static_cast<uint64_t>(OperationMode::READ_MODE));
}

/**
 * @tc.name: CheckUriPersistentPermission_WriteMode_012
 * @tc.desc: Verify CheckUriPersistentPermission with WRITE_MODE flag.
 *           Branch: flag = WRITE_MODE
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_WriteMode_012, TestSize.Level1)
{
    std::string uriString = "file://docs/storage/Users/currentUser/Download/com.example.test/file.txt";
    Uri uri(uriString);
    std::vector<Uri> uriVec = {uri};
    uint32_t callerTokenId = 1001;
    uint32_t flag = OperationMode::WRITE_MODE;
    std::string bundleName = "com.example.test";
    std::vector<PolicyInfo> pathPolicies;

    std::string expectedPath = "/storage/Users/currentUser/Download/com.example.test/file.txt";
    OH_EXPECT_RET({expectedPath}, FileUri, GetRealPathBySA, const std::string&);

    auto results = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0], true);
    EXPECT_EQ(pathPolicies[0].mode, static_cast<uint64_t>(OperationMode::WRITE_MODE));
}

/**
 * @tc.name: CheckUriPersistentPermission_ReadWriteMode_013
 * @tc.desc: Verify CheckUriPersistentPermission with READ_MODE | WRITE_MODE flag.
 *           Branch: flag = READ_MODE | WRITE_MODE
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_ReadWriteMode_013, TestSize.Level1)
{
    std::string uriString = "file://docs/storage/Users/currentUser/Download/com.example.test/file.txt";
    Uri uri(uriString);
    std::vector<Uri> uriVec = {uri};
    uint32_t callerTokenId = 1001;
    uint32_t flag = OperationMode::READ_MODE | OperationMode::WRITE_MODE;
    std::string bundleName = "com.example.test";
    std::vector<PolicyInfo> pathPolicies;

    std::string expectedPath = "/storage/Users/currentUser/Download/com.example.test/file.txt";
    OH_EXPECT_RET({expectedPath}, FileUri, GetRealPathBySA, const std::string&);

    auto results = FilePermissionManager::CheckUriPersistentPermission(uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(results.size(), 1U);
    EXPECT_EQ(results[0], true);
    EXPECT_EQ(pathPolicies[0].mode, static_cast<uint64_t>(OperationMode::READ_MODE | OperationMode::WRITE_MODE));
}

}  // namespace AAFwk
}  // namespace OHOS
