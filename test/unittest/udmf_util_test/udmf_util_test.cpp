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
#include "udmf_util.h"
#undef private
#include "hilog_tag_wrapper.h"
#include "udmf_client.h"
#include "ability_manager_errors.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {
class UdmfUtilTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
};

void UdmfUtilTest::SetUpTestCase() {}

void UdmfUtilTest::TearDownTestCase() {}

void UdmfUtilTest::SetUp()
{
    UDMF::UdmfClient::Init();
}

void UdmfUtilTest::TearDown() {}

/*
 * Feature: UdmfUtilTest
 * Function: AddPrivilege_001
 * SubFunction: NA
 */
HWTEST_F(UdmfUtilTest, AddPrivilege_001, TestSize.Level1)
{
    std::string key = "udmfKey";
    uint32_t targetTokenId = 100001;
    std::string readPermission = "";
    auto ret = UdmfUtil::AddPrivilege(key, targetTokenId, readPermission);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: UdmfUtilTest
 * Function: AddPrivilege_002
 * SubFunction: NA
 */
HWTEST_F(UdmfUtilTest, AddPrivilege_002, TestSize.Level1)
{
    std::string key = "udmfKey";
    uint32_t targetTokenId = 100001;
    std::string readPermission = "";
    UDMF::UdmfClient::addPrivilegeRet_ = INNER_ERR;
    auto ret = UdmfUtil::AddPrivilege(key, targetTokenId, readPermission);
    EXPECT_EQ(ret, INNER_ERR);
}

/*
 * Feature: UdmfUtilTest
 * Function: ProcessUdmfKey_001
 * SubFunction: NA
 */
HWTEST_F(UdmfUtilTest, ProcessUdmfKey_001, TestSize.Level1)
{
    std::string key = "udmfKey";
    uint32_t targetTokenId = 100001;
    auto ret = UdmfUtil::ProcessUdmfKey(key, targetTokenId);
    EXPECT_EQ(ret, ERR_OK);
}

/*
 * Feature: UdmfUtilTest
 * Function: ProcessUdmfKey_002
 * SubFunction: NA
 */
HWTEST_F(UdmfUtilTest, ProcessUdmfKey_002, TestSize.Level1)
{
    std::string key = "udmfKey";
    uint32_t targetTokenId = 100001;
    UDMF::UdmfClient::addPrivilegeRet_ = INNER_ERR;
    auto ret = UdmfUtil::ProcessUdmfKey(key, targetTokenId);
    EXPECT_EQ(ret, ERR_UPMS_ADD_PRIVILEGED_FAILED);
}
} // namespace AAFwk
} // namespace OHOS
