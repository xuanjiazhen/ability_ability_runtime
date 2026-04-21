/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 */

#ifndef MOCK_MODULAR_OBJECT_EXTENSION_TYPES_H
#define MOCK_MODULAR_OBJECT_EXTENSION_TYPES_H

#include <memory>
#include "extension_ability_info.h"
#include "modular_object_extension_ability.h"
#include "native_extension/context_impl.h"
#include "native_extension/extension_ability_impl.h"

#ifdef __cplusplus
extern "C" {
#endif

struct OH_AbilityRuntime_ModularObjectExtensionContext : public AbilityRuntime_Context {};

struct OH_AbilityRuntime_ModularObjectExtensionInstance : public AbilityRuntime_ExtensionInstance {
    std::shared_ptr<OH_AbilityRuntime_ModularObjectExtensionContext> context;
    OH_AbilityRuntime_ModObjExtensionAbility_OnCreateFunc onCreateFunc = nullptr;
    OH_AbilityRuntime_ModObjExtensionAbility_OnDestroyFunc onDestroyFunc = nullptr;
    OH_AbilityRuntime_ModObjExtensionAbility_OnConnectFunc onConnectFunc = nullptr;
    OH_AbilityRuntime_ModObjExtensionAbility_OnDisconnectFunc onDisconnectFunc = nullptr;
};

#ifdef __cplusplus
}
#endif

#endif
