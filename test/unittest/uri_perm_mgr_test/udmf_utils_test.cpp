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
#define private public
#include "udmf_utils.h"
#undef private

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class UDMFUtils : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void UDMFUtils::SetUpTestCase(void)
{}

void UDMFUtils::TearDownTestCase(void)
{}

void UDMFUtils::SetUp()
{}

void UDMFUtils::TearDown()
{}

/**
 * @tc.number: AddPrivilege_0100
 * @tc.desc: Test AddPrivilege works
 * @tc.type: FUNC
 */
HWTEST_F(UDMFUtils, AddPrivilege_0100, TestSize.Level2)
{
    std::string key = "";
    uint32_t targetTokenId = 100002;
    auto ret = AAFwk::UDMFUtils::AddPrivilege(key, targetTokenId);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: AddPrivilege_0200
 * @tc.desc: Test AddPrivilege works
 * @tc.type: FUNC
 */
HWTEST_F(UDMFUtils, AddPrivilege_0200, TestSize.Level2)
{
    std::string key = "Testkey";
    uint32_t targetTokenId = 100002;
    auto ret = AAFwk::UDMFUtils::AddPrivilege(key, targetTokenId);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: AddPrivilege_0300
 * @tc.desc: Test AddPrivilege works
 * @tc.type: FUNC
 */
HWTEST_F(UDMFUtils, AddPrivilege_0300, TestSize.Level2)
{
    std::string key = "Testkey";
    uint32_t targetTokenId = 100002;
    std::string readPermission = "readAndKeep";
    auto ret = AAFwk::UDMFUtils::AddPrivilege(key, targetTokenId, readPermission);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: GetBatchData_0100
 * @tc.desc: Test GetBatchData works
 * @tc.type: FUNC
 */
HWTEST_F(UDMFUtils, GetBatchData_0100, TestSize.Level2)
{
    std::string key = "";
    std::vector<std::string> uris;
    auto ret = AAFwk::UDMFUtils::GetBatchData(key, uris);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: GetBatchData_0200
 * @tc.desc: Test GetBatchData works
 * @tc.type: FUNC
 */
HWTEST_F(UDMFUtils, GetBatchData_0200, TestSize.Level2)
{
    std::string key = "udmf://DataHub/com.example.insertdata/ap\\t5kKMYTOSHBh9\\f1@817VnBBvxI[e";
    std::vector<std::string> uris;
    auto ret = AAFwk::UDMFUtils::GetBatchData(key, uris);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: GetBatchData_0300
 * @tc.desc: Test GetBatchData works
 * @tc.type: FUNC
 */
HWTEST_F(UDMFUtils, GetBatchData_0300, TestSize.Level2)
{
    std::string key;
    UDMF::CustomOption option = {.intention = UDMF::Intention::UD_INTENTION_SYSTEM_SHARE};
    UDMF::UnifiedData unifiedData;
    auto record = std::make_shared<UDMF::UnifiedRecord>();
    unifiedData.AddRecord(record);
    UDMF::UdmfClient::GetInstance().SetData(option, unifiedData, key);
    std::vector<std::string> uris;
    auto ret = AAFwk::UDMFUtils::GetBatchData(key, uris);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: ProcessUdmfKey_0100
 * @tc.desc: Test ProcessUdmfKey works
 * @tc.type: FUNC
 */
HWTEST_F(UDMFUtils, ProcessUdmfKey_0100, TestSize.Level2)
{
    std::string key = "";
    uint32_t callerTokenId = 100001;
    uint32_t targetTokenId = 100002;
    std::vector<std::string> uris;
    auto ret = AAFwk::UDMFUtils::ProcessUdmfKey(key, callerTokenId, targetTokenId, uris);
    EXPECT_NE(ret, ERR_OK);
}

/**
 * @tc.number: ProcessUdmfKey_0200
 * @tc.desc: Test ProcessUdmfKey works
 * @tc.type: FUNC
 */
HWTEST_F(UDMFUtils, ProcessUdmfKey_0200, TestSize.Level2)
{
    std::string key = "Testkey";
    uint32_t callerTokenId = 100001;
    uint32_t targetTokenId = 100002;
    std::vector<std::string> uris;
    auto ret = AAFwk::UDMFUtils::ProcessUdmfKey(key, callerTokenId, targetTokenId, uris);
    EXPECT_NE(ret, ERR_OK);
}
}  // namespace AbilityRuntime
}  // namespace OHOS