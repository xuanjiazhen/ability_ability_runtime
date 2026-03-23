
/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with License.
 * You may obtain a copy of License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "ability_runtime/modular_object_dispatcher.h"

#include <algorithm>
#include <atomic>
#include <cctype>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <mutex>
#include <new>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "hilog_tag_wrapper.h"

#if __has_include(<nlohmann/json.hpp>)
#include <nlohmann/json.hpp>
#define MO_DISPATCHER_HAS_JSON 1
#else
#define MO_DISPATCHER_HAS_JSON 0
#endif

using namespace OHOS::AbilityRuntime;
using namespace OHOS::AAFwk;

namespace {
constexpr const char* MO_METADATA_JSON_ENV = "ABILITY_RUNTIME_MO_DISPATCHER_METADATA_JSON";
constexpr const char* MO_METADATA_FILE_ENV = "ABILITY_RUNTIME_MO_DISPATCHER_METADATA_FILE";

struct ParameterMetadata {
    std::string name;
    uint32_t memberId = 0;
    OH_AbilityRuntime_MoDispatcher_Vt type = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
};

struct MethodMetadata {
    std::string interfaceName;
    std::string name;
    uint32_t memberId = 0;
    uint32_t ipcCode = 0;
    OH_AbilityRuntime_MoDispatcher_Vt returnType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID;
    std::vector<ParameterMetadata> parameters;
};

struct InterfaceMetadata {
    std::string name;
    uint32_t memberId = 0;
    bool isCallback = false;
    std::string descriptorJson;
    std::vector<MethodMetadata> methods;
};

struct EnumValueMetadata {
    std::string name;
    uint32_t memberId = 0;
    int32_t value = 0;
};

struct EnumMetadata {
    std::string name;
    uint32_t memberId = 0;
    std::vector<EnumValueMetadata> values;
};

struct StructFieldMetadata {
    std::string name;
    uint32_t memberId = 0;
    OH_AbilityRuntime_MoDispatcher_Vt type = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
};

struct StructMetadata {
    std::string name;
    uint32_t memberId = 0;
    std::vector<StructFieldMetadata> fields;
};

struct MetadataData {
    std::string packageName;
    std::string version;
    std::string mainServiceInterfaceName;
    std::vector<InterfaceMetadata> interfaces;
    std::vector<EnumMetadata> enums;
    std::vector<StructMetadata> structs;

    std::unordered_map<std::string, uint32_t> nameToMemberId;
    std::unordered_map<uint32_t, std::string> memberIdToName;
    std::unordered_map<uint32_t, MethodMetadata> methodsByMemberId;
};

std::mutex gDescriptorCacheMutex;
std::unordered_map<std::string, std::weak_ptr<MetadataData>> gDescriptorCache;

OH_AbilityRuntime_MoDispatcher_Variant MakeDefaultVariant(OH_AbilityRuntime_MoDispatcher_Vt vt)
{
    OH_AbilityRuntime_MoDispatcher_Variant variant {};
    variant.vt = vt;
    variant.reserved1 = 0;
    variant.reserved2 = 0;
    variant.reserved3 = 0;
    return variant;
}

AbilityRuntime_ErrorCode CopyStringToBuffer(const std::string& src, char* dst, uint32_t maxLen)
{
    if (dst == nullptr || maxLen == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (src.size() + 1 > maxLen) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (!src.empty()) {
        (void)memcpy(dst, src.data(), src.size());
    }
    dst[src.size()] = '\0';
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

std::string ToUpper(std::string value)
{
    std::transform(value.begin(), value.end(), value.begin(),
        [](unsigned char c) { return static_cast<char>(std::toupper(c)); });
    return value;
}

std::optional<OH_AbilityRuntime_MoDispatcher_Vt> ParseVtFromString(const std::string& value)
{
    const std::string v = ToUpper(value);
    if (v == "VT_I4" || v == "I32" || v == "INT32") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32;
    if (v == "VT_I64" || v == "I64" || v == "INT64" || v == "LONG") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64;
    if (v == "VT_BOOL" || v == "BOOL") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL;
    if (v == "VT_R8" || v == "F64" || v == "DOUBLE") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64;
    if (v == "VT_R4" || v == "F32" || v == "FLOAT") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F32;
    if (v == "VT_STRING" || v == "VT_BSTR" || v == "STRING") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING;
    if (v == "VT_U32" || v == "U32") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U32;
    if (v == "VT_U64" || v == "U64") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U64;
    if (v == "VT_U16" || v == "U16") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U16;
    if (v == "VT_U8" || v == "U8") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U8;
    if (v == "VT_I16" || v == "I16") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I16;
    if (v == "VT_I8" || v == "I8" || v == "INT8") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I8;
    if (v == "VT_VOID" || v == "VOID") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID;
    if (v == "VT_EMPTY" || v == "EMPTY") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    if (v == "VT_ARRAY" || v == "ARRAY") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY;
    if (v == "VT_VECTOR" || v == "VECTOR") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR;
    if (v == "VT_SET" || v == "SET") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET;
    if (v == "VT_MAP" || v == "MAP") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP;
    if (v == "VT_STRUCT" || v == "STRUCT") return OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT;
    return std::nullopt;
}

class VariantHolder {
public:
    VariantHolder() : value_(MakeDefaultVariant(OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY)) {}
    explicit VariantHolder(const OH_AbilityRuntime_MoDispatcher_Variant& src)
    {
        Set(src);
    }

    void Set(const OH_AbilityRuntime_MoDispatcher_Variant& src)
    {
        value_ = src;
        if (src.vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING) {
            if (src.u.bstrVal == nullptr) {
                str_.reset();
            } else {
                str_ = std::string(src.u.bstrVal);
            }
            value_.u.bstrVal = str_.has_value() ? const_cast<char*>(str_->c_str()) : nullptr;
        } else {
            str_.reset();
        }
    }

    void SetDefault(OH_AbilityRuntime_MoDispatcher_Vt vt)
    {
        value_ = MakeDefaultVariant(vt);
        str_.reset();
    }

    void Export(OH_AbilityRuntime_MoDispatcher_Variant* out) const
    {
        if (out == nullptr) {
            return;
        }
        *out = value_;
        if (value_.vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING) {
            out->u.bstrVal = str_.has_value() ? const_cast<char*>(str_->c_str()) : nullptr;
        }
    }

    const OH_AbilityRuntime_MoDispatcher_Variant& Raw() const
    {
        return value_;
    }

private:
    OH_AbilityRuntime_MoDispatcher_Variant value_ {};
    std::optional<std::string> str_;
};

bool VariantEquals(const VariantHolder& lhs, const OH_AbilityRuntime_MoDispatcher_Variant& rhs)
{
    const auto& left = lhs.Raw();
    if (left.vt != rhs.vt) {
        return false;
    }
    switch (left.vt) {
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY:
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID:
            return true;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL:
            return left.u.boolVal == rhs.u.boolVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I8:
            return left.u.i8Val == rhs.u.i8Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I16:
            return left.u.i16Val == rhs.u.i16Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32:
            return left.u.i32Val == rhs.u.i32Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64:
            return left.u.i64Val == rhs.u.i64Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U8:
            return left.u.u8Val == rhs.u.u8Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U16:
            return left.u.u16Val == rhs.u.u16Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U32:
            return left.u.u32Val == rhs.u.u32Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U64:
            return left.u.u64Val == rhs.u.u64Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F32:
            return left.u.f32Val == rhs.u.f32Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64:
            return left.u.f64Val == rhs.u.f64Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING:
            if (left.u.bstrVal == nullptr || rhs.u.bstrVal == nullptr) {
                return left.u.bstrVal == rhs.u.bstrVal;
            }
            return std::string(left.u.bstrVal) == std::string(rhs.u.bstrVal);
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY:
            return left.u.parrayVal == rhs.u.parrayVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR:
            return left.u.pvectorVal == rhs.u.pvectorVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET:
            return left.u.psetVal == rhs.u.psetVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP:
            return left.u.pmapVal == rhs.u.pmapVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT:
            return left.u.pstructVal == rhs.u.pstructVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY:
            return left.u.premoteProxyVal == rhs.u.premoteProxyVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_STUB:
            return left.u.premoteStubVal == rhs.u.premoteStubVal;
        default:
            return false;
    }
}

#if MO_DISPATCHER_HAS_JSON
using Json = nlohmann::json;

uint32_t ReadUint(const Json& obj, std::initializer_list<const char*> keys, uint32_t defaultValue = 0)
{
    for (const char* key : keys) {
        auto it = obj.find(key);
        if (it == obj.end()) {
            continue;
        }
        if (it->is_number_unsigned()) {
            return it->get<uint32_t>();
        }
        if (it->is_number_integer()) {
            int64_t value = it->get<int64_t>();
            return value < 0 ? defaultValue : static_cast<uint32_t>(value);
        }
        if (it->is_string()) {
            try {
                return static_cast<uint32_t>(std::stoul(it->get<std::string>()));
            } catch (...) {
                return defaultValue;
            }
        }
    }
    return defaultValue;
}

std::string ReadString(const Json& obj, std::initializer_list<const char*> keys)
{
    for (const char* key : keys) {
        auto it = obj.find(key);
        if (it != obj.end() && it->is_string()) {
            return it->get<std::string>();
        }
    }
    return "";
}

void BuildLookupMaps(MetadataData& metadata)
{
    metadata.nameToMemberId.clear();
    metadata.memberIdToName.clear();
    metadata.methodsByMemberId.clear();

    for (const auto& interfaceInfo : metadata.interfaces) {
        if (!interfaceInfo.name.empty() && interfaceInfo.memberId != 0) {
            metadata.nameToMemberId.emplace(interfaceInfo.name, interfaceInfo.memberId);
            metadata.memberIdToName[interfaceInfo.memberId] = interfaceInfo.name;
        }
        for (const auto& method : interfaceInfo.methods) {
            std::string qualified = interfaceInfo.name.empty() ? method.name : interfaceInfo.name + "::" + method.name;
            if (!method.name.empty() && method.memberId != 0) {
                metadata.nameToMemberId.emplace(method.name, method.memberId);
                metadata.nameToMemberId.emplace(qualified, method.memberId);
                metadata.memberIdToName[method.memberId] = qualified;
                metadata.methodsByMemberId[method.memberId] = method;
            }
        }
    }

    for (const auto& enumInfo : metadata.enums) {
        if (!enumInfo.name.empty() && enumInfo.memberId != 0) {
            metadata.nameToMemberId.emplace(enumInfo.name, enumInfo.memberId);
            metadata.memberIdToName[enumInfo.memberId] = enumInfo.name;
        }
        for (const auto& value : enumInfo.values) {
            std::string qualified = enumInfo.name.empty() ? value.name : enumInfo.name + "::" + value.name;
            if (!value.name.empty() && value.memberId != 0) {
                metadata.nameToMemberId.emplace(value.name, value.memberId);
                metadata.nameToMemberId.emplace(qualified, value.memberId);
                metadata.memberIdToName[value.memberId] = qualified;
            }
        }
    }

    for (const auto& structInfo : metadata.structs) {
        if (!structInfo.name.empty() && structInfo.memberId != 0) {
            metadata.nameToMemberId.emplace(structInfo.name, structInfo.memberId);
            metadata.memberIdToName[structInfo.memberId] = structInfo.name;
        }
        for (const auto& field : structInfo.fields) {
            std::string qualified = structInfo.name.empty() ? field.name : structInfo.name + "::" + field.name;
            if (!field.name.empty() && field.memberId != 0) {
                metadata.nameToMemberId.emplace(field.name, field.memberId);
                metadata.nameToMemberId.emplace(qualified, field.memberId);
                metadata.memberIdToName[field.memberId] = qualified;
            }
        }
    }

    if (metadata.mainServiceInterfaceName.empty() && !metadata.interfaces.empty()) {
        metadata.mainServiceInterfaceName = metadata.interfaces.front().name;
    }
}

AbilityRuntime_ErrorCode ParseMetadataJson(const std::string& jsonText, std::shared_ptr<MetadataData>& outMetadata)
{
    Json root = Json::parse(jsonText, nullptr, false);
    if (root.is_discarded() || !root.is_object()) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }

    auto metadata = std::make_shared<MetadataData>();
    metadata->packageName = ReadString(root, {"package", "packageName"});
    metadata->version = ReadString(root, {"version"});
    metadata->mainServiceInterfaceName = ReadString(root, {"mainServiceInterface", "mainServiceInterfaceName"});

    if (root.contains("interfaces") && root.at("interfaces").is_array()) {
        for (const auto& iface : root.at("interfaces")) {
            if (!iface.is_object()) {
                continue;
            }
            InterfaceMetadata interfaceInfo;
            interfaceInfo.name = ReadString(iface, {"interfaceName", "name"});
            interfaceInfo.memberId = ReadUint(iface, {"memberId", "id"});
            interfaceInfo.isCallback = iface.contains("isCallback") && iface.at("isCallback").is_boolean() &&
                iface.at("isCallback").get<bool>();
            interfaceInfo.descriptorJson = iface.dump();

            if (iface.contains("methods") && iface.at("methods").is_array()) {
                for (const auto& method : iface.at("methods")) {
                    if (!method.is_object()) {
                        continue;
                    }
                    MethodMetadata methodInfo;
                    methodInfo.interfaceName = interfaceInfo.name;
                    methodInfo.name = ReadString(method, {"name", "methodName"});
                    methodInfo.memberId = ReadUint(method, {"memberId", "id"});
                    methodInfo.ipcCode = ReadUint(method, {"ipcCode", "code"}, methodInfo.memberId);
                    if (method.contains("returnType")) {
                        std::optional<OH_AbilityRuntime_MoDispatcher_Vt> parsed = std::nullopt;
                        if (method.at("returnType").is_string()) {
                            parsed = ParseVtFromString(method.at("returnType").get<std::string>());
                        } else if (method.at("returnType").is_object() &&
                            method.at("returnType").contains("vt") && method.at("returnType").at("vt").is_string()) {
                            parsed = ParseVtFromString(method.at("returnType").at("vt").get<std::string>());
                        }
                        if (parsed.has_value()) {
                            methodInfo.returnType = *parsed;
                        }
                    }
                    if (method.contains("parameters") && method.at("parameters").is_array()) {
                        for (const auto& parameter : method.at("parameters")) {
                            if (!parameter.is_object()) {
                                continue;
                            }
                            ParameterMetadata p;
                            p.name = ReadString(parameter, {"name", "paramName"});
                            p.memberId = ReadUint(parameter, {"memberId", "id"});
                            if (parameter.contains("vt") && parameter.at("vt").is_string()) {
                                auto parsed = ParseVtFromString(parameter.at("vt").get<std::string>());
                                if (parsed.has_value()) {
                                    p.type = *parsed;
                                }
                            } else if (parameter.contains("type") && parameter.at("type").is_string()) {
                                auto parsed = ParseVtFromString(parameter.at("type").get<std::string>());
                                if (parsed.has_value()) {
                                    p.type = *parsed;
                                }
                            }
                            methodInfo.parameters.emplace_back(std::move(p));
                        }
                    }
                    interfaceInfo.methods.emplace_back(std::move(methodInfo));
                }
            }
            metadata->interfaces.emplace_back(std::move(interfaceInfo));
        }
    }

    BuildLookupMaps(*metadata);
    outMetadata = std::move(metadata);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}
#else
AbilityRuntime_ErrorCode ParseMetadataJson(const std::string& jsonText, std::shared_ptr<MetadataData>& outMetadata)
{
    (void)jsonText;
    (void)outMetadata;
    return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}
#endif

AbilityRuntime_ErrorCode ReadFileToString(const std::string& path, std::string& content)
{
    std::ifstream in(path, std::ios::in | std::ios::binary);
    if (!in.is_open()) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    content.assign((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode LoadMetadataFromEnvironment(std::shared_ptr<MetadataData>& metadata)
{
    const char* jsonEnv = std::getenv(MO_METADATA_JSON_ENV);
    if (jsonEnv != nullptr && jsonEnv[0] != '\0') {
        auto ret = ParseMetadataJson(std::string(jsonEnv), metadata);
        if (ret == ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
            return ret;
        }
    }

    const char* fileEnv = std::getenv(MO_METADATA_FILE_ENV);
    if (fileEnv != nullptr && fileEnv[0] != '\0') {
        std::string content;
        auto readRet = ReadFileToString(fileEnv, content);
        if (readRet == ABILITY_RUNTIME_ERROR_CODE_NO_ERROR && !content.empty()) {
            auto parseRet = ParseMetadataJson(content, metadata);
            if (parseRet == ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
                return parseRet;
            }
        }
    }

    metadata = std::make_shared<MetadataData>();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

const InterfaceMetadata* FindInterfaceByName(const MetadataData& metadata, const std::string& name)
{
    auto it = std::find_if(metadata.interfaces.begin(), metadata.interfaces.end(),
        [&name](const InterfaceMetadata& interfaceInfo) { return interfaceInfo.name == name; });
    return it == metadata.interfaces.end() ? nullptr : &(*it);
}

const MethodMetadata* FindMethodByName(const InterfaceMetadata& interfaceInfo, const std::string& methodName)
{
    auto it = std::find_if(interfaceInfo.methods.begin(), interfaceInfo.methods.end(),
        [&methodName](const MethodMetadata& methodInfo) { return methodInfo.name == methodName; });
    return it == interfaceInfo.methods.end() ? nullptr : &(*it);
}

} // namespace

struct OH_AbilityRuntime_MoDispatcher_TypeDescriptor {
    std::shared_ptr<MetadataData> metadata;
    std::atomic<uint32_t> refCount {1};
};

struct OH_AbilityRuntime_MoDispatcher {
    OHIPCRemoteProxy* remoteProxy = nullptr;
    std::mutex mutex;
    std::unordered_map<std::string, uint32_t> memberIdCache;
    std::shared_ptr<MetadataData> metadata;
    OH_AbilityRuntime_MoDispatcher_TypeDescriptor* cachedTypeDescriptor = nullptr;
};

struct OH_AbilityRuntime_MoDispatcher_Array {
    OH_AbilityRuntime_MoDispatcher_Vt elementType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    std::vector<VariantHolder> elements;
    std::mutex mutex;
};

struct OH_AbilityRuntime_MoDispatcher_Vector {
    OH_AbilityRuntime_MoDispatcher_Vt elementType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    std::vector<VariantHolder> elements;
    std::mutex mutex;
};

struct OH_AbilityRuntime_MoDispatcher_Set {
    OH_AbilityRuntime_MoDispatcher_Vt elementType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    std::vector<VariantHolder> elements;
    std::mutex mutex;
};

struct OH_AbilityRuntime_MoDispatcher_Map {
    OH_AbilityRuntime_MoDispatcher_Vt keyType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    OH_AbilityRuntime_MoDispatcher_Vt valueType = OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY;
    std::vector<std::pair<VariantHolder, VariantHolder>> entries;
    std::mutex mutex;
};

struct OH_AbilityRuntime_MoDispatcher_Struct {
    std::string structName;
    std::unordered_map<std::string, VariantHolder> fields;
    std::mutex mutex;
};

static void AddRefTypeDescriptor(OH_AbilityRuntime_MoDispatcher_TypeDescriptor* typeDescriptor)
{
    if (typeDescriptor != nullptr) {
        (void)typeDescriptor->refCount.fetch_add(1, std::memory_order_relaxed);
    }
}

static void ReleaseTypeDescriptor(OH_AbilityRuntime_MoDispatcher_TypeDescriptor* typeDescriptor)
{
    if (typeDescriptor != nullptr && typeDescriptor->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
        delete typeDescriptor;
    }
}

static AbilityRuntime_ErrorCode EnsureDispatcherMetadata(OH_AbilityRuntime_MoDispatcher* dispatcher)
{
    if (dispatcher->metadata != nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    return LoadMetadataFromEnvironment(dispatcher->metadata);
}

static const EnumMetadata* FindEnumByName(const MetadataData& metadata, const std::string& enumName)
{
    auto it = std::find_if(metadata.enums.begin(), metadata.enums.end(),
        [&enumName](const EnumMetadata& enumInfo) { return enumInfo.name == enumName; });
    return it == metadata.enums.end() ? nullptr : &(*it);
}

static const StructMetadata* FindStructByName(const MetadataData& metadata, const std::string& structName)
{
    auto it = std::find_if(metadata.structs.begin(), metadata.structs.end(),
        [&structName](const StructMetadata& structInfo) { return structInfo.name == structName; });
    return it == metadata.structs.end() ? nullptr : &(*it);
}

#ifdef __cplusplus
extern "C" {
#endif

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_CreateInstance(
    OHIPCRemoteProxy* remoteProxy, OH_AbilityRuntime_MoDispatcherHandle* ppMoDispatcher)
{
    if (remoteProxy == nullptr || ppMoDispatcher == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* dispatcher = new (std::nothrow) OH_AbilityRuntime_MoDispatcher();
    if (dispatcher == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    dispatcher->remoteProxy = remoteProxy;
    *ppMoDispatcher = dispatcher;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void OH_AbilityRuntime_MoDispatcher_Release(OH_AbilityRuntime_MoDispatcherHandle* ppMoDispatcher)
{
    if (ppMoDispatcher == nullptr || *ppMoDispatcher == nullptr) {
        return;
    }
    auto* dispatcher = *ppMoDispatcher;
    if (dispatcher->cachedTypeDescriptor != nullptr) {
        ReleaseTypeDescriptor(dispatcher->cachedTypeDescriptor);
    }
    delete dispatcher;
    *ppMoDispatcher = nullptr;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_HasTypeDescriptor(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher, uint32_t* pctinfo)
{
    if (pMoDispatcher == nullptr || pctinfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pctinfo = pMoDispatcher->remoteProxy != nullptr ? 1U : 0U;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_GetTypeDescriptor(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher, OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle* ppTypeDescriptor)
{
    if (pMoDispatcher == nullptr || ppTypeDescriptor == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    std::lock_guard<std::mutex> lock(pMoDispatcher->mutex);
    auto ret = EnsureDispatcherMetadata(pMoDispatcher);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }

    if (pMoDispatcher->cachedTypeDescriptor == nullptr) {
        auto* typeDescriptor = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_TypeDescriptor();
        if (typeDescriptor == nullptr) {
            return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        }
        typeDescriptor->metadata = pMoDispatcher->metadata;
        pMoDispatcher->cachedTypeDescriptor = typeDescriptor;
    }

    AddRefTypeDescriptor(pMoDispatcher->cachedTypeDescriptor);
    *ppTypeDescriptor = pMoDispatcher->cachedTypeDescriptor;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_QueryMemIDsOfNames(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher, const char** rgszNames, uint32_t cNames,
    uint32_t* pMemID)
{
    if (pMoDispatcher == nullptr || rgszNames == nullptr || pMemID == nullptr || cNames == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    std::lock_guard<std::mutex> lock(pMoDispatcher->mutex);
    auto ret = EnsureDispatcherMetadata(pMoDispatcher);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }

    for (uint32_t index = 0; index < cNames; ++index) {
        if (rgszNames[index] == nullptr) {
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        std::string name(rgszNames[index]);
        auto it = pMoDispatcher->memberIdCache.find(name);
        if (it != pMoDispatcher->memberIdCache.end()) {
            pMemID[index] = it->second;
            continue;
        }
        auto m = pMoDispatcher->metadata->nameToMemberId.find(name);
        if (m == pMoDispatcher->metadata->nameToMemberId.end()) {
            return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
        }
        pMoDispatcher->memberIdCache[name] = m->second;
        pMemID[index] = m->second;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_CallMethod(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher, uint32_t memID,
    OH_AbilityRuntime_MoDispatcher_Inputparams* pInputParams,
    OH_AbilityRuntime_MoDispatcher_Variant* pResult)
{
    if (pMoDispatcher == nullptr || pResult == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    std::lock_guard<std::mutex> lock(pMoDispatcher->mutex);
    auto ret = EnsureDispatcherMetadata(pMoDispatcher);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }

    auto methodIt = pMoDispatcher->metadata->methodsByMemberId.find(memID);
    if (methodIt == pMoDispatcher->metadata->methodsByMemberId.end()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }

    const auto& method = methodIt->second;
    uint32_t count = static_cast<uint32_t>(method.parameters.size());
    if ((count > 0 && pInputParams == nullptr) || (pInputParams != nullptr && pInputParams->cArgs != count)) {
        return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
    }
    if (pInputParams != nullptr && pInputParams->rgvarg == nullptr && count > 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (pInputParams != nullptr) {
        for (uint32_t index = 0; index < count; ++index) {
            if (pInputParams->rgvarg[index].vt != method.parameters[index].type) {
                return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
            }
        }
    }

    *pResult = MakeDefaultVariant(method.returnType);
    return ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED;
}

void OH_AbilityRuntime_TypeDescriptor_Release(OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle *pTypeDescriptor)
{
    if (pTypeDescriptor == nullptr || *pTypeDescriptor == nullptr) {
        return;
    }
    ReleaseTypeDescriptor(*pTypeDescriptor);
    *pTypeDescriptor = nullptr;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetPackage(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, char* pbstrPackage, uint32_t cMaxPackage)
{
    if (pTypeDescriptor == nullptr || pbstrPackage == nullptr || cMaxPackage == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return CopyStringToBuffer(pTypeDescriptor->metadata->packageName, pbstrPackage, cMaxPackage);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetVersion(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, char* pbstrVersion, uint32_t cMaxVersion)
{
    if (pTypeDescriptor == nullptr || pbstrVersion == nullptr || cMaxVersion == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return CopyStringToBuffer(pTypeDescriptor->metadata->version, pbstrVersion, cMaxVersion);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMemIDsOfNames(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* szName, uint32_t* pMemID)
{
    if (pTypeDescriptor == nullptr || szName == nullptr || pMemID == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto it = pTypeDescriptor->metadata->nameToMemberId.find(szName);
    if (it == pTypeDescriptor->metadata->nameToMemberId.end()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *pMemID = it->second;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMemberName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t memID, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto it = pTypeDescriptor->metadata->memberIdToName.find(memID);
    if (it == pTypeDescriptor->metadata->memberIdToName.end()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    return CopyStringToBuffer(it->second, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcInterfaces)
{
    if (pTypeDescriptor == nullptr || pcInterfaces == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pcInterfaces = static_cast<uint32_t>(pTypeDescriptor->metadata->interfaces.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0 ||
        index >= pTypeDescriptor->metadata->interfaces.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return CopyStringToBuffer(pTypeDescriptor->metadata->interfaces[index].name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceMemberId(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrName, uint32_t* pMemID)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || pMemID == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto* interfaceInfo = FindInterfaceByName(*pTypeDescriptor->metadata, pbstrName);
    if (interfaceInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *pMemID = interfaceInfo->memberId;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceDescriptor(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrName,
    char* pbstrDescriptor, uint32_t cMaxDescriptor)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || pbstrDescriptor == nullptr || cMaxDescriptor == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto* interfaceInfo = FindInterfaceByName(*pTypeDescriptor->metadata, pbstrName);
    if (interfaceInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    return CopyStringToBuffer(interfaceInfo->descriptorJson, pbstrDescriptor, cMaxDescriptor);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceIsCallback(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrName, bool* pIsCallback)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || pIsCallback == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto* interfaceInfo = FindInterfaceByName(*pTypeDescriptor->metadata, pbstrName);
    if (interfaceInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *pIsCallback = interfaceInfo->isCallback;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMainServiceInterfaceName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (pTypeDescriptor->metadata->mainServiceInterfaceName.empty()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    return CopyStringToBuffer(pTypeDescriptor->metadata->mainServiceInterfaceName, pbstrName, cMaxName);
}

static AbilityRuntime_ErrorCode GetMetadataByDescriptor(const char* descriptor, std::shared_ptr<MetadataData>& metadata)
{
    if (descriptor == nullptr || descriptor[0] == '\0') {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }

    std::string key(descriptor);
    {
        std::lock_guard<std::mutex> lock(gDescriptorCacheMutex);
        auto it = gDescriptorCache.find(key);
        if (it != gDescriptorCache.end()) {
            auto cached = it->second.lock();
            if (cached != nullptr) {
                metadata = std::move(cached);
                return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
            }
        }
    }

    std::shared_ptr<MetadataData> parsed;
    auto ret = ParseMetadataJson(key, parsed);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }

    {
        std::lock_guard<std::mutex> lock(gDescriptorCacheMutex);
        gDescriptorCache[key] = parsed;
    }
    metadata = std::move(parsed);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodCount(
    const char* pbstrDescriptor, const char* pbstrInterfaceName, uint32_t* pcMethods)
{
    if (pbstrDescriptor == nullptr || pbstrInterfaceName == nullptr || pcMethods == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::shared_ptr<MetadataData> metadata;
    auto ret = GetMetadataByDescriptor(pbstrDescriptor, metadata);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto* interfaceInfo = FindInterfaceByName(*metadata, pbstrInterfaceName);
    if (interfaceInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *pcMethods = static_cast<uint32_t>(interfaceInfo->methods.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodName(
    const char* pbstrDescriptor, const char* pbstrInterfaceName, uint32_t index,
    char* pbstrName, uint32_t cMaxName)
{
    if (pbstrDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::shared_ptr<MetadataData> metadata;
    auto ret = GetMetadataByDescriptor(pbstrDescriptor, metadata);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto* interfaceInfo = FindInterfaceByName(*metadata, pbstrInterfaceName);
    if (interfaceInfo == nullptr || index >= interfaceInfo->methods.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return CopyStringToBuffer(interfaceInfo->methods[index].name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId(
    const char* pbstrDescriptor, const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t* pMemID)
{
    if (pbstrDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr || pMemID == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::shared_ptr<MetadataData> metadata;
    auto ret = GetMetadataByDescriptor(pbstrDescriptor, metadata);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto* interfaceInfo = FindInterfaceByName(*metadata, pbstrInterfaceName);
    if (interfaceInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    const auto* methodInfo = FindMethodByName(*interfaceInfo, pbstrMethodName);
    if (methodInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *pMemID = methodInfo->memberId;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType(
    const char* pbstrDescriptor, const char* pbstrInterfaceName, const char* pbstrMethodName,
    OH_AbilityRuntime_MoDispatcher_Vt* pReturnType)
{
    if (pbstrDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr || pReturnType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::shared_ptr<MetadataData> metadata;
    auto ret = GetMetadataByDescriptor(pbstrDescriptor, metadata);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto* interfaceInfo = FindInterfaceByName(*metadata, pbstrInterfaceName);
    if (interfaceInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    const auto* methodInfo = FindMethodByName(*interfaceInfo, pbstrMethodName);
    if (methodInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *pReturnType = methodInfo->returnType;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount(
    const char* pbstrDescriptor, const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t* pcParams)
{
    if (pbstrDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr || pcParams == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::shared_ptr<MetadataData> metadata;
    auto ret = GetMetadataByDescriptor(pbstrDescriptor, metadata);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto* interfaceInfo = FindInterfaceByName(*metadata, pbstrInterfaceName);
    if (interfaceInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    const auto* methodInfo = FindMethodByName(*interfaceInfo, pbstrMethodName);
    if (methodInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *pcParams = static_cast<uint32_t>(methodInfo->parameters.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamType(
    const char* pbstrDescriptor, const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t iParamIndex, OH_AbilityRuntime_MoDispatcher_Vt* pParamType)
{
    if (pbstrDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr || pParamType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::shared_ptr<MetadataData> metadata;
    auto ret = GetMetadataByDescriptor(pbstrDescriptor, metadata);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto* interfaceInfo = FindInterfaceByName(*metadata, pbstrInterfaceName);
    if (interfaceInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    const auto* methodInfo = FindMethodByName(*interfaceInfo, pbstrMethodName);
    if (methodInfo == nullptr || iParamIndex >= methodInfo->parameters.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pParamType = methodInfo->parameters[iParamIndex].type;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamName(
    const char* pbstrDescriptor, const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t iParamIndex, char* pbstrName, uint32_t cMaxName)
{
    if (pbstrDescriptor == nullptr || pbstrInterfaceName == nullptr || pbstrMethodName == nullptr ||
        pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::shared_ptr<MetadataData> metadata;
    auto ret = GetMetadataByDescriptor(pbstrDescriptor, metadata);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    const auto* interfaceInfo = FindInterfaceByName(*metadata, pbstrInterfaceName);
    if (interfaceInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    const auto* methodInfo = FindMethodByName(*interfaceInfo, pbstrMethodName);
    if (methodInfo == nullptr || iParamIndex >= methodInfo->parameters.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return CopyStringToBuffer(methodInfo->parameters[iParamIndex].name, pbstrName, cMaxName);
}
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcEnums)
{
    if (pTypeDescriptor == nullptr || pcEnums == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pcEnums = static_cast<uint32_t>(pTypeDescriptor->metadata->enums.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0 ||
        index >= pTypeDescriptor->metadata->enums.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return CopyStringToBuffer(pTypeDescriptor->metadata->enums[index].name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumMemberId(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName, uint32_t* pMemID)
{
    if (pTypeDescriptor == nullptr || pbstrEnumName == nullptr || pMemID == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto* enumInfo = FindEnumByName(*pTypeDescriptor->metadata, pbstrEnumName);
    if (enumInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *pMemID = enumInfo->memberId;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumEnumValueCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName, uint32_t* pcValues)
{
    if (pTypeDescriptor == nullptr || pbstrEnumName == nullptr || pcValues == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto* enumInfo = FindEnumByName(*pTypeDescriptor->metadata, pbstrEnumName);
    if (enumInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *pcValues = static_cast<uint32_t>(enumInfo->values.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValueName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName, uint32_t iValueIndex,
    char* pbstrValueName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrEnumName == nullptr || pbstrValueName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto* enumInfo = FindEnumByName(*pTypeDescriptor->metadata, pbstrEnumName);
    if (enumInfo == nullptr || iValueIndex >= enumInfo->values.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return CopyStringToBuffer(enumInfo->values[iValueIndex].name, pbstrValueName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValue(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName, const char* pbstrValueName,
    int32_t* pValue)
{
    if (pTypeDescriptor == nullptr || pbstrEnumName == nullptr || pbstrValueName == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto* enumInfo = FindEnumByName(*pTypeDescriptor->metadata, pbstrEnumName);
    if (enumInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    auto it = std::find_if(enumInfo->values.begin(), enumInfo->values.end(),
        [pbstrValueName](const EnumValueMetadata& value) { return value.name == pbstrValueName; });
    if (it == enumInfo->values.end()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *pValue = it->value;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcStructs)
{
    if (pTypeDescriptor == nullptr || pcStructs == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pcStructs = static_cast<uint32_t>(pTypeDescriptor->metadata->structs.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0 ||
        index >= pTypeDescriptor->metadata->structs.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return CopyStringToBuffer(pTypeDescriptor->metadata->structs[index].name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructMemberId(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName, uint32_t* pMemID)
{
    if (pTypeDescriptor == nullptr || pbstrStructName == nullptr || pMemID == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto* structInfo = FindStructByName(*pTypeDescriptor->metadata, pbstrStructName);
    if (structInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *pMemID = structInfo->memberId;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName, uint32_t* pcFields)
{
    if (pTypeDescriptor == nullptr || pbstrStructName == nullptr || pcFields == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto* structInfo = FindStructByName(*pTypeDescriptor->metadata, pbstrStructName);
    if (structInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *pcFields = static_cast<uint32_t>(structInfo->fields.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName, uint32_t iFieldIndex,
    char* pbstrFieldName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrStructName == nullptr || pbstrFieldName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto* structInfo = FindStructByName(*pTypeDescriptor->metadata, pbstrStructName);
    if (structInfo == nullptr || iFieldIndex >= structInfo->fields.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return CopyStringToBuffer(structInfo->fields[iFieldIndex].name, pbstrFieldName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldType(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName, const char* pbstrFieldName,
    OH_AbilityRuntime_MoDispatcher_Vt* pFieldType)
{
    if (pTypeDescriptor == nullptr || pbstrStructName == nullptr || pbstrFieldName == nullptr || pFieldType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto* structInfo = FindStructByName(*pTypeDescriptor->metadata, pbstrStructName);
    if (structInfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    auto it = std::find_if(structInfo->fields.begin(), structInfo->fields.end(),
        [pbstrFieldName](const StructFieldMetadata& field) { return field.name == pbstrFieldName; });
    if (it == structInfo->fields.end()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    *pFieldType = it->type;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_Create(
    OH_AbilityRuntime_MoDispatcher_Vt elementType, uint32_t size,
    OH_AbilityRuntime_MoDispatcher_ArrayHandle* ppArray)
{
    if (ppArray == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* array = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Array();
    if (array == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    array->elementType = elementType;
    array->elements.resize(size);
    for (auto& element : array->elements) {
        element.SetDefault(elementType);
    }
    *ppArray = array;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_GetElementType(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, OH_AbilityRuntime_MoDispatcher_Vt* pElementType)
{
    if (pArray == nullptr || pElementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pElementType = pArray->elementType;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_Set(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t index,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pArray == nullptr || pValue == nullptr || index >= pArray->elements.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (!IsTypeMatch(pArray->elementType, pValue->vt)) {
        return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
    }
    pArray->elements[index].Set(*pValue);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_Get(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t index,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pArray == nullptr || pValue == nullptr || index >= pArray->elements.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pArray->elements[index].Export(pValue);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_GetSize(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t* pSize)
{
    if (pArray == nullptr || pSize == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pArray->elements.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_Resize(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t newSize)
{
    if (pArray == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    size_t oldSize = pArray->elements.size();
    pArray->elements.resize(newSize);
    for (size_t i = oldSize; i < pArray->elements.size(); i++) {
        pArray->elements[i].SetDefault(pArray->elementType);
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void OH_AbilityRuntime_MoDispatcher_Array_Release(OH_AbilityRuntime_MoDispatcher_ArrayHandle* ppArray)
{
    if (ppArray == nullptr || *ppArray == nullptr) {
        return;
    }
    delete *ppArray;
    *ppArray = nullptr;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Create(
    OH_AbilityRuntime_MoDispatcher_Vt elementType, OH_AbilityRuntime_MoDispatcher_VectorHandle* ppVector)
{
    if (ppVector == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* vector = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Vector();
    if (vector == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    vector->elementType = elementType;
    *ppVector = vector;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_GetElementType(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, OH_AbilityRuntime_MoDispatcher_Vt* pElementType)
{
    if (pVector == nullptr || pElementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pElementType = pVector->elementType;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Add(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pVector == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (!IsTypeMatch(pVector->elementType, pValue->vt)) {
        return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
    }
    pVector->elements.emplace_back(*pValue);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Get(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, uint32_t index,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pVector == nullptr || pValue == nullptr || index >= pVector->elements.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pVector->elements[index].Export(pValue);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_GetSize(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, uint32_t* pSize)
{
    if (pVector == nullptr || pSize == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pVector->elements.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Clear(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector)
{
    if (pVector == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pVector->elements.clear();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void OH_AbilityRuntime_MoDispatcher_Vector_Release(OH_AbilityRuntime_MoDispatcher_VectorHandle* ppVector)
{
    if (ppVector == nullptr || *ppVector == nullptr) {
        return;
    }
    delete *ppVector;
    *ppVector = nullptr;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Create(
    OH_AbilityRuntime_MoDispatcher_Vt elementType, OH_AbilityRuntime_MoDispatcher_SetHandle* ppSet)
{
    if (ppSet == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* set = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Set();
    if (set == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    set->elementType = elementType;
    *ppSet = set;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_GetElementType(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, OH_AbilityRuntime_MoDispatcher_Vt* pElementType)
{
    if (pSet == nullptr || pElementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pElementType = pSet->elementType;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Add(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pSet == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (!IsTypeMatch(pSet->elementType, pValue->vt)) {
        return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
    }
    for (const auto& item : pSet->elements) {
        if (VariantEquals(item, *pValue)) {
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    pSet->elements.emplace_back(*pValue);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Remove(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pSet == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    for (auto it = pSet->elements.begin(); it != pSet->elements.end(); ++it) {
        if (VariantEquals(*it, *pValue)) {
            pSet->elements.erase(it);
            break;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Contains(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, const OH_AbilityRuntime_MoDispatcher_Variant* pValue, bool* pExists)
{
    if (pSet == nullptr || pValue == nullptr || pExists == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pExists = false;
    for (const auto& item : pSet->elements) {
        if (VariantEquals(item, *pValue)) {
            *pExists = true;
            break;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_GetSize(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, uint32_t* pSize)
{
    if (pSet == nullptr || pSize == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pSet->elements.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Clear(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet)
{
    if (pSet == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pSet->elements.clear();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void OH_AbilityRuntime_MoDispatcher_Set_Release(OH_AbilityRuntime_MoDispatcher_SetHandle* ppSet)
{
    if (ppSet == nullptr || *ppSet == nullptr) {
        return;
    }
    delete *ppSet;
    *ppSet = nullptr;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Create(
    OH_AbilityRuntime_MoDispatcher_Vt keyType, OH_AbilityRuntime_MoDispatcher_Vt valueType,
    OH_AbilityRuntime_MoDispatcher_MapHandle* ppMap)
{
    if (ppMap == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* map = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Map();
    if (map == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    map->keyType = keyType;
    map->valueType = valueType;
    *ppMap = map;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetKeyType(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, OH_AbilityRuntime_MoDispatcher_Vt* pKeyType)
{
    if (pMap == nullptr || pKeyType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pKeyType = pMap->keyType;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetValueType(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, OH_AbilityRuntime_MoDispatcher_Vt* pValueType)
{
    if (pMap == nullptr || pValueType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pValueType = pMap->valueType;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Put(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pMap == nullptr || pKey == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (!IsTypeMatch(pMap->keyType, pKey->vt) || !IsTypeMatch(pMap->valueType, pValue->vt)) {
        return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
    }
    for (auto& entry : pMap->entries) {
        if (VariantEquals(entry.first, *pKey)) {
            entry.second.Set(*pValue);
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    pMap->entries.emplace_back(VariantHolder(*pKey), VariantHolder(*pValue));
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Get(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pMap == nullptr || pKey == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    for (const auto& entry : pMap->entries) {
        if (VariantEquals(entry.first, *pKey)) {
            entry.second.Export(pValue);
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Remove(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey)
{
    if (pMap == nullptr || pKey == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    for (auto it = pMap->entries.begin(); it != pMap->entries.end(); ++it) {
        if (VariantEquals(it->first, *pKey)) {
            pMap->entries.erase(it);
            break;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_ContainsKey(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey, bool* pExists)
{
    if (pMap == nullptr || pKey == nullptr || pExists == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pExists = false;
    for (const auto& entry : pMap->entries) {
        if (VariantEquals(entry.first, *pKey)) {
            *pExists = true;
            break;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetSize(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, uint32_t* pSize)
{
    if (pMap == nullptr || pSize == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pMap->entries.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Clear(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap)
{
    if (pMap == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pMap->entries.clear();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void OH_AbilityRuntime_MoDispatcher_Map_Release(OH_AbilityRuntime_MoDispatcher_MapHandle* ppMap)
{
    if (ppMap == nullptr || *ppMap == nullptr) {
        return;
    }
    delete *ppMap;
    *ppMap = nullptr;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_Create(
    const char* structName, OH_AbilityRuntime_MoDispatcher_StructHandle* ppStruct)
{
    if (structName == nullptr || ppStruct == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* obj = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Struct();
    if (obj == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    obj->structName = structName;
    *ppStruct = obj;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_GetName(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, char* pbstrName, uint32_t cMaxName)
{
    if (pStruct == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return CopyStringToBuffer(pStruct->structName, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_SetField(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, const char* szName,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pStruct == nullptr || szName == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pStruct->fields[szName] = VariantHolder(*pValue);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_GetField(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, const char* szName,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pStruct == nullptr || szName == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto it = pStruct->fields.find(szName);
    if (it == pStruct->fields.end()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    it->second.Export(pValue);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void OH_AbilityRuntime_MoDispatcher_Struct_Release(OH_AbilityRuntime_MoDispatcher_StructHandle* ppStruct)
{
    if (ppStruct == nullptr || *ppStruct == nullptr) {
        return;
    }
    delete *ppStruct;
    *ppStruct = nullptr;
}

#ifdef __cplusplus
}
#endif
