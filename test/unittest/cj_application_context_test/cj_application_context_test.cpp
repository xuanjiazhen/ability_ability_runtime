/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#define private public
#include "cj_ability_delegator.h"
#include "cj_ability_lifecycle_callback.h"
#include "cj_ability_runtime_error.h"
#include "cj_application_context.h"
#include "ability_delegator_registry.h"
#include "application_context.h"
#include "cj_utils_ffi.h"
#include "window_stage_impl.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::FFI;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace ApplicationContextCJ {

// Mock类
class MockCjAbilityLifecycleCallback : public CjAbilityLifecycleCallbackImpl {
public:
    MOCK_METHOD(void, OnAbilityCreate, (const int64_t &ability));
    MOCK_METHOD(void, OnWindowStageCreate, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnWindowStageActive, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnWindowStageInactive, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnWindowStageDestroy, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnAbilityDestroy, (const int64_t &ability));
    MOCK_METHOD(void, OnAbilityForeground, (const int64_t &ability));
    MOCK_METHOD(void, OnAbilityBackground, (const int64_t &ability));
    MOCK_METHOD(void, OnAbilityContinue, (const int64_t &ability));

    MOCK_METHOD(void, OnAbilityWillCreate, (const int64_t &ability));
    MOCK_METHOD(void, OnWindowStageWillCreate, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnWindowStageWillDestroy, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnAbilityWillDestroy, (const int64_t &ability));
    MOCK_METHOD(void, OnAbilityWillForeground, (const int64_t &ability));
    MOCK_METHOD(void, OnAbilityWillBackground, (const int64_t &ability));
    MOCK_METHOD(void, OnNewWant, (const int64_t &ability));
    MOCK_METHOD(void, OnWillNewWant, (const int64_t &ability));
    MOCK_METHOD(void, OnAbilityWillContinue, (const int64_t &ability));
    MOCK_METHOD(void, OnWindowStageWillRestore, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnWindowStageRestore, (const int64_t &ability, WindowStagePtr windowStage));
    MOCK_METHOD(void, OnAbilityWillSaveState, (const int64_t &ability));
    MOCK_METHOD(void, OnAbilitySaveState, (const int64_t &ability));
};

class CjApplicationContextTest : public testing::Test {
public:
    CjApplicationContextTest()
    {}
    ~CjApplicationContextTest()
    {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<CJApplicationContext> InitCJApplicationContext();
};
void CjApplicationContextTest::SetUpTestCase() {}
void CjApplicationContextTest::TearDownTestCase() {}
void CjApplicationContextTest::SetUp() {}
void CjApplicationContextTest::TearDown() {}
std::shared_ptr<CJApplicationContext> CjApplicationContextTest::InitCJApplicationContext()
{
    // Create an ApplicationContext object
    auto appContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    std::weak_ptr<AbilityRuntime::ApplicationContext> wp(appContext);
    // Create an CJApplicationContext object
    return std::make_shared<CJApplicationContext>(std::move(wp));
}

/**
 * @tc.name: CJApplicationContextTestGetArea_001
 * @tc.desc: CjApplicationContextTest test for GetArea.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestGetArea_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    // 测试 GetArea 函数
    int area = cjAppContext->GetArea();
    EXPECT_EQ(area, 1);
}

/**
 * @tc.name: CJApplicationContextTestGetApplicationInfo_001
 * @tc.desc: CjApplicationContextTest test for GetApplicationInfo.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestGetApplicationInfo_001, TestSize.Level1)
{
    std::shared_ptr<ContextImpl> contextImpl = std::make_shared<ContextImpl>();
    auto appContext = std::make_shared<AbilityRuntime::ApplicationContext>();
    appContext->AttachContextImpl(contextImpl);
    auto appInfo = std::make_shared<AppExecFwk::ApplicationInfo>();
    appInfo->name = "TestApp";
    appInfo->bundleName = "com.example.testapp";
    appContext->SetApplicationInfo(appInfo);
    std::weak_ptr<AbilityRuntime::ApplicationContext> wp(appContext);

    auto cjAppContext = std::make_shared<CJApplicationContext>(std::move(wp));
    // 测试 GetApplicationInfo 函数
    EXPECT_NE(cjAppContext->GetApplicationInfo(), nullptr);
}

/**
 * @tc.name: CJApplicationContextTestIsA
 * bilityLifecycleCallbackEmpty_001
 * @tc.desc: CjApplicationContextTest test for IsAbilityLifecycleCallbackEmpty.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestIsAbilityLifecycleCallbackEmpty_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    bool result = cjAppContext->IsAbilityLifecycleCallbackEmpty();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name: CJApplicationContextTestOnOnEnvironment_001
 * @tc.desc: CjApplicationContextTest test for OnOnEnvironment.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnOnEnvironment_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto cfgCallback = [](AbilityRuntime::CConfiguration) {};
    auto memCallback = [](int32_t) {};
    EXPECT_EQ(cjAppContext->OnOnEnvironment(cfgCallback, memCallback, true, nullptr), -1);

    cjAppContext->envCallback_ = nullptr;
    EXPECT_EQ(cjAppContext->OnOnEnvironment(cfgCallback, memCallback, true, nullptr), -1);
}

/**
 * @tc.name: CJApplicationContextTestOnOnAbilityLifecycle_001
 * @tc.desc: CjApplicationContextTest test for OnOnAbilityLifecycle.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnOnAbilityLifecycle_001, TestSize.Level1)
{
    CArrI64 cFuncIds;
    bool isSync = true;
    int32_t err = 0;
    int32_t *errCode = &err;
    auto cjAppContext = InitCJApplicationContext();
    cjAppContext->applicationContext_.reset();
    cjAppContext->callback_ = std::make_shared<AbilityRuntime::CjAbilityLifecycleCallbackImpl>();
    EXPECT_EQ(cjAppContext->OnOnAbilityLifecycle(cFuncIds, isSync, errCode), -1);

    cjAppContext->applicationContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    EXPECT_EQ(cjAppContext->OnOnAbilityLifecycle(cFuncIds, isSync, errCode), -1);
}

/**
 * @tc.name: CJApplicationContextTestOnOnApplicationStateChange_001
 * @tc.desc: CjApplicationContextTest test for OnOnApplicationStateChange.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnOnApplicationStateChange_001, TestSize.Level1)
{
    auto foregroundCallback = []() {};
    auto backgroundCallback = []() {};
    int32_t err = 0;
    int32_t *errCode = &err;
    auto cjAppContext = InitCJApplicationContext();
    EXPECT_EQ(cjAppContext->OnOnApplicationStateChange(foregroundCallback, backgroundCallback, errCode), 0);

    cjAppContext->applicationStateCallback_ = nullptr;
    EXPECT_EQ(cjAppContext->OnOnApplicationStateChange(foregroundCallback, backgroundCallback, errCode), 1);
}

/**
 * @tc.name: CJApplicationContextTestOnOffEnvironment_001
 * @tc.desc: CjApplicationContextTest test for OnOffEnvironment.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnOffEnvironment_001, TestSize.Level1)
{
    int32_t callbackId = 10;
    int32_t err = 0;
    int32_t *errCode = &err;
    auto cjAppContext = InitCJApplicationContext();
    cjAppContext->applicationContext_.reset();
    cjAppContext->envCallback_ = std::make_shared<CjEnvironmentCallback>();
    cjAppContext->OnOffEnvironment(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);

    cjAppContext->applicationContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    cjAppContext->OnOffEnvironment(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);

    cjAppContext->envCallback_ = nullptr;
    cjAppContext->OnOffEnvironment(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CJApplicationContextTestOnOffAbilityLifecycle_001
 * @tc.desc: CjApplicationContextTest test for OnOffAbilityLifecycle.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnOffAbilityLifecycle_001, TestSize.Level1)
{
    int32_t callbackId = 10;
    int32_t err = 0;
    int32_t *errCode = &err;
    auto cjAppContext = InitCJApplicationContext();
    cjAppContext->applicationContext_.reset();
    cjAppContext->callback_ = std::make_shared<AbilityRuntime::CjAbilityLifecycleCallbackImpl>();
    cjAppContext->OnOffAbilityLifecycle(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);

    cjAppContext->applicationContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    cjAppContext->OnOffAbilityLifecycle(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);

    cjAppContext->callback_ = nullptr;
    cjAppContext->OnOffAbilityLifecycle(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CJApplicationContextTestOnOffApplicationStateChange_001
 * @tc.desc: CjApplicationContextTest test for OnOffApplicationStateChange.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnOffApplicationStateChange_001, TestSize.Level1)
{
    int32_t callbackId = 10;
    int32_t err = 0;
    int32_t *errCode = &err;
    auto cjAppContext = InitCJApplicationContext();
    cjAppContext->OnOffApplicationStateChange(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);

    cjAppContext->applicationContext_ = std::make_shared<AbilityRuntime::ApplicationContext>();
    cjAppContext->applicationStateCallback_ = std::make_shared<CjApplicationStateChangeCallback>();
    cjAppContext->OnOffApplicationStateChange(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);

    cjAppContext->applicationStateCallback_ = nullptr;
    cjAppContext->OnOffApplicationStateChange(callbackId, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnAbilityCreate_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnAbilityCreate.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnAbilityCreate_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnAbilityCreate(_)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    cjAppContext->DispatchOnAbilityCreate(10);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnWindowStageCreate_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnWindowStageCreate.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnWindowStageCreate_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnWindowStageCreate(_, _)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    auto win = std::make_shared<Rosen::WindowScene>();
    auto winStage = new Rosen::CJWindowStageImpl(win);
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(winStage);
    cjAppContext->DispatchOnWindowStageCreate(10, windowStage);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchWindowStageFocus_001
 * @tc.desc: CjApplicationContextTest test for DispatchWindowStageFocus.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchWindowStageFocus_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnWindowStageActive(_, _)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    auto win = std::make_shared<Rosen::WindowScene>();
    auto winStage = new Rosen::CJWindowStageImpl(win);
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(winStage);
    cjAppContext->DispatchWindowStageFocus(10, windowStage);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchWindowStageUnfocus_001
 * @tc.desc: CjApplicationContextTest test for DispatchWindowStageUnfocus.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchWindowStageUnfocus_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnWindowStageInactive(_, _)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    auto win = std::make_shared<Rosen::WindowScene>();
    auto winStage = new Rosen::CJWindowStageImpl(win);
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(winStage);
    cjAppContext->DispatchWindowStageUnfocus(10, windowStage);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnWindowStageDestroy_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnWindowStageDestroy.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnWindowStageDestroy_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnWindowStageDestroy(_, _)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    auto win = std::make_shared<Rosen::WindowScene>();
    auto winStage = new Rosen::CJWindowStageImpl(win);
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(winStage);
    cjAppContext->DispatchOnWindowStageDestroy(10, windowStage);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnAbilityDestroy_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnAbilityDestroy.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnAbilityDestroy_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnAbilityDestroy(_)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    cjAppContext->DispatchOnAbilityDestroy(10);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnAbilityForeground_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnAbilityForeground.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnAbilityForeground_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnAbilityForeground(_)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    cjAppContext->DispatchOnAbilityForeground(10);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnAbilityBackground_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnAbilityBackground.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnAbilityBackground_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnAbilityBackground(_)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    cjAppContext->DispatchOnAbilityBackground(10);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextDispatchOnAbilityContinue_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnAbilityContinue.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextDispatchOnAbilityContinue_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnAbilityContinue(_)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    cjAppContext->DispatchOnAbilityContinue(10);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnAbilityWillCreate_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnAbilityWillCreate.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnAbilityWillCreate_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnAbilityWillCreate(_)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    cjAppContext->DispatchOnAbilityWillCreate(10);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnWindowStageWillCreate_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnWindowStageWillCreate.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnWindowStageWillCreate_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnWindowStageWillCreate(_, _)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    auto win = std::make_shared<Rosen::WindowScene>();
    auto winStage = new Rosen::CJWindowStageImpl(win);
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(winStage);
    cjAppContext->DispatchOnWindowStageWillCreate(10, windowStage);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnWindowStageWillDestroy_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnWindowStageWillDestroy.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnWindowStageWillDestroy_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnWindowStageWillDestroy(_, _)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    auto win = std::make_shared<Rosen::WindowScene>();
    auto winStage = new Rosen::CJWindowStageImpl(win);
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(winStage);
    cjAppContext->DispatchOnWindowStageWillDestroy(10, windowStage);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnAbilityWillDestroy_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnAbilityWillDestroy.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnAbilityWillDestroy_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnAbilityWillDestroy(_)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    cjAppContext->DispatchOnAbilityWillDestroy(10);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnAbilityWillForeground_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnAbilityWillForeground.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnAbilityWillForeground_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnAbilityWillForeground(_)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    cjAppContext->DispatchOnAbilityWillForeground(10);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnAbilityWillBackground_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnAbilityWillBackground.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnAbilityWillBackground_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnAbilityWillBackground(_)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    cjAppContext->DispatchOnAbilityWillBackground(10);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnNewWant_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnNewWant.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnNewWant_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnNewWant(_)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    cjAppContext->DispatchOnNewWant(10);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnNewWant_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnNewWant.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnWillNewWant_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnWillNewWant(_)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    cjAppContext->DispatchOnWillNewWant(10);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnAbilityWillContinue_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnAbilityWillContinue.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnAbilityWillContinue_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnAbilityWillContinue(_)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    cjAppContext->DispatchOnAbilityWillContinue(10);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnWindowStageWillRestore_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnWindowStageWillRestore.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnWindowStageWillRestore_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnWindowStageWillRestore(_, _)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    auto win = std::make_shared<Rosen::WindowScene>();
    auto winStage = new Rosen::CJWindowStageImpl(win);
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(winStage);
    cjAppContext->DispatchOnWindowStageWillRestore(10, windowStage);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnWindowStageRestore_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnWindowStageRestore.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnWindowStageRestore_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnWindowStageRestore(_, _)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    auto win = std::make_shared<Rosen::WindowScene>();
    auto winStage = new Rosen::CJWindowStageImpl(win);
    WindowStagePtr windowStage = reinterpret_cast<WindowStagePtr>(winStage);
    cjAppContext->DispatchOnWindowStageRestore(10, windowStage);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnAbilityWillSaveState_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnAbilityWillSaveState.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnAbilityWillSaveState_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnAbilityWillSaveState(_)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    cjAppContext->DispatchOnAbilityWillSaveState(10);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestDispatchOnAbilitySaveState_001
 * @tc.desc: CjApplicationContextTest test for DispatchOnAbilitySaveState.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestDispatchOnAbilitySaveState_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    auto mock = std::make_shared<MockCjAbilityLifecycleCallback>();
    EXPECT_CALL(*mock, OnAbilitySaveState(_)).Times(1);
    cjAppContext->RegisterAbilityLifecycleCallback(mock);
    cjAppContext->DispatchOnAbilitySaveState(10);
    cjAppContext->UnregisterAbilityLifecycleCallback(mock);
}

/**
 * @tc.name: CJApplicationContextTestOnGetRunningProcessInformation_001
 * @tc.desc: CjApplicationContextTest test for OnGetRunningProcessInformation.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnGetRunningProcessInformation_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    cjAppContext->applicationContext_.reset();
    int32_t err = 0;
    int32_t* errorCode = &err;
    auto info = cjAppContext->OnGetRunningProcessInformation(errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST);
    EXPECT_EQ(info, nullptr);
}

/**
 * @tc.name: CJApplicationContextTestOnOnKillProcessBySelf_001
 * @tc.desc: CjApplicationContextTest test for OnKillProcessBySelf.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnOnKillProcessBySelf_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    cjAppContext->applicationContext_.reset();
    int32_t err = 0;
    int32_t* errorCode = &err;
    cjAppContext->OnKillProcessBySelf(true, errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST);

    auto applicationContext = std::make_shared<ApplicationContext>();
    cjAppContext = std::make_shared<CJApplicationContext>(applicationContext);
    err = 0;
    cjAppContext->OnKillProcessBySelf(true, errorCode);
    EXPECT_EQ(*errorCode, 0);
}

/**
 * @tc.name: CJApplicationContextTestOnGetCurrentAppCloneIndex_001
 * @tc.desc: CjApplicationContextTest test for OnGetCurrentAppCloneIndex.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnGetCurrentAppCloneIndex_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    cjAppContext->applicationContext_.reset();
    int32_t err = 0;
    int32_t* errorCode = &err;
    auto index = cjAppContext->OnGetCurrentAppCloneIndex(errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST);

    auto applicationContext = std::make_shared<ApplicationContext>();
    cjAppContext = std::make_shared<CJApplicationContext>(applicationContext);
    index = cjAppContext->OnGetCurrentAppCloneIndex(errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_NOT_APP_CLONE);
    EXPECT_EQ(index, -1);
}

/**
 * @tc.name: CJApplicationContextTestOnRestartApp_001
 * @tc.desc: CjApplicationContextTest test for OnRestartApp.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnRestartApp_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    cjAppContext->applicationContext_.reset();
    int32_t err = 0;
    int32_t* errorCode = &err;
    cjAppContext->OnRestartApp(AAFwk::Want(), errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST);

    auto applicationContext = std::make_shared<ApplicationContext>();
    cjAppContext = std::make_shared<CJApplicationContext>(applicationContext);
    cjAppContext->OnRestartApp(AAFwk::Want(), errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: CJApplicationContextTestOnClearUpApplicationData_001
 * @tc.desc: CjApplicationContextTest test for OnClearUpApplicationData.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnClearUpApplicationData_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    cjAppContext->applicationContext_.reset();
    int32_t err = 0;
    int32_t* errorCode = &err;
    cjAppContext->OnClearUpApplicationData(errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST);

    auto applicationContext = std::make_shared<ApplicationContext>();
    cjAppContext = std::make_shared<CJApplicationContext>(applicationContext);
    err = 0;
    cjAppContext->OnClearUpApplicationData(errorCode);
    EXPECT_EQ(*errorCode, 0);
}

/**
 * @tc.name: CJApplicationContextTestOnSetSupportedProcessCacheSelf_001
 * @tc.desc: CjApplicationContextTest test for OnSetSupportedProcessCacheSelf.
 * @tc.type: FUNC
 */
HWTEST_F(CjApplicationContextTest, CJApplicationContextTestOnSetSupportedProcessCacheSelf_001, TestSize.Level1)
{
    auto cjAppContext = InitCJApplicationContext();
    cjAppContext->applicationContext_.reset();
    int32_t err = 0;
    int32_t* errorCode = &err;
    cjAppContext->OnSetSupportedProcessCacheSelf(true, errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_CONTEXT_NOT_EXIST);

    auto applicationContext = std::make_shared<ApplicationContext>();
    cjAppContext = std::make_shared<CJApplicationContext>(applicationContext);
    cjAppContext->OnSetSupportedProcessCacheSelf(true, errorCode);
    EXPECT_EQ(*errorCode, ERR_ABILITY_RUNTIME_EXTERNAL_INTERNAL_ERROR);
}
}  // namespace AbilityRuntime
}  // namespace OHOS