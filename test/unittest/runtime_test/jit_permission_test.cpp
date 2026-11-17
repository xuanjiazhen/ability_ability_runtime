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

#include "accesstoken_kit.h"
#include "ipc_skeleton.h"
#include "js_runtime.h"
#include "js_runtime_lite.h"
#include "jsnapi.h"

using namespace testing::ext;

namespace OHOS::AbilityRuntime::Test {
namespace {
struct JitAclState {
    uint64_t token = 123;
    int result = 0;
    std::vector<int32_t> states {0, 0};
    int tokenReads = 0;
    int queries = 0;
    int vmCreateCalls = 0;
    bool enabled = false;
};
JitAclState g_jitAclState;
} // namespace
} // namespace OHOS::AbilityRuntime::Test

// Test-only replacements, following the runtime_test/mock_jsnapi.cpp convention.
namespace OHOS {
decltype(IPCSkeleton::GetSelfTokenID()) IPCSkeleton::GetSelfTokenID()
{
    ++AbilityRuntime::Test::g_jitAclState.tokenReads;
    return static_cast<decltype(IPCSkeleton::GetSelfTokenID())>(AbilityRuntime::Test::g_jitAclState.token);
}
} // namespace OHOS

namespace OHOS::Security::AccessToken {
int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenId, const std::vector<std::string>& permissions,
    std::vector<int32_t>& states, bool crossIpc)
{
    auto& testState = AbilityRuntime::Test::g_jitAclState;
    const std::vector<std::string> expected = {
        "ohos.permission.kernel.ALLOW_EXECUTABLE_FORT_MEMORY",
        "ohos.permission.kernel.ALLOW_EXECUTABLE_FORT_MEMORY_FOR_ARKTS"
    };
    EXPECT_EQ(tokenId, static_cast<AccessTokenID>(testState.token));
    EXPECT_EQ(permissions, expected);
    EXPECT_TRUE(crossIpc);
    ++testState.queries;
    states = testState.states;
    return testState.result;
}
} // namespace OHOS::Security::AccessToken

namespace panda {
EcmaVM* JSNApi::CreateJSVM(const RuntimeOption& option)
{
    auto& testState = OHOS::AbilityRuntime::Test::g_jitAclState;
    ++testState.vmCreateCalls;
    // RuntimeOption declares JSNApi as a friend, so this private getter is accessible here.
    testState.enabled = option.GetEnableJIT();
    return nullptr; // Capture the configuration without creating a real VM.
}
} // namespace panda

namespace OHOS::AbilityRuntime::Test {
class JitPermissionTest : public testing::Test {
public:
    void SetUp() override
    {
        g_jitAclState = JitAclState {};
    }

    void CheckJitOption(bool expected, bool requested = true, bool preload = false)
    {
        Runtime::Options options;
        options.jitEnabled = requested;
        options.preload = preload;
        options.loadAce = false;
        for (bool useLite : {false, true}) {
            SCOPED_TRACE(useLite ? "JsRuntimeLite" : "JsRuntime");
            bool checkPermissions = useLite || !preload;
            bool expectedEnabled = checkPermissions ? expected : requested;
            g_jitAclState.queries = 0;
            g_jitAclState.tokenReads = 0;
            g_jitAclState.vmCreateCalls = 0;
            g_jitAclState.enabled = !expectedEnabled;
            // The VM mock intentionally fails after capturing the option in both paths.
            if (useLite) {
                std::shared_ptr<JsEnv::JsEnvironment> jsEnv;
                EXPECT_EQ(JsRuntimeLite::GetInstance().CreateJsEnv(options, jsEnv), napi_generic_failure);
            } else {
                JsRuntime runtime;
                EXPECT_FALSE(runtime.Init(options));
            }
            ASSERT_EQ(g_jitAclState.vmCreateCalls, 1);
            EXPECT_EQ(g_jitAclState.enabled, expectedEnabled);
            EXPECT_EQ(g_jitAclState.queries, checkPermissions && g_jitAclState.token != 0 ? 1 : 0);
            EXPECT_EQ(g_jitAclState.tokenReads, checkPermissions ? 1 : 0);
        }
    }
};

// Both permissions are required; either denial must disable JIT.
HWTEST_F(JitPermissionTest, BothPermissionsRequired, TestSize.Level1)
{
    for (const auto& states : std::vector<std::vector<int32_t>> {{0, 0}, {-1, 0}, {0, -1}, {-1, -1}}) {
        SCOPED_TRACE(testing::PrintToString(states));
        g_jitAclState.states = states;
        CheckJitOption(states == std::vector<int32_t>({0, 0}));
    }
}

// Permissions must not override the original framework policy.
HWTEST_F(JitPermissionTest, OriginalJitDisabled, TestSize.Level1)
{
    CheckJitOption(false, false);
}

// A failed query is denied even if it also returns granted states.
HWTEST_F(JitPermissionTest, QueryFailed, TestSize.Level1)
{
    g_jitAclState.result = -1;
    CheckJitOption(false);
}

// Invalid tokens must be denied without querying the permission service.
HWTEST_F(JitPermissionTest, InvalidToken, TestSize.Level1)
{
    g_jitAclState.token = 0;
    CheckJitOption(false);
}

// Missing/extra results and unknown states must fail closed.
HWTEST_F(JitPermissionTest, InvalidPermissionResults, TestSize.Level1)
{
    for (const auto& states : std::vector<std::vector<int32_t>> {{}, {0}, {0, 0, 0}, {0, 1}}) {
        SCOPED_TRACE(testing::PrintToString(states));
        g_jitAclState.states = states;
        CheckJitOption(false);
    }
}

// Only the ordinary runtime skips preload checks; Lite always checks the current application token.
HWTEST_F(JitPermissionTest, PreloadPermissionPolicy, TestSize.Level1)
{
    for (bool requested : {false, true}) {
        SCOPED_TRACE(requested);
        g_jitAclState.token = 123;
        g_jitAclState.result = 0;
        g_jitAclState.states = {0, 0};
        CheckJitOption(requested, requested, true);

        g_jitAclState.states = {0, -1};
        CheckJitOption(false, requested, true);

        g_jitAclState.states = {0, 0};
        g_jitAclState.result = -1;
        CheckJitOption(false, requested, true);

        g_jitAclState.token = 0;
        g_jitAclState.result = 0;
        CheckJitOption(false, requested, true);
    }
}
} // namespace OHOS::AbilityRuntime::Test
