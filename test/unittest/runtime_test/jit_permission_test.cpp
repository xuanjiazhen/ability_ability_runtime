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
#include "ark_native_engine.h"
#include "ipc_skeleton.h"
#include "js_environment.h"
#include "js_runtime.h"

using namespace testing::ext;

namespace OHOS::AbilityRuntime::Test {
namespace {
struct JitAclState {
    uint64_t token = 123;
    int result = 0;
    std::vector<int32_t> states {0, 0};
    int tokenReads = 0;
    int queries = 0;
    int configurations = 0;
    bool enabled = false;
    bool createRealEnvironment = false;
    int postForkCalls = 0;
};
JitAclState g_jitAclState;
} // namespace
} // namespace OHOS::AbilityRuntime::Test

// These replacements are linked only into jit_permission_test, like mock_jsnapi.cpp in runtime_test.
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

namespace OHOS::JsEnv {
bool JsEnvironment::Initialize(const panda::RuntimeOption& option, void* jsEngine)
{
    auto& testState = AbilityRuntime::Test::g_jitAclState;
    ++testState.configurations;
    testState.enabled = option.GetEnableJIT();
    // Fresh-environment tests stop after capture; reuse tests need a real preloaded VM/engine.
    if (!testState.createRealEnvironment) {
        return false;
    }
    vm_ = panda::JSNApi::CreateJSVM(option);
    if (vm_ == nullptr) {
        return false;
    }
    engine_ = new ArkNativeEngine(vm_, jsEngine);
    return true;
}
} // namespace OHOS::JsEnv

namespace panda {
void JSNApi::PostFork(EcmaVM*, const RuntimeOption& option)
{
    auto& testState = OHOS::AbilityRuntime::Test::g_jitAclState;
    ++testState.configurations;
    ++testState.postForkCalls;
    testState.enabled = option.GetEnableJIT();
}
} // namespace panda

namespace OHOS::AbilityRuntime::Test {
class JitPermissionTest : public testing::Test {
public:
    void SetUp() override
    {
        g_jitAclState = JitAclState {};
    }

    void CheckFresh(bool expected, bool requested = true, bool preload = false)
    {
        Runtime::Options options;
        options.jitEnabled = requested;
        options.preload = preload;
        options.loadAce = false;
        g_jitAclState.queries = 0;
        g_jitAclState.tokenReads = 0;
        g_jitAclState.configurations = 0;
        JsRuntime runtime;
        EXPECT_FALSE(runtime.Init(options)); // The environment mock deliberately stops after capture.
        // Fails if the test replacement is not used by the runtime library.
        ASSERT_EQ(g_jitAclState.configurations, 1);
        EXPECT_EQ(g_jitAclState.enabled, expected);
        EXPECT_EQ(g_jitAclState.queries, !preload && g_jitAclState.token != 0 ? 1 : 0);
        EXPECT_EQ(g_jitAclState.tokenReads, preload ? 0 : 1);
    }

    void CheckPostPreload(bool expected, bool requested = true)
    {
        JsRuntime runtime;
        // Prepare preload through the public entry point, without accessing runtime private members.
        Runtime::Options preloadOptions;
        preloadOptions.preload = true;
        preloadOptions.isStageModel = false;
        preloadOptions.loadAce = false;
        g_jitAclState.createRealEnvironment = true;
        ASSERT_TRUE(runtime.Init(preloadOptions));
        g_jitAclState.createRealEnvironment = false;
        Runtime::Options options;
        options.jitEnabled = requested;
        options.loadAce = false;
        g_jitAclState.queries = 0;
        g_jitAclState.tokenReads = 0;
        g_jitAclState.configurations = 0;
        g_jitAclState.postForkCalls = 0;
        // Check the captured PostFork option; later application setup is outside this test's scope.
        (void)runtime.Init(options);
        ASSERT_EQ(g_jitAclState.postForkCalls, 1);
        ASSERT_EQ(g_jitAclState.configurations, 1);
        EXPECT_EQ(g_jitAclState.enabled, expected);
        EXPECT_EQ(g_jitAclState.queries, g_jitAclState.token != 0 ? 1 : 0);
    }
};

/**
 * @tc.name: BothPermissionsGranted
 * @tc.desc: Both grants allow the original JIT request.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, BothPermissionsGranted, TestSize.Level1)
{
    CheckFresh(true);
}

/**
 * @tc.name: OldPermissionDenied
 * @tc.desc: The existing executable memory permission is required.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, OldPermissionDenied, TestSize.Level1)
{
    g_jitAclState.states = {-1, 0};
    CheckFresh(false);
}

/**
 * @tc.name: ArkTsPermissionDenied
 * @tc.desc: The new ArkTS permission is required.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, ArkTsPermissionDenied, TestSize.Level1)
{
    g_jitAclState.states = {0, -1};
    CheckFresh(false);
}

/**
 * @tc.name: BothPermissionsDenied
 * @tc.desc: No permission must disable JIT.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, BothPermissionsDenied, TestSize.Level1)
{
    g_jitAclState.states = {-1, -1};
    CheckFresh(false);
}

/**
 * @tc.name: OriginalJitDisabled
 * @tc.desc: Permissions cannot override a disabled framework switch.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, OriginalJitDisabled, TestSize.Level1)
{
    CheckFresh(false, false);
}

/**
 * @tc.name: QueryFailedWithGrantedStates
 * @tc.desc: An unsuccessful query cannot enable JIT.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, QueryFailedWithGrantedStates, TestSize.Level1)
{
    g_jitAclState.result = -1;
    CheckFresh(false);
}

/**
 * @tc.name: EmptyResult
 * @tc.desc: Empty results fail closed.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, EmptyResult, TestSize.Level1)
{
    g_jitAclState.states.clear();
    CheckFresh(false);
}

/**
 * @tc.name: MissingResult
 * @tc.desc: A missing result fails closed.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, MissingResult, TestSize.Level1)
{
    g_jitAclState.states = {0};
    CheckFresh(false);
}

/**
 * @tc.name: ExtraResult
 * @tc.desc: An extra result fails closed.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, ExtraResult, TestSize.Level1)
{
    g_jitAclState.states = {0, 0, 0};
    CheckFresh(false);
}

/**
 * @tc.name: UnexpectedState
 * @tc.desc: Only PERMISSION_GRANTED is accepted.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, UnexpectedState, TestSize.Level1)
{
    g_jitAclState.states = {0, 1};
    CheckFresh(false);
}

/**
 * @tc.name: InvalidToken
 * @tc.desc: An invalid token is denied without querying AccessToken.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, InvalidToken, TestSize.Level1)
{
    g_jitAclState.token = 0;
    CheckFresh(false);
}

/**
 * @tc.name: PreloadEnabled
 * @tc.desc: Preload skips the application permission check.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, PreloadEnabled, TestSize.Level1)
{
    g_jitAclState.token = 0;
    g_jitAclState.result = -1;
    CheckFresh(true, true, true);
}

/**
 * @tc.name: PreloadDisabled
 * @tc.desc: Preload preserves a disabled JIT switch.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, PreloadDisabled, TestSize.Level1)
{
    CheckFresh(false, false, true);
}

/**
 * @tc.name: PostPreloadGranted
 * @tc.desc: The reused environment receives the granted JIT option.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, PostPreloadGranted, TestSize.Level1)
{
    CheckPostPreload(true);
}

/**
 * @tc.name: PostPreloadDenied
 * @tc.desc: The reused environment receives a disabled JIT option when a permission is missing.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, PostPreloadDenied, TestSize.Level1)
{
    g_jitAclState.states = {0, -1};
    CheckPostPreload(false);
}

/**
 * @tc.name: PreloadThenApplication
 * @tc.desc: Application permissions are checked after preload.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, PreloadThenApplication, TestSize.Level1)
{
    CheckFresh(true, true, true);
    g_jitAclState.states = {0, -1};
    CheckFresh(false);
}

/**
 * @tc.name: PermissionsNotCached
 * @tc.desc: Revocation and restoration are observed on initialization.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, PermissionsNotCached, TestSize.Level1)
{
    CheckFresh(true);
    g_jitAclState.states = {0, -1};
    CheckFresh(false);
    g_jitAclState.states = {0, 0};
    CheckFresh(true);
}

/**
 * @tc.name: TokenNotCached
 * @tc.desc: Each initialization uses the current process token.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, TokenNotCached, TestSize.Level1)
{
    CheckFresh(true);
    g_jitAclState.token = 456;
    g_jitAclState.states = {-1, 0};
    CheckFresh(false);
}

/**
 * @tc.name: AccessTokenIdWidth
 * @tc.desc: The IPC return is converted to the AccessToken API type.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, AccessTokenIdWidth, TestSize.Level1)
{
    g_jitAclState.token = (uint64_t {1} << 32) | 123;
    CheckFresh(true);
}

} // namespace OHOS::AbilityRuntime::Test
