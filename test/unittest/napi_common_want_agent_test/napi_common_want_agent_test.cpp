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

#include "js_environment.h"
#include "js_runtime_lite.h"
#include "napi_common_want_agent.h"
#include "napi/native_api.h"
#include "runtime.h"

namespace OHOS {
namespace AppExecFwk {
using namespace testing;
using namespace testing::ext;

class NapiCommonWantAgentTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void NapiCommonWantAgentTest::SetUpTestCase()
{
}

void NapiCommonWantAgentTest::TearDownTestCase()
{
}

void NapiCommonWantAgentTest::SetUp()
{
}

void NapiCommonWantAgentTest::TearDown()
{
}

/**
 * @tc.number: WrapWantAgent_0100
 * @tc.name: WrapWantAgent_0100
 * @tc.desc: WrapWantAgent.
 */
HWTEST_F(NapiCommonWantAgentTest, WrapWantAgent_0100, Function | MediumTest | Level1)
{
    OHOS::AbilityRuntime::Runtime::Options options;
    std::shared_ptr<OHOS::JsEnv::JsEnvironment> jsEnv = nullptr;
    auto err = AbilityRuntime::JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv);
    EXPECT_EQ(err, napi_status::napi_ok);
    napi_env env = reinterpret_cast<napi_env>(jsEnv->GetNativeEngine());
    EXPECT_NE(env, nullptr);

    std::shared_ptr<AbilityRuntime::WantAgent::WantAgent> wantAgentPtr =
        std::make_shared<AbilityRuntime::WantAgent::WantAgent>();
    AbilityRuntime::WantAgent::WantAgent* wantAgent = wantAgentPtr.get();
    EXPECT_NE(wantAgent, nullptr);
    auto callBack = [](napi_env env, void* data, void* hint) -> void {};
    napi_finalize finalizeCb = static_cast<napi_finalize>(callBack);
    EXPECT_NE(finalizeCb, nullptr);

    auto test = OHOS::AppExecFwk::WrapWantAgent(env, wantAgent, finalizeCb);
    EXPECT_NE(test, nullptr);

    err = AbilityRuntime::JsRuntimeLite::GetInstance().RemoveJsEnv(reinterpret_cast<napi_env>(jsEnv->GetNativeEngine()));
    EXPECT_EQ(err, napi_status::napi_ok);
    wantAgent = nullptr;
}

/**
 * @tc.number: UnwrapWantAgent_0100
 * @tc.name: UnwrapWantAgent_0100
 * @tc.desc: UnwrapWantAgent.
 */
HWTEST_F(NapiCommonWantAgentTest, UnwrapWantAgent_0100, Function | MediumTest | Level1)
{
    napi_env env = nullptr;
    napi_value jsParam = nullptr;
    void **result = nullptr;
    OHOS::AppExecFwk::UnwrapWantAgent(env, jsParam, result);
}
} // namespace AppExecFwk
} // namespace OHOS

