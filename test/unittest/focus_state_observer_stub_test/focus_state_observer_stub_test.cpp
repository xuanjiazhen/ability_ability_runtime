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

#define private public
#define protected public
#include "focus_state_data.h"
#include "focus_state_observer_interface.h"
#include "focus_state_observer_stub.h"
#include "mock_focus_state_observer_stub.h"
#include "parcel.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class FocusStateObserverStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<MockFocusStateObserverStub> observerStub_ {nullptr};
};

void FocusStateObserverStubTest::SetUpTestCase(void) {}
void FocusStateObserverStubTest::TearDownTestCase(void) {}
void FocusStateObserverStubTest::SetUp()
{
    observerStub_ = new (std::nothrow) MockFocusStateObserverStub();
}
void FocusStateObserverStubTest::TearDown()
{
    if (observerStub_ != nullptr) {
        testing::Mock::AllowLeak(observerStub_);
        observerStub_ = nullptr;
    }
}

/**
 * @tc.number: HandleOnFocusStateChanged_0100
 * @tc.desc: Test HandleOnFocusStateChanged with valid FocusStateData.
 * @tc.type: FUNC
 */
HWTEST_F(FocusStateObserverStubTest, HandleOnFocusStateChanged_0100, TestSize.Level1)
{
    auto stub = new (std::nothrow) MockFocusStateObserverStub();
    ASSERT_NE(stub, nullptr);
    EXPECT_CALL(*stub, OnFocusStateChanged(_)).Times(1);
    MessageParcel data;
    FocusStateData focusStateData;
    focusStateData.focusedUid = 10001;
    focusStateData.unfocusedUid = 10002;
    focusStateData.focusedBundleName = "com.example.focused";
    focusStateData.unfocusedBundleName = "com.example.unfocused";
    data.WriteParcelable(&focusStateData);
    MessageParcel reply;
    auto result = stub->HandleOnFocusStateChanged(data, reply);
    EXPECT_EQ(NO_ERROR, result);
    testing::Mock::AllowLeak(stub);
}

/**
 * @tc.number: OnRemoteRequest_0100
 * @tc.desc: Test OnRemoteRequest dispatches ON_FOCUS_STATE_CHANGED.
 * @tc.type: FUNC
 */
HWTEST_F(FocusStateObserverStubTest, OnRemoteRequest_0100, TestSize.Level1)
{
    auto stub = new (std::nothrow) MockFocusStateObserverStub();
    ASSERT_NE(stub, nullptr);
    EXPECT_CALL(*stub, OnFocusStateChanged(_)).Times(1);
    MessageParcel data;
    data.WriteInterfaceToken(IFocusStateObserver::GetDescriptor());
    FocusStateData focusStateData;
    focusStateData.focusedUid = 10001;
    focusStateData.unfocusedUid = -1;
    focusStateData.focusedBundleName = "com.example.first";
    data.WriteParcelable(&focusStateData);
    MessageParcel reply;
    MessageOption option;
    auto result = stub->OnRemoteRequest(
        static_cast<uint32_t>(IFocusStateObserver::Message::ON_FOCUS_STATE_CHANGED),
        data, reply, option);
    EXPECT_EQ(NO_ERROR, result);
    testing::Mock::AllowLeak(stub);
}
}  // namespace AppExecFwk
}  // namespace OHOS
