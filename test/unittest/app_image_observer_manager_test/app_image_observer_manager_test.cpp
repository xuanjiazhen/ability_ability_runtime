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
#include <gmock/gmock.h>

#include "app_image_observer_manager.h"

#include "mock_app_image_lifecycle_callback.h"

using namespace testing::ext;

namespace OHOS {
namespace AbilityRuntime {
class AppImageObserverManagerTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void AppImageObserverManagerTest::SetUpTestCase(void)
{}

void AppImageObserverManagerTest::TearDownTestCase(void)
{}

void AppImageObserverManagerTest::SetUp()
{}

void AppImageObserverManagerTest::TearDown()
{}

/**
 * @tc.number: RegisterImageLifecycleCallback_0100
 * @tc.name: RegisterImageLifecycleCallback
 * @tc.desc: pass in nullptr parameters, and the callback saved in the manager is also nullptr
 */
HWTEST_F(AppImageObserverManagerTest, RegisterImageLifecycleCallback_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "RegisterImageLifecycleCallback_0100 start";
    AppExecFwk::AppImageObserverManager observerManager;
    std::shared_ptr<MockAppImageLifeCycleCallback> appImageLifeCycleCallback = nullptr;
    observerManager.RegisterImageLifecycleCallback(appImageLifeCycleCallback);
    EXPECT_EQ(1, observerManager.appImageLifeCycleCallback_.size());
    GTEST_LOG_(INFO) << "RegisterImageLifecycleCallback_0100 end";
}

/**
 * @tc.number: NotifyApplicationUpdate_0100
 * @tc.name: NotifyApplicationUpdate
 * @tc.desc: pass 1 register a valid callback, NotifyApplicationUpdate is called
 *                2 the callback saved in the manager is valid
 */
HWTEST_F(AppImageObserverManagerTest, NotifyApplicationUpdate_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyApplicationUpdate_0100 start";
    AppExecFwk::AppImageObserverManager observerManager;
    observerManager.appImageLifeCycleCallback_.clear();
    observerManager.NotifyApplicationUpdate();
    std::shared_ptr<MockAppImageLifeCycleCallback> appImageLifeCycleCallback = nullptr;
    observerManager.RegisterImageLifecycleCallback(appImageLifeCycleCallback);
    appImageLifeCycleCallback = std::make_shared<MockAppImageLifeCycleCallback>();
    observerManager.RegisterImageLifecycleCallback(appImageLifeCycleCallback);
    EXPECT_EQ(2, observerManager.appImageLifeCycleCallback_.size());
    EXPECT_CALL(*appImageLifeCycleCallback, NotifyApplicationUpdate()).Times(1);
    observerManager.NotifyApplicationUpdate();
    GTEST_LOG_(INFO) << "NotifyApplicationUpdate_0100 end";
}

/**
 * @tc.number: NotifyApplicationPreAbilityCreate_0100
 * @tc.name: NotifyApplicationPreAbilityCreate
 * @tc.desc: pass 1 register a valid callback, NotifyApplicationPreAbilityCreate is called
 *                2 the callback saved in the manager is valid
 */
HWTEST_F(AppImageObserverManagerTest, NotifyApplicationPreAbilityCreate_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "NotifyApplicationPreAbilityCreate_0100 start";
    AppExecFwk::AppImageObserverManager observerManager;
    observerManager.appImageLifeCycleCallback_.clear();
    observerManager.NotifyApplicationPreAbilityCreate();
    std::shared_ptr<MockAppImageLifeCycleCallback> appImageLifeCycleCallback = nullptr;
    observerManager.RegisterImageLifecycleCallback(appImageLifeCycleCallback);
    appImageLifeCycleCallback = std::make_shared<MockAppImageLifeCycleCallback>();
    observerManager.RegisterImageLifecycleCallback(appImageLifeCycleCallback);
    EXPECT_EQ(2, observerManager.appImageLifeCycleCallback_.size());
    EXPECT_CALL(*appImageLifeCycleCallback, NotifyApplicationPreAbilityCreate()).Times(1);
    observerManager.NotifyApplicationPreAbilityCreate();
    GTEST_LOG_(INFO) << "NotifyApplicationPreAbilityCreate_0100 end";
}

/**
 * @tc.number: GetImageProcessType_0100
 * @tc.name: GetImageProcessType
 * @tc.desc: pass 1 register a valid callback, GetImageProcessType is called
 *                2 the callback saved in the manager is valid
 */
HWTEST_F(AppImageObserverManagerTest, GetImageProcessType_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetImageProcessType_0100 start";
    int32_t imageProcessType = 10;
    EXPECT_EQ(AppExecFwk::AppImageObserverManager::GetInstance().GetImageProcessType(), 0);
    AppExecFwk::AppImageObserverManager::GetInstance().SetImageProcessType(imageProcessType);
    EXPECT_EQ(AppExecFwk::AppImageObserverManager::GetInstance().GetImageProcessType(), imageProcessType);
    AppExecFwk::AppImageObserverManager::GetInstance().SetImageProcessType(0);
    GTEST_LOG_(INFO) << "GetImageProcessType_0100 end";
}

/**
 * @tc.number: IsAbilityCreated_0100
 * @tc.name: IsAbilityCreated
 * @tc.desc: pass 1 register a valid callback, IsAbilityCreated is called
 *                2 the callback saved in the manager is valid
 */
HWTEST_F(AppImageObserverManagerTest, IsAbilityCreated_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "GetImageProcessType_0100 start";
    EXPECT_FALSE(AppExecFwk::AppImageObserverManager::GetInstance().IsAbilityCreated());
    AppExecFwk::AppImageObserverManager::GetInstance().SetAbilityCreated(true);
    EXPECT_TRUE(AppExecFwk::AppImageObserverManager::GetInstance().IsAbilityCreated());
    AppExecFwk::AppImageObserverManager::GetInstance().SetAbilityCreated(false);
    GTEST_LOG_(INFO) << "GetImageProcessType_0100 end";
}
}  // namespace AbilityRuntime
}  // namespace OHOS