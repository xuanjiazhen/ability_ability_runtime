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
#include <gmock/gmock.h>

#include "js_agent_connector_stub_impl.h"
#include "js_agent_connection.h"
#include "js_agent_connector_stub_impl_test_mock.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AgentRuntime {
class JsAgentConnectorStubImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    napi_env env_{reinterpret_cast<napi_env>(0x1234)};
};

void JsAgentConnectorStubImplTest::SetUpTestCase(void)
{}
void JsAgentConnectorStubImplTest::TearDownTestCase(void)
{}
void JsAgentConnectorStubImplTest::SetUp()
{}
void JsAgentConnectorStubImplTest::TearDown()
{}

/*
 * Feature: JsAgentConnectorStubImpl
 * Function: Constructor
 * SubFunction: NA
 * FunctionPoints: JsAgentConnectorStubImpl construction
 * EnvConditions: NA
 * CaseDescription: Verify that JsAgentConnectorStubImpl can be constructed successfully
 */
HWTEST_F(JsAgentConnectorStubImplTest, JsAgentConnectorStubImpl_Constructor_001, TestSize.Level1)
{
    wptr<JSAgentConnection> weakConnection;

    sptr<JsAgentConnectorStubImpl> stub = new JsAgentConnectorStubImpl(weakConnection);

    EXPECT_NE(stub, nullptr);
}

/*
 * Feature: JsAgentConnectorStubImpl
 * Function: Constructor
 * SubFunction: NA
 * FunctionPoints: JsAgentConnectorStubImpl construction with valid connection
 * EnvConditions: NA
 * CaseDescription: Verify that JsAgentConnectorStubImpl can be constructed with valid connection
 */
HWTEST_F(JsAgentConnectorStubImplTest, JsAgentConnectorStubImpl_Constructor_002, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMockImpl> connection = new JSAgentConnectionTestMockImpl(env_);
    wptr<JSAgentConnection> weakConnection = connection;

    sptr<JsAgentConnectorStubImpl> stub = new JsAgentConnectorStubImpl(weakConnection);

    EXPECT_NE(stub, nullptr);
}

/*
 * Feature: JsAgentConnectorStubImpl
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with null connection
 * EnvConditions: Connection is null (expired weak pointer)
 * CaseDescription: Verify that SendData returns error when connection is null
 */
HWTEST_F(JsAgentConnectorStubImplTest, JsAgentConnectorStubImpl_SendData_001, TestSize.Level1)
{
    wptr<JSAgentConnection> weakConnection;  // Null weak pointer
    sptr<JsAgentConnectorStubImpl> stub = new JsAgentConnectorStubImpl(weakConnection);

    std::string testData = "test data";
    int32_t result = stub->SendData(testData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
}

/*
 * Feature: JsAgentConnectorStubImpl
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with valid connection
 * EnvConditions: Valid connection
 * CaseDescription: Verify that SendData forwards to connection successfully
 */
HWTEST_F(JsAgentConnectorStubImplTest, JsAgentConnectorStubImpl_SendData_002, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMockImpl> connection = new JSAgentConnectionTestMockImpl(env_);
    wptr<JSAgentConnection> weakConnection = connection;
    sptr<JsAgentConnectorStubImpl> stub = new JsAgentConnectorStubImpl(weakConnection);

    std::string testData = "test data";
    int32_t result = stub->SendData(testData);

    EXPECT_EQ(result, 0);
}

/*
 * Feature: JsAgentConnectorStubImpl
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with empty string
 * EnvConditions: Valid connection
 * CaseDescription: Verify that SendData works with empty string
 */
HWTEST_F(JsAgentConnectorStubImplTest, JsAgentConnectorStubImpl_SendData_003, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMockImpl> connection = new JSAgentConnectionTestMockImpl(env_);
    wptr<JSAgentConnection> weakConnection = connection;
    sptr<JsAgentConnectorStubImpl> stub = new JsAgentConnectorStubImpl(weakConnection);

    std::string emptyData = "";
    int32_t result = stub->SendData(emptyData);

    EXPECT_EQ(result, 0);
}

/*
 * Feature: JsAgentConnectorStubImpl
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with long string
 * EnvConditions: Valid connection
 * CaseDescription: Verify that SendData works with long string
 */
HWTEST_F(JsAgentConnectorStubImplTest, JsAgentConnectorStubImpl_SendData_004, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMockImpl> connection = new JSAgentConnectionTestMockImpl(env_);
    wptr<JSAgentConnection> weakConnection = connection;
    sptr<JsAgentConnectorStubImpl> stub = new JsAgentConnectorStubImpl(weakConnection);

    std::string longData(10000, 'x');
    int32_t result = stub->SendData(longData);

    EXPECT_EQ(result, 0);
}

/*
 * Feature: JsAgentConnectorStubImpl
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData with expired weak pointer
 * EnvConditions: Connection object destroyed
 * CaseDescription: Verify that SendData returns error when connection is destroyed
 */
HWTEST_F(JsAgentConnectorStubImplTest, JsAgentConnectorStubImpl_SendData_005, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMockImpl> connection = new JSAgentConnectionTestMockImpl(env_);
    wptr<JSAgentConnection> weakConnection = connection;
    sptr<JsAgentConnectorStubImpl> stub = new JsAgentConnectorStubImpl(weakConnection);

    // Destroy the connection
    connection = nullptr;

    std::string testData = "test data";
    int32_t result = stub->SendData(testData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
}

/*
 * Feature: JsAgentConnectorStubImpl
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with null connection
 * EnvConditions: Connection is null (expired weak pointer)
 * CaseDescription: Verify that Authorize returns error when connection is null
 */
HWTEST_F(JsAgentConnectorStubImplTest, JsAgentConnectorStubImpl_Authorize_001, TestSize.Level1)
{
    wptr<JSAgentConnection> weakConnection;  // Null weak pointer
    sptr<JsAgentConnectorStubImpl> stub = new JsAgentConnectorStubImpl(weakConnection);

    std::string authData = "auth data";
    int32_t result = stub->Authorize(authData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
}

/*
 * Feature: JsAgentConnectorStubImpl
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with valid connection
 * EnvConditions: Valid connection
 * CaseDescription: Verify that Authorize forwards to connection successfully
 */
HWTEST_F(JsAgentConnectorStubImplTest, JsAgentConnectorStubImpl_Authorize_002, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMockImpl> connection = new JSAgentConnectionTestMockImpl(env_);
    wptr<JSAgentConnection> weakConnection = connection;
    sptr<JsAgentConnectorStubImpl> stub = new JsAgentConnectorStubImpl(weakConnection);

    std::string authData = "auth data";
    int32_t result = stub->Authorize(authData);

    EXPECT_EQ(result, 0);
}

/*
 * Feature: JsAgentConnectorStubImpl
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with empty string
 * EnvConditions: Valid connection
 * CaseDescription: Verify that Authorize works with empty string
 */
HWTEST_F(JsAgentConnectorStubImplTest, JsAgentConnectorStubImpl_Authorize_003, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMockImpl> connection = new JSAgentConnectionTestMockImpl(env_);
    wptr<JSAgentConnection> weakConnection = connection;
    sptr<JsAgentConnectorStubImpl> stub = new JsAgentConnectorStubImpl(weakConnection);

    std::string emptyData = "";
    int32_t result = stub->Authorize(emptyData);

    EXPECT_EQ(result, 0);
}

/*
 * Feature: JsAgentConnectorStubImpl
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with long string
 * EnvConditions: Valid connection
 * CaseDescription: Verify that Authorize works with long string
 */
HWTEST_F(JsAgentConnectorStubImplTest, JsAgentConnectorStubImpl_Authorize_004, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMockImpl> connection = new JSAgentConnectionTestMockImpl(env_);
    wptr<JSAgentConnection> weakConnection = connection;
    sptr<JsAgentConnectorStubImpl> stub = new JsAgentConnectorStubImpl(weakConnection);

    std::string longData(10000, 'y');
    int32_t result = stub->Authorize(longData);

    EXPECT_EQ(result, 0);
}

/*
 * Feature: JsAgentConnectorStubImpl
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize with expired weak pointer
 * EnvConditions: Connection object destroyed
 * CaseDescription: Verify that Authorize returns error when connection is destroyed
 */
HWTEST_F(JsAgentConnectorStubImplTest, JsAgentConnectorStubImpl_Authorize_005, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMockImpl> connection = new JSAgentConnectionTestMockImpl(env_);
    wptr<JSAgentConnection> weakConnection = connection;
    sptr<JsAgentConnectorStubImpl> stub = new JsAgentConnectorStubImpl(weakConnection);

    // Destroy the connection
    connection = nullptr;

    std::string authData = "auth data";
    int32_t result = stub->Authorize(authData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityRuntime::AbilityErrorCode::ERROR_CODE_INNER));
}

/*
 * Feature: JsAgentConnectorStubImpl
 * Function: Multiple calls
 * SubFunction: NA
 * FunctionPoints: Multiple SendData and Authorize calls
 * EnvConditions: Valid connection
 * CaseDescription: Verify that multiple calls work correctly
 */
HWTEST_F(JsAgentConnectorStubImplTest, JsAgentConnectorStubImpl_MultipleCalls_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMockImpl> connection = new JSAgentConnectionTestMockImpl(env_);
    wptr<JSAgentConnection> weakConnection = connection;
    sptr<JsAgentConnectorStubImpl> stub = new JsAgentConnectorStubImpl(weakConnection);

    // Multiple SendData calls
    for (int i = 0; i < 10; i++) {
        std::string data = "test data " + std::to_string(i);
        int32_t result = stub->SendData(data);
        EXPECT_EQ(result, 0);
    }

    // Multiple Authorize calls
    for (int i = 0; i < 10; i++) {
        std::string data = "auth data " + std::to_string(i);
        int32_t result = stub->Authorize(data);
        EXPECT_EQ(result, 0);
    }
}

} // namespace AgentRuntime
} // namespace OHOS
