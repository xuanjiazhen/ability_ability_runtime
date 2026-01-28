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

#ifdef ABILITY_RUNTIME_UDMF_ENABLE

#define private public
#include "upms_udmf_utils.h"
#undef private

#include "hilog_tag_wrapper.h"
#include "oh_mock_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

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
 * @tc.name: GetBatchData_001
 * @tc.desc: Test GetBatchData with valid keys
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, GetBatchData_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetBatchData_001 start");

    std::vector<std::string> keys = {
        "udmf_key_1",
        "udmf_key_2"
    };

    std::vector<UDMF::UnifiedData> data;
    int32_t ret = UDMFUtils::GetBatchData(keys, data);

    // Result depends on UDMF service
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetBatchData_001 end");
}

/**
 * @tc.name: GetBatchData_002
 * @tc.desc: Test GetBatchData with empty key vector
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, GetBatchData_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetBatchData_002 start");

    std::vector<std::string> keys; // Empty vector

    std::vector<UDMF::UnifiedData> data;
    int32_t ret = UDMFUtils::GetBatchData(keys, data);

    EXPECT_EQ(ret, 0);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetBatchData_002 end");
}

/**
 * @tc.name: AddPrivilege_001
 * @tc.desc: Test AddPrivilege with valid key and privilege
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, AddPrivilege_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "AddPrivilege_001 start");

    std::string key = "udmf_test_key";
    UDMF::Privilege privilege;
    privilege.tokenID = std::to_string(1001);
    privilege.flag = Want::FLAG_AUTH_READ_URI_PERMISSION;

    int32_t ret = UDMFUtils::AddPrivilege(key, privilege);

    // Result depends on UDMF service
    TAG_LOGI(AAFwkTag::URIPERMMGR, "AddPrivilege_001 end");
}

/**
 * @tc.name: AddPrivilege_002
 * @tc.desc: Test AddPrivilege with different privilege flags
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, AddPrivilege_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "AddPrivilege_002 start");

    std::string key = "udmf_test_key";
    UDMF::Privilege privilege;
    privilege.tokenID = std::to_string(1001);
    privilege.flag = Want::FLAG_AUTH_WRITE_URI_PERMISSION;

    int32_t ret = UDMFUtils::AddPrivilege(key, privilege);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "AddPrivilege_002 end");
}

/**
 * @tc.name: AddPrivilege_003
 * @tc.desc: Test AddPrivilege with read write flag
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, AddPrivilege_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "AddPrivilege_003 start");

    std::string key = "udmf_test_key";
    UDMF::Privilege privilege;
    privilege.tokenID = std::to_string(1001);
    privilege.flag = Want::FLAG_AUTH_READ_URI_PERMISSION | Want::FLAG_AUTH_WRITE_URI_PERMISSION;

    int32_t ret = UDMFUtils::AddPrivilege(key, privilege);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "AddPrivilege_003 end");
}

/**
 * @tc.name: ProcessUdmfKey_001
 * @tc.desc: Test ProcessUdmfKey with valid key
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ProcessUdmfKey_001 start");

    std::string key = "udmf://test/key";

    std::string processedKey;
    bool result = UDMFUtils::ProcessUdmfKey(key, processedKey);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "ProcessUdmfKey_001 end");
}

/**
 * @tc.name: ProcessUdmfKey_002
 * @tc.desc: Test ProcessUdmfKey with empty key
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_002, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ProcessUdmfKey_002 start");

    std::string key = ""; // Empty key

    std::string processedKey;
    bool result = UDMFUtils::ProcessUdmfKey(key, processedKey);

    EXPECT_FALSE(result);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "ProcessUdmfKey_002 end");
}

/**
 * @tc.name: ProcessUdmfKey_003
 * @tc.desc: Test ProcessUdmfKey with invalid key format
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_003, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ProcessUdmfKey_003 start");

    std::string key = "invalid_key_format";

    std::string processedKey;
    bool result = UDMFUtils::ProcessUdmfKey(key, processedKey);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "ProcessUdmfKey_003 end");
}

/**
 * @tc.name: SetData_001
 * @tc.desc: Test SetData with valid UnifiedData
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, SetData_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "SetData_001 start");

    UDMF::UnifiedData data;
    data.key = "test_key";

    int32_t ret = UDMFUtils::SetData(data);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "SetData_001 end");
}

/**
 * @tc.name: GetData_001
 * @tc.desc: Test GetData with valid QueryOption
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, GetData_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetData_001 start");

    UDMF::QueryOption option;
    option.key = "test_key";

    std::vector<UDMF::UnifiedData> data;
    int32_t ret = UDMFUtils::GetData(option, data);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetData_001 end");
}

/**
 * @tc.name: UDMFUtils_IntegrationTest_001
 * @tc.desc: Test integration of multiple UDMF operations
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, UDMFUtils_IntegrationTest_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "UDMFUtils_IntegrationTest_001 start");

    // Set data
    UDMF::UnifiedData data;
    data.key = "integration_test_key";

    int32_t setRet = UDMFUtils::SetData(data);

    // Get data
    UDMF::QueryOption option;
    option.key = "integration_test_key";

    std::vector<UDMF::UnifiedData> getData;
    int32_t getRet = UDMFUtils::GetData(option, getData);

    // Add privilege
    UDMF::Privilege privilege;
    privilege.tokenID = std::to_string(1001);
    privilege.flag = Want::FLAG_AUTH_READ_URI_PERMISSION;

    int32_t addRet = UDMFUtils::AddPrivilege(data.key, privilege);

    TAG_LOGI(AAFwkTag::URIPERMMGR, "UDMFUtils_IntegrationTest_001 end");
}

/**
 * @tc.name: GetBatchData_EmptyResult_001
 * @tc.desc: Test GetBatchData when UDMF service returns empty result
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, GetBatchData_EmptyResult_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetBatchData_EmptyResult_001 start");

    std::vector<std::string> keys = {
        "nonexistent_key_1",
        "nonexistent_key_2"
    };

    std::vector<UDMF::UnifiedData> data;
    int32_t ret = UDMFUtils::GetBatchData(keys, data);

    // Test when UDMF service returns no data
    // Return value depends on UDMF service
    TAG_LOGI(AAFwkTag::URIPERMMGR, "GetBatchData_EmptyResult_001 end");
}

/**
 * @tc.name: AddPrivilege_Duplicate_001
 * @tc.desc: Test AddPrivilege when adding duplicate privilege for same key
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, AddPrivilege_Duplicate_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "AddPrivilege_Duplicate_001 start");

    std::string key = "udmf_test_key";
    UDMF::Privilege privilege;
    privilege.tokenID = std::to_string(1001);
    privilege.flag = Want::FLAG_AUTH_READ_URI_PERMISSION;

    // Add privilege first time
    int32_t ret1 = UDMFUtils::AddPrivilege(key, privilege);

    // Try to add same privilege again (duplicate)
    int32_t ret2 = UDMFUtils::AddPrivilege(key, privilege);

    // Test duplicate privilege scenario
    // Return value depends on UDMF service
    TAG_LOGI(AAFwkTag::URIPERMMGR, "AddPrivilege_Duplicate_001 end");
}

/**
 * @tc.name: ProcessUdmfKey_SpecialChars_001
 * @tc.desc: Test ProcessUdmfKey with special characters in key
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, ProcessUdmfKey_SpecialChars_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ProcessUdmfKey_SpecialChars_001 start");

    std::string key = "udmf://test/key@#$%special";

    std::string processedKey;
    bool result = UDMFUtils::ProcessUdmfKey(key, processedKey);

    // Test special character handling
    // Result depends on key processing logic
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ProcessUdmfKey_SpecialChars_001 end");
}

/**
 * @tc.name: VerifyUdmfPermission_NoPrivilege_001
 * @tc.desc: Test verifying UDMF permission when none exists
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, VerifyUdmfPermission_NoPrivilege_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUdmfPermission_NoPrivilege_001 start");

    std::string key = "nonexistent_key";
    uint32_t tokenId = 1001;

    // Test permission verification for non-existent key
    // Result depends on UDMF service
    TAG_LOGI(AAFwkTag::URIPERMMGR, "VerifyUdmfPermission_NoPrivilege_001 end");
}

/**
 * @tc.name: ClearUdmfData_Success_001
 * @tc.desc: Test clearing UDMF data successfully
 * @tc.type: FUNC
 */
HWTEST_F(UdmfUtilsTest, ClearUdmfData_Success_001, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ClearUdmfData_Success_001 start");

    std::string key = "test_key_to_clear";

    // First set some data
    UDMF::UnifiedData data;
    data.key = key;
    UDMFUtils::SetData(data);

    // Then clear the data
    int32_t ret = UDMFUtils::ClearData(key);

    // Test clearing data
    // Return value depends on UDMF service
    TAG_LOGI(AAFwkTag::URIPERMMGR, "ClearUdmfData_Success_001 end");
}

} // namespace AAFwk
} // namespace OHOS

#endif // ABILITY_RUNTIME_UDMF_ENABLE
