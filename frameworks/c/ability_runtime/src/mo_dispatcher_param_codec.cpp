
/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 */

#include "mo_dispatcher_param_codec.h"

#include <cstdlib>
#include <cstring>

#include "ipc_error_code.h"
#include "mo_dispatcher_complex_type_manager.h"
#include "securec.h"

namespace OHOS::AbilityRuntime {
namespace {
AbilityRuntime_ErrorCode CheckIPC(int32_t ipcRet)
{
    return ipcRet == OH_IPC_SUCCESS ? ABILITY_RUNTIME_ERROR_CODE_NO_ERROR : ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}

AbilityRuntime_ErrorCode WriteTypeTag(OHIPCParcel* parcel, OH_AbilityRuntime_MoDispatcher_Vt type)
{
    return CheckIPC(OH_IPCParcel_WriteInt32(parcel, static_cast<int32_t>(type)));
}

AbilityRuntime_ErrorCode ReadTypeTag(const OHIPCParcel* parcel, OH_AbilityRuntime_MoDispatcher_Vt* type)
{
    int32_t rawType = 0;
    auto ret = CheckIPC(OH_IPCParcel_ReadInt32(parcel, &rawType));
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    *type = static_cast<OH_AbilityRuntime_MoDispatcher_Vt>(rawType);
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}
} // namespace

AbilityRuntime_ErrorCode MoDispatcherParamCodec::ValidateInputParams(const MoMethodMeta& methodMeta,
    const OH_AbilityRuntime_MoDispatcher_Inputparams* inputParams)
{
    if (inputParams == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    if (inputParams->cArgs != methodMeta.params.size() || inputParams->rgvarg == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_TYPE_MISMATCH;
    }
    for (uint32_t i = 0; i < inputParams->cArgs; i++) {
        auto ret = MoDispatcherComplexTypeManager::ValidateVariantType(&inputParams->rgvarg[i], methodMeta.params[i].type);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
            return ret;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherParamCodec::MarshalCallRequest(uint32_t memberId,
    const OH_AbilityRuntime_MoDispatcher_Inputparams* inputParams, OHIPCParcel* dataParcel)
{
    if (dataParcel == nullptr || inputParams == nullptr || inputParams->rgvarg == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = CheckIPC(OH_IPCParcel_WriteInt32(dataParcel, static_cast<int32_t>(memberId)));
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    ret = CheckIPC(OH_IPCParcel_WriteInt32(dataParcel, static_cast<int32_t>(inputParams->cArgs)));
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }
    for (uint32_t i = 0; i < inputParams->cArgs; i++) {
        ret = WriteVariant(dataParcel, &inputParams->rgvarg[i]);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
            return ret;
        }
    }
    return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
}

AbilityRuntime_ErrorCode MoDispatcherParamCodec::UnmarshalCallResult(const OHIPCParcel* replyParcel,
    OH_AbilityRuntime_MoDispatcher_Variant* result)
{
    if (replyParcel == nullptr || result == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    return ReadVariant(replyParcel, result);
}

AbilityRuntime_ErrorCode MoDispatcherParamCodec::WriteVariant(OHIPCParcel* parcel,
    const OH_AbilityRuntime_MoDispatcher_Variant* value)
{
    if (parcel == nullptr || value == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = WriteTypeTag(parcel, value->vt);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }

    switch (value->vt) {
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY:
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID:
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL:
            return CheckIPC(OH_IPCParcel_WriteInt8(parcel, value->u.boolVal ? 1 : 0));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I8:
            return CheckIPC(OH_IPCParcel_WriteInt8(parcel, value->u.i8Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I16:
            return CheckIPC(OH_IPCParcel_WriteInt16(parcel, value->u.i16Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32:
            return CheckIPC(OH_IPCParcel_WriteInt32(parcel, value->u.i32Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64:
            return CheckIPC(OH_IPCParcel_WriteInt64(parcel, value->u.i64Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U8:
            return CheckIPC(OH_IPCParcel_WriteInt8(parcel, static_cast<int8_t>(value->u.u8Val)));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U16:
            return CheckIPC(OH_IPCParcel_WriteInt16(parcel, static_cast<int16_t>(value->u.u16Val)));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U32:
            return CheckIPC(OH_IPCParcel_WriteInt32(parcel, static_cast<int32_t>(value->u.u32Val)));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U64:
            return CheckIPC(OH_IPCParcel_WriteInt64(parcel, static_cast<int64_t>(value->u.u64Val)));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F32:
            return CheckIPC(OH_IPCParcel_WriteFloat(parcel, value->u.f32Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64:
            return CheckIPC(OH_IPCParcel_WriteDouble(parcel, value->u.f64Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING:
            return CheckIPC(OH_IPCParcel_WriteString(parcel, value->u.bstrVal == nullptr ? "" : value->u.bstrVal));
        default:
            break;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY) {
        if (value->u.parrayVal == nullptr) {
            return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        }
        ret = CheckIPC(OH_IPCParcel_WriteInt32(parcel, static_cast<int32_t>(value->u.parrayVal->elementType)));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckIPC(OH_IPCParcel_WriteInt32(parcel, static_cast<int32_t>(value->u.parrayVal->elements.size())));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (const auto& item : value->u.parrayVal->elements) {
            OH_AbilityRuntime_MoDispatcher_Variant temp;
            MoDispatcherComplexTypeManager::LoadVariant(item, &temp);
            ret = WriteVariant(parcel, &temp);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR) {
        if (value->u.pvectorVal == nullptr) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        ret = CheckIPC(OH_IPCParcel_WriteInt32(parcel, static_cast<int32_t>(value->u.pvectorVal->elementType)));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckIPC(OH_IPCParcel_WriteInt32(parcel, static_cast<int32_t>(value->u.pvectorVal->elements.size())));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (const auto& item : value->u.pvectorVal->elements) {
            OH_AbilityRuntime_MoDispatcher_Variant temp;
            MoDispatcherComplexTypeManager::LoadVariant(item, &temp);
            ret = WriteVariant(parcel, &temp);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET) {
        if (value->u.psetVal == nullptr) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        ret = CheckIPC(OH_IPCParcel_WriteInt32(parcel, static_cast<int32_t>(value->u.psetVal->elementType)));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckIPC(OH_IPCParcel_WriteInt32(parcel, static_cast<int32_t>(value->u.psetVal->elements.size())));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (const auto& item : value->u.psetVal->elements) {
            OH_AbilityRuntime_MoDispatcher_Variant temp;
            MoDispatcherComplexTypeManager::LoadVariant(item, &temp);
            ret = WriteVariant(parcel, &temp);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP) {
        if (value->u.pmapVal == nullptr) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        ret = CheckIPC(OH_IPCParcel_WriteInt32(parcel, static_cast<int32_t>(value->u.pmapVal->keyType)));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckIPC(OH_IPCParcel_WriteInt32(parcel, static_cast<int32_t>(value->u.pmapVal->valueType)));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckIPC(OH_IPCParcel_WriteInt32(parcel, static_cast<int32_t>(value->u.pmapVal->entries.size())));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (const auto& item : value->u.pmapVal->entries) {
            OH_AbilityRuntime_MoDispatcher_Variant key;
            OH_AbilityRuntime_MoDispatcher_Variant val;
            MoDispatcherComplexTypeManager::LoadVariant(item.first, &key);
            MoDispatcherComplexTypeManager::LoadVariant(item.second, &val);
            ret = WriteVariant(parcel, &key);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = WriteVariant(parcel, &val);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT) {
        if (value->u.pstructVal == nullptr) return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
        ret = CheckIPC(OH_IPCParcel_WriteString(parcel, value->u.pstructVal->name.c_str()));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckIPC(OH_IPCParcel_WriteInt32(parcel, static_cast<int32_t>(value->u.pstructVal->fields.size())));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (const auto& item : value->u.pstructVal->fields) {
            ret = CheckIPC(OH_IPCParcel_WriteString(parcel, item.first.c_str()));
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            OH_AbilityRuntime_MoDispatcher_Variant temp;
            MoDispatcherComplexTypeManager::LoadVariant(item.second, &temp);
            ret = WriteVariant(parcel, &temp);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY) {
        return CheckIPC(OH_IPCParcel_WriteRemoteProxy(parcel, value->u.premoteProxyVal));
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_STUB) {
        return CheckIPC(OH_IPCParcel_WriteRemoteStub(parcel, value->u.premoteStubVal));
    }
    return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}

AbilityRuntime_ErrorCode MoDispatcherParamCodec::ReadVariant(const OHIPCParcel* parcel,
    OH_AbilityRuntime_MoDispatcher_Variant* value)
{
    if (parcel == nullptr || value == nullptr) {
        return ABILITY_RUNTIME_ERROR_CODE_PARAM_INVALID;
    }
    auto ret = ReadTypeTag(parcel, &value->vt);
    if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) {
        return ret;
    }

    switch (value->vt) {
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_EMPTY:
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VOID:
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_BOOL: {
            int8_t v = 0;
            ret = CheckIPC(OH_IPCParcel_ReadInt8(parcel, &v));
            value->u.boolVal = (v != 0);
            return ret;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I8:
            return CheckIPC(OH_IPCParcel_ReadInt8(parcel, &value->u.i8Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I16:
            return CheckIPC(OH_IPCParcel_ReadInt16(parcel, &value->u.i16Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I32:
            return CheckIPC(OH_IPCParcel_ReadInt32(parcel, &value->u.i32Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_I64:
            return CheckIPC(OH_IPCParcel_ReadInt64(parcel, &value->u.i64Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U8: {
            int8_t v = 0;
            ret = CheckIPC(OH_IPCParcel_ReadInt8(parcel, &v));
            value->u.u8Val = static_cast<uint8_t>(v);
            return ret;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U16: {
            int16_t v = 0;
            ret = CheckIPC(OH_IPCParcel_ReadInt16(parcel, &v));
            value->u.u16Val = static_cast<uint16_t>(v);
            return ret;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U32: {
            int32_t v = 0;
            ret = CheckIPC(OH_IPCParcel_ReadInt32(parcel, &v));
            value->u.u32Val = static_cast<uint32_t>(v);
            return ret;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_U64: {
            int64_t v = 0;
            ret = CheckIPC(OH_IPCParcel_ReadInt64(parcel, &v));
            value->u.u64Val = static_cast<uint64_t>(v);
            return ret;
        }
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F32:
            return CheckIPC(OH_IPCParcel_ReadFloat(parcel, &value->u.f32Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_F64:
            return CheckIPC(OH_IPCParcel_ReadDouble(parcel, &value->u.f64Val));
        case OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRING: {
            const char* text = OH_IPCParcel_ReadString(parcel);
            if (text == nullptr) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
            const auto len = std::strlen(text);
            auto* mem = static_cast<char*>(std::malloc(len + 1));
            if (mem == nullptr) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
            if (strcpy_s(mem, len + 1, text) != EOK) {
                std::free(mem);
                return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
            }
            value->u.bstrVal = mem;
            return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
        }
        default:
            break;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_ARRAY) {
        int32_t elemType = 0;
        int32_t size = 0;
        ret = CheckIPC(OH_IPCParcel_ReadInt32(parcel, &elemType));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckIPC(OH_IPCParcel_ReadInt32(parcel, &size));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR || size < 0) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        OH_AbilityRuntime_MoDispatcher_ArrayHandle array = nullptr;
        ret = MoDispatcherComplexTypeManager::ArrayCreate(static_cast<OH_AbilityRuntime_MoDispatcher_Vt>(elemType),
            static_cast<uint32_t>(size), &array);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (uint32_t i = 0; i < static_cast<uint32_t>(size); i++) {
            OH_AbilityRuntime_MoDispatcher_Variant elem;
            ret = ReadVariant(parcel, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = MoDispatcherComplexTypeManager::ArraySet(array, i, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        value->u.parrayVal = array;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_VECTOR) {
        int32_t elemType = 0;
        int32_t size = 0;
        ret = CheckIPC(OH_IPCParcel_ReadInt32(parcel, &elemType));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckIPC(OH_IPCParcel_ReadInt32(parcel, &size));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR || size < 0) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        OH_AbilityRuntime_MoDispatcher_VectorHandle vector = nullptr;
        ret = MoDispatcherComplexTypeManager::VectorCreate(static_cast<OH_AbilityRuntime_MoDispatcher_Vt>(elemType), &vector);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (uint32_t i = 0; i < static_cast<uint32_t>(size); i++) {
            OH_AbilityRuntime_MoDispatcher_Variant elem;
            ret = ReadVariant(parcel, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = MoDispatcherComplexTypeManager::VectorAdd(vector, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        value->u.pvectorVal = vector;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_SET) {
        int32_t elemType = 0;
        int32_t size = 0;
        ret = CheckIPC(OH_IPCParcel_ReadInt32(parcel, &elemType));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckIPC(OH_IPCParcel_ReadInt32(parcel, &size));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR || size < 0) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        OH_AbilityRuntime_MoDispatcher_SetHandle setHandle = nullptr;
        OH_AbilityRuntime_MoDispatcher_SetHandle* setHolder = &setHandle;
        ret = MoDispatcherComplexTypeManager::SetCreate(static_cast<OH_AbilityRuntime_MoDispatcher_Vt>(elemType), &setHolder);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (uint32_t i = 0; i < static_cast<uint32_t>(size); i++) {
            OH_AbilityRuntime_MoDispatcher_Variant elem;
            ret = ReadVariant(parcel, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = MoDispatcherComplexTypeManager::SetAdd(setHandle, &elem);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        value->u.psetVal = setHandle;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_MAP) {
        int32_t keyType = 0;
        int32_t valueType = 0;
        int32_t size = 0;
        ret = CheckIPC(OH_IPCParcel_ReadInt32(parcel, &keyType));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckIPC(OH_IPCParcel_ReadInt32(parcel, &valueType));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        ret = CheckIPC(OH_IPCParcel_ReadInt32(parcel, &size));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR || size < 0) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        OH_AbilityRuntime_MoDispatcher_MapHandle map = nullptr;
        ret = MoDispatcherComplexTypeManager::MapCreate(static_cast<OH_AbilityRuntime_MoDispatcher_Vt>(keyType),
            static_cast<OH_AbilityRuntime_MoDispatcher_Vt>(valueType), &map);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (uint32_t i = 0; i < static_cast<uint32_t>(size); i++) {
            OH_AbilityRuntime_MoDispatcher_Variant k;
            OH_AbilityRuntime_MoDispatcher_Variant v;
            ret = ReadVariant(parcel, &k);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = ReadVariant(parcel, &v);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = MoDispatcherComplexTypeManager::MapPut(map, &k, &v);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        value->u.pmapVal = map;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_STRUCT) {
        const char* structName = OH_IPCParcel_ReadString(parcel);
        if (structName == nullptr) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        int32_t fieldCount = 0;
        ret = CheckIPC(OH_IPCParcel_ReadInt32(parcel, &fieldCount));
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR || fieldCount < 0) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
        OH_AbilityRuntime_MoDispatcher_StructHandle structObj = nullptr;
        ret = MoDispatcherComplexTypeManager::StructCreate(structName, &structObj);
        if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        for (uint32_t i = 0; i < static_cast<uint32_t>(fieldCount); i++) {
            const char* fieldName = OH_IPCParcel_ReadString(parcel);
            if (fieldName == nullptr) return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
            OH_AbilityRuntime_MoDispatcher_Variant fieldValue;
            ret = ReadVariant(parcel, &fieldValue);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
            ret = MoDispatcherComplexTypeManager::StructSetField(structObj, fieldName, &fieldValue);
            if (ret != ABILITY_RUNTIME_ERROR_CODE_NO_ERROR) return ret;
        }
        value->u.pstructVal = structObj;
        return ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_PROXY) {
        value->u.premoteProxyVal = OH_IPCParcel_ReadRemoteProxy(parcel);
        return value->u.premoteProxyVal == nullptr ? ABILITY_RUNTIME_ERROR_CODE_INTERNAL : ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    if (value->vt == OH_ABILITY_RUNTIME_MO_DISPATCHER_VT_IPC_REMOTE_STUB) {
        value->u.premoteStubVal = OH_IPCParcel_ReadRemoteStub(parcel);
        return value->u.premoteStubVal == nullptr ? ABILITY_RUNTIME_ERROR_CODE_INTERNAL : ABILITY_RUNTIME_ERROR_CODE_NO_ERROR;
    }
    return ABILITY_RUNTIME_ERROR_CODE_INTERNAL;
}
} // namespace OHOS::AbilityRuntime
