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
#include "focused_application_info.h"
#include "focused_application_observer_stub.h"
#include "app_mgr_service_inner.h"
#include "app_state_observer_manager.h"
#include "mock_ability_token.h"
#include "mock_app_scheduler.h"
#include "mock_bundle_manager.h"
#include "mock_native_token.h"
#include "mock_my_flag.h"
#include "mock_permission_verification.h"
#include "mock_task_handler_wrap.h"
#undef private
#undef protected

#ifdef SUPPORT_SCREEN
#include "focus_change_info.h"
#endif

using namespace testing;
using namespace testing::ext;
namespace OHOS {
namespace AppExecFwk {
using OHOS::AAFwk::MyFlag;

namespace {
constexpr int32_t FOCUSED_UID = 10001;
constexpr int32_t UNFOCUSED_UID = 10002;
constexpr int32_t SAME_UID = 10001;
constexpr int32_t FIRST_FOCUS_UNFOCUSED_UID = -1;
constexpr int32_t INVALID_UID = -1;
constexpr int64_t MAIN_DISPLAY_ID = 0;
constexpr int64_t NON_MAIN_DISPLAY_ID = 1;
constexpr int64_t INVALID_DISPLAY_ID = -1;
const std::string TEST_BUNDLE_NAME = "com.test.focused";
}  // namespace

class TestFocusedApplicationObserver : public FocusedApplicationObserverStub {
public:
    TestFocusedApplicationObserver() = default;
    ~TestFocusedApplicationObserver() override = default;

    void OnFocusedApplicationChanged(const FocusedApplicationInfo &focusedApplicationInfo) override
    {
        called_ = true;
        focusedUid_ = focusedApplicationInfo.focusedUid;
        focusedBundleName_ = focusedApplicationInfo.focusedBundleName;
    }

    bool IsCalled() const { return called_; }
    int32_t GetFocusedUid() const { return focusedUid_; }
    const std::string &GetFocusedBundleName() const { return focusedBundleName_; }

    void Reset()
    {
        called_ = false;
        focusedUid_ = -1;
        focusedBundleName_.clear();
    }

private:
    bool called_ = false;
    int32_t focusedUid_ = -1;
    std::string focusedBundleName_;
};

class AppMgrServiceInnerFocusedAppTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;

    std::shared_ptr<AppMgrServiceInner> inner_;
    std::shared_ptr<AppRunningRecord> appRecord_;
};

void AppMgrServiceInnerFocusedAppTest::SetUpTestCase()
{
    MockNativeToken::SetNativeToken();
}

void AppMgrServiceInnerFocusedAppTest::TearDownTestCase() {}

void AppMgrServiceInnerFocusedAppTest::SetUp()
{
    MyFlag::flag_ = 1;
    inner_ = std::make_shared<AppMgrServiceInner>();
    ASSERT_NE(inner_, nullptr);
    ApplicationInfo appInfo;
    appInfo.bundleName = TEST_BUNDLE_NAME;
    appRecord_ = std::make_shared<AppRunningRecord>(std::make_shared<ApplicationInfo>(appInfo), 0, "process");
    ASSERT_NE(appRecord_, nullptr);
}

void AppMgrServiceInnerFocusedAppTest::TearDown()
{
    auto manager = DelayedSingleton<AppStateObserverManager>::GetInstance();
    if (manager) {
        std::lock_guard<ffrt::mutex> lock(manager->focusedApplicationObserverLock_);
        manager->focusedApplicationObserverMap_.clear();
    }
    appRecord_.reset();
    inner_.reset();
    MyFlag::flag_ = 1;
}

#ifdef SUPPORT_SCREEN
/**
 * @tc.number: NotifyFocusedApplicationChanged_0100
 * @tc.desc: AC-4 cross-app focus switch on main display, verify no crash. Also covers AC-16.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusedAppTest, NotifyFocusedApplicationChanged_0100, TestSize.Level1)
{
    EXPECT_NE(inner_, nullptr);
    inner_->NotifyFocusedApplicationChanged(FOCUSED_UID, UNFOCUSED_UID, MAIN_DISPLAY_ID, appRecord_);
}

/**
 * @tc.number: NotifyFocusedApplicationChanged_0200
 * @tc.desc: AC-5 same uid focus change, verify no crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusedAppTest, NotifyFocusedApplicationChanged_0200, TestSize.Level1)
{
    EXPECT_NE(inner_, nullptr);
    inner_->NotifyFocusedApplicationChanged(SAME_UID, SAME_UID, MAIN_DISPLAY_ID, appRecord_);
}

/**
 * @tc.number: NotifyFocusedApplicationChanged_0300
 * @tc.desc: AC-7 non-main display focus change, verify no crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusedAppTest, NotifyFocusedApplicationChanged_0300, TestSize.Level1)
{
    EXPECT_NE(inner_, nullptr);
    inner_->NotifyFocusedApplicationChanged(FOCUSED_UID, UNFOCUSED_UID, NON_MAIN_DISPLAY_ID, appRecord_);
}

/**
 * @tc.number: NotifyFocusedApplicationChanged_0400
 * @tc.desc: AC-9 null appRecord, verify no crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusedAppTest, NotifyFocusedApplicationChanged_0400, TestSize.Level1)
{
    EXPECT_NE(inner_, nullptr);
    inner_->NotifyFocusedApplicationChanged(FOCUSED_UID, UNFOCUSED_UID, MAIN_DISPLAY_ID, nullptr);
}

/**
 * @tc.number: NotifyFocusedApplicationChanged_0500
 * @tc.desc: AC-15 invalid displayId=-1, verify no crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusedAppTest, NotifyFocusedApplicationChanged_0500, TestSize.Level1)
{
    EXPECT_NE(inner_, nullptr);
    inner_->NotifyFocusedApplicationChanged(FOCUSED_UID, UNFOCUSED_UID, INVALID_DISPLAY_ID, appRecord_);
}

/**
 * @tc.number: NotifyFocusedApplicationChanged_0600
 * @tc.desc: AC-16 focus switching to main display, verify no crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusedAppTest, NotifyFocusedApplicationChanged_0600, TestSize.Level1)
{
    EXPECT_NE(inner_, nullptr);
    inner_->NotifyFocusedApplicationChanged(FOCUSED_UID, UNFOCUSED_UID, MAIN_DISPLAY_ID, appRecord_);
}

/**
 * @tc.number: NotifyFocusedApplicationChanged_0700
 * @tc.desc: AC-17 first focus with unfocusedUid=-1, verify no crash.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusedAppTest, NotifyFocusedApplicationChanged_0700, TestSize.Level1)
{
    EXPECT_NE(inner_, nullptr);
    inner_->NotifyFocusedApplicationChanged(FOCUSED_UID, FIRST_FOCUS_UNFOCUSED_UID, MAIN_DISPLAY_ID, appRecord_);
}

/**
 * @tc.number: NotifyFocusedApplicationChanged_0800
 * @tc.desc: AC-5/7/15 guard cases do not notify registered observer.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusedAppTest, NotifyFocusedApplicationChanged_0800, TestSize.Level1)
{
    EXPECT_NE(inner_, nullptr);
    auto observer = sptr<TestFocusedApplicationObserver>(new TestFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    ASSERT_EQ(ERR_OK, inner_->RegisterFocusedApplicationObserver(observer));

    inner_->NotifyFocusedApplicationChanged(SAME_UID, SAME_UID, MAIN_DISPLAY_ID, appRecord_);
    EXPECT_FALSE(observer->IsCalled());

    inner_->NotifyFocusedApplicationChanged(FOCUSED_UID, UNFOCUSED_UID, NON_MAIN_DISPLAY_ID, appRecord_);
    EXPECT_FALSE(observer->IsCalled());

    inner_->NotifyFocusedApplicationChanged(FOCUSED_UID, UNFOCUSED_UID, INVALID_DISPLAY_ID, appRecord_);
    EXPECT_FALSE(observer->IsCalled());

    inner_->UnregisterFocusedApplicationObserver(observer);
}

/**
 * @tc.number: HandleFocused_0100
 * @tc.desc: AC-9 HandleFocused with null appRecord, no crash, observer not notified.
 * @tc.type: FUNC
 */
HWTEST_F(AppMgrServiceInnerFocusedAppTest, HandleFocused_0100, TestSize.Level1)
{
    EXPECT_NE(inner_, nullptr);
    auto observer = sptr<TestFocusedApplicationObserver>(new TestFocusedApplicationObserver());
    ASSERT_NE(observer, nullptr);
    ASSERT_EQ(ERR_OK, inner_->RegisterFocusedApplicationObserver(observer));

    sptr<OHOS::Rosen::FocusChangeInfo> focusChangeInfo = new OHOS::Rosen::FocusChangeInfo();
    focusChangeInfo->pid_ = 9999;
    focusChangeInfo->uid_ = FOCUSED_UID;
    focusChangeInfo->displayId_ = static_cast<OHOS::Rosen::DisplayId>(MAIN_DISPLAY_ID);
    inner_->HandleFocused(focusChangeInfo);
    EXPECT_FALSE(observer->IsCalled());

    inner_->UnregisterFocusedApplicationObserver(observer);
}
#endif // SUPPORT_SCREEN

} // namespace AppExecFwk
} // namespace OHOS
