/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "ErrorCodeTest.h"
#include <cstdint>
#include <native_buffer/native_buffer.h>
#include <native_image/native_image.h>
#include <native_vsync/graphic_error_code.h>
#include <native_buffer/graphic_error_code.h>
#include <native_vsync/native_vsync.h>
#include <native_drawing/drawing_error_code.h>
#include <native_drawing/drawing_record_cmd.h>
#include <native_buffer/native_buffer.h>

void addErrorCodeTest(std::vector<napi_property_descriptor>& descData)
{
    descData.push_back({"OHNativeErrorCodeOK", nullptr, OH_OHNativeErrorCodeOK,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHNativeErrorCodeInvalid", nullptr, OH_OHNativeErrorCodeInvalid,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHDrawingBufferErrorCodeOK", nullptr, OH_Drawing_BufferErrorCodeOK,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHNativeErrorCodeBufferInvalid", nullptr, OH_OHNativeErrorCodeBufferInvalid,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHNativeErrorCodeBufferSupport", nullptr, OH_OHNativeErrorCodeBufferSupport,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHDrawingBufferHDIError", nullptr, OH_Drawing_BufferHDIError,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHNativeErrorCodeImageSupport", nullptr, OH_OHNativeErrorCodeImageSupport,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHNativeErrorCodeImageNoBuffer", nullptr, OH_OHNativeErrorCodeImageNoBuffer,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHNativeErrorCodeVSyncOk", nullptr, OH_OHNativeErrorCodeVSyncOk,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHNativeErrorCodeVSyncInvalid", nullptr, OH_OHNativeErrorCodeVSyncInvalid,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHNativeErrorCodeWindowInvalid", nullptr, OH_OHNativeErrorCodeWindowInvalid,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHNativeErrorCodeWindowOk", nullptr, OH_OHNativeErrorCodeWindowOk,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHDrawingErrorCodeSuccess", nullptr, OH_Drawing_DrawingErrorCodeSuccess,
        nullptr, nullptr, nullptr, napi_default, nullptr});
}

 napi_value OH_OHNativeErrorCodeOK(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_NativeImage* image = OH_NativeImage_Create(0, 0);
    int32_t errorCode = OH_NativeImage_SetDropBufferMode(image, true);
    napi_create_int32(env,  errorCode == NATIVE_ERROR_OK, &result);
    return result;
}

 napi_value OH_OHNativeErrorCodeInvalid(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_NativeImage* image = nullptr;
    int32_t errorCode = OH_NativeImage_SetDropBufferMode(image, true);
    napi_create_int32(env,  errorCode == NATIVE_ERROR_INVALID_ARGUMENTS, &result);
    return result;
}

napi_value OH_Drawing_BufferErrorCodeOK(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_NativeBuffer_Planes outPlanes;
    OH_NativeBuffer_Config config = {
        .width = 0x100,
        .height = 0x100,
        .format = 12,
        .usage = (1ULL << 0) | (1ULL << 1) |  (1ULL << 3),
    };
    OH_NativeBuffer* buffer = OH_NativeBuffer_Alloc(&config);
    int32_t res = OH_NativeBuffer_Unmap(buffer);
    napi_create_int32(env, res == NATIVE_ERROR_OK, &result);
    return result;
}

 napi_value OH_OHNativeErrorCodeBufferInvalid(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_NativeBuffer_ColorSpace colorSpace;
    int32_t res = OH_NativeBuffer_GetColorSpace(nullptr, &colorSpace);
    napi_create_int32(env, res == NATIVE_ERROR_INVALID_ARGUMENTS, &result);
    return result;
}

 napi_value OH_OHNativeErrorCodeBufferSupport(napi_env env, napi_callback_info info)
{
    napi_value result;
        OH_NativeBuffer_Config config = {
        .width = 0x100,
        .height = 0x100,
        .format = 12,
        .usage = (1ULL << 0) | (1ULL << 1) | (1ULL << 3) | (1ULL << 4) | (1ULL << 5) | (1ULL << 6)|(1ULL << 7),
    };
    OH_NativeBuffer* buffer = OH_NativeBuffer_Alloc(&config);
    int32_t size = 10;
    uint8_t *metadata = new uint8_t(size);
    int32_t res = OH_NativeBuffer_SetMetadataValue(buffer, OH_HDR_STATIC_METADATA, size, metadata);
    napi_create_int32(env, res == NATIVE_ERROR_UNSUPPORTED, &result);
    return result;
}

napi_value OH_Drawing_BufferHDIError(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_NativeBuffer_Planes outPlanes;
    OH_NativeBuffer_Config config = {
        .width = 0x100,
        .height = 0x100,
        .format = 12,
        .usage = (1ULL << 0) | (1ULL << 1) |  (1ULL << 3),
    };
    OH_NativeBuffer* buffer = OH_NativeBuffer_Alloc(&config);
    void *virAddr = nullptr;
    int32_t res = OH_NativeBuffer_MapPlanes(buffer, &virAddr, &outPlanes);
    napi_create_int32(env, res == NATIVE_ERROR_HDI_ERROR, &result);
    return result;
}

 napi_value OH_OHNativeErrorCodeImageSupport(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_NativeImage* pimg =  OH_ConsumerSurface_Create();
    int32_t res =  OH_ConsumerSurface_SetDefaultUsage(pimg, 800000000000);
    
    napi_create_int32(env, res == NATIVE_ERROR_OK, &result);
    return result;
}

 napi_value OH_OHNativeErrorCodeImageNoBuffer(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_NativeImage* image = OH_NativeImage_Create(0, 0);
    OHNativeWindowBuffer* nativeWindowBuffer = nullptr;
    int fenceFd = 0;
    int32_t  errorCode = OH_NativeImage_AcquireNativeWindowBuffer(image, &nativeWindowBuffer, &fenceFd);
    napi_create_int32(env,  errorCode == NATIVE_ERROR_NO_BUFFER, &result);
    return result;
}

 napi_value OH_OHNativeErrorCodeVSyncOk(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_NativeVSync* nativeVsync = OH_NativeVSync_Create("Test", 4);
    OH_NativeVSync_FrameCallback callback = [](long long timestamp, void *data){};
    void* data = nullptr;
    int32_t res = OH_NativeVSync_RequestFrame(nativeVsync, callback, data);
    int32_t res2 = OH_NativeVSync_DVSyncSwitch(nativeVsync, true);
    napi_create_int32(env, res == NATIVE_ERROR_OK, &result);
    return result;
}
 napi_value OH_OHNativeErrorCodeVSyncInvalid(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_NativeVSync* nativeVsync = nullptr;
    OH_NativeVSync_FrameCallback callback = [](long long timestamp, void* data){};
    void* data = nullptr;
    int32_t res = OH_NativeVSync_RequestFrame(nativeVsync, callback, data);
    napi_create_int32(env, res == NATIVE_ERROR_INVALID_ARGUMENTS, &result);
    return result;
}


 napi_value OH_OHNativeErrorCodeWindowOk(napi_env env, napi_callback_info info)
{
    napi_value result;
    uint32_t textureId = 1;
    uint32_t textureTarget = 2;
    OH_NativeImage* image = OH_NativeImage_Create(textureId, textureTarget);
    OHNativeWindow* winNative = OH_NativeImage_AcquireNativeWindow(image);
    int32_t size = 0;
    uint8_t *metadata = nullptr;
    int32_t res = OH_NativeWindow_GetMetadataValue(winNative, OH_HDR_METADATA_TYPE, &size, &metadata);
    napi_create_int32(env, res == NATIVE_ERROR_UNSUPPORTED, &result);
    return result;
}

 napi_value OH_OHNativeErrorCodeWindowConsumerDisConnected(napi_env env, napi_callback_info info)
{
    napi_value result;
    uint32_t textureId = 1;
    uint32_t textureTarget = 2;
    OH_NativeImage* image = OH_NativeImage_Create(textureId, textureTarget);
    OHNativeWindow* winNative = OH_NativeImage_AcquireNativeWindow(image);
    int32_t res = OH_NativeWindow_CleanCache(winNative);
    napi_create_int32(env, res == NATIVE_ERROR_CONSUMER_DISCONNECTED, &result);
    return result;
}

 napi_value OH_OHNativeErrorCodeWindowInvalid(napi_env env, napi_callback_info info)
{
    napi_value result;
    int32_t res = OH_NativeWindow_WriteToParcel(nullptr, nullptr);
    napi_create_int32(env, res == NATIVE_ERROR_INVALID_ARGUMENTS, &result);
    return result;
}

napi_value OH_Drawing_DrawingErrorCodeSuccess(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Drawing_RecordCmdUtils* recordCmdUtils = OH_Drawing_RecordCmdUtilsCreate();
    OH_Drawing_RecordCmd* recordCmd;
    OH_Drawing_ErrorCode errCode = OH_Drawing_RecordCmdUtilsFinishRecording(recordCmdUtils, &recordCmd);
    napi_create_int32(env, errCode == OH_DRAWING_ERROR_ALLOCATION_FAILED, &result);
    return result;
}
