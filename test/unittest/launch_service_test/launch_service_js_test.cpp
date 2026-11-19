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
#include <memory>
#include "ecmascript/napi/include/jsnapi.h"
#include "native_engine/impl/ark/ark_native_engine.h"
#include "js_launch_service_manager.h"
using namespace testing::ext;
namespace OHOS::AbilityRuntime {
class LaunchServiceJsTest : public testing::Test {
public:
    void SetUp() override
    {
        panda::RuntimeOption option;
        vm_ = panda::JSNApi::CreateJSVM(option);
        ASSERT_NE(vm_, nullptr);
        engine_ = std::make_unique<ArkNativeEngine>(vm_, nullptr);
        env_ = reinterpret_cast<napi_env>(engine_.get());
    }
    void TearDown() override
    {
        engine_.reset();
        if (vm_ != nullptr) { panda::JSNApi::DestroyJSVM(vm_); }
    }
    panda::ecmascript::EcmaVM *vm_ = nullptr;
    std::unique_ptr<ArkNativeEngine> engine_;
    napi_env env_ = nullptr;
};

/**
 * @tc.name: JsLaunchServiceManagerInit_001
 * @tc.desc: Export all current functions and enum values through the real NAPI runtime.
 * @tc.type: FUNC
 */
HWTEST_F(LaunchServiceJsTest, JsLaunchServiceManagerInit_001, TestSize.Level1)
{
    napi_handle_scope scope = nullptr;
    ASSERT_EQ(napi_open_handle_scope(env_, &scope), napi_ok);
    napi_value exports = nullptr;
    ASSERT_EQ(napi_create_object(env_, &exports), napi_ok);
    EXPECT_EQ(JsLaunchServiceManagerInit(env_, exports), exports);
    for (const char *name : {"register", "unregister", "start", "stop", "getInformation",
        "getAllRegisteredInformation", "setLaunchServicesEnabled", "onLaunchServiceObserver", "offLaunchServiceObserver"}) {
        napi_value function = nullptr;
        ASSERT_EQ(napi_get_named_property(env_, exports, name, &function), napi_ok);
        napi_valuetype type = napi_undefined;
        ASSERT_EQ(napi_typeof(env_, function, &type), napi_ok);
        EXPECT_EQ(type, napi_function);
    }
    bool exists = true;
    ASSERT_EQ(napi_has_named_property(env_, exports, "setLaunchServicesEnable", &exists), napi_ok);
    EXPECT_FALSE(exists);
    napi_value state = nullptr;
    napi_value enabled = nullptr;
    int32_t value = -1;
    ASSERT_EQ(napi_get_named_property(env_, exports, "LaunchServiceState", &state), napi_ok);
    ASSERT_EQ(napi_get_named_property(env_, state, "ENABLED", &enabled), napi_ok);
    ASSERT_EQ(napi_get_value_int32(env_, enabled, &value), napi_ok);
    EXPECT_EQ(value, 1);
    EXPECT_EQ(napi_close_handle_scope(env_, scope), napi_ok);
}
} // namespace OHOS::AbilityRuntime
