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
#include <mutex>

#define private public
#include "ability_auto_startup_client.h"
#undef private

#include "refbase.h"
#include "oh_mock_utils.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AbilityAutoStartupClientTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AbilityAutoStartupClientTest::SetUpTestCase()
{}

void AbilityAutoStartupClientTest::TearDownTestCase()
{}

void AbilityAutoStartupClientTest::SetUp()
{}

void AbilityAutoStartupClientTest::TearDown()
{
    AAFwk::AbilityAutoStartupClient::instance_ = nullptr;
}

/**
 * @tc.name: GetInstance_001
 * @tc.desc: Verify GetInstance call
 * @tc.type: FUNC
 */
HWTEST_F(AbilityAutoStartupClientTest, GetInstance_001, TestSize.Level1)
{
    EXPECT_EQ(AAFwk::AbilityAutoStartupClient::instance_, nullptr);
    std::shared_ptr<AAFwk::AbilityAutoStartupClient> instance = AAFwk::AbilityAutoStartupClient::GetInstance();
    EXPECT_NE(AAFwk::AbilityAutoStartupClient::instance_, nullptr);
}

/**
 * @tc.name: Connect_001
 * @tc.desc: Verify Connect call when proxy is not nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AbilityAutoStartupClientTest, Connect_001, TestSize.Level1)
{
    EXPECT_EQ(AAFwk::AbilityAutoStartupClient::instance_, nullptr);
    std::shared_ptr<AAFwk::AbilityAutoStartupClient> instance = AAFwk::AbilityAutoStartupClient::GetInstance();
    EXPECT_NE(AAFwk::AbilityAutoStartupClient::instance_, nullptr);
    instance->proxy_ = sptr<AAFwk::IAbilityManager>::MakeSptr();
    EXPECT_EQ(instance->Connect(), ERR_OK);
}

/**
 * @tc.name: Connect_002
 * @tc.desc: Verify Connect call when systemManager is nullptr
 * @tc.type: FUNC
 */
HWTEST_F(AbilityAutoStartupClientTest, Connect_002, TestSize.Level1)
{
    EXPECT_EQ(AAFwk::AbilityAutoStartupClient::instance_, nullptr);
    std::shared_ptr<AAFwk::AbilityAutoStartupClient> instance = AAFwk::AbilityAutoStartupClient::GetInstance();
    EXPECT_NE(AAFwk::AbilityAutoStartupClient::instance_, nullptr);
    OH_MOCK_METHOD_EXPECT_RET({nullptr}, SystemAbilityManagerClient, GetSystemAbilityManager);
    EXPECT_EQ(instance->Connect(), ERR_OK);
}
} // namespace AppExecFwk
} // namespace OHOS