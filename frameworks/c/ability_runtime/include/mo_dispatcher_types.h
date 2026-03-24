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

#ifndef ABILITY_RUNTIME_MO_DISPATCHER_TYPES_H
#define ABILITY_RUNTIME_MO_DISPATCHER_TYPES_H

#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "modular_object_dispatcher.h"

namespace OHOS::AbilityRuntime {
class MoDispatcherMetadataManager;

struct MoStructFieldMeta {
    std::string name;
    OH_AbilityRuntime_MoDispatcher_Vt type = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    uint32_t memberId = 0;
};

struct MoStructMeta {
    std::string name;
    uint32_t memberId = 0;
    std::vector<MoStructFieldMeta> fields;
};

struct MoMethodParamMeta {
    std::string name;
    OH_AbilityRuntime_MoDispatcher_Vt type = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
};

struct MoMethodMeta {
    std::string interfaceName;
    std::string name;
    uint32_t memberId = 0;
    uint32_t ipcCode = 0;
    bool oneway = false;
    OH_AbilityRuntime_MoDispatcher_Vt returnType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID;
    std::vector<MoMethodParamMeta> params;
};

struct MoEnumValueMeta {
    std::string name;
    int32_t value = 0;
    uint32_t memberId = 0;
};

struct MoEnumMeta {
    std::string name;
    uint32_t memberId = 0;
    std::vector<MoEnumValueMeta> values;
};

struct MoInterfaceMeta {
    std::string name;
    std::string descriptor;
    uint32_t memberId = 0;
    bool isCallback = false;
    bool isMainService = false;
    std::string descriptorJson;
    std::vector<MoMethodMeta> methods;
};

struct MoVariantStorage {
    OH_AbilityRuntime_MoDispatcher_Variant value {};
    std::string stringStorage;
};
} // namespace OHOS::AbilityRuntime

struct OH_AbilityRuntime_MoDispatcher {
    OHIPCRemoteProxy* remoteProxy = nullptr;
    std::shared_ptr<OHOS::AbilityRuntime::MoDispatcherMetadataManager> metadataManager;
};

struct OH_AbilityRuntime_MoDispatcher_TypeDescriptor {
    std::shared_ptr<OHOS::AbilityRuntime::MoDispatcherMetadataManager> metadataManager;
};

struct OH_AbilityRuntime_MoDispatcher_Array {
    OH_AbilityRuntime_MoDispatcher_Vt elementType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    std::vector<OHOS::AbilityRuntime::MoVariantStorage> elements;
};

struct OH_AbilityRuntime_MoDispatcher_Vector {
    OH_AbilityRuntime_MoDispatcher_Vt elementType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    std::vector<OHOS::AbilityRuntime::MoVariantStorage> elements;
};

struct OH_AbilityRuntime_MoDispatcher_Set {
    OH_AbilityRuntime_MoDispatcher_Vt elementType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    std::vector<OHOS::AbilityRuntime::MoVariantStorage> elements;
};

struct OH_AbilityRuntime_MoDispatcher_Map {
    OH_AbilityRuntime_MoDispatcher_Vt keyType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    OH_AbilityRuntime_MoDispatcher_Vt valueType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    std::vector<std::pair<OHOS::AbilityRuntime::MoVariantStorage, OHOS::AbilityRuntime::MoVariantStorage>> entries;
};

struct OH_AbilityRuntime_MoDispatcher_Struct {
    std::string name;
    std::unordered_map<std::string, OH_AbilityRuntime_MoDispatcher_Vt> fieldTypes;
    std::unordered_map<std::string, OHOS::AbilityRuntime::MoVariantStorage> fields;
};

#endif // ABILITY_RUNTIME_MO_DISPATCHER_TYPES_H
