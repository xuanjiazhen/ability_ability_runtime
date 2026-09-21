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
#include "app_mgr_service_inner.h"
#include "app_state_observer_manager.h"
#include "focused_application_info.h"
#include "focused_application_observer_stub.h"
#undef private
#undef protected
#include "hilog_tag_wrapper.h"
#include "mock_ability_token.h"
#include "mock_app_scheduler.h"
#include "mock_bundle_manager.h"
#include "mock_native_token.h"
#include "mock_permission_verification.h"
#include "mock_task_handler_wrap.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceInnerFocusTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<AppMgrServiceInner> inner_;
    sptr<FocusedApplicationObserverStub> testObserver_;
    std::shared_ptr<AppStateObserverManager> mgr_;
};

void AppMgrServiceInnerFocusTest::SetUpTestCase(void) {}
void AppMgrServiceInnerFocusTest::TearDownTestCase(void) {}
void AppMgrServiceInnerFocusTest::SetUp()
{
    inner_ = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner_, nullptr);
    mgr_ = DelayedSingleton<AppStateObserverManager>::GetInstance();
    ASSERT_NE(mgr_, nullptr);
    if (mgr_->handler_ == nullptr) {
        mgr_->handler_ = AAFwk::TaskHandlerWrap::Create();
    }
    testObserver_ = new FocusedApplicationObserverStub();
}
void AppMgrServiceInnerFocusTest::TearDown()
{
    if (testObserver_ != nullptr) {
        mgr_->UnregisterFocusedApplicationObserver(testObserver_);
    }
    inner_ = nullptr;
    testObserver_ = nullptr;
}

class TestFocusedApplicationObserver : public FocusedApplicationObserverStub {
public:
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
 * @tc.number: NotifyFocusedApplicationChanged_0100
 * @tc.desc: AC-4: cross-app switch on main display does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusTest, NotifyFocusedApplicationChanged_0100, TestSize.Level1)
{
    auto appRecord = std::make_shared<AppRunningRecord>(
        ApplicationInfo(), 0, std::shared_ptr<AbilityRunningRecord>(nullptr));
    ASSERT_NE(appRecord, nullptr);
    inner_->NotifyFocusedApplicationChanged(10001, 10002, 0, appRecord);
}

/**
 * @tc.number: NotifyFocusedApplicationChanged_0200
 * @tc.desc: AC-5: same uid focus change does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusTest, NotifyFocusedApplicationChanged_0200, TestSize.Level1)
{
    auto appRecord = std::make_shared<AppRunningRecord>(
        ApplicationInfo(), 0, std::shared_ptr<AbilityRunningRecord>(nullptr));
    ASSERT_NE(appRecord, nullptr);
    inner_->NotifyFocusedApplicationChanged(10001, 10001, 0, appRecord);
}

/**
 * @tc.number: NotifyFocusedApplicationChanged_0300
 * @tc.desc: AC-7: non-main display does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusTest, NotifyFocusedApplicationChanged_0300, TestSize.Level1)
{
    auto appRecord = std::make_shared<AppRunningRecord>(
        ApplicationInfo(), 0, std::shared_ptr<AbilityRunningRecord>(nullptr));
    ASSERT_NE(appRecord, nullptr);
    inner_->NotifyFocusedApplicationChanged(10001, 10002, 1, appRecord);
}

/**
 * @tc.number: NotifyFocusedApplicationChanged_0400
 * @tc.desc: AC-9: null appRecord does not crash (NotifyFocusedApplicationChanged still proceeds).
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusTest, NotifyFocusedApplicationChanged_0400, TestSize.Level1)
{
    std::shared_ptr<AppRunningRecord> appRecord = nullptr;
    inner_->NotifyFocusedApplicationChanged(10001, 10002, 0, appRecord);
}

/**
 * @tc.number: NotifyFocusedApplicationChanged_0500
 * @tc.desc: AC-15: invalid displayId (negative) does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusTest, NotifyFocusedApplicationChanged_0500, TestSize.Level1)
{
    auto appRecord = std::make_shared<AppRunningRecord>(
        ApplicationInfo(), 0, std::shared_ptr<AbilityRunningRecord>(nullptr));
    ASSERT_NE(appRecord, nullptr);
    inner_->NotifyFocusedApplicationChanged(10001, 10002, -1, appRecord);
}

/**
 * @tc.number: NotifyFocusedApplicationChanged_0600
 * @tc.desc: AC-16: focus switching to main display does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusTest, NotifyFocusedApplicationChanged_0600, TestSize.Level1)
{
    auto appRecord = std::make_shared<AppRunningRecord>(
        ApplicationInfo(), 0, std::shared_ptr<AbilityRunningRecord>(nullptr));
    ASSERT_NE(appRecord, nullptr);
    inner_->NotifyFocusedApplicationChanged(10001, 10002, 0, appRecord);
}

/**
 * @tc.number: NotifyFocusedApplicationChanged_0700
 * @tc.desc: AC-17: first focus with no previous focused app does not crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusTest, NotifyFocusedApplicationChanged_0700, TestSize.Level1)
{
    auto appRecord = std::make_shared<AppRunningRecord>(
        ApplicationInfo(), 0, std::shared_ptr<AbilityRunningRecord>(nullptr));
    ASSERT_NE(appRecord, nullptr);
    inner_->NotifyFocusedApplicationChanged(10001, -1, 0, appRecord);
}

/**
 * @tc.number: GetBundleNameByUid_0100
 * @tc.desc: GetBundleNameByUid with invalid uid returns empty.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusTest, GetBundleNameByUid_0100, TestSize.Level1)
{
    std::string result = inner_->GetBundleNameByUid(-1);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: GetBundleNameByUid_0200
 * @tc.desc: GetBundleNameByUid with null appRunningManager returns empty.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusTest, GetBundleNameByUid_0200, TestSize.Level1)
{
    inner_->appRunningManager_ = nullptr;
    std::string result = inner_->GetBundleNameByUid(10001);
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.number: HandleNotifyFocusedApplicationChanged_0100
 * @tc.desc: AC-4: HandleNotifyFocusedApplicationChanged delivers callback with correct data.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusTest, HandleNotifyFocusedApplicationChanged_0100, TestSize.Level1)
{
    auto observer = sptr<TestFocusedApplicationObserver>(new TestFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    mgr_->RegisterFocusedApplicationObserver(observer);

    FocusedApplicationInfo data;
    data.focusedUid = 10001;
    data.focusedBundleName = "com.example.focused";
    mgr_->HandleNotifyFocusedApplicationChanged(data);

    EXPECT_TRUE(observer->called_);
    EXPECT_EQ(observer->receivedData_.focusedUid, 10001);
    EXPECT_EQ(observer->receivedData_.focusedBundleName, "com.example.focused");
    mgr_->UnregisterFocusedApplicationObserver(observer);
}

/**
 * @tc.number: HandleNotifyFocusedApplicationChanged_0200
 * @tc.desc: AC-17: HandleNotifyFocusedApplicationChanged delivers with empty bundleName.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusTest, HandleNotifyFocusedApplicationChanged_0200, TestSize.Level1)
{
    auto observer = sptr<TestFocusedApplicationObserver>(new TestFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    mgr_->RegisterFocusedApplicationObserver(observer);

    FocusedApplicationInfo data;
    data.focusedUid = 10001;
    data.focusedBundleName = "";
    mgr_->HandleNotifyFocusedApplicationChanged(data);

    EXPECT_TRUE(observer->called_);
    EXPECT_TRUE(observer->receivedData_.focusedBundleName.empty());
    mgr_->UnregisterFocusedApplicationObserver(observer);
}
}  // namespace AppExecFwk
}  // namespace OHOS
