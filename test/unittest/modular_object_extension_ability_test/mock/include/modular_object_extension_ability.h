/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 */

#ifndef MOCK_MODULAR_OBJECT_EXTENSION_ABILITY_H
#define MOCK_MODULAR_OBJECT_EXTENSION_ABILITY_H

#include "ability_runtime_common.h"
#include "extension_ability.h"
#include "ipc_cparcel.h"
#include "modular_object_extension_context.h"
#include "want.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct OH_AbilityRuntime_ModularObjectExtensionInstance OH_AbilityRuntime_ModObjExtensionInstance;
typedef OH_AbilityRuntime_ModObjExtensionInstance *OH_AbilityRuntime_ModObjExtensionInstanceHandle;

typedef void (*OH_AbilityRuntime_ModObjExtensionAbility_OnCreateFunc)(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance, AbilityBase_Want *want);
typedef void (*OH_AbilityRuntime_ModObjExtensionAbility_OnDestroyFunc)(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance);
typedef OHIPCRemoteStub *(*OH_AbilityRuntime_ModObjExtensionAbility_OnConnectFunc)(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance, AbilityBase_Want *want);
typedef void (*OH_AbilityRuntime_ModObjExtensionAbility_OnDisconnectFunc)(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance);

AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnCreateFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnCreateFunc onCreateFunc);
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnDestroyFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnDestroyFunc onDestroyFunc);
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnConnectFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnConnectFunc onConnectFunc);
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_RegisterOnDisconnectFunc(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionAbility_OnDisconnectFunc onDisconnectFunc);
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_GetContextFromInstance(
    OH_AbilityRuntime_ModObjExtensionInstanceHandle instance,
    OH_AbilityRuntime_ModObjExtensionContextHandle *context);
AbilityRuntime_ErrorCode OH_AbilityRuntime_ModObjExtensionAbility_GetInstanceFromBase(
    AbilityRuntime_ExtensionInstanceHandle baseExtensionInstance,
    OH_AbilityRuntime_ModObjExtensionInstanceHandle *modObjExtensionInstance);

#ifdef __cplusplus
}
#endif

#endif
