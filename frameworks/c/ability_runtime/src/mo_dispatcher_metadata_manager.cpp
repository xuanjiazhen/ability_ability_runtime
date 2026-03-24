
/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "mo_dispatcher_metadata_manager.h"

#include <algorithm>
#include <cctype>
#include <optional>
#include <unistd.h>

#include "ipc_cremote_object.h"
#include "ipc_error_code.h"
#include "mo_dispatcher_complex_type_manager.h"
#include "nlohmann/json.hpp"

namespace OHOS::AbilityRuntime {
namespace {
using Json = nlohmann::json;

bool ReadAllFromFd(int32_t fd, std::string* out)
{
    if (fd < 0 || out == nullptr) {
        return false;
    }
    out->clear();
    char buffer[4096] = {0};
    ssize_t n = 0;
    while ((n = read(fd, buffer, sizeof(buffer))) > 0) {
        out->append(buffer, static_cast<size_t>(n));
    }
    close(fd);
    return n == 0;
}

std::string ToLower(const std::string& v)
{
    std::string out = v;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return out;
}

void InsertNameMap(std::unordered_map<std::string, uint32_t>& map, const std::string& key, uint32_t value)
{
    if (!key.empty() && map.find(key) == map.end()) {
        map.emplace(key, value);
    }
}

void InsertIdMap(std::unordered_map<uint32_t, std::string>& map, uint32_t key, const std::string& value)
{
    if (!value.empty() && map.find(key) == map.end()) {
        map.emplace(key, value);
    }
}

uint32_t GetDispId(const Json& obj)
{
    if (!obj.is_object() || !obj.contains("dispID")) {
        return 0;
    }
    if (obj["dispID"].is_number_unsigned()) {
        return obj["dispID"].get<uint32_t>();
    }
    if (obj["dispID"].is_number_integer()) {
        return static_cast<uint32_t>(obj["dispID"].get<int32_t>());
    }
    return 0;
}

std::optional<Json> FindInterface(const Json& descriptor, const std::string& interfaceName)
{
    auto match = [&interfaceName](const Json& node) {
        return node.is_object() && node.contains("name") && node["name"].is_string() &&
            (interfaceName.empty() || node["name"].get<std::string>() == interfaceName);
    };

    if (descriptor.is_object() && descriptor.contains("interfaces") && descriptor["interfaces"].is_array()) {
        for (const auto& item : descriptor["interfaces"]) {
            if (match(item)) {
                return item;
            }
        }
        return std::nullopt;
    }
    if (descriptor.is_array()) {
        for (const auto& item : descriptor) {
            if (match(item)) {
                return item;
            }
        }
        return std::nullopt;
    }
    if (match(descriptor)) {
        return descriptor;
    }
    return std::nullopt;
}

std::optional<Json> FindMethod(const Json& interfaceObj, const std::string& methodName)
{
    if (!interfaceObj.is_object() || !interfaceObj.contains("methods") || !interfaceObj["methods"].is_array()) {
        return std::nullopt;
    }
    for (const auto& method : interfaceObj["methods"]) {
        if (method.is_object() && method.contains("name") && method["name"].is_string() &&
            method["name"].get<std::string>() == methodName) {
            return method;
        }
    }
    return std::nullopt;
}
} // namespace
OH_AbilityRuntime_MoDispatcher_Vt MoDispatcherMetadataManager::ParseTypeInfo(const std::string& typeName)
{
    const std::string type = ToLower(typeName);
    if (type == "void") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID;
    if (type == "bool" || type == "boolean") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL;
    if (type == "i8" || type == "int8") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I8;
    if (type == "i16" || type == "int16") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I16;
    if (type == "i32" || type == "int32" || type == "enum") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32;
    if (type == "i64" || type == "int64") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64;
    if (type == "u8" || type == "uint8") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U8;
    if (type == "u16" || type == "uint16") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U16;
    if (type == "u32" || type == "uint32") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U32;
    if (type == "u64" || type == "uint64") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U64;
    if (type == "f32" || type == "float") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F32;
    if (type == "f64" || type == "double") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64;
    if (type == "string") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING;
    if (type == "array") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY;
    if (type == "vector") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR;
    if (type == "set") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET;
    if (type == "map") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP;
    if (type == "struct") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT;
    if (type == "interface") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY;
    return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::EnsureLoaded(OHIPCRemoteProxy* remoteProxy, bool* sendRequestFailed)
{
    if (sendRequestFailed != nullptr) {
        *sendRequestFailed = false;
    }
    if (remoteProxy == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (loaded_) {
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    std::string jsonText;
    auto ret = RequestMetadataJson(remoteProxy, &jsonText, sendRequestFailed);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    ret = ParseMetadata(jsonText);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    loaded_ = true;
    MoDispatcherComplexTypeManager::RegisterStructMetadata(structs_);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::RequestMetadataJson(OHIPCRemoteProxy* remoteProxy,
    std::string* jsonText, bool* sendRequestFailed)
{
    if (remoteProxy == nullptr || jsonText == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    OHIPCParcel* dataParcel = OH_IPCParcel_Create();
    OHIPCParcel* replyParcel = OH_IPCParcel_Create();
    if (dataParcel == nullptr || replyParcel == nullptr) {
        if (dataParcel != nullptr) {
            OH_IPCParcel_Destroy(dataParcel);
        }
        if (replyParcel != nullptr) {
            OH_IPCParcel_Destroy(replyParcel);
        }
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    OH_IPC_MessageOption option { .mode = OH_IPC_REQUEST_MODE_SYNC, .timeout = 0, .reserved = nullptr };
    int ipcRet = OH_IPCRemoteProxy_SendRequest(remoteProxy, IPC_CODE_GET_TLB_FD, dataParcel, replyParcel, &option);
    if (ipcRet != OH_IPC_SUCCESS) {
        if (sendRequestFailed != nullptr) {
            *sendRequestFailed = true;
        }
        OH_IPCParcel_Destroy(dataParcel);
        OH_IPCParcel_Destroy(replyParcel);
        return ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED;
    }

    int32_t fd = -1;
    bool loadedFromFd = false;
    if (OH_IPCParcel_ReadFileDescriptor(replyParcel, &fd) == OH_IPC_SUCCESS && fd >= 0) {
        loadedFromFd = ReadAllFromFd(fd, jsonText);
    }
    if (!loadedFromFd) {
        const char* jsonStr = OH_IPCParcel_ReadString(replyParcel);
        if (jsonStr == nullptr) {
            OH_IPCParcel_Destroy(dataParcel);
            OH_IPCParcel_Destroy(replyParcel);
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        *jsonText = jsonStr;
    }

    OH_IPCParcel_Destroy(dataParcel);
    OH_IPCParcel_Destroy(replyParcel);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}
AbilityRuntime_ErrorCode MoDispatcherMetadataManager::ParseMetadata(const std::string& jsonText)
{
    Json root = Json::parse(jsonText, nullptr, false);
    if (root.is_discarded() || !root.is_object()) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    interfaces_.clear();
    enums_.clear();
    structs_.clear();
    nameToMemberId_.clear();
    memberIdToName_.clear();
    memberIdToMethod_.clear();
    mainServiceInterface_.clear();

    package_ = root.value("package", "");
    if (root.contains("version")) {
        if (root["version"].is_string()) {
            version_ = root["version"].get<std::string>();
        } else if (root["version"].is_number_integer()) {
            version_ = std::to_string(root["version"].get<int32_t>());
        }
    }

    if (root.contains("enums") && root["enums"].is_array()) {
        for (const auto& enumObj : root["enums"]) {
            if (!enumObj.is_object()) {
                continue;
            }
            MoEnumMeta meta;
            meta.name = enumObj.value("name", "");
            meta.memberId = GetDispId(enumObj);
            InsertNameMap(nameToMemberId_, meta.name, meta.memberId);
            InsertIdMap(memberIdToName_, meta.memberId, meta.name);
            if (enumObj.contains("values") && enumObj["values"].is_array()) {
                for (const auto& valueObj : enumObj["values"]) {
                    MoEnumValueMeta value;
                    value.name = valueObj.value("name", "");
                    value.value = valueObj.value("value", 0);
                    value.memberId = GetDispId(valueObj);
                    meta.values.emplace_back(value);
                    InsertNameMap(nameToMemberId_, value.name, value.memberId);
                    InsertIdMap(memberIdToName_, value.memberId, value.name);
                }
            }
            enums_.emplace_back(std::move(meta));
        }
    }

    if (root.contains("structs") && root["structs"].is_array()) {
        for (const auto& structObj : root["structs"]) {
            if (!structObj.is_object()) {
                continue;
            }
            MoStructMeta meta;
            meta.name = structObj.value("name", "");
            meta.memberId = GetDispId(structObj);
            InsertNameMap(nameToMemberId_, meta.name, meta.memberId);
            InsertIdMap(memberIdToName_, meta.memberId, meta.name);
            if (structObj.contains("fields") && structObj["fields"].is_array()) {
                for (const auto& fieldObj : structObj["fields"]) {
                    MoStructFieldMeta field;
                    field.name = fieldObj.value("name", "");
                    field.memberId = GetDispId(fieldObj);
                    if (fieldObj.contains("type_info") && fieldObj["type_info"].is_object()) {
                        field.type = ParseTypeInfo(fieldObj["type_info"].value("type", ""));
                    }
                    meta.fields.emplace_back(field);
                    InsertNameMap(nameToMemberId_, field.name, field.memberId);
                    InsertIdMap(memberIdToName_, field.memberId, field.name);
                }
            }
            structs_.emplace_back(std::move(meta));
        }
    }

    if (root.contains("interfaces") && root["interfaces"].is_array()) {
        for (const auto& interfaceObj : root["interfaces"]) {
            if (!interfaceObj.is_object()) {
                continue;
            }
            MoInterfaceMeta interfaceMeta;
            interfaceMeta.name = interfaceObj.value("name", "");
            interfaceMeta.descriptor = interfaceObj.value("descriptor", "");
            interfaceMeta.memberId = GetDispId(interfaceObj);
            interfaceMeta.isCallback = interfaceObj.value("is_callback", false) || interfaceObj.value("isCallback", false);
            interfaceMeta.isMainService = interfaceObj.value("is_main_service", false) ||
                interfaceObj.value("main_service", false) || interfaceObj.value("isMainService", false);
            interfaceMeta.descriptorJson = interfaceObj.dump();
            InsertNameMap(nameToMemberId_, interfaceMeta.name, interfaceMeta.memberId);
            InsertIdMap(memberIdToName_, interfaceMeta.memberId, interfaceMeta.name);

            if (interfaceObj.contains("methods") && interfaceObj["methods"].is_array()) {
                for (const auto& methodObj : interfaceObj["methods"]) {
                    MoMethodMeta method;
                    method.interfaceName = interfaceMeta.name;
                    method.name = methodObj.value("name", "");
                    method.memberId = GetDispId(methodObj);
                    method.ipcCode = methodObj.value("code", 0);
                    method.oneway = methodObj.value("oneway", false);
                    if (methodObj.contains("return_type") && methodObj["return_type"].is_object()) {
                        method.returnType = ParseTypeInfo(methodObj["return_type"].value("type", ""));
                    }
                    if (methodObj.contains("parameters") && methodObj["parameters"].is_array()) {
                        for (const auto& paramObj : methodObj["parameters"]) {
                            MoMethodParamMeta param;
                            param.name = paramObj.value("name", "");
                            if (paramObj.contains("type_info") && paramObj["type_info"].is_object()) {
                                param.type = ParseTypeInfo(paramObj["type_info"].value("type", ""));
                            }
                            method.params.emplace_back(param);
                            const uint32_t paramMemberId = GetDispId(paramObj);
                            InsertNameMap(nameToMemberId_, param.name, paramMemberId);
                            InsertIdMap(memberIdToName_, paramMemberId, param.name);
                        }
                    }
                    interfaceMeta.methods.emplace_back(method);
                    memberIdToMethod_[method.memberId] = method;
                    InsertNameMap(nameToMemberId_, method.name, method.memberId);
                    InsertNameMap(nameToMemberId_, interfaceMeta.name + "." + method.name, method.memberId);
                    InsertIdMap(memberIdToName_, method.memberId, method.name);
                }
            }

            if (interfaceMeta.isMainService && mainServiceInterface_.empty()) {
                mainServiceInterface_ = interfaceMeta.name;
            }
            interfaces_.emplace_back(std::move(interfaceMeta));
        }
    }

    if (mainServiceInterface_.empty() && !interfaces_.empty()) {
        mainServiceInterface_ = interfaces_[0].name;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::QueryMemberIds(const char** names, uint32_t count,
    uint32_t* memberIds) const
{
    if (names == nullptr || memberIds == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    for (uint32_t i = 0; i < count; i++) {
        if (names[i] == nullptr) {
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        auto it = nameToMemberId_.find(names[i]);
        if (it == nameToMemberId_.end()) {
            return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
        }
        memberIds[i] = it->second;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::QueryMemberId(const std::string& name, uint32_t* memberId) const
{
    if (memberId == nullptr || name.empty()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    auto it = nameToMemberId_.find(name);
    if (it == nameToMemberId_.end()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *memberId = it->second;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::QueryMemberName(uint32_t memberId, std::string* name) const
{
    if (name == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    auto it = memberIdToName_.find(memberId);
    if (it == memberIdToName_.end()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *name = it->second;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetMethodMeta(uint32_t memberId, MoMethodMeta* methodMeta) const
{
    if (methodMeta == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    auto it = memberIdToMethod_.find(memberId);
    if (it == memberIdToMethod_.end()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *methodMeta = it->second;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}
AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetPackage(std::string* packageName) const
{
    if (packageName == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    *packageName = package_;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetVersion(std::string* version) const
{
    if (version == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    *version = version_;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetMainServiceInterfaceName(std::string* interfaceName) const
{
    if (interfaceName == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    if (mainServiceInterface_.empty()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *interfaceName = mainServiceInterface_;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetInterfaceCount(uint32_t* count) const
{
    if (count == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    *count = static_cast<uint32_t>(interfaces_.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetInterfaceName(uint32_t index, std::string* name) const
{
    if (name == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_ || index >= interfaces_.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *name = interfaces_[index].name;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetInterfaceMemberId(const std::string& interfaceName,
    uint32_t* memberId) const
{
    if (memberId == nullptr || interfaceName.empty()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return QueryMemberId(interfaceName, memberId);
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetInterfaceDescriptor(const std::string& interfaceName,
    std::string* descriptor) const
{
    if (descriptor == nullptr || interfaceName.empty()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    for (const auto& item : interfaces_) {
        if (item.name == interfaceName) {
            *descriptor = item.descriptorJson;
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetInterfaceIsCallback(const std::string& interfaceName,
    bool* isCallback) const
{
    if (isCallback == nullptr || interfaceName.empty()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    for (const auto& item : interfaces_) {
        if (item.name == interfaceName) {
            *isCallback = item.isCallback;
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetEnumCount(uint32_t* count) const
{
    if (count == nullptr) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    *count = static_cast<uint32_t>(enums_.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetEnumName(uint32_t index, std::string* name) const
{
    if (name == nullptr) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_ || index >= enums_.size()) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    *name = enums_[index].name;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetEnumMemberId(const std::string& enumName, uint32_t* memberId) const
{
    return QueryMemberId(enumName, memberId);
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetEnumValueCount(const std::string& enumName, uint32_t* count) const
{
    if (count == nullptr || enumName.empty()) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    for (const auto& item : enums_) {
        if (item.name == enumName) {
            *count = static_cast<uint32_t>(item.values.size());
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetEnumValueName(const std::string& enumName, uint32_t index,
    std::string* valueName) const
{
    if (valueName == nullptr || enumName.empty()) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    for (const auto& item : enums_) {
        if (item.name == enumName) {
            if (index >= item.values.size()) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
            *valueName = item.values[index].name;
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetEnumValue(const std::string& enumName,
    const std::string& valueName, int32_t* value) const
{
    if (value == nullptr || enumName.empty() || valueName.empty()) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    for (const auto& item : enums_) {
        if (item.name == enumName) {
            for (const auto& enumValue : item.values) {
                if (enumValue.name == valueName) {
                    *value = enumValue.value;
                    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
                }
            }
            return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetStructCount(uint32_t* count) const
{
    if (count == nullptr) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    *count = static_cast<uint32_t>(structs_.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetStructName(uint32_t index, std::string* name) const
{
    if (name == nullptr) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_ || index >= structs_.size()) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    *name = structs_[index].name;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetStructMemberId(const std::string& structName,
    uint32_t* memberId) const
{
    return QueryMemberId(structName, memberId);
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetStructFieldCount(const std::string& structName,
    uint32_t* count) const
{
    if (count == nullptr || structName.empty()) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    for (const auto& item : structs_) {
        if (item.name == structName) {
            *count = static_cast<uint32_t>(item.fields.size());
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetStructFieldName(const std::string& structName, uint32_t index,
    std::string* fieldName) const
{
    if (fieldName == nullptr || structName.empty()) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    for (const auto& item : structs_) {
        if (item.name == structName) {
            if (index >= item.fields.size()) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
            *fieldName = item.fields[index].name;
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetStructFieldType(const std::string& structName,
    const std::string& fieldName, OH_AbilityRuntime_MoDispatcher_Vt* fieldType) const
{
    if (fieldType == nullptr || structName.empty() || fieldName.empty()) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!loaded_) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    for (const auto& item : structs_) {
        if (item.name == structName) {
            for (const auto& field : item.fields) {
                if (field.name == fieldName) {
                    *fieldType = field.type;
                    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
                }
            }
            return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
}
AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetMethodCountFromDescriptor(const char* descriptorJson,
    const char* interfaceName, uint32_t* count)
{
    if (descriptorJson == nullptr || interfaceName == nullptr || count == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    Json descriptor = Json::parse(descriptorJson, nullptr, false);
    if (descriptor.is_discarded()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto interfaceObj = FindInterface(descriptor, interfaceName);
    if (!interfaceObj.has_value()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (!interfaceObj->contains("methods") || !(*interfaceObj)["methods"].is_array()) {
        *count = 0;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    *count = static_cast<uint32_t>((*interfaceObj)["methods"].size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetMethodNameFromDescriptor(const char* descriptorJson,
    const char* interfaceName, uint32_t index, std::string* methodName)
{
    if (descriptorJson == nullptr || interfaceName == nullptr || methodName == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    Json descriptor = Json::parse(descriptorJson, nullptr, false);
    if (descriptor.is_discarded()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto interfaceObj = FindInterface(descriptor, interfaceName);
    if (!interfaceObj.has_value() || !interfaceObj->contains("methods") || !(*interfaceObj)["methods"].is_array()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto& methods = (*interfaceObj)["methods"];
    if (index >= methods.size() || !methods[index].contains("name") || !methods[index]["name"].is_string()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *methodName = methods[index]["name"].get<std::string>();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetMethodMemberIdFromDescriptor(const char* descriptorJson,
    const char* interfaceName, const char* methodName, uint32_t* memberId)
{
    if (descriptorJson == nullptr || interfaceName == nullptr || methodName == nullptr || memberId == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    Json descriptor = Json::parse(descriptorJson, nullptr, false);
    if (descriptor.is_discarded()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto interfaceObj = FindInterface(descriptor, interfaceName);
    if (!interfaceObj.has_value()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto methodObj = FindMethod(*interfaceObj, methodName);
    if (!methodObj.has_value()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *memberId = GetDispId(*methodObj);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetMethodReturnTypeFromDescriptor(const char* descriptorJson,
    const char* interfaceName, const char* methodName, OH_AbilityRuntime_MoDispatcher_Vt* returnType)
{
    if (descriptorJson == nullptr || interfaceName == nullptr || methodName == nullptr || returnType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    Json descriptor = Json::parse(descriptorJson, nullptr, false);
    if (descriptor.is_discarded()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto interfaceObj = FindInterface(descriptor, interfaceName);
    if (!interfaceObj.has_value()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto methodObj = FindMethod(*interfaceObj, methodName);
    if (!methodObj.has_value() || !methodObj->contains("return_type") || !(*methodObj)["return_type"].is_object()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *returnType = ParseTypeInfo((*methodObj)["return_type"].value("type", ""));
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetMethodParamCountFromDescriptor(const char* descriptorJson,
    const char* interfaceName, const char* methodName, uint32_t* count)
{
    if (descriptorJson == nullptr || interfaceName == nullptr || methodName == nullptr || count == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    Json descriptor = Json::parse(descriptorJson, nullptr, false);
    if (descriptor.is_discarded()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto interfaceObj = FindInterface(descriptor, interfaceName);
    if (!interfaceObj.has_value()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto methodObj = FindMethod(*interfaceObj, methodName);
    if (!methodObj.has_value() || !methodObj->contains("parameters") || !(*methodObj)["parameters"].is_array()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *count = static_cast<uint32_t>((*methodObj)["parameters"].size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetMethodParamTypeFromDescriptor(const char* descriptorJson,
    const char* interfaceName, const char* methodName, uint32_t paramIndex,
    OH_AbilityRuntime_MoDispatcher_Vt* paramType)
{
    if (descriptorJson == nullptr || interfaceName == nullptr || methodName == nullptr || paramType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    Json descriptor = Json::parse(descriptorJson, nullptr, false);
    if (descriptor.is_discarded()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto interfaceObj = FindInterface(descriptor, interfaceName);
    auto methodObj = interfaceObj.has_value() ? FindMethod(*interfaceObj, methodName) : std::nullopt;
    if (!methodObj.has_value() || !methodObj->contains("parameters") || !(*methodObj)["parameters"].is_array()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    const auto& params = (*methodObj)["parameters"];
    if (paramIndex >= params.size() || !params[paramIndex].contains("type_info") || !params[paramIndex]["type_info"].is_object()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *paramType = ParseTypeInfo(params[paramIndex]["type_info"].value("type", ""));
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherMetadataManager::GetMethodParamNameFromDescriptor(const char* descriptorJson,
    const char* interfaceName, const char* methodName, uint32_t paramIndex, std::string* paramName)
{
    if (descriptorJson == nullptr || interfaceName == nullptr || methodName == nullptr || paramName == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    Json descriptor = Json::parse(descriptorJson, nullptr, false);
    if (descriptor.is_discarded()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto interfaceObj = FindInterface(descriptor, interfaceName);
    auto methodObj = interfaceObj.has_value() ? FindMethod(*interfaceObj, methodName) : std::nullopt;
    if (!methodObj.has_value() || !methodObj->contains("parameters") || !(*methodObj)["parameters"].is_array()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    const auto& params = (*methodObj)["parameters"];
    if (paramIndex >= params.size() || !params[paramIndex].contains("name") || !params[paramIndex]["name"].is_string()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *paramName = params[paramIndex]["name"].get<std::string>();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

} // namespace OHOS::AbilityRuntime
