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

#include "ability_manager_errors.h"
#include "event_report.h"
#define private public
#include "file_uri_distribution_utils.h"
#undef private
#include "system_ability_definition.h"
#include "tokenid_kit.h"
#include "mock_my_flag.h"
#include "accesstoken_kit.h"
#include "uri.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class FileUriDistributionUtilsNewTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void FileUriDistributionUtilsNewTest::SetUpTestCase() {}

void FileUriDistributionUtilsNewTest::TearDownTestCase() {}

void FileUriDistributionUtilsNewTest::SetUp() {}

void FileUriDistributionUtilsNewTest::TearDown() {}

/*
 * Feature: FUDUtils
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: FUDUtils SendShareUnPrivilegeUriEvent
 */
HWTEST_F(FileUriDistributionUtilsNewTest, SendShareUnPrivilegeUriEvent_001, TestSize.Level1)
{
    uint32_t callerTokenId = 1;
    uint32_t targetTokenId = 2;
    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[callerTokenId] = TokenInfo(callerTokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP,
        "com.test.test", "com.test.test");
    MyFlag::tokenInfos_[targetTokenId] = TokenInfo(targetTokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP,
        "com.test.test", "com.test.test");
    auto result = FUDUtils::SendShareUnPrivilegeUriEvent(callerTokenId, targetTokenId);
    EXPECT_TRUE(result);
}

/*
 * Feature: FUDUtils
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: FUDUtils CheckAndCreateEventInfo
 */
HWTEST_F(FileUriDistributionUtilsNewTest, CheckAndCreateEventInfo_001, TestSize.Level1)
{
    uint32_t callerTokenId = 1;
    uint32_t targetTokenId = 2;
    EventInfo eventInfo;
    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[callerTokenId] = TokenInfo(callerTokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP,
        "com.test.ohos", "com.test.ohos");
    MyFlag::tokenInfos_[targetTokenId] = TokenInfo(targetTokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP,
        "com.test.ohos", "com.test.ohos");
    auto result = FUDUtils::CheckAndCreateEventInfo(callerTokenId, targetTokenId, eventInfo);
    EXPECT_FALSE(result);
}

/*
 * Feature: FUDUtils
 * Function: GenerateFUDAppInfo
 * SubFunction: NA
 * FunctionPoints: FUDUtils GenerateFUDAppInfo
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GenerateFUDAppInfo_001, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    FUDAppInfo info = { .tokenId = tokenId };
    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[tokenId] = TokenInfo(tokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_NATIVE);
    auto result = FUDUtils::GenerateFUDAppInfo(info);
    EXPECT_FALSE(result);
}

/*
 * Feature: FUDUtils
 * Function: GenerateFUDAppInfo
 * SubFunction: NA
 * FunctionPoints: FUDUtils GenerateFUDAppInfo
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GenerateFUDAppInfo_002, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    FUDAppInfo info = { .tokenId = tokenId };
    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[tokenId] = TokenInfo(tokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP);
    MyFlag::retHapSuccValue_ = Security::AccessToken::AccessTokenKitRet::RET_FAILED;
    auto result = FUDUtils::GenerateFUDAppInfo(info);
    EXPECT_FALSE(result);
}

/*
 * Feature: FUDUtils
 * Function: GenerateFUDAppInfo
 * SubFunction: NA
 * FunctionPoints: FUDUtils GenerateFUDAppInfo
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GenerateFUDAppInfo_003, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    FUDAppInfo info = { .tokenId = tokenId };
    std::string bundleName = "testApp";
    std::string processName = "testApp";
    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[tokenId] = TokenInfo(tokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP,
        processName, bundleName);
    MyFlag::retHapSuccValue_ = Security::AccessToken::AccessTokenKitRet::RET_SUCCESS;
    auto result = FUDUtils::GenerateFUDAppInfo(info);
    EXPECT_TRUE(result);
    EXPECT_EQ(info.bundleName, bundleName);
}

/*
 * Feature: FUDUtils
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: FUDUtils GetAlterableBundleNameByTokenId
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GetAlterableBundleNameByTokenId_001, TestSize.Level1)
{
    uint32_t tokenId = 1;
    std::string bundleName = "";
    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[tokenId] = TokenInfo(tokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP);
    MyFlag::retHapSuccValue_ = Security::AccessToken::AccessTokenKitRet::RET_FAILED;
    auto result = FUDUtils::GetAlterableBundleNameByTokenId(tokenId, bundleName);
    EXPECT_FALSE(result);
}

/*
 * Feature: FUDUtils
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: FUDUtils GetAppIdByBundleName
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GetAppIdByBundleName_001, TestSize.Level1)
{
    std::string bundleName = "";
    std::string appId = "";
    auto result = FUDUtils::GetAppIdByBundleName(bundleName, appId);
    EXPECT_EQ(result, INNER_ERR);
}

/*
 * Feature: FUDUtils
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: FUDUtils GetTokenIdByBundleName
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GetTokenIdByBundleName_001, TestSize.Level1)
{
    std::string bundleName = "";
    int32_t appIndex = 1001;
    uint32_t tokenId = 1;
    auto result = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex, tokenId);
    EXPECT_EQ(result, ERR_GET_TARGET_BUNDLE_INFO_FAILED);
}

/*
 * Feature: FUDUtils
 * Function: IsDocsCloudUri
 * SubFunction: NA
 * FunctionPoints: FUDUtils IsDocsCloudUri
 */
HWTEST_F(FileUriDistributionUtilsNewTest, IsDocsCloudUri_001, TestSize.Level1)
{
    Uri uri("ccc");
    EXPECT_FALSE(FUDUtils::IsDocsCloudUri(uri));
}

/*
 * Feature: FUDUtils
 * Function: IsDocsCloudUri
 * SubFunction: NA
 * FunctionPoints: FUDUtils IsDocsCloudUri
 */
HWTEST_F(FileUriDistributionUtilsNewTest, IsDocsCloudUri_002, TestSize.Level1)
{
    Uri uri("file://docs/temp.txt?networkid=1001");
    EXPECT_TRUE(FUDUtils::IsDocsCloudUri(uri));
}

/*
 * Feature: FUDUtils
 * Function: CheckUriTypeIsValid
 * SubFunction: NA
 * FunctionPoints: FUDUtils CheckUriTypeIsValid
 */
HWTEST_F(FileUriDistributionUtilsNewTest, CheckUriTypeIsValid_001, TestSize.Level1)
{
    Uri uri("ccc");
    EXPECT_FALSE(FUDUtils::CheckUriTypeIsValid(uri));
}

/*
 * Feature: FUDUtils
 * Function: CheckUriTypeIsValid
 * SubFunction: NA
 * FunctionPoints: FUDUtils CheckUriTypeIsValid
 */
HWTEST_F(FileUriDistributionUtilsNewTest, CheckUriTypeIsValid_002, TestSize.Level1)
{
    Uri uri("file:///path/to/file");
    EXPECT_TRUE(FUDUtils::CheckUriTypeIsValid(uri));
}

/*
 * Feature: FUDUtils
 * Function: GetBundleApiTargetVersion
 * SubFunction: NA
 * FunctionPoints: FUDUtils GetBundleApiTargetVersion
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GetBundleApiTargetVersion_001, TestSize.Level1)
{
    std::string bundleName = "ohos.bundle.test";
    int32_t targetApiVersion = 0;
    EXPECT_FALSE(FUDUtils::GetBundleApiTargetVersion(bundleName, targetApiVersion));
}

/*
 * Feature: FUDUtils
 * Function: GetDirByBundleNameAndAppIndex
 * SubFunction: NA
 * FunctionPoints: FUDUtils GetDirByBundleNameAndAppIndex
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GetDirByBundleNameAndAppIndex_001, TestSize.Level1)
{
    std::string bundleName = "ohos.bundle.test";
    std::string dirName = "/Data";
    EXPECT_TRUE(FUDUtils::GetDirByBundleNameAndAppIndex(bundleName, 0, dirName));
}

/*
 * Feature: FUDUtils
 * Function: SendSystemAppGrantUriPermissionEvent
 * SubFunction: NA
 */
HWTEST_F(FileUriDistributionUtilsNewTest, SendSystemAppGrantUriPermissionEvent_001, TestSize.Level1)
{
    uint32_t callerTokenId = 1;
    uint32_t targetTokenId = 2;
    std::vector<std::string> uriVec = { "file://com.example.system/data/test.txt" };
    const std::vector<CheckResult> resVec = { CheckResult(true, 0) };

    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[callerTokenId] = TokenInfo(callerTokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP,
        "com.example.system", "com.example.system");
    MyFlag::tokenInfos_[targetTokenId] = TokenInfo(targetTokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP,
        "com.example.normal", "com.example.normal");
    
    auto ret = FUDUtils::SendSystemAppGrantUriPermissionEvent(callerTokenId, targetTokenId, uriVec, resVec);
    EXPECT_EQ(ret, true);
}

/*
 * Feature: FUDUtils
 * Function: GetCurrentAccountId
 * SubFunction: NA
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GetCurrentAccountId_001, TestSize.Level1)
{
    auto ret = FUDUtils::GetCurrentAccountId();
    EXPECT_EQ(ret, 200);
}

/*
 * Feature: FUDUtils
 * Function: IsFoundationCall
 * SubFunction: NA
 */
HWTEST_F(FileUriDistributionUtilsNewTest, IsFoundationCall_001, TestSize.Level1)
{
    auto ret = FUDUtils::IsFoundationCall();
    EXPECT_EQ(ret, true);
}

/*
 * Feature: FUDUtils
 * Function: IsPrivilegedSACall
 * SubFunction: NA
 */
HWTEST_F(FileUriDistributionUtilsNewTest, IsPrivilegedSACall_001, TestSize.Level1)
{
    auto ret = FUDUtils::IsPrivilegedSACall();
    EXPECT_EQ(ret, true);
}

/*
 * Feature: FUDUtils
 * Function: CheckIsSystemAppByBundleName
 * SubFunction: NA
 */
HWTEST_F(FileUriDistributionUtilsNewTest, CheckIsSystemAppByBundleName_001, TestSize.Level1)
{
    std::string bundleName = "com.example.system";
    auto ret = FUDUtils::CheckIsSystemAppByBundleName(bundleName);
    EXPECT_EQ(ret, true);
}

/*
 * Feature: FUDUtils
 * Function: GetBundleNameByTokenId
 * SubFunction: NA
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GetBundleNameByTokenId_001, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    std::string bundleName = "";
    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[tokenId] = TokenInfo(tokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP,
        "com.example.test", "com.example.test");
    MyFlag::retHapSuccValue_ = Security::AccessToken::AccessTokenKitRet::RET_SUCCESS;
    
    auto ret = FUDUtils::GetBundleNameByTokenId(tokenId, bundleName);
    EXPECT_EQ(ret, true);
    EXPECT_EQ(bundleName, "com.example.test");
}

/*
 * Feature: FUDUtils
 * Function: GetTokenIdByBundleName
 * SubFunction: NA
 */
HWTEST_F(FileUriDistributionUtilsNewTest, GetTokenIdByBundleName_002, TestSize.Level1)
{
    std::string bundleName = "com.example.test";
    int32_t appIndex = 0;
    uint32_t tokenId = 0;

    auto ret = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex, tokenId);
    EXPECT_EQ(ret, ERR_GET_TARGET_BUNDLE_INFO_FAILED);

    int32_t appIndex1 = 1;
    auto ret1 = FUDUtils::GetTokenIdByBundleName(bundleName, appIndex1, tokenId);
    EXPECT_EQ(ret1, ERR_GET_TARGET_BUNDLE_INFO_FAILED);
}

/*
 * Feature: FUDUtils
 * Function: IsSandboxApp
 * SubFunction: NA
 */
HWTEST_F(FileUriDistributionUtilsNewTest, IsSandboxApp_001, TestSize.Level1)
{
    uint32_t tokenId = 1001;
    MyFlag::tokenInfos_.clear();
    MyFlag::tokenInfos_[tokenId] = TokenInfo(tokenId, Security::AccessToken::ATokenTypeEnum::TOKEN_HAP);
    MyFlag::retHapSuccValue_ = Security::AccessToken::AccessTokenKitRet::RET_SUCCESS;
    
    auto ret = FUDUtils::IsSandboxApp(tokenId);
    EXPECT_EQ(ret, true);
}

/*
 * Feature: FUDUtils
 * Function: CheckUriTypeIsValid
 * SubFunction: NA
 */
HWTEST_F(FileUriDistributionUtilsNewTest, CheckUriTypeIsValid_003, TestSize.Level1)
{
    Uri uri("content://com.example/test.txt");
    
    auto ret = FUDUtils::CheckUriTypeIsValid(uri);
    EXPECT_EQ(ret, true);
}

/*
 * Feature: FUDUtils
 * Function: IsDocsCloudUri
 * SubFunction: NA
 */
HWTEST_F(FileUriDistributionUtilsNewTest, IsDocsCloudUri_003, TestSize.Level1)
{
    Uri uri("file://docs/temp.txt");
    
    auto ret = FUDUtils::IsDocsCloudUri(uri);
    EXPECT_EQ(ret, false);
}

/*
 * Feature: FUDUtils
 * Function: IsUdmfOrPasteboardCall
 * SubFunction: NA
 * FunctionPoints: CallingUid是UDMF_UID返回true
 */
HWTEST_F(FileUriDistributionUtilsNewTest, IsUdmfOrPasteboardCall_001, TestSize.Level1)
{
    auto ret = FUDUtils::IsUdmfOrPasteboardCall();
    EXPECT_EQ(ret, true);
}

/*
 * Feature: FUDUtils
 * Function: IsBrokerCaller
 * SubFunction: NA
 */
HWTEST_F(FileUriDistributionUtilsNewTest, IsBrokerCaller_001, TestSize.Level1)
{
    auto ret = FUDUtils::IsBrokerCaller();
    EXPECT_EQ(ret, true);
}

/*
 * Feature: FUDUtils
 * Function: IsDFSCall
 * SubFunction: NA
 */
HWTEST_F(FileUriDistributionUtilsNewTest, IsDFSCall_001, TestSize.Level1)
{
    auto ret = FUDUtils::IsDFSCall();
    EXPECT_EQ(ret, true);
}
}  // namespace AAFwk
}  // namespace OHOS