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
#include "napi/native_api.h"
#include <cstdint>
#include <hilog/log.h>
#include <multimedia/player_framework/native_avcodec_base.h>
#include <native_image/native_image.h>
#include <native_window/external_window.h>
#include <native_window/graphic_error_code.h>
#include <native_buffer/native_buffer.h>
#include "ace/xcomponent/native_interface_xcomponent.h"
#include "ExternalWindowTest.h"
#include "nativewindow.h"
#define SUCCESS 0
#define CONSTANT_100000 100000
#define CONSTANT_60 60
void addExternalWindowTest(std::vector<napi_property_descriptor>& descData)
{
    descData.push_back({"testOHScalingModeToWindow", nullptr, testOHScalingModeToWindow,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testOHScalingModeCrop", nullptr, testOHScalingModeCrop,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testOHScalingModeNoCrop", nullptr, testOHScalingModeNoCrop,
             nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testOhMetakeyRedPrimaryY", nullptr, testOhMetakeyRedPrimaryY,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testOhMetakeyGreenPrimaryX", nullptr, testOhMetakeyGreenPrimaryX,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testOhMetakeyGreenPrimaryY", nullptr, testOhMetakeyGreenPrimaryY,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testOhMetakeyBluePrimaryX", nullptr, testOhMetakeyBluePrimaryX,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testOhMetakeyBluePrimaryY", nullptr, testOhMetakeyBluePrimaryY,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testOhMetakeyWhitePrimaryX", nullptr, testOhMetakeyWhitePrimaryX,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testOhMetakeyWhitePrimaryY", nullptr, testOhMetakeyWhitePrimaryY,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testOhMetakeyMaxLuminance", nullptr, testOhMetakeyMaxLuminance,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testOhMetakeyMinLuminance", nullptr, testOhMetakeyMinLuminance,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testOhMetakeyMaxContentLightLevel", nullptr, testOhMetakeyMaxContentLightLevel,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testOhMetakeyMaxFrameAverageLightLevel", nullptr, testOhMetakeyMaxFrameAverageLightLevel,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testOhMetakeyHdrVivid", nullptr, testOhMetakeyHdrVivid,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testNativeWindowWTParcelSuccess", nullptr, testNativeWindowWTParcelSuccess,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testNativeWindowWTParcelInvalidArguments", nullptr, testNativeWindowWTParcelInvalidArguments,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testNativeWindowRFParcelSuccess", nullptr, testNativeWindowRFParcelSuccess,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testNativeWindowRFParcelInvalidArguments", nullptr, testNativeWindowRFParcelInvalidArguments,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testNativeWindowWindowHandleOptSetTimeOut", nullptr, testNativeWindowWindowHandleOptSetTimeOut,
        nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"testNativeWindowWindowHandleOptGetTimeOut", nullptr, testNativeWindowWindowHandleOptGetTimeOut,
        nullptr, nullptr, nullptr, napi_default, nullptr});
}
napi_value testOHScalingModeToWindow(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    uint32_t sequence = 1000;
    int32_t res = OH_NativeWindow_NativeWindowSetScalingMode(nullptr, sequence, OH_SCALING_MODE_SCALE_TO_WINDOW);
    delete initNative;
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testOHScalingModeCrop(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    OHNativeWindow *nativeWindow = nullptr;
    uint32_t sequence = 1000;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowSetScalingMode(nullptr, sequence, OH_SCALING_MODE_SCALE_CROP);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testOHScalingModeNoCrop(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    OHNativeWindow *nativeWindow = nullptr;
    uint32_t sequence = 1000;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowSetScalingMode(nullptr, sequence, OH_SCALING_MODE_NO_SCALE_CROP);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testOhMetakeyRedPrimaryY(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    uint32_t sequence = 1000;
    const OHHDRMetaData metaData[] = {{OH_METAKEY_RED_PRIMARY_Y, 0}};
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowSetMetaData(nullptr, sequence, sizeof(metaData), metaData);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testOhMetakeyGreenPrimaryX(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    uint32_t sequence = 1000;
    const OHHDRMetaData metaData[] = {{OH_METAKEY_GREEN_PRIMARY_X, 0}};
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowSetMetaData(nullptr, sequence, sizeof(metaData), metaData);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testOhMetakeyGreenPrimaryY(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    uint32_t sequence = 1000;
    const OHHDRMetaData metaData[] = {{OH_METAKEY_GREEN_PRIMARY_Y, 0}};
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowSetMetaData(nullptr, sequence, sizeof(metaData), metaData);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testOhMetakeyBluePrimaryX(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    uint32_t sequence = 1000;
    const OHHDRMetaData metaData[] = {{OH_METAKEY_BLUE_PRIMARY_X, 0}};
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowSetMetaData(nullptr, sequence, sizeof(metaData), metaData);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testOhMetakeyBluePrimaryY(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    uint32_t sequence = 1000;
    const OHHDRMetaData metaData[] = {{OH_METAKEY_BLUE_PRIMARY_Y, 0}};
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowSetMetaData(nullptr, sequence, sizeof(metaData), metaData);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testOhMetakeyWhitePrimaryX(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    uint32_t sequence = 1000;
    const OHHDRMetaData metaData[] = {{OH_METAKEY_WHITE_PRIMARY_X, 0}};
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowSetMetaData(nullptr, sequence, sizeof(metaData), metaData);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testOhMetakeyWhitePrimaryY(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    uint32_t sequence = 1000;
    const OHHDRMetaData metaData[] = {{OH_METAKEY_WHITE_PRIMARY_Y, 0}};
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowSetMetaData(nullptr, sequence, sizeof(metaData), metaData);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testOhMetakeyMaxLuminance(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    uint32_t sequence = 1000;
    const OHHDRMetaData metaData[] = {{OH_METAKEY_MAX_LUMINANCE, 0}};
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowSetMetaData(nullptr, sequence, sizeof(metaData), metaData);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testOhMetakeyMinLuminance(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    uint32_t sequence = 1000;
    const OHHDRMetaData metaData[] = {{OH_METAKEY_MIN_LUMINANCE, 0}};
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowSetMetaData(nullptr, sequence, sizeof(metaData), metaData);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testOhMetakeyMaxContentLightLevel(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    uint32_t sequence = 1000;
    const OHHDRMetaData metaData[] = {{OH_METAKEY_MAX_CONTENT_LIGHT_LEVEL, 0}};
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowSetMetaData(nullptr, sequence, sizeof(metaData), metaData);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testOhMetakeyMaxFrameAverageLightLevel(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    uint32_t sequence = 1000;
    const OHHDRMetaData metaData[] = {{OH_METAKEY_MAX_FRAME_AVERAGE_LIGHT_LEVEL, 0}};
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowSetMetaData(nullptr, sequence, sizeof(metaData), metaData);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testOhMetakeyHdrVivid(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow *initNative = new InitNativeWindow();
    uint32_t sequence = 1000;
    const OHHDRMetaData metaData[] = {{OH_METAKEY_HDR_VIVID, 0}};
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowSetMetaData(nullptr, sequence, sizeof(metaData), metaData);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testNativeWindowWTParcelSuccess(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow* initNative = new InitNativeWindow();
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    OHIPCParcel *parcel = OH_IPCParcel_Create();
    int32_t res = OH_NativeWindow_WriteToParcel(nativeWindow, parcel);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testNativeWindowWTParcelInvalidArguments(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow* initNative = new InitNativeWindow();
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_WriteToParcel(nativeWindow, nullptr);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testNativeWindowRFParcelSuccess(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow* initNative = new InitNativeWindow();
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    OHIPCParcel *parcel = OH_IPCParcel_Create();
    OH_NativeWindow_WriteToParcel(nativeWindow, parcel);
    int32_t res = OH_NativeWindow_ReadFromParcel(parcel, &nativeWindow);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testNativeWindowRFParcelInvalidArguments(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    InitNativeWindow* initNative = new InitNativeWindow();
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_ReadFromParcel(nullptr, nullptr);
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testNativeWindowWindowHandleOptSetTimeOut(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    int32_t flag;
    int32_t timeout = 5000;
    InitNativeWindow* initNative = new InitNativeWindow();
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, SET_TIMEOUT, &timeout);
    delete initNative;
    napi_create_int32(env, res, &result);
    return result;
}

napi_value testNativeWindowWindowHandleOptGetTimeOut(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    int32_t timeout = 5000;
    InitNativeWindow* initNative = new InitNativeWindow();
    OHNativeWindow *nativeWindow = nullptr;
    nativeWindow = initNative->returnNativeWindow();
    int32_t res = OH_NativeWindow_NativeWindowHandleOpt(nativeWindow, GET_TIMEOUT, &timeout);
    delete initNative;
    napi_create_int32(env, res, &result);
    return result;
}