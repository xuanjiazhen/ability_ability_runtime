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
#include "upms_udmf_utils.h"
#include "ability_manager_errors.h"
#include "ipc_skeleton.h"
#include "file_uri_distribution_utils.h"
#include "udmf_client.h"
#include "oh_mock_utils.h"

#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

using namespace UDMF;

constexpr uint32_t TEST_CALLER_TOKEN_ID = 1001;
constexpr uint32_t TEST_TARGET_TOKEN_ID = 1002;
constexpr uint32_t TEST_SELF_TOKEN_ID = 1003;
constexpr const char* TEST_KEY = "test_udmf_key";
constexpr const char* TEST_AUTHORITY = "com.example.test";
constexpr const char* TEST_URI = "file://media/test.jpg";

class UdmfUtilsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UdmfUtilsTest::SetUpTestCase() {}

void UdmfUtilsTest::TearDownTestCase() {}

void UdmfUtilsTest::SetUp() {}

void UdmfUtilsTest::TearDown() {}

/**
 * @tc.name: ProcessUdmfKey_NotCreatedByCaller_001
 * @tc.desc: Verify ProcessUdmfKey returns ERR_UPMS_KEY_IS_NOT_CREATE_BY_CALLER when key not created by caller.
 *           Branch: IsUdKeyCreateByCaller returns false
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_NotCreatedByCaller_001, TestSize.Level1)
{
    // Arrange: GetBundleNameByUdKey returns different authority than caller
    auto expectBundleNames = std::vector<std::string>{"other.bundle"};
    OH_EXPECT_RET(expectBundleNames, UdmfClient, GetBundleNameByUdKey, const std::string&);

    // GetAlterableBundleNameByTokenId returns caller authority
    std::vector<std::string> expectResults = {TEST_AUTHORITY};
    OH_EXPECT_RET_AND_OUTPUT({true}, expectResults, FUDUtils, GetAlterableBundleNameByTokenId,
        uint32_t, std::string& bundleName);

    // Act: Call ProcessUdmfKey
    std::vector<std::string> uris;
    int32_t ret = UDMFUtils::ProcessUdmfKey(TEST_KEY, TEST_CALLER_TOKEN_ID, TEST_TARGET_TOKEN_ID, uris);

    // Assert: Should return ERR_UPMS_KEY_IS_NOT_CREATE_BY_CALLER
    EXPECT_EQ(ret, ERR_UPMS_KEY_IS_NOT_CREATE_BY_CALLER);
}

/**
 * @tc.name: ProcessUdmfKey_AddPrivilegeTargetFailed_002
 * @tc.desc: Verify ProcessUdmfKey returns ERR_UPMS_ADD_PRIVILEGED_FAILED when AddPrivilege for target fails.
 *           Branch: IsUdKeyCreateByCaller returns true, AddPrivilege(target) fails
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_AddPrivilegeTargetFailed_002, TestSize.Level1)
{
    // Arrange: GetBundleNameByUdKey returns same authority as caller
    auto expectBundleNames = std::vector<std::string>{TEST_AUTHORITY};
    OH_EXPECT_RET(expectBundleNames, UdmfClient, GetBundleNameByUdKey, const std::string&);

    // GetAlterableBundleNameByTokenId returns caller authority
    std::vector<std::string> expectResults = {TEST_AUTHORITY};
    OH_EXPECT_RET_AND_OUTPUT({true}, expectResults, FUDUtils, GetAlterableBundleNameByTokenId,
        uint32_t, std::string& bundleName);

    // AddPrivilege fails
    auto expectAddPrivilegeRets = std::vector<int32_t>{INNER_ERR};
    OH_EXPECT_RET(expectAddPrivilegeRets, UdmfClient, AddPrivilege,
        const QueryOption&, const Privilege&);

    // GetSelfTokenID returns test token id
    auto expectSelfTokenIds = std::vector<uint64_t>{static_cast<uint64_t>(TEST_SELF_TOKEN_ID)};
    OH_EXPECT_RET(expectSelfTokenIds, IPCSkeleton, GetSelfTokenID);

    // Act: Call ProcessUdmfKey
    std::vector<std::string> uris;
    int32_t ret = UDMFUtils::ProcessUdmfKey(TEST_KEY, TEST_CALLER_TOKEN_ID, TEST_TARGET_TOKEN_ID, uris);

    // Assert: Should return ERR_UPMS_ADD_PRIVILEGED_FAILED
    EXPECT_EQ(ret, ERR_UPMS_ADD_PRIVILEGED_FAILED);
}

/**
 * @tc.name: ProcessUdmfKey_AddPrivilegeSelfFailed_003
 * @tc.desc: Verify ProcessUdmfKey returns ERR_UPMS_ADD_PRIVILEGED_FAILED when AddPrivilege for self fails.
 *           Branch: IsUdKeyCreateByCaller returns true, AddPrivilege(target) succeeds,
 *           AddPrivilege(self) fails
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_AddPrivilegeSelfFailed_003, TestSize.Level1)
{
    // Arrange: GetBundleNameByUdKey returns same authority as caller
    auto expectBundleNames = std::vector<std::string>{TEST_AUTHORITY};
    OH_EXPECT_RET(expectBundleNames, UdmfClient, GetBundleNameByUdKey, const std::string&);

    // GetAlterableBundleNameByTokenId returns caller authority
    std::vector<std::string> expectResults = {TEST_AUTHORITY};
    OH_EXPECT_RET_AND_OUTPUT({true}, expectResults, FUDUtils, GetAlterableBundleNameByTokenId,
        uint32_t, std::string& bundleName);

    // AddPrivilege for target succeeds, for self fails
    auto expectAddPrivilegeRets = std::vector<int32_t>{ERR_OK, INNER_ERR};
    OH_EXPECT_RET(expectAddPrivilegeRets, UdmfClient, AddPrivilege,
        const QueryOption&, const Privilege&);

    // GetSelfTokenID returns test token id
    auto expectSelfTokenIds = std::vector<uint64_t>{static_cast<uint64_t>(TEST_SELF_TOKEN_ID)};
    OH_EXPECT_RET(expectSelfTokenIds, IPCSkeleton, GetSelfTokenID);

    // Act: Call ProcessUdmfKey
    std::vector<std::string> uris;
    int32_t ret = UDMFUtils::ProcessUdmfKey(TEST_KEY, TEST_CALLER_TOKEN_ID, TEST_TARGET_TOKEN_ID, uris);

    // Assert: Should return ERR_UPMS_ADD_PRIVILEGED_FAILED
    EXPECT_EQ(ret, ERR_UPMS_ADD_PRIVILEGED_FAILED);
}

/**
 * @tc.name: ProcessUdmfKey_GetBatchDataFailed_004
 * @tc.desc: Verify ProcessUdmfKey returns ERR_UPMS_GET_FILE_URIS_BY_KEY_FAILED when GetBatchData fails.
 *           Branch: IsUdKeyCreateByCaller returns true, AddPrivilege(target) succeeds,
 *           AddPrivilege(self) succeeds, GetBatchData fails
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_GetBatchDataFailed_004, TestSize.Level1)
{
    // Arrange: GetBundleNameByUdKey returns same authority as caller
    auto expectBundleNames = std::vector<std::string>{TEST_AUTHORITY};
    OH_EXPECT_RET(expectBundleNames, UdmfClient, GetBundleNameByUdKey, const std::string&);

    // GetAlterableBundleNameByTokenId returns caller authority
    std::vector<std::string> expectResults = {TEST_AUTHORITY};
    OH_EXPECT_RET_AND_OUTPUT({true}, expectResults, FUDUtils, GetAlterableBundleNameByTokenId,
        uint32_t, std::string& bundleName);

    // AddPrivilege succeeds (called twice: for target and self)
    auto expectAddPrivilegeRets = std::vector<int32_t>{ERR_OK, ERR_OK};
    OH_EXPECT_RET(expectAddPrivilegeRets, UdmfClient, AddPrivilege,
        const QueryOption&, const Privilege&);

    // GetSelfTokenID returns test token id
    auto expectSelfTokenIds = std::vector<uint64_t>{static_cast<uint64_t>(TEST_SELF_TOKEN_ID)};
    OH_EXPECT_RET(expectSelfTokenIds, IPCSkeleton, GetSelfTokenID);

    // GetBatchData fails
    std::vector<std::vector<UnifiedData>> expectUnifiedDataList = {{}};
    OH_EXPECT_RET_AND_OUTPUT({INNER_ERR}, expectUnifiedDataList, UdmfClient, GetBatchData,
        const QueryOption&, std::vector<UnifiedData>& unifiedDataset);

    // Act: Call ProcessUdmfKey
    std::vector<std::string> uris;
    int32_t ret = UDMFUtils::ProcessUdmfKey(TEST_KEY, TEST_CALLER_TOKEN_ID, TEST_TARGET_TOKEN_ID, uris);

    // Assert: Should return ERR_UPMS_GET_FILE_URIS_BY_KEY_FAILED
    EXPECT_EQ(ret, ERR_UPMS_GET_FILE_URIS_BY_KEY_FAILED);
}

/**
 * @tc.name: ProcessUdmfKey_Success_005
 * @tc.desc: Verify ProcessUdmfKey returns ERR_OK on successful execution.
 *           Branch: All steps succeed
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_Success_005, TestSize.Level1)
{
    // Arrange: GetBundleNameByUdKey returns same authority as caller
    auto expectBundleNames = std::vector<std::string>{TEST_AUTHORITY};
    OH_EXPECT_RET(expectBundleNames, UdmfClient, GetBundleNameByUdKey, const std::string&);

    // GetAlterableBundleNameByTokenId returns caller authority
    std::vector<std::string> expectResults = {TEST_AUTHORITY};
    OH_EXPECT_RET_AND_OUTPUT({true}, expectResults, FUDUtils, GetAlterableBundleNameByTokenId,
        uint32_t, std::string& bundleName);

    // AddPrivilege succeeds (called twice: for target and self)
    auto expectAddPrivilegeRets = std::vector<int32_t>{ERR_OK, ERR_OK};
    OH_EXPECT_RET(expectAddPrivilegeRets, UdmfClient, AddPrivilege,
        const QueryOption&, const Privilege&);

    // GetSelfTokenID returns test token id
    auto expectSelfTokenIds = std::vector<uint64_t>{static_cast<uint64_t>(TEST_SELF_TOKEN_ID)};
    OH_EXPECT_RET(expectSelfTokenIds, IPCSkeleton, GetSelfTokenID);

    // GetBatchData succeeds with valid URIs
    auto record = std::make_shared<UnifiedRecord>(TEST_URI);
    std::vector<std::shared_ptr<UnifiedRecord>> records = {record};
    auto unifiedData = UnifiedData(records);
    std::vector<std::vector<UnifiedData>> expectUnifiedDataList = {{unifiedData}};
    OH_EXPECT_RET_AND_OUTPUT({ERR_OK}, expectUnifiedDataList, UdmfClient, GetBatchData,
        const QueryOption&, std::vector<UnifiedData>& unifiedDataset);

    // Act: Call ProcessUdmfKey
    std::vector<std::string> uris;
    int32_t ret = UDMFUtils::ProcessUdmfKey(TEST_KEY, TEST_CALLER_TOKEN_ID, TEST_TARGET_TOKEN_ID, uris);

    // Assert: Should return ERR_OK and contain the test URI
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_EQ(uris.size(), 1u);
    EXPECT_EQ(uris[0], TEST_URI);
}

/**
 * @tc.name: ProcessUdmfKey_GetAlterableBundleNameFailed_006
 * @tc.desc: Verify ProcessUdmfKey returns ERR_UPMS_KEY_IS_NOT_CREATE_BY_CALLER when
 *           GetAlterableBundleNameByTokenId fails.
 *           Branch: IsUdKeyCreateByCaller returns false (GetAlterableBundleNameByTokenId returns false)
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_GetAlterableBundleNameFailed_006, TestSize.Level1)
{
    // Arrange: GetBundleNameByUdKey returns same authority as caller
    auto expectBundleNames = std::vector<std::string>{TEST_AUTHORITY};
    OH_EXPECT_RET(expectBundleNames, UdmfClient, GetBundleNameByUdKey, const std::string&);

    // GetAlterableBundleNameByTokenId fails (returns false)
    std::vector<std::string> expectResults = {TEST_AUTHORITY};
    OH_EXPECT_RET_AND_OUTPUT({false}, expectResults, FUDUtils, GetAlterableBundleNameByTokenId,
        uint32_t, std::string& bundleName);

    // Act: Call ProcessUdmfKey
    std::vector<std::string> uris;
    int32_t ret = UDMFUtils::ProcessUdmfKey(TEST_KEY, TEST_CALLER_TOKEN_ID, TEST_TARGET_TOKEN_ID, uris);

    // Assert: Should return ERR_UPMS_KEY_IS_NOT_CREATE_BY_CALLER
    EXPECT_EQ(ret, ERR_UPMS_KEY_IS_NOT_CREATE_BY_CALLER);
}

}  // namespace AAFwk
}  // namespace OHOS
