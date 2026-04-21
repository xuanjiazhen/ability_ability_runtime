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

#include "modular_object_extension_manager.h"
#include "ability_manager/include/modular_object_extension_info.h"
#include "connect_options.h"
#include "connect_options_impl.h"

using namespace testing::ext;

class ModularObjectExtensionManagerConnectTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

// ==================== Connect - Parameter Validation ====================

HWTEST_F(ModularObjectExtensionManagerConnectTest, Connect_NullConnectOptions_001, TestSize.Level1)
{
    int64_t connectionId = 0;
    auto ret = OH_AbilityRuntime_ConnectModularObjectExtensionAbility(nullptr, nullptr, &connectionId);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionManagerConnectTest, Connect_NullConnectionId_001, TestSize.Level1)
{
    OH_AbilityRuntime_ConnectOptions *options = OH_AbilityRuntime_CreateConnectOptions();
    ASSERT_NE(options, nullptr);
    auto ret = OH_AbilityRuntime_ConnectModularObjectExtensionAbility(nullptr, options, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    OH_AbilityRuntime_DestroyConnectOptions(options);
}

HWTEST_F(ModularObjectExtensionManagerConnectTest, Connect_NullState_001, TestSize.Level1)
{
    OH_AbilityRuntime_ConnectOptions options;
    options.state = nullptr;
    int64_t connectionId = 0;
    auto ret = OH_AbilityRuntime_ConnectModularObjectExtensionAbility(nullptr, &options, &connectionId);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionManagerConnectTest, Connect_StateNotAlive_001, TestSize.Level1)
{
    OH_AbilityRuntime_ConnectOptions *options = OH_AbilityRuntime_CreateConnectOptions();
    ASSERT_NE(options, nullptr);
    ASSERT_NE(options->state, nullptr);
    options->state->alive = false;
    int64_t connectionId = 0;
    auto ret = OH_AbilityRuntime_ConnectModularObjectExtensionAbility(nullptr, options, &connectionId);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    OH_AbilityRuntime_DestroyConnectOptions(options);
}

// ==================== Disconnect - Connection Not Found ====================

HWTEST_F(ModularObjectExtensionManagerConnectTest, Disconnect_ConnectionNotFound_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_DisconnectModularObjectExtensionAbility(-1);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionManagerConnectTest, Disconnect_ConnectionNotFound_002, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_DisconnectModularObjectExtensionAbility(99999);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ==================== ReleaseAllExtensionInfos ====================

HWTEST_F(ModularObjectExtensionManagerConnectTest, ReleaseAllExtensionInfos_Null_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_ReleaseAllExtensionInfos(nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

HWTEST_F(ModularObjectExtensionManagerConnectTest, ReleaseAllExtensionInfos_NullPointer_001, TestSize.Level1)
{
    OH_AbilityRuntime_AllModObjExtensionInfosHandle handle = nullptr;
    auto ret = OH_AbilityRuntime_ReleaseAllExtensionInfos(&handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
}

// ==================== GetCount null checks ====================

HWTEST_F(ModularObjectExtensionManagerConnectTest, GetCount_Null_001, TestSize.Level1)
{
    size_t count = 0;
    auto ret = OH_AbilityRuntime_GetCountFromAllModObjExtensionInfos(nullptr, &count);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionManagerConnectTest, GetCount_NullCount_001, TestSize.Level1)
{
    struct AllInfos {
        int dummy;
    };
    AllInfos infos;
    auto ret = OH_AbilityRuntime_GetCountFromAllModObjExtensionInfos(
        reinterpret_cast<OH_AbilityRuntime_AllModObjExtensionInfosHandle>(&infos), nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ==================== GetModObjExtensionInfoByIndex null checks ====================

HWTEST_F(ModularObjectExtensionManagerConnectTest, GetByIndex_Null_001, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjExtensionInfoHandle handle = nullptr;
    auto ret = OH_AbilityRuntime_GetModObjExtensionInfoByIndex(nullptr, 0, &handle);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionManagerConnectTest, GetByIndex_NullOutHandle_001, TestSize.Level1)
{
    struct AllInfos {
        int dummy;
    };
    AllInfos infos;
    auto ret = OH_AbilityRuntime_GetModObjExtensionInfoByIndex(
        reinterpret_cast<OH_AbilityRuntime_AllModObjExtensionInfosHandle>(&infos), 0, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ==================== AcquireSelf null check ====================

HWTEST_F(ModularObjectExtensionManagerConnectTest, AcquireSelf_Null_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_AcquireSelfModularObjectExtensionInfos(nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ==================== GetLaunchMode/ProcessMode/ThreadMode null checks ====================

HWTEST_F(ModularObjectExtensionManagerConnectTest, GetLaunchMode_Null_001, TestSize.Level1)
{
    OH_AbilityRuntime_LaunchMode mode;
    auto ret = OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode(nullptr, &mode);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionManagerConnectTest, GetLaunchMode_NullMode_001, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjExtensionInfoHandle handle = reinterpret_cast<OH_AbilityRuntime_ModObjExtensionInfoHandle>(1);
    auto ret = OH_AbilityRuntime_GetModularObjectExtensionInfoLaunchMode(handle, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionManagerConnectTest, GetProcessMode_Null_001, TestSize.Level1)
{
    OH_AbilityRuntime_ProcessMode mode;
    auto ret = OH_AbilityRuntime_GetModularObjectExtensionInfoProcessMode(nullptr, &mode);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionManagerConnectTest, GetThreadMode_Null_001, TestSize.Level1)
{
    OH_AbilityRuntime_ThreadMode mode;
    auto ret = OH_AbilityRuntime_GetModularObjectExtensionInfoThreadMode(nullptr, &mode);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ==================== GetElementName null checks ====================

HWTEST_F(ModularObjectExtensionManagerConnectTest, GetElementName_Null_001, TestSize.Level1)
{
    AbilityBase_Element element;
    auto ret = OH_AbilityRuntime_GetModularObjectExtensionInfoElementName(nullptr, &element);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionManagerConnectTest, GetElementName_NullElement_001, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjExtensionInfoHandle handle = reinterpret_cast<OH_AbilityRuntime_ModObjExtensionInfoHandle>(1);
    auto ret = OH_AbilityRuntime_GetModularObjectExtensionInfoElementName(handle, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

// ==================== GetDisableState null checks ====================

HWTEST_F(ModularObjectExtensionManagerConnectTest, GetDisableState_Null_001, TestSize.Level1)
{
    bool isDisabled = false;
    auto ret = OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState(nullptr, &isDisabled);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionManagerConnectTest, GetDisableState_NullOut_001, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjExtensionInfoHandle handle =
        reinterpret_cast<OH_AbilityRuntime_ModObjExtensionInfoHandle>(1);
    auto ret = OH_AbilityRuntime_GetModularObjectExtensionInfoDisableState(handle, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}
