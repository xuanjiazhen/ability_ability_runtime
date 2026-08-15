/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <gtest/hwext/gtest-multithread.h>
#define protected public
#define private public
#include "js_ui_service_host_proxy.h"
#undef private
#undef protected
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "js_runtime_lite.h"

using namespace testing::ext;
using namespace OHOS::AAFwk;

namespace OHOS {
namespace AbilityRuntime {
class JsUIServiceHostStubTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

protected:
    NativeEngine* engine_;
};

void JsUIServiceHostStubTest::SetUpTestCase(void)
{}

void JsUIServiceHostStubTest::TearDownTestCase(void)
{}

void JsUIServiceHostStubTest::SetUp()
{
}

void JsUIServiceHostStubTest::TearDown()
{}

/**
 * @tc.number: CheckCallerIsSystemApp_0100
 * @tc.name: CheckCallerIsSystemApp
 * @tc.desc: JsUIServiceHostProxy CheckCallerIsSystemApp
 */
HWTEST_F(JsUIServiceHostStubTest, CheckCallerIsSystemApp_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CheckCallerIsSystemApp_0100 start");
    sptr<IRemoteObject> impl;
    JsUIServiceHostProxy proxy (impl);
    auto res = proxy.CheckCallerIsSystemApp();
    EXPECT_EQ(res, false);
    TAG_LOGI(AAFwkTag::TEST, "CheckCallerIsSystemApp_0100 end");
}

/**
 * @tc.number: CreateJsUIServiceHostProxy_0100
 * @tc.name: CreateJsUIServiceHostProxy
 * @tc.desc: JsUIServiceHostProxy CreateJsUIServiceHostProxy
 */
HWTEST_F(JsUIServiceHostStubTest, CreateJsUIServiceHostProxy_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateJsUIServiceHostProxy_0100 start");
    sptr<IRemoteObject> impl;
    JsUIServiceHostProxy proxy (impl);
    napi_env env = {};

    auto res = proxy.CreateJsUIServiceHostProxy(env, impl);
    EXPECT_EQ(res, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "CreateJsUIServiceHostProxy_0100 end");
}

/**
 * @tc.number: CreateJsUIServiceHostProxy_0200
 * @tc.name: CreateJsUIServiceHostProxy
 * @tc.desc: JsUIServiceHostProxy CreateJsUIServiceHostProxy
 */
HWTEST_F(JsUIServiceHostStubTest, CreateJsUIServiceHostProxy_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateJsUIServiceHostProxy_0200 start");
    sptr<AppExecFwk::MockAbilityToken> impl = new (std::nothrow) AppExecFwk::MockAbilityToken();
    JsUIServiceHostProxy proxy (impl);
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, napi_status::napi_ok);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());

    auto res = proxy.CreateJsUIServiceHostProxy(env, impl);
    EXPECT_NE(res, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "CreateJsUIServiceHostProxy_0200 end");
}

/**
 * @tc.number: SendData_0100
 * @tc.name: SendData
 * @tc.desc: JsUIServiceHostProxy SendData
 */
HWTEST_F(JsUIServiceHostStubTest, SendData_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendData_0100 start");
    sptr<AppExecFwk::MockAbilityToken> impl = new (std::nothrow) AppExecFwk::MockAbilityToken();
    auto proxy = std::make_shared<JsUIServiceHostProxy>(impl);
    napi_env env = {};
    napi_callback_info info = {};

    proxy->SendData(env, info);
    EXPECT_NE(proxy, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "SendData_0100 end");
}

/**
 * @tc.number: Finalizer_0100
 * @tc.name: Finalizer
 * @tc.desc: JsUIServiceHostProxy Finalizer
 */
HWTEST_F(JsUIServiceHostStubTest, Finalizer_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Finalizer_0100 start");
    sptr<AppExecFwk::MockAbilityToken> impl = new (std::nothrow) AppExecFwk::MockAbilityToken();
    auto proxy = std::make_shared<JsUIServiceHostProxy>(impl);
    napi_env env = {};
    napi_callback_info info = {};
    void* data = nullptr;
    void* hint = nullptr;
    proxy->Finalizer(env, data, hint);
    EXPECT_NE(proxy, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "Finalizer_0100 end");
}

/**
 * @tc.number: OnSendData_0100
 * @tc.name: Finalizer
 * @tc.desc: JsUIServiceHostProxy Finalizer
 */
HWTEST_F(JsUIServiceHostStubTest, OnSendData_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnSendData_0100 start");
    sptr<AppExecFwk::MockAbilityToken> impl = new (std::nothrow) AppExecFwk::MockAbilityToken();
    auto proxy = std::make_shared<JsUIServiceHostProxy>(impl);
    napi_env env = {};
    NapiCallbackInfo info;
    proxy->OnSendData(env, info);
    EXPECT_EQ(proxy->proxy_, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnSendData_0100 end");
}

/**
 * @tc.number: OnSendData_0200
 * @tc.name: OnSendData_0200
 * @tc.desc: OnSendData throws NOT_SYSTEM_APP when caller is not system app with valid env.
 */
HWTEST_F(JsUIServiceHostStubTest, OnSendData_0200, TestSize.Level1)
{
    sptr<AppExecFwk::MockAbilityToken> impl = new (std::nothrow) AppExecFwk::MockAbilityToken();
    auto proxy = std::make_shared<JsUIServiceHostProxy>(impl);
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    NapiCallbackInfo info{1};
    proxy->OnSendData(env, info);
    bool hasException = false;
    napi_is_exception_pending(env, &hasException);
    EXPECT_TRUE(hasException);
}

/**
 * @tc.number: Constructor_0100
 * @tc.name: Constructor_0100
 * @tc.desc: Constructor with null impl results in null proxy_.
 */
HWTEST_F(JsUIServiceHostStubTest, Constructor_0100, TestSize.Level1)
{
    sptr<IRemoteObject> impl = nullptr;
    JsUIServiceHostProxy proxy(impl);
    EXPECT_EQ(proxy.proxy_, nullptr);
}

/**
 * @tc.number: Finalizer_0200
 * @tc.name: Finalizer_0200
 * @tc.desc: Finalizer destructs a real JsUIServiceHostProxy object without crash.
 */
HWTEST_F(JsUIServiceHostStubTest, Finalizer_0200, TestSize.Level1)
{
    sptr<AppExecFwk::MockAbilityToken> impl = new (std::nothrow) AppExecFwk::MockAbilityToken();
    auto* raw = new JsUIServiceHostProxy(impl);
    JsUIServiceHostProxy::Finalizer(nullptr, raw, nullptr);
    SUCCEED();
}
} // namespace AbilityRuntime
} // namespace OHOS
