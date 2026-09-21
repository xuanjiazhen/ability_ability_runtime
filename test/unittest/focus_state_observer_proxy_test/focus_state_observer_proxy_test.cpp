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
#include "focus_state_observer_proxy.h"
#include "focus_state_observer_stub.h"
#include "parcel.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class FocusStateObserverProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    sptr<FocusStateObserverStub> stub_ {nullptr};
    sptr<FocusStateObserverProxy> proxy_ {nullptr};
};

void FocusStateObserverProxyTest::SetUpTestCase(void) {}
void FocusStateObserverProxyTest::TearDownTestCase(void) {}

class TestFocusStateObserverStub : public FocusStateObserverStub {
public:
    void OnFocusStateChanged(const FocusStateData &focusStateData) override
    {
        receivedData_ = focusStateData;
    }
    FocusStateData receivedData_;
};

void FocusStateObserverProxyTest::SetUp()
{
    stub_ = new (std::nothrow) TestFocusStateObserverStub();
    ASSERT_NE(stub_, nullptr);
    proxy_ = new (std::nothrow) FocusStateObserverProxy(stub_);
    ASSERT_NE(proxy_, nullptr);
}

void FocusStateObserverProxyTest::TearDown()
{
    if (proxy_ != nullptr) {
        proxy_ = nullptr;
    }
    if (stub_ != nullptr) {
        stub_ = nullptr;
    }
}

/**
 * @tc.number: OnFocusStateChanged_0100
 * @tc.desc: Test proxy OnFocusStateChanged sends data correctly.
 * @tc.type: FUNC
 */
HWTEST_F(FocusStateObserverProxyTest, OnFocusStateChanged_0100, TestSize.Level1)
{
    FocusStateData data;
    data.focusedUid = 10001;
    data.unfocusedUid = 10002;
    data.focusedBundleName = "com.example.focused";
    data.unfocusedBundleName = "com.example.unfocused";
    proxy_->OnFocusStateChanged(data);
    auto stub = iface_cast<TestFocusStateObserverStub>(stub_);
    ASSERT_NE(stub, nullptr);
    EXPECT_EQ(stub->receivedData_.focusedUid, 10001);
    EXPECT_EQ(stub->receivedData_.unfocusedUid, 10002);
    EXPECT_EQ(stub->receivedData_.focusedBundleName, "com.example.focused");
    EXPECT_EQ(stub->receivedData_.unfocusedBundleName, "com.example.unfocused");
}

/**
 * @tc.number: OnFocusStateChanged_0200
 * @tc.desc: Test proxy OnFocusStateChanged with first-focus values (unfocusedUid = -1).
 * @tc.type: FUNC
 */
HWTEST_F(FocusStateObserverProxyTest, OnFocusStateChanged_0200, TestSize.Level1)
{
    FocusStateData data;
    data.focusedUid = 10001;
    data.unfocusedUid = -1;
    data.focusedBundleName = "com.example.first";
    data.unfocusedBundleName = "";
    proxy_->OnFocusStateChanged(data);
    auto stub = iface_cast<TestFocusStateObserverStub>(stub_);
    ASSERT_NE(stub, nullptr);
    EXPECT_EQ(stub->receivedData_.focusedUid, 10001);
    EXPECT_EQ(stub->receivedData_.unfocusedUid, -1);
    EXPECT_EQ(stub->receivedData_.unfocusedBundleName, "");
}
}  // namespace AppExecFwk
}  // namespace OHOS
