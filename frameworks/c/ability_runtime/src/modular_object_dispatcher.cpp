
/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "modular_object_dispatcher.h"

#include <memory>
#include <string>

#include "ipc_cremote_object.h"
#include "mo_dispatcher_complex_type_manager.h"
#include "mo_dispatcher_metadata_manager.h"
#include "mo_dispatcher_param_codec.h"
#include "mo_dispatcher_types.h"
#include "securec.h"

using OHOS::AbilityRuntime::MoDispatcherComplexTypeManager;
using OHOS::AbilityRuntime::MoDispatcherMetadataManager;
using OHOS::AbilityRuntime::MoDispatcherParamCodec;

namespace {
AbilityRuntime_ErrorCode CopyStringToBuffer(const std::string& src, char* dst, uint32_t max)
{
    if (dst == nullptr || max == 0 || src.size() + 1 > max) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return strcpy_s(dst, max, src.c_str()) == EOK ? ABILITY_RUNTIME_ERROR_CODE_NO_ERROR :
        ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}

AbilityRuntime_ErrorCode EnsureMetadataLoaded(OH_AbilityRuntime_MoDispatcherHandle dispatcher, bool mapSendFailed)
{
    bool sendFailed = false;
    auto ret = dispatcher->metadataManager->EnsureLoaded(dispatcher->remoteProxy, &sendFailed);
    if (ret == ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    if (mapSendFailed && sendFailed) {
        return ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED;
    }
    return ret;
}
} // namespace

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
    dispatcher->metadataManager = std::make_shared<MoDispatcherMetadataManager>();
    *ppMoDispatcher = dispatcher;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

void OH_AbilityRuntime_MoDispatcher_Release(OH_AbilityRuntime_MoDispatcherHandle* ppMoDispatcher)
{
    if (ppMoDispatcher == nullptr || *ppMoDispatcher == nullptr) {
        return;
    }
    delete *ppMoDispatcher;
    *ppMoDispatcher = nullptr;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_HasTypeDescriptor(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher, uint32_t* pctinfo)
{
    if (pMoDispatcher == nullptr || pctinfo == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = pMoDispatcher->metadataManager->EnsureLoaded(pMoDispatcher->remoteProxy, nullptr);
    *pctinfo = (ret == ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) ? 1 : 0;
    return (ret == ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) ? ABILITY_RUNTIME_ERROR_CODE_NO_ERROR :
        ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_GetTypeDescriptor(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher, OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle* ppTypeDescriptor)
{
    if (pMoDispatcher == nullptr || ppTypeDescriptor == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = EnsureMetadataLoaded(pMoDispatcher, true);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    auto* typeDesc = new (std::nothrow) OH_AbilityRuntime_MoDispatcher_TypeDescriptor();
    if (typeDesc == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    typeDesc->metadataManager = pMoDispatcher->metadataManager;
    *ppTypeDescriptor = typeDesc;
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_QueryMemIDsOfNames(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher, const char** rgszNames, uint32_t cNames,
    uint32_t* pMemID)
{
    if (pMoDispatcher == nullptr || rgszNames == nullptr || pMemID == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = EnsureMetadataLoaded(pMoDispatcher, false);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
    }
    return pMoDispatcher->metadataManager->QueryMemberIds(rgszNames, cNames, pMemID);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_CallMethod(
    OH_AbilityRuntime_MoDispatcherHandle pMoDispatcher, uint32_t memID,
    OH_AbilityRuntime_MoDispatcher_Inputparams* pInputParams,
    OH_AbilityRuntime_MoDispatcher_Variant* pResult)
{
    if (pMoDispatcher == nullptr || pInputParams == nullptr || pResult == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = EnsureMetadataLoaded(pMoDispatcher, true);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }

    OHOS::AbilityRuntime::MoMethodMeta methodMeta;
    ret = pMoDispatcher->metadataManager->GetMethodMeta(memID, &methodMeta);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ABILITY_RUNTIME_ERROR_CODE_PROPERTY_NOT_FOUND;
    }
    ret = MoDispatcherParamCodec::ValidateInputParams(methodMeta, pInputParams);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
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

    ret = MoDispatcherParamCodec::MarshalCallRequest(memID, pInputParams, dataParcel);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        OH_IPCParcel_Destroy(dataParcel);
        OH_IPCParcel_Destroy(replyParcel);
        return ret;
    }

    OH_IPC_MessageOption option { .mode = OH_IPC_REQUEST_MODE_SYNC, .timeout = 0, .reserved = nullptr };
    int ipcRet = OH_IPCRemoteProxy_SendRequest(pMoDispatcher->remoteProxy, methodMeta.ipcCode, dataParcel, replyParcel, &option);
    if (ipcRet != OH_IPC_SUCCESS) {
        OH_IPCParcel_Destroy(dataParcel);
        OH_IPCParcel_Destroy(replyParcel);
        return ABILITY_RUNTIME_ERROR_CODE_SEND_REQUEST_FAILED;
    }

    ret = MoDispatcherParamCodec::UnmarshalCallResult(replyParcel, pResult);
    OH_IPCParcel_Destroy(dataParcel);
    OH_IPCParcel_Destroy(replyParcel);
    return ret;
}
void OH_AbilityRuntime_TypeDescriptor_Release(OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle *pTypeDescriptor)
{
    if (pTypeDescriptor == nullptr || *pTypeDescriptor == nullptr) {
        return;
    }
    delete *pTypeDescriptor;
    *pTypeDescriptor = nullptr;
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetPackage(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, char* pbstrPackage, uint32_t cMaxPackage)
{
    if (pTypeDescriptor == nullptr || pbstrPackage == nullptr || cMaxPackage == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string package;
    auto ret = pTypeDescriptor->metadataManager->GetPackage(&package);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(package, pbstrPackage, cMaxPackage);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetVersion(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, char* pbstrVersion, uint32_t cMaxVersion)
{
    if (pTypeDescriptor == nullptr || pbstrVersion == nullptr || cMaxVersion == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string version;
    auto ret = pTypeDescriptor->metadataManager->GetVersion(&version);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(version, pbstrVersion, cMaxVersion);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMemIDsOfNames(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* szName, uint32_t* pMemID)
{
    if (pTypeDescriptor == nullptr || szName == nullptr || pMemID == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->QueryMemberId(szName, pMemID);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMemberName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t memID, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->QueryMemberName(memID, &name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcInterfaces)
{
    if (pTypeDescriptor == nullptr || pcInterfaces == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetInterfaceCount(pcInterfaces);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetInterfaceName(index, &name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceMemberId(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrName, uint32_t* pMemID)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || pMemID == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetInterfaceMemberId(pbstrName, pMemID);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceDescriptor(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrName,
    char* pbstrDescriptor, uint32_t cMaxDescriptor)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || pbstrDescriptor == nullptr || cMaxDescriptor == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string descriptor;
    auto ret = pTypeDescriptor->metadataManager->GetInterfaceDescriptor(pbstrName, &descriptor);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(descriptor, pbstrDescriptor, cMaxDescriptor);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetInterfaceIsCallback(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrName, bool* pIsCallback)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || pIsCallback == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetInterfaceIsCallback(pbstrName, pIsCallback);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMainServiceInterfaceName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetMainServiceInterfaceName(&name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodCount(
    const char* pbstrDescriptor, const char* pbstrInterfaceName, uint32_t* pcMethods)
{
    return MoDispatcherMetadataManager::GetMethodCountFromDescriptor(pbstrDescriptor, pbstrInterfaceName, pcMethods);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodName(
    const char* pbstrDescriptor, const char* pbstrInterfaceName, uint32_t index,
    char* pbstrName, uint32_t cMaxName)
{
    if (pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string methodName;
    auto ret = MoDispatcherMetadataManager::GetMethodNameFromDescriptor(pbstrDescriptor, pbstrInterfaceName, index, &methodName);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(methodName, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodMemberId(
    const char* pbstrDescriptor, const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t* pMemID)
{
    return MoDispatcherMetadataManager::GetMethodMemberIdFromDescriptor(
        pbstrDescriptor, pbstrInterfaceName, pbstrMethodName, pMemID);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodReturnType(
    const char* pbstrDescriptor, const char* pbstrInterfaceName, const char* pbstrMethodName,
    OH_AbilityRuntime_MoDispatcher_Vt* pReturnType)
{
    return MoDispatcherMetadataManager::GetMethodReturnTypeFromDescriptor(
        pbstrDescriptor, pbstrInterfaceName, pbstrMethodName, pReturnType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamCount(
    const char* pbstrDescriptor, const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t* pcParams)
{
    return MoDispatcherMetadataManager::GetMethodParamCountFromDescriptor(
        pbstrDescriptor, pbstrInterfaceName, pbstrMethodName, pcParams);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamType(
    const char* pbstrDescriptor, const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t iParamIndex, OH_AbilityRuntime_MoDispatcher_Vt* pParamType)
{
    return MoDispatcherMetadataManager::GetMethodParamTypeFromDescriptor(
        pbstrDescriptor, pbstrInterfaceName, pbstrMethodName, iParamIndex, pParamType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetMethodParamName(
    const char* pbstrDescriptor, const char* pbstrInterfaceName, const char* pbstrMethodName,
    uint32_t iParamIndex, char* pbstrName, uint32_t cMaxName)
{
    if (pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string paramName;
    auto ret = MoDispatcherMetadataManager::GetMethodParamNameFromDescriptor(
        pbstrDescriptor, pbstrInterfaceName, pbstrMethodName, iParamIndex, &paramName);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(paramName, pbstrName, cMaxName);
}
AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcEnums)
{
    if (pTypeDescriptor == nullptr || pcEnums == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetEnumCount(pcEnums);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetEnumName(index, &name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumMemberId(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName, uint32_t* pMemID)
{
    if (pTypeDescriptor == nullptr || pbstrEnumName == nullptr || pMemID == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetEnumMemberId(pbstrEnumName, pMemID);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumEnumValueCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName, uint32_t* pcValues)
{
    if (pTypeDescriptor == nullptr || pbstrEnumName == nullptr || pcValues == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetEnumValueCount(pbstrEnumName, pcValues);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValueName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName, uint32_t iValueIndex,
    char* pbstrValueName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrEnumName == nullptr || pbstrValueName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string valueName;
    auto ret = pTypeDescriptor->metadataManager->GetEnumValueName(pbstrEnumName, iValueIndex, &valueName);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(valueName, pbstrValueName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetEnumValue(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrEnumName,
    const char* pbstrValueName, int32_t* pValue)
{
    if (pTypeDescriptor == nullptr || pbstrEnumName == nullptr || pbstrValueName == nullptr || pValue == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetEnumValue(pbstrEnumName, pbstrValueName, pValue);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t* pcStructs)
{
    if (pTypeDescriptor == nullptr || pcStructs == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetStructCount(pcStructs);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, uint32_t index, char* pbstrName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetStructName(index, &name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(name, pbstrName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructMemberId(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName, uint32_t* pMemID)
{
    if (pTypeDescriptor == nullptr || pbstrStructName == nullptr || pMemID == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetStructMemberId(pbstrStructName, pMemID);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldCount(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName, uint32_t* pcFields)
{
    if (pTypeDescriptor == nullptr || pbstrStructName == nullptr || pcFields == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetStructFieldCount(pbstrStructName, pcFields);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldName(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName, uint32_t iFieldIndex,
    char* pbstrFieldName, uint32_t cMaxName)
{
    if (pTypeDescriptor == nullptr || pbstrStructName == nullptr || pbstrFieldName == nullptr || cMaxName == 0) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    std::string name;
    auto ret = pTypeDescriptor->metadataManager->GetStructFieldName(pbstrStructName, iFieldIndex, &name);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    return CopyStringToBuffer(name, pbstrFieldName, cMaxName);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_TypeDescriptor_GetStructFieldType(
    OH_AbilityRuntime_MoDispatcher_TypeDescriptorHandle pTypeDescriptor, const char* pbstrStructName, const char* pbstrFieldName,
    OH_AbilityRuntime_MoDispatcher_Vt* pFieldType)
{
    if (pTypeDescriptor == nullptr || pbstrStructName == nullptr || pbstrFieldName == nullptr || pFieldType == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return pTypeDescriptor->metadataManager->GetStructFieldType(pbstrStructName, pbstrFieldName, pFieldType);
}

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_Create(
    OH_AbilityRuntime_MoDispatcher_Vt elementType, uint32_t size, OH_AbilityRuntime_MoDispatcher_ArrayHandle* ppArray)
{ return MoDispatcherComplexTypeManager::ArrayCreate(elementType, size, ppArray); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_GetElementType(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, OH_AbilityRuntime_MoDispatcher_Vt* pElementType)
{ return MoDispatcherComplexTypeManager::ArrayGetElementType(pArray, pElementType); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_Set(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t index, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{ return MoDispatcherComplexTypeManager::ArraySet(pArray, index, pValue); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_Get(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t index, OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{ return MoDispatcherComplexTypeManager::ArrayGet(pArray, index, pValue); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_GetSize(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t* pSize)
{ return MoDispatcherComplexTypeManager::ArrayGetSize(pArray, pSize); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Array_Resize(
    OH_AbilityRuntime_MoDispatcher_ArrayHandle pArray, uint32_t newSize)
{ return MoDispatcherComplexTypeManager::ArrayResize(pArray, newSize); }
void OH_AbilityRuntime_MoDispatcher_Array_Release(OH_AbilityRuntime_MoDispatcher_ArrayHandle* ppArray)
{ MoDispatcherComplexTypeManager::ArrayRelease(ppArray); }

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Create(
    OH_AbilityRuntime_MoDispatcher_Vt elementType, OH_AbilityRuntime_MoDispatcher_VectorHandle* ppVector)
{ return MoDispatcherComplexTypeManager::VectorCreate(elementType, ppVector); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_GetElementType(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, OH_AbilityRuntime_MoDispatcher_Vt* pElementType)
{ return MoDispatcherComplexTypeManager::VectorGetElementType(pVector, pElementType); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Add(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{ return MoDispatcherComplexTypeManager::VectorAdd(pVector, pValue); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Get(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, uint32_t index, OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{ return MoDispatcherComplexTypeManager::VectorGet(pVector, index, pValue); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_GetSize(
    OH_AbilityRuntime_MoDispatcher_VectorHandle pVector, uint32_t* pSize)
{ return MoDispatcherComplexTypeManager::VectorGetSize(pVector, pSize); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Vector_Clear(OH_AbilityRuntime_MoDispatcher_VectorHandle pVector)
{ return MoDispatcherComplexTypeManager::VectorClear(pVector); }
void OH_AbilityRuntime_MoDispatcher_Vector_Release(OH_AbilityRuntime_MoDispatcher_VectorHandle* ppVector)
{ MoDispatcherComplexTypeManager::VectorRelease(ppVector); }

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Create(
    OH_AbilityRuntime_MoDispatcher_Vt elementType, OH_AbilityRuntime_MoDispatcher_SetHandle** ppSet)
{ return MoDispatcherComplexTypeManager::SetCreate(elementType, ppSet); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_GetElementType(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, OH_AbilityRuntime_MoDispatcher_Vt* pElementType)
{ return MoDispatcherComplexTypeManager::SetGetElementType(pSet, pElementType); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Add(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{ return MoDispatcherComplexTypeManager::SetAdd(pSet, pValue); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Remove(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{ return MoDispatcherComplexTypeManager::SetRemove(pSet, pValue); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Contains(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, const OH_AbilityRuntime_MoDispatcher_Variant* pValue, bool* pExists)
{ return MoDispatcherComplexTypeManager::SetContains(pSet, pValue, pExists); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_GetSize(
    OH_AbilityRuntime_MoDispatcher_SetHandle pSet, uint32_t* pSize)
{ return MoDispatcherComplexTypeManager::SetGetSize(pSet, pSize); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Set_Clear(OH_AbilityRuntime_MoDispatcher_SetHandle pSet)
{ return MoDispatcherComplexTypeManager::SetClear(pSet); }
void OH_AbilityRuntime_MoDispatcher_Set_Release(OH_AbilityRuntime_MoDispatcher_SetHandle* ppSet)
{ MoDispatcherComplexTypeManager::SetRelease(ppSet); }

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Create(
    OH_AbilityRuntime_MoDispatcher_Vt keyType, OH_AbilityRuntime_MoDispatcher_Vt valueType,
    OH_AbilityRuntime_MoDispatcher_MapHandle* ppMap)
{ return MoDispatcherComplexTypeManager::MapCreate(keyType, valueType, ppMap); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetKeyType(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, OH_AbilityRuntime_MoDispatcher_Vt* pKeyType)
{ return MoDispatcherComplexTypeManager::MapGetKeyType(pMap, pKeyType); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetValueType(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, OH_AbilityRuntime_MoDispatcher_Vt* pValueType)
{ return MoDispatcherComplexTypeManager::MapGetValueType(pMap, pValueType); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Put(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{ return MoDispatcherComplexTypeManager::MapPut(pMap, pKey, pValue); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Get(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{ return MoDispatcherComplexTypeManager::MapGet(pMap, pKey, pValue); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Remove(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey)
{ return MoDispatcherComplexTypeManager::MapRemove(pMap, pKey); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_ContainsKey(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, const OH_AbilityRuntime_MoDispatcher_Variant* pKey, bool* pExists)
{ return MoDispatcherComplexTypeManager::MapContainsKey(pMap, pKey, pExists); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_GetSize(
    OH_AbilityRuntime_MoDispatcher_MapHandle pMap, uint32_t* pSize)
{ return MoDispatcherComplexTypeManager::MapGetSize(pMap, pSize); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Map_Clear(OH_AbilityRuntime_MoDispatcher_MapHandle pMap)
{ return MoDispatcherComplexTypeManager::MapClear(pMap); }
void OH_AbilityRuntime_MoDispatcher_Map_Release(OH_AbilityRuntime_MoDispatcher_MapHandle* ppMap)
{ MoDispatcherComplexTypeManager::MapRelease(ppMap); }

AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_Create(
    const char* structName, OH_AbilityRuntime_MoDispatcher_StructHandle* ppStruct)
{ return MoDispatcherComplexTypeManager::StructCreate(structName, ppStruct); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_GetName(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, char* pbstrName, uint32_t cMaxName)
{ return MoDispatcherComplexTypeManager::StructGetName(pStruct, pbstrName, cMaxName); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_SetField(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, const char* szName,
    const OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{ return MoDispatcherComplexTypeManager::StructSetField(pStruct, szName, pValue); }
AbilityRuntime_ErrorCode OH_AbilityRuntime_MoDispatcher_Struct_GetField(
    OH_AbilityRuntime_MoDispatcher_StructHandle pStruct, const char* szName,
    OH_AbilityRuntime_MoDispatcher_Variant* pValue)
{ return MoDispatcherComplexTypeManager::StructGetField(pStruct, szName, pValue); }
void OH_AbilityRuntime_MoDispatcher_Struct_Release(OH_AbilityRuntime_MoDispatcher_StructHandle* ppStruct)
{ MoDispatcherComplexTypeManager::StructRelease(ppStruct); }

#ifdef __cplusplus
}
#endif
