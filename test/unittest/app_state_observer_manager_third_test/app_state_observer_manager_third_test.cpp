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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "app_state_observer_manager.h"
#include "image_process_state_observer_stub.h"
#include "iremote_broker.h"
#include "mock_my_flag.h"
using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AppExecFwk {
namespace {
constexpr int OBSERVER_SINGLE_COUNT_LOG = 40;
constexpr int OBSERVER_SINGLE_STEP_LOG = 10;
}

class MockTaskHandlerWrap : public AAFwk::TaskHandlerWrap {
public:
    explicit MockTaskHandlerWrap(const std::string& queueName = "") : TaskHandlerWrap(queueName) {};

    virtual ~MockTaskHandlerWrap() {};
    std::shared_ptr<AAFwk::InnerTaskHandle> SubmitTaskInner(
        std::function<void()>&& task, const AAFwk::TaskAttribute& taskAttr) override
        {
            task();
            return nullptr;
        }
    bool CancelTaskInner(const std::shared_ptr<AAFwk::InnerTaskHandle>& taskHandle) override
    {
        return false;
    }

    void WaitTaskInner(const std::shared_ptr<AAFwk::InnerTaskHandle>& taskHandle) override
    {
        return;
    }

    uint64_t GetTaskCount() override
    {
        return tasks_.size();
    }
};

class MockImageProcessStateObserverStub : public ImageProcessStateObserverStub {
public:
    MockImageProcessStateObserverStub() = default;
    virtual ~MockImageProcessStateObserverStub() = default;

    MOCK_METHOD1(OnImageProcessStateChanged, void(const ImageProcessStateData& imageProcessStateData));
    MOCK_METHOD2(OnForkAllWorkProcessFailed, void(const ImageProcessStateData& imageProcessStateData, int32_t));
};

class AppStateObserverManagerThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AppRunningRecord> MockAppRecord();
};

void AppStateObserverManagerThirdTest::SetUpTestCase()
{}

void AppStateObserverManagerThirdTest::TearDownTestCase()
{}

void AppStateObserverManagerThirdTest::SetUp()
{}

void AppStateObserverManagerThirdTest::TearDown()
{}

std::shared_ptr<AppRunningRecord> AppStateObserverManagerThirdTest::MockAppRecord()
{
    ApplicationInfo appInfo;
    appInfo.accessTokenId = 1;
    std::shared_ptr<ApplicationInfo> info = std::make_shared<ApplicationInfo>(appInfo);
    info->accessTokenId = 1;
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(info, 0, "process");
    std::shared_ptr<PriorityObject> priorityObject = std::make_shared<PriorityObject>();
    priorityObject->SetPid(1);
    appRecord->priorityObject_ = priorityObject;
    appRecord->SetUid(1);
    appRecord->SetState(ApplicationState::APP_STATE_CREATE);
    appRecord->SetContinuousTaskAppState(false);
    appRecord->SetKeepAliveEnableState(false);
    appRecord->SetKeepAliveDkv(false);
    appRecord->SetEmptyKeepAliveAppState(false);
    appRecord->SetRequestProcCode(1);
    appRecord->isFocused_ = false;
    return appRecord;
}

/*
 * Feature: AppStateObserverManager
 * Function: RegisterImageProcessStateObserver
 * FunctionPoints: AppStateObserverManager RegisterImageProcessStateObserver
 * CaseDescription: RegisterImageProcessStateObserver_ShouldReturnPermissionDeniedWhenNotPermitted
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    RegisterImageProcessStateObserver_ShouldReturnPermissionDeniedWhenNotPermitted, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    AAFwk::MyFlag::flag_ = ERR_PERMISSION_DENIED;
    sptr<MockImageProcessStateObserverStub> observer = nullptr;
    EXPECT_EQ(manager->RegisterImageProcessStateObserver(observer), ERR_PERMISSION_DENIED);
}

/*
 * Feature: AppStateObserverManager
 * Function: RegisterImageProcessStateObserver
 * FunctionPoints: AppStateObserverManager RegisterImageProcessStateObserver
 * CaseDescription: RegisterImageProcessStateObserver_ShouldReturnInvalidValueWhenObserverIsNullptr
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    RegisterImageProcessStateObserver_ShouldReturnInvalidValueWhenObserverIsNullptr, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    AAFwk::MyFlag::flag_ = 0;
    sptr<MockImageProcessStateObserverStub> observer = nullptr;
    EXPECT_EQ(manager->RegisterImageProcessStateObserver(observer), ERR_INVALID_VALUE);
}

/*
 * Feature: AppStateObserverManager
 * Function: RegisterImageProcessStateObserver
 * FunctionPoints: AppStateObserverManager RegisterImageProcessStateObserver
 * CaseDescription: RegisterImageProcessStateObserver_ShouldReturnInvalidValueWhenObserverExist
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    RegisterImageProcessStateObserver_ShouldReturnInvalidValueWhenObserverExist, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    AAFwk::MyFlag::flag_ = 0;
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    ASSERT_NE(manager, nullptr);
    manager->imageProcessStateObserverMap_.emplace(observer, 0);
    EXPECT_EQ(manager->RegisterImageProcessStateObserver(observer), ERR_INVALID_VALUE);
}

/*
 * Feature: AppStateObserverManager
 * Function: RegisterImageProcessStateObserver
 * FunctionPoints: AppStateObserverManager RegisterImageProcessStateObserver
 * CaseDescription: RegisterImageProcessStateObserver_ShouldReturnErrOkWhenObserverNotExist
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    RegisterImageProcessStateObserver_ShouldReturnErrOkWhenObserverNotExist, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    AAFwk::MyFlag::flag_ = 0;
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    EXPECT_EQ(manager->RegisterImageProcessStateObserver(observer), ERR_OK);
}

/*
 * Feature: AppStateObserverManager
 * Function: RegisterImageProcessStateObserver
 * FunctionPoints: AppStateObserverManager RegisterImageProcessStateObserver
 * CaseDescription: RegisterImageProcessStateObserver_ShouldReturnMultiErrOkWhenObserverNotExist
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    RegisterImageProcessStateObserver_ShouldReturnMultiErrOkWhenObserverNotExist, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    AAFwk::MyFlag::flag_ = 0;
    sptr<MockImageProcessStateObserverStub> observer = nullptr;
    for (int32_t i = 0; i <= OBSERVER_SINGLE_COUNT_LOG + OBSERVER_SINGLE_STEP_LOG; i++) {
        observer = new (std::nothrow) MockImageProcessStateObserverStub();
        EXPECT_EQ(manager->RegisterImageProcessStateObserver(observer), ERR_OK);
    }
}

/*
 * Feature: UnregisterImageProcessStateObserver
 * Function: UnregisterImageProcessStateObserver
 * FunctionPoints: AppStateObserverManager UnregisterImageProcessStateObserver
 * CaseDescription: UnregisterImageProcessStateObserver_ShouldReturnInvalidValueWhenObserverIsNullptr
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    RegisterImageProcessStateObserver_ShouldReturnInvalidValueWhenObserverIsNullptr, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    sptr<MockImageProcessStateObserverStub> observer = nullptr;
    EXPECT_EQ(manager->UnregisterImageProcessStateObserver(observer), ERR_INVALID_VALUE);
}

/*
 * Feature: AppStateObserverManager
 * Function: UnregisterImageProcessStateObserver
 * FunctionPoints: AppStateObserverManager UnregisterImageProcessStateObserver
 * CaseDescription: UnregisterImageProcessStateObserver_ShouldReturnPermissionDeniedWhenNotPermitted
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    UnregisterImageProcessStateObserver_ShouldReturnPermissionDeniedWhenNotPermitted, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    AAFwk::MyFlag::flag_ = ERR_PERMISSION_DENIED;
    sptr<MockImageProcessStateObserverStub> observer = nullptr;
    EXPECT_EQ(manager->UnregisterImageProcessStateObserver(observer), ERR_PERMISSION_DENIED);
}

/*
 * Feature: AppStateObserverManager
 * Function: UnregisterImageProcessStateObserver
 * FunctionPoints: AppStateObserverManager UnregisterImageProcessStateObserver
 * CaseDescription: UnregisterImageProcessStateObserver_ShouldReturnErrOkWhenObserverExist
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    UnregisterImageProcessStateObserver_ShouldReturnErrOkWhenObserverExist, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    AAFwk::MyFlag::flag_ = 0;
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->RegisterImageProcessStateObserver(observer), ERR_OK);
    EXPECT_EQ(manager->UnregisterImageProcessStateObserver(observer), ERR_OK);
    EXPECT_EQ(manager->RegisterImageProcessStateObserver(observer), ERR_OK);
    EXPECT_EQ(manager->UnregisterImageProcessStateObserver(observer), ERR_OK);
}

/*
 * Feature: AppStateObserverManager
 * Function: UnregisterImageProcessStateObserver
 * FunctionPoints: AppStateObserverManager UnregisterImageProcessStateObserver
 * CaseDescription: UnregisterImageProcessStateObserver_ShouldReturnInvalidValueWhenObserverNotExist
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    UnregisterImageProcessStateObserver_ShouldReturnInvalidValueWhenObserverNotExist, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    AAFwk::MyFlag::flag_ = 0;
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    EXPECT_EQ(manager->UnregisterImageProcessStateObserver(observer), ERR_INVALID_VALUE);
}

/*
 * Feature: AppStateObserverManager
 * Function: WrapProcessData
 * FunctionPoints: AppStateObserverManager WrapProcessData
 * CaseDescription: WrapProcessData_ShouldReturnInputWindowFocus
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    WrapProcessData_ShouldReturnInputWindowFocus, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    auto appRecord = MockAppRecord();
    bool isFromWindowFocusChanged = true;
    ImageProcessType type = ImageProcessType::WORK;
    appRecord->SetImageProcessType(type);
    ProcessData processData = manager->WrapProcessData(appRecord, isFromWindowFocusChanged);
    EXPECT_EQ(processData.isFromWindowFocusChanged, isFromWindowFocusChanged);
    EXPECT_EQ(processData.imageProcessType, static_cast<int32_t>(type));
}

/*
 * Feature: AppStateObserverManager
 * Function: IsImageProcessObserverExist
 * FunctionPoints: AppStateObserverManager IsImageProcessObserverExist
 * CaseDescription: IsImageProcessObserverExist_ShouldReturnFalseWhenObserverIsNullptr
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    IsImageProcessObserverExist_ShouldReturnFalseWhenObserverIsNullptr, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    sptr<MockImageProcessStateObserverStub> observer = nullptr;
    ASSERT_NE(manager, nullptr);
    EXPECT_FALSE(manager->IsImageProcessObserverExist(observer));
}

/*
 * Feature: AppStateObserverManager
 * Function: IsImageProcessObserverExist
 * FunctionPoints: AppStateObserverManager IsImageProcessObserverExist
 * CaseDescription: IsImageProcessObserverExist_ShouldReturnFalseWhenObserverIsNotExist
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    IsImageProcessObserverExist_ShouldReturnFalseWhenObserverIsNotExist, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    ASSERT_NE(manager, nullptr);
    EXPECT_FALSE(manager->IsImageProcessObserverExist(observer));
}

/*
 * Feature: AppStateObserverManager
 * Function: IsImageProcessObserverExist
 * FunctionPoints: AppStateObserverManager IsImageProcessObserverExist
 * CaseDescription: IsImageProcessObserverExist_ShouldReturnTrueWhenObserverIsNotExist
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    IsImageProcessObserverExist_ShouldReturnTrueWhenObserverIsNotExist, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    AAFwk::MyFlag::flag_ = 0;
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    ASSERT_NE(manager, nullptr);
    EXPECT_EQ(manager->RegisterImageProcessStateObserver(observer), ERR_OK);
    EXPECT_TRUE(manager->IsImageProcessObserverExist(observer));
}

/*
 * Feature: AppStateObserverManager
 * Function: OnObserverDied
 * FunctionPoints: AppStateObserverManager OnObserverDied
 * CaseDescription: OnObserverDied_ShouldUnregisterObserverWhenTypeIsIMAGE_PROCESS_STATE_OBSERVER
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    OnObserverDied_ShouldUnregisterObserverWhenTypeIsIMAGE_PROCESS_STATE_OBSERVER, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    AAFwk::MyFlag::flag_ = 0;
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    sptr<IRemoteObject> remoteObject = observer;
    wptr<IRemoteObject> remote(remoteObject);
    manager->imageProcessStateObserverMap_.emplace(observer, 0);
    EXPECT_EQ(manager->imageProcessStateObserverMap_.size(), 1);
    ObserverType type = ObserverType::IMAGE_PROCESS_STATE_OBSERVER;
    manager->OnObserverDied(remote, type);
    EXPECT_EQ(manager->imageProcessStateObserverMap_.size(), 0);
}

/*
 * Feature: AppStateObserverManager
 * Function: WrapImageProcessStateData
 * FunctionPoints: AppStateObserverManager WrapImageProcessStateData
 * CaseDescription: WrapImageProcessStateData_ShouldReturnImageProcessStateData
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    WrapImageProcessStateData_ShouldReturnImageProcessStateData, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto appRecord = MockAppRecord();
    int32_t imagePid = 100;
    ImageProcessState state = ImageProcessState::IMAGE_PROCESS_CREATE;
    ImageProcessStateData data = manager->WrapImageProcessStateData(imagePid, appRecord, state);
    EXPECT_EQ(data.imagePid, imagePid);
    EXPECT_EQ(data.state, static_cast<int32_t>(state));
    EXPECT_EQ(data.originalPid, appRecord->GetPid());
    EXPECT_EQ(data.uid, appRecord->GetUid());
    EXPECT_EQ(data.bundleName, appRecord->GetBundleName());
}

/*
 * Feature: AppStateObserverManager
 * Function: OnImageProcessStateChanged
 * FunctionPoints: AppStateObserverManager OnImageProcessStateChanged
 * CaseDescription: OnImageProcessStateChanged_ShouldNotExecImageProcessStateChangeCallbackWhenHandlerIsNullptr
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    OnImageProcessStateChanged_ShouldNotExecImageProcessStateChangeCallbackWhenHandlerIsNullptr, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    int32_t imagePid = 100;
    ImageProcessState state = ImageProcessState::IMAGE_PROCESS_CREATE;
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    manager->imageProcessStateObserverMap_.emplace(observer, 0);
    EXPECT_CALL(*observer, OnImageProcessStateChanged(_))Times(0);
    manager->OnImageProcessStateChanged(imagePid, nullptr, state);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnImageProcessStateChanged
 * FunctionPoints: AppStateObserverManager OnImageProcessStateChanged
 * CaseDescription: OnImageProcessStateChanged_ShouldNotExecImageProcessStateChangeCallbackWhenAppRecordIsNullptr
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    OnImageProcessStateChanged_ShouldNotExecImageProcessStateChangeCallbackWhenAppRecordIsNullptr, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto mockHandler = std::make_shared<MockTaskHandlerWrap>();
    manager->handler_ = mockHandler;
    ASSERT_NE(manager->handler_, nullptr);
    int32_t imagePid = 100;
    ImageProcessState state = ImageProcessState::IMAGE_PROCESS_CREATE;
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    manager->imageProcessStateObserverMap_.emplace(observer, 0);
    EXPECT_CALL(*observer, OnImageProcessStateChanged(_))Times(0);
    manager->OnImageProcessStateChanged(imagePid, nullptr, state);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnImageProcessStateChanged
 * FunctionPoints: AppStateObserverManager OnImageProcessStateChanged
 * CaseDescription: OnImageProcessStateChanged_ShouldNotExecImageProcessStateChangeCallbackWhenMapIsEmpty
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    OnImageProcessStateChanged_ShouldNotExecImageProcessStateChangeCallbackWhenMapIsEmpty, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto mockHandler = std::make_shared<MockTaskHandlerWrap>();
    manager->handler_ = mockHandler;
    ASSERT_NE(manager->handler_, nullptr);
    int32_t imagePid = 100;
    auto appRecord = MockAppRecord();
    ImageProcessState state = ImageProcessState::IMAGE_PROCESS_CREATE;
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    EXPECT_CALL(*observer, OnImageProcessStateChanged(_))Times(0);
    manager->OnImageProcessStateChanged(imagePid, appRecord, state);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnImageProcessStateChanged
 * FunctionPoints: AppStateObserverManager OnImageProcessStateChanged
 * CaseDescription: OnImageProcessStateChanged_ShouldExecImageProcessStateChangeCallbackWhenMapIsExist
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    OnImageProcessStateChanged_ShouldExecImageProcessStateChangeCallbackWhenMapIsExist, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto mockHandler = std::make_shared<MockTaskHandlerWrap>();
    ASSERT_NE(mockHandler, nullptr);
    manager->handler_ = mockHandler;
    int32_t imagePid = 100;
    auto appRecord = MockAppRecord();
    ImageProcessState state = ImageProcessState::IMAGE_PROCESS_CREATE;
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    manager->imageProcessStateObserverMap_.emplace(observer, 0);
    EXPECT_CALL(*observer, OnImageProcessStateChanged(_))Times(1);
    manager->OnImageProcessStateChanged(imagePid, appRecord, state);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnForkAllWorkProcessFailed
 * FunctionPoints: AppStateObserverManager OnForkAllWorkProcessFailed
 * CaseDescription: OnForkAllWorkProcessFailed_ShouldNotExecImageProcessFailedCallbackWhenHandlerIsNullptr
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    OnForkAllWorkProcessFailed_ShouldNotExecImageProcessFailedCallbackWhenHandlerIsNullptr, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    int32_t imagePid = 100;
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    manager->imageProcessStateObserverMap_.emplace(observer, 0);
    EXPECT_CALL(*observer, OnForkAllWorkProcessFailed(_, _))Times(0);
    manager->OnForkAllWorkProcessFailed(imagePid, nullptr, 0);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnForkAllWorkProcessFailed
 * FunctionPoints: AppStateObserverManager OnForkAllWorkProcessFailed
 * CaseDescription: OnForkAllWorkProcessFailed_ShouldNotExecImageProcessFailedCallbackWhenAppRecordIsNullptr
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    OnForkAllWorkProcessFailed_ShouldNotExecImageProcessFailedCallbackWhenAppRecordIsNullptr, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto mockHandler = std::make_shared<MockTaskHandlerWrap>();
    manager->handler_ = mockHandler;
    ASSERT_NE(manager->handler_, nullptr);
    int32_t imagePid = 100;
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    manager->imageProcessStateObserverMap_.emplace(observer, 0);
    EXPECT_CALL(*observer, OnForkAllWorkProcessFailed(_, _))Times(0);
    manager->OnForkAllWorkProcessFailed(imagePid, nullptr, 0);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnForkAllWorkProcessFailed
 * FunctionPoints: AppStateObserverManager OnForkAllWorkProcessFailed
 * CaseDescription: OnForkAllWorkProcessFailed_ShouldNotExecImageProcessFailedCallbackWhenMapIsEmpty
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    OnForkAllWorkProcessFailed_ShouldNotExecImageProcessFailedCallbackWhenMapIsEmpty, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto mockHandler = std::make_shared<MockTaskHandlerWrap>();
    manager->handler_ = mockHandler;
    ASSERT_NE(manager->handler_, nullptr);
    int32_t imagePid = 100;
    auto appRecord = MockAppRecord();
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    EXPECT_CALL(*observer, OnForkAllWorkProcessFailed(_, _))Times(0);
    manager->OnForkAllWorkProcessFailed(imagePid, appRecord, 0);
}

/*
 * Feature: AppStateObserverManager
 * Function: OnForkAllWorkProcessFailed
 * FunctionPoints: AppStateObserverManager OnForkAllWorkProcessFailed
 * CaseDescription: OnForkAllWorkProcessFailed_ShouldExecImageProcessFailedCallbackWhenMapIsExist
 */
HWTEST_F(AppStateObserverManagerThirdTest,
    OnForkAllWorkProcessFailed_ShouldExecImageProcessFailedCallbackWhenMapIsExist, TestSize.Level1)
{
    auto manager = std::make_shared<AppStateObserverManager>();
    ASSERT_NE(manager, nullptr);
    auto mockHandler = std::make_shared<MockTaskHandlerWrap>();
    ASSERT_NE(mockHandler, nullptr);
    manager->handler_ = mockHandler;
    int32_t imagePid = 100;
    auto appRecord = MockAppRecord();
    sptr<MockImageProcessStateObserverStub> observer = new (std::nothrow) MockImageProcessStateObserverStub();
    manager->imageProcessStateObserverMap_.emplace(observer, 0);
    EXPECT_CALL(*observer, OnForkAllWorkProcessFailed(_, _))Times(1);
    manager->OnForkAllWorkProcessFailed(imagePid, appRecord, 0);
}
} // namespace AppExecFwk
} // namespace OHOS