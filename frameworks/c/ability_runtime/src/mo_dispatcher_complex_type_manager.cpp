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

#include "mo_dispatcher_complex_type_manager.h"

#include <algorithm>
#include <cstring>
#include <mutex>
#include <unordered_map>

#include "securec.h"

namespace OHOS::AbilityRuntime {
namespace {
std::mutex g_structMetaMutex;
std::unordered_map<std::string, std::unordered_map<std::string, OH_AbilityRuntime_MoDispatcher_Vt>> g_structFieldTypes;
std::unordered_map<std::string, std::vector<std::string>> g_structFieldOrder;

AbilityRuntime_ErrorCode CopyStringToBuffer(const std::string& src, char* dst, uint32_t maxLen)
{
    if (dst == nullptr || maxLen == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (src.size() + 1 > maxLen) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (strcpy_s(dst, maxLen, src.c_str()) != EOK) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

MoVariantStorage CreateDefaultVariantStorage(OH_AbilityRuntime_MoDispatcher_Vt type)
{
    MoVariantStorage storage;
    storage.value.vt = type;
    if (type == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING) {
        storage.stringStorage = "";
        storage.value.u.bstrVal = const_cast<char*>(storage.stringStorage.c_str());
    }
    return storage;
}
} // namespace

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::ArrayCreate(OH_AbilityRuntime_MoDispatcher_Vt elementType,
    uint32_t size, OH_AbilityRuntime_MoDispatcher_ArrayHandle* ppArray)
{
    if (ppArray == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* array = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Array();
    if (array == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    array->elementType = elementType;
    array->elements.reserve(size);
    for (uint32_t i = 0; i < size; i++) {
        array->elements.emplace_back(CreateDefaultVariantStorage(elementType));
    }
    *ppArray = array;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::ArrayGetElementType(OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray,
    OH_AbilityRuntime_MoDispatcher_Vt* pElementType)
{
    if (pArray == nullptr || pElementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pElementType = pArray->elementType;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::ArraySet(OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray,
    uint32_t index, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pArray == nullptr || pValue == nullptr || index >= pArray->elements.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantType(pValue, pArray->elementType);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return StoreVariant(pValue, &pArray->elements[index]);
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::ArrayGet(OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray,
    uint32_t index, OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pArray == nullptr || pValue == nullptr || index >= pArray->elements.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return LoadVariant(pArray->elements[index], pValue);
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::ArrayGetSize(OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray,
    uint32_t* pSize)
{
    if (pArray == nullptr || pSize == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pArray->elements.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::ArrayResize(OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray,
    uint32_t newSize)
{
    if (pArray == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    const auto oldSize = static_cast<uint32_t>(pArray->elements.size());
    pArray->elements.resize(newSize);
    for (uint32_t i = oldSize; i < newSize; i++) {
        pArray->elements[i] = CreateDefaultVariantStorage(pArray->elementType);
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void MoDispatcherComplexTypeManager::ArrayRelease(OH_AbilityRuntime_MoDispatcher_ArrayHandle* ppArray)
{
    if (ppArray == nullptr || *ppArray == nullptr) {
        return;
    }
    delete *ppArray;
    *ppArray = nullptr;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::VectorCreate(OH_AbilityRuntime_MoDispatcher_Vt elementType,
    OH_AbilityRuntime_MoDispatcher_VectorHandle* ppVector)
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

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::VectorGetElementType(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, OH_AbilityRuntime_MoDispatcher_Vt* pElementType)
{
    if (pVector == nullptr || pElementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pElementType = pVector->elementType;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::VectorAdd(OH_AbilityRuntime_MoDispatcher_VectorHandle pVector,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pVector == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantType(pValue, pVector->elementType);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    MoVariantStorage storage;
    ret = StoreVariant(pValue, &storage);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    pVector->elements.emplace_back(std::move(storage));
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::VectorGet(OH_AbilityRuntime_MoDispatcher_VectorHandle pVector,
    uint32_t index, OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pVector == nullptr || pValue == nullptr || index >= pVector->elements.size()) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return LoadVariant(pVector->elements[index], pValue);
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::VectorGetSize(OH_AbilityRuntime_MoDispatcher_VectorHandle pVector,
    uint32_t* pSize)
{
    if (pVector == nullptr || pSize == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pVector->elements.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::VectorClear(OH_AbilityRuntime_MoDispatcher_VectorHandle pVector)
{
    if (pVector == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pVector->elements.clear();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void MoDispatcherComplexTypeManager::VectorRelease(OH_AbilityRuntime_MoDispatcher_VectorHandle* ppVector)
{
    if (ppVector == nullptr || *ppVector == nullptr) {
        return;
    }
    delete *ppVector;
    *ppVector = nullptr;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetCreate(OH_AbilityRuntime_MoDispatcher_Vt elementType,
    OH_AbilityRuntime_MoDispatcher_SetHandle** ppSet)
{
    if (ppSet == nullptr || *ppSet == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* set = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Set();
    if (set == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    set->elementType = elementType;
    **ppSet = set;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetGetElementType(OH_AbilityRuntime_MoDispatcher_SetHandle pSet,
    OH_AbilityRuntime_MoDispatcher_Vt* pElementType)
{
    if (pSet == nullptr || pElementType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pElementType = pSet->elementType;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetAdd(OH_AbilityRuntime_MoDispatcher_SetHandle pSet,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pSet == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantType(pValue, pSet->elementType);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    for (const auto& element : pSet->elements) {
        if (VariantEquals(element, pValue)) {
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
    }
    MoVariantStorage storage;
    ret = StoreVariant(pValue, &storage);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    pSet->elements.emplace_back(std::move(storage));
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetRemove(OH_AbilityRuntime_MoDispatcher_SetHandle pSet,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pSet == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto iter = std::find_if(pSet->elements.begin(), pSet->elements.end(),
        [pValue](const MoVariantStorage& item) { return VariantEquals(item, pValue); });
    if (iter != pSet->elements.end()) {
        pSet->elements.erase(iter);
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetContains(OH_AbilityRuntime_MoDispatcher_SetHandle pSet,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue, bool* pExists)
{
    if (pSet == nullptr || pValue == nullptr || pExists == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pExists = std::any_of(pSet->elements.begin(), pSet->elements.end(),
        [pValue](const MoVariantStorage& item) { return VariantEquals(item, pValue); });
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetGetSize(OH_AbilityRuntime_MoDispatcher_SetHandle pSet,
    uint32_t* pSize)
{
    if (pSet == nullptr || pSize == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pSet->elements.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::SetClear(OH_AbilityRuntime_MoDispatcher_SetHandle pSet)
{
    if (pSet == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pSet->elements.clear();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void MoDispatcherComplexTypeManager::SetRelease(OH_AbilityRuntime_MoDispatcher_SetHandle* ppSet)
{
    if (ppSet == nullptr || *ppSet == nullptr) {
        return;
    }
    delete *ppSet;
    *ppSet = nullptr;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapCreate(OH_AbilityRuntime_MoDispatcher_Vt keyType,
    OH_AbilityRuntime_MoDispatcher_Vt valueType, OH_AbilityRuntime_MoDispatcher_MapHandle* ppMap)
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

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapGetKeyType(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
    OH_AbilityRuntime_MoDispatcher_Vt* pKeyType)
{
    if (pMap == nullptr || pKeyType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pKeyType = pMap->keyType;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapGetValueType(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
    OH_AbilityRuntime_MoDispatcher_Vt* pValueType)
{
    if (pMap == nullptr || pValueType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pValueType = pMap->valueType;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapPut(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
    const OH_AbilityRuntime_MoDispatcher_Variant* pKey, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pMap == nullptr || pKey == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ValidateVariantType(pKey, pMap->keyType);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    ret = ValidateVariantType(pValue, pMap->valueType);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }

    for (auto& entry : pMap->entries) {
        if (VariantEquals(entry.first, pKey)) {
            return StoreVariant(pValue, &entry.second);
        }
    }

    std::pair<MoVariantStorage, MoVariantStorage> entry;
    ret = StoreVariant(pKey, &entry.first);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    ret = StoreVariant(pValue, &entry.second);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    pMap->entries.emplace_back(std::move(entry));
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapGet(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
    const OH_AbilityRuntime_MoDispatcher_Variant* pKey, OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pMap == nullptr || pKey == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    for (const auto& entry : pMap->entries) {
        if (VariantEquals(entry.first, pKey)) {
            return LoadVariant(entry.second, pValue);
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapRemove(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
    const OH_AbilityRuntime_MoDispatcher_Variant* pKey)
{
    if (pMap == nullptr || pKey == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto iter = std::find_if(pMap->entries.begin(), pMap->entries.end(),
        [pKey](const auto& item) { return VariantEquals(item.first, pKey); });
    if (iter != pMap->entries.end()) {
        pMap->entries.erase(iter);
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapContainsKey(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
    const OH_AbilityRuntime_MoDispatcher_Variant* pKey, bool* pExists)
{
    if (pMap == nullptr || pKey == nullptr || pExists == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pExists = std::any_of(pMap->entries.begin(), pMap->entries.end(),
        [pKey](const auto& item) { return VariantEquals(item.first, pKey); });
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapGetSize(OH_AbilityRuntime_MoDispatcher_MapHandle pMap,
    uint32_t* pSize)
{
    if (pMap == nullptr || pSize == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *pSize = static_cast<uint32_t>(pMap->entries.size());
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::MapClear(OH_AbilityRuntime_MoDispatcher_MapHandle pMap)
{
    if (pMap == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    pMap->entries.clear();
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void MoDispatcherComplexTypeManager::MapRelease(OH_AbilityRuntime_MoDispatcher_MapHandle* ppMap)
{
    if (ppMap == nullptr || *ppMap == nullptr) {
        return;
    }
    delete *ppMap;
    *ppMap = nullptr;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::StructCreate(const char* structName,
    OH_AbilityRuntime_MoDispatcher_StructHandle* ppStruct)
{
    if (structName == nullptr || ppStruct == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto* object = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_Struct();
    if (object == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    object->name = structName;
    {
        std::lock_guard<std::mutex> lock(g_structMetaMutex);
        auto iter = g_structFieldTypes.find(object->name);
        if (iter != g_structFieldTypes.end()) {
            object->fieldTypes = iter->second;
        }
    }
    *ppStruct = object;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::StructGetName(OH_AbilityRuntime_MoDispatcher_StructHandle pStruct,
    char* pbstrName, uint32_t cMaxName)
{
    if (pStruct == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return CopyStringToBuffer(pStruct->name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::StructSetField(OH_AbilityRuntime_MoDispatcher_StructHandle pStruct,
    const char* szName, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pStruct == nullptr || szName == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto fieldTypeIter = pStruct->fieldTypes.find(szName);
    if (fieldTypeIter == pStruct->fieldTypes.end()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    auto ret = ValidateVariantType(pValue, fieldTypeIter->second);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    MoVariantStorage storage;
    ret = StoreVariant(pValue, &storage);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    pStruct->fields[szName] = std::move(storage);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::StructGetField(OH_AbilityRuntime_MoDispatcher_StructHandle pStruct,
    const char* szName, OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{
    if (pStruct == nullptr || szName == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto iter = pStruct->fields.find(szName);
    if (iter == pStruct->fields.end()) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    return LoadVariant(iter->second, pValue);
}

void MoDispatcherComplexTypeManager::StructRelease(OH_AbilityRuntime_MoDispatcher_StructHandle* ppStruct)
{
    if (ppStruct == nullptr || *ppStruct == nullptr) {
        return;
    }
    delete *ppStruct;
    *ppStruct = nullptr;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::StoreVariant(const OH_AbilityRuntime_MoDispatcher_Variant* src,
    MoVariantStorage* dst)
{
    if (src == nullptr || dst == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    dst->value = *src;
    dst->stringStorage.clear();
    if (src->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING) {
        dst->stringStorage = (src->u.bstrVal != nullptr) ? src->u.bstrVal : "";
        dst->value.u.bstrVal = const_cast<char*>(dst->stringStorage.c_str());
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::LoadVariant(const MoVariantStorage& src,
    OH_AbilityRuntime_MoDispatcher_Variant* dst)
{
    if (dst == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    *dst = src.value;
    if (dst->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING) {
        dst->u.bstrVal = const_cast<char*>(src.stringStorage.c_str());
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherComplexTypeManager::ValidateVariantType(
    const OH_AbilityRuntime_MoDispatcher_Variant* value, OH_AbilityRuntime_MoDispatcher_Vt expectedType)
{
    if (value == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (value->vt == expectedType) {
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if ((expectedType == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY &&
        value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_STUB) ||
        (expectedType == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_STUB &&
        value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY)) {
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
}

bool MoDispatcherComplexTypeManager::VariantEquals(const MoVariantStorage& lhs,
    const OH_AbilityRuntime_MoDispatcher_Variant* rhs)
{
    if (rhs == nullptr) {
        return false;
    }
    if (lhs.value.vt != rhs->vt) {
        return false;
    }
    switch (lhs.value.vt) {
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY:
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID:
            return true;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL:
            return lhs.value.u.boolVal == rhs->u.boolVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I8:
            return lhs.value.u.i8Val == rhs->u.i8Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I16:
            return lhs.value.u.i16Val == rhs->u.i16Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32:
            return lhs.value.u.i32Val == rhs->u.i32Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64:
            return lhs.value.u.i64Val == rhs->u.i64Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U8:
            return lhs.value.u.u8Val == rhs->u.u8Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U16:
            return lhs.value.u.u16Val == rhs->u.u16Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U32:
            return lhs.value.u.u32Val == rhs->u.u32Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U64:
            return lhs.value.u.u64Val == rhs->u.u64Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F32:
            return lhs.value.u.f32Val == rhs->u.f32Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64:
            return lhs.value.u.f64Val == rhs->u.f64Val;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING: {
            const char* lhsStr = lhs.stringStorage.c_str();
            const char* rhsStr = (rhs->u.bstrVal != nullptr) ? rhs->u.bstrVal : "";
            return std::strcmp(lhsStr, rhsStr) == 0;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY:
            return lhs.value.u.parrayVal == rhs->u.parrayVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR:
            return lhs.value.u.pvectorVal == rhs->u.pvectorVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET:
            return lhs.value.u.psetVal == rhs->u.psetVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP:
            return lhs.value.u.pmapVal == rhs->u.pmapVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT:
            return lhs.value.u.pstructVal == rhs->u.pstructVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY:
            return lhs.value.u.premoteProxyVal == rhs->u.premoteProxyVal;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_STUB:
            return lhs.value.u.premoteStubVal == rhs->u.premoteStubVal;
        default:
            return false;
    }
}

void MoDispatcherComplexTypeManager::RegisterStructMetadata(const std::vector<MoStructMeta>& structs)
{
    std::lock_guard<std::mutex> lock(g_structMetaMutex);
    g_structFieldTypes.clear();
    g_structFieldOrder.clear();
    for (const auto& structMeta : structs) {
        std::unordered_map<std::string, OH_AbilityRuntime_MoDispatcher_Vt> fieldTypeMap;
        std::vector<std::string> fieldOrder;
        for (const auto& field : structMeta.fields) {
            fieldTypeMap[field.name] = field.type;
            fieldOrder.emplace_back(field.name);
        }
        g_structFieldTypes[structMeta.name] = std::move(fieldTypeMap);
        g_structFieldOrder[structMeta.name] = std::move(fieldOrder);
    }
}

bool MoDispatcherComplexTypeManager::GetStructFieldType(const std::string& structName, const std::string& fieldName,
    OH_AbilityRuntime_MoDispatcher_Vt* fieldType)
{
    if (fieldType == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(g_structMetaMutex);
    auto structIter = g_structFieldTypes.find(structName);
    if (structIter == g_structFieldTypes.end()) {
        return false;
    }
    auto fieldIter = structIter->second.find(fieldName);
    if (fieldIter == structIter->second.end()) {
        return false;
    }
    *fieldType = fieldIter->second;
    return true;
}

bool MoDispatcherComplexTypeManager::GetStructFieldNames(const std::string& structName,
    std::vector<std::string>* fieldNames)
{
    if (fieldNames == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(g_structMetaMutex);
    auto iter = g_structFieldOrder.find(structName);
    if (iter == g_structFieldOrder.end()) {
        return false;
    }
    *fieldNames = iter->second;
    return true;
}

bool MoDispatcherComplexTypeManager::IsPrimitiveType(OH_AbilityRuntime_MoDispatcher_Vt type)
{
    return type >= OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL && type <= OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64;
}
} // namespace OHOS::AbilityRuntime
