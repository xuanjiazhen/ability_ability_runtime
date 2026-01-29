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
#include "ability_manager_errors.h"
#include "accesstoken_kit.h"
#include "bundle_mgr_client.h"
#include "bundle_mgr_helper.h"
#include "event_report.h"
#include "file_uri_distribution_utils.h"
#include "fud_constants.h"
#include "global_constant.h"
#include "ipc_skeleton.h"
#include "oh_mock_utils.h"
#include "os_account_manager_wrapper.h"
#include "permission_verification.h"
#include "tokenid_kit.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::Security;
using namespace OHOS::Security::AccessToken;
using namespace OHOS::AppExecFwk;

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
    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);
    HapTokenInfo hapInfo1;
    hapInfo1.bundleName = "com.example.caller";
    auto expectHapInfo1 = std::vector<HapTokenInfo>{{hapInfo1}};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo1, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

    HapTokenInfo hapInfo2;
    hapInfo2.bundleName = "com.example.target";
    auto expectHapInfo2 = std::vector<HapTokenInfo>{{hapInfo2}};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo2, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

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
    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);
    OH_EXPECT_RET({static_cast<int>(AccessTokenKitRet::RET_FAILED)},
        AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

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
    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);
    HapTokenInfo hapInfo;
    hapInfo.bundleName = "com.example.caller";
    auto expectHapInfo = std::vector<HapTokenInfo>{hapInfo};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);
    OH_EXPECT_RET({true}, TokenIdKit, IsSystemAppByFullTokenID, uint64_t);

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

/**
 * @tc.name: SendSystemAppGrantUriPermissionEvent_AllResultsFalse_003
 * @tc.desc: Verify SendSystemAppGrantUriPermissionEvent returns false when all results are false.
 *           Branch: No resVec[i].result is true
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, SendSystemAppGrantUriPermissionEvent_AllResultsFalse_003, TestSize.Level1)
{
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    std::vector<std::string> uriVec = {"file://media/test.jpg"};
    std::vector<CheckResult> resVec = {CheckResult()}; // result defaults to false

    // Mock CheckAndCreateEventInfo to succeed
    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);
    HapTokenInfo hapInfo1;
    hapInfo1.bundleName = "com.example.caller";
    auto expectHapInfo1 = std::vector<HapTokenInfo>{{hapInfo1}};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo1, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);
    OH_EXPECT_RET({true}, TokenIdKit, IsSystemAppByFullTokenID, uint64_t);

    HapTokenInfo hapInfo2;
    hapInfo2.bundleName = "com.example.target";
    auto expectHapInfo2 = std::vector<HapTokenInfo>{{hapInfo2}};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo2, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);
    OH_EXPECT_RET({false}, TokenIdKit, IsSystemAppByFullTokenID, uint64_t);

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
    auto mockOsAccount = new OsAccountManagerWrapper();

    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, OsAccountManagerWrapper, QueryActiveOsAccountIds,
        std::vector<int32_t>&);

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
        OsAccountManagerWrapper, QueryActiveOsAccountIds, std::vector<int32_t>&);

    int32_t accountId = FUDUtils::GetCurrentAccountId();

    EXPECT_EQ(accountId, 0);  // DEFAULT_USER_ID
}

/**
 * @tc.name: GetCurrentAccountId_EmptyArray_003
 * @tc.desc: Verify GetCurrentAccountId returns DEFAULT_USER_ID when QueryActiveOsAccountIds
 *           returns empty array.
 *           Branch: osActiveAccountIds.empty()
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetCurrentAccountId_EmptyArray_003, TestSize.Level1)
{
    auto mockOsAccount = new OsAccountManagerWrapper();

    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, OsAccountManagerWrapper, QueryActiveOsAccountIds,
        std::vector<int32_t>&);

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
 * @tc.name: IsPrivilegedSACall_NoPermission_NoCollaborationUID_003
 * @tc.desc: Verify IsPrivilegedSACall returns false when no permission and UID is not
 *           COLLABORATION_FWK_UID.
 *           Branch: No permission, UID != COLLABORATION_FWK_UID
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsPrivilegedSACall_NoPermission_NoCollaborationUID_003, TestSize.Level1)
{
    OH_GLOBAL_EXPECT_RET({static_cast<int32_t>(9999)}, IPCSkeleton::GetCallingUid);
    OH_GLOBAL_EXPECT_RET({static_cast<uint32_t>(1001)}, IPCSkeleton::GetCallingTokenID);
    OH_EXPECT_RET({false}, PermissionVerification, VerifyPermissionByTokenId, const int&, const std::string&);

    bool result = FUDUtils::IsPrivilegedSACall();

    EXPECT_EQ(result, false);
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
    auto mockHelper = new BundleMgrHelper();

    ApplicationInfo appInfo;
    appInfo.isSystemApp = true;
    OH_EXPECT_RET_AND_OUTPUT({true}, {{appInfo}}, BundleMgrHelper, GetApplicationInfo,
        const std::string&, int32_t, int32_t, ApplicationInfo&);

    OH_EXPECT_RET({true}, TokenIdKit, IsSystemAppByFullTokenID, uint64_t);

    bool result = FUDUtils::CheckIsSystemAppByBundleName(bundleName);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: GetBundleApiTargetVersion_GetApplicationInfoFails_001
 * @tc.desc: Verify GetBundleApiTargetVersion returns false when GetApplicationInfo fails.
 *           Branch: GetApplicationInfo returns false
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetBundleApiTargetVersion_GetApplicationInfoFails_001, TestSize.Level1)
{
    std::string bundleName = "com.example.app";
    int32_t targetApiVersion = 0;

    auto mockHelper = new BundleMgrHelper();

    OH_EXPECT_RET_AND_OUTPUT({false}, {}, BundleMgrHelper, GetApplicationInfo,
        const std::string&, int32_t, int32_t, ApplicationInfo&);

    bool result = FUDUtils::GetBundleApiTargetVersion(bundleName, targetApiVersion);

    EXPECT_EQ(result, false);
    EXPECT_EQ(targetApiVersion, 0);  // Should remain unchanged
}

/**
 * @tc.name: GetBundleApiTargetVersion_Success_003
 * @tc.desc: Verify GetBundleApiTargetVersion succeeds.
 *           Branch: GetApplicationInfo succeeds
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetBundleApiTargetVersion_Success_003, TestSize.Level1)
{
    std::string bundleName = "com.example.app";
    int32_t targetApiVersion = 0;

    auto mockHelper = new BundleMgrHelper();

    ApplicationInfo appInfo;
    appInfo.apiTargetVersion = 9;
    OH_EXPECT_RET_AND_OUTPUT({true}, {{appInfo}}, BundleMgrHelper, GetApplicationInfo,
        const std::string&, int32_t, int32_t, ApplicationInfo&);

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

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    HapTokenInfo hapInfo;
    hapInfo.bundleName = "com.example.app";
    auto expectHapInfo = std::vector<HapTokenInfo>{hapInfo};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo, AccessTokenKit, GetHapTokenInfo, uint32_t,
        HapTokenInfo&);

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

    auto mockHelper = new BundleMgrHelper();

    BundleInfo bundleInfo;
    bundleInfo.applicationInfo.applicationInfo.accessTokenIdEx = 1001;
    OH_EXPECT_RET_AND_OUTPUT({true}, {{bundleInfo}}, BundleMgrHelper, GetBundleInfo,
        const std::string&, int32_t, BundleInfo&, int32_t);

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

    auto mockHelper = new BundleMgrHelper();

    BundleInfo bundleInfo;
    bundleInfo.applicationInfo.applicationInfo.accessTokenIdEx = 1002;
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int32_t>(ERR_OK)}, {{bundleInfo}}, BundleMgrHelper, GetCloneBundleInfo,
        const std::string&, int32_t, int32_t, BundleInfo&, int32_t);

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

    auto mockHelper = new BundleMgrHelper();

    BundleInfo bundleInfo;
    bundleInfo.applicationInfo.applicationInfo.accessTokenIdEx = 1003;
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int32_t>(ERR_OK)}, {{bundleInfo}}, BundleMgrHelper, GetSandboxBundleInfo,
        const std::string&, int32_t, int32_t, BundleInfo&);

    int32_t result = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex, tokenId);

    EXPECT_EQ(result, ERR_OK);
}

/**
 * @tc.name: IsSandboxApp_NativeToken_001
 * @tc.desc: Verify IsSandboxApp returns false for native token.
 *           Branch: tokenType != TOKEN_HAP
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSandboxApp_NativeToken_001, TestSize.Level1)
{
    uint32_t tokenId = 1001;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_NATIVE)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    bool result = FUDUtils::IsSandboxApp(tokenId);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: IsSandboxApp_SandboxApp_002
 * @tc.desc: Verify IsSandboxApp returns true for sandbox app.
 *           Branch: TOKEN_HAP, instIndex > MAX_APP_CLONE_INDEX
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSandboxApp_SandboxApp_002, TestSize.Level1)
{
    uint32_t tokenId = 1001;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    HapTokenInfo hapInfo;
    hapInfo.instIndex = 999;  // > MAX_APP_CLONE_INDEX
    auto expectHapInfo = std::vector<HapTokenInfo>{hapInfo};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo, AccessTokenKit, GetHapTokenInfo, uint32_t,
        HapTokenInfo&);

    bool result = FUDUtils::IsSandboxApp(tokenId);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsSandboxApp_GetHapInfoFails_003
 * @tc.desc: Verify IsSandboxApp returns false when GetHapTokenInfo fails.
 *           Branch: GetHapTokenInfo returns RET_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSandboxApp_GetHapInfoFails_003, TestSize.Level1)
{
    uint32_t tokenId = 1001;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_FAILED)},
        {}, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

    bool result = FUDUtils::IsSandboxApp(tokenId);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: IsSandboxApp_NotSandboxApp_004
 * @tc.desc: Verify IsSandboxApp returns false for non-sandbox app.
 *           Branch: TOKEN_HAP, instIndex <= MAX_APP_CLONE_INDEX
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSandboxApp_NotSandboxApp_004, TestSize.Level1)
{
    uint32_t tokenId = 1001;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    HapTokenInfo hapInfo;
    hapInfo.instIndex = 0;
    auto expectHapInfo = std::vector<HapTokenInfo>{hapInfo};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo, AccessTokenKit, GetHapTokenInfo, uint32_t,
        HapTokenInfo&);

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

// ============================================================================
// CheckAndCreateEventInfo Method Tests
// ============================================================================

/**
 * @tc.name: CheckAndCreateEventInfo_GetCallerBundleNameFails_001
 * @tc.desc: Verify CheckAndCreateEventInfo fails when GetBundleNameByTokenId fails for caller.
 *           Branch: GetBundleNameByTokenId(callerTokenId) fails
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckAndCreateEventInfo_GetCallerBundleNameFails_001, TestSize.Level1)
{
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    EventInfo eventInfo;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_FAILED)},
        {}, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

    bool result = FUDUtils::CheckAndCreateEventInfo(callerTokenId, targetTokenId, eventInfo);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: CheckAndCreateEventInfo_CallerNotSystem_002
 * @tc.desc: Verify CheckAndCreateEventInfo fails when caller is not system app.
 *           Branch: CheckIsSystemAppByBundleName(callerBundleName) returns false
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckAndCreateEventInfo_CallerNotSystem_002, TestSize.Level1)
{
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    EventInfo eventInfo;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    HapTokenInfo hapInfo1;
    hapInfo1.bundleName = "com.example.caller";
    auto expectHapInfo1 = std::vector<HapTokenInfo>{{hapInfo1}};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo1, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

    auto mockHelper = new BundleMgrHelper();

    ApplicationInfo appInfo;
    appInfo.isSystemApp = false;
    OH_EXPECT_RET_AND_OUTPUT({true}, {{appInfo}}, BundleMgrHelper, GetApplicationInfo,
        const std::string&, int32_t, int32_t, ApplicationInfo&);

    bool result = FUDUtils::CheckAndCreateEventInfo(callerTokenId, targetTokenId, eventInfo);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: CheckAndCreateEventInfo_GetTargetBundleNameFails_003
 * @tc.desc: Verify CheckAndCreateEventInfo fails when GetBundleNameByTokenId fails for target.
 *           Branch: GetBundleNameByTokenId(targetTokenId) fails
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckAndCreateEventInfo_GetTargetBundleNameFails_003, TestSize.Level1)
{
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    EventInfo eventInfo;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    HapTokenInfo hapInfo1;
    hapInfo1.bundleName = "com.example.caller";
    auto expectHapInfo1 = std::vector<HapTokenInfo>{{hapInfo1}};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo1, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

    auto mockHelper = new BundleMgrHelper();

    ApplicationInfo appInfo1;
    appInfo1.isSystemApp = true;
    OH_EXPECT_RET_AND_OUTPUT({true}, {{appInfo1}}, BundleMgrHelper, GetApplicationInfo,
        const std::string&, int32_t, int32_t, ApplicationInfo&);

    // Second call to GetHapTokenInfo fails
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_FAILED)},
        {}, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

    bool result = FUDUtils::CheckAndCreateEventInfo(callerTokenId, targetTokenId, eventInfo);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: CheckAndCreateEventInfo_TargetIsSystem_004
 * @tc.desc: Verify CheckAndCreateEventInfo fails when target is system app.
 *           Branch: CheckIsSystemAppByBundleName(targetBundleName) returns true
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckAndCreateEventInfo_TargetIsSystem_004, TestSize.Level1)
{
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    EventInfo eventInfo;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    HapTokenInfo hapInfo1;
    hapInfo1.bundleName = "com.example.caller";
    auto expectHapInfo1 = std::vector<HapTokenInfo>{{hapInfo1}};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo1, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

    HapTokenInfo hapInfo2;
    hapInfo2.bundleName = "com.example.target";
    auto expectHapInfo2 = std::vector<HapTokenInfo>{{hapInfo2}};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo2, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

    auto mockHelper = new BundleMgrHelper();

    ApplicationInfo appInfo1;
    appInfo1.isSystemApp = true;
    OH_EXPECT_RET_AND_OUTPUT({true}, {{appInfo1}}, BundleMgrHelper, GetApplicationInfo,
        const std::string&, int32_t, int32_t, ApplicationInfo&);

    ApplicationInfo appInfo2;
    appInfo2.isSystemApp = true;
    OH_EXPECT_RET_AND_OUTPUT({true}, {{appInfo2}}, BundleMgrHelper, GetApplicationInfo,
        const std::string&, int32_t, int32_t, ApplicationInfo&);

    bool result = FUDUtils::CheckAndCreateEventInfo(callerTokenId, targetTokenId, eventInfo);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: CheckAndCreateEventInfo_Success_005
 * @tc.desc: Verify CheckAndCreateEventInfo succeeds.
 *           Branch: All checks pass
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckAndCreateEventInfo_Success_005, TestSize.Level1)
{
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    EventInfo eventInfo;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    HapTokenInfo hapInfo1;
    hapInfo1.bundleName = "com.example.caller";
    auto expectHapInfo1 = std::vector<HapTokenInfo>{{hapInfo1}};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo1, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

    HapTokenInfo hapInfo2;
    hapInfo2.bundleName = "com.example.target";
    auto expectHapInfo2 = std::vector<HapTokenInfo>{{hapInfo2}};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo2, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

    auto mockHelper = new BundleMgrHelper();

    ApplicationInfo appInfo1;
    appInfo1.isSystemApp = true;
    OH_EXPECT_RET_AND_OUTPUT({true}, {{appInfo1}}, BundleMgrHelper, GetApplicationInfo,
        const std::string&, int32_t, int32_t, ApplicationInfo&);

    ApplicationInfo appInfo2;
    appInfo2.isSystemApp = false;
    OH_EXPECT_RET_AND_OUTPUT({true}, {{appInfo2}}, BundleMgrHelper, GetApplicationInfo,
        const std::string&, int32_t, int32_t, ApplicationInfo&);

    bool result = FUDUtils::CheckAndCreateEventInfo(callerTokenId, targetTokenId, eventInfo);

    EXPECT_EQ(result, true);
    EXPECT_EQ(eventInfo.callerBundleName, "com.example.caller");
    EXPECT_EQ(eventInfo.bundleName, "com.example.target");
}

// ============================================================================
// Additional Test Cases for >80% Branch Coverage
// ============================================================================

/**
 * @tc.name: SendSystemAppGrantUriPermissionEvent_CheckAndCreateFails_004
 * @tc.desc: Verify SendSystemAppGrantUriPermissionEvent fails when CheckAndCreateEventInfo fails.
 *           Branch: CheckAndCreateEventInfo returns false
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, SendSystemAppGrantUriPermissionEvent_CheckAndCreateFails_004, TestSize.Level1)
{
    uint32_t callerTokenId = 1001;
    uint32_t targetTokenId = 1002;
    std::vector<std::string> uriVec = {"file://media/test.jpg"};
    std::vector<CheckResult> resVec = {CheckResult()};

    // Mock GetBundleNameByTokenId to fail
    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);
    OH_EXPECT_RET({static_cast<int>(AccessTokenKitRet::RET_FAILED)},
        AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

    bool result = FUDUtils::SendSystemAppGrantUriPermissionEvent(callerTokenId, targetTokenId, uriVec, resVec);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: CheckIsSystemAppByBundleName_GetApplicationInfoFails_002
 * @tc.desc: Verify CheckIsSystemAppByBundleName returns false when GetApplicationInfo fails.
 *           Branch: GetApplicationInfo returns false
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckIsSystemAppByBundleName_GetApplicationInfoFails_002, TestSize.Level1)
{
    std::string bundleName = "com.example.app";

    auto mockHelper = new BundleMgrHelper();

    OH_EXPECT_RET_AND_OUTPUT({false}, {}, BundleMgrHelper, GetApplicationInfo,
        const std::string&, int32_t, int32_t, ApplicationInfo&);

    bool result = FUDUtils::CheckIsSystemAppByBundleName(bundleName);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: CheckIsSystemAppByBundleName_NotSystem_003
 * @tc.desc: Verify CheckIsSystemAppByBundleName returns false when not system app.
 *           Branch: IsSystemAppByFullTokenID returns false
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckIsSystemAppByBundleName_NotSystem_003, TestSize.Level1)
{
    std::string bundleName = "com.example.normal";

    auto mockHelper = new BundleMgrHelper();

    ApplicationInfo appInfo;
    appInfo.isSystemApp = false;
    OH_EXPECT_RET_AND_OUTPUT({true}, {{appInfo}}, BundleMgrHelper, GetApplicationInfo,
        const std::string&, int32_t, int32_t, ApplicationInfo&);

    OH_EXPECT_RET({false}, TokenIdKit, IsSystemAppByFullTokenID, uint64_t);

    bool result = FUDUtils::CheckIsSystemAppByBundleName(bundleName);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetBundleNameByTokenId_NativeToken_002
 * @tc.desc: Verify GetBundleNameByTokenId returns false for native token.
 *           Branch: tokenType != TOKEN_HAP
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetBundleNameByTokenId_NativeToken_002, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    std::string bundleName;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_NATIVE)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    bool result = FUDUtils::GetBundleNameByTokenId(tokenId, bundleName);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetBundleNameByTokenId_GetHapInfoFails_003
 * @tc.desc: Verify GetBundleNameByTokenId returns false when GetHapTokenInfo fails.
 *           Branch: GetHapTokenInfo returns RET_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetBundleNameByTokenId_GetHapInfoFails_003, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    std::string bundleName;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_FAILED)},
        {}, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

    bool result = FUDUtils::GetBundleNameByTokenId(tokenId, bundleName);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetTokenIdByBundleName_GetBundleInfoFails_004
 * @tc.desc: Verify GetTokenIdByBundleName fails when GetBundleInfo fails.
 *           Branch: appIndex == 0, GetBundleInfo fails
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetTokenIdByBundleName_GetBundleInfoFails_004, TestSize.Level1)
{
    std::string bundleName = "com.example.app";
    int32_t appIndex = 0;
    uint32_t tokenId = 0;

    auto mockHelper = new BundleMgrHelper();

    OH_GLOBAL_EXPECT_RET({static_cast<int32_t>(100)}, GetCurrentAccountId);

    OH_EXPECT_RET_AND_OUTPUT({false}, {}, BundleMgrHelper, GetBundleInfo,
        const std::string&, int32_t, BundleInfo&, int32_t);

    int32_t result = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex, tokenId);

    EXPECT_EQ(result, ERR_GET_TARGET_BUNDLE_INFO_FAILED);
}

/**
 * @tc.name: IsSystemAppCall_Success_001
 * @tc.desc: Verify IsSystemAppCall returns true.
 *           Branch: IsSystemAppCall returns true
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSystemAppCall_Success_001, TestSize.Level1)
{
    OH_EXPECT_RET({true}, PermissionVerification, IsSystemAppCall);

    bool result = FUDUtils::IsSystemAppCall();

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: IsSystemAppCall_False_002
 * @tc.desc: Verify IsSystemAppCall returns false.
 *           Branch: IsSystemAppCall returns false
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, IsSystemAppCall_False_002, TestSize.Level1)
{
    OH_EXPECT_RET({false}, PermissionVerification, IsSystemAppCall);

    bool result = FUDUtils::IsSystemAppCall();

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: CheckIsSystemAppByTokenId_Success_001
 * @tc.desc: Verify CheckIsSystemAppByTokenId succeeds.
 *           Branch: GetBundleNameByTokenId succeeds, CheckIsSystemAppByBundleName succeeds
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckIsSystemAppByTokenId_Success_001, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    std::string bundleName;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    HapTokenInfo hapInfo;
    hapInfo.bundleName = "com.example.system";
    auto expectHapInfo = std::vector<HapTokenInfo>{hapInfo};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo, AccessTokenKit, GetHapTokenInfo, uint32_t,
        HapTokenInfo&);

    auto mockHelper = new BundleMgrHelper();

    ApplicationInfo appInfo;
    appInfo.isSystemApp = true;
    OH_EXPECT_RET_AND_OUTPUT({true}, {{appInfo}}, BundleMgrHelper, GetApplicationInfo,
        const std::string&, int32_t, int32_t, ApplicationInfo&);

    bool result = FUDUtils::CheckIsSystemAppByTokenId(tokenId);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: CheckIsSystemAppByTokenId_GetBundleNameFails_002
 * @tc.desc: Verify CheckIsSystemAppByTokenId fails when GetBundleNameByTokenId fails.
 *           Branch: GetBundleNameByTokenId returns false
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, CheckIsSystemAppByTokenId_GetBundleNameFails_002, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    std::string bundleName;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_FAILED)},
        {}, AccessTokenKit, GetHapTokenInfo, uint32_t,
        HapTokenInfo&);

    bool result = FUDUtils::CheckIsSystemAppByTokenId(tokenId);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetDirByBundleNameAndAppIndex_GetDirFails_001
 * @tc.desc: Verify GetDirByBundleNameAndAppIndex fails when GetDirByBundleNameAndAppIndex fails.
 *           Branch: GetDirByBundleNameAndAppIndex returns error
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetDirByBundleNameAndAppIndex_GetDirFails_001, TestSize.Level1)
{
    std::string bundleName = "com.example.app";
    int32_t appIndex = 0;
    std::string dirName;

    auto mockClient = new BundleMgrClient();

    OH_EXPECT_RET({static_cast<int32_t>(INNER_ERR)}, BundleMgrClient, GetDirByBundleNameAndAppIndex,
        const std::string&, int32_t, std::string& dirName);

    bool result = FUDUtils::GetDirByBundleNameAndAppIndex(bundleName, appIndex, dirName);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetDirByBundleNameAndAppIndex_Success_003
 * @tc.desc: Verify GetDirByBundleNameAndAppIndex succeeds.
 *           Branch: GetDirByBundleNameAndAppIndex succeeds
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetDirByBundleNameAndAppIndex_Success_003, TestSize.Level1)
{
    std::string bundleName = "com.example.app";
    int32_t appIndex = 0;
    std::string dirName;
    std::string expectedDir = "/data/app/el2/base";

    auto mockClient = new BundleMgrClient();

    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, BundleMgrClient, GetDirByBundleNameAndAppIndex,
        const std::string&, int32_t, std::string& dirName);

    bool result = FUDUtils::GetDirByBundleNameAndAppIndex(bundleName, appIndex, dirName);

    EXPECT_EQ(result, true);
    EXPECT_EQ(dirName, expectedDir);
}

/**
 * @tc.name: GetAlterableBundleNameByTokenId_NativeToken_001
 * @tc.desc: Verify GetAlterableBundleNameByTokenId returns false for native token.
 *           Branch: tokenType != TOKEN_HAP
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetAlterableBundleNameByTokenId_NativeToken_001, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    std::string bundleName;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_NATIVE)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    bool result = FUDUtils::GetAlterableBundleNameByTokenId(tokenId, bundleName);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetAlterableBundleNameByTokenId_GetHapInfoFails_002
 * @tc.desc: Verify GetAlterableBundleNameByTokenId fails when GetHapTokenInfo fails.
 *           Branch: GetHapTokenInfo returns RET_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetAlterableBundleNameByTokenId_GetHapInfoFails_002, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    std::string bundleName;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_FAILED)},
        {}, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

    bool result = FUDUtils::GetAlterableBundleNameByTokenId(tokenId, bundleName);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetAlterableBundleNameByTokenId_GetDirFails_003
 * @tc.desc: Verify GetAlterableBundleNameByTokenId fails when GetDirByBundleNameAndAppIndex fails.
 *           Branch: GetDirByBundleNameAndAppIndex returns false
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetAlterableBundleNameByTokenId_GetDirFails_003, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    std::string bundleName;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    HapTokenInfo hapInfo;
    hapInfo.bundleName = "com.example.app";
    hapInfo.instIndex = 0;
    auto expectHapInfo = std::vector<HapTokenInfo>{hapInfo};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo, AccessTokenKit, GetHapTokenInfo, uint32_t,
        HapTokenInfo&);

    auto mockClient = new BundleMgrClient();

    OH_EXPECT_RET({static_cast<int32_t>(INNER_ERR)}, BundleMgrClient, GetDirByBundleNameAndAppIndex,
        const std::string&, int32_t, std::string& dirName);

    bool result = FUDUtils::GetAlterableBundleNameByTokenId(tokenId, bundleName);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GetAlterableBundleNameByTokenId_Success_004
 * @tc.desc: Verify GetAlterableBundleNameByTokenId succeeds.
 *           Branch: All operations succeed
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetAlterableBundleNameByTokenId_Success_004, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    std::string bundleName;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    HapTokenInfo hapInfo;
    hapInfo.bundleName = "com.example.app";
    hapInfo.instIndex = 0;
    auto expectHapInfo = std::vector<HapTokenInfo>{hapInfo};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo, AccessTokenKit, GetHapTokenInfo, uint32_t,
        HapTokenInfo&);

    auto mockClient = new BundleMgrClient();

    std::string expectedDir = "/data/app/el2/base";
    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, BundleMgrClient, GetDirByBundleNameAndAppIndex,
        const std::string&, int32_t, std::string& dirName);

    bool result = FUDUtils::GetAlterableBundleNameByTokenId(tokenId, bundleName);

    EXPECT_EQ(result, true);
}

/**
 * @tc.name: GetAppIdByBundleName_GetAppIdFails_001
 * @tc.desc: Verify GetAppIdByBundleName fails when GetAppIdByBundleName returns empty.
 *           Branch: appId.empty()
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetAppIdByBundleName_GetAppIdFails_001, TestSize.Level1)
{
    std::string bundleName = "com.example.app";
    std::string appId;

    auto mockHelper = new BundleMgrHelper();

    OH_GLOBAL_EXPECT_RET({static_cast<int32_t>(100)}, GetCurrentAccountId);

    OH_EXPECT_RET({std::string("")}, BundleMgrHelper, GetAppIdByBundleName,
        const std::string&, int32_t);

    bool result = FUDUtils::GetAppIdByBundleName(bundleName, appId);

    EXPECT_EQ(result, INNER_ERR);
}

/**
 * @tc.name: GetAppIdByBundleName_Success_003
 * @tc.desc: Verify GetAppIdByBundleName succeeds.
 *           Branch: All operations succeed
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GetAppIdByBundleName_Success_003, TestSize.Level1)
{
    std::string bundleName = "com.example.app";
    std::string appId;
    std::string expectedAppId = "com.example.app_123";

    auto mockHelper = new BundleMgrHelper();

    OH_GLOBAL_EXPECT_RET({static_cast<int32_t>(100)}, GetCurrentAccountId);

    std::string expectedAppId = "com.example.app_123";
    OH_EXPECT_RET({expectedAppId}, BundleMgrHelper, GetAppIdByBundleName,
        const std::string&, int32_t);

    bool result = FUDUtils::GetAppIdByBundleName(bundleName, appId);

    EXPECT_EQ(result, ERR_OK);
    EXPECT_EQ(appId, expectedAppId);
}

/**
 * @tc.name: GenerateFUDAppInfo_NativeToken_001
 * @tc.desc: Verify GenerateFUDAppInfo returns false for native token.
 *           Branch: tokenType != TOKEN_HAP
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GenerateFUDAppInfo_NativeToken_001, TestSize.Level1)
{
    FUDAppInfo info;
    info.tokenId = 1001;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_NATIVE)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    bool result = FUDUtils::GenerateFUDAppInfo(info);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GenerateFUDAppInfo_GetHapInfoFails_002
 * @tc.desc: Verify GenerateFUDAppInfo fails when GetHapTokenInfo fails.
 *           Branch: GetHapTokenInfo returns RET_FAILED
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GenerateFUDAppInfo_GetHapInfoFails_002, TestSize.Level1)
{
    FUDAppInfo info;
    info.tokenId = 1001;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_FAILED)},
        {}, AccessTokenKit, GetHapTokenInfo, uint32_t, HapTokenInfo&);

    bool result = FUDUtils::GenerateFUDAppInfo(info);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GenerateFUDAppInfo_GetDirFails_003
 * @tc.desc: Verify GenerateFUDAppInfo fails when GetDirByBundleNameAndAppIndex fails.
 *           Branch: GetDirByBundleNameAndAppIndex returns false
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GenerateFUDAppInfo_GetDirFails_003, TestSize.Level1)
{
    FUDAppInfo info;
    info.tokenId = 1001;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    HapTokenInfo hapInfo;
    hapInfo.bundleName = "com.example.app";
    hapInfo.instIndex = 0;
    auto expectHapInfo = std::vector<HapTokenInfo>{hapInfo};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo, AccessTokenKit, GetHapTokenInfo, uint32_t,
        HapTokenInfo&);

    auto mockClient = new BundleMgrClient();

    OH_EXPECT_RET({static_cast<int32_t>(INNER_ERR)}, BundleMgrClient, GetDirByBundleNameAndAppIndex,
        const std::string&, int32_t, std::string& dirName);

    bool result = FUDUtils::GenerateFUDAppInfo(info);

    EXPECT_EQ(result, false);
}

/**
 * @tc.name: GenerateFUDAppInfo_Success_004
 * @tc.desc: Verify GenerateFUDAppInfo succeeds.
 *           Branch: All operations succeed
 * @tc.type: FUNC
 */
HWTEST_F(FileUriDistributionUtilsTest, GenerateFUDAppInfo_Success_004, TestSize.Level1)
{
    FUDAppInfo info;
    info.tokenId = 1001;

    OH_EXPECT_RET({static_cast<int>(ATokenTypeEnum::TOKEN_HAP)},
        AccessTokenKit, GetTokenTypeFlag, uint32_t);

    HapTokenInfo hapInfo;
    hapInfo.bundleName = "com.example.app";
    hapInfo.instIndex = 0;
    hapInfo.userID = 100;
    auto expectHapInfo = std::vector<HapTokenInfo>{hapInfo};
    OH_EXPECT_RET_AND_OUTPUT({static_cast<int>(AccessTokenKitRet::RET_SUCCESS)},
        expectHapInfo, AccessTokenKit, GetHapTokenInfo, uint32_t,
        HapTokenInfo&);

    auto mockClient = new BundleMgrClient();

    std::string expectedDir = "/data/app/el2/base";
    OH_EXPECT_RET({static_cast<int32_t>(ERR_OK)}, BundleMgrClient, GetDirByBundleNameAndAppIndex,
        const std::string&, int32_t, std::string& dirName);

    bool result = FUDUtils::GenerateFUDAppInfo(info);

    EXPECT_EQ(result, true);
    EXPECT_EQ(info.userId, 100);
    EXPECT_EQ(info.bundleName, "com.example.app");
    EXPECT_EQ(info.alterBundleName, expectedDir);
}

}  // namespace AAFwk
}  // namespace OHOS
