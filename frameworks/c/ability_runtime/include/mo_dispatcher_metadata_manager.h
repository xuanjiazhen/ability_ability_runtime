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

#ifndef ABILITY_RUNTIME_MO_DISPATCHER_METADATA_MANAGER_H
#define ABILITY_RUNTIME_MO_DISPATCHER_METADATA_MANAGER_H

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

#include "mo_dispatcher_types.h"

namespace OHOS::AbilityRuntime {
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

class MoDispatcherMetadataManager {
public:
    static constexpr uint32_t IPC_CODE_GET_TLB_FD = 0x00FFFF;

    AbilityRuntime_ErrorCode EnsureLoaded(OHIPCRemoteProxy* remoteProxy, bool* sendRequestFailed = nullptr);

    AbilityRuntime_ErrorCode QueryMemberIds(const char** names, uint32_t count, uint32_t* memberIds) const;
    AbilityRuntime_ErrorCode QueryMemberId(const std::string& name, uint32_t* memberId) const;
    AbilityRuntime_ErrorCode QueryMemberName(uint32_t memberId, std::string* name) const;

    AbilityRuntime_ErrorCode GetMethodMeta(uint32_t memberId, MoMethodMeta* methodMeta) const;

    AbilityRuntime_ErrorCode GetPackage(std::string* packageName) const;
    AbilityRuntime_ErrorCode GetVersion(std::string* version) const;
    AbilityRuntime_ErrorCode GetMainServiceInterfaceName(std::string* interfaceName) const;

    AbilityRuntime_ErrorCode GetInterfaceCount(uint32_t* count) const;
    AbilityRuntime_ErrorCode GetInterfaceName(uint32_t index, std::string* name) const;
    AbilityRuntime_ErrorCode GetInterfaceMemberId(const std::string& interfaceName, uint32_t* memberId) const;
    AbilityRuntime_ErrorCode GetInterfaceDescriptor(const std::string& interfaceName, std::string* descriptor) const;
    AbilityRuntime_ErrorCode GetInterfaceIsCallback(const std::string& interfaceName, bool* isCallback) const;

    AbilityRuntime_ErrorCode GetEnumCount(uint32_t* count) const;
    AbilityRuntime_ErrorCode GetEnumName(uint32_t index, std::string* name) const;
    AbilityRuntime_ErrorCode GetEnumMemberId(const std::string& enumName, uint32_t* memberId) const;
    AbilityRuntime_ErrorCode GetEnumValueCount(const std::string& enumName, uint32_t* count) const;
    AbilityRuntime_ErrorCode GetEnumValueName(const std::string& enumName, uint32_t index, std::string* valueName) const;
    AbilityRuntime_ErrorCode GetEnumValue(const std::string& enumName, const std::string& valueName, int32_t* value) const;

    AbilityRuntime_ErrorCode GetStructCount(uint32_t* count) const;
    AbilityRuntime_ErrorCode GetStructName(uint32_t index, std::string* name) const;
    AbilityRuntime_ErrorCode GetStructMemberId(const std::string& structName, uint32_t* memberId) const;
    AbilityRuntime_ErrorCode GetStructFieldCount(const std::string& structName, uint32_t* count) const;
    AbilityRuntime_ErrorCode GetStructFieldName(const std::string& structName, uint32_t index, std::string* fieldName) const;
    AbilityRuntime_ErrorCode GetStructFieldType(const std::string& structName, const std::string& fieldName,
        OH_AbilityRuntime_MoDispatcher_Vt* fieldType) const;

    static AbilityRuntime_ErrorCode GetMethodCountFromDescriptor(const char* descriptorJson,
        const char* interfaceName, uint32_t* count);
    static AbilityRuntime_ErrorCode GetMethodNameFromDescriptor(const char* descriptorJson,
        const char* interfaceName, uint32_t index, std::string* methodName);
    static AbilityRuntime_ErrorCode GetMethodMemberIdFromDescriptor(const char* descriptorJson,
        const char* interfaceName, const char* methodName, uint32_t* memberId);
    static AbilityRuntime_ErrorCode GetMethodReturnTypeFromDescriptor(const char* descriptorJson,
        const char* interfaceName, const char* methodName, OH_AbilityRuntime_MoDispatcher_Vt* returnType);
    static AbilityRuntime_ErrorCode GetMethodParamCountFromDescriptor(const char* descriptorJson,
        const char* interfaceName, const char* methodName, uint32_t* count);
    static AbilityRuntime_ErrorCode GetMethodParamTypeFromDescriptor(const char* descriptorJson,
        const char* interfaceName, const char* methodName, uint32_t paramIndex,
        OH_AbilityRuntime_MoDispatcher_Vt* paramType);
    static AbilityRuntime_ErrorCode GetMethodParamNameFromDescriptor(const char* descriptorJson,
        const char* interfaceName, const char* methodName, uint32_t paramIndex, std::string* paramName);

private:
    static OH_AbilityRuntime_MoDispatcher_Vt ParseTypeInfo(const std::string& typeName);

    AbilityRuntime_ErrorCode ParseMetadata(const std::string& jsonText);
    AbilityRuntime_ErrorCode RequestMetadataJson(OHIPCRemoteProxy* remoteProxy, std::string* jsonText,
        bool* sendRequestFailed);

    bool loaded_ = false;
    std::string package_;
    std::string version_;
    std::string mainServiceInterface_;

    std::vector<MoInterfaceMeta> interfaces_;
    std::vector<MoEnumMeta> enums_;
    std::vector<MoStructMeta> structs_;

    std::unordered_map<std::string, uint32_t> nameToMemberId_;
    std::unordered_map<uint32_t, std::string> memberIdToName_;
    std::unordered_map<uint32_t, MoMethodMeta> memberIdToMethod_;

    mutable std::mutex mutex_;
};
} // namespace OHOS::AbilityRuntime

#endif // ABILITY_RUNTIME_MO_DISPATCHER_METADATA_MANAGER_H
