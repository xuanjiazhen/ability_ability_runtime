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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "message_parcel.h"

#define private public
#include "query_erms_observer_proxy.h"
#undef private

#include "ipc_types.h"
#include "iremote_stub.h"
#include "query_erms_observer_interface.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
namespace {
constexpr int32_t SEND_REQUEST_ERROR = -1;
}

class QueryERMSObserverStubMock : public IRemoteStub<IQueryERMSObserver> {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.IQueryERMSObserver");

    MOCK_METHOD4(SendRequest, int(uint32_t, MessageParcel &, MessageParcel &, MessageOption &));

    void OnQueryFinished(const std::string &, const std::string &, const AtomicServiceStartupRule &, int) override {}

    int InvokeSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return NO_ERROR;
    }

    int InvokeErrorSendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
    {
        code_ = code;
        return SEND_REQUEST_ERROR;
    }

    uint32_t code_ = 0;
};

class QueryERMSObserverProxyTest : public testing::Test {
public:
    void SetUp() override
    {
        ResetMessageParcelState();
        mock_ = new QueryERMSObserverStubMock();
        proxy_ = new QueryERMSObserverProxy(mock_);
    }

    sptr<QueryERMSObserverProxy> proxy_ { nullptr };
    sptr<QueryERMSObserverStubMock> mock_ { nullptr };
};

/**
 * @tc.number: WriteInterfaceToken_0100
 * @tc.name: WriteInterfaceToken
 * @tc.desc: Write token into parcel data.
 */
HWTEST_F(QueryERMSObserverProxyTest, WriteInterfaceToken_0100, Function | MediumTest | Level1)
{
    MessageParcel data;
    auto ret = proxy_->WriteInterfaceToken(data);
    EXPECT_TRUE(ret);
    EXPECT_EQ(GetWriteInterfaceTokenCalls(), 1u);
}

/**
 * @tc.number: OnQueryFinished_0100
 * @tc.name: OnQueryFinished
 * @tc.desc: Verify send request success path.
 */
HWTEST_F(QueryERMSObserverProxyTest, OnQueryFinished_0100, Function | MediumTest | Level1)
{
    AtomicServiceStartupRule rule { true, false };
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &QueryERMSObserverStubMock::InvokeSendRequest));
    proxy_->OnQueryFinished("appId", "12:00", rule, 0);
    EXPECT_EQ(mock_->code_, IQueryERMSObserver::ON_QUERY_FINISHED);
    EXPECT_EQ(GetWriteInterfaceTokenCalls(), 1u);
    EXPECT_EQ(GetWriteStringCalls(), 2u);
    EXPECT_EQ(GetWriteBoolCalls(), 2u);
    EXPECT_EQ(GetWriteInt32Calls(), 1u);
}

/**
 * @tc.number: OnQueryFinished_0200
 * @tc.name: OnQueryFinished
 * @tc.desc: Verify remote null branch.
 */
HWTEST_F(QueryERMSObserverProxyTest, OnQueryFinished_0200, Function | MediumTest | Level1)
{
    AtomicServiceStartupRule rule { true, true };
    sptr<IRemoteObject> remote = nullptr;
    QueryERMSObserverProxy proxy(remote);
    proxy.OnQueryFinished("appId", "12:00", rule, 0);
    EXPECT_EQ(GetWriteInterfaceTokenCalls(), 1u);
    EXPECT_EQ(GetWriteStringCalls(), 2u);
    EXPECT_EQ(GetWriteBoolCalls(), 2u);
    EXPECT_EQ(GetWriteInt32Calls(), 1u);
}

/**
 * @tc.number: OnQueryFinished_0300
 * @tc.name: OnQueryFinished
 * @tc.desc: Verify send request error branch.
 */
HWTEST_F(QueryERMSObserverProxyTest, OnQueryFinished_0300, Function | MediumTest | Level1)
{
    AtomicServiceStartupRule rule { false, false };
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _))
        .Times(1)
        .WillOnce(Invoke(mock_.GetRefPtr(), &QueryERMSObserverStubMock::InvokeErrorSendRequest));
    proxy_->OnQueryFinished("appId", "12:00", rule, -1);
    EXPECT_EQ(mock_->code_, IQueryERMSObserver::ON_QUERY_FINISHED);
    EXPECT_EQ(GetWriteInterfaceTokenCalls(), 1u);
    EXPECT_EQ(GetWriteStringCalls(), 2u);
    EXPECT_EQ(GetWriteBoolCalls(), 2u);
    EXPECT_EQ(GetWriteInt32Calls(), 1u);
}

/**
 * @tc.number: OnQueryFinished_0400
 * @tc.name: OnQueryFinished
 * @tc.desc: Verify WriteInterfaceToken failure branch.
 */
HWTEST_F(QueryERMSObserverProxyTest, OnQueryFinished_0400, Function | MediumTest | Level1)
{
    AtomicServiceStartupRule rule { true, false };
    MockWriteInterfaceToken(false);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(0);
    proxy_->OnQueryFinished("appId", "12:00", rule, 0);
    EXPECT_EQ(GetWriteInterfaceTokenCalls(), 1u);
    EXPECT_EQ(GetWriteStringCalls(), 0u);
    EXPECT_EQ(GetWriteBoolCalls(), 0u);
    EXPECT_EQ(GetWriteInt32Calls(), 0u);
    EXPECT_EQ(mock_->code_, 0u);
}

/**
 * @tc.number: OnQueryFinished_0500
 * @tc.name: OnQueryFinished
 * @tc.desc: Verify first WriteString failure branch.
 */
HWTEST_F(QueryERMSObserverProxyTest, OnQueryFinished_0500, Function | MediumTest | Level1)
{
    AtomicServiceStartupRule rule { true, false };
    MockWriteString(false, 0);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(0);
    proxy_->OnQueryFinished("appId", "12:00", rule, 0);
    EXPECT_EQ(GetWriteInterfaceTokenCalls(), 1u);
    EXPECT_EQ(GetWriteStringCalls(), 1u);
    EXPECT_EQ(GetWriteBoolCalls(), 0u);
    EXPECT_EQ(GetWriteInt32Calls(), 0u);
    EXPECT_EQ(mock_->code_, 0u);
}

/**
 * @tc.number: OnQueryFinished_0600
 * @tc.name: OnQueryFinished
 * @tc.desc: Verify second WriteString failure branch.
 */
HWTEST_F(QueryERMSObserverProxyTest, OnQueryFinished_0600, Function | MediumTest | Level1)
{
    AtomicServiceStartupRule rule { true, false };
    MockWriteString(false, 1);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(0);
    proxy_->OnQueryFinished("appId", "12:00", rule, 0);
    EXPECT_EQ(GetWriteInterfaceTokenCalls(), 1u);
    EXPECT_EQ(GetWriteStringCalls(), 2u);
    EXPECT_EQ(GetWriteBoolCalls(), 0u);
    EXPECT_EQ(GetWriteInt32Calls(), 0u);
    EXPECT_EQ(mock_->code_, 0u);
}

/**
 * @tc.number: OnQueryFinished_0700
 * @tc.name: OnQueryFinished
 * @tc.desc: Verify WriteBool failure branch.
 */
HWTEST_F(QueryERMSObserverProxyTest, OnQueryFinished_0700, Function | MediumTest | Level1)
{
    AtomicServiceStartupRule rule { true, false };
    MockWriteBool(false, 0);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(0);
    proxy_->OnQueryFinished("appId", "12:00", rule, 0);
    EXPECT_EQ(GetWriteInterfaceTokenCalls(), 1u);
    EXPECT_EQ(GetWriteStringCalls(), 2u);
    EXPECT_EQ(GetWriteBoolCalls(), 1u);
    EXPECT_EQ(GetWriteInt32Calls(), 0u);
    EXPECT_EQ(mock_->code_, 0u);
}

/**
 * @tc.number: OnQueryFinished_0800
 * @tc.name: OnQueryFinished
 * @tc.desc: Verify WriteInt32 failure branch.
 */
HWTEST_F(QueryERMSObserverProxyTest, OnQueryFinished_0800, Function | MediumTest | Level1)
{
    AtomicServiceStartupRule rule { true, false };
    MockWriteInt32(false, 0);
    EXPECT_CALL(*mock_, SendRequest(_, _, _, _)).Times(0);
    proxy_->OnQueryFinished("appId", "12:00", rule, 0);
    EXPECT_EQ(GetWriteInterfaceTokenCalls(), 1u);
    EXPECT_EQ(GetWriteStringCalls(), 2u);
    EXPECT_EQ(GetWriteBoolCalls(), 2u);
    EXPECT_EQ(GetWriteInt32Calls(), 1u);
    EXPECT_EQ(mock_->code_, 0u);
}
} // namespace AbilityRuntime
} // namespace OHOS
