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
#include "file_uri_distribution_utils.h"
#undef private

#include "hilog_tag_wrapper.h"
#include "oh_mock_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

class FileUriDistributionUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void FileUriDistributionUtilsTest::SetUpTestCase() {}

void FileUriDistributionUtilsTest::TearDownTestCase() {}

void FileUriDistributionUtilsTest::SetUp() {}

void FileUriDistributionUtilsTest::TearDown() {}

/**
 * @tc.name: GetBundleNameByTokenId_001
 * @tc.desc: Test GetBundleNameByTokenId with valid token
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetBundleNameByTokenId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetBundleNameByTokenId_001 start");

    OH_EXPECT_RET({true}, MockBundleMgrClient, GetBundleNameForUid, const int, std::string&);

    uint32_t tokenId = 1001;
    std::string bundleName;

    bool result = FUDUtils::GetBundleNameByTokenId(tokenId, bundleName);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetBundleNameByTokenId_001 end");
}

/**
 * @tc.name: IsSystemAppCall_001
 * @tc.desc: Test IsSystemAppCall with system app
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSystemAppCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsSystemAppCall_001 start");

    OH_EXPECT_RET({true}, MockAccessTokenKit, IsSystemAppByTokenID, uint32_t);

    bool result = FUDUtils::IsSystemAppCall();

    EXPECT_TRUE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsSystemAppCall_001 end");
}

/**
 * @tc.name: IsSystemAppCall_002
 * @tc.desc: Test IsSystemAppCall with non-system app
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSystemAppCall_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsSystemAppCall_002 start");

    OH_EXPECT_RET({false}, MockAccessTokenKit, IsSystemAppByTokenID, uint32_t);

    bool result = FUDUtils::IsSystemAppCall();

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsSystemAppCall_002 end");
}

/**
 * @tc.name: CheckUriTypeIsValid_001
 * @tc.desc: Test CheckUriTypeIsValid with valid file uri
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckUriTypeIsValid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriTypeIsValid_001 start");

    std::string uriStr = "file://com.example.app/data/storage/el2/base/test.txt";

    bool result = FUDUtils::CheckUriTypeIsValid(uriStr);

    EXPECT_TRUE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriTypeIsValid_001 end");
}

/**
 * @tc.name: CheckUriTypeIsValid_002
 * @tc.desc: Test CheckUriTypeIsValid with invalid uri scheme
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckUriTypeIsValid_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriTypeIsValid_002 start");

    std::string uriStr = "http://com.example.app/data/test.txt";

    bool result = FUDUtils::CheckUriTypeIsValid(uriStr);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriTypeIsValid_002 end");
}

/**
 * @tc.name: IsSAOrSystemAppCall_001
 * @tc.desc: Test IsSAOrSystemAppCall with system ability
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSAOrSystemAppCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsSAOrSystemAppCall_001 start");

    OH_EXPECT_RET({true}, FUDUtils, IsDFSCall);

    bool result = FUDUtils::IsSAOrSystemAppCall();

    EXPECT_TRUE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsSAOrSystemAppCall_001 end");
}

/**
 * @tc.name: IsPrivilegedSACall_001
 * @tc.desc: Test IsPrivilegedSACall
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsPrivilegedSACall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsPrivilegedSACall_001 start");

    OH_EXPECT_RET({true}, FUDUtils, IsSAOrSystemAppCall);

    bool result = FUDUtils::IsPrivilegedSACall();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsPrivilegedSACall_001 end");
}

/**
 * @tc.name: IsUdmfOrPasteboardCall_001
 * @tc.desc: Test IsUdmfOrPasteboardCall
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsUdmfOrPasteboardCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsUdmfOrPasteboardCall_001 start");

    OH_EXPECT_RET({true}, FUDUtils, IsSAOrSystemAppCall);

    bool result = FUDUtils::IsUdmfOrPasteboardCall();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsUdmfOrPasteboardCall_001 end");
}

/**
 * @tc.name: GetTokenIdByBundleName_001
 * @tc.desc: Test GetTokenIdByBundleName
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetTokenIdByBundleName_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetTokenIdByBundleName_001 start");

    OH_EXPECT_RET({true}, MockBundleMgrClient, GetBundleInfo, const std::string&, int32_t, AppExecFwk::BundleInfo&);

    std::string bundleName = "com.example.target";
    int32_t appIndex = 0;
    uint32_t tokenId = 0;

    int32_t ret = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex, tokenId);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetTokenIdByBundleName_001 end");
}

/**
 * @tc.name: GetTokenIdByBundleName_002
 * @tc.desc: Test GetTokenIdByBundleName with invalid bundle name
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetTokenIdByBundleName_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetTokenIdByBundleName_002 start");

    OH_EXPECT_RET({false}, MockBundleMgrClient, GetBundleInfo, const std::string&, int32_t, AppExecFwk::BundleInfo&);

    std::string bundleName = "invalid.bundle.name";
    int32_t appIndex = 0;
    uint32_t tokenId = 0;

    int32_t ret = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex, tokenId);

    EXPECT_NE(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetTokenIdByBundleName_002 end");
}

/**
 * @tc.name: IsDocsCloudUri_001
 * @tc.desc: Test IsDocsCloudUri with docs cloud uri
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsDocsCloudUri_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsDocsCloudUri_001 start");

    Uri uri("file://com.example.app/data/storage/el2/base/docs/test.doc?networkid=test");

    bool result = FUDUtils::IsDocsCloudUri(uri);

    EXPECT_TRUE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsDocsCloudUri_001 end");
}

/**
 * @tc.name: IsDocsCloudUri_002
 * @tc.desc: Test IsDocsCloudUri with local uri
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsDocsCloudUri_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsDocsCloudUri_002 start");

    Uri uri("file://com.example.app/data/storage/el2/base/test.txt");

    bool result = FUDUtils::IsDocsCloudUri(uri);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsDocsCloudUri_002 end");
}

/**
 * @tc.name: CheckCalledBySandBox_001
 * @tc.desc: Test CheckCalledBySandBox
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckCalledBySandBox_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckCalledBySandBox_001 start");

    int32_t ret = FUDUtils::CheckCalledBySandBox();

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckCalledBySandBox_001 end");
}

/**
 * @tc.name: GetBundleNameByTokenId_002
 * @tc.desc: Test GetBundleNameByTokenId with invalid token
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetBundleNameByTokenId_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetBundleNameByTokenId_002 start");

    OH_EXPECT_RET({false}, MockBundleMgrClient, GetBundleNameForUid, const int, std::string&);

    uint32_t tokenId = 0; // Invalid token
    std::string bundleName;

    bool result = FUDUtils::GetBundleNameByTokenId(tokenId, bundleName);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetBundleNameByTokenId_002 end");
}

/**
 * @tc.name: CheckUriTypeIsValid_003
 * @tc.desc: Test CheckUriTypeIsValid with content uri
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckUriTypeIsValid_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriTypeIsValid_003 start");

    std::string uriStr = "content://com.example.app/data/test.txt";

    bool result = FUDUtils::CheckUriTypeIsValid(uriStr);

    EXPECT_TRUE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriTypeIsValid_003 end");
}

/**
 * @tc.name: CheckUriTypeIsValid_004
 * @tc.desc: Test CheckUriTypeIsValid with empty string
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckUriTypeIsValid_004, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriTypeIsValid_004 start");

    std::string uriStr = ""; // Empty string

    bool result = FUDUtils::CheckUriTypeIsValid(uriStr);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriTypeIsValid_004 end");
}

/**
 * @tc.name: IsDFSCall_001
 * @tc.desc: Test IsDFSCall with DFS call
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsDFSCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsDFSCall_001 start");

    bool result = FUDUtils::IsDFSCall();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsDFSCall_001 end");
}

/**
 * @tc.name: IsFoundationCall_001
 * @tc.desc: Test IsFoundationCall
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsFoundationCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsFoundationCall_001 start");

    bool result = FUDUtils::IsFoundationCall();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsFoundationCall_001 end");
}

/**
 * @tc.name: IsPrivilegedSACall_002
 * @tc.desc: Test IsPrivilegedSACall with non-privileged call
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsPrivilegedSACall_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsPrivilegedSACall_002 start");

    OH_EXPECT_RET({false}, FUDUtils, IsSAOrSystemAppCall);

    bool result = FUDUtils::IsPrivilegedSACall();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsPrivilegedSACall_002 end");
}

/**
 * @tc.name: IsUdmfOrPasteboardCall_002
 * @tc.desc: Test IsUdmfOrPasteboardCall with non-udmf call
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsUdmfOrPasteboardCall_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsUdmfOrPasteboardCall_002 start");

    OH_EXPECT_RET({false}, FUDUtils, IsSAOrSystemAppCall);

    bool result = FUDUtils::IsUdmfOrPasteboardCall();

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsUdmfOrPasteboardCall_002 end");
}

/**
 * @tc.name: GetAlterableBundleNameByTokenId_001
 * @tc.desc: Test GetAlterableBundleNameByTokenId
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetAlterableBundleNameByTokenId_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetAlterableBundleNameByTokenId_001 start");

    uint32_t tokenId = 1001;
    std::string alterableBundleName;

    FUDUtils::GetAlterableBundleNameByTokenId(tokenId, alterableBundleName);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetAlterableBundleNameByTokenId_001 end");
}

/**
 * @tc.name: GetDirByBundleNameAndAppIndex_001
 * @tc.desc: Test GetDirByBundleNameAndAppIndex
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetDirByBundleNameAndAppIndex_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetDirByBundleNameAndAppIndex_001 start");

    std::string bundleName = "com.example.app";
    int32_t appIndex = 0;
    std::string dir;

    FUDUtils::GetDirByBundleNameAndAppIndex(bundleName, appIndex, dir);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetDirByBundleNameAndAppIndex_001 end");
}

/**
 * @tc.name: GetTokenIdByBundleName_003
 * @tc.desc: Test GetTokenIdByBundleName with different app index
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetTokenIdByBundleName_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetTokenIdByBundleName_003 start");

    OH_EXPECT_RET({true}, MockBundleMgrClient, GetBundleInfo, const std::string&, int32_t, AppExecFwk::BundleInfo&);

    std::string bundleName = "com.example.target";
    int32_t appIndex = 1; // Different app index
    uint32_t tokenId = 0;

    int32_t ret = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex, tokenId);

    EXPECT_EQ(ret, ERR_OK);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetTokenIdByBundleName_003 end");
}

/**
 * @tc.name: IsDocsCloudUri_003
 * @tc.desc: Test IsDocsCloudUri without networkid
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsDocsCloudUri_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsDocsCloudUri_003 start");

    Uri uri("file://com.example.app/data/storage/el2/base/docs/test.doc");

    bool result = FUDUtils::IsDocsCloudUri(uri);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsDocsCloudUri_003 end");
}

/**
 * @tc.name: IsDistributedCall_001
 * @tc.desc: Test IsDistributedCall
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsDistributedCall_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsDistributedCall_001 start");

    bool result = FUDUtils::IsDistributedCall();

    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsDistributedCall_001 end");
}

/**
 * @tc.name: SendSystemAppGrantUriPermissionEvent_001
 * @tc.desc: Test SendSystemAppGrantUriPermissionEvent
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, SendSystemAppGrantUriPermissionEvent_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "SendSystemAppGrantUriPermissionEvent_001 start");

    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    std::vector<std::string> uriVec = {
        "file://com.example.app/data/storage/el2/base/test.txt"
    };
    std::vector<CheckResult> checkResult;

    FUDUtils::SendSystemAppGrantUriPermissionEvent(callerTokenId, targetTokenId, uriVec, checkResult);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "SendSystemAppGrantUriPermissionEvent_001 end");
}

/**
 * @tc.name: CheckUriTypeIsValid_MultipleSchemes_001
 * @tc.desc: Test CheckUriTypeIsValid with various URI schemes
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckUriTypeIsValid_MultipleSchemes_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriTypeIsValid_MultipleSchemes_001 start");

    std::vector<std::string> validUris = {
        "file://com.example.app/data/test.txt",
        "content://com.example.app/data/test.txt",
        "file://media/image/test.jpg"
    };

    for (auto uri : validUris) {
        bool result = FUDUtils::CheckUriTypeIsValid(uri);
        EXPECT_TRUE(result);
    }

    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckUriTypeIsValid_MultipleSchemes_001 end");
}

/**
 * @tc.name: GetBundleNameByTokenId_MultipleTokens_001
 * @tc.desc: Test GetBundleNameByTokenId with multiple token IDs
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetBundleNameByTokenId_MultipleTokens_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetBundleNameByTokenId_MultipleTokens_001 start");

    OH_EXPECT_RET({true}, MockBundleMgrClient, GetBundleNameForUid, const int, std::string&);

    std::vector<uint32_t> tokenIds = {1001, 1002, 1003};

    for (auto tokenId : tokenIds) {
        std::string bundleName;
        bool result = FUDUtils::GetBundleNameByTokenId(tokenId, bundleName);
    }

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetBundleNameByTokenId_MultipleTokens_001 end");
}

/**
 * @tc.name: SendShareUnPrivilegeUriEvent_CallerFail_001
 * @tc.desc: Test SendShareUnPrivilegeUriEvent when caller token is invalid
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, SendShareUnPrivilegeUriEvent_CallerFail_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "SendShareUnPrivilegeUriEvent_CallerFail_001 start");

    uint32_t callerTokenId = 0;  // Invalid token
    uint32_t targetTokenId = 1002;
    OH_EXPECT_RET({false}, MockBundleMgrClient, GetBundleNameForUid, const int, std::string&);

    bool result = FUDUtils::SendShareUnPrivilegeUriEvent(callerTokenId, targetTokenId);

    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "SendShareUnPrivilegeUriEvent_CallerFail_001 end");
}

/**
 * @tc.name: SendShareUnPrivilegeUriEvent_TargetFail_001
 * @tc.desc: Test SendShareUnPrivilegeUriEvent when target token is invalid
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, SendShareUnPrivilegeUriEvent_TargetFail_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "SendShareUnPrivilegeUriEvent_TargetFail_001 start");

    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 0;  // Invalid token
    OH_EXPECT_RET({true}, MockBundleMgrClient, GetBundleNameForUid, const int, std::string&);
    OH_EXPECT_RET({false}, MockBundleMgrClient, GetBundleNameForUid, const int, std::string&);

    bool result = FUDUtils::SendShareUnPrivilegeUriEvent(callerTokenId, targetTokenId);

    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "SendShareUnPrivilegeUriEvent_TargetFail_001 end");
}

/**
 * @tc.name: SendSystemAppGrantUriPermissionEvent_EmptyResult_001
 * @tc.desc: Test SendSystemAppGrantUriPermissionEvent with empty result vector
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, SendSystemAppGrantUriPermissionEvent_EmptyResult_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "SendSystemAppGrantUriPermissionEvent_EmptyResult_001 start");

    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    std::vector<std::string> uriVec;
    std::vector<CheckResult> checkResult;  // Empty vector

    bool result = FUDUtils::SendSystemAppGrantUriPermissionEvent(callerTokenId, targetTokenId, uriVec, checkResult);

    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "SendSystemAppGrantUriPermissionEvent_EmptyResult_001 end");
}

/**
 * @tc.name: CheckAndCreateEventInfo_CallerNotSystem_001
 * @tc.desc: Test CheckAndCreateEventInfo when caller is not system app
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckAndCreateEventInfo_CallerNotSystem_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckAndCreateEventInfo_CallerNotSystem_001 start");

    uint32_t callerTokenId = 1001;  // Non-system app
    uint32_t targetTokenId = 1002;
    EventInfo eventInfo;
    OH_EXPECT_RET({true}, MockBundleMgrClient, GetBundleNameForUid, const int, std::string&);
    OH_EXPECT_RET({false}, MockTokenIdKit, IsSystemAppByFullTokenID, uint64_t);

    bool result = FUDUtils::CheckAndCreateEventInfo(callerTokenId, targetTokenId, eventInfo);

    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckAndCreateEventInfo_CallerNotSystem_001 end");
}

/**
 * @tc.name: CheckAndCreateEventInfo_TargetIsSystem_001
 * @tc.desc: Test CheckAndCreateEventInfo when target is system app
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckAndCreateEventInfo_TargetIsSystem_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckAndCreateEventInfo_TargetIsSystem_001 start");

    uint32_t callerTokenId = 1001;  // System app
    uint32_t targetTokenId = 1002;  // System app
    EventInfo eventInfo;
    OH_EXPECT_RET({true}, MockBundleMgrClient, GetBundleNameForUid, const int, std::string&);
    OH_EXPECT_RET({true}, MockTokenIdKit, IsSystemAppByFullTokenID, uint64_t);

    bool result = FUDUtils::CheckAndCreateEventInfo(callerTokenId, targetTokenId, eventInfo);

    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckAndCreateEventInfo_TargetIsSystem_001 end");
}

/**
 * @tc.name: GetCurrentAccountId_QueryFailed_001
 * @tc.desc: Test GetCurrentAccountId when QueryActiveOsAccountIds fails
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetCurrentAccountId_QueryFailed_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetCurrentAccountId_QueryFailed_001 start");

    OH_EXPECT_RET({ERR_OK}, MockOsAccountManagerWrapper, QueryActiveOsAccountIds, std::vector<int32_t>&);

    int32_t accountId = FUDUtils::GetCurrentAccountId();

    EXPECT_EQ(accountId, 0);  // DEFAULT_USER_ID
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetCurrentAccountId_QueryFailed_001 end");
}

/**
 * @tc.name: GetCurrentAccountId_EmptyResult_001
 * @tc.desc: Test GetCurrentAccountId when account list is empty
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetCurrentAccountId_EmptyResult_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetCurrentAccountId_EmptyResult_001 start");

    OH_EXPECT_RET({ERR_OK}, MockOsAccountManagerWrapper, QueryActiveOsAccountIds, std::vector<int32_t>&);

    int32_t accountId = FUDUtils::GetCurrentAccountId();

    EXPECT_EQ(accountId, 0);  // DEFAULT_USER_ID
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetCurrentAccountId_EmptyResult_001 end");
}

/**
 * @tc.name: CheckIsSystemAppByBundleName_NullHelper_001
 * @tc.desc: Test CheckIsSystemAppByBundleName when bundleMgrHelper is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckIsSystemAppByBundleName_NullHelper_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckIsSystemAppByBundleName_NullHelper_001 start");

    std::string bundleName = "com.example.app";
    // Mock BundleMgrHelper returning nullptr

    bool result = FUDUtils::CheckIsSystemAppByBundleName(bundleName);

    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckIsSystemAppByBundleName_NullHelper_001 end");
}

/**
 * @tc.name: CheckIsSystemAppByBundleName_GetAppInfoFail_001
 * @tc.desc: Test CheckIsSystemAppByBundleName when GetApplicationInfo fails
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckIsSystemAppByBundleName_GetAppInfoFail_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckIsSystemAppByBundleName_GetAppInfoFail_001 start");

    std::string bundleName = "com.example.app";
    OH_EXPECT_RET({false}, MockBundleMgrHelper, GetApplicationInfo, const std::string&, int32_t, int32_t, AppExecFwk::ApplicationInfo&);

    bool result = FUDUtils::CheckIsSystemAppByBundleName(bundleName);

    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "CheckIsSystemAppByBundleName_GetAppInfoFail_001 end");
}

/**
 * @tc.name: GetBundleApiTargetVersion_NullHelper_001
 * @tc.desc: Test GetBundleApiTargetVersion when bundleMgrHelper is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetBundleApiTargetVersion_NullHelper_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetBundleApiTargetVersion_NullHelper_001 start");

    std::string bundleName = "com.example.app";
    int32_t targetApiVersion;
    // Mock BundleMgrHelper returning nullptr

    bool result = FUDUtils::GetBundleApiTargetVersion(bundleName, targetApiVersion);

    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetBundleApiTargetVersion_NullHelper_001 end");
}

/**
 * @tc.name: GetTokenIdByBundleName_NullBms_001
 * @tc.desc: Test GetTokenIdByBundleName when bms is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetTokenIdByBundleName_NullBms_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetTokenIdByBundleName_NullBms_001 start");

    std::string bundleName = "com.example.app";
    int32_t appIndex = 0;
    uint32_t tokenId;
    // Mock BundleMgrHelper returning nullptr

    int32_t ret = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex, tokenId);

    EXPECT_NE(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetTokenIdByBundleName_NullBms_001 end");
}

/**
 * @tc.name: GetTokenIdByBundleName_SandboxIndex_001
 * @tc.desc: Test GetTokenIdByBundleName with sandbox app index
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetTokenIdByBundleName_SandboxIndex_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetTokenIdByBundleName_SandboxIndex_001 start");

    std::string bundleName = "com.example.app";
    int32_t appIndex = 100;  // > MAX_APP_CLONE_INDEX
    uint32_t tokenId;
    OH_EXPECT_RET({ERR_OK}, MockBundleMgrHelper, GetSandboxBundleInfo, const std::string&, int32_t, int32_t, AppExecFwk::BundleInfo&);

    int32_t ret = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex, tokenId);

    EXPECT_EQ(ret, ERR_OK);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetTokenIdByBundleName_SandboxIndex_001 end");
}

/**
 * @tc.name: GenerateFUDAppInfo_NotHapToken_001
 * @tc.desc: Test GenerateFUDAppInfo with non-HAP token
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GenerateFUDAppInfo_NotHapToken_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GenerateFUDAppInfo_NotHapToken_001 start");

    FUDAppInfo info;
    info.tokenId = 1001;
    OH_EXPECT_RET({ATokenTypeEnum::TOKEN_NATIVE}, MockAccessTokenKit, GetTokenTypeFlag, uint64_t);

    bool result = FUDUtils::GenerateFUDAppInfo(info);

    EXPECT_FALSE(result);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GenerateFUDAppInfo_NotHapToken_001 end");
}

/**
 * @tc.name: IsSandboxApp_SandboxIndex_001
 * @tc.desc: Test IsSandboxApp with sandbox app index
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSandboxApp_SandboxIndex_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsSandboxApp_SandboxIndex_001 start");

    uint32_t tokenId = 1001;
    OH_EXPECT_RET({ATokenTypeEnum::TOKEN_HAP}, MockAccessTokenKit, GetTokenTypeFlag, uint64_t);
    // Mock HapTokenInfo with instIndex > MAX_APP_CLONE_INDEX

    bool result = FUDUtils::IsSandboxApp(tokenId);

    // Result depends on mocked instIndex value
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsSandboxApp_SandboxIndex_001 end");
}

/**
 * @tc.name: IsUdmfOrPasteboardCall_UdmfUid_001
 * @tc.desc: Test IsUdmfOrPasteboardCall with UDMF UID
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsUdmfOrPasteboardCall_UdmfUid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsUdmfOrPasteboardCall_UdmfUid_001 start");

    OH_EXPECT_RET({3012}, MockIPCSkeleton, GetCallingUid);

    bool result = FUDUtils::IsUdmfOrPasteboardCall();

    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsUdmfOrPasteboardCall_UdmfUid_001 end");
}

/**
 * @tc.name: IsBrokerCaller_BrokerUid_001
 * @tc.desc: Test IsBrokerCaller with broker pasteboard UID
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsBrokerCaller_BrokerUid_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsBrokerCaller_BrokerUid_001 start");

    OH_EXPECT_RET({5557}, MockIPCSkeleton, GetCallingUid);

    bool result = FUDUtils::IsBrokerCaller();

    EXPECT_TRUE(result);
    TAG_LOGI(AAFwkTag::URIPERMMGR, "IsBrokerCaller_BrokerUid_001 end");
}

} // namespace AAFwk
} // namespace OHOS
