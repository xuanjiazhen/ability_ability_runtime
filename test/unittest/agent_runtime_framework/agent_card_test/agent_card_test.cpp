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
#include <gmock/gmock.h>
#include "agent_card.h"

using namespace OHOS;
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AgentRuntime {
class AgentCardTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp() override;
    void TearDown() override;
};

void AgentCardTest::SetUpTestCase(void)
{}

void AgentCardTest::TearDownTestCase(void)
{}

void AgentCardTest::SetUp(void)
{}

void AgentCardTest::TearDown(void)
{}

/**
 * @tc.name: ProviderMarshallingTest_001
 * @tc.desc: Test Provider Marshalling method with valid data
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderMarshallingTest_001, TestSize.Level1)
{
    Provider provider;
    provider.organization = "test1";
    provider.url = "abc";

    Parcel parcelMock;
    bool result = provider.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: ProviderMarshallingTest_002
 * @tc.desc: Test Provider Marshalling method with empty strings
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderMarshallingTest_002, TestSize.Level1)
{
    Provider provider;
    provider.organization = "";
    provider.url = "";

    Parcel parcelMock;
    bool result = provider.Marshalling(parcelMock);

    EXPECT_TRUE(result);
}

/**
 * @tc.name: ProviderMarshallingTest_003
 * @tc.desc: Test Provider Marshalling method when WriteString fails for organization
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderMarshallingTest_003, TestSize.Level1)
{
    Provider provider;
    provider.organization = "test";
    provider.url = "test";

    Parcel parcelMock;
    bool result = provider.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}

/**
 * @tc.name: ProviderMarshallingTest_004
 * @tc.desc: Test Provider Marshalling method when WriteString fails for url
 * @tc.type: FUNC
 * @tc.require: AR000H1N32
 */
HWTEST_F(AgentCardTest, ProviderMarshallingTest_004, TestSize.Level1)
{
    Provider provider;
    provider.organization = "test";
    provider.url = "test";

    Parcel parcelMock;
    bool result = provider.Marshalling(parcelMock);

    EXPECT_FALSE(result);
}
} // namespace AgentRuntime
} // namespace OHOS