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

    void OnLoadedDone(int32_t hostPid, std::string bundleName, int32_t extensionAbilityId) override
    {
        onLoadedDoneCalled_ = true;
        hostPid_ = hostPid;
        bundleName_ = std::move(bundleName);
        extensionAbilityId_ = extensionAbilityId;
    }

    void OnDestroyDone(int32_t hostPid, std::string bundleName, int32_t extensionAbilityId) override
    {
        onDestroyDoneCalled_ = true;
        hostPid_ = hostPid;
        bundleName_ = std::move(bundleName);
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
    
    int32_t GetHostPid() const { return hostPid_; }
    std::string GetBundleName() const { return bundleName_; }
    int32_t GetExtensionAbilityId() const { return extensionAbilityId_; }
    int32_t GetRequestCode() const { return requestCode_; }
    int32_t GetInnerErrCode() const { return innerErrCode_; }

    void Reset()
    {
        onLoadedDoneCalled_ = false;
        onDestroyDoneCalled_ = false;
        onPreloadSuccessCalled_ = false;
        hostPid_ = 0;
        bundleName_.clear();
        extensionAbilityId_ = 0;
        requestCode_ = 0;
        innerErrCode_ = 0;
    }

private:
    bool onLoadedDoneCalled_ = false;
    bool onDestroyDoneCalled_ = false;
    bool onPreloadSuccessCalled_ = false;
    int32_t hostPid_ = 0;
    std::string bundleName_;
    int32_t extensionAbilityId_ = 0;
    int32_t requestCode_ = 0;
    int32_t innerErrCode_ = 0;
};

class MockRemoteObject : public IRemoteStub<IPreloadUIExtensionExecuteCallback> {
public:
    MockRemoteObject() = default;
    virtual ~MockRemoteObject() = default;

    MOCK_METHOD4(SendRequest, int(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option));
    
    void OnLoadedDone(int32_t hostPid, std::string bundleName, int32_t extensionAbilityId) override {}
    void OnDestroyDone(int32_t hostPid, std::string bundleName, int32_t extensionAbilityId) override {}
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
 * @tc.name: OnRemoteRequest_InvalidToken_0100
 * @tc.desc: Test OnRemoteRequest with invalid interface token
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, OnRemoteRequest_InvalidToken_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(u"invalid.token");
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE,
        data, reply, option);
    EXPECT_EQ(result, ERR_INVALID_STATE);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnRemoteRequest_InvalidCode_0100
 * @tc.desc: Test OnRemoteRequest with invalid code
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, OnRemoteRequest_InvalidCode_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IPreloadUIExtensionExecuteCallback::GetDescriptor());
    uint32_t invalidCode = 9999;
    auto result = stub_->OnRemoteRequest(invalidCode, data, reply, option);
    EXPECT_NE(result, ERR_OK);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

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
    int32_t hostPid = 12345;
    std::string bundleName = "com.example.test";
    int32_t extensionAbilityId = 100;
    data.WriteInt32(hostPid);
    data.WriteString(bundleName);
    data.WriteInt32(extensionAbilityId);
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE,
        data, reply, option);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(stub_->IsOnLoadedDoneCalled());
    EXPECT_EQ(stub_->GetHostPid(), hostPid);
    EXPECT_EQ(stub_->GetBundleName(), bundleName);
    EXPECT_EQ(stub_->GetExtensionAbilityId(), extensionAbilityId);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: HandleOnLoadedDone_EmptyBundleName_0200
 * @tc.desc: Test HandleOnLoadedDone with empty bundle name
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, HandleOnLoadedDone_EmptyBundleName_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IPreloadUIExtensionExecuteCallback::GetDescriptor());
    int32_t hostPid = 12345;
    std::string bundleName = "";
    int32_t extensionAbilityId = 100;
    data.WriteInt32(hostPid);
    data.WriteString(bundleName);
    data.WriteInt32(extensionAbilityId);
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE,
        data, reply, option);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(stub_->IsOnLoadedDoneCalled());
    EXPECT_EQ(stub_->GetBundleName(), "");
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
    int32_t hostPid = 67890;
    std::string bundleName = "com.example.destroy";
    int32_t extensionAbilityId = 200;
    data.WriteInt32(hostPid);
    data.WriteString(bundleName);
    data.WriteInt32(extensionAbilityId);
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_DESTROY_DONE,
        data, reply, option);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(stub_->IsOnDestroyDoneCalled());
    EXPECT_EQ(stub_->GetHostPid(), hostPid);
    EXPECT_EQ(stub_->GetBundleName(), bundleName);
    EXPECT_EQ(stub_->GetExtensionAbilityId(), extensionAbilityId);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: HandleOnPreloadSuccess_0100
 * @tc.desc: Test HandleOnPreloadSuccess with valid parameters
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, HandleOnPreloadSuccess_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IPreloadUIExtensionExecuteCallback::GetDescriptor());
    int32_t requestCode = 1001;
    int32_t extensionAbilityId = 300;
    int32_t innerErrCode = 0;
    data.WriteInt32(requestCode);
    data.WriteInt32(extensionAbilityId);
    data.WriteInt32(innerErrCode);
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_SUCCESS,
        data, reply, option);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(stub_->IsOnPreloadSuccessCalled());
    EXPECT_EQ(stub_->GetRequestCode(), requestCode);
    EXPECT_EQ(stub_->GetExtensionAbilityId(), extensionAbilityId);
    EXPECT_EQ(stub_->GetInnerErrCode(), innerErrCode);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: HandleOnPreloadSuccess_WithError_0200
 * @tc.desc: Test HandleOnPreloadSuccess with error code
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackStubTest, HandleOnPreloadSuccess_WithError_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(IPreloadUIExtensionExecuteCallback::GetDescriptor());
    int32_t requestCode = 1001;
    int32_t extensionAbilityId = 300;
    int32_t innerErrCode = -1;
    data.WriteInt32(requestCode);
    data.WriteInt32(extensionAbilityId);
    data.WriteInt32(innerErrCode);
    auto result = stub_->OnRemoteRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_SUCCESS,
        data, reply, option);
    EXPECT_EQ(result, ERR_OK);
    EXPECT_TRUE(stub_->IsOnPreloadSuccessCalled());
    EXPECT_EQ(stub_->GetInnerErrCode(), innerErrCode);
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnLoadedDone_Success_0100
 * @tc.desc: Test OnLoadedDone proxy method with successful SendRequest
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, OnLoadedDone_Success_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    
    int32_t hostPid = 12345;
    std::string bundleName = "com.example.test";
    int32_t extensionAbilityId = 100;
    
    EXPECT_CALL(*mockRemote_, SendRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE, _, _, _))
        .WillOnce(Return(ERR_OK));
    
    proxy_->OnLoadedDone(hostPid, bundleName, extensionAbilityId);
    
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnLoadedDone_SendRequestFailed_0200
 * @tc.desc: Test OnLoadedDone proxy method with failed SendRequest
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, OnLoadedDone_SendRequestFailed_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    
    int32_t hostPid = 12345;
    std::string bundleName = "com.example.test";
    int32_t extensionAbilityId = 100;
    
    EXPECT_CALL(*mockRemote_, SendRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_LOADED_DONE, _, _, _))
        .WillOnce(Return(ERR_INVALID_STATE));
    
    proxy_->OnLoadedDone(hostPid, bundleName, extensionAbilityId);
    
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnLoadedDone_EmptyBundleName_0300
 * @tc.desc: Test OnLoadedDone with empty bundle name
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, OnLoadedDone_EmptyBundleName_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    
    int32_t hostPid = 12345;
    std::string bundleName = "";
    int32_t extensionAbilityId = 100;
    
    EXPECT_CALL(*mockRemote_, SendRequest(_, _, _, _))
        .WillOnce(Return(ERR_OK));
    
    proxy_->OnLoadedDone(hostPid, bundleName, extensionAbilityId);
    
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnDestroyDone_Success_0100
 * @tc.desc: Test OnDestroyDone proxy method with successful SendRequest
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, OnDestroyDone_Success_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    
    int32_t hostPid = 67890;
    std::string bundleName = "com.example.destroy";
    int32_t extensionAbilityId = 200;
    
    EXPECT_CALL(*mockRemote_, SendRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_DESTROY_DONE, _, _, _))
        .WillOnce(Return(ERR_OK));
    
    proxy_->OnDestroyDone(hostPid, bundleName, extensionAbilityId);
    
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnDestroyDone_SendRequestFailed_0200
 * @tc.desc: Test OnDestroyDone proxy method with failed SendRequest
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, OnDestroyDone_SendRequestFailed_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    
    int32_t hostPid = 67890;
    std::string bundleName = "com.example.destroy";
    int32_t extensionAbilityId = 200;
    
    EXPECT_CALL(*mockRemote_, SendRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_DESTROY_DONE, _, _, _))
        .WillOnce(Return(ERR_INVALID_STATE));
    
    proxy_->OnDestroyDone(hostPid, bundleName, extensionAbilityId);
    
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnPreloadSuccess_Success_0100
 * @tc.desc: Test OnPreloadSuccess proxy method with successful SendRequest
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, OnPreloadSuccess_Success_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    
    int32_t requestCode = 1001;
    int32_t extensionAbilityId = 300;
    int32_t innerErrCode = 0;
    
    EXPECT_CALL(*mockRemote_, SendRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_SUCCESS, _, _, _))
        .WillOnce(Return(ERR_OK));
    
    proxy_->OnPreloadSuccess(requestCode, extensionAbilityId, innerErrCode);
    
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnPreloadSuccess_SendRequestFailed_0200
 * @tc.desc: Test OnPreloadSuccess proxy method with failed SendRequest
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, OnPreloadSuccess_SendRequestFailed_0200, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    
    int32_t requestCode = 1001;
    int32_t extensionAbilityId = 300;
    int32_t innerErrCode = 0;
    
    EXPECT_CALL(*mockRemote_, SendRequest(
        IPreloadUIExtensionExecuteCallback::ON_PRELOAD_UI_EXTENSION_ABILITY_SUCCESS, _, _, _))
        .WillOnce(Return(ERR_INVALID_STATE));
    
    proxy_->OnPreloadSuccess(requestCode, extensionAbilityId, innerErrCode);
    
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: OnPreloadSuccess_WithErrorCode_0300
 * @tc.desc: Test OnPreloadSuccess with error code
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, OnPreloadSuccess_WithErrorCode_0300, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    
    int32_t requestCode = 1001;
    int32_t extensionAbilityId = 300;
    int32_t innerErrCode = -1;
    
    EXPECT_CALL(*mockRemote_, SendRequest(_, _, _, _))
        .WillOnce(Return(ERR_OK));
    
    proxy_->OnPreloadSuccess(requestCode, extensionAbilityId, innerErrCode);
    
    TAG_LOGI(AAFwkTag::TEST, "end");
}

/**
 * @tc.name: MultipleCallsSequence_0100
 * @tc.desc: Test multiple sequential calls
 * @tc.type: FUNC
 */
HWTEST_F(PreloadUIExtensionExecuteCallbackProxyTest, MultipleCallsSequence_0100, TestSize.Level1)
{
    TAG_LOGI(AAFwkTag::TEST, "begin");
    
    EXPECT_CALL(*mockRemote_, SendRequest(_, _, _, _))
        .Times(3)
        .WillRepeatedly(Return(ERR_OK));
    
    proxy_->OnLoadedDone(123, "com.test.app1", 100);
    proxy_->OnPreloadSuccess(456, 200, 0);
    proxy_->OnDestroyDone(789, "com.test.app2", 300);
    
    TAG_LOGI(AAFwkTag::TEST, "end");
}

} // namespace AAFwk
} // namespace OHOS