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
#include "focused_application_info.h"
#include "focused_application_observer_interface.h"
#include "focused_application_observer_stub.h"
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
constexpr int32_t TEST_MAX_FOCUSED_APP_OBSERVER_PER_APP = 10;
constexpr int32_t TEST_ERR_OBSERVER_NOT_EXIST = 16300004;
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

class MockFocusedApplicationObserver : public FocusedApplicationObserverStub {
public:
    MockFocusedApplicationObserver() = default;
    ~MockFocusedApplicationObserver() override = default;
    void OnFocusedApplicationChanged(const FocusedApplicationInfo &focusedApplicationInfo) override
    {
        receivedData_ = focusedApplicationInfo;
        called_ = true;
    }
    void Reset()
    {
        called_ = false;
        receivedData_ = FocusedApplicationInfo();
    }
    FocusedApplicationInfo receivedData_;
    bool called_ = false;
};

/**
 * @tc.number: RegisterFocusedApplicationObserver_0100
 * @tc.desc: AC-1: register null observer returns ERR_INVALID_VALUE.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, RegisterFocusedApplicationObserver_0100, TestSize.Level1)
{
    sptr<IFocusedApplicationObserver> observer = nullptr;
    auto ret = manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: RegisterFocusedApplicationObserver_0200
 * @tc.desc: AC-1: register valid observer succeeds and map contains it.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, RegisterFocusedApplicationObserver_0200, TestSize.Level1)
{
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    auto ret = manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_FALSE(manager_->focusedApplicationObserverMap_.empty());
    EXPECT_TRUE(manager_->IsFocusedApplicationObserverExist(observer));
    manager_->UnregisterFocusedApplicationObserver(observer);
    EXPECT_FALSE(manager_->IsFocusedApplicationObserverExist(observer));
}

/**
 * @tc.number: UnregisterFocusedApplicationObserver_0100
 * @tc.desc: AC-6: unregister null observer returns ERR_INVALID_VALUE.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, UnregisterFocusedApplicationObserver_0100, TestSize.Level1)
{
    sptr<IFocusedApplicationObserver> observer = nullptr;
    auto ret = manager_->UnregisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ret, ERR_INVALID_VALUE);
}

/**
 * @tc.number: UnregisterFocusedApplicationObserver_0200
 * @tc.desc: AC-6: unregister valid observer succeeds and map is empty.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, UnregisterFocusedApplicationObserver_0200, TestSize.Level1)
{
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_FALSE(manager_->focusedApplicationObserverMap_.empty());
    auto ret = manager_->UnregisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ret, ERR_OK);
    EXPECT_TRUE(manager_->IsFocusedApplicationObserverExist(observer) == false);
}

/**
 * @tc.number: UnregisterFocusedApplicationObserver_0300
 * @tc.desc: AC-6: unregister a non-existent observer returns ERR_OBSERVER_NOT_EXIST.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, UnregisterFocusedApplicationObserver_0300, TestSize.Level1)
{
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    EXPECT_FALSE(manager_->IsFocusedApplicationObserverExist(observer));
    auto ret = manager_->UnregisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ret, TEST_ERR_OBSERVER_NOT_EXIST);
}

/**
 * @tc.number: HandleNotifyFocusedApplicationChanged_0100
 * @tc.desc: AC-4: HandleNotifyFocusedApplicationChanged delivers callback with correct data to registered observer.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, HandleNotifyFocusedApplicationChanged_0100, TestSize.Level1)
{
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    manager_->RegisterFocusedApplicationObserver(observer);

    FocusedApplicationInfo data;
    data.focusedUid = 10001;
    data.focusedBundleName = "com.example.focused";
    manager_->HandleNotifyFocusedApplicationChanged(data);

    EXPECT_TRUE(observer->called_);
    EXPECT_EQ(observer->receivedData_.focusedUid, 10001);
    EXPECT_EQ(observer->receivedData_.focusedBundleName, "com.example.focused");
    manager_->UnregisterFocusedApplicationObserver(observer);
}

/**
 * @tc.number: HandleNotifyFocusedApplicationChanged_0200
 * @tc.desc: AC-4: HandleNotifyFocusedApplicationChanged with no observers does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, HandleNotifyFocusedApplicationChanged_0200, TestSize.Level1)
{
    FocusedApplicationInfo data;
    data.focusedUid = 10001;
    EXPECT_TRUE(manager_->focusedApplicationObserverMap_.empty());
    manager_->HandleNotifyFocusedApplicationChanged(data);
    EXPECT_TRUE(manager_->focusedApplicationObserverMap_.empty());
}

/**
 * @tc.number: HandleNotifyFocusedApplicationChanged_0300
 * @tc.desc: AC-17: HandleNotifyFocusedApplicationChanged delivers with empty bundleName.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, HandleNotifyFocusedApplicationChanged_0300, TestSize.Level1)
{
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    manager_->RegisterFocusedApplicationObserver(observer);

    FocusedApplicationInfo data;
    data.focusedUid = 10001;
    data.focusedBundleName = "";
    manager_->HandleNotifyFocusedApplicationChanged(data);

    EXPECT_TRUE(observer->called_);
    EXPECT_TRUE(observer->receivedData_.focusedBundleName.empty());
    manager_->UnregisterFocusedApplicationObserver(observer);
}

/**
 * @tc.number: RegisterFocusedApplicationObserver_0300
 * @tc.desc: AC-14: same observer registered twice returns error (duplicate detection).
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, RegisterFocusedApplicationObserver_0300, TestSize.Level1)
{
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    auto ret1 = manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_EQ(ret1, ERR_OK);
    auto ret2 = manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_NE(ret2, ERR_OK);
    manager_->UnregisterFocusedApplicationObserver(observer);
}

/**
 * @tc.number: IsFocusedApplicationObserverExist_0100
 * @tc.desc: IsFocusedApplicationObserverExist returns false for unregistered observer.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, IsFocusedApplicationObserverExist_0100, TestSize.Level1)
{
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    EXPECT_FALSE(manager_->IsFocusedApplicationObserverExist(observer));
}

/**
 * @tc.number: IsFocusedApplicationObserverExist_0200
 * @tc.desc: IsFocusedApplicationObserverExist returns true after registration.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, IsFocusedApplicationObserverExist_0200, TestSize.Level1)
{
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_TRUE(manager_->IsFocusedApplicationObserverExist(observer));
    manager_->UnregisterFocusedApplicationObserver(observer);
}

/**
 * @tc.number: OnObserverDied_0100
 * @tc.desc: AC-8: OnObserverDied removes dead observer from map.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, OnObserverDied_0100, TestSize.Level1)
{
    auto observer = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    manager_->RegisterFocusedApplicationObserver(observer);
    EXPECT_TRUE(manager_->IsFocusedApplicationObserverExist(observer));

    wptr<IRemoteObject> remote = observer->AsObject();
    manager_->OnObserverDied(remote, ObserverType::FOCUSED_APPLICATION_OBSERVER);

    EXPECT_FALSE(manager_->IsFocusedApplicationObserverExist(observer));
}

/**
 * @tc.number: OnObserverDied_0200
 * @tc.desc: AC-8: OnObserverDied does not affect other observers.
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, OnObserverDied_0200, TestSize.Level1)
{
    auto observer1 = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    auto observer2 = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
    ASSERT_NE(observer1, nullptr);
    ASSERT_NE(observer2, nullptr);
    manager_->RegisterFocusedApplicationObserver(observer1);
    manager_->RegisterFocusedApplicationObserver(observer2);
    EXPECT_TRUE(manager_->IsFocusedApplicationObserverExist(observer1));
    EXPECT_TRUE(manager_->IsFocusedApplicationObserverExist(observer2));

    wptr<IRemoteObject> remote = observer1->AsObject();
    manager_->OnObserverDied(remote, ObserverType::FOCUSED_APPLICATION_OBSERVER);

    EXPECT_FALSE(manager_->IsFocusedApplicationObserverExist(observer1));
    EXPECT_TRUE(manager_->IsFocusedApplicationObserverExist(observer2));
    manager_->UnregisterFocusedApplicationObserver(observer2);
}

/**
 * @tc.number: RegisterFocusedApplicationObserver_0400
 * @tc.desc: AC-14: registering more than the former per-app limit now succeeds (no upper limit).
 * @tc.type: FUNC
 */
HWTEST_F(AppStateObserverManagerFocusTest, RegisterFocusedApplicationObserver_0400, TestSize.Level1)
{
    std::vector<sptr<MockFocusedApplicationObserver>> observers;
    for (int32_t i = 0; i < TEST_MAX_FOCUSED_APP_OBSERVER_PER_APP + 1; ++i) {
        auto obs = sptr<MockFocusedApplicationObserver>(new MockFocusedApplicationObserver());
        ASSERT_NE(obs, nullptr);
        auto ret = manager_->RegisterFocusedApplicationObserver(obs);
        EXPECT_EQ(ret, ERR_OK) << "register " << i << " failed";
        observers.push_back(obs);
    }
    for (const auto &obs : observers) {
        manager_->UnregisterFocusedApplicationObserver(obs);
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
