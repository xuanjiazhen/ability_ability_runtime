/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "ability_manager_client.h"
#include "mock_media_permission_manager.h"
#include "mock_accesstoken_kit.h"
#include "mock_app_mgr_service.h"
#include "mock_app_utils.h"
#include "mock_bundle_mgr_helper.h"
#include "mock_ipc_skeleton.h"
#include "mock_my_flag.h"
#include "mock_native_token.h"
#include "mock_permission_verification.h"
#include "mock_system_ability_manager_client.h"
#include "ability_manager_errors.h"
#include "file_permission_manager.h"
#include "hilog_tag_wrapper.h"
#include "event_report.h"
#include "sandbox_manager_kit.h"
#include "system_ability_definition.h"
#include "tokenid_kit.h"
#define private public
#include "batch_uri.h"
#include "file_uri_distribution_utils.h"
#include "uri_permission_manager_client.h"
#include "uri_permission_manager_stub_impl.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
namespace {
constexpr int MAX_URI_COUNT = 200000;
const std::string POLICY_INFO_PATH = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
}
using namespace AccessControl::SandboxManager;
class UriPermissionImplTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UriPermissionImplTest::SetUpTestCase()
{
    AppExecFwk::MockNativeToken::SetNativeToken();
}

void UriPermissionImplTest::TearDownTestCase() {}

void UriPermissionImplTest::SetUp()
{
    MyFlag::Init();
    AppUtils::Init();
    SandboxManagerKit::Init();
    IPCSkeleton::callerTokenId = 0;
    IPCSkeleton::callerUId = 0;
    StorageManager::StorageManagerServiceMock::isZero = true;
    MockSystemAbilityManager::isNullptr = false;
    AbilityManagerClient::collaborator_ = nullptr;
    AbilityManagerClient::isNullInstance = false;
}

void UriPermissionImplTest::TearDown() {}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    Uri uri("file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt");
    uint32_t flag = 0;
    std::string targetBundleName = "name2";
    int32_t funcResult = -1;
    upms->GrantUriPermission(uri, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, ERR_NOT_SYSTEM_APP);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_002, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    Uri uri("file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt");
    MyFlag::isSandboxAppRet_ = false;
    uint32_t flag = 1;
    std::string targetBundleName = "name2";
    int32_t funcResult = -1;
    upms->GrantUriPermission(uri, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_003, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    Uri uri("file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt");
    MyFlag::isSandboxAppRet_ = true;
    uint32_t flag = 1;
    std::string targetBundleName = "name2";
    int32_t funcResult = -1;
    upms->GrantUriPermission(uri, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, ERR_CODE_GRANT_URI_PERMISSION);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_004, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    Uri uri("file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt");
    uint32_t flag = 0;
    std::string targetBundleName = "name2";
    int32_t funcResult = -1;
    upms->GrantUriPermission(uri, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, ERR_CODE_INVALID_URI_FLAG);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_005, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    Uri uri("file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt");
    uint32_t flag = 1;
    std::string targetBundleName = "name2";
    int32_t funcResult = -1;
    MyFlag::getTokenIdByBundleNameStatus_ = ERR_GET_TARGET_BUNDLE_INFO_FAILED;
    upms->GrantUriPermission(uri, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, ERR_GET_TARGET_BUNDLE_INFO_FAILED);
}

/*
 * Feature: GrantUriPermission
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_006, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    Uri uri("invalid://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt");
    uint32_t flag = 1;
    std::string targetBundleName = "com.example.app1001";
    int32_t funcResult = -1;
    upms->GrantUriPermission(uri, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, ERR_CODE_INVALID_URI_TYPE);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_007, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    Uri uri("file://com.example.app1002/data/storage/el2/base/haps/entry/files/test_A.txt");
    uint32_t flag = 1;
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");
    std::string targetBundleName = "com.example.app1003";
    int32_t funcResult = -1;
    upms->GrantUriPermission(uri, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_009, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    std::vector<std::string> uriStrVec = { uriStr };

    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uriStrVec, rawData);
    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    uint32_t flag = 0;
    std::string targetBundleName = "name2";
    int32_t funcResult = -1;
    upms->GrantUriPermission(stubRawData, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, ERR_CODE_INVALID_URI_FLAG);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_010, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    std::vector<std::string> uriStrVec = { uriStr };

    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uriStrVec, rawData);
    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    uint32_t flag = 1;
    std::string targetBundleName = "name2";
    int32_t funcResult = -1;
    upms->GrantUriPermission(stubRawData, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_011, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    std::vector<std::string> uriStrVec = { uriStr };

    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uriStrVec, rawData);
    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    uint32_t flag = 2;
    MockSystemAbilityManager::isNullptr = false;
    std::string targetBundleName = "name2";
    int32_t funcResult = -1;
    upms->GrantUriPermission(stubRawData, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, CHECK_PERMISSION_FAILED);
    MockSystemAbilityManager::isNullptr = true;
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_012, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    std::vector<std::string> uriStrVec = { uriStr };

    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uriStrVec, rawData);
    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    uint32_t flag = 2;
    std::string targetBundleName = "name2";
    MockSystemAbilityManager::isNullptr = false;
    StorageManager::StorageManagerServiceMock::isZero = false;
    int32_t funcResult = -1;
    upms->GrantUriPermission(stubRawData, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, CHECK_PERMISSION_FAILED);
    MockSystemAbilityManager::isNullptr = true;
    StorageManager::StorageManagerServiceMock::isZero = true;
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_013, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    uint32_t tmpFlag = 1;
    uint32_t fromTokenId = 2;
    uint32_t targetTokenId = 3;
    std::string targetBundleName = "name2";
    GrantInfo info = { tmpFlag, fromTokenId, targetTokenId };
    std::list<GrantInfo> infoList = { info };
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    upms->uriMap_.emplace(uriStr, infoList);
    std::vector<std::string> uriStrVec = { uriStr };

    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uriStrVec, rawData);
    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    MockSystemAbilityManager::isNullptr = false;
    int32_t funcResult = -1;
    upms->GrantUriPermission(stubRawData, tmpFlag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, CHECK_PERMISSION_FAILED);
    MockSystemAbilityManager::isNullptr = true;
}

/*
 * Feature: GrantUriPermission
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_014, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    uint32_t tmpFlag = 1;
    uint32_t fromTokenId = 2;
    uint32_t targetTokenId = 3;
    std::string targetBundleName = "name2";
    GrantInfo info = { tmpFlag, fromTokenId, targetTokenId };
    std::list<GrantInfo> infoList = { info };
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    upms->uriMap_.emplace(uriStr, infoList);
    std::vector<std::string> uriStrVec = { uriStr };

    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uriStrVec, rawData);
    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    MockSystemAbilityManager::isNullptr = false;
    uint32_t flag = 2;
    int32_t funcResult = -1;
    upms->GrantUriPermission(stubRawData, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, CHECK_PERMISSION_FAILED);
    MockSystemAbilityManager::isNullptr = true;
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_015, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    uint32_t tmpFlag = 1;
    uint32_t fromTokenId = 2;
    uint32_t targetTokenId = 3;
    std::string targetBundleName = "name2";
    GrantInfo info = { tmpFlag, fromTokenId, targetTokenId };
    std::list<GrantInfo> infoList = { info };
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    upms->uriMap_.emplace(uriStr, infoList);
    std::vector<std::string> uriStrVec = { uriStr };

    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uriStrVec, rawData);
    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    MockSystemAbilityManager::isNullptr = false;
    uint32_t flag = 2;
    int32_t funcResult = -1;
    upms->GrantUriPermission(stubRawData, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, CHECK_PERMISSION_FAILED);
    MockSystemAbilityManager::isNullptr = true;
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_016, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    MyFlag::flag_ &= (~MyFlag::IS_SA_CALL);
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    std::vector<std::string> uriStrVec = { uriStr };

    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uriStrVec, rawData);
    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    uint32_t flag = 1;
    std::string targetBundleName = "name1001";
    int32_t funcResult = -1;
    upms->GrantUriPermission(stubRawData, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, ERR_NOT_SYSTEM_APP);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_017, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    MyFlag::flag_ &= (~MyFlag::IS_SA_CALL);
    auto uriStr = "file://com.example.test/data/storage/el2/base/haps/entry/files/test_A.txt";
    std::vector<std::string> uriStrVec(200000 + 1, uriStr);

    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uriStrVec, rawData);
    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    uint32_t flag = 1;
    std::string targetBundleName = "name1001";
    int32_t funcResult = -1;
    upms->GrantUriPermission(stubRawData, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: URIPermissionManagerService
 * Function: GrantUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService GrantUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_GrantUriPermission_018, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    MyFlag::flag_ &= (~MyFlag::IS_SA_CALL);
    std::vector<std::string> uriStrVec;

    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uriStrVec, rawData);
    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    uint32_t flag = 1;
    std::string targetBundleName = "name1001";
    int32_t funcResult = -1;
    upms->GrantUriPermission(stubRawData, flag, targetBundleName, 0, 0, funcResult);
    EXPECT_EQ(funcResult, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    int32_t appIndex = 0;
    std::string targetBundleName = "com.example.testB1002";
    auto uri = Uri("file://com.example.testA/data/storage/el2/base/haps/entry/files/tets_A.txt");
    int32_t funcResult = -1;
    upms->RevokeUriPermissionManually(uri, targetBundleName, appIndex, funcResult);
    EXPECT_EQ(funcResult, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_002, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isSystemAppCall_ = true;
    int32_t appIndex = 0;
    std::string targetBundleName = "com.example.testB1002";
    auto uri = Uri("invalid://com.example.testA/data/storage/el2/base/haps/entry/files/test_A.txt");
    int32_t funcResult = -1;
    upms->RevokeUriPermissionManually(uri, targetBundleName, appIndex, funcResult);
    EXPECT_EQ(funcResult, ERR_CODE_INVALID_URI_TYPE);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_005, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isSystemAppCall_ = true;
    int32_t appIndex = 0;
    std::string targetBundleName = "com.example.test1002";
    auto docsUri = Uri("file://docs/DestTop/Text/test_001.txt");
    auto path = "/DestTop/Text/test_001.txt";
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app1001", "com.example.app1001");
    auto callerTokenId = 1001;
    auto targetTokenId = 1002;
    upms->AddPolicyRecordCache(callerTokenId, targetTokenId, path);
    bool recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, true);
    SandboxManagerKit::unSetPolicyRet_ = INNER_ERR;
    MyFlag::isUriTypeValid_ = true;
    MyFlag::upmsUtilsTokenId_ = 1002;
    MyFlag::getTokenIdByBundleNameStatus_ = ERR_OK;
    int32_t funcResult = -1;
    upms->RevokeUriPermissionManually(docsUri, targetBundleName, appIndex, funcResult);
    EXPECT_EQ(funcResult, INNER_ERR);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_006, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isSystemAppCall_ = true;
    int32_t appIndex = 0;
    auto docsUri = Uri("file://docs/DestTop/Text/test_001.txt");
    auto path = "/DestTop/Text/test_001.txt";
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.test1002", "com.example.test1002");
    auto callerTokenId = 1001;
    auto targetTokenId = 1002;
    upms->AddPolicyRecordCache(callerTokenId, targetTokenId, path);
    bool recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, true);
    std::string targetBundleName = "com.example.test1002";
    MyFlag::isUriTypeValid_ = true;
    MyFlag::upmsUtilsTokenId_ = 1002;
    MyFlag::getTokenIdByBundleNameStatus_ = ERR_OK;
    int32_t funcResult = -1;
    upms->RevokeUriPermissionManually(docsUri, targetBundleName, appIndex, funcResult);
    EXPECT_EQ(funcResult, ERR_OK);
    recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, false);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokeUriPermissionManually
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService RevokeUriPermissionManually
 */
HWTEST_F(UriPermissionImplTest, Upms_RevokeUriPermissionManually_007, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isSystemAppCall_ = true;
    int32_t appIndex = 0;
    auto docsUri = Uri("file://docs/DestTop/Text/test_001.txt");
    auto path = "/DestTop/Text/test_001.txt";
    IPCSkeleton::callerTokenId = 1002;
    MyFlag::tokenInfos[1002] = TokenInfo(1002, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.test1002", "com.example.test1002");
    auto callerTokenId = 1001;
    auto targetTokenId = 1002;
    upms->AddPolicyRecordCache(callerTokenId, targetTokenId, path);
    bool recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, true);
    std::string targetBundleName = "com.example.test1002";
    int32_t funcResult = -1;
    MyFlag::isUriTypeValid_ = true;
    MyFlag::upmsUtilsTokenId_ = 1002;
    MyFlag::getTokenIdByBundleNameStatus_ = ERR_OK;
    upms->RevokeUriPermissionManually(docsUri, targetBundleName, appIndex, funcResult);
    EXPECT_EQ(funcResult, ERR_OK);
    recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, false);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokePolicyUriPermission
 * SubFunction: NA
 * FunctionPoints: RevokePolicyUriPermission
 */
HWTEST_F(UriPermissionImplTest, UPMS_RevokePolicyUriPermission_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto callerTokenId = 1001;
    auto targetTokenId = 1002;
    std::string path = "/DestTop/Text/test_001.txt";
    upms->AddPolicyRecordCache(callerTokenId, targetTokenId, path);
    bool recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, true);
    upms->RevokePolicyUriPermission(targetTokenId);
    recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, false);
}

/*
 * Feature: URIPermissionManagerService
 * Function: RevokePolicyUriPermission
 * SubFunction: NA
 * FunctionPoints: RevokePolicyUriPermission
 */
HWTEST_F(UriPermissionImplTest, UPMS_RevokePolicyUriPermission_002, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto callerTokenId = 1001;
    auto targetTokenId = 1002;
    std::string path = "/DestTop/Text/test_002.txt";
    upms->AddPolicyRecordCache(callerTokenId, targetTokenId, path);
    bool recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, true);
    upms->RevokePolicyUriPermission(callerTokenId);
    recordExists = (upms->policyMap_.find(path) != upms->policyMap_.end());
    EXPECT_EQ(recordExists, true);
}

/*
 * Feature: URIPermissionManagerService
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService ConnectManager
 */
HWTEST_F(UriPermissionImplTest, Upms_ConnectManager_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    SystemAbilityManagerClient::nullptrFlag = true;
    sptr<StorageManager::IStorageManager> storageManager = nullptr;
    upms->ConnectManager(storageManager, -1);
    SystemAbilityManagerClient::nullptrFlag = false;
    EXPECT_EQ(storageManager, nullptr);
}

/*
 * Feature: URIPermissionManagerService
 * Function: ConnectManager
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService ConnectManager
 */
HWTEST_F(UriPermissionImplTest, Upms_ConnectManager_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    MockSystemAbilityManager::isNullptr = true;
    sptr<StorageManager::IStorageManager> storageManager = nullptr;
    upms->ConnectManager(storageManager, -1);
    MockSystemAbilityManager::isNullptr = false;
    EXPECT_EQ(storageManager, nullptr);
}

/*
 * Feature: URIPermissionManagerService
 * Function: VerifyUriPermission
 * SubFunction: NA
 * FunctionPoints: URIPermissionManagerService VerifyUriPermission
 */
HWTEST_F(UriPermissionImplTest, Upms_VerifyUriPermission_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ |= MyFlag::IS_SA_CALL;
    MyFlag::isDFSCallRet_ = true;
    MyFlag::isDocsCloudUri_ = true;
    auto callerTokenId = 1001;
    auto targetTokenId = 1002;
    auto invalidTokenId = 1003;
    std::string uri = "file://docs/test_001.jpg?networkid=10001";
    auto flagRead = 1;
    auto flagWrite = 2;
    auto flagReadWrite = 3;

    // read
    upms->AddTempUriPermission(uri, flagRead, callerTokenId, targetTokenId);
    bool funcResult = false;
    upms->VerifyUriPermission(Uri(uri), flagRead, targetTokenId, funcResult);
    EXPECT_EQ(funcResult, true);
    upms->VerifyUriPermission(Uri(uri), flagWrite, targetTokenId, funcResult);
    EXPECT_EQ(funcResult, false);
    upms->VerifyUriPermission(Uri(uri), flagReadWrite, targetTokenId, funcResult);
    EXPECT_EQ(funcResult, false);
    upms->uriMap_.clear();

    // write
    upms->AddTempUriPermission(uri, flagWrite, callerTokenId, targetTokenId);
    upms->VerifyUriPermission(Uri(uri), flagRead, targetTokenId, funcResult);
    EXPECT_EQ(funcResult, true);
    upms->VerifyUriPermission(Uri(uri), flagWrite, targetTokenId, funcResult);
    EXPECT_EQ(funcResult, true);
    upms->VerifyUriPermission(Uri(uri), flagReadWrite, targetTokenId, funcResult);
    EXPECT_EQ(funcResult, true);
    upms->uriMap_.clear();

    // flagReadWrite
    upms->AddTempUriPermission(uri, flagReadWrite, callerTokenId, targetTokenId);
    upms->VerifyUriPermission(Uri(uri), flagRead, targetTokenId, funcResult);
    EXPECT_EQ(funcResult, true);
    upms->VerifyUriPermission(Uri(uri), flagWrite, targetTokenId, funcResult);
    EXPECT_EQ(funcResult, true);
    upms->VerifyUriPermission(Uri(uri), flagReadWrite, targetTokenId, funcResult);
    EXPECT_EQ(funcResult, true);
    upms->uriMap_.clear();
    
    // no permission record
    upms->AddTempUriPermission(uri, flagReadWrite, callerTokenId, targetTokenId);
    upms->VerifyUriPermission(Uri(uri), flagRead, invalidTokenId, funcResult);
    EXPECT_EQ(funcResult, false);
}


/*
 * Feature: UriPermissionManagerStubImpl
 * Function: RevokeAllUriPermission
 * SubFunction: NA
 * FunctionPoints: RevokeAllUriPermission called by SA or SystemApp.
*/
HWTEST_F(UriPermissionImplTest, RevokeAllUriPermission_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    IPCSkeleton::callerUId = 5523;
    int32_t funcResult = -1;
    AbilityManagerClient::collaborator_ = std::make_shared<IAbilityManagerCollaborator>();
    auto collaborator = AbilityManagerClient::collaborator_;
    collaborator->RevokeUriPermission(0);
    AbilityManagerClient::collaborator_ = nullptr;
    MyFlag::upmsUtilsIsFoundationCallRet_ = true;
    upms->RevokeAllUriPermissions(1002, funcResult);
    IPCSkeleton::callerUId = 0;
    EXPECT_EQ(funcResult, ERR_OK);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: RevokeAllUriPermission
 * SubFunction: NA
 * FunctionPoints: RevokeAllUriPermission not called by SA or SystemApp.
*/
HWTEST_F(UriPermissionImplTest, RevokeAllUriPermission_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::flag_ &= (~MyFlag::IS_SA_CALL);
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "tempProcess");
    IPCSkeleton::callerTokenId = 1001;
    int32_t funcResult = -1;
    upms->RevokeAllUriPermissions(1002, funcResult);
    EXPECT_EQ(funcResult, CHECK_PERMISSION_FAILED);
}

#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
/*
 * Feature: UriPermissionManagerStubImpl
 * Function: Active
 * SubFunction: NA
 * FunctionPoints: Active Uri permission without FILE_ACCESS_PERSIST permission.
*/
HWTEST_F(UriPermissionImplTest, UPMS_Active_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    constexpr int32_t SANDBOX_MANAGER_PERMISSION_DENIED = 1;
    // get policy data
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    PolicyInfo policyInfo;
    policyInfo.path = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    policyInfo.mode = 1;
    std::vector<PolicyInfo> policyInfoArray = { policyInfo };
    UriPermissionRawData policyRawData;
    upmc.PolicyInfoToRawData(policyInfoArray, policyRawData);
    // make param
    std::vector<uint32_t> result;
    int32_t funcResult = -1;
    // call Active
    auto ret = upms->Active(policyRawData, result, funcResult);
    EXPECT_EQ(funcResult, SANDBOX_MANAGER_PERMISSION_DENIED);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: Active
 * SubFunction: NA
 * FunctionPoints: Active Uri permission with FILE_ACCESS_PERSIST permission.
*/
HWTEST_F(UriPermissionImplTest, UPMS_Active_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    // get policy data
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    PolicyInfo policyInfo;
    policyInfo.path = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    policyInfo.mode = 1;
    std::vector<PolicyInfo> policyInfoArray = { policyInfo };
    UriPermissionRawData policyRawData;
    upmc.PolicyInfoToRawData(policyInfoArray, policyRawData);
    // make param
    std::vector<uint32_t> result;
    int32_t funcResult = -1;
    // call Active
    MyFlag::permissionFileAccessPersist_ = true;
    SandboxManagerKit::startAccessingPolicyRet_ = ERR_OK;
    upms->Active(policyRawData, result, funcResult);
    EXPECT_EQ(funcResult, ERR_OK);

    SandboxManagerKit::startAccessingPolicyRet_ = INNER_ERR;
    upms->Active(policyRawData, result, funcResult);
    EXPECT_EQ(funcResult, INNER_ERR);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: Active
 * SubFunction: NA
 * FunctionPoints: policy vector is empty.
*/
HWTEST_F(UriPermissionImplTest, UPMS_Active_003, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    // get policy data
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::vector<PolicyInfo> policyInfoArray;
    UriPermissionRawData policyRawData;
    upmc.PolicyInfoToRawData(policyInfoArray, policyRawData);
    // make param
    std::vector<uint32_t> result;
    int32_t funcResult = -1;
    // call Active
    MyFlag::permissionFileAccessPersist_ = true;
    auto ret = upms->Active(policyRawData, result, funcResult);
    EXPECT_EQ(ret, ERR_URI_LIST_OUT_OF_RANGE);
}
#endif

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: do not have permission to call GrantUriPermissionPrivileged.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "tempProcess");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = false;

    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.app1002";
    uint32_t flag = 1;
    const std::vector<Uri> uris = { uri1 };
    std::vector<std::string> stringUris;
    for (const Uri& uri : uris) {
        stringUris.push_back(uri.ToString());
    }
    const std::vector<std::string> stringUriVec = stringUris;
    int32_t funcResult = -1;
    upms->GrantUriPermissionPrivileged(stringUriVec, flag, targetBundleName, 0, 0, 0, funcResult);
    EXPECT_EQ(funcResult, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: flag is 0.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = true;

    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.app1002";
    uint32_t flag = 0;
    const std::vector<Uri> uris = { uri1 };
    std::vector<std::string> stringUris;
    for (const Uri& uri : uris) {
        stringUris.push_back(uri.ToString());
    }
    const std::vector<std::string> stringUriVec = stringUris;
    int32_t funcResult = -1;
    upms->GrantUriPermissionPrivileged(stringUriVec, flag, targetBundleName, 0, 0, 0, funcResult);
    MyFlag::permissionPrivileged_ = false;
    EXPECT_EQ(funcResult, ERR_CODE_INVALID_URI_FLAG);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: targetBundleName is invalid.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_003, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = true;

    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.invalid";
    uint32_t flag = 1;
    const std::vector<Uri> uris = { uri1 };
    std::vector<std::string> stringUris;
    for (const Uri& uri : uris) {
        stringUris.push_back(uri.ToString());
    }
    const std::vector<std::string> stringUriVec = stringUris;
    int32_t funcResult = -1;
    MyFlag::isUriTypeValid_ = true;
    MyFlag::getTokenIdByBundleNameStatus_ = ERR_GET_TARGET_BUNDLE_INFO_FAILED;
    upms->GrantUriPermissionPrivileged(stringUriVec, flag, targetBundleName, 0, 0, 0, funcResult);
    MyFlag::permissionPrivileged_ = false;
    EXPECT_EQ(funcResult, ERR_GET_TARGET_BUNDLE_INFO_FAILED);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: type of uri is invalid.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_004, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = true;

    auto uri1 = Uri("http://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.app1002";
    uint32_t flag = 1;
    const std::vector<Uri> uris = { uri1 };
    std::vector<std::string> stringUris;
    for (const Uri& uri : uris) {
        stringUris.push_back(uri.ToString());
    }
    const std::vector<std::string> stringUriVec = stringUris;
    int32_t funcResult = -1;
    upms->GrantUriPermissionPrivileged(stringUriVec, flag, targetBundleName, 0, 0, 0, funcResult);
    MyFlag::permissionPrivileged_ = false;
    EXPECT_EQ(funcResult, ERR_CODE_INVALID_URI_TYPE);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: Create Share File failed.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_005, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = 1001;
    IPCSkeleton::callerUId = 5523;
    MyFlag::permissionPrivileged_ = true;
    MyFlag::permissionAllMedia_ = true;

    auto uri1 = Uri("file://docs/Photo/1/IMG_001/test_001.jpg?networkid=10001");
    std::string targetBundleName = "com.example.app1002";
    uint32_t flag = 1;
    const std::vector<Uri> uris = { uri1 };
    std::vector<std::string> stringUris;
    for (const Uri& uri : uris) {
        stringUris.push_back(uri.ToString());
    }
    const std::vector<std::string> stringUriVec = stringUris;
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();
    StorageManager::StorageManagerServiceMock::isZero = false;
    int32_t funcResult = -1;
    MyFlag::isUriTypeValid_ = true;
    upms->GrantUriPermissionPrivileged(stringUriVec, flag, targetBundleName, 0, 0, 0, funcResult);
    EXPECT_EQ(funcResult, INNER_ERR);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: do not have permission to call GrantUriPermissionPrivileged.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_007, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "tempProcess");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = false;

    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.app1002";
    uint32_t flag = 1;
    const std::vector<Uri> uris = { uri1 };
    std::vector<std::string> stringUris;
    for (const Uri& uri : uris) {
        stringUris.push_back(uri.ToString());
    }
    UriPermissionRawData rawData;
    upmc.StringVecToRawData(stringUris, rawData);
    int32_t funcResult = -1;
    MyFlag::isUriTypeValid_ = true;
    upms->GrantUriPermissionPrivileged(rawData, flag, targetBundleName, 0, 0, 0, funcResult);
    EXPECT_EQ(funcResult, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: flag is 0.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_008, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = true;

    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.app1002";
    uint32_t flag = 0;
    const std::vector<Uri> uris = { uri1 };
    std::vector<std::string> stringUris;
    for (const Uri& uri : uris) {
        stringUris.push_back(uri.ToString());
    }
    UriPermissionRawData rawData;
    upmc.StringVecToRawData(stringUris, rawData);
    int32_t funcResult = -1;
    MyFlag::isUriTypeValid_ = true;
    upms->GrantUriPermissionPrivileged(rawData, flag, targetBundleName, 0, 0, 0, funcResult);
    MyFlag::permissionPrivileged_ = false;
    EXPECT_EQ(funcResult, ERR_CODE_INVALID_URI_FLAG);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: targetBundleName is invalid.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_009, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = true;

    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.invalid";
    uint32_t flag = 1;
    const std::vector<Uri> uris = { uri1 };
    std::vector<std::string> stringUris;
    for (const Uri& uri : uris) {
        stringUris.push_back(uri.ToString());
    }
    UriPermissionRawData rawData;
    upmc.StringVecToRawData(stringUris, rawData);
    int32_t funcResult = -1;
    MyFlag::isUriTypeValid_ = true;
    MyFlag::getTokenIdByBundleNameStatus_ = ERR_GET_TARGET_BUNDLE_INFO_FAILED;
    upms->GrantUriPermissionPrivileged(rawData, flag, targetBundleName, 0, 0, 0, funcResult);
    MyFlag::permissionPrivileged_ = false;
    EXPECT_EQ(funcResult, ERR_GET_TARGET_BUNDLE_INFO_FAILED);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: type of uri is invalid.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_010, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = true;

    auto uri1 = Uri("http://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.app1002";
    uint32_t flag = 1;
    const std::vector<Uri> uris = { uri1 };
    std::vector<std::string> stringUris;
    for (const Uri& uri : uris) {
        stringUris.push_back(uri.ToString());
    }
    UriPermissionRawData rawData;
    upmc.StringVecToRawData(stringUris, rawData);
    int32_t funcResult = -1;
    MyFlag::isUriTypeValid_ = false;
    upms->GrantUriPermissionPrivileged(rawData, flag, targetBundleName, 0, 0, 0, funcResult);
    MyFlag::permissionPrivileged_ = false;
    EXPECT_EQ(funcResult, ERR_CODE_INVALID_URI_TYPE);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: GrantUriPermissionPrivileged
 * SubFunction: NA
 * FunctionPoints: Create Share File failed.
*/
HWTEST_F(UriPermissionImplTest, GrantUriPermissionPrivileged_011, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();

    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_NATIVE, "foundation");
    IPCSkeleton::callerTokenId = 1001;
    MyFlag::permissionPrivileged_ = true;

    auto uri1 = Uri("file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt");
    std::string targetBundleName = "com.example.app1002";
    uint32_t flag = 1;
    const std::vector<Uri> uris = { uri1 };
    std::vector<std::string> stringUris;
    for (const Uri& uri : uris) {
        stringUris.push_back(uri.ToString());
    }
    UriPermissionRawData rawData;
    upmc.StringVecToRawData(stringUris, rawData);
    upms->storageManager_ = new StorageManager::StorageManagerServiceMock();
    StorageManager::StorageManagerServiceMock::isZero = false;
    int32_t funcResult = -1;
    MyFlag::isUriTypeValid_ = true;
    upms->GrantUriPermissionPrivileged(rawData, flag, targetBundleName, 0, 0, -1, funcResult);
    MyFlag::permissionPrivileged_ = false;
    EXPECT_EQ(funcResult, INNER_ERR);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: CheckUriAuthorization not called by SA or SystemApp.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isPrivilegedSACall_ = false;
    std::string uri = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uris, rawData);

    UriPermissionRawData stubPolicyRawData;
    stubPolicyRawData.size = rawData.size;
    EXPECT_EQ(stubPolicyRawData.RawDataCpy(rawData.data), ERR_NONE);

    uint32_t flag = 1;
    uint32_t tokenId = 1001;
    std::vector<bool> expectRes(1, false);
    UriPermissionRawData funcResult;
    upms->CheckUriAuthorization(stubPolicyRawData, flag, tokenId, funcResult);

    std::vector<bool> expectResVec(1, true);
    upmc.RawDataToBoolVec(funcResult, expectResVec);
    EXPECT_EQ(expectResVec, expectRes);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: flag is 0.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isPrivilegedSACall_ = true;
    std::string uri = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uris, rawData);

    UriPermissionRawData stubPolicyRawData;
    stubPolicyRawData.size = rawData.size;
    EXPECT_EQ(stubPolicyRawData.RawDataCpy(rawData.data), ERR_NONE);

    uint32_t flag = 0;
    uint32_t tokenId = 1001;
    std::vector<bool> expectRes(1, false);
    UriPermissionRawData funcResult;
    upms->CheckUriAuthorization(stubPolicyRawData, flag, tokenId, funcResult);

    std::vector<bool> expectResVec(1, true);
    upmc.RawDataToBoolVec(funcResult, expectResVec);
    EXPECT_EQ(expectResVec, expectRes);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: uri is invalid.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_003, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isPrivilegedSACall_ = true;
    std::string uri = "http://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uris, rawData);

    UriPermissionRawData stubPolicyRawData;
    stubPolicyRawData.size = rawData.size;
    EXPECT_EQ(stubPolicyRawData.RawDataCpy(rawData.data), ERR_NONE);

    uint32_t flag = 1;
    uint32_t tokenId = 1001;
    std::vector<bool> expectRes(1, false);
    UriPermissionRawData funcResult;
    upms->CheckUriAuthorization(stubPolicyRawData, flag, tokenId, funcResult);

    std::vector<bool> expectResVec(1, true);
    upmc.RawDataToBoolVec(funcResult, expectResVec);
    EXPECT_EQ(expectResVec, expectRes);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: check uri authorization failed, have no permission.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_004, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isPrivilegedSACall_ = true;
    std::string uri = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uris, rawData);

    UriPermissionRawData stubPolicyRawData;
    stubPolicyRawData.size = rawData.size;
    EXPECT_EQ(stubPolicyRawData.RawDataCpy(rawData.data), ERR_NONE);

    uint32_t flag = 1;
    uint32_t tokenId = 1002;
    std::vector<bool> expectRes(1, false);
    UriPermissionRawData funcResult;
    MyFlag::upmsUtilsAlterBundleName_ = "com.example.app";
    MyFlag::upmsUtilsGetAlterBundleNameByTokenIdRet_ = true;
    upms->CheckUriAuthorization(stubPolicyRawData, flag, tokenId, funcResult);

    std::vector<bool> expectResVec(1, true);
    upmc.RawDataToBoolVec(funcResult, expectResVec);
    EXPECT_EQ(expectResVec, expectRes);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: check uri authorization success.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_005, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isPrivilegedSACall_ = true;
    MyFlag::tokenInfos[2001] = TokenInfo(2001, MyATokenTypeEnum::TOKEN_HAP,
        "com.example.app2001", "com.example.app2001");
    std::string uri = "file://com.example.app2001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uris, rawData);

    UriPermissionRawData stubPolicyRawData;
    stubPolicyRawData.size = rawData.size;
    EXPECT_EQ(stubPolicyRawData.RawDataCpy(rawData.data), ERR_NONE);
    uint32_t flag = 1;
    uint32_t tokenId = 2001;

    std::vector<bool> expectRes(1, true);
    UriPermissionRawData funcResult;
    MyFlag::upmsUtilsAlterBundleName_ = "com.example.app2001";
    MyFlag::upmsUtilsGetAlterBundleNameByTokenIdRet_ = true;
    upms->CheckUriAuthorization(stubPolicyRawData, flag, tokenId, funcResult);

    std::vector<bool> expectResVec(1, false);
    upmc.RawDataToBoolVec(funcResult, expectResVec);
    EXPECT_EQ(expectResVec, expectRes);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: CheckUriAuthorization not called by SA or SystemApp.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_006, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isPrivilegedSACall_ = false;
    std::string uri = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    std::vector<bool> funcResult(1, false);
    uint32_t flag = 1;
    uint32_t tokenId = 1001;
    upms->CheckUriAuthorization(uris, flag, tokenId, funcResult);

    std::vector<bool> expectRes(1, false);
    EXPECT_EQ(expectRes, funcResult);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: flag is 0.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_007, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isPrivilegedSACall_ = true;
    std::string uri = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    std::vector<bool> funcResult(1, false);
    uint32_t flag = 0;
    uint32_t tokenId = 1001;
    upms->CheckUriAuthorization(uris, flag, tokenId, funcResult);

    std::vector<bool> expectRes(1, false);
    EXPECT_EQ(expectRes, funcResult);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: uri is invalid.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_008, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isPrivilegedSACall_ = true;
    std::string uri = "http://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    std::vector<bool> funcResult(1, false);
    uint32_t flag = 1;
    uint32_t tokenId = 1001;
    upms->CheckUriAuthorization(uris, flag, tokenId, funcResult);

    std::vector<bool> expectRes(1, false);
    EXPECT_EQ(expectRes, funcResult);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: check uri authorization failed, have no permission.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_009, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isPrivilegedSACall_ = true;
    std::string uri = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    std::vector<bool> funcResult(1, false);
    uint32_t flag = 1;
    uint32_t tokenId = 1002;
    upms->CheckUriAuthorization(uris, flag, tokenId, funcResult);

    std::vector<bool> expectRes(1, false);
    EXPECT_EQ(expectRes, funcResult);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckUriAuthorization
 * SubFunction: NA
 * FunctionPoints: check uri authorization success.
*/
HWTEST_F(UriPermissionImplTest, CheckUriAuthorization_010, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isPrivilegedSACall_ = true;
    std::string uri = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    std::vector<bool> funcResult(1, false);
    uint32_t flag = 1;
    uint32_t tokenId = 1001;
    MyFlag::tokenInfos[1001] = TokenInfo(1001, MyATokenTypeEnum::TOKEN_HAP, "", "com.example.app1001");
    MyFlag::upmsUtilsAlterBundleName_ = "com.example.app1001";
    MyFlag::upmsUtilsGetAlterBundleNameByTokenIdRet_ = true;
    upms->CheckUriAuthorization(uris, flag, tokenId, funcResult);

    std::vector<bool> expectRes(1, true);
    EXPECT_EQ(expectRes, funcResult);
}

#ifdef ABILITY_RUNTIME_FEATURE_SANDBOXMANAGER
/*
 * Feature: UriPermissionManagerStubImpl
 * Function: RawDataToPolicyInfo
 * SubFunction: NA
 * FunctionPoints: call PolicyInfoToRawData and RawDataToPolicyInfo sucess.
*/
HWTEST_F(UriPermissionImplTest, RawDataToPolicyInfo_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    PolicyInfo policyInfo1;
    policyInfo1.path = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    policyInfo1.mode = 1;
    PolicyInfo policyInfo2;
    policyInfo2.path = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_002.txt";
    policyInfo2.mode = 2;
    std::vector<PolicyInfo> policyInfoArray;
    policyInfoArray.push_back(policyInfo1);
    policyInfoArray.push_back(policyInfo2);
    UriPermissionRawData policyRawData;
    upmc.PolicyInfoToRawData(policyInfoArray, policyRawData);

    UriPermissionRawData stubPolicyRawData;
    stubPolicyRawData.size = policyRawData.size;
    EXPECT_EQ(stubPolicyRawData.RawDataCpy(policyRawData.data), ERR_NONE);

    std::vector<PolicyInfo> resultPolicyInfo;
    upms->RawDataToPolicyInfo(stubPolicyRawData, resultPolicyInfo);
    EXPECT_EQ(policyInfoArray.size(), resultPolicyInfo.size());
    bool result = true;
    for (int32_t i = 0; i < policyInfoArray.size(); ++i) {
        if (policyInfoArray[i].path != resultPolicyInfo[i].path ||
            policyInfoArray[i].mode != resultPolicyInfo[i].mode) {
            result = false;
            break;
        }
    }
    EXPECT_TRUE(result);
}
#endif

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: RawDataToStringVec
 * SubFunction: NA
 * FunctionPoints: call StringVecToRawData and RawDataToStringVec sucess.
*/
HWTEST_F(UriPermissionImplTest, RawDataToStringVec_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uri = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris = { uri };
    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uris, rawData);

    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    std::vector<std::string> resultStringVec;
    upms->RawDataToStringVec(stubRawData, resultStringVec);
    EXPECT_EQ(uris.size(), resultStringVec.size());
    bool result = true;
    for (int32_t i = 0; i < uris.size(); ++i) {
        if (uris[i].compare(resultStringVec[i]) != 0) {
            result = false;
            break;
        }
    }
    EXPECT_TRUE(result);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: RawDataToStringVec
 * SubFunction: NA
 * FunctionPoints: call StringVecToRawData and RawDataToStringVec fail.
 */
HWTEST_F(UriPermissionImplTest, RawDataToStringVec_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::string uri = "file://com.example.app1001/data/storage/el2/base/haps/entry/files/test_001.txt";
    const std::vector<std::string> uris(300000, uri);
    UriPermissionRawData rawData;
    upmc.StringVecToRawData(uris, rawData);

    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    std::vector<std::string> resultStringVec;
    auto result = upms->RawDataToStringVec(stubRawData, resultStringVec);
    EXPECT_EQ(result, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: BoolVecToRawData
 * SubFunction: NA
 * FunctionPoints: call BoolVecToRawData and RawDataToBoolVec sucess.
*/
HWTEST_F(UriPermissionImplTest, BoolVecToRawData_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::vector<bool> boolVec(1, true);
    UriPermissionRawData rawData;
    std::vector<char> boolVecToCharVec;
    upms->BoolVecToRawData(boolVec, rawData, boolVecToCharVec);

    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    std::vector<bool> resultBoolVec(1, false);
    upmc.RawDataToBoolVec(stubRawData, resultBoolVec);
    EXPECT_EQ(boolVec.size(), resultBoolVec.size());
    bool result = true;
    for (int32_t i = 0; i < boolVec.size(); ++i) {
        if (boolVec[i] != resultBoolVec[i]) {
            result = false;
            break;
        }
    }
    EXPECT_TRUE(result);
}

#ifdef ABILITY_RUNTIME_UDMF_ENABLE
/*
 * Feature: UriPermissionManagerStubImpl
 * Function: UriPermissionImplTest
 * SubFunction: NA
 * FunctionPoints: capacity not support
 */
HWTEST_F(UriPermissionImplTest, Upmsi_CheckGrantUriPermissionByKeyAsCaller_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    AppUtils::isSupportGrantUriPermission_ = false;
    auto result = upms->CheckGrantUriPermissionByKeyAsCaller();
    EXPECT_EQ(result, ERR_CAPABILITY_NOT_SUPPORT);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckGrantUriPermissionByKeyAsCaller
 * SubFunction: NA
 * FunctionPoints: not system app call
 */
HWTEST_F(UriPermissionImplTest, Upmsi_CheckGrantUriPermissionByKeyAsCaller_002, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isSystemAppCall_ = false;
    auto result = upms->CheckGrantUriPermissionByKeyAsCaller();
    EXPECT_EQ(result, ERR_NOT_SYSTEM_APP);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckGrantUriPermissionByKeyAsCaller
 * SubFunction: NA
 * FunctionPoints: no grantUriPermissonByKeyAsCaller permission
 */
HWTEST_F(UriPermissionImplTest, Upmsi_CheckGrantUriPermissionByKeyAsCaller_003, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isSystemAppCall_ = true;
    MyFlag::permissionGrantUriPermissionAsCaller_ = false;
    auto result = upms->CheckGrantUriPermissionByKeyAsCaller();
    EXPECT_EQ(result, CHECK_PERMISSION_FAILED);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckGrantUriPermissionByKeyAsCaller
 * SubFunction: NA
 * FunctionPoints: CheckCalledBySandBox failed
 */
HWTEST_F(UriPermissionImplTest, Upmsi_CheckGrantUriPermissionByKeyAsCaller_004, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isSystemAppCall_ = true;
    MyFlag::permissionGrantUriPermissionAsCaller_ = true;
    MyFlag::isSandboxAppRet_ = true;
    auto result = upms->CheckGrantUriPermissionByKeyAsCaller();
    EXPECT_EQ(result, ERR_CODE_GRANT_URI_PERMISSION);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckGrantUriPermissionByKeyAsCaller
 * SubFunction: NA
 * FunctionPoints: success
 */
HWTEST_F(UriPermissionImplTest, Upmsi_CheckGrantUriPermissionByKeyAsCaller_005, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isSystemAppCall_ = true;
    MyFlag::permissionGrantUriPermissionAsCaller_ = true;
    MyFlag::isSandboxAppRet_ = false;
    auto result = upms->CheckGrantUriPermissionByKeyAsCaller();
    EXPECT_EQ(result, ERR_OK);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckGrantUriPermissionByKeyParams
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerStubImpl CheckGrantUriPermissionByKeyParams
 */
HWTEST_F(UriPermissionImplTest, Upmsi_CheckGrantUriPermissionByKeyParams_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    const std::string key = "";
    uint32_t flag = 0;
    FUDAppInfo calerAppInfo = { .tokenId = 1001 };
    FUDAppInfo targetAppInfo = { .tokenId = 1002 };
    std::vector<std::string> uris;
    auto ret = upms->CheckGrantUriPermissionByKeyParams(key, flag, calerAppInfo, targetAppInfo, uris);
    EXPECT_EQ(ret, ERR_CODE_INVALID_URI_FLAG);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckGrantUriPermissionByKeyParams
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerStubImpl CheckGrantUriPermissionByKeyParams
 */
HWTEST_F(UriPermissionImplTest, Upmsi_CheckGrantUriPermissionByKeyParams_002, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    const std::string key = "";
    uint32_t flag = 1;
    FUDAppInfo calerAppInfo = { .tokenId = 1001 };
    FUDAppInfo targetAppInfo = { .tokenId = 1001 };
    std::vector<std::string> uris;
    auto ret = upms->CheckGrantUriPermissionByKeyParams(key, flag, calerAppInfo, targetAppInfo, uris);
    EXPECT_EQ(ret, ERR_UPMS_INVALID_TARGET_TOKENID);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckGrantUriPermissionByKeyParams
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerStubImpl CheckGrantUriPermissionByKeyParams
 */
HWTEST_F(UriPermissionImplTest, Upmsi_CheckGrantUriPermissionByKeyParams_003, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    const std::string key = "";
    uint32_t flag = 1;
    FUDAppInfo calerAppInfo = { .tokenId = 1001 };
    FUDAppInfo targetAppInfo = { .tokenId = 1002 };
    std::vector<std::string> uris;
    MyFlag::fudUtilsGenerateFUDAppInfoRet_ = false;
    auto ret = upms->CheckGrantUriPermissionByKeyParams(key, flag, calerAppInfo, targetAppInfo, uris);
    EXPECT_EQ(ret, ERR_UPMS_INVALID_CALLER_TOKENID);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckGrantUriPermissionByKeyParams
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerStubImpl CheckGrantUriPermissionByKeyParams
 */
HWTEST_F(UriPermissionImplTest, Upmsi_CheckGrantUriPermissionByKeyParams_004, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    const std::string key = "";
    uint32_t flag = 1;
    FUDAppInfo calerAppInfo = { .tokenId = 1001 };
    FUDAppInfo targetAppInfo = { .tokenId = 1002 };
    std::vector<std::string> uris;
    MyFlag::processUdmfKeyRet_ = INNER_ERR;
    auto ret = upms->CheckGrantUriPermissionByKeyParams(key, flag, calerAppInfo, targetAppInfo, uris);
    EXPECT_EQ(ret, ERR_UPMS_GET_FILE_URIS_BY_KEY_FAILED);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckGrantUriPermissionByKeyParams
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerStubImpl CheckGrantUriPermissionByKeyParams
 */
HWTEST_F(UriPermissionImplTest, Upmsi_CheckGrantUriPermissionByKeyParams_005, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    const std::string key = "";
    uint32_t flag = 1;
    FUDAppInfo calerAppInfo = { .tokenId = 1001 };
    FUDAppInfo targetAppInfo = { .tokenId = 1002 };
    std::vector<std::string> uris;
    auto ret = upms->CheckGrantUriPermissionByKeyParams(key, flag, calerAppInfo, targetAppInfo, uris);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: CheckGrantUriPermissionByKeyParams
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerStubImpl CheckGrantUriPermissionByKeyParams
 */
HWTEST_F(UriPermissionImplTest, Upmsi_CheckGrantUriPermissionByKeyParams_006, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    const std::string key = "";
    uint32_t flag = 1;
    FUDAppInfo calerAppInfo = { .tokenId = 1001, .userId = 1 };
    FUDAppInfo targetAppInfo = { .tokenId = 1002, .userId = 2 };
    std::vector<std::string> uris;
    auto ret = upms->CheckGrantUriPermissionByKeyParams(key, flag, calerAppInfo, targetAppInfo, uris);
    EXPECT_EQ(ret, ERR_UPMS_INVALID_TARGET_TOKENID);
}

/*
 * Feature: UriPermissionManagerService
 * Function: GrantUriPermissionByKey
 * SubFunction: NA
 * FunctionPoints: GrantUriPermissionByKey
 */
HWTEST_F(UriPermissionImplTest, Upmsi_GrantUriPermissionByKey_001, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    std::string key = "udmfKey";
    uint32_t flag = 1;
    uint32_t targetTokenId = 1002;
    int32_t funcResult = -1;
    auto ret = upms->GrantUriPermissionByKey(key, flag, targetTokenId, funcResult);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(funcResult, ERR_NOT_SYSTEM_APP);
}

/*
 * Feature: UriPermissionManagerService
 * Function: GrantUriPermissionByKey
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService GrantUriPermissionByKey
 */
HWTEST_F(UriPermissionImplTest, Upmsi_GrantUriPermissionByKey_002, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isSystemAppCall_ = true;
    std::string key = "udmfKey";
    uint32_t flag = 1;
    uint32_t targetTokenId = 1002;
    int32_t funcResult = -1;
    MyFlag::isSandboxAppRet_ = false;
    IPCSkeleton::callerTokenId = targetTokenId;
    auto ret = upms->GrantUriPermissionByKey(key, flag, targetTokenId, funcResult);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(funcResult, ERR_UPMS_INVALID_TARGET_TOKENID);
}

/*
 * Feature: UriPermissionManagerService
 * Function: GrantUriPermissionByKey
 * SubFunction: NA
 * FunctionPoints: UriPermissionManagerService GrantUriPermissionByKey
 */
HWTEST_F(UriPermissionImplTest, Upmsi_GrantUriPermissionByKey_003, TestSize.Level1)
{
    auto upms = std::make_shared<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    MyFlag::isSystemAppCall_ = true;
    std::string key = "udmfKey";
    uint32_t flag = 1;
    uint32_t targetTokenId = 1002;
    int32_t funcResult = -1;
    MyFlag::isSandboxAppRet_ = false;
    MyFlag::fudUtilsGenerateFUDAppInfoRet_ = false;
    auto ret = upms->GrantUriPermissionByKey(key, flag, targetTokenId, funcResult);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(funcResult, INNER_ERR);
}
#endif // ABILITY_RUNTIME_UDMF_ENABLE

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: BoolVecToRawData
 * SubFunction: NA
 * FunctionPoints: call BoolVecToRawData and RawDataToBoolVec failed.
*/
HWTEST_F(UriPermissionImplTest, RawDataToBoolVec_001, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::vector<bool> boolVec;
    UriPermissionRawData rawData;
    std::vector<char> boolVecToCharVec;
    upms->BoolVecToRawData(boolVec, rawData, boolVecToCharVec);

    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    std::vector<bool> resultBoolVec(1, false);
    auto result = upmc.RawDataToBoolVec(stubRawData, resultBoolVec);
    EXPECT_EQ(result, false);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: BoolVecToRawData
 * SubFunction: NA
 * FunctionPoints: call BoolVecToRawData and RawDataToBoolVec failed.
*/
HWTEST_F(UriPermissionImplTest, RawDataToBoolVec_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::vector<bool> boolVec(MAX_URI_COUNT + 1, true);
    UriPermissionRawData rawData;
    std::vector<char> boolVecToCharVec;
    upms->BoolVecToRawData(boolVec, rawData, boolVecToCharVec);

    UriPermissionRawData stubRawData;
    stubRawData.size = rawData.size;
    EXPECT_EQ(stubRawData.RawDataCpy(rawData.data), ERR_NONE);

    std::vector<bool> resultBoolVec(1, false);
    auto result = upmc.RawDataToBoolVec(stubRawData, resultBoolVec);
    EXPECT_EQ(result, false);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: RawDataToPolicyInfo
 * SubFunction: NA
 * FunctionPoints: call PolicyInfoToRawData and RawDataToPolicyInfo failed.
*/
HWTEST_F(UriPermissionImplTest, RawDataToPolicyInfo_002, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    std::vector<PolicyInfo> policyInfoArray;
    UriPermissionRawData policyRawData;
    upmc.PolicyInfoToRawData(policyInfoArray, policyRawData);

    UriPermissionRawData stubPolicyRawData;
    stubPolicyRawData.size = policyRawData.size;
    EXPECT_EQ(stubPolicyRawData.RawDataCpy(policyRawData.data), ERR_NONE);

    std::vector<PolicyInfo> resultPolicyInfo;
    auto result = upms->RawDataToPolicyInfo(stubPolicyRawData, resultPolicyInfo);
    EXPECT_EQ(result, ERR_URI_LIST_OUT_OF_RANGE);
}

/*
 * Feature: UriPermissionManagerStubImpl
 * Function: RawDataToPolicyInfo
 * SubFunction: NA
 * FunctionPoints: call PolicyInfoToRawData and RawDataToPolicyInfo failed.
*/
HWTEST_F(UriPermissionImplTest, RawDataToPolicyInfo_003, TestSize.Level1)
{
    auto upms = std::make_unique<UriPermissionManagerStubImpl>();
    ASSERT_NE(upms, nullptr);
    auto& upmc = AAFwk::UriPermissionManagerClient::GetInstance();
    PolicyInfo policyInfo;
    policyInfo.path = POLICY_INFO_PATH;
    policyInfo.mode = 1;
    std::vector<PolicyInfo> policyInfoArray;
    for (int32_t i = 0; i < MAX_URI_COUNT + 1; ++i) {
        policyInfoArray.push_back(policyInfo);
    }
    UriPermissionRawData policyRawData;
    upmc.PolicyInfoToRawData(policyInfoArray, policyRawData);

    UriPermissionRawData stubPolicyRawData;
    stubPolicyRawData.size = policyRawData.size;
    EXPECT_EQ(stubPolicyRawData.RawDataCpy(policyRawData.data), ERR_NONE);

    std::vector<PolicyInfo> resultPolicyInfo;
    auto result = upms->RawDataToPolicyInfo(stubPolicyRawData, resultPolicyInfo);
    EXPECT_EQ(result, ERR_URI_LIST_OUT_OF_RANGE);
}
}  // namespace AAFwk
}  // namespace OHOS
