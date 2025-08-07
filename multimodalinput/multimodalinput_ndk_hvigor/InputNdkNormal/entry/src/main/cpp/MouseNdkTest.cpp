/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include "multimodalinput/oh_input_manager.h"
#include "multimodalinput/oh_key_code.h"
#include "hilog/log.h"
#include <bits/alltypes.h>
#include <thread>
#include <iostream>

#undef LOG_TAG
#define LOG_TAG "MMI"

const int GLOBAL_RESMGR = 0xFF00;
const char *TAG = "[SensorCapiSample]";

static napi_value GetPointerLocation1(napi_env env, napi_callback_info info)
{
    napi_value result;

    int32_t displayId;
    double displayX;
    double displayY;
    Input_Result retResult = OH_Input_GetPointerLocation(&displayId, &displayX, &displayY);
    OH_LOG_INFO(LOG_APP, "SUB_MMI_Input_Api_GetPointerLocation_0100 retResult:%{public}d", retResult);
    napi_create_int32(env, (retResult == INPUT_SUCCESS || 
                        retResult == INPUT_DEVICE_NOT_SUPPORTED || 
                        retResult INPUT_SERVICE_EXCEPTION) ?  : 0, &result);
    return result;
}

static napi_value GetPointerLocation2(napi_env env, napi_callback_info info)
{
    napi_value result;

    double displayX;
    double displayY;
    Input_Result retResult = OH_Input_GetPointerLocation(nullptr, &displayX, &displayY);
    OH_LOG_INFO(LOG_APP, "SUB_MMI_Input_Api_GetPointerLocation_0200 retResult:%{public}d", retResult);
    napi_create_int32(env, (retResult == INPUT_PARAMETER_ERROR) ? 1 : 0, &result);
    return result;
}

static napi_value GetPointerLocation3(napi_env env, napi_callback_info info)
{
    napi_value result;

    int32_t displayId;
    double displayY;
    Input_Result retResult = OH_Input_GetPointerLocation(&displayId, nullptr, &displayY);
    OH_LOG_INFO(LOG_APP, "SUB_MMI_Input_Api_GetPointerLocation_0300 retResult:%{public}d", retResult);
    napi_create_int32(env, (retResult == INPUT_PARAMETER_ERROR) ? 1 : 0, &result);
    return result;
}

static napi_value GetPointerLocation4(napi_env env, napi_callback_info info)
{
    napi_value result;

    int32_t displayId;
    double displayX;
    Input_Result retResult = OH_Input_GetPointerLocation(&displayId, &displayX, nullptr);
    OH_LOG_INFO(LOG_APP, "SUB_MMI_Input_Api_GetPointerLocation_0400 retResult:%{public}d", retResult);
    napi_create_int32(env, (retResult == INPUT_PARAMETER_ERROR) ? 1 : 0, &result);
    return result;
}

static napi_value GetPointerLocation5(napi_env env, napi_callback_info info)
{
    napi_value result;

    Input_Result retResult = OH_Input_GetPointerLocation(nullptr, nullptr, nullptr);
    OH_LOG_INFO(LOG_APP, "SUB_MMI_Input_Api_GetPointerLocation_0500 retResult:%{public}d", retResult);
    napi_create_int32(env, (retResult == INPUT_PARAMETER_ERROR) ? 1 : 0, &result);
    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        {"GetPointerLocation1", nullptr, GetPointerLocation1, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetPointerLocation2", nullptr, GetPointerLocation2, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetPointerLocation3", nullptr, GetPointerLocation3, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetPointerLocation4", nullptr, GetPointerLocation4, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"GetPointerLocation5", nullptr, GetPointerLocation5, nullptr, nullptr, nullptr, napi_default, nullptr},
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "libmousendk",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void)
{
    napi_module_register(&demoModule);
}