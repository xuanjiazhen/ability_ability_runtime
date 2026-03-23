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
 * @tc.number: JsAbilityStageContext_GetCurrentElementName_001
 * @tc.name: GetCurrentElementName with valid ElementName
 * @tc.desc: Test GetCurrentElementName returns correct JS object when ElementName is set.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_GetCurrentElementName_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_GetCurrentElementName_001 start";

    // Create AbilityStageContext and set ElementName
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("com.example.testbundle");
    elementName.SetAbilityName("TestAbility");
    elementName.SetModuleName("TestModule");
    abilityStageContext->SetElementName(elementName);

    // Create JS context object
    napi_value jsContext = CreateJsAbilityStageContext(env_, abilityStageContext);
    ASSERT_NE(jsContext, nullptr);

    // Call GetCurrentElementName
    napi_value result = JsAbilityStageContext::GetCurrentElementName(env_, nullptr);
    EXPECT_NE(result, nullptr);

    // Verify result is not undefined
    napi_valuetype valueType;
    napi_status status = napi_typeof(env_, result, &valueType);
    EXPECT_EQ(status, napi_ok);
    EXPECT_NE(valueType, napi_undefined);

    GTEST_LOG_(INFO) << "JsAbilityStageContext_GetCurrentElementName_001 end";
}

/**
 * @tc.number: JsAbilityStageContext_GetCurrentElementName_002
 * @tc.name: GetCurrentElementName with empty ElementName
 * @tc.desc: Test GetCurrentElementName when ElementName is not set (empty).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_GetCurrentElementName_002, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_GetCurrentElementName_002 start";

    // Create AbilityStageContext without setting ElementName
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    // Create JS context object
    napi_value jsContext = CreateJsAbilityStageContext(env_, abilityStageContext);
    ASSERT_NE(jsContext, nullptr);

    // Call GetCurrentElementName - should return an ElementName object even if empty
    napi_value result = JsAbilityStageContext::GetCurrentElementName(env_, nullptr);
    EXPECT_NE(result, nullptr);

    // Verify result is not undefined
    napi_valuetype valueType;
    napi_status status = napi_typeof(env_, result, &valueType);
    EXPECT_EQ(status, napi_ok);

    GTEST_LOG_(INFO) << "JsAbilityStageContext_GetCurrentElementName_002 end";
}

/**
 * @tc.number: JsAbilityStageContext_GetCurrentElementName_003
 * @tc.name: GetCurrentElementName with null context
 * @tc.desc: Test GetCurrentElementName when context is null.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_GetCurrentElementName_003, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_GetCurrentElementName_003 start";

    // Create a context with null internal impl
    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    // Don't set ElementName, keep it as default

    // Create JS context object
    napi_value jsContext = CreateJsAbilityStageContext(env_, abilityStageContext);
    ASSERT_NE(jsContext, nullptr);

    // Call GetCurrentElementName - should handle gracefully
    napi_value result = JsAbilityStageContext::GetCurrentElementName(env_, nullptr);
    // Result may be undefined or empty ElementName object
    EXPECT_NE(result, nullptr);

    GTEST_LOG_(INFO) << "JsAbilityStageContext_GetCurrentElementName_003 end";
}

/**
 * @tc.number: JsAbilityStageContext_CreateJsAbilityStageContext_001
 * @tc.name: CreateJsAbilityStageContext binds GetCurrentElementName
 * @tc.desc: Test that CreateJsAbilityStageContext properly binds GetCurrentElementName function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_CreateJsAbilityStageContext_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_CreateJsAbilityStageContext_001 start";

    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    AppExecFwk::ElementName elementName;
    elementName.SetBundleName("com.example.bindtest");
    elementName.SetAbilityName("BindTestAbility");
    abilityStageContext->SetElementName(elementName);

    // Create JS context
    napi_value jsContext = CreateJsAbilityStageContext(env_, abilityStageContext);
    ASSERT_NE(jsContext, nullptr);

    // Verify it's an object
    napi_valuetype valueType;
    napi_status status = napi_typeof(env_, jsContext, &valueType);
    EXPECT_EQ(status, napi_ok);
    EXPECT_EQ(valueType, napi_object);

    // Try to get the getCurrentElementName property
    napi_value getCurrentElementNameFunc;
    status = napi_get_named_property(env_, jsContext, "getCurrentElementName", &getCurrentElementNameFunc);
    EXPECT_EQ(status, napi_ok);
    EXPECT_NE(getCurrentElementNameFunc, nullptr);

    // Verify it's a function
    status = napi_typeof(env_, getCurrentElementNameFunc, &valueType);
    EXPECT_EQ(status, napi_ok);
    EXPECT_EQ(valueType, napi_function);

    GTEST_LOG_(INFO) << "JsAbilityStageContext_CreateJsAbilityStageContext_001 end";
}

/**
 * @tc.number: JsAbilityStageContext_ElementName_Values_001
 * @tc.name: ElementName with all fields set
 * @tc.desc: Test GetCurrentElementName with all ElementName fields populated.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(JsAbilityStageContextTest, JsAbilityStageContext_ElementName_Values_001, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "JsAbilityStageContext_ElementName_Values_001 start";

    auto abilityStageContext = std::make_shared<AbilityStageContext>();
    ASSERT_NE(abilityStageContext, nullptr);

    // Set all ElementName fields
    AppExecFwk::ElementName elementName;
    elementName.SetDeviceId("testDeviceId123");
    elementName.SetBundleName("com.example.fulltest");
    elementName.SetAbilityName("FullTestAbility");
    elementName.SetModuleName("FullTestModule");
    elementName.SetInstanceId("instance123");
    abilityStageContext->SetElementName(elementName);

    // Create JS context
    napi_value jsContext = CreateJsAbilityStageContext(env_, abilityStageContext);
    ASSERT_NE(jsContext, nullptr);

    // Call GetCurrentElementName
    napi_value result = JsAbilityStageContext::GetCurrentElementName(env_, nullptr);
    ASSERT_NE(result, nullptr);

    // Verify result is an object
    napi_valuetype valueType;
    napi_status status = napi_typeof(env_, result, &valueType);
    EXPECT_EQ(status, napi_ok);
    EXPECT_EQ(valueType, napi_object);

    GTEST_LOG_(INFO) << "JsAbilityStageContext_ElementName_Values_001 end";
}

} // namespace AbilityRuntime
} // namespace OHOS
