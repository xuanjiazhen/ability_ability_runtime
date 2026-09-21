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
#include "app_state_observer_manager.h"
#include "focus_state_data.h"
#include "focus_state_observer_interface.h"
#include "focus_state_observer_stub.h"
#undef private
#undef protected
#include "hilog_tag_wrapper.h"
#include "mock_native_token.h"
#include "mock_permission_verification.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int32_t TEST_MAX_FOCUS_OBSERVER_PER_APP = 10;
constexpr int32_t TEST_ERR_FOCUS_OBSERVER_EXCEED_LIMIT = 16000165;
}
class AppStateObserverManagerFocusTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<AppStateObserverManager> manager_;
};

void AppStateObserverManagerFocusTest::SetUpTestCase(void) {}
void AppStateObserverManagerFocusTest::TearDownTestCase(void) {}
void AppStateObserverManagerFocusTest::SetUp()
{
    manager_ = AppStateObserverManager::GetInstance();
    ASSERT_NE(manager_, nullptr);
}
void AppStateObserverManagerFocusTest::TearDown()
{
    manager_ = nullptr;
}

class MockFocusStateObserver : public FocusStateObserverStub {
public:
    MockFocusStateObserver() = default;
    ~MockFocusStateObserver() override = default;
    void OnFocusStateChanged(const FocusStateData &focusStateData) override
    {
        receivedData_ = focusStateData;
        called_ = true;
    }
    void Reset()
    {
        called_ = false;
        receivedData_ = FocusStateData();
    }
    FocusStateData receivedData_;
    bool called_ = false;
};

/**
 * @tc.number: RegisterFocusStateObserver_0100
 * @tc.desc: AC-1: register null observer returns ERR_INVALID_VALUE.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, RegisterFocusStateObserver_0100, TestSize.Level1)
{
    sptr<IFocusStateObserver> observer = nullptr;
    auto ret = manager_->RegisterFocusStateObserver(observer);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: RegisterFocusStateObserver_0200
 * @tc.desc: AC-1: register valid observer succeeds and map contains it.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, RegisterFocusStateObserver_0200, TestSize.Level1)
{
    auto observer = sptr<MockFocusStateObserver>(new MockFocusStateObserver());
    ASSERT_NE(observer, nullptr);
    auto ret = manager_->RegisterFocusStateObserver(observer);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_FALSE(manager_->focusStateObserverMap_.empty());
    EXPECT_TRUE(manager_->IsFocusStateObserverExist(observer));
    manager_->UnregisterFocusStateObserver(observer);
    EXPECT_FALSE(manager_->IsFocusStateObserverExist(observer));
}

/**
 * @tc.number: UnregisterFocusStateObserver_0100
 * @tc.desc: AC-6: unregister null observer returns ERR_INVALID_VALUE.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, UnregisterFocusStateObserver_0100, TestSize.Level1)
{
    sptr<IFocusStateObserver> observer = nullptr;
    auto ret = manager_->UnregisterFocusStateObserver(observer);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: UnregisterFocusStateObserver_0200
 * @tc.desc: AC-6: unregister valid observer succeeds and map is empty.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, UnregisterFocusStateObserver_0200, TestSize.Level1)
{
    auto observer = sptr<MockFocusStateObserver>(new MockFocusStateObserver());
    ASSERT_NE(observer, nullptr);
    manager_->RegisterFocusStateObserver(observer);
    EXPECT_FALSE(manager_->focusStateObserverMap_.empty());
    auto ret = manager_->UnregisterFocusStateObserver(observer);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(manager_->IsFocusStateObserverExist(observer) == false);
}

/**
 * @tc.number: HandleNotifyFocusChanged_0100
 * @tc.desc: AC-4: HandleNotifyFocusChanged delivers callback with correct data to registered observer.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, HandleNotifyFocusChanged_0100, TestSize.Level1)
{
    auto observer = sptr<MockFocusStateObserver>(new MockFocusStateObserver());
    ASSERT_NE(observer, nullptr);
    manager_->RegisterFocusStateObserver(observer);

    FocusStateData data;
    data.focusedUid = 10001;
    data.unfocusedUid = 10002;
    data.focusedBundleName = "com.example.focused";
    data.unfocusedBundleName = "com.example.unfocused";
    manager_->HandleNotifyFocusChanged(data);

    EXPECT_TRUE(observer->called_);
    EXPECT_EQ(observer->receivedData_.focusedUid, 10001);
    EXPECT_EQ(observer->receivedData_.unfocusedUid, 10002);
    EXPECT_EQ(observer->receivedData_.focusedBundleName, "com.example.focused");
    EXPECT_EQ(observer->receivedData_.unfocusedBundleName, "com.example.unfocused");
    manager_->UnregisterFocusStateObserver(observer);
}

/**
 * @tc.number: HandleNotifyFocusChanged_0200
 * @tc.desc: AC-4: HandleNotifyFocusChanged with no observers does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, HandleNotifyFocusChanged_0200, TestSize.Level1)
{
    FocusStateData data;
    data.focusedUid = 10001;
    data.unfocusedUid = 10002;
    EXPECT_TRUE(manager_->focusStateObserverMap_.empty());
    manager_->HandleNotifyFocusChanged(data);
    EXPECT_TRUE(manager_->focusStateObserverMap_.empty());
}

/**
 * @tc.number: HandleNotifyFocusChanged_0300
 * @tc.desc: AC-17: HandleNotifyFocusChanged with unfocusedUid=-1 delivers with empty bundleName.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, HandleNotifyFocusChanged_0300, TestSize.Level1)
{
    auto observer = sptr<MockFocusStateObserver>(new MockFocusStateObserver());
    ASSERT_NE(observer, nullptr);
    manager_->RegisterFocusStateObserver(observer);

    FocusStateData data;
    data.focusedUid = 10001;
    data.unfocusedUid = -1;
    data.focusedBundleName = "com.example.first";
    data.unfocusedBundleName = "";
    manager_->HandleNotifyFocusChanged(data);

    EXPECT_TRUE(observer->called_);
    EXPECT_EQ(observer->receivedData_.unfocusedUid, -1);
    EXPECT_TRUE(observer->receivedData_.unfocusedBundleName.empty());
    manager_->UnregisterFocusStateObserver(observer);
}

/**
 * @tc.number: RegisterFocusStateObserver_0300
 * @tc.desc: AC-14: same observer registered twice returns error (duplicate detection).
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, RegisterFocusStateObserver_0300, TestSize.Level1)
{
    auto observer = sptr<MockFocusStateObserver>(new MockFocusStateObserver());
    ASSERT_NE(observer, nullptr);
    auto ret1 = manager_->RegisterFocusStateObserver(observer);
    EXPECT_EQ(ret1, ERR_OK);
    auto ret2 = manager_->RegisterFocusStateObserver(observer);
    EXPECT_NE(ret2, ERR_OK);
    manager_->UnregisterFocusStateObserver(observer);
}

/**
 * @tc.number: IsFocusStateObserverExist_0100
 * @tc.desc: IsFocusStateObserverExist returns false for unregistered observer.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, IsFocusStateObserverExist_0100, TestSize.Level1)
{
    auto observer = sptr<MockFocusStateObserver>(new MockFocusStateObserver());
    ASSERT_NE(observer, nullptr);
    EXPECT_FALSE(manager_->IsFocusStateObserverExist(observer));
}

/**
 * @tc.number: IsFocusStateObserverExist_0200
 * @tc.desc: IsFocusStateObserverExist returns true after registration.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, IsFocusStateObserverExist_0200, TestSize.Level1)
{
    auto observer = sptr<MockFocusStateObserver>(new MockFocusStateObserver());
    ASSERT_NE(observer, nullptr);
    manager_->RegisterFocusStateObserver(observer);
    EXPECT_TRUE(manager_->IsFocusStateObserverExist(observer));
    manager_->UnregisterFocusStateObserver(observer);
}

/**
 * @tc.number: OnObserverDied_0100
 * @tc.desc: AC-8: OnObserverDied removes dead observer from map.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, OnObserverDied_0100, TestSize.Level1)
{
    auto observer = sptr<MockFocusStateObserver>(new MockFocusStateObserver());
    ASSERT_NE(observer, nullptr);
    manager_->RegisterFocusStateObserver(observer);
    EXPECT_TRUE(manager_->IsFocusStateObserverExist(observer));

    wptr<IRemoteObject> remote = observer->AsObject();
    manager_->OnObserverDied(remote, ObserverType::FOCUS_STATE_OBSERVER);

    EXPECT_FALSE(manager_->IsFocusStateObserverExist(observer));
}

/**
 * @tc.number: OnObserverDied_0200
 * @tc.desc: AC-8: OnObserverDied does not affect other observers.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, OnObserverDied_0200, TestSize.Level1)
{
    auto observer1 = sptr<MockFocusStateObserver>(new MockFocusStateObserver());
    auto observer2 = sptr<MockFocusStateObserver>(new MockFocusStateObserver());
    ASSERT_NE(observer1, nullptr);
    ASSERT_NE(observer2, nullptr);
    manager_->RegisterFocusStateObserver(observer1);
    manager_->RegisterFocusStateObserver(observer2);
    EXPECT_TRUE(manager_->IsFocusStateObserverExist(observer1));
    EXPECT_TRUE(manager_->IsFocusStateObserverExist(observer2));

    wptr<IRemoteObject> remote = observer1->AsObject();
    manager_->OnObserverDied(remote, ObserverType::FOCUS_STATE_OBSERVER);

    EXPECT_FALSE(manager_->IsFocusStateObserverExist(observer1));
    EXPECT_TRUE(manager_->IsFocusStateObserverExist(observer2));
    manager_->UnregisterFocusStateObserver(observer2);
}

/**
 * @tc.number: RegisterFocusStateObserver_0400
 * @tc.desc: AC-14: 11th observer from same app returns ERR_REGISTERED_FOCUS_OBSERVER_EXCEED_LIMIT.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, RegisterFocusStateObserver_0400, TestSize.Level1)
{
    std::lock_guard lockRegister(manager_->focusStateObserverLock_);
    manager_->focusStateObserverMap_.clear();
    std::vector<sptr<MockFocusStateObserver>> observers;
    for (int32_t i = 0; i < TEST_MAX_FOCUS_OBSERVER_PER_APP; ++i) {
        auto obs = sptr<MockFocusStateObserver>(new MockFocusStateObserver());
        ASSERT_NE(obs, nullptr);
        auto ret = manager_->RegisterFocusStateObserver(obs);
        EXPECT_EQ(ret, ERR_OK) << "register " << i << " failed";
        observers.push_back(obs);
    }
    auto obs11 = sptr<MockFocusStateObserver>(new MockFocusStateObserver());
    ASSERT_NE(obs11, nullptr);
    auto ret = manager_->RegisterFocusStateObserver(obs11);
    EXPECT_EQ(ret, TEST_ERR_FOCUS_OBSERVER_EXCEED_LIMIT);
    for (const auto &obs : observers) {
        manager_->UnregisterFocusStateObserver(obs);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
