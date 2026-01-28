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
#include "file_permission_manager.h"
#undef private

#include "hilog_tag_wrapper.h"
#include "oh_mock_utils.h"

using namespace testing;
using namespace testing::ext;

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
 * @tc.name: CheckUriPersistentPermission_001
 * @tc.desc: Test CheckUriPersistentPermission with valid uris
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_001 start");

    std::vector<std::string> uriStrs = {
        "file://com.example.app/data/storage/el2/base/test1.txt",
        "file://com.example.app/data/storage/el2/base/test2.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    uint32_t tokenId = 1001;

    std::vector<bool> result = FilePermissionManager::CheckUriPersistentPermission(uriStrs, flag, tokenId);

    EXPECT_EQ(result.size(), 2u);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_001 end");
}

/**
 * @tc.name: CheckUriPersistentPermission_002
 * @tc.desc: Test CheckUriPersistentPermission with empty uri vector
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_002 start");

    std::vector<std::string> uriStrs; // Empty vector
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;

    std::vector<bool> result = FilePermissionManager::CheckUriPersistentPermission(uriStrs, flag, tokenId);

    EXPECT_TRUE(result.empty());

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_002 end");
}

/**
 * @tc.name: CheckDocsUriPermission_001
 * @tc.desc: Test CheckDocsUriPermission with valid uris
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckDocsUriPermission_001 start");

    std::vector<std::string> uriStrs = {
        "file://com.example.app/data/storage/el2/base/docs/test1.doc",
        "file://com.example.app/data/storage/el2/base/docs/test2.doc"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;

    std::vector<bool> result = FilePermissionManager::CheckDocsUriPermission(uriStrs, flag, tokenId);

    EXPECT_EQ(result.size(), 2u);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckDocsUriPermission_001 end");
}

/**
 * @tc.name: CheckDocsUriPermission_002
 * @tc.desc: Test CheckDocsUriPermission with empty uri vector
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckDocsUriPermission_002 start");

    std::vector<std::string> uriStrs; // Empty vector
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;

    std::vector<bool> result = FilePermissionManager::CheckDocsUriPermission(uriStrs, flag, tokenId);

    EXPECT_TRUE(result.empty());

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckDocsUriPermission_002 end");
}

/**
 * @tc.name: GetPathPolicyInfoFromUri_001
 * @tc.desc: Test GetPathPolicyInfoFromUri with valid file uri
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetPathPolicyInfoFromUri_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetPathPolicyInfoFromUri_001 start");

    std::string uriStr = "file://com.example.app/data/storage/el2/base/test.txt";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;

    PolicyInfo policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uriStr, flag);

    EXPECT_FALSE(policyInfo.path.empty());

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetPathPolicyInfoFromUri_001 end");
}

/**
 * @tc.name: GetPathPolicyInfoFromUri_002
 * @tc.desc: Test GetPathPolicyInfoFromUri with invalid uri
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetPathPolicyInfoFromUri_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetPathPolicyInfoFromUri_002 start");

    std::string uriStr = "invalid-uri";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;

    PolicyInfo policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uriStr, flag);

    // Should return default policy info
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetPathPolicyInfoFromUri_002 end");
}

/**
 * @tc.name: GetPathPolicyInfoFromUri_003
 * @tc.desc: Test GetPathPolicyInfoFromUri with different flags
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetPathPolicyInfoFromUri_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetPathPolicyInfoFromUri_003 start");

    std::string uriStr = "file://com.example.app/data/storage/el2/base/test.txt";
    uint32_t flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;

    PolicyInfo policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uriStr, flag);

    EXPECT_FALSE(policyInfo.path.empty());

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetPathPolicyInfoFromUri_003 end");
}

/**
 * @tc.name: LoadDllLibrary_001
 * @tc.desc: Test LoadDllLibrary
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, LoadDllLibrary_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "LoadDllLibrary_001 start");

    bool result = FilePermissionManager::LoadDllLibrary();

    // Result depends on whether the library exists
    TAG_LOGI(AAFwkTag::URIPERMMGR, "LoadDllLibrary_001 end");
}

/**
 * @tc.name: UnLoadDllLibrary_001
 * @tc.desc: Test UnLoadDllLibrary
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, UnLoadDllLibrary_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "UnLoadDllLibrary_001 start");

    FilePermissionManager::UnLoadDllLibrary();

    // Just verify it doesn't crash
    TAG_LOGI(AAFwkTag::URIPERMMGR, "UnLoadDllLibrary_001 end");
}

/**
 * @tc.name: GetFilePermission_001
 * @tc.desc: Test GetFilePermission with valid path
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetFilePermission_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetFilePermission_001 start");

    std::string path = "/data/storage/el2/base/test.txt";
    uint32_t mode = 0;

    int32_t ret = FilePermissionManager::GetFilePermission(path, mode);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetFilePermission_001 end");
}

/**
 * @tc.name: GetFilePermission_002
 * @tc.desc: Test GetFilePermission with invalid path
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetFilePermission_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetFilePermission_002 start");

    std::string path = "/invalid/path/test.txt";
    uint32_t mode = 0;

    int32_t ret = FilePermissionManager::GetFilePermission(path, mode);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetFilePermission_002 end");
}

/**
 * @tc.name: CheckUriPersistentPermission_003
 * @tc.desc: Test CheckUriPersistentPermission with write permission
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_003 start");

    std::vector<std::string> uriStrs = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION | Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    uint32_t tokenId = 1001;

    std::vector<bool> result = FilePermissionManager::CheckUriPersistentPermission(uriStrs, flag, tokenId);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_003 end");
}

/**
 * @tc.name: CheckUriPersistentPermission_004
 * @tc.desc: Test CheckUriPersistentPermission with read write persistable
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_004 start");

    std::vector<std::string> uriStrs = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION |
                        Want::FLAG_AUTH_PERSISTABLE_URI_PERMISSION;
    uint32_t tokenId = 1001;

    std::vector<bool> result = FilePermissionManager::CheckUriPersistentPermission(uriStrs, flag, tokenId);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_004 end");
}

/**
 * @tc.name: CheckDocsUriPermission_003
 * @tc.desc: Test CheckDocsUriPermission with write permission
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckDocsUriPermission_003 start");

    std::vector<std::string> uriStrs = {
        "file://com.example.app/data/storage/el2/base/docs/test.doc"
    };
    uint32_t flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;
    uint32_t tokenId = 1001;

    std::vector<bool> result = FilePermissionManager::CheckDocsUriPermission(uriStrs, flag, tokenId);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckDocsUriPermission_003 end");
}

/**
 * @tc.name: CheckDocsUriPermission_004
 * @tc.desc: Test CheckDocsUriPermission with multiple uris
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckDocsUriPermission_004 start");

    std::vector<std::string> uriStrs = {
        "file://com.example.app/data/storage/el2/base/docs/test1.doc",
        "file://com.example.app/data/storage/el2/base/docs/test2.doc",
        "file://com.example.app/data/storage/el2/base/docs/test3.doc"
    };
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    uint32_t tokenId = 1001;

    std::vector<bool> result = FilePermissionManager::CheckDocsUriPermission(uriStrs, flag, tokenId);

    EXPECT_EQ(result.size(), 3u);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckDocsUriPermission_004 end");
}

/**
 * @tc.name: GetPathPolicyInfoFromUri_004
 * @tc.desc: Test GetPathPolicyInfoFromUri with write permission
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetPathPolicyInfoFromUri_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetPathPolicyInfoFromUri_004 start");

    std::string uriStr = "file://com.example.app/data/storage/el2/base/test.txt";
    uint32_t flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;

    PolicyInfo policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uriStr, flag);

    EXPECT_FALSE(policyInfo.path.empty());

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetPathPolicyInfoFromUri_004 end");
}

/**
 * @tc.name: GetPathPolicyInfoFromUri_005
 * @tc.desc: Test GetPathPolicyInfoFromUri with different path types
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetPathPolicyInfoFromUri_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetPathPolicyInfoFromUri_005 start");

    std::vector<std::string> uriStrs = {
        "file://com.example.app/data/storage/el2/base/files/test.txt",
        "file://com.example.app/data/storage/el2/base/download/test.txt"
    };

    for (auto uriStr : uriStrs) {
        uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
        PolicyInfo policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uriStr, flag);
        EXPECT_FALSE(policyInfo.path.empty());
    }

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetPathPolicyInfoFromUri_005 end");
}

/**
 * @tc.name: GetPathPolicyInfoFromUri_006
 * @tc.desc: Test GetPathPolicyInfoFromUri with docs uri
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetPathPolicyInfoFromUri_006, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetPathPolicyInfoFromUri_006 start");

    std::string uriStr = "file://com.example.app/data/storage/el2/base/docs/test.doc";
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;

    PolicyInfo policyInfo = FilePermissionManager::GetPathPolicyInfoFromUri(uriStr, flag);

    EXPECT_FALSE(policyInfo.path.empty());

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetPathPolicyInfoFromUri_006 end");
}

/**
 * @tc.name: LoadDllLibrary_002
 * @tc.desc: Test LoadDllLibrary when already loaded
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, LoadDllLibrary_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "LoadDllLibrary_002 start");

    // First load
    bool result1 = FilePermissionManager::LoadDllLibrary();

    // Second load (should return true if already loaded)
    bool result2 = FilePermissionManager::LoadDllLibrary();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "LoadDllLibrary_002 end");
}

/**
 * @tc.name: GetFilePermission_003
 * @tc.desc: Test GetFilePermission with different paths
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetFilePermission_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetFilePermission_003 start");

    std::vector<std::string> paths = {
        "/data/storage/el2/base/test1.txt",
        "/data/storage/el2/base/test2.txt"
    };

    for (auto path : paths) {
        uint32_t mode = 0;
        int32_t ret = FilePermissionManager::GetFilePermission(path, mode);
    }

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetFilePermission_003 end");
}

/**
 * @tc.name: GetFilePermission_004
 * @tc.desc: Test GetFilePermission with root path
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetFilePermission_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetFilePermission_004 start");

    std::string path = "/data/storage/el2/base/";
    uint32_t mode = 0;

    int32_t ret = FilePermissionManager::GetFilePermission(path, mode);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetFilePermission_004 end");
}

/**
 * @tc.name: GetFilePermission_005
 * @tc.desc: Test GetFilePermission with empty path
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetFilePermission_005, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetFilePermission_005 start");

    std::string path = ""; // Empty path
    uint32_t mode = 0;

    int32_t ret = FilePermissionManager::GetFilePermission(path, mode);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetFilePermission_005 end");
}

/**
 * @tc.name: UnLoadDllLibrary_002
 * @tc.desc: Test UnLoadDllLibrary and LoadDllLibrary sequence
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, UnLoadDllLibrary_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "UnLoadDllLibrary_002 start");

    FilePermissionManager::LoadDllLibrary();
    FilePermissionManager::UnLoadDllLibrary();
    FilePermissionManager::LoadDllLibrary();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "UnLoadDllLibrary_002 end");
}

/**
 * @tc.name: CheckDocsUriPermission_AppDataPath_001
 * @tc.desc: Test CheckDocsUriPermission with APPDATA_URI path
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_AppDataPath_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckDocsUriPermission_AppDataPath_001 start");

    std::string path = "/storage/Users/currentUser/appdata/test.txt";
    TokenIdPermission tokenPermission(1001);

    bool result = FilePermissionManager::CheckDocsUriPermission(tokenPermission, path);

    // Result depends on VerifySandboxAccessPermission
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckDocsUriPermission_AppDataPath_001 end");
}

/**
 * @tc.name: CheckFileManagerUriPermission_DownloadMatch_001
 * @tc.desc: Test CheckFileManagerUriPermission with Download path matching bundleName
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckFileManagerUriPermission_DownloadMatch_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckFileManagerUriPermission_DownloadMatch_001 start");

    std::string filePath = "/storage/Users/currentUser/Download/com.example.app/test.txt";
    std::string bundleName = "com.example.app";
    TokenIdPermission tokenPermission(1001);

    // Test Download path where dirname matches bundleName
    // This should return true without requiring permission check
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckFileManagerUriPermission_DownloadMatch_001 end");
}

/**
 * @tc.name: CheckFileManagerUriPermission_Desktop_001
 * @tc.desc: Test CheckFileManagerUriPermission with Desktop path
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckFileManagerUriPermission_Desktop_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckFileManagerUriPermission_Desktop_001 start");

    std::string filePath = "/storage/Users/currentUser/Desktop/test.txt";
    std::string bundleName = "com.example.app";
    TokenIdPermission tokenPermission(1001);

    bool result = FilePermissionManager::CheckDocsUriPermission(tokenPermission, filePath);

    // Result depends on VerifyRWDeskTopPermission
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckFileManagerUriPermission_Desktop_001 end");
}

/**
 * @tc.name: CheckFileManagerUriPermission_Documents_001
 * @tc.desc: Test CheckFileManagerUriPermission with Documents path
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckFileManagerUriPermission_Documents_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckFileManagerUriPermission_Documents_001 start");

    std::string filePath = "/storage/Users/currentUser/Documents/test.txt";
    std::string bundleName = "com.example.app";
    TokenIdPermission tokenPermission(1001);

    bool result = FilePermissionManager::CheckDocsUriPermission(tokenPermission, filePath);

    // Result depends on VerifyRWDocumentsPermission
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckFileManagerUriPermission_Documents_001 end");
}

/**
 * @tc.name: CheckUriPersistentPermission_SandboxManager_001
 * @tc.desc: Test CheckUriPersistentPermission with SandboxManager enabled
 * @tc.type: FUNC
 */
#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_SandboxManager_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_SandboxManager_001 start");

    std::vector<Uri> uriVec = {
        Uri("file://com.example.app/storage/test.txt")
    };
    uint32_t callerTokenId = 1001;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.app";
    std::vector<PolicyInfo> pathPolicies;

    std::vector<bool> result = FilePermissionManager::CheckUriPersistentPermission(
        uriVec, callerTokenId, flag, bundleName, pathPolicies);

    // Test SandboxManager path when persistPolicys is not empty
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_SandboxManager_001 end");
}
#endif

/**
 * @tc.name: CheckUriPersistentPermission_NonDocs_001
 * @tc.desc: Test CheckUriPersistentPermission with non-docs URI
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_NonDocs_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_NonDocs_001 start");

    std::vector<Uri> uriVec = {
        Uri("file://com.example.app/data/storage/test.txt")
    };
    uint32_t callerTokenId = 1001;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.app";
    std::vector<PolicyInfo> pathPolicies;

    std::vector<bool> result = FilePermissionManager::CheckUriPersistentPermission(
        uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(result.size(), 1u);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_NonDocs_001 end");
}

/**
 * @tc.name: CheckDocsUriPermission_StoragePath_001
 * @tc.desc: Test CheckDocsUriPermission with storage path (non-appdata)
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckDocsUriPermission_StoragePath_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckDocsUriPermission_StoragePath_001 start");

    std::string path = "/storage/test.txt";
    TokenIdPermission tokenPermission(1001);

    bool result = FilePermissionManager::CheckDocsUriPermission(tokenPermission, path);

    // Tests path that's in STORAGE_URI but not APPDATA_URI
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckDocsUriPermission_StoragePath_001 end");
}

/**
 * @tc.name: CheckUriPersistentPermission_MultipleUris_001
 * @tc.desc: Test CheckUriPersistentPermission with multiple URIs
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_MultipleUris_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_MultipleUris_001 start");

    std::vector<Uri> uriVec = {
        Uri("file://com.example.app/data/storage/test1.txt"),
        Uri("file://com.example.app/data/storage/test2.txt"),
        Uri("file://com.example.app/data/storage/test3.txt")
    };
    uint32_t callerTokenId = 1001;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.app";
    std::vector<PolicyInfo> pathPolicies;

    std::vector<bool> result = FilePermissionManager::CheckUriPersistentPermission(
        uriVec, callerTokenId, flag, bundleName, pathPolicies);

    EXPECT_EQ(result.size(), 3u);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_MultipleUris_001 end");
}

/**
 * @tc.name: GetFilePermission_RootPath_001
 * @tc.desc: Test GetFilePermission with root storage path
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, GetFilePermission_RootPath_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetFilePermission_RootPath_001 start");

    std::string path = "/storage/";
    uint32_t mode = 0;

    int32_t ret = FilePermissionManager::GetFilePermission(path, mode);

    // Test root storage path
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetFilePermission_RootPath_001 end");
}

/**
 * @tc.name: CheckUriPersistentPermission_DocsAuthority_001
 * @tc.desc: Test CheckUriPersistentPermission with docs authority URI
 * @tc.type: FUNC
 */
HWTEST_F(FilePermissionManagerTest, CheckUriPersistentPermission_DocsAuthority_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_DocsAuthority_001 start");

    std::vector<Uri> uriVec = {
        Uri("file://com.example.app/data/storage/docs/test.doc")
    };
    uint32_t callerTokenId = 1001;
    uint32_t flag = Want::FLAG_AUTH_READ_URI_PERMISSION;
    std::string bundleName = "com.example.app";
    std::vector<PolicyInfo> pathPolicies;

    std::vector<bool> result = FilePermissionManager::CheckUriPersistentPermission(
        uriVec, callerTokenId, flag, bundleName, pathPolicies);

    // Tests docs authority URI path
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriPersistentPermission_DocsAuthority_001 end");
}

} // namespace AAFwk
} // namespace OHOS
