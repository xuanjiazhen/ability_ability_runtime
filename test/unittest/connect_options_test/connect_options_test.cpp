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

#include "connect_options.h"
#include "connect_options_impl.h"

using namespace testing::ext;

namespace {

static void OnConnectCallback(OH_AbilityRuntime_ConnectOptions *opts,
    AbilityBase_Element *element, OHIPCRemoteProxy *proxy) {}

static void OnDisconnectCallback(OH_AbilityRuntime_ConnectOptions *opts,
    AbilityBase_Element *element) {}

static void OnFailedCallback(OH_AbilityRuntime_ConnectOptions *opts, AbilityRuntime_ErrorCode code) {}

} // namespace

class ConnectOptionsTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp() override {}
    void TearDown() override {}
};

// ==================== CreateConnectOptions ====================

HWTEST_F(ConnectOptionsTest, CreateConnectOptions_001, TestSize.Level1)
{
    auto *opts = OH_AbilityRuntime_CreateConnectOptions();
    ASSERT_NE(opts, nullptr);
    EXPECT_NE(opts->state, nullptr);
    EXPECT_TRUE(opts->state->alive);
    EXPECT_EQ(opts->state->owner, opts);
    EXPECT_EQ(opts->state->onConnectCallback, nullptr);
    EXPECT_EQ(opts->state->onDisconnectCallback, nullptr);
    EXPECT_EQ(opts->state->onFailedCallback, nullptr);
    OH_AbilityRuntime_DestroyConnectOptions(opts);
}

// ==================== DestroyConnectOptions ====================

HWTEST_F(ConnectOptionsTest, DestroyConnectOptions_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_DestroyConnectOptions(nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ConnectOptionsTest, DestroyConnectOptions_002, TestSize.Level1)
{
    OH_AbilityRuntime_ConnectOptions opts;
    opts.state = nullptr;
    auto ret = OH_AbilityRuntime_DestroyConnectOptions(&opts);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ConnectOptionsTest, DestroyConnectOptions_003, TestSize.Level1)
{
    auto *opts = OH_AbilityRuntime_CreateConnectOptions();
    ASSERT_NE(opts, nullptr);
    auto ret = OH_AbilityRuntime_DestroyConnectOptions(opts);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

// ==================== SetOnConnectCallback ====================

HWTEST_F(ConnectOptionsTest, SetOnConnectCallback_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnConnectCallback(nullptr, OnConnectCallback);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ConnectOptionsTest, SetOnConnectCallback_002, TestSize.Level1)
{
    auto *opts = OH_AbilityRuntime_CreateConnectOptions();
    ASSERT_NE(opts, nullptr);
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnConnectCallback(opts, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    OH_AbilityRuntime_DestroyConnectOptions(opts);
}

HWTEST_F(ConnectOptionsTest, SetOnConnectCallback_003, TestSize.Level1)
{
    OH_AbilityRuntime_ConnectOptions opts;
    opts.state = nullptr;
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnConnectCallback(&opts, OnConnectCallback);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ConnectOptionsTest, SetOnConnectCallback_004, TestSize.Level1)
{
    auto *opts = OH_AbilityRuntime_CreateConnectOptions();
    ASSERT_NE(opts, nullptr);
    opts->state->alive = false;
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnConnectCallback(opts, OnConnectCallback);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    OH_AbilityRuntime_DestroyConnectOptions(opts);
}

HWTEST_F(ConnectOptionsTest, SetOnConnectCallback_005, TestSize.Level1)
{
    auto *opts = OH_AbilityRuntime_CreateConnectOptions();
    ASSERT_NE(opts, nullptr);
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnConnectCallback(opts, OnConnectCallback);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(opts->state->onConnectCallback, OnConnectCallback);
    OH_AbilityRuntime_DestroyConnectOptions(opts);
}

// ==================== SetOnDisconnectCallback ====================

HWTEST_F(ConnectOptionsTest, SetOnDisconnectCallback_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnDisconnectCallback(nullptr, OnDisconnectCallback);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ConnectOptionsTest, SetOnDisconnectCallback_002, TestSize.Level1)
{
    auto *opts = OH_AbilityRuntime_CreateConnectOptions();
    ASSERT_NE(opts, nullptr);
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnDisconnectCallback(opts, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    OH_AbilityRuntime_DestroyConnectOptions(opts);
}

HWTEST_F(ConnectOptionsTest, SetOnDisconnectCallback_003, TestSize.Level1)
{
    OH_AbilityRuntime_ConnectOptions opts;
    opts.state = nullptr;
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnDisconnectCallback(&opts, OnDisconnectCallback);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ConnectOptionsTest, SetOnDisconnectCallback_004, TestSize.Level1)
{
    auto *opts = OH_AbilityRuntime_CreateConnectOptions();
    ASSERT_NE(opts, nullptr);
    opts->state->alive = false;
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnDisconnectCallback(opts, OnDisconnectCallback);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    OH_AbilityRuntime_DestroyConnectOptions(opts);
}

HWTEST_F(ConnectOptionsTest, SetOnDisconnectCallback_005, TestSize.Level1)
{
    auto *opts = OH_AbilityRuntime_CreateConnectOptions();
    ASSERT_NE(opts, nullptr);
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnDisconnectCallback(opts, OnDisconnectCallback);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(opts->state->onDisconnectCallback, OnDisconnectCallback);
    OH_AbilityRuntime_DestroyConnectOptions(opts);
}

// ==================== SetOnFailedCallback ====================

HWTEST_F(ConnectOptionsTest, SetOnFailedCallback_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnFailedCallback(nullptr, OnFailedCallback);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ConnectOptionsTest, SetOnFailedCallback_002, TestSize.Level1)
{
    auto *opts = OH_AbilityRuntime_CreateConnectOptions();
    ASSERT_NE(opts, nullptr);
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnFailedCallback(opts, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    OH_AbilityRuntime_DestroyConnectOptions(opts);
}

HWTEST_F(ConnectOptionsTest, SetOnFailedCallback_003, TestSize.Level1)
{
    OH_AbilityRuntime_ConnectOptions opts;
    opts.state = nullptr;
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnFailedCallback(&opts, OnFailedCallback);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ConnectOptionsTest, SetOnFailedCallback_004, TestSize.Level1)
{
    auto *opts = OH_AbilityRuntime_CreateConnectOptions();
    ASSERT_NE(opts, nullptr);
    opts->state->alive = false;
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnFailedCallback(opts, OnFailedCallback);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    OH_AbilityRuntime_DestroyConnectOptions(opts);
}

HWTEST_F(ConnectOptionsTest, SetOnFailedCallback_005, TestSize.Level1)
{
    auto *opts = OH_AbilityRuntime_CreateConnectOptions();
    ASSERT_NE(opts, nullptr);
    auto ret = OH_AbilityRuntime_ConnectOptions_SetOnFailedCallback(opts, OnFailedCallback);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_EQ(opts->state->onFailedCallback, OnFailedCallback);
    OH_AbilityRuntime_DestroyConnectOptions(opts);
}
