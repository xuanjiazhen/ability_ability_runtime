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
#include <native_vsync/native_vsync.h>
#include <native_drawing/drawing_error_code.h>
#include <native_drawing/drawing_record_cmd.h>
#include <native_buffer/native_buffer.h>
#include <native_drawing/drawing_text_lineTypography.h>
#include <native_drawing/drawing_text_typography.h>
#include <native_drawing/drawing_font_collection.h>
#include <native_effect/effect_filter.h>

#include <native_drawing/drawing_brush.h>
#include <native_drawing/drawing_filter.h>
#include <native_drawing/drawing_shadow_layer.h>
#include <native_drawing/drawing_pixel_map.h>
#include <native_drawing/drawing_record_cmd.h>
#include <native_drawing/drawing_region.h>
#include <native_drawing/drawing_register_font.h>
#include <native_drawing/drawing_canvas.h>
#include <native_drawing/drawing_pixel_map.h>
#include <native_drawing/drawing_types.h>
#include <native_drawing/drawing_font.h>
#include <multimedia/image_framework/image/pixelmap_native.h>
#include <native_drawing/drawing_text_font_descriptor.h>
#include <multimedia/player_framework/native_avbuffer.h>
#include <GLES3/gl3.h>

#include <native_image/native_image.h>
#include <native_drawing/drawing_matrix.h>

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
    descData.push_back({"OHNativeErrorCodeWindowUNKNOW", nullptr, OH_NativeErrorCodeWindowUNKNOW,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHNativeErrorCodeWindowOk", nullptr, OH_OHNativeErrorCodeWindowOk,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHDrawingErrorCodeSuccess", nullptr, OH_Drawing_DrawingErrorCodeSuccess,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHDrawingLineTypographyGetLineBreakInvalue", nullptr,
        OH_Drawing_LineTypographyGetLineBreakInvalue, nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"EffectTileModeREPEATSuccess", nullptr, EffectTileModeREPEATSuccess,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"EffectTileModeMIRRORSuccess", nullptr, EffectTileModeMIRRORSuccess,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"EffectTileModeDECALSuccess", nullptr, EffectTileModeDECALSuccess,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"samplingOptionsCopySuccess", nullptr, OH_Drawing_SamplingOptionsCopySuccess,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"samplingOptionsCopyInvalue", nullptr, OH_Drawing_SamplingOptionsCopyInvalue,
        nullptr, nullptr, nullptr, napi_default, nullptr});
}

 napi_value OH_OHNativeErrorCodeOK(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_NativeImage* image = OH_NativeImage_Create(0, 0);
    int32_t errorCode = OH_NativeImage_SetDropBufferMode(image, true);
    int32_t res = OH_ConsumerSurface_SetDefaultSize(image, 100, 100);
    napi_create_int32(env,  errorCode == NATIVE_ERROR_OK, &result);
    return result;
}

 napi_value OH_OHNativeErrorCodeInvalid(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_NativeImage* image = nullptr;
    int32_t errorCode = OH_NativeImage_SetDropBufferMode(image, true);
    napi_create_int32(env, errorCode == NATIVE_ERROR_INVALID_ARGUMENTS, &result);
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
    uint8_t* metadata = new uint8_t(size);
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
        .usage = (1ULL << 0) | (1ULL << 1) | (1ULL << 3),
    };
    OH_NativeBuffer* buffer = OH_NativeBuffer_Alloc(&config);
    void *virAddr = nullptr;
    OH_NativeBuffer* buffer2 = nullptr;
    int32_t res0 = OH_NativeBuffer_GetSeqNum(buffer);
    int32_t res = OH_NativeBuffer_MapPlanes(buffer, &virAddr, &outPlanes);
    napi_create_int32(env, res == NATIVE_ERROR_HDI_ERROR, &result);
    return result;
}

napi_value OH_OHNativeErrorCodeImageSupport(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_NativeImage* pimg = OH_ConsumerSurface_Create();
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
    OH_NativeVSync_FrameCallback callback = [](long long timestamp, void *data) {};
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
    OH_NativeVSync_FrameCallback callback = [](long long timestamp, void* data) {};
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

napi_value OH_NativeErrorCodeWindowUNKNOW(napi_env env, napi_callback_info info)
{
    napi_value result;
    GLuint textureId = 0;
    OH_NativeImage *nativeImage = OH_NativeImage_Create(textureId, GL_TEXTURE_BINDING_2D);
    OH_NativeImage_AttachContext(nativeImage, textureId);
    OHNativeWindow *nativeWindow = OH_NativeImage_AcquireNativeWindow(nativeImage);
    
    OHNativeWindowBuffer *windowBuffer = nullptr;
    int fd;
    float matrix[16] = {0};
    OH_NativeBuffer *nativebuffer = nullptr;
    int32_t err0 = OH_NativeWindow_NativeWindowRequestBuffer(nativeWindow, &windowBuffer, &fd);
    napi_create_int32(env, err0 == NATIVE_ERROR_UNKNOWN, &result);
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

napi_value OH_Drawing_LineTypographyGetLineBreakInvalue(napi_env env, napi_callback_info info)
{
    napi_value result;
    int lineNumber = 3;
    OH_Drawing_TypographyStyle* typoStyle = OH_Drawing_CreateTypographyStyle();
    OH_Drawing_SetTypographyTextAlign(typoStyle, TEXT_ALIGN_CENTER);
    OH_Drawing_SetTypographyTextMaxLines(typoStyle, lineNumber);
    OH_Drawing_FontCollection* fc = OH_Drawing_CreateSharedFontCollection();
    OH_Drawing_TypographyCreate* handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
    OH_Drawing_LineTypography* lineTypography = OH_Drawing_CreateLineTypography(handler);
    size_t startIndex = 0;
    double width = 0;
    napi_create_int32(env, 1, &result);
    return result;
}

napi_value OH_Drawing_LineTypographyCreateLineInvalue(napi_env env, napi_callback_info info)
{
    napi_value result;
    int lineNumber = 3;
    OH_Drawing_TypographyStyle* typoStyle = OH_Drawing_CreateTypographyStyle();
    OH_Drawing_SetTypographyTextAlign(typoStyle, TEXT_ALIGN_CENTER);
    OH_Drawing_SetTypographyTextMaxLines(typoStyle, lineNumber);
    OH_Drawing_FontCollection* fc = OH_Drawing_CreateSharedFontCollection();
    OH_Drawing_TypographyCreate* handler = OH_Drawing_CreateTypographyHandler(typoStyle, fc);
    OH_Drawing_LineTypography* lineTypography = OH_Drawing_CreateLineTypography(handler);
    size_t startIndex = 0;
    double width = 3;
    size_t size = 0;
    napi_create_int32(env, size == 0, &result);
    return result;
}

napi_value EffectTileModeREPEATSuccess(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Filter* filter = nullptr;
    float radius = 10;
    uint32_t width = 100;
    uint32_t height = 100;
    OH_PixelmapNative* pixelmap = nullptr;

    uint8_t* data = new uint8_t[100 * 100 * 4];
    size_t dataLength = 100 * 100 * 4;
    for(int i = 0; i < dataLength; i++) {
        data[i] = 0xFF;
    }
    OH_Pixelmap_InitializationOptions* options = nullptr;
    OH_PixelmapInitializationOptions_Create(&options);
    OH_PixelmapInitializationOptions_SetWidth(options, width);
    OH_PixelmapInitializationOptions_SetHeight(options, height);
    Image_ErrorCode imgErrCode = OH_PixelmapInitializationOptions_SetPixelFormat(options,
        NATIVEBUFFER_PIXEL_FMT_BGRA_8888);
    OH_PixelmapNative *pixelMapNative = nullptr;
    Image_ErrorCode errCode0 = OH_PixelmapNative_CreatePixelmap(data, dataLength, options, &pixelMapNative);
    OH_Filter_CreateEffect(pixelMapNative, &filter);
    EffectErrorCode errCode = OH_Filter_BlurWithTileMode(filter, radius, REPEAT);
    OH_Filter_Blur(filter, radius);
    napi_create_int32(env,  errCode == EFFECT_SUCCESS, &result);
    return result;
}

napi_value EffectTileModeMIRRORSuccess(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Filter* filter = nullptr;
    float radius = 10;
    uint32_t width = 100;
    uint32_t height = 100;
    OH_PixelmapNative* pixelmap = nullptr;

    uint8_t* data = new uint8_t[100 * 100 * 4];
    size_t dataLength = 100 * 100 * 4;
    for(int i = 0; i < dataLength; i++) {
        data[i] = 0xFF;
    }
    OH_Pixelmap_InitializationOptions* options = nullptr;
    OH_PixelmapInitializationOptions_Create(&options);
    OH_PixelmapInitializationOptions_SetWidth(options, width);
    OH_PixelmapInitializationOptions_SetHeight(options, height);
    Image_ErrorCode imgErrCode = OH_PixelmapInitializationOptions_SetPixelFormat(options,
        NATIVEBUFFER_PIXEL_FMT_BGRA_8888);
    OH_PixelmapNative *pixelMapNative = nullptr;
    Image_ErrorCode errCode0 = OH_PixelmapNative_CreatePixelmap(data, dataLength, options, &pixelMapNative);
    OH_Filter_CreateEffect(pixelMapNative, &filter);
    EffectErrorCode errCode = OH_Filter_BlurWithTileMode(filter, radius, MIRROR);
    OH_Filter_Blur(filter, radius);
    napi_create_int32(env,  errCode == EFFECT_SUCCESS, &result);
    return result;
}

napi_value EffectTileModeDECALSuccess(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Filter* filter = nullptr;
    float radius = 10;
    uint32_t width = 100;
    uint32_t height = 100;
    OH_PixelmapNative* pixelmap = nullptr;

    uint8_t* data = new uint8_t[100 * 100 * 4];
    size_t dataLength = 100 * 100 * 4;
    for(int i = 0; i < dataLength; i++) {
        data[i] = 0xFF;
    }
    OH_Pixelmap_InitializationOptions* options = nullptr;
    OH_PixelmapInitializationOptions_Create(&options);
    OH_PixelmapInitializationOptions_SetWidth(options, width);
    OH_PixelmapInitializationOptions_SetHeight(options, height);
    Image_ErrorCode imgErrCode = OH_PixelmapInitializationOptions_SetPixelFormat(options,
        NATIVEBUFFER_PIXEL_FMT_BGRA_8888);
    OH_PixelmapNative *pixelMapNative = nullptr;
    Image_ErrorCode errCode0 = OH_PixelmapNative_CreatePixelmap(data, dataLength, options, &pixelMapNative);
    OH_Filter_CreateEffect(pixelMapNative, &filter);
    EffectErrorCode errCode = OH_Filter_BlurWithTileMode(filter, radius, DECAL);
    OH_Filter_Blur(filter, radius);
    napi_create_int32(env,  errCode == EFFECT_SUCCESS, &result);
    return result;
}

napi_value OH_Drawing_SamplingOptionsCopySuccess(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Drawing_SamplingOptions* samplingOptions = OH_Drawing_SamplingOptionsCreate(FILTER_MODE_LINEAR,
        MIPMAP_MODE_LINEAR);
    OH_Drawing_SamplingOptions* amp2 = OH_Drawing_SamplingOptionsCopy(samplingOptions);
    napi_create_int32(env,  amp2 != nullptr, &result);
    return result;
}

napi_value OH_Drawing_SamplingOptionsCopyInvalue(napi_env env, napi_callback_info info)
{
    napi_value result;
    OH_Drawing_SamplingOptions* amp2 = OH_Drawing_SamplingOptionsCopy(nullptr);
    napi_create_int32(env,  amp2 == nullptr, &result);
    return result;
}


