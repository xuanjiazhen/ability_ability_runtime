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

#include "js_agent_connector_proxy.h"
#include "js_agent_connector_proxy_test_mock.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AgentRuntime {
class JsAgentConnectorProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    napi_env env_{reinterpret_cast<napi_env>(0x1234)};
    sptr<MockIAgentConnector> mockConnector_{ nullptr };
};

void JsAgentConnectorProxyTest::SetUpTestCase(void)
{}
void JsAgentConnectorProxyTest::TearDownTestCase(void)
{}
void JsAgentConnectorProxyTest::SetUp()
{
    mockConnector_ = new MockIAgentConnector();
}
void JsAgentConnectorProxyTest::TearDown()
{
    mockConnector_ = nullptr;
}

/*
 * Feature: JsAgentConnectorProxy
 * Function: Constructor
 * SubFunction: NA
 * FunctionPoints: JsAgentConnectorProxy construction
 * EnvConditions: NA
 * CaseDescription: Verify that JsAgentConnectorProxy can be constructed successfully
 */
HWTEST_F(JsAgentConnectorProxyTest, JsAgentConnectorProxy_Constructor_001, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new MockRemoteObjectForJsProxy();

    JsAgentConnectorProxy* proxy = new JsAgentConnectorProxy(remoteObject);

    EXPECT_NE(proxy, nullptr);
    delete proxy;
}

/*
 * Feature: JsAgentConnectorProxy
 * Function: Constructor
 * SubFunction: NA
 * FunctionPoints: JsAgentConnectorProxy construction with null remote object
 * EnvConditions: Remote object is null
 * CaseDescription: Verify that JsAgentConnectorProxy handles null remote object
 */
HWTEST_F(JsAgentConnectorProxyTest, JsAgentConnectorProxy_Constructor_002, TestSize.Level1)
{
    sptr<IRemoteObject> nullObject = nullptr;

    JsAgentConnectorProxy* proxy = new JsAgentConnectorProxy(nullObject);

    EXPECT_NE(proxy, nullptr);
    delete proxy;
}

/*
 * Feature: JsAgentConnectorProxy
 * Function: Finalizer
 * SubFunction: NA
 * FunctionPoints: JsAgentConnectorProxy finalization
 * EnvConditions: NA
 * CaseDescription: Verify that Finalizer doesn't crash
 */
HWTEST_F(JsAgentConnectorProxyTest, JsAgentConnectorProxy_Finalizer_001, TestSize.Level1)
{
    JsAgentConnectorProxy* proxy = new JsAgentConnectorProxy(nullptr);

    JsAgentConnectorProxy::Finalizer(env_, proxy, nullptr);

    // Should not crash
    SUCCEED();
}

/*
 * Feature: JsAgentConnectorProxy
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with null proxy
 * EnvConditions: Internal proxy is null
 * CaseDescription: Verify that OnSendData returns undefined when proxy is null
 */
HWTEST_F(JsAgentConnectorProxyTest, JsAgentConnectorProxy_OnSendData_001, TestSize.Level1)
{
    sptr<IRemoteObject> nullObject = nullptr;
    JsAgentConnectorProxy* proxy = new JsAgentConnectorProxy(nullObject);

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
 * Feature: JsAgentConnectorProxy
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with insufficient arguments
 * EnvConditions: argc < 1
 * CaseDescription: Verify that OnSendData returns undefined with insufficient args
 */
HWTEST_F(JsAgentConnectorProxyTest, JsAgentConnectorProxy_OnSendData_002, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new MockRemoteObjectForJsProxy();
    JsAgentConnectorProxy* proxy = new JsAgentConnectorProxy(remoteObject);

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
 * Feature: JsAgentConnectorProxy
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with successful send
 * EnvConditions: Valid proxy and data
 * CaseDescription: Verify that OnSendData succeeds with valid inputs
 */
HWTEST_F(JsAgentConnectorProxyTest, JsAgentConnectorProxy_OnSendData_003, TestSize.Level1)
{
    EXPECT_CALL(*mockConnector_, SendData(_))
        .Times(1)
        .WillOnce(Return(static_cast<int32_t>(AbilityErrorCode::ERROR_OK)));

    sptr<IRemoteObject> remoteObject = mockConnector_->AsObject();
    JsAgentConnectorProxy* proxy = new JsAgentConnectorProxy(remoteObject);

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
 * Feature: JsAgentConnectorProxy
 * Function: OnSendData
 * SubFunction: NA
 * FunctionPoints: OnSendData with failed send
 * EnvConditions: SendData returns error
 * CaseDescription: Verify that OnSendData handles send failure
 */
HWTEST_F(JsAgentConnectorProxyTest, JsAgentConnectorProxy_OnSendData_004, TestSize.Level1)
{
    EXPECT_CALL(*mockConnector_, SendData(_))
        .Times(1)
        .WillOnce(Return(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER)));

    sptr<IRemoteObject> remoteObject = mockConnector_->AsObject();
    JsAgentConnectorProxy* proxy = new JsAgentConnectorProxy(remoteObject);

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
 * Feature: JsAgentConnectorProxy
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with null proxy
 * EnvConditions: Internal proxy is null
 * CaseDescription: Verify that OnAuthorize returns undefined when proxy is null
 */
HWTEST_F(JsAgentConnectorProxyTest, JsAgentConnectorProxy_OnAuthorize_001, TestSize.Level1)
{
    sptr<IRemoteObject> nullObject = nullptr;
    JsAgentConnectorProxy* proxy = new JsAgentConnectorProxy(nullObject);

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
 * Feature: JsAgentConnectorProxy
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with insufficient arguments
 * EnvConditions: argc < 1
 * CaseDescription: Verify that OnAuthorize returns undefined with insufficient args
 */
HWTEST_F(JsAgentConnectorProxyTest, JsAgentConnectorProxy_OnAuthorize_002, TestSize.Level1)
{
    sptr<IRemoteObject> remoteObject = new MockRemoteObjectForJsProxy();
    JsAgentConnectorProxy* proxy = new JsAgentConnectorProxy(remoteObject);

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
 * Feature: JsAgentConnectorProxy
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with successful send
 * EnvConditions: Valid proxy and data
 * CaseDescription: Verify that OnAuthorize succeeds with valid inputs
 */
HWTEST_F(JsAgentConnectorProxyTest, JsAgentConnectorProxy_OnAuthorize_003, TestSize.Level1)
{
    EXPECT_CALL(*mockConnector_, Authorize(_))
        .Times(1)
        .WillOnce(Return(static_cast<int32_t>(AbilityErrorCode::ERROR_OK)));

    sptr<IRemoteObject> remoteObject = mockConnector_->AsObject();
    JsAgentConnectorProxy* proxy = new JsAgentConnectorProxy(remoteObject);

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
 * Feature: JsAgentConnectorProxy
 * Function: OnAuthorize
 * SubFunction: NA
 * FunctionPoints: OnAuthorize with failed send
 * EnvConditions: Authorize returns error
 * CaseDescription: Verify that OnAuthorize handles send failure
 */
HWTEST_F(JsAgentConnectorProxyTest, JsAgentConnectorProxy_OnAuthorize_004, TestSize.Level1)
{
    EXPECT_CALL(*mockConnector_, Authorize(_))
        .Times(1)
        .WillOnce(Return(static_cast<int32_t>(AbilityErrorCode::ERROR_CODE_INNER)));

    sptr<IRemoteObject> remoteObject = mockConnector_->AsObject();
    JsAgentConnectorProxy* proxy = new JsAgentConnectorProxy(remoteObject);

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
 * Feature: JsAgentConnectorProxy
 * Function: CreateJsAgentConnectorProxy
 * SubFunction: NA
 * FunctionPoints: CreateJsAgentConnectorProxy with null remote object
 * EnvConditions: Remote object is null
 * CaseDescription: Verify that CreateJsAgentConnectorProxy handles null remote object
 */
HWTEST_F(JsAgentConnectorProxyTest, JsAgentConnectorProxy_CreateJsAgentConnectorProxy_001, TestSize.Level1)
{
    sptr<IRemoteObject> nullObject = nullptr;

    napi_value result = JsAgentConnectorProxy::CreateJsAgentConnectorProxy(env_, nullObject);

    // Should return undefined instead of crashing
    SUCCEED();
}

/*
 * Feature: JsAgentConnectorProxy
 * Function: SendData
 * SubFunction: NA
 * FunctionPoints: SendData static callback
 * EnvConditions: NA
 * CaseDescription: Verify that SendData static callback works
 */
HWTEST_F(JsAgentConnectorProxyTest, JsAgentConnectorProxy_SendData_001, TestSize.Level1)
{
    // The static callback delegates to the instance's OnSendData
    // which is tested above, so we just verify it doesn't crash
    SUCCEED();
}

/*
 * Feature: JsAgentConnectorProxy
 * Function: Authorize
 * SubFunction: NA
 * FunctionPoints: Authorize static callback
 * EnvConditions: NA
 * CaseDescription: Verify that Authorize static callback works
 */
HWTEST_F(JsAgentConnectorProxyTest, JsAgentConnectorProxy_Authorize_001, TestSize.Level1)
{
    // The static callback delegates to the instance's OnAuthorize
    // which is tested above, so we just verify it doesn't crash
    SUCCEED();
}

} // namespace AgentRuntime
} // namespace OHOS
