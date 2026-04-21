/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 */

#include <gtest/gtest.h>

#include "modular_object_extension_ability.h"
#include "modular_object_extension_types.h"

using namespace testing::ext;

namespace {

static void MockOnCreateFunc(OH_AbilityRuntime_ModObjExtensionInstanceHandle instance, AbilityBase_Want *want) {}
static void MockOnDestroyFunc(OH_AbilityRuntime_ModObjExtensionInstanceHandle instance) {}
static OHIPCRemoteStub *MockOnConnectFunc(OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    AbilityBase_Want *want) { return nullptr; }
static void MockOnDisconnectFunc(OH_AbilityRuntime_ModObjExtensionInstanceHandle instance) {}

OH_AbilityRuntime_ModObjExtensionInstanceHandle CreateValidInstance()
{
    auto *inst = new OH_AbilityRuntime_ModularObjectExtensionInstance();
    inst->type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    inst->context = std::make_shared<OH_AbilityRuntime_ModularObjectExtensionContext>();
    return inst;
}

void DestroyInstance(OH_AbilityRuntime_ModObjExtensionInstanceHandle inst)
{
    delete reinterpret_cast<OH_AbilityRuntime_ModularObjectExtensionInstance *>(inst);
}

} // namespace

class ModularObjectExtensionAbilityTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

// ==================== RegisterOnCreateFunc ====================

HWTEST_F(ModularObjectExtensionAbilityTest, RegisterOnCreateFunc_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnCreateFunc(nullptr, MockOnCreateFunc);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionAbilityTest, RegisterOnCreateFunc_002, TestSize.Level1)
{
    auto *inst = CreateValidInstance();
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnCreateFunc(inst, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    DestroyInstance(inst);
}

HWTEST_F(ModularObjectExtensionAbilityTest, RegisterOnCreateFunc_003, TestSize.Level1)
{
    auto *inst = CreateValidInstance();
    inst->type = OHOS::AppExecFwk::ExtensionAbilityType::SERVICE;
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnCreateFunc(inst, MockOnCreateFunc);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE);
    DestroyInstance(inst);
}

HWTEST_F(ModularObjectExtensionAbilityTest, RegisterOnCreateFunc_004, TestSize.Level1)
{
    auto *inst = CreateValidInstance();
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnCreateFunc(inst, MockOnCreateFunc);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_NE(inst->onCreateFunc, nullptr);
    DestroyInstance(inst);
}

// ==================== RegisterOnDestroyFunc ====================

HWTEST_F(ModularObjectExtensionAbilityTest, RegisterOnDestroyFunc_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnDestroyFunc(nullptr, MockOnDestroyFunc);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionAbilityTest, RegisterOnDestroyFunc_002, TestSize.Level1)
{
    auto *inst = CreateValidInstance();
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnDestroyFunc(inst, MockOnDestroyFunc);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_NE(inst->onDestroyFunc, nullptr);
    DestroyInstance(inst);
}

// ==================== RegisterOnConnectFunc ====================

HWTEST_F(ModularObjectExtensionAbilityTest, RegisterOnConnectFunc_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnConnectFunc(nullptr, MockOnConnectFunc);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionAbilityTest, RegisterOnConnectFunc_002, TestSize.Level1)
{
    auto *inst = CreateValidInstance();
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnConnectFunc(inst, MockOnConnectFunc);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_NE(inst->onConnectFunc, nullptr);
    DestroyInstance(inst);
}

// ==================== RegisterOnDisconnectFunc ====================

HWTEST_F(ModularObjectExtensionAbilityTest, RegisterOnDisconnectFunc_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnDisconnectFunc(nullptr, MockOnDisconnectFunc);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionAbilityTest, RegisterOnDisconnectFunc_002, TestSize.Level1)
{
    auto *inst = CreateValidInstance();
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnDisconnectFunc(inst, MockOnDisconnectFunc);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_NE(inst->onDisconnectFunc, nullptr);
    DestroyInstance(inst);
}

// ==================== GetContextFromInstance ====================

HWTEST_F(ModularObjectExtensionAbilityTest, GetContextFromInstance_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_GetContextFromInstance(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionAbilityTest, GetContextFromInstance_002, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjExtensionContextHandle ctx = nullptr;
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_GetContextFromInstance(nullptr, &ctx);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionAbilityTest, GetContextFromInstance_003, TestSize.Level1)
{
    auto *inst = CreateValidInstance();
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_GetContextFromInstance(inst, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
    DestroyInstance(inst);
}

HWTEST_F(ModularObjectExtensionAbilityTest, GetContextFromInstance_004, TestSize.Level1)
{
    auto *inst = CreateValidInstance();
    inst->type = OHOS::AppExecFwk::ExtensionAbilityType::SERVICE;
    OH_AbilityRuntime_ModObjExtensionContextHandle ctx = nullptr;
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_GetContextFromInstance(inst, &ctx);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE);
    DestroyInstance(inst);
}

HWTEST_F(ModularObjectExtensionAbilityTest, GetContextFromInstance_005, TestSize.Level1)
{
    auto *inst = CreateValidInstance();
    inst->context = nullptr;
    OH_AbilityRuntime_ModObjExtensionContextHandle ctx = nullptr;
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_GetContextFromInstance(inst, &ctx);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_CONTEXT_NOT_EXIST);
    DestroyInstance(inst);
}

HWTEST_F(ModularObjectExtensionAbilityTest, GetContextFromInstance_006, TestSize.Level1)
{
    auto *inst = CreateValidInstance();
    OH_AbilityRuntime_ModObjExtensionContextHandle ctx = nullptr;
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_GetContextFromInstance(inst, &ctx);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_NE(ctx, nullptr);
    DestroyInstance(inst);
}

// ==================== GetInstanceFromBase ====================

HWTEST_F(ModularObjectExtensionAbilityTest, GetInstanceFromBase_001, TestSize.Level1)
{
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_GetInstanceFromBase(nullptr, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionAbilityTest, GetInstanceFromBase_002, TestSize.Level1)
{
    OH_AbilityRuntime_ModObjExtensionInstanceHandle out = nullptr;
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_GetInstanceFromBase(nullptr, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionAbilityTest, GetInstanceFromBase_003, TestSize.Level1)
{
    AbilityRuntime_ExtensionInstance base;
    base.type = OHOS::AppExecFwk::ExtensionAbilityType::SERVICE;
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_GetInstanceFromBase(&base, nullptr);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID);
}

HWTEST_F(ModularObjectExtensionAbilityTest, GetInstanceFromBase_004, TestSize.Level1)
{
    AbilityRuntime_ExtensionInstance base;
    base.type = OHOS::AppExecFwk::ExtensionAbilityType::SERVICE;
    OH_AbilityRuntime_ModObjExtensionInstanceHandle out = nullptr;
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_GetInstanceFromBase(&base, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_INCORRECT_ABILITY_TYPE);
}

HWTEST_F(ModularObjectExtensionAbilityTest, GetInstanceFromBase_005, TestSize.Level1)
{
    AbilityRuntime_ExtensionInstance base;
    base.type = OHOS::AppExecFwk::ExtensionAbilityType::MODULAR_OBJECT;
    OH_AbilityRuntime_ModObjExtensionInstanceHandle out = nullptr;
    auto ret = OH_AbilityRuntime_ModObjExtensionAbility_GetInstanceFromBase(&base, &out);
    EXPECT_EQ(ret, ABILITY_RUNTIME_ERROR_CODE_NO_ERROR);
    EXPECT_NE(out, nullptr);
}
