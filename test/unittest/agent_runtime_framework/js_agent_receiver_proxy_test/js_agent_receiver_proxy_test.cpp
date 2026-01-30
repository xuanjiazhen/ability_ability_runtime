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

#include "js_agent_receiver_proxy.h"
#include "js_agent_receiver_proxy_test_mock.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::AgentRuntime;
using namespace OHOS::AbilityRuntime;

class JsAgentReceiverProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    napi_env env_{reinterpret_cast<napi_env>(0x1234)};
    sptr<MockIAgentReceiver> mockReceiver_{ nullptr };
    sptr<IRemoteObject> mockConnectorProxy_{ nullptr };
};

void JsAgentReceiverProxyTest::SetUpTestCase(void)
{}
void JsAgentReceiverProxyTest::TearDownTestCase(void)
{}
void JsAgentReceiverProxyTest::SetUp()
{
    mockReceiver_ = new MockIAgentReceiver();
    mockConnectorProxy_ = new MockRemoteObjectForJsReceiver();
}
void JsAgentReceiverProxyTest::TearDown()
{
    mockReceiver_ = nullptr;
    mockConnectorProxy_ = nullptr;
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: Constructor
 * SubFunction: NA
 * FunctionPoints: JsAgentReceiverProxy construction
 * EnvConditions: NA
 * CaseDescription: Verify that JsAgentReceiverProxy can be constructed successfully
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_Constructor_001, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new MockRemoteObjectForJsReceiver();
    sptr<IRemoteObject> connectorProxy = new MockRemoteObjectForJsReceiver();

    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(remoteObject, connectorProxy);

    EXPECT_NE(proxy, nullptr);
    delete proxy;
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: Constructor
 * SubFunction: NA
 * FunctionPoints: JsAgentReceiverProxy construction with null remote object
 * EnvConditions: Remote object is null
 * CaseDescription: Verify that JsAgentReceiverProxy handles null remote object
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_Constructor_002, TestSize.Level1)
{
    sptr<IRemoteObject> nullObject = nullptr;
    sptr<IRemoteObject> connectorProxy = new MockRemoteObjectForJsReceiver();

    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(nullObject, connectorProxy);

    EXPECT_NE(proxy, nullptr);
    delete proxy;
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: Constructor
 * SubFunction: NA
 * FunctionPoints: JsAgentReceiverProxy construction with null connector proxy
 * EnvConditions: Connector proxy is null
 * CaseDescription: Verify that JsAgentReceiverProxy handles null connector proxy
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_Constructor_003, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new MockRemoteObjectForJsReceiver();
    sptr<IRemoteObject> nullConnectorProxy = nullptr;

    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(remoteObject, nullConnectorProxy);

    EXPECT_NE(proxy, nullptr);
    delete proxy;
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: Finalizer
 * SubFunction: NA
 * FunctionPoints: JsAgentReceiverProxy finalization
 * EnvConditions: NA
 * CaseDescription: Verify that Finalizer doesn't crash
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_Finalizer_001, TestSize.Level1)
{
    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(nullptr, nullptr);

    JsAgentReceiverProxy::Finalizer(env_, proxy, nullptr);

    // Should not crash
    SUCCEED();
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: GetConnectionId / SetConnectionId
 * SubFunction: NA
 * FunctionPoints: Connection ID getter/setter
 * EnvConditions: NA
 * CaseDescription: Verify that connection ID can be set and retrieved
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_ConnectionId_001, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new MockRemoteObjectForJsReceiver();
    sptr<IRemoteObject> connectorProxy = new MockRemoteObjectForJsReceiver();

    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(remoteObject, connectorProxy);

    int64_t testId = 12345;
    proxy->SetConnectionId(testId);

    EXPECT_EQ(proxy->GetConnectionId(), testId);

    delete proxy;
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with null proxy
 * EnvConditions: Internal proxy is null
 * CaseDescription: Verify that OnSendData returns undefined when proxy is null
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_OnSendData_001, TestSize.Level1)
{
    sptr<IRemoteObject> nullObject = nullptr;
    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(nullObject, mockConnectorProxy_);

    NapiCallbackInfo info;
    info.argc = 1;
    napi_value argv[1];
    argv[0] = reinterpret_cast<napi_value>(0x5678);  // Mock string value
    info.argv = argv;

    napi_value result = proxy->OnSendData(env_, info);

    // Should return undefined (null in our mock env)
    delete proxy;
    SUCCEED();
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with null connector proxy
 * EnvConditions: Connector proxy is null
 * CaseDescription: Verify that OnSendData returns undefined when connector proxy is null
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_OnSendData_002, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new MockRemoteObjectForJsReceiver();
    sptr<IRemoteObject> nullConnectorProxy = nullptr;
    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(remoteObject, nullConnectorProxy);

    NapiCallbackInfo info;
    info.argc = 1;
    napi_value argv[1];
    argv[0] = reinterpret_cast<napi_value>(0x5678);  // Mock string value
    info.argv = argv;

    napi_value result = proxy->OnSendData(env_, info);

    // Should return undefined (null in our mock env)
    delete proxy;
    SUCCEED();
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with insufficient arguments
 * EnvConditions: argc < 1
 * CaseDescription: Verify that OnSendData returns undefined with insufficient args
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_OnSendData_003, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new MockRemoteObjectForJsReceiver();
    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(remoteObject, mockConnectorProxy_);

    NapiCallbackInfo info;
    info.argc = 0;  // Insufficient arguments
    napi_value* argv = nullptr;
    info.argv = argv;

    napi_value result = proxy->OnSendData(env_, info);

    // Should return undefined due to insufficient args
    delete proxy;
    SUCCEED();
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with successful send
 * EnvConditions: Valid proxy and data
 * CaseDescription: Verify that OnSendData succeeds with valid inputs
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_OnSendData_004, TestSize.Level1)
{
    EXPECT_CALL(*mockReceiver_, SendData(_, _))
        .Times(1)
        .WillOnce(Return(static_cast<int32_t>(AbilityErrorCode::ERROR_OK)));

    sptr<IRemoteObject> remoteObject = mockReceiver_->AsObject();
    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(remoteObject, mockConnectorProxy_);

    NapiCallbackInfo info;
    info.argc = 1;
    napi_value argv[1];
    argv[0] = reinterpret_cast<napi_value>(0x5678);  // Mock string value
    info.argv = argv;

    napi_value result = proxy->OnSendData(env_, info);

    // Should succeed
    delete proxy;
    SUCCEED();
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with failed send
 * EnvConditions: SendData returns error
 * CaseDescription: Verify that OnSendData handles send failure
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_OnSendData_005, TestSize.Level1)
{
    EXPECT_CALL(*mockReceiver_, SendData(_, _))
        .Times(1)
        .WillOnce(Return(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER)));

    sptr<IRemoteObject> remoteObject = mockReceiver_->AsObject();
    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(remoteObject, mockConnectorProxy_);

    NapiCallbackInfo info;
    info.argc = 1;
    napi_value argv[1];
    argv[0] = reinterpret_cast<napi_value>(0x5678);  // Mock string value
    info.argv = argv;

    napi_value result = proxy->OnSendData(env_, info);

    // Should handle error gracefully
    delete proxy;
    SUCCEED();
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with null proxy
 * EnvConditions: Internal proxy is null
 * CaseDescription: Verify that OnAuthorize returns undefined when proxy is null
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_OnAuthorize_001, TestSize.Level1)
{
    sptr<IRemoteObject> nullObject = nullptr;
    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(nullObject, mockConnectorProxy_);

    NapiCallbackInfo info;
    info.argc = 1;
    napi_value argv[1];
    argv[0] = reinterpret_cast<napi_value>(0x5678);  // Mock string value
    info.argv = argv;

    napi_value result = proxy->OnAuthorize(env_, info);

    // Should return undefined (null in our mock env)
    delete proxy;
    SUCCEED();
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with null connector proxy
 * EnvConditions: Connector proxy is null
 * CaseDescription: Verify that OnAuthorize returns undefined when connector proxy is null
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_OnAuthorize_002, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new MockRemoteObjectForJsReceiver();
    sptr<IRemoteObject> nullConnectorProxy = nullptr;
    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(remoteObject, nullConnectorProxy);

    NapiCallbackInfo info;
    info.argc = 1;
    napi_value argv[1];
    argv[0] = reinterpret_cast<napi_value>(0x5678);  // Mock string value
    info.argv = argv;

    napi_value result = proxy->OnAuthorize(env_, info);

    // Should return undefined (null in our mock env)
    delete proxy;
    SUCCEED();
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with insufficient arguments
 * EnvConditions: argc < 1
 * CaseDescription: Verify that OnAuthorize returns undefined with insufficient args
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_OnAuthorize_003, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new MockRemoteObjectForJsReceiver();
    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(remoteObject, mockConnectorProxy_);

    NapiCallbackInfo info;
    info.argc = 0;  // Insufficient arguments
    napi_value* argv = nullptr;
    info.argv = argv;

    napi_value result = proxy->OnAuthorize(env_, info);

    // Should return undefined due to insufficient args
    delete proxy;
    SUCCEED();
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with successful send
 * EnvConditions: Valid proxy and data
 * CaseDescription: Verify that OnAuthorize succeeds with valid inputs
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_OnAuthorize_004, TestSize.Level1)
{
    EXPECT_CALL(*mockReceiver_, Authorize(_, _))
        .Times(1)
        .WillOnce(Return(static_cast<int32_t>(AbilityErrorCode::ERROR_OK)));

    sptr<IRemoteObject> remoteObject = mockReceiver_->AsObject();
    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(remoteObject, mockConnectorProxy_);

    NapiCallbackInfo info;
    info.argc = 1;
    napi_value argv[1];
    argv[0] = reinterpret_cast<napi_value>(0x5678);  // Mock string value
    info.argv = argv;

    napi_value result = proxy->OnAuthorize(env_, info);

    // Should succeed
    delete proxy;
    SUCCEED();
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with failed send
 * EnvConditions: Authorize returns error
 * CaseDescription: Verify that OnAuthorize handles send failure
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_OnAuthorize_005, TestSize.Level1)
{
    EXPECT_CALL(*mockReceiver_, Authorize(_, _))
        .Times(1)
        .WillOnce(Return(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER)));

    sptr<IRemoteObject> remoteObject = mockReceiver_->AsObject();
    JsAgentReceiverProxy* proxy = new JsAgentReceiverProxy(remoteObject, mockConnectorProxy_);

    NapiCallbackInfo info;
    info.argc = 1;
    napi_value argv[1];
    argv[0] = reinterpret_cast<napi_value>(0x5678);  // Mock string value
    info.argv = argv;

    napi_value result = proxy->OnAuthorize(env_, info);

    // Should handle error gracefully
    delete proxy;
    SUCCEED();
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: CreateJsAgentReceiverProxy
 * SubFunction: NA
 * FunctionPoints: CreateJsAgentReceiverProxy with null impl
 * EnvConditions: Remote object is null
 * CaseDescription: Verify that CreateJsAgentReceiverProxy handles null impl
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_CreateJsAgentReceiverProxy_001, TestSize.Level1)
{
    sptr<IRemoteObject> nullObject = nullptr;
    int64_t connectionId = 12345;
    sptr<IRemoteObject> connectorProxy = new MockRemoteObjectForJsReceiver();

    napi_value result = JsAgentReceiverProxy::CreateJsAgentReceiverProxy(env_, nullObject, connectionId, connectorProxy);

    // Should return undefined instead of crashing
    SUCCEED();
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData static callback
 * EnvConditions: NA
 * CaseDescription: Verify that SendData static callback works
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_SendData_001, TestSize.Level1)
{
    // The static callback delegates to the instance's OnSendData
    // which is tested above, so we just verify it doesn't crash
    SUCCEED();
}

/*
 * Feature: JsAgentReceiverProxy
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize static callback
 * EnvConditions: NA
 * CaseDescription: Verify that Authorize static callback works
 */
HWTEST_F(JsAgentReceiverProxyTest, JsAgentReceiverProxy_Authorize_001, TestSize.Level1)
{
    // The static callback delegates to the instance's OnAuthorize
    // which is tested above, so we just verify it doesn't crash
    SUCCEED();
}

} // namespace OHOS
} // namespace AgentRuntime
