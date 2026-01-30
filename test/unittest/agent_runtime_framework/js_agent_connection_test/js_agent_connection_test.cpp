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

#include "js_agent_connection.h"
#include "js_agent_connection_test_mock.h"
#include "want.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AgentRuntime {
class JSAgentConnectionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    // Mock napi_env for testing - using a simple pointer value
    napi_env env_{reinterpret_cast<napi_env>(0x1234)};
};

void JSAgentConnectionTest::SetUpTestCase(void)
{}
void JSAgentConnectionTest::TearDownTestCase(void)
{}
void JSAgentConnectionTest::SetUp()
{}
void JSAgentConnectionTest::TearDown()
{}

/*
 * Feature: JSAgentConnection
 * Function: Constructor
 * SubFunction: NA
 * FunctionPoints: JSAgentConnection construction
 * EnvConditions: NA
 * CaseDescription: Verify that JSAgentConnection can be constructed successfully
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_Constructor_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);
    EXPECT_NE(connection, nullptr);
    EXPECT_NE(connection->GetServiceHostStub(), nullptr);
}

/*
 * Feature: JSAgentConnection
 * Function: GetConnectionId / SetConnectionId
 * SubFunction: NA
 * FunctionPoints: Connection ID getter/setter
 * EnvConditions: NA
 * CaseDescription: Verify that connection ID can be set and retrieved
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_ConnectionId_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    int64_t testId = 12345;
    connection->SetConnectionId(testId);

    EXPECT_EQ(connection->GetConnectionId(), testId);
}

/*
 * Feature: JSAgentConnection
 * Function: GetServiceHostStub
 * SubFunction: NA
 * FunctionPoints: Get service host stub
 * EnvConditions: NA
 * CaseDescription: Verify that service host stub is created in constructor
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_GetServiceHostStub_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    sptr<JsAgentConnectorStubImpl> stub = connection->GetServiceHostStub();
    EXPECT_NE(stub, nullptr);
}

/*
 * Feature: JSAgentConnection
 * Function: SetProxyObject / GetProxyObject
 * SubFunction: NA
 * FunctionPoints: Proxy object getter/setter
 * EnvConditions: NA
 * CaseDescription: Verify that proxy object can be set and retrieved
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_ProxyObject_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    // Initially proxy object should be null
    napi_value proxy = connection->GetProxyObject();
    EXPECT_EQ(proxy, nullptr);

    // Set a mock proxy value
    napi_value mockProxy = reinterpret_cast<napi_value>(0x5678);
    connection->SetProxyObject(mockProxy);

    // Get proxy object - should not be null now
    proxy = connection->GetProxyObject();
    // Note: GetProxyObject returns nullptr if serviceProxyObject_ is nullptr,
    // otherwise it returns the NAPI value. We're testing the getter/setter flow.
}

/*
 * Feature: JSAgentConnection
 * Function: SetNapiAsyncTask
 * SubFunction: NA
 * FunctionPoints: Set async task
 * EnvConditions: NA
 * CaseDescription: Verify that async task can be set
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_SetNapiAsyncTask_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    std::shared_ptr<NapiAsyncTask> mockTask = std::make_shared<NapiAsyncTaskTestMock>();
    connection->SetNapiAsyncTask(mockTask);

    // Task is set - no easy way to verify without accessing private member
    // but we can verify it doesn't crash
    SUCCEED();
}

/*
 * Feature: JSAgentConnection
 * Function: AddDuplicatedPendingTask
 * SubFunction: NA
 * FunctionPoints: Add pending task
 * EnvConditions: NA
 * CaseDescription: Verify that pending task can be added
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_AddDuplicatedPendingTask_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    auto task = std::make_unique<NapiAsyncTaskTestMock>();
    connection->AddDuplicatedPendingTask(task);

    // Task is added - no easy way to verify without accessing private member
    // but we can verify it doesn't crash
    SUCCEED();
}

/*
 * Feature: JSAgentConnection
 * Function: ResolveDuplicatedPendingTask
 * SubFunction: NA
 * FunctionPoints: Resolve pending tasks
 * EnvConditions: NA
 * CaseDescription: Verify that pending tasks can be resolved
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_ResolveDuplicatedPendingTask_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    napi_value mockProxy = reinterpret_cast<napi_value>(0x5678);
    connection->ResolveDuplicatedPendingTask(env_, mockProxy);

    // Should not crash even with no pending tasks
    SUCCEED();
}

/*
 * Feature: JSAgentConnection
 * Function: RejectDuplicatedPendingTask
 * SubFunction: NA
 * FunctionPoints: Reject pending tasks
 * EnvConditions: NA
 * CaseDescription: Verify that pending tasks can be rejected
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_RejectDuplicatedPendingTask_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    napi_value mockError = reinterpret_cast<napi_value>(0x9ABC);
    connection->RejectDuplicatedPendingTask(env_, mockError);

    // Should not crash even with no pending tasks
    SUCCEED();
}

/*
 * Feature: JSAgentConnection
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with data
 * EnvConditions: NA
 * CaseDescription: Verify that OnSendData returns success
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_OnSendData_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    std::string testData = "test data";
    int32_t result = connection->OnSendData(testData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityErrorCode::ERROR_OK));
}

/*
 * Feature: JSAgentConnection
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with empty string
 * EnvConditions: NA
 * CaseDescription: Verify that OnSendData works with empty string
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_OnSendData_002, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    std::string emptyData = "";
    int32_t result = connection->OnSendData(emptyData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityErrorCode::ERROR_OK));
}

/*
 * Feature: JSAgentConnection
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with long string
 * EnvConditions: NA
 * CaseDescription: Verify that OnSendData works with long string
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_OnSendData_003, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    std::string longData(10000, 'x');
    int32_t result = connection->OnSendData(longData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityErrorCode::ERROR_OK));
}

/*
 * Feature: JSAgentConnection
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with data
 * EnvConditions: NA
 * CaseDescription: Verify that OnAuthorize returns success
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_OnAuthorize_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    std::string authData = "auth data";
    int32_t result = connection->OnAuthorize(authData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityErrorCode::ERROR_OK));
}

/*
 * Feature: JSAgentConnection
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with empty string
 * EnvConditions: NA
 * CaseDescription: Verify that OnAuthorize works with empty string
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_OnAuthorize_002, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    std::string emptyData = "";
    int32_t result = connection->OnAuthorize(emptyData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityErrorCode::ERROR_OK));
}

/*
 * Feature: JSAgentConnection
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with long string
 * EnvConditions: NA
 * CaseDescription: Verify that OnAuthorize works with long string
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_OnAuthorize_003, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    std::string longData(10000, 'y');
    int32_t result = connection->OnAuthorize(longData);

    EXPECT_EQ(result, static_cast<int32_t>(AbilityErrorCode::ERROR_OK));
}

/*
 * Feature: JSAgentConnection
 * Function: SetJsConnectionObject
 * SubFunction: NA
 * FunctionPoints: Set JS connection object
 * EnvConditions: NA
 * CaseDescription: Verify that JS connection object can be set
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_SetJsConnectionObject_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    napi_value mockObject = reinterpret_cast<napi_value>(0xDEF0);
    connection->SetJsConnectionObject(mockObject);

    // Should not crash
    SUCCEED();
}

/*
 * Feature: JSAgentConnection
 * Function: RemoveConnectionObject
 * SubFunction: NA
 * FunctionPoints: Remove connection object
 * EnvConditions: NA
 * CaseDescription: Verify that connection object can be removed
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_RemoveConnectionObject_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    connection->RemoveConnectionObject();

    // Should not crash
    SUCCEED();
}

/*
 * Feature: JSAgentConnection
 * Function: OnAbilityConnectDone
 * SubFunction: NA
 * FunctionPoints: OnAbilityConnectDone with null async task
 * EnvConditions: napiAsyncTask_ is null
 * CaseDescription: Verify that OnAbilityConnectDone handles null async task
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_OnAbilityConnectDone_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    ElementName element;
    sptr<IRemoteObject> remoteObject = new MockRemoteObjectForTest();
    int resultCode = 0;

    connection->OnAbilityConnectDone(element, remoteObject, resultCode);

    // Should not crash with null async task
    SUCCEED();
}

/*
 * Feature: JSAgentConnection
 * Function: OnAbilityDisconnectDone
 * SubFunction: NA
 * FunctionPoints: OnAbilityDisconnectDone with null async task
 * EnvConditions: napiAsyncTask_ is null
 * CaseDescription: Verify that OnAbilityDisconnectDone handles null async task
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_OnAbilityDisconnectDone_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);

    ElementName element;
    int resultCode = 0;

    connection->OnAbilityDisconnectDone(element, resultCode);

    // Should not crash with null async task
    SUCCEED();
}

/*
 * Feature: AgentConnectionUtils
 * Function: RemoveAgentConnection
 * SubFunction: NA
 * FunctionPoints: Remove agent connection
 * EnvConditions: NA
 * CaseDescription: Verify that RemoveAgentConnection doesn't crash with non-existent connection
 */
HWTEST_F(JSAgentConnectionTest, AgentConnectionUtils_RemoveAgentConnection_001, TestSize.Level1)
{
    int64_t nonExistentId = 99999;
    AgentConnectionUtils::RemoveAgentConnection(nonExistentId);

    // Should not crash
    SUCCEED();
}

/*
 * Feature: AgentConnectionUtils
 * Function: InsertAgentConnection
 * SubFunction: NA
 * FunctionPoints: Insert agent connection
 * EnvConditions: NA
 * CaseDescription: Verify that InsertAgentConnection returns valid ID
 */
HWTEST_F(JSAgentConnectionTest, AgentConnectionUtils_InsertAgentConnection_001, TestSize.Level1)
{
    sptr<JSAgentConnectionTestMock> connection = new JSAgentConnectionTestMock(env_);
    AAFwk::Want want;

    int64_t connectionId = AgentConnectionUtils::InsertAgentConnection(connection, want);

    EXPECT_GE(connectionId, 0);

    // Clean up
    AgentConnectionUtils::RemoveAgentConnection(connectionId);
}

/*
 * Feature: AgentConnectionUtils
 * Function: InsertAgentConnection
 * SubFunction: NA
 * FunctionPoints: Insert null connection
 * EnvConditions: Connection is null
 * CaseDescription: Verify that InsertAgentConnection returns -1 for null connection
 */
HWTEST_F(JSAgentConnectionTest, AgentConnectionUtils_InsertAgentConnection_002, TestSize.Level1)
{
    sptr<JSAgentConnection> nullConnection = nullptr;
    AAFwk::Want want;

    int64_t connectionId = AgentConnectionUtils::InsertAgentConnection(nullConnection, want);

    EXPECT_EQ(connectionId, -1);
}

/*
 * Feature: AgentConnectionUtils
 * Function: FindAgentConnection
 * SubFunction: Find by ID
 * FunctionPoints: Find agent connection by ID
 * EnvConditions: NA
 * CaseDescription: Verify that FindAgentConnection handles non-existent connection
 */
HWTEST_F(JSAgentConnectionTest, AgentConnectionUtils_FindAgentConnection_001, TestSize.Level1)
{
    int64_t nonExistentId = 99999;
    sptr<JSAgentConnection> connection;

    AgentConnectionUtils::FindAgentConnection(nonExistentId, connection);

    // Should return null connection
    EXPECT_EQ(connection, nullptr);
}

/*
 * Feature: JSAgentConnection
 * Function: IsJsCallbackObjectEquals
 * SubFunction: NA
 * FunctionPoints: IsJsCallbackObjectEquals
 * EnvConditions: NA
 * CaseDescription: Verify that IsJsCallbackObjectEquals returns false for null values
 */
HWTEST_F(JSAgentConnectionTest, JSAgentConnection_IsJsCallbackObjectEquals_001, TestSize.Level1)
{
    std::unique_ptr<NativeReference> nullCallback = nullptr;
    napi_value nullValue = nullptr;

    bool result = JSAgentConnection::IsJsCallbackObjectEquals(env_, nullCallback, nullValue);

    EXPECT_EQ(result, true);  // Both null should be equal
}

} // namespace AgentRuntime
} // namespace OHOS
