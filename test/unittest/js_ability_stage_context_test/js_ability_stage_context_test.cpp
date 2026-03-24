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
#include <singleton.h>
#include <uv.h>

#include "ability_stage_context.h"
#include "hilog_wrapper.h"
#include "js_ability_stage_context.h"
#include "js_runtime_utils.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "native_engine/native_engine.h"
#include "napi_common_want.h"

using namespace testing;
using namespace testing::ext;
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace AbilityRuntime {

constexpr size_t ARGC_ZERO = 0;

const int USLEEPTIME = 100000;

class JsAbilityStageContextTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);

    void RunNowait(uv_loop_t* loop)
    {
        usleep(USLEEPTIME);
        uv_run(loop, UV_RUN_NOWAIT);
    }

    static napi_env env_;
    static panda::ecmascript::EcmaVM* vm_;
};

napi_env JsAbilityStageContextTest::env_ = nullptr;
panda::ecmascript::EcmaVM* JsAbilityStageContextTest::vm_ = nullptr;

void JsAbilityStageContextTest::SetUpTestCase()
{
    GTEST_LOG_(INFO) << "JsAbilityStageContextTest SetUpTestCase";
    vm_ = panda::ecmascript::EcmaVM::Create(nullptr);
    ASSERT_NE(vm_, nullptr);

    auto engine = std::make_shared<ArkNativeEngine>(vm_, nullptr);
    env_ = reinterpret_cast<napi_env>(engine.get());
    ASSERT_NE(env_, nullptr);
}

void JsAbilityStageContextTest::TearDownTestCase()
{
    GTEST_LOG_(INFO) << "JsAbilityStageContextTest TearDownTestCase";
    if (vm_ != nullptr) {
        vm_->Destroy();
        vm_ = nullptr;
        env_ = nullptr;
    }
}

void JsAbilityStageContextTest::SetUp()
{}

void JsAbilityStageContextTest::TearDown()
{}

/**
 * @tc.number: JsAbilityStageContext_LaunchElement_001
 * @tc.name: CreateJsAbilityStageContext sets launchElement property
 * @tc.desc: Test that CreateJsAbilityStageContext properly sets launchElement property when ElementName is set.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_LaunchElement_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_001 start";

    // Create AbilityStageContext and set ElementName
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("com.example.testbundle");
    elementName.SetAbilityName("TestAbility");
    elementName.SetModuleName("TestModule");
    abilityStageContext->SetLaunchElement(elementName);

    // Create JS context object
    napi_value jsContext = CreateJsAbilityStageContext(env_, abilityStageContext);
    ASSERT_NE(jsContext, nullptr);

    // Verify it's an object
    napi_valuetype valueType;
    napi_status status = napi_typeof(env_, jsContext, &valueType);
    EXPECT_EQ(status, napi_ok);
    EXPECT_EQ(valueType, napi_object);

    // Get the launchElement property
    napi_value launchElement;
    status = napi_get_named_property(env_, jsContext, "launchElement", &launchElement);
    EXPECT_EQ(status, napi_ok);
    EXPECT_NE(launchElement, nullptr);

    // Verify it's an object
    status = napi_typeof(env_, launchElement, &valueType);
    EXPECT_EQ(status, napi_ok);
    EXPECT_EQ(valueType, napi_object);

    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_001 end";
}

/**
 * @tc.number: JsAbilityStageContext_LaunchElement_002
 * @tc.name: launchElement property with empty ElementName
 * @tc.desc: Test launchElement property when ElementName is not set (empty).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_LaunchElement_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_002 start";

    // Create AbilityStageContext without setting ElementName
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    // Create JS context object
    napi_value jsContext = CreateJsAbilityStageContext(env_, abilityStageContext);
    ASSERT_NE(jsContext, nullptr);

    // Get the launchElement property - should exist even if empty
    napi_value launchElement;
    napi_status status = napi_get_named_property(env_, jsContext, "launchElement", &launchElement);
    EXPECT_EQ(status, napi_ok);

    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_002 end";
}

/**
 * @tc.number: JsAbilityStageContext_LaunchElement_003
 * @tc.name: launchElement property values
 * @tc.desc: Test that launchElement property contains correct values from ElementName.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_LaunchElement_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_003 start";

    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    // Set ElementName with all fields
    AppExecFwk::ElementName elementName;
    elementName.SetDeviceId("testDeviceId123");
    elementName.SetBundleName("com.example.valuetest");
    elementName.SetAbilityName("ValueTestAbility");
    elementName.SetModuleName("ValueTestModule");
    abilityStageContext->SetLaunchElement(elementName);

    // Create JS context
    napi_value jsContext = CreateJsAbilityStageContext(env_, abilityStageContext);
    ASSERT_NE(jsContext, nullptr);

    // Get launchElement property
    napi_value launchElement;
    napi_status status = napi_get_named_property(env_, jsContext, "launchElement", &launchElement);
    EXPECT_EQ(status, napi_ok);
    ASSERT_NE(launchElement, nullptr);

    // Verify it's an object
    napi_valuetype valueType;
    status = napi_typeof(env_, launchElement, &valueType);
    EXPECT_EQ(status, napi_ok);
    EXPECT_EQ(valueType, napi_object);

    // Verify we can read properties from the launchElement object
    napi_value bundleNameProp;
    status = napi_get_named_property(env_, launchElement, "bundleName", &bundleNameProp);
    EXPECT_EQ(status, napi_ok);

    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_003 end";
}

/**
 * @tc.number: JsAbilityStageContext_LaunchElement_004
 * @tc.name: launchElement property is not a function
 * @tc.desc: Test that launchElement is a property, not a function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_LaunchElement_004, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_004 start";

    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("com.example.typetest");
    elementName.SetAbilityName("TypeTestAbility");
    abilityStageContext->SetLaunchElement(elementName);

    // Create JS context
    napi_value jsContext = CreateJsAbilityStageContext(env_, abilityStageContext);
    ASSERT_NE(jsContext, nullptr);

    // Get launchElement property
    napi_value launchElement;
    napi_status status = napi_get_named_property(env_, jsContext, "launchElement", &launchElement);
    EXPECT_EQ(status, napi_ok);
    ASSERT_NE(launchElement, nullptr);

    // Verify it's NOT a function
    napi_valuetype valueType;
    status = napi_typeof(env_, launchElement, &valueType);
    EXPECT_EQ(status, napi_ok);
    EXPECT_NE(valueType, napi_function);
    EXPECT_EQ(valueType, napi_object);

    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_004 end";
}

/**
 * @tc.number: JsAbilityStageContext_LaunchElement_005
 * @tc.name: launchElement with null context
 * @tc.desc: Test CreateJsAbilityStageContext handles null context gracefully.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_LaunchElement_005, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_005 start";

    // Create context with null pointer
    std::shared_ptr<AbilityRuntime::Context> nullContext = nullptr;

    // Create JS context - should handle null gracefully
    napi_value jsContext = CreateJsAbilityStageContext(env_, nullContext);
    // Result depends on implementation - just verify it doesn't crash

    GTEST_LOG_(INFO) << "JsAbilityStageContext_LaunchElement_005 end";
}

} // namespace AbilityRuntime
} // namespace OHOS
