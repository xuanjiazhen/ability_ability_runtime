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
#include <gmock/gmock.h>

#define private public
#define protected public
#include "js_ui_service_proxy.h"
#include "js_ui_service_extension.h"
#include "ui_service_extension_module_loader.h"
#include "js_ui_service_extension_context.cpp"
#undef private
#undef protected

#include "js_runtime_lite.h"
#include "mock_ability_token.h"
#include "ability_handler.h"
#include "ohos_application.h"
#include "runtime.h"
#include "ui_service_extension_connection_constants.h"
#include "ui_service_host_stub.h"
#include "napi_common_want.h"


using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {

class MockWindow : public Rosen::Window {
public:
    virtual Ace::UIContent* GetUIContent() const
    {
        return uiContent_.get();
    }

    std::unique_ptr<Ace::UIContent> uiContent_ = Ace::UIContent::Create(nullptr, nullptr);
};

class NativeReferenceMock : public NativeReference {
public:
    NativeReferenceMock() = default;
    virtual ~NativeReferenceMock() = default;
    MOCK_METHOD0(Ref, uint32_t());
    MOCK_METHOD0(Unref, uint32_t());
    MOCK_METHOD0(Get, napi_value());
    MOCK_METHOD0(GetData, void*());
    virtual operator napi_value() override
    {
        return reinterpret_cast<napi_value>(this);
    }
    MOCK_METHOD0(SetDeleteSelf, void());
    MOCK_METHOD0(GetRefCount, uint32_t());
    MOCK_METHOD0(GetFinalRun, bool());
    napi_value GetNapiValue() override
    {
        napi_env env{nullptr};
        napi_value object = AppExecFwk::CreateJSObject(env);
        return object;
    }
};

class JsUIServiceProxyTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    napi_env env_ = nullptr;
    std::unique_ptr<Runtime> runtime;
    std::shared_ptr<JsUIServiceExtension> jsUIServiceExtension;
};

void JsUIServiceProxyTest::SetUpTestCase()
{}

void JsUIServiceProxyTest::TearDownTestCase()
{}

void JsUIServiceProxyTest::SetUp()
{}

void JsUIServiceProxyTest::TearDown()
{}

class IRemoteObjectMocker : public IRemoteObject {
public:
    IRemoteObjectMocker() : IRemoteObject {u"IRemoteObjectMocker"}
    {
    }

    ~IRemoteObjectMocker()
    {
    }

    int32_t GetObjectRefCount()
    {
        return 0;
    }

    int SendRequest(uint32_t code, MessageParcel& data, MessageParcel& reply, MessageOption& option)
    {
        return 0;
    }

    bool IsProxyObject() const
    {
        return true;
    }

    bool CheckObjectLegality() const
    {
        return true;
    }

    bool AddDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    bool RemoveDeathRecipient(const sptr<DeathRecipient>& recipient)
    {
        return true;
    }

    bool AddRefreshRecipient(const sptr<RefreshRecipient> &recipient)
    {
        return true;
    }

    bool RemoveRefreshRecipient(const sptr<RefreshRecipient> &recipient)
    {
        return true;
    }

    sptr<IRemoteBroker> AsInterface()
    {
        return nullptr;
    }

    int Dump(int fd, const std::vector<std::u16string>& args)
    {
        return 0;
    }
};

class MockIUIService : public OHOS::AAFwk::IUIService {
public:
    MOCK_METHOD2(SendData, int32_t(sptr<IRemoteObject>, OHOS::AAFwk::WantParams&));
    sptr<IRemoteObject> AsObject() override
    {
        return nullptr;
    }
};

/**
 * @tc.number: CreateJsUIServiceProxy_0100
 * @tc.name: CreateJsUIServiceProxy
 * @tc.desc: SystemAbilityStatusChangeListener CreateJsUIServiceProxy
 */
HWTEST_F(JsUIServiceProxyTest, CreateJsUIServiceProxy_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateJsUIServiceProxy_0100 start");
    sptr<IRemoteObject> impl = nullptr;
    sptr<IRemoteObject> hostProxy = nullptr;
    auto infos = std::make_shared<AAFwk::JsUIServiceProxy>(impl, hostProxy);
    int64_t connectionId = 1;
    napi_value result = infos->CreateJsUIServiceProxy(env_, impl, connectionId, hostProxy);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "CreateJsUIServiceProxy_0100 end");
}

/**
 * @tc.number: CreateJsUIServiceProxy_0200
 * @tc.name: CreateJsUIServiceProxy
 * @tc.desc: SystemAbilityStatusChangeListener CreateJsUIServiceProxy
 */
HWTEST_F(JsUIServiceProxyTest, CreateJsUIServiceProxy_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "CreateJsUIServiceProxy_0200 start");
    sptr<IRemoteObject> impl = new IRemoteObjectMocker();
    sptr<IRemoteObject> hostProxy = new IRemoteObjectMocker();
    auto infos = std::make_shared<AAFwk::JsUIServiceProxy>(impl, hostProxy);
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, napi_status::napi_ok);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    int64_t connectionId = 1;
    napi_value result = infos->CreateJsUIServiceProxy(env, impl, connectionId, hostProxy);
    EXPECT_NE(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "CreateJsUIServiceProxy_0200 end");
}

/**
 * @tc.number: Finalizer_0100
 * @tc.name: Finalizer_0100
 * @tc.desc: SystemAbilityStatusChangeListener Finalizer_0100
 */
HWTEST_F(JsUIServiceProxyTest, Finalizer_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "Finalizer_0100 start");
    sptr<IRemoteObject> impl = nullptr;
    sptr<IRemoteObject> hostProxy = nullptr;
    auto infos = std::make_shared<AAFwk::JsUIServiceProxy>(impl, hostProxy);
    infos->Finalizer(env_, nullptr, nullptr);
    EXPECT_NE(infos, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "Finalizer_0100 end");
}

/**
 * @tc.number: SendData_0100
 * @tc.name: SendData_0100
 * @tc.desc: SystemAbilityStatusChangeListener SendData_0100
 */
HWTEST_F(JsUIServiceProxyTest, SendData_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "SendData_0100 start");
    sptr<IRemoteObject> impl = nullptr;
    sptr<IRemoteObject> hostProxy = nullptr;
    auto infos = std::make_shared<AAFwk::JsUIServiceProxy>(impl, hostProxy);
    napi_callback_info info{nullptr};
    napi_value result = infos->SendData(env_, info);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "SendData_0100 end");
}

/**
 * @tc.number: OnSendData_0100
 * @tc.name: OnSendData_0100
 * @tc.desc: SystemAbilityStatusChangeListener OnSendData_0100
 */
HWTEST_F(JsUIServiceProxyTest, OnSendData_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnSendData_0100 start");
    sptr<IRemoteObject> impl = new IRemoteObjectMocker();;
    sptr<IRemoteObject> hostProxy = new IRemoteObjectMocker();;
    auto infos = std::make_shared<AAFwk::JsUIServiceProxy>(impl, hostProxy);
    infos->hostProxy_ = nullptr;
    infos->proxy_ = nullptr;
    NapiCallbackInfo info{0};
    napi_value result = infos->OnSendData(env_, info);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnSendData_0100 end");
}

/**
 * @tc.number: OnSendData_0200
 * @tc.name: OnSendData_0200
 * @tc.desc: SystemAbilityStatusChangeListener OnSendData_0200
 */

HWTEST_F(JsUIServiceProxyTest, OnSendData_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnSendData_0200 start");
    sptr<IRemoteObject> impl = new IRemoteObjectMocker();;
    sptr<IRemoteObject> hostProxy = new IRemoteObjectMocker();;
    auto infos = std::make_shared<AAFwk::JsUIServiceProxy>(impl, hostProxy);
    NapiCallbackInfo info{0};
    info.argv[0] = AppExecFwk::CreateJSObject(env_);
    napi_value result = infos->OnSendData(env_, info);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnSendData_0200 end");
}

/**
 * @tc.number: OnSendData_0300
 * @tc.name: OnSendData_0300
 * @tc.desc: SystemAbilityStatusChangeListener OnSendData_0300
 */

HWTEST_F(JsUIServiceProxyTest, OnSendData_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnSendData_0300 start");
    sptr<IRemoteObject> impl = new IRemoteObjectMocker();;
    sptr<IRemoteObject> hostProxy = new IRemoteObjectMocker();;
    auto infos = std::make_shared<AAFwk::JsUIServiceProxy>(impl, hostProxy);
    NapiCallbackInfo info{1};
    napi_value result = infos->OnSendData(env_, info);
    EXPECT_EQ(result, nullptr);
    TAG_LOGI(AAFwkTag::TEST, "OnSendData_0300 end");
}

/**
 * @tc.number: GetConnectionId_0100
 * @tc.name: GetConnectionId
 * @tc.desc: Verify SetConnectionId and GetConnectionId round-trip.
 */
HWTEST_F(JsUIServiceProxyTest, GetConnectionId_0100, TestSize.Level1)
{
    sptr<IRemoteObject> impl = new IRemoteObjectMocker();
    sptr<IRemoteObject> hostProxy = new IRemoteObjectMocker();
    auto infos = std::make_shared<AAFwk::JsUIServiceProxy>(impl, hostProxy);
    infos->SetConnectionId(42);
    EXPECT_EQ(infos->GetConnectionId(), 42);
}

/**
 * @tc.number: OnSendData_0400
 * @tc.name: OnSendData_0400
 * @tc.desc: OnSendData throws when proxy_ and hostProxy_ are null with valid env.
 */
HWTEST_F(JsUIServiceProxyTest, OnSendData_0400, TestSize.Level1)
{
    auto infos = std::make_shared<AAFwk::JsUIServiceProxy>(nullptr, nullptr);
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    NapiCallbackInfo info{1};
    infos->OnSendData(env, info);
    bool hasException = false;
    napi_is_exception_pending(env, &hasException);
    EXPECT_TRUE(hasException);
}

/**
 * @tc.number: OnSendData_0500
 * @tc.name: OnSendData_0500
 * @tc.desc: OnSendData throws too-few-parameters when argc < 1 with valid proxy.
 */
HWTEST_F(JsUIServiceProxyTest, OnSendData_0500, TestSize.Level1)
{
    sptr<IRemoteObject> impl = new IRemoteObjectMocker();
    sptr<IRemoteObject> hostProxy = new IRemoteObjectMocker();
    auto infos = std::make_shared<AAFwk::JsUIServiceProxy>(impl, hostProxy);
    infos->proxy_ = sptr<IUIService>(static_cast<IUIService*>(new MockIUIService()));
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    NapiCallbackInfo info{0};
    infos->OnSendData(env, info);
    bool hasException = false;
    napi_is_exception_pending(env, &hasException);
    EXPECT_TRUE(hasException);
}

/**
 * @tc.number: OnSendData_0600
 * @tc.name: OnSendData_0600
 * @tc.desc: OnSendData throws when UnwrapWantParams fails (argv not WantParams).
 */
HWTEST_F(JsUIServiceProxyTest, OnSendData_0600, TestSize.Level1)
{
    sptr<IRemoteObject> impl = new IRemoteObjectMocker();
    sptr<IRemoteObject> hostProxy = new IRemoteObjectMocker();
    auto infos = std::make_shared<AAFwk::JsUIServiceProxy>(impl, hostProxy);
    infos->proxy_ = sptr<IUIService>(static_cast<IUIService*>(new MockIUIService()));
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    NapiCallbackInfo info{1};
    napi_value undefined = nullptr;
    napi_get_undefined(env, &undefined);
    info.argv[0] = undefined;
    infos->OnSendData(env, info);
    bool hasException = false;
    napi_is_exception_pending(env, &hasException);
    EXPECT_TRUE(hasException);
}

/**
 * @tc.number: OnSendData_0700
 * @tc.name: OnSendData_0700
 * @tc.desc: OnSendData throws when proxy_->SendData returns failure.
 */
HWTEST_F(JsUIServiceProxyTest, OnSendData_0700, TestSize.Level1)
{
    sptr<IRemoteObject> impl = new IRemoteObjectMocker();
    sptr<IRemoteObject> hostProxy = new IRemoteObjectMocker();
    auto infos = std::make_shared<AAFwk::JsUIServiceProxy>(impl, hostProxy);
    auto* mockService = new MockIUIService();
    infos->proxy_ = sptr<IUIService>(static_cast<IUIService*>(mockService));
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    AAFwk::WantParams params;
    NapiCallbackInfo info{1};
    info.argv[0] = AppExecFwk::WrapWantParams(env, params);
    EXPECT_CALL(*mockService, SendData).WillOnce(testing::Return(-1));
    infos->OnSendData(env, info);
    bool hasException = false;
    napi_is_exception_pending(env, &hasException);
    EXPECT_TRUE(hasException);
}

/**
 * @tc.number: OnSendData_0800
 * @tc.name: OnSendData_0800
 * @tc.desc: OnSendData succeeds (no exception) when proxy_->SendData returns OK.
 */
HWTEST_F(JsUIServiceProxyTest, OnSendData_0800, TestSize.Level1)
{
    sptr<IRemoteObject> impl = new IRemoteObjectMocker();
    sptr<IRemoteObject> hostProxy = new IRemoteObjectMocker();
    auto infos = std::make_shared<AAFwk::JsUIServiceProxy>(impl, hostProxy);
    auto* mockService = new MockIUIService();
    infos->proxy_ = sptr<IUIService>(static_cast<IUIService*>(mockService));
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    AAFwk::WantParams params;
    NapiCallbackInfo info{1};
    info.argv[0] = AppExecFwk::WrapWantParams(env, params);
    EXPECT_CALL(*mockService, SendData).WillOnce(testing::Return(0));
    infos->OnSendData(env, info);
    bool hasException = false;
    napi_is_exception_pending(env, &hasException);
    EXPECT_FALSE(hasException);
}

/**
 * @tc.number: Finalizer_0200
 * @tc.name: Finalizer_0200
 * @tc.desc: Finalizer destructs a real JsUIServiceProxy object without crash.
 */
HWTEST_F(JsUIServiceProxyTest, Finalizer_0200, TestSize.Level1)
{
    sptr<IRemoteObject> impl = new IRemoteObjectMocker();
    sptr<IRemoteObject> hostProxy = new IRemoteObjectMocker();
    auto* raw = new AAFwk::JsUIServiceProxy(impl, hostProxy);
    AAFwk::JsUIServiceProxy::Finalizer(nullptr, raw, nullptr);
    SUCCEED();
}

} // namespace AbilityRuntime
} // namespace OHOS
