/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#include "cj_application_context_ffi.h"
#include "cj_application_context_ffi.cpp"
#include "cj_ability_runtime_error.h"
#include "ffi_remote_data.h"
#undef private
#undef protected

using namespace testing;
using namespace testing::ext;
using namespace OHOS::FFI;
using namespace OHOS::AbilityRuntime;
using namespace OHOS::AppExecFwk;

namespace OHOS {
namespace ApplicationContextCJ {
class FfiCjApplicationContextTest : public testing::Test {
public:
    FfiCjApplicationContextTest()
    {}
    ~FfiCjApplicationContextTest()
    {}
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void FfiCjApplicationContextTest::SetUpTestCase()
{}

void FfiCjApplicationContextTest::TearDownTestCase()
{}

void FfiCjApplicationContextTest::SetUp()
{}

void FfiCjApplicationContextTest::TearDown()
{}

/**
 * @tc.name: FfiCjApplicationContextTestFFIGetArea_0100
 * @tc.desc: FfiCjApplicationContextTest test for FFIGetArea.
 * @tc.type: FUNC
 */
HWTEST_F(FfiCjApplicationContextTest, FfiCjApplicationContextTestFFIGetArea_0100, TestSize.Level1)
{
    auto testId = 1;
    int64_t area = FFIGetArea(testId);
    EXPECT_EQ(area, 1);

    std::shared_ptr<AbilityRuntime::ApplicationContext> applicationContext = std::make_shared<ApplicationContext>();
    std::weak_ptr<AbilityRuntime::ApplicationContext> applicationContextTest = applicationContext;
    auto cjApplicationContext = FFI::FFIData::Create<CJApplicationContext>(std::move(applicationContextTest));
    testId = cjApplicationContext->GetID();

    area = FFIGetArea(testId);
    EXPECT_EQ(area, 1);
}

/**
 * @tc.name: FfiCjApplicationContextTestFFICJApplicationInfo_0100
 * @tc.desc: FfiCjApplicationContextTest test for FFICJApplicationInfo.
 * @tc.type: FUNC
 */
HWTEST_F(FfiCjApplicationContextTest, FfiCjApplicationContextTestFFICJApplicationInfo_0100, TestSize.Level1)
{
    int64_t testId = 1;
    auto info = FFICJApplicationInfo(testId);
    EXPECT_EQ(info, nullptr);
}

/**
 * @tc.name: FfiCjApplicationContextTestFfiCJApplicationContextOnOnEnvironment_0100
 * @tc.desc: FfiCjApplicationContextTest test for FfiCJApplicationContextOnOnEnvironment.
 * @tc.type: FUNC
 */
HWTEST_F(FfiCjApplicationContextTest,
    FfiCjApplicationContextTestFfiCJApplicationContextOnOnEnvironment_0100, TestSize.Level1)
{
    int64_t id = 1;
    auto cfgCallback = [](AbilityRuntime::CConfiguration) {};
    auto memCallback = [](int32_t) {};
    int32_t code = -1;
    int32_t *errCode = &code;
    int32_t result =  FfiCJApplicationContextOnOnEnvironment(id, cfgCallback, memCallback, errCode);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: FfiCjApplicationContextTestFfiCJApplicationContextOnOnAbilityLifecycle_0100
 * @tc.desc: FfiCjApplicationContextTest test for FfiCJApplicationContextOnOnAbilityLifecycle.
 * @tc.type: FUNC
 */
HWTEST_F(FfiCjApplicationContextTest,
    FfiCjApplicationContextTestFfiCJApplicationContextOnOnAbilityLifecycle_0100, TestSize.Level1)
{
    int64_t id = 1;
    CArrI64 cFuncIds = {nullptr, 0};
    int32_t code = 1;
    int32_t *errCode = &code;
    int32_t result =  FfiCJApplicationContextOnOnAbilityLifecycle(id, cFuncIds, errCode);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: FfiCjApplicationContextTestFfiCJApplicationContextOnOnApplicationStateChange_0100
 * @tc.desc: FfiCjApplicationContextTest test for FfiCJApplicationContextOnOnApplicationStateChange.
 * @tc.type: FUNC
 */
HWTEST_F(FfiCjApplicationContextTest,
    FfiCjApplicationContextTestFfiCJApplicationContextOnOnApplicationStateChange_0100, TestSize.Level1)
{
    int64_t id = 1;
    auto foregroundCallback = [](void) {};
    auto backgroundCallback = [](void) {};
    int32_t code = 1;
    int32_t *errCode = &code;
    int32_t result =  FfiCJApplicationContextOnOnApplicationStateChange(id, foregroundCallback,
        backgroundCallback, errCode);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: FfiCjApplicationContextTestConvertToJsAppProcessState_0100
 * @tc.desc: FfiCjApplicationContextTest test for ConvertToJsAppProcessState.
 * @tc.type: FUNC
 */
HWTEST_F(FfiCjApplicationContextTest, FfiCjApplicationContextTestConvertToJsAppProcessState_0100, TestSize.Level1)
{
    bool isFocused = false;
    AppExecFwk::AppProcessState appProcessState = AppExecFwk::AppProcessState::APP_STATE_CREATE;
    auto result = ConvertToJsAppProcessState(appProcessState, isFocused);
    EXPECT_EQ(result, STATE_CREATE);

    appProcessState = AppExecFwk::AppProcessState::APP_STATE_READY;
    result = ConvertToJsAppProcessState(appProcessState, isFocused);
    EXPECT_EQ(result, STATE_CREATE);

    appProcessState = AppExecFwk::AppProcessState::APP_STATE_FOREGROUND;
    result = ConvertToJsAppProcessState(appProcessState, isFocused);
    EXPECT_EQ(result, STATE_FOREGROUND);

    isFocused = true;
    result = ConvertToJsAppProcessState(appProcessState, isFocused);
    EXPECT_EQ(result, STATE_ACTIVE);

    appProcessState = AppExecFwk::AppProcessState::APP_STATE_BACKGROUND;
    result = ConvertToJsAppProcessState(appProcessState, isFocused);
    EXPECT_EQ(result, STATE_BACKGROUND);

    appProcessState = AppExecFwk::AppProcessState::APP_STATE_TERMINATED;
    result = ConvertToJsAppProcessState(appProcessState, isFocused);
    EXPECT_EQ(result, STATE_DESTROY);
    
    appProcessState = AppExecFwk::AppProcessState::APP_STATE_END;
    result = ConvertToJsAppProcessState(appProcessState, isFocused);
    EXPECT_EQ(result, STATE_DESTROY);

    appProcessState = AppExecFwk::AppProcessState::APP_STATE_FOCUS;
    result = ConvertToJsAppProcessState(appProcessState, isFocused);
    EXPECT_EQ(result, STATE_DESTROY);
}

/**
 * @tc.name: FfiCjApplicationContextTestFfiCJApplicationContextGetRunningProcessInformation_0100
 * @tc.desc: FfiCjApplicationContextTest test for FfiCJApplicationContextGetRunningProcessInformation.
 * @tc.type: FUNC
 */
HWTEST_F(FfiCjApplicationContextTest,
    FfiCjApplicationContextTestFfiCJApplicationContextGetRunningProcessInformation_0100, TestSize.Level1)
{
    int64_t id = 1;
    int32_t code = 1;
    int32_t *errCode = &code;
    CArrProcessInformation cArrProcessInformation = {.head = nullptr, .size = 0};
    auto information =  FfiCJApplicationContextGetRunningProcessInformation(id, errCode);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}

/**
 * @tc.name: FfiCjApplicationContextTestFfiCJApplicationContextGetCurrentAppCloneIndex_0100
 * @tc.desc: FfiCjApplicationContextTest test for FfiCJApplicationContextGetCurrentAppCloneIndex.
 * @tc.type: FUNC
 */
HWTEST_F(FfiCjApplicationContextTest,
    FfiCjApplicationContextTestFfiCJApplicationContextGetCurrentAppCloneIndex_0100, TestSize.Level1)
{
    int64_t id = 1;
    int32_t code = 1;
    int32_t *errCode = &code;
    CArrProcessInformation cArrProcessInformation = {.head = nullptr, .size = 0};
    int32_t result =  FfiCJApplicationContextGetCurrentAppCloneIndex(id, errCode);
    EXPECT_EQ(result, -1);
    EXPECT_EQ(*errCode, ERR_ABILITY_RUNTIME_EXTERNAL_INVALID_PARAMETER);
}
}  // namespace AbilityRuntime
}  // namespace OHOS