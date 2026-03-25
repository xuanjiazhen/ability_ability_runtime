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

#define private public
#include "app_state_observer_manager.h"
#undef private
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
        return task_.size();
    }
};

class MockImageErrorHandlerStub : public ImageProcessStateObserverStub {
public:
    MockImageProcessStateObserverStub() = default;
    virtual ~MockImageProcessStateObserverStub() = default;

    MOCK_METHOD1(OnImageProcessStateChanged, void(const ImageProcessStateData &imageProcessStateData));
    MOCK_METHOD2(OnForkAllWorkProcessFailed, void(const ImageProcessStateData &imageProcessStateData));
};

class AppStateObserverManagerThirdTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    std::shared_ptr<AppRunningRecord> MockAppRecord();
}

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
    std::shared_ptr<ApplicationInfo> info = std::make_shared<AppRunningRecord>(appInfo);
    info->accessTokenId = 1;
    std::shared_ptr<AppRunningRecord> appRecord = std::make_shared<AppRunningRecord>(info, 0, "process");
    std::shared_ptr<PriorityObject> priorityObject;
    priorityObject->SetUid(1);
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

/**
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
}  // namespace AppExecFwk
}  // namespace OHOS