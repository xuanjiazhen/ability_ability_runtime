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
#include <gtest/hwext/gtest-multithread.h>

#include "connect_server_manager.h"
#include "hilog_tag_wrapper.h"
#include "js_environment.h"
#include "js_runtime_common.h"
#include "js_runtime_lite.h"
#include "native_engine/native_engine.h"

using namespace testing;
using namespace testing::ext;
using namespace testing::mt;

namespace OHOS {
namespace AbilityRuntime {

class JsRuntimeCommonTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void JsRuntimeCommonTest::SetUpTestCase() {}

void JsRuntimeCommonTest::TearDownTestCase() {}

void JsRuntimeCommonTest::SetUp() {}

void JsRuntimeCommonTest::TearDown() {}

/**
 * @tc.name: IsDebugModeTest_0100
 * @tc.desc: IsDebugModeTest
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeCommonTest, IsDebugModeTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsDebugModeTest_0100 start";
    JsRuntimeCommon::GetInstance().SetDebugMode(true);
    EXPECT_TRUE(JsRuntimeCommon::GetInstance().IsDebugMode());
    GTEST_LOG_(INFO) << "IsDebugModeTest_0100 end";
}

/**
 * @tc.name: IsDebugAppTest_0100
 * @tc.desc: IsDebugAppTest
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeCommonTest, IsDebugAppTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsDebugAppTest_0100 start";
    JsRuntimeCommon::GetInstance().SetDebugApp(true);
    EXPECT_TRUE(JsRuntimeCommon::GetInstance().IsDebugApp());
    GTEST_LOG_(INFO) << "IsDebugAppTest_0100 end";
}

/**
 * @tc.name: IsNativeStartTest_0100
 * @tc.desc: IsNativeStartTest
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeCommonTest, IsNativeStartTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "IsNativeStartTest_0100 start";
    JsRuntimeCommon::GetInstance().SetNativeStart(true);
    EXPECT_TRUE(JsRuntimeCommon::GetInstance().IsNativeStart());
    GTEST_LOG_(INFO) << "IsNativeStartTest_0100 end";
}

/**
 * @tc.name: StartDebuggerModuleTest_0100
 * @tc.desc: StartDebuggerModuleTest
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeCommonTest, StartDebuggerModuleTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDebuggerModuleTest_0100 start";
    JsRuntimeCommon::GetInstance().StartDebuggerModule(true, true);
    EXPECT_TRUE(JsRuntimeCommon::GetInstance().IsDebugMode());
    EXPECT_TRUE(JsRuntimeCommon::GetInstance().IsDebugApp());
    EXPECT_TRUE(JsRuntimeCommon::GetInstance().IsNativeStart());
    GTEST_LOG_(INFO) << "StartDebuggerModuleTest_0100 end";
}

/**
 * @tc.name: StartDebugModeTest_0100
 * @tc.desc: StartDebugModeTest
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeCommonTest, StartDebugModeTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StartDebuggerModuleTest_0100 start";
    auto ret = napi_status::napi_invalid_arg;
    EXPECT_EQ(ret, JsRuntimeCommon::GetInstance().StartDebugMode(nullptr, "thread"));
    JsRuntimeCommon::GetInstance().SetNativeStart(false);

    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    auto nativeEngine = jsEnv->GetNativeEngine();
    JsRuntimeCommon::GetInstance().StartDebugMode(nativeEngine, "thread");
    JsRuntimeCommon::GetInstance().SetNativeStart(true);
    ret = napi_status::napi_ok;
    EXPECT_EQ(ret, JsRuntimeCommon::GetInstance().StartDebugMode(nativeEngine, "thread"));
    EXPECT_EQ(ret, JsRuntimeCommon::GetInstance().StopDebugMode(nativeEngine));
    GTEST_LOG_(INFO) << "StartDebuggerModuleTest_0100 end";
}

/**
 * @tc.name: StopDebugModeTest_0100
 * @tc.desc: StopDebugModeTest
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeCommonTest, StopDebugModeTest_0100, TestSize.Level1)
{
    GTEST_LOG_(INFO) << "StopDebugModeTest_0100 start";
    auto ret = napi_status::napi_invalid_arg;
    EXPECT_EQ(ret, JsRuntimeCommon::GetInstance().StopDebugMode(nullptr));
    GTEST_LOG_(INFO) << "StopDebugModeTest_0100 end";
}

/**
 * @tc.name: IsDebugModeTest_0200
 * @tc.desc: SetDebugMode(false) and verify IsDebugMode returns false.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeCommonTest, IsDebugModeTest_0200, TestSize.Level1)
{
    JsRuntimeCommon::GetInstance().SetDebugMode(false);
    EXPECT_FALSE(JsRuntimeCommon::GetInstance().IsDebugMode());
}

/**
 * @tc.name: IsDebugAppTest_0200
 * @tc.desc: SetDebugApp(false) and verify IsDebugApp returns false.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeCommonTest, IsDebugAppTest_0200, TestSize.Level1)
{
    JsRuntimeCommon::GetInstance().SetDebugApp(false);
    EXPECT_FALSE(JsRuntimeCommon::GetInstance().IsDebugApp());
}

/**
 * @tc.name: IsNativeStartTest_0200
 * @tc.desc: SetNativeStart(false) and verify IsNativeStart returns false.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeCommonTest, IsNativeStartTest_0200, TestSize.Level1)
{
    JsRuntimeCommon::GetInstance().SetNativeStart(false);
    EXPECT_FALSE(JsRuntimeCommon::GetInstance().IsNativeStart());
}

/**
 * @tc.name: StartDebuggerModuleTest_0200
 * @tc.desc: StartDebuggerModule(false,false) forces debugMode_ true (invariant).
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeCommonTest, StartDebuggerModuleTest_0200, TestSize.Level1)
{
    JsRuntimeCommon::GetInstance().SetDebugMode(false);
    JsRuntimeCommon::GetInstance().StartDebuggerModule(false, false);
    EXPECT_TRUE(JsRuntimeCommon::GetInstance().IsDebugMode());
    EXPECT_FALSE(JsRuntimeCommon::GetInstance().IsDebugApp());
    EXPECT_FALSE(JsRuntimeCommon::GetInstance().IsNativeStart());
}

/**
 * @tc.name: StartDebuggerModuleTest_0300
 * @tc.desc: StartDebuggerModule(true,false) forces debugMode_ true (invariant).
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeCommonTest, StartDebuggerModuleTest_0300, TestSize.Level1)
{
    JsRuntimeCommon::GetInstance().SetDebugMode(false);
    JsRuntimeCommon::GetInstance().StartDebuggerModule(true, false);
    EXPECT_TRUE(JsRuntimeCommon::GetInstance().IsDebugMode());
    EXPECT_TRUE(JsRuntimeCommon::GetInstance().IsDebugApp());
    EXPECT_FALSE(JsRuntimeCommon::GetInstance().IsNativeStart());
}

/**
 * @tc.name: StartDebuggerModuleTest_0400
 * @tc.desc: StartDebuggerModule(false,true) forces debugMode_ true (invariant).
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeCommonTest, StartDebuggerModuleTest_0400, TestSize.Level1)
{
    JsRuntimeCommon::GetInstance().SetDebugMode(false);
    JsRuntimeCommon::GetInstance().StartDebuggerModule(false, true);
    EXPECT_TRUE(JsRuntimeCommon::GetInstance().IsDebugMode());
    EXPECT_FALSE(JsRuntimeCommon::GetInstance().IsDebugApp());
    EXPECT_TRUE(JsRuntimeCommon::GetInstance().IsNativeStart());
}

/**
 * @tc.name: StartDebugModeTest_0200
 * @tc.desc: StartDebugMode is idempotent (repeat call returns napi_ok).
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeCommonTest, StartDebugModeTest_0200, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    auto nativeEngine = jsEnv->GetNativeEngine();
    EXPECT_EQ(napi_status::napi_ok, JsRuntimeCommon::GetInstance().StartDebugMode(nativeEngine, "thread"));
    EXPECT_EQ(napi_status::napi_ok, JsRuntimeCommon::GetInstance().StartDebugMode(nativeEngine, "thread"));
    JsRuntimeCommon::GetInstance().StopDebugMode(nativeEngine);
}

/**
 * @tc.name: StopDebugModeTest_0200
 * @tc.desc: StopDebugMode without prior Start (orphan) returns napi_ok without crash.
 * @tc.type: FUNC
 */
HWTEST_F(JsRuntimeCommonTest, StopDebugModeTest_0200, TestSize.Level1)
{
    AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    auto nativeEngine = jsEnv->GetNativeEngine();
    EXPECT_EQ(napi_status::napi_ok, JsRuntimeCommon::GetInstance().StopDebugMode(nativeEngine));
}
} // namespace AbilityRuntime
} // namespace OHOS
