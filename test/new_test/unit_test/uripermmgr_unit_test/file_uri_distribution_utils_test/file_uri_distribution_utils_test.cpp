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
#include "file_uri_distribution_utils.h"
#include "permission_verification.h"
#include "accesstoken_kit.h"
#include "tokenid_kit.h"
#include "bundle_mgr_helper.h"
#include "bundle_mgr_client.h"
#include "os_account_manager_wrapper.h"
#include "event_report.h"
#include "ipc_skeleton.h"
#include "fud_constants.h"
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

// ============================================================================
// Event Reporting Methods
// ============================================================================

/**
 * @tc.name: SendShareUnPrivilegeUriEvent_Success_001
 * @tc.desc: Verify SendShareUnPrivilegeUriEvent succeeds.
 *           Branch: Both GetBundleNameByTokenId succeed
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, SendShareUnPrivilegeUriEvent_Success_001, TestSize.Level1)
{
    uint32_t callTokenId = 1001;
    uint32_t targetTokenId = 1002;

    // Mock GetBundleNameByTokenId to succeed twice
    OH_GLOBAL_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, IPCSkeleton::GetCallingTokenID);
    OH_EXPECT_RET({static_cast<int>(Security::AccessToken::ATokenTypeEnum::TOKEN_HAP)},
        Security::AccessToken::AccessTokenKit, GetTokenTypeFlag, uint32_t);
    Security::AccessToken::HapTokenInfo hapInfo1;
    hapInfo1.bundleName = "com.example.caller";
    OH_EXPECT_RET({static_cast<int>(Security::AccessToken::AccessTokenKitRet::RET_SUCCESS)},
        Security::AccessToken::AccessTokenKit, GetHapTokenInfo, uint32_t, Security::AccessToken::HapTokenInfo&);

    Security::AccessToken::HapTokenInfo hapInfo2;
    hapInfo2.bundleName = "com.example.target";
    OH_EXPECT_RET({static_cast<int>(Security::AccessToken::AccessTokenKitRet::RET_SUCCESS)},
        Security::AccessToken::AccessTokenKit, GetHapTokenInfo, uint32_t, Security::AccessToken::HapTokenInfo&);

    bool result = FUDUtils::SendShareUnPrivilegeUriEvent(callTokenId, targetTokenId);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: SendShareUnPrivilegeUriEvent_FirstGetBundleNameFails_002
 * @tc.desc: Verify SendShareUnPrivilegeUriEvent fails when first GetBundleNameByTokenId fails.
 *           Branch: First GetBundleNameByTokenId fails
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, SendShareUnPrivilegeUriEvent_FirstGetBundleNameFails_002, TestSize.Level1)
{
    uint32_t callTokenId = 1001;
    uint32_t targetTokenId = 1002;

    OH_GLOBAL_EXPECT_RET({static_cast<uint32_t>(callTokenId)}, IPCSkeleton::GetCallingTokenID);
    OH_EXPECT_RET({static_cast<int>(Security::AccessToken::ATokenTypeEnum::TOKEN_HAP)},
        Security::AccessToken::AccessTokenKit, GetTokenTypeFlag, uint32_t);
    OH_EXPECT_RET({static_cast<int>(Security::AccessToken::AccessTokenKitRet::RET_FAILED)},
        Security::AccessToken::AccessTokenKit, GetHapTokenInfo, uint32_t, Security::AccessToken::HapTokenInfo&);

    bool result = FUDUtils::SendShareUnPrivilegeUriEvent(callTokenId, targetTokenId);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: SendSystemAppGrantUriPermissionEvent_Success_001
 * @tc.desc: Verify SendSystemAppGrantUriPermissionEvent succeeds.
 *           Branch: CheckAndCreateEventInfo succeeds, at least one result true
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, SendSystemAppGrantUriPermissionEvent_Success_001, TestSize.Level1)
{
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    std::vector<std::string> uriVec = {"file://media/test.jpg"};
    std::vector<CheckResult> resVec = {CheckResult()};

    // Mock GetBundleNameByTokenId to succeed
    OH_EXPECT_RET({static_cast<int>(Security::AccessToken::ATokenTypeEnum::TOKEN_HAP)},
        Security::AccessToken::AccessTokenKit, GetTokenTypeFlag, uint32_t);
    Security::AccessToken::HapTokenInfo hapInfo;
    hapInfo.bundleName = "com.example.caller";
    OH_EXPECT_RET({static_cast<int>(Security::AccessToken::AccessTokenKitRet::RET_SUCCESS)},
        Security::AccessToken::AccessTokenKit, GetHapTokenInfo, uint32_t, Security::AccessToken::HapTokenInfo&);
    OH_EXPECT_RET({true}, Security::AccessToken::TokenIdKit, IsSystemAppByFullTokenID, uint64_t);

    bool result = FUDUtils::SendSystemAppGrantUriPermissionEvent(callerTokenId, targetTokenId, uriVec, resVec);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: SendSystemAppGrantUriPermissionEvent_EmptyVectors_002
 * @tc.desc: Verify SendSystemAppGrantUriPermissionEvent with empty vectors.
 *           Branch: uriVec empty
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, SendSystemAppGrantUriPermissionEvent_EmptyVectors_002, TestSize.Level1)
{
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    std::vector<std::string> uriVec;
    std::vector<CheckResult> resVec;

    bool result = FUDUtils::SendSystemAppGrantUriPermissionEvent(callerTokenId, targetTokenId, uriVec, resVec);

    EXPECT_EQ(result, false);
}

// ============================================================================
// Account & UID Methods
// ============================================================================

/**
 * @tc.name: GetCurrentAccountId_Success_001
 * @tc.desc: Verify GetCurrentAccountId succeeds.
 *           Branch: QueryActiveOsAccountIds returns ERR_OK, not empty
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetCurrentAccountId_Success_001, TestSize.Level1)
{
    std::vector<int32_t> accountIds = {100};
    OH_GLOBAL_EXPECT_RET({static_cast<int32_t>(ERR_OK)},
        AccountSandbox::OsAccountManagerWrapper, QueryActiveOsAccountIds, std::vector<int32_t>&);

    int32_t accountId = FUDUtils::GetCurrentAccountId();

    EXPECT_GT(accountId, 0);
}

/**
 * @tc.name: GetCurrentAccountId_QueryFails_002
 * @tc.desc: Verify GetCurrentAccountId fails when QueryActiveOsAccountIds fails.
 *           Branch: ret != ERR_OK
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetCurrentAccountId_QueryFails_002, TestSize.Level1)
{
    OH_GLOBAL_EXPECT_RET({static_cast<int32_t>(INNER_ERR)},
        AccountSandbox::OsAccountManagerWrapper, QueryActiveOsAccountIds, std::vector<int32_t>&);

    int32_t accountId = FUDUtils::GetCurrentAccountId();

    EXPECT_EQ(accountId, 0);  // DEFAULT_USER_ID
}

/**
 * @tc.name: IsFoundationCall_True_001
 * @tc.desc: Verify IsFoundationCall returns true.
 *           Branch: UID == FOUNDATION_UID
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsFoundationCall_True_001, TestSize.Level1)
{
    OH_GLOBAL_EXPECT_RET({static_cast<int32_t>(FUDConstants::FOUNDATION_UID)}, IPCSkeleton::GetCallingUid);

    bool result = FUDUtils::IsFoundationCall();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsFoundationCall_False_002
 * @tc.desc: Verify IsFoundationCall returns false.
 *           Branch: UID != FOUNDATION_UID
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsFoundationCall_False_002, TestSize.Level1)
{
    OH_GLOBAL_EXPECT_RET({static_cast<int32_t>(1000)}, IPCSkeleton::GetCallingUid);

    bool result = FUDUtils::IsFoundationCall();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: IsSAOrSystemAppCall_SystemApp_001
 * @tc.desc: Verify IsSAOrSystemAppCall returns true for system app.
 *           Branch: IsSystemAppCall returns true
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSAOrSystemAppCall_SystemApp_001, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, IsSystemAppCall);

    bool result = FUDUtils::IsSAOrSystemAppCall();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsSAOrSystemAppCall_SA_002
 * @tc.desc: Verify IsSAOrSystemAppCall returns true for SA.
 *           Branch: IsSACall returns true
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSAOrSystemAppCall_SA_002, TestSize.Level1)
{
    OH_EXPECT_RET({false}, PermissionVerification, IsSystemAppCall);
    OH_EXPECT_RET({true}, PermissionVerification, IsSACall);

    bool result = FUDUtils::IsSAOrSystemAppCall();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsPrivilegedSACall_HasPermission_001
 * @tc.desc: Verify IsPrivilegedSACall returns true with permission.
 *           Branch: Has GRANT_URI_PERMISSION_PRIVILEGED
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsPrivilegedSACall_HasPermission_001, TestSize.Level1)
{
    OH_GLOBAL_EXPECT_RET({static_cast<int32_t>(1000)}, IPCSkeleton::GetCallingUid);
    OH_EXPECT_RET({true}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    bool result = FUDUtils::IsPrivilegedSACall();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsPrivilegedSACall_CollaborationUID_002
 * @tc.desc: Verify IsPrivilegedSACall returns true for collaboration UID.
 *           Branch: UID == COLLABORATION_FWK_UID
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsPrivilegedSACall_CollaborationUID_002, TestSize.Level1)
{
    OH_GLOBAL_EXPECT_RET({static_cast<int32_t>(FUDConstants::COLLABORATION_FWK_UID)}, IPCSkeleton::GetCallingUid);
    OH_EXPECT_RET({false}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    bool result = FUDUtils::IsPrivilegedSACall();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsUdmfOrPasteboardCall_UDMF_001
 * @tc.desc: Verify IsUdmfOrPasteboardCall returns true for UDMF.
 *           Branch: UID == UDMF_UID
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsUdmfOrPasteboardCall_UDMF_001, TestSize.Level1)
{
    OH_GLOBAL_EXPECT_RET({static_cast<int32_t>(FUDConstants::UDMF_UID)}, IPCSkeleton::GetCallingUid);

    bool result = FUDUtils::IsUdmfOrPasteboardCall();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsDFSCall_True_001
 * @tc.desc: Verify IsDFSCall returns true.
 *           Branch: UID == DFS_UID
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsDFSCall_True_001, TestSize.Level1)
{
    OH_GLOBAL_EXPECT_RET({static_cast<int32_t>(FUDConstants::DFS_UID)}, IPCSkeleton::GetCallingUid);

    bool result = FUDUtils::IsDFSCall();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsBrokerCaller_True_001
 * @tc.desc: Verify IsBrokerCaller returns true.
 *           Branch: UID == BROKER_PASTEBOARD_UID
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsBrokerCaller_True_001, TestSize.Level1)
{
    OH_GLOBAL_EXPECT_RET({static_cast<int32_t>(FUDConstants::BROKER_PASTEBOARD_UID)}, IPCSkeleton::GetCallingUid);

    bool result = FUDUtils::IsBrokerCaller();

    EXPECT_EQ(result, true);
}

// ============================================================================
// Bundle & Token Methods
// ============================================================================

/**
 * @tc.name: CheckIsSystemAppByBundleName_Success_001
 * @tc.desc: Verify CheckIsSystemAppByBundleName succeeds.
 *           Branch: GetApplicationInfo succeeds, IsSystemAppByFullTokenID true
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckIsSystemAppByBundleName_Success_001, TestSize.Level1)
{
    std::string bundleName = "com.example.system";

    // Mock BundleMgrHelper
    auto mockHelper = sptr<AppExecFwk::BundleMgrHelper>::MakeSptr();
    OH_MOCK_METHOD_RET_SPTR(sptr<AppExecFwk::BundleMgrHelper>, AppExecFwk::BundleMgrHelper, GetBundleMgrHelper);
    OH_EXPECT_RET({mockHelper}, AppExecFwk::BundleMgrHelper, GetBundleMgrHelper);

    AppExecFwk::ApplicationInfo appInfo;
    appInfo.isSystemApp = true;
    OH_EXPECT_RET_AND_OUTPUT({true}, {{appInfo}}, AppExecFwk::BundleMgrHelper, GetApplicationInfo,
        const std::string&, int32_t, int32_t, AppExecFwk::ApplicationInfo&);

    OH_EXPECT_RET({true}, Security::AccessToken::TokenIdKit, IsSystemAppByFullTokenID, uint64_t);

    bool result = FUDUtils::CheckIsSystemAppByBundleName(bundleName);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: GetBundleApiTargetVersion_Success_001
 * @tc.desc: Verify GetBundleApiTargetVersion succeeds.
 *           Branch: GetApplicationInfo succeeds
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetBundleApiTargetVersion_Success_001, TestSize.Level1)
{
    std::string bundleName = "com.example.app";
    int32_t targetApiVersion = 0;

    auto mockHelper = sptr<AppExecFwk::BundleMgrHelper>::MakeSptr();
    OH_MOCK_METHOD_RET_SPTR(sptr<AppExecFwk::BundleMgrHelper>, AppExecFwk::BundleMgrHelper, GetBundleMgrHelper);
    OH_EXPECT_RET({mockHelper}, AppExecFwk::BundleMgrHelper, GetBundleMgrHelper);

    AppExecFwk::ApplicationInfo appInfo;
    appInfo.apiTargetVersion = 9;
    OH_EXPECT_RET_AND_OUTPUT({true}, {{appInfo}}, AppExecFwk::BundleMgrHelper, GetApplicationInfo,
        const std::string&, int32_t, int32_t, AppExecFwk::ApplicationInfo&);

    bool result = FUDUtils::GetBundleApiTargetVersion(bundleName, targetApiVersion);

    EXPECT_EQ(result, true);
    EXPECT_EQ(targetApiVersion, 9);
}

/**
 * @tc.name: GetBundleNameByTokenId_Success_001
 * @tc.desc: Verify GetBundleNameByTokenId succeeds.
 *           Branch: TOKEN_HAP, GetHapTokenInfo succeeds
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetBundleNameByTokenId_Success_001, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    std::string bundleName;

    OH_EXPECT_RET({static_cast<int>(Security::AccessToken::ATokenTypeEnum::TOKEN_HAP)},
        Security::AccessToken::AccessTokenKit, GetTokenTypeFlag, uint32_t);

    Security::AccessToken::HapTokenInfo hapInfo;
    hapInfo.bundleName = "com.example.app";
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(Security::AccessToken::AccessTokenKitRet::RET_SUCCESS)},
        {{hapInfo}}, Security::AccessToken::AccessTokenKit, GetHapTokenInfo, uint32_t,
        Security::AccessToken::HapTokenInfo&);

    bool result = FUDUtils::GetBundleNameByTokenId(tokenId, bundleName);

    EXPECT_EQ(result, true);
    EXPECT_EQ(bundleName, "com.example.app");
}

/**
 * @tc.name: GetTokenIdByBundleName_AppIndex0_Success_001
 * @tc.desc: Verify GetTokenIdByBundleName succeeds with appIndex 0.
 *           Branch: appIndex == 0, GetBundleInfo succeeds
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetTokenIdByBundleName_AppIndex0_Success_001, TestSize.Level1)
{
    std::string bundleName = "com.example.app";
    int32_t appIndex = 0;
    uint32_t tokenId = 0;

    auto mockHelper = sptr<AppExecFwk::BundleMgrHelper>::MakeSptr();
    OH_MOCK_METHOD_RET_SPTR(sptr<AppExecFwk::BundleMgrHelper>, AppExecFwk::BundleMgrHelper, GetBundleMgrHelper);
    OH_EXPECT_RET({mockHelper}, AppExecFwk::BundleMgrHelper, GetBundleMgrHelper);

    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.applicationInfo.applicationInfo.accessTokenIdEx = 1001;
    OH_EXPECT_RET_AND_OUTPUT({true}, {{bundleInfo}}, AppExecFwk::BundleMgrHelper, GetBundleInfo,
        const std::string&, int32_t, AppExecFwk::BundleInfo&, int32_t);

    int32_t result = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex, tokenId);

    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetTokenIdByBundleName_Clone_Success_002
 * @tc.desc: Verify GetTokenIdByBundleName succeeds with clone app.
 *           Branch: appIndex <= MAX_APP_CLONE_INDEX
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetTokenIdByBundleName_Clone_Success_002, TestSize.Level1)
{
    std::string bundleName = "com.example.app";
    int32_t appIndex = 1;
    uint32_t tokenId = 0;

    auto mockHelper = sptr<AppExecFwk::BundleMgrHelper>::MakeSptr();
    OH_MOCK_METHOD_RET_SPTR(sptr<AppExecFwk::BundleMgrHelper>, AppExecFwk::BundleMgrHelper, GetBundleMgrHelper);
    OH_EXPECT_RET({mockHelper}, AppExecFwk::BundleMgrHelper, GetBundleMgrHelper);

    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.applicationInfo.applicationInfo.accessTokenIdEx = 1002;
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int32_t>(ERR_OK)}, {{bundleInfo}}, AppExecFwk::BundleMgrHelper, GetCloneBundleInfo,
        const std::string&, int32_t, int32_t, AppExecFwk::BundleInfo&, int32_t);

    int32_t result = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex, tokenId);

    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: GetTokenIdByBundleName_Sandbox_Success_003
 * @tc.desc: Verify GetTokenIdByBundleName succeeds with sandbox app.
 *           Branch: appIndex > MAX_APP_CLONE_INDEX
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetTokenIdByBundleName_Sandbox_Success_003, TestSize.Level1)
{
    std::string bundleName = "com.example.app";
    int32_t appIndex = 999;  // > MAX_APP_CLONE_INDEX
    uint32_t tokenId = 0;

    auto mockHelper = sptr<AppExecFwk::BundleMgrHelper>::MakeSptr();
    OH_MOCK_METHOD_RET_SPTR(sptr<AppExecFwk::BundleMgrHelper>, AppExecFwk::BundleMgrHelper, GetBundleMgrHelper);
    OH_EXPECT_RET({mockHelper}, AppExecFwk::BundleMgrHelper, GetBundleMgrHelper);

    AppExecFwk::BundleInfo bundleInfo;
    bundleInfo.applicationInfo.applicationInfo.accessTokenIdEx = 1003;
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int32_t>(ERR_OK)}, {{bundleInfo}}, AppExecFwk::BundleMgrHelper, GetSandboxBundleInfo,
        const std::string&, int32_t, int32_t, AppExecFwk::BundleInfo&);

    int32_t result = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex, tokenId);

    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: IsSandboxApp_SandboxApp_001
 * @tc.desc: Verify IsSandboxApp returns true for sandbox app.
 *           Branch: TOKEN_HAP, instIndex > MAX_APP_CLONE_INDEX
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSandboxApp_SandboxApp_001, TestSize.Level1)
{
    uint32_t tokenId = 1001;

    OH_EXPECT_RET({static_cast<int>(Security::AccessToken::ATokenTypeEnum::TOKEN_HAP)},
        Security::AccessToken::AccessTokenKit, GetTokenTypeFlag, uint32_t);

    Security::AccessToken::HapTokenInfo hapInfo;
    hapInfo.instIndex = 999;  // > MAX_APP_CLONE_INDEX
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(Security::AccessToken::AccessTokenKitRet::RET_SUCCESS)},
        {{hapInfo}}, Security::AccessToken::AccessTokenKit, GetHapTokenInfo, uint32_t,
        Security::AccessToken::HapTokenInfo&);

    bool result = FUDUtils::IsSandboxApp(tokenId);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsSandboxApp_NotSandboxApp_002
 * @tc.desc: Verify IsSandboxApp returns false for non-sandbox app.
 *           Branch: TOKEN_HAP, instIndex <= MAX_APP_CLONE_INDEX
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSandboxApp_NotSandboxApp_002, TestSize.Level1)
{
    uint32_t tokenId = 1001;

    OH_EXPECT_RET({static_cast<int>(Security::AccessToken::ATokenTypeEnum::TOKEN_HAP)},
        Security::AccessToken::AccessTokenKit, GetTokenTypeFlag, uint32_t);

    Security::AccessToken::HapTokenInfo hapInfo;
    hapInfo.instIndex = 0;
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(Security::AccessToken::AccessTokenKitRet::RET_SUCCESS)},
        {{hapInfo}}, Security::AccessToken::AccessTokenKit, GetHapTokenInfo, uint32_t,
        Security::AccessToken::HapTokenInfo&);

    bool result = FUDUtils::IsSandboxApp(tokenId);

    EXPECT_EQ(result, false);
}

// ============================================================================
// URI Validation Methods
// ============================================================================

/**
 * @tc.name: CheckUriTypeIsValid_FileScheme_001
 * @tc.desc: Verify CheckUriTypeIsValid returns true for file scheme.
 *           Branch: scheme == FILE_SCHEME
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckUriTypeIsValid_FileScheme_001, TestSize.Level1)
{
    Uri uri("file://media/test.jpg");

    bool result = FUDUtils::CheckUriTypeIsValid(uri);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: CheckUriTypeIsValid_ContentScheme_002
 * @tc.desc: Verify CheckUriTypeIsValid returns true for content scheme.
 *           Branch: scheme == CONTENT_SCHEME
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckUriTypeIsValid_ContentScheme_002, TestSize.Level1)
{
    Uri uri("content://media/test.jpg");

    bool result = FUDUtils::CheckUriTypeIsValid(uri);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: CheckUriTypeIsValid_InvalidScheme_003
 * @tc.desc: Verify CheckUriTypeIsValid returns false for invalid scheme.
 *           Branch: scheme != FILE_SCHEME && scheme != CONTENT_SCHEME
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckUriTypeIsValid_InvalidScheme_003, TestSize.Level1)
{
    Uri uri("http://example.com/file.jpg");

    bool result = FUDUtils::CheckUriTypeIsValid(uri);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: IsDocsCloudUri_Valid_001
 * @tc.desc: Verify IsDocsCloudUri returns true for valid cloud URI.
 *           Branch: authority == DOCS_AUTHORITY and contains networkId
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsDocsCloudUri_Valid_001, TestSize.Level1)
{
    Uri uri("file://docs/test.txt?networkId=123");

    bool result = FUDUtils::IsDocsCloudUri(uri);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsDocsCloudUri_NoNetworkId_002
 * @tc.desc: Verify IsDocsCloudUri returns false when no networkId.
 *           Branch: authority == DOCS_AUTHORITY but no networkId
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsDocsCloudUri_NoNetworkId_002, TestSize.Level1)
{
    Uri uri("file://docs/test.txt");

    bool result = FUDUtils::IsDocsCloudUri(uri);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: IsDocsCloudUri_NotDocs_003
 * @tc.desc: Verify IsDocsCloudUri returns false for non-docs URI.
 *           Branch: authority != DOCS_AUTHORITY
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsDocsCloudUri_NotDocs_003, TestSize.Level1)
{
    Uri uri("file://media/test.jpg");

    bool result = FUDUtils::IsDocsCloudUri(uri);

    EXPECT_EQ(result, false);
}

}  // namespace AAFwk
}  // namespace OHOS
