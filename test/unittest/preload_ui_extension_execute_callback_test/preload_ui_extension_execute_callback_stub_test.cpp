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
#include "preload_ui_extension_execute_callback_stub.h"
#include "preload_ui_extension_execute_callback_proxy.h"
#include "hilog_tag_wrapper.h"
#include "iremote_object.h"
#include "iremote_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AAFwk {

class MockPreloadUIExtensionExecuteCallback : public PreloadUIExtensionExecuteCallbackStub {
public:
    MockPreloadUIExtensionExecuteCallback() = default;
    virtual ~MockPreloadUIExtensionExecuteCallback() = default;

    void OnLoadedDone(int32_t extensionAbilityId) override
    {
        onLoadedDoneCalled_ = true;
        extensionAbilityId_ = extensionAbilityId;
    }

    void OnDestroyDone(int32_t extensionAbilityId) override
    {
        onDestroyDoneCalled_ = true;
        extensionAbilityId_ = extensionAbilityId;
    }

    void OnPreloadSuccess(int32_t requestCode, int32_t extensionAbilityId, int32_t innerErrCode) override
    {
        onPreloadSuccessCalled_ = true;
        requestCode_ = requestCode;
        extensionAbilityId_ = extensionAbilityId;
        innerErrCode_ = innerErrCode;
    }

    bool IsOnLoadedDoneCalled() const { return onLoadedDoneCalled_; }
    bool IsOnDestroyDoneCalled() const { return onDestroyDoneCalled_; }
    bool IsOnPreloadSuccessCalled() const { return onPreloadSuccessCalled_; }
    
    int32_t GetExtensionAbilityId() const { return extensionAbilityId_; }
    int32_t GetRequestCode() const { return requestCode_; }
    int32_t GetInnerErrCode() const { return innerErrCode_; }

    void Reset()
    {
        onLoadedDoneCalled_ = false;
        onDestroyDoneCalled_ = false;
        onPreloadSuccessCalled_ = false;
        extensionAbilityId_ = 0;
        requestCode_ = 0;
        innerErrCode_ = 0;
    }

private:
    bool onLoadedDoneCalled_ = false;
    bool onDestroyDoneCalled_ = false;
    bool onPreloadSuccessCalled_ = false;
    int32_t extensionAbilityId_ = 0;
    int32_t requestCode_ = 0;
    int32_t innerErrCode_ = 0;
};

class MockRemoteObject : public IRemoteStub<IPreloadUIExtensionExecuteCallback> {
public:
    MockRemoteObject() = default;
    virtual ~MockRemoteObject() = default;

    MOCK_METHOD4(SendRequest, int(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));
    
    void OnLoadedDone(int32_t extensionAbilityId) override {}
    void OnDestroyDone(int32_t extensionAbilityId) override {}
    void OnPreloadSuccess(int32_t requestCode, int32_t extensionAbilityId, int32_t innerErrCode) override {}
    
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override
    {
        return ERR_OK;
    }
};

class PreloadUIExtensionExecuteCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() override
    {
        stub_ = new (std::nothrow) MockPreloadUIExtensionExecuteCallback();
    }
    void TearDown() override
    {
        stub_ = nullptr;
    }

    sptr<MockPreloadUIExtensionExecuteCallback> stub_ = nullptr;
};

class PreloadUIExtensionExecuteCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    
    void SetUp() override
    {
        mockRemote_ = new (std::nothrow) MockRemoteObject();
        ASSERT_NE(mockRemote_, nullptr);
        proxy_ = new (std::nothrow) PreloadUIExtensionExecuteCallbackProxy(mockRemote_);
        ASSERT_NE(proxy_, nullptr);
    }
    
    void TearDown() override
    {
        proxy_ = nullptr;
        mockRemote_ = nullptr;
    }

    sptr<MockRemoteObject> mockRemote_ = nullptr;
    sptr<PreloadUIExtensionExecuteCallbackProxy> proxy_ = nullptr;
};

/**
 * @tc.name: HandleOnLoadedDone_0100
 * @tc.desc: Test HandleOnLoadedDone with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, HandleOnLoadedDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IPreloadUIExtensionExecuteCallback::GetDescriptor());
    
    int32_t extensionAbilityId = 100;
    data.WriteInt32(extensionAbilityId);
    
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE,
        data, reply, option);
    
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(stub_->IsOnLoadedDoneCalled());
    EXPECT_EQ(stub_->GetExtensionAbilityId(), extensionAbilityId);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: HandleOnLoadedDone_EmptyBundleName_0200
 * @tc.desc: Test HandleOnLoadedDone with another extensionAbilityId
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, HandleOnLoadedDone_EmptyBundleName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IPreloadUIExtensionExecuteCallback::GetDescriptor());
    
    int32_t extensionAbilityId = 200;
    data.WriteInt32(extensionAbilityId);
    
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE,
        data, reply, option);
    
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(stub_->IsOnLoadedDoneCalled());
    EXPECT_EQ(stub_->GetExtensionAbilityId(), extensionAbilityId);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: HandleOnDestroyDone_0100
 * @tc.desc: Test HandleOnDestroyDone with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, HandleOnDestroyDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IPreloadUIExtensionExecuteCallback::GetDescriptor());
    
    int32_t extensionAbilityId = 300;
    data.WriteInt32(extensionAbilityId);
    
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_DESTROY_DONE, data, reply, option);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(stub_->IsOnDestroyDoneCalled());
    EXPECT_EQ(stub_->GetExtensionAbilityId(), extensionAbilityId);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnRemoteRequest_TokenMismatch_0100
 * @tc.desc: Test InterfaceToken mismatch.
 *           Expected: ERR_INVALID_STATE
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, OnRemoteRequest_TokenMismatch_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequest_TokenMismatch_0100 start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(u"Invalid.Token.Descriptor");
    
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE, data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    TAG_LOGI(AAFwkTag::TEST, "OnRemoteRequest_TokenMismatch_0100 end");
}

/**
 * @tc.name: HandleOnPreloadSuccess_0100
 * @tc.desc: Test ON_PRELOAD_UI_EXTENSION_ABILITY_SUCCESS branch and HandleOnPreloadSuccess impl.
 *           Expected: ERR_OK and callback invoked.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, HandleOnPreloadSuccess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "HandleOnPreloadSuccess_0100 start");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(IPreloadUIExtensionExecuteCallback::GetDescriptor());
    
    int32_t requestCode = 10;
    int32_t extensionAbilityId = 500;
    int32_t innerErrCode = 0;

    data.WriteInt32(requestCode);
    data.WriteInt32(extensionAbilityId);
    data.WriteInt32(innerErrCode);
    
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_SUCCESS,
        data, reply, option);
    
    EXPECT_EQ(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "HandleOnPreloadSuccess_0100 end");
}

/**
 * @tc.name: OnLoadedDone_0100
 * @tc.desc: Test OnLoadedDone normal execution.
 *           Verify SendRequest is called with correct code.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, OnLoadedDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnLoadedDone_0100 start");
    int32_t extensionId = 1001;
    EXPECT_CALL(*mockRemote_, SendRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE,
        _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));
    proxy_->OnLoadedDone(extensionId);
    TAG_LOGI(AAFwkTag::TEST, "OnLoadedDone_0100 end");
}

/**
 * @tc.name: OnLoadedDone_0200
 * @tc.desc: Test OnLoadedDone when SendRequest fails.
 *           Verify logic handles error gracefully (logs error, no crash).
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, OnLoadedDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnLoadedDone_0200 start");
    int32_t extensionId = 1002;
    EXPECT_CALL(*mockRemote_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_INVALID_VALUE));
    proxy_->OnLoadedDone(extensionId);
    TAG_LOGI(AAFwkTag::TEST, "OnLoadedDone_0200 end");
}

/**
 * @tc.name: OnDestroyDone_0100
 * @tc.desc: Test OnDestroyDone normal execution.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, OnDestroyDone_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnDestroyDone_0100 start");
    int32_t extensionId = 2001;

    EXPECT_CALL(*mockRemote_, SendRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_DESTROY_DONE,
        _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));
    proxy_->OnDestroyDone(extensionId);
    TAG_LOGI(AAFwkTag::TEST, "OnDestroyDone_0100 end");
}

/**
 * @tc.name: OnDestroyDone_0200
 * @tc.desc: Test OnDestroyDone failure case.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, OnDestroyDone_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnDestroyDone_0200 start");
    int32_t extensionId = 2002;

    EXPECT_CALL(*mockRemote_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_NULL_OBJECT));
    proxy_->OnDestroyDone(extensionId);
    TAG_LOGI(AAFwkTag::TEST, "OnDestroyDone_0200 end");
}

/**
 * @tc.name: OnPreloadSuccess_0100
 * @tc.desc: Test OnPreloadSuccess normal execution.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, OnPreloadSuccess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnPreloadSuccess_0100 start");
    int32_t reqCode = 10;
    int32_t extId = 500;
    int32_t errCode = 0;

    EXPECT_CALL(*mockRemote_, SendRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_SUCCESS,
        _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_OK));
    proxy_->OnPreloadSuccess(reqCode, extId, errCode);
    TAG_LOGI(AAFwkTag::TEST, "OnPreloadSuccess_0100 end");
}

/**
 * @tc.name: OnPreloadSuccess_0200
 * @tc.desc: Test OnPreloadSuccess failure case.
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, OnPreloadSuccess_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "OnPreloadSuccess_0200 start");
    
    EXPECT_CALL(*mockRemote_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Return(ERR_TIMED_OUT));
    proxy_->OnPreloadSuccess(1, 1, 1);
    TAG_LOGI(AAFwkTag::TEST, "OnPreloadSuccess_0200 end");
}
} // namespace AAFwk
} // namespace OHOS