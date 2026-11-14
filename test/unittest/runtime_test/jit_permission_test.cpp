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
#define private public
#define protected public
#include "js_environment.h"
#include "js_runtime.h"
#undef private
#undef protected

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
};
JitAclState state;
} // namespace
} // namespace OHOS::AbilityRuntime::Test

// These replacements are linked only into jit_permission_test, like mock_jsnapi.cpp in runtime_test.
namespace OHOS {
decltype(IPCSkeleton::GetSelfTokenID()) IPCSkeleton::GetSelfTokenID()
{
    ++AbilityRuntime::Test::state.tokenReads;
    return static_cast<decltype(IPCSkeleton::GetSelfTokenID())>(AbilityRuntime::Test::state.token);
}
} // namespace OHOS

namespace OHOS::Security::AccessToken {
int AccessTokenKit::VerifyAccessToken(AccessTokenID tokenId, const std::vector<std::string>& permissions,
    std::vector<int32_t>& states, bool crossIpc)
{
    auto& state = AbilityRuntime::Test::state;
    const std::vector<std::string> expected = {
        "ohos.permission.kernel.ALLOW_EXECUTABLE_FORT_MEMORY",
        "ohos.permission.kernel.ALLOW_EXECUTABLE_FORT_MEMORY_FOR_ARKTS"
    };
    EXPECT_EQ(tokenId, static_cast<AccessTokenID>(state.token));
    EXPECT_EQ(permissions, expected);
    EXPECT_TRUE(crossIpc);
    ++state.queries;
    states = state.states;
    return state.result;
}
} // namespace OHOS::Security::AccessToken

namespace OHOS::JsEnv {
bool JsEnvironment::Initialize(const panda::RuntimeOption& option, void*)
{
    auto& state = AbilityRuntime::Test::state;
    ++state.configurations;
    state.enabled = option.GetEnableJIT();
    // Capture the option without creating a VM; CreateJsEnv is expected to report initialization failure.
    return false;
}
} // namespace OHOS::JsEnv

namespace panda {
void JSNApi::PostFork(EcmaVM*, const RuntimeOption& option)
{
    auto& state = OHOS::AbilityRuntime::Test::state;
    ++state.configurations;
    state.enabled = option.GetEnableJIT();
}
} // namespace panda

namespace OHOS::AbilityRuntime::Test {
class JitPermissionTest : public testing::Test {
public:
    void SetUp() override
    {
        state = JitAclState {};
    }

    void CheckFresh(bool expected, bool requested = true, bool preload = false)
    {
        Runtime::Options options;
        options.jitEnabled = requested;
        options.preload = preload;
        options.loadAce = false;
        state.queries = 0;
        state.tokenReads = 0;
        state.configurations = 0;
        JsRuntime runtime;
        EXPECT_FALSE(runtime.CreateJsEnv(options)); // The environment mock deliberately stops after capture.
        ASSERT_EQ(state.configurations, 1); // Fails if the test replacement is not used by the runtime library.
        EXPECT_EQ(state.enabled, expected);
        EXPECT_EQ(state.queries, !preload && state.token != 0 ? 1 : 0);
        EXPECT_EQ(state.tokenReads, preload ? 0 : 1);
    }

    void CheckPostPreload(bool expected, bool requested = true)
    {
        JsRuntime runtime;
        // Supply a real VM/engine so PostPreload passes its pointer checks and can reinitialize its UV loop.
        runtime.jsEnv_ = std::make_shared<JsEnv::JsEnvironment>();
        panda::RuntimeOption option;
        option.SetEnableJIT(false);
        runtime.jsEnv_->vm_ = panda::JSNApi::CreateJSVM(option);
        ASSERT_NE(runtime.jsEnv_->vm_, nullptr);
        runtime.jsEnv_->engine_ = new ArkNativeEngine(runtime.jsEnv_->vm_, nullptr);
        Runtime::Options options;
        options.jitEnabled = requested;
        options.loadAce = false;
        state.queries = 0;
        state.tokenReads = 0;
        state.configurations = 0;
        runtime.PostPreload(options);
        ASSERT_EQ(state.configurations, 1);
        EXPECT_EQ(state.enabled, expected);
        EXPECT_EQ(state.queries, state.token != 0 ? 1 : 0);
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
    state.states = {-1, 0};
    CheckFresh(false);
}

/**
 * @tc.name: ArkTsPermissionDenied
 * @tc.desc: The new ArkTS permission is required.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, ArkTsPermissionDenied, TestSize.Level1)
{
    state.states = {0, -1};
    CheckFresh(false);
}

/**
 * @tc.name: BothPermissionsDenied
 * @tc.desc: No permission must disable JIT.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, BothPermissionsDenied, TestSize.Level1)
{
    state.states = {-1, -1};
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
    state.result = -1;
    CheckFresh(false);
}

/**
 * @tc.name: EmptyResult
 * @tc.desc: Empty results fail closed.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, EmptyResult, TestSize.Level1)
{
    state.states.clear();
    CheckFresh(false);
}

/**
 * @tc.name: MissingResult
 * @tc.desc: A missing result fails closed.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, MissingResult, TestSize.Level1)
{
    state.states = {0};
    CheckFresh(false);
}

/**
 * @tc.name: ExtraResult
 * @tc.desc: An extra result fails closed.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, ExtraResult, TestSize.Level1)
{
    state.states = {0, 0, 0};
    CheckFresh(false);
}

/**
 * @tc.name: UnexpectedState
 * @tc.desc: Only PERMISSION_GRANTED is accepted.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, UnexpectedState, TestSize.Level1)
{
    state.states = {0, 1};
    CheckFresh(false);
}

/**
 * @tc.name: InvalidToken
 * @tc.desc: An invalid token is denied without querying AccessToken.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, InvalidToken, TestSize.Level1)
{
    state.token = 0;
    CheckFresh(false);
}

/**
 * @tc.name: PreloadEnabled
 * @tc.desc: Preload skips the application permission check.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, PreloadEnabled, TestSize.Level1)
{
    state.token = 0;
    state.result = -1;
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
    state.states = {0, -1};
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
    state.states = {0, -1};
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
    state.states = {0, -1};
    CheckFresh(false);
    state.states = {0, 0};
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
    state.token = 456;
    state.states = {-1, 0};
    CheckFresh(false);
}

/**
 * @tc.name: AccessTokenIdWidth
 * @tc.desc: The IPC return is converted to the AccessToken API type.
 * @tc.type: FUNC
 */
HWTEST_F(JitPermissionTest, AccessTokenIdWidth, TestSize.Level1)
{
    state.token = (uint64_t {1} << 32) | 123;
    CheckFresh(true);
}

} // namespace OHOS::AbilityRuntime::Test
