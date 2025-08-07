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
#include <LocationKit/oh_location.h>
#include <LocationKit/oh_location_type.h>
#include <js_native_api_types.h>
#include <unistd.h>

const int ADDITIONS_MAX_LEN = 512;
 typedef struct Location_Info {
     double latitude;
     double longitude;
     double altitude;
     double accuracy;
     double speed;
     double direction;
     int64_t timeForFix;
     int64_t timeSinceBoot;
     char additions[ADDITIONS_MAX_LEN];
     double altitudeAccuracy;
     double speedAccuracy;
     double directionAccuracy;
     int64_t uncertaintyOfTimeSinceBoot;
     Location_SourceType locationSourceType;
 } Location_Info;
namespace {
static const Location_RequestConfig *requestConfig = OH_Location_CreateRequestConfig();
Location_Info localInfo= {
  latitude: 0.0,
  longitude: 0.0,
  altitude: 0.0,
  accuracy: 0.0,
  speed: 0.0,
  direction: 0.0,
  timeForFix: 0,
  timeSinceBoot: 0,
  additions:  "",
  altitudeAccuracy: 0.0,
  speedAccuracy: 0.0,
  directionAccuracy: 0.0,
  uncertaintyOfTimeSinceBoot: 0,
  locationSourceType: LOCATION_SOURCE_TYPE_GNSS,
};
Location_BasicInfo baseInfo;
char additionalInfo = 'X';
}


static napi_value OHLocationInfoGetAdditionalInfo_test(napi_env env, napi_callback_info info)
{
    Location_Info *location;
    char additionalInfo[] = "";
    Location_ResultCode errorCode = OH_LocationInfo_GetAdditionalInfo(nullptr, nullptr, 0);
    napi_value result = nullptr;
    napi_create_int32(env, errorCode, &result);
    return result;
}

static napi_value OHLocationInfoGetAdditionalInfo(napi_env env, napi_callback_info info)
{
    Location_Info *location;
    char additionalInfo[] = "";
    Location_ResultCode errorCode = OH_LocationInfo_GetAdditionalInfo(nullptr, additionalInfo, 0);
    napi_value result = nullptr;
    napi_create_int32(env, errorCode, &result);
    return result;
}
static napi_value OH_Location_IsLocatingEnabled1(napi_env env, napi_callback_info info)
{
    bool flag = true;
    bool *enabled = &flag;
    Location_ResultCode startCode = OH_Location_IsLocatingEnabled(enabled);
    napi_value result = nullptr;
    napi_create_int32(env, startCode, &result);
    return result;
}
static napi_value OH_Location_IsLocatingEnabled2(napi_env env, napi_callback_info info)
{
    Location_ResultCode startCode = OH_Location_IsLocatingEnabled(nullptr);
    napi_value result = nullptr;
    napi_create_int32(env, startCode, &result);
    return result;
}

static napi_value OH_Location_StartLocating1(napi_env env, napi_callback_info info)
{
    Location_RequestConfig *config = OH_Location_CreateRequestConfig();
    Location_ResultCode startCode1 = OH_Location_StartLocating(config);
    napi_value result = nullptr;
    napi_create_int32(env, startCode1, &result);
    return result;
}
static napi_value OH_Location_StartLocating2(napi_env env, napi_callback_info info)
{
    Location_ResultCode startCode = OH_Location_StartLocating(nullptr);
    napi_value result = nullptr;
    napi_create_int32(env, startCode, &result);
    return result;
}
static napi_value OH_Location_StopLocating2(napi_env env, napi_callback_info info)
{
    Location_ResultCode stopCode = OH_Location_StopLocating(nullptr);
    napi_value result = nullptr;
    napi_create_int32(env, stopCode, &result);
    return result;
}
static napi_value OHLocationInfoGetAdditionalInfo0100(napi_env env, napi_callback_info info)
{
    Location_Info *location;
    char additionalInfo[] = "";
    Location_ResultCode errorCode = OH_LocationInfo_GetAdditionalInfo(location, additionalInfo, 0);
    napi_value result = nullptr;
    napi_create_int32(env, errorCode, &result);
    return result;
}
static napi_value OH_LocationRequestConfig_SetUseScene0100(napi_env env, napi_callback_info info)
{
    try {
        Location_RequestConfig *requestConfig = OH_Location_CreateRequestConfig();
        OH_LocationRequestConfig_SetUseScene(requestConfig, LOCATION_USE_SCENE_SPORT);
        int32_t errorCode = 0;
        napi_value result = nullptr;
        napi_create_int32(env, errorCode, &result);
        return result;
    } catch (...) {
        int32_t errorCode = 1;
        napi_value result = nullptr;
        napi_create_int32(env, errorCode, &result);
        return result;
    }
}
static napi_value OH_LocationRequestConfig_SetUseScene0101(napi_env env, napi_callback_info info)
{
    try {
        Location_RequestConfig *requestConfig = OH_Location_CreateRequestConfig();
        OH_LocationRequestConfig_SetUseScene(requestConfig, LOCATION_USE_SCENE_TRANSPORT);
        int32_t errorCode = 0;
        napi_value result = nullptr;
        napi_create_int32(env, errorCode, &result);
        return result;
    } catch (...) {
        int32_t errorCode = 1;
        napi_value result = nullptr;
        napi_create_int32(env, errorCode, &result);
        return result;
    }
}
static napi_value OH_LocationRequestConfig_SetUseScene0102(napi_env env, napi_callback_info info)
{
    try {
        Location_RequestConfig *requestConfig = OH_Location_CreateRequestConfig();
        OH_LocationRequestConfig_SetUseScene(requestConfig, LOCATION_USE_SCENE_DAILY_LIFE_SERVICE);
        int32_t errorCode = 0;
        napi_value result = nullptr;
        napi_create_int32(env, errorCode, &result);
        return result;
    } catch (...) {
        int32_t errorCode = 1;
        napi_value result = nullptr;
        napi_create_int32(env, errorCode, &result);
        return result;
    }
}
static napi_value OH_LocationRequestConfig_SetPowerConsumptionScene0100(napi_env env, napi_callback_info info)
{
    try {
        Location_RequestConfig *requestConfig = OH_Location_CreateRequestConfig();
        OH_LocationRequestConfig_SetPowerConsumptionScene(requestConfig, LOCATION_LOW_POWER_CONSUMPTION);
        int32_t errorCode = 0;
        napi_value result = nullptr;
        napi_create_int32(env, errorCode, &result);
        return result;
    } catch (...) {
        int32_t errorCode = 1;
        napi_value result = nullptr;
        napi_create_int32(env, errorCode, &result);
        return result;
    }
}
static napi_value OH_LocationRequestConfig_SetPowerConsumptionScene0101(napi_env env, napi_callback_info info)
{
    try {
        Location_RequestConfig *requestConfig = OH_Location_CreateRequestConfig();
        OH_LocationRequestConfig_SetPowerConsumptionScene(requestConfig, LOCATION_NO_POWER_CONSUMPTION);
        int32_t errorCode = 0;
        napi_value result = nullptr;
        napi_create_int32(env, errorCode, &result);
        return result;
    } catch (...) {
        int32_t errorCode = 1;
        napi_value result = nullptr;
        napi_create_int32(env, errorCode, &result);
        return result;
    }
}

 static napi_value Location_BasicInfo0100(napi_env env, napi_callback_info info)
 {
    localInfo.locationSourceType = LOCATION_SOURCE_TYPE_GNSS;
    localInfo.locationSourceType = LOCATION_SOURCE_TYPE_GNSS;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.locationSourceType == LOCATION_SOURCE_TYPE_GNSS) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0101(napi_env env, napi_callback_info info)
{
    localInfo.locationSourceType = LOCATION_SOURCE_TYPE_NETWORK;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.locationSourceType == LOCATION_SOURCE_TYPE_NETWORK) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0102(napi_env env, napi_callback_info info)
 {
    localInfo.locationSourceType = LOCATION_SOURCE_TYPE_INDOOR;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.locationSourceType == LOCATION_SOURCE_TYPE_INDOOR) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0103(napi_env env, napi_callback_info info)
 {
    localInfo.locationSourceType = LOCATION_SOURCE_TYPE_RTK;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.locationSourceType == LOCATION_SOURCE_TYPE_RTK) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0104(napi_env env, napi_callback_info info)
 {
    localInfo.latitude = 90;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.latitude == 90) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0105(napi_env env, napi_callback_info info)
{
    localInfo.longitude = 90;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.longitude == 90) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0106(napi_env env, napi_callback_info info)
{
    localInfo.altitude = 100;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.altitude == 100) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0107(napi_env env, napi_callback_info info)
{
    localInfo.accuracy = 100;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.accuracy == 100) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0108(napi_env env, napi_callback_info info)
{
    localInfo.speed = 100;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.speed == 100) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0109(napi_env env, napi_callback_info info)
{
    localInfo.direction = 100;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.direction == 100) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0110(napi_env env, napi_callback_info info)
{
    localInfo.timeForFix = 123;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.timeForFix == 123) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0111(napi_env env, napi_callback_info info)
{
    localInfo.timeSinceBoot = 123;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.timeSinceBoot == 123) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}

 static napi_value Location_BasicInfo0112(napi_env env, napi_callback_info info)
{
    localInfo.altitudeAccuracy = 1.0;
    localInfo.speedAccuracy = 1.0;
    localInfo.directionAccuracy = 1.0;
    sleep(2);
    Location_Info qaz = localInfo;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&qaz);
    if (locationInfo.altitudeAccuracy == 1.0) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0113(napi_env env, napi_callback_info info)
{
    localInfo.speedAccuracy = 90;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.speedAccuracy == 90) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0114(napi_env env, napi_callback_info info)
{
    localInfo.directionAccuracy = 90;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.directionAccuracy == 90) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0115(napi_env env, napi_callback_info info)
{
    localInfo.uncertaintyOfTimeSinceBoot = 90;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.uncertaintyOfTimeSinceBoot == 90) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
 static napi_value Location_BasicInfo0116(napi_env env, napi_callback_info info)
{
    localInfo.locationSourceType = LOCATION_SOURCE_TYPE_RTK;
    Location_BasicInfo locationInfo = OH_LocationInfo_GetBasicInfo(&localInfo);
    if (locationInfo.locationSourceType == LOCATION_SOURCE_TYPE_RTK) {
        napi_value result = nullptr;
        napi_create_int32(env, 1, &result);
        return result;
    } else {
        napi_value result = nullptr;
        napi_create_int32(env, 0, &result);
        return result;
    }
}
EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        {"OHLocationInfoGetAdditionalInfo", nullptr, OHLocationInfoGetAdditionalInfo, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"OH_Location_IsLocatingEnabled1", nullptr, OH_Location_IsLocatingEnabled1, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"OH_Location_IsLocatingEnabled2", nullptr, OH_Location_IsLocatingEnabled2, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"OH_Location_StartLocating1", nullptr, OH_Location_StartLocating1, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"OH_Location_StartLocating2", nullptr, OH_Location_StartLocating2, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"OH_Location_StopLocating2", nullptr, OH_Location_StopLocating2, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"OHLocationInfoGetAdditionalInfo0100", nullptr, OHLocationInfoGetAdditionalInfo0100, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"OH_LocationRequestConfig_SetUseScene0100", nullptr, OH_LocationRequestConfig_SetUseScene0100, nullptr,
         nullptr, nullptr, napi_default, nullptr},
        {"OH_LocationRequestConfig_SetUseScene0101", nullptr, OH_LocationRequestConfig_SetUseScene0101, nullptr,
         nullptr, nullptr, napi_default, nullptr},
        {"OH_LocationRequestConfig_SetUseScene0102", nullptr, OH_LocationRequestConfig_SetUseScene0102, nullptr,
         nullptr, nullptr, napi_default, nullptr},
        {"OH_LocationRequestConfig_SetPowerConsumptionScene0100", nullptr,
         OH_LocationRequestConfig_SetPowerConsumptionScene0100, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"OH_LocationRequestConfig_SetPowerConsumptionScene0101", nullptr,
         OH_LocationRequestConfig_SetPowerConsumptionScene0101, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0100", nullptr, Location_BasicInfo0100, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0101", nullptr, Location_BasicInfo0101, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0102", nullptr, Location_BasicInfo0102, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0103", nullptr, Location_BasicInfo0103, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0104", nullptr, Location_BasicInfo0104, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0105", nullptr, Location_BasicInfo0105, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0106", nullptr, Location_BasicInfo0106, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0107", nullptr, Location_BasicInfo0107, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0108", nullptr, Location_BasicInfo0108, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0109", nullptr, Location_BasicInfo0109, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0110", nullptr, Location_BasicInfo0110, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0111", nullptr, Location_BasicInfo0111, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0112", nullptr, Location_BasicInfo0112, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0113", nullptr, Location_BasicInfo0113, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0114", nullptr, Location_BasicInfo0114, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0115", nullptr, Location_BasicInfo0115, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Location_BasicInfo0116", nullptr, Location_BasicInfo0116, nullptr, nullptr, nullptr, napi_default, nullptr}
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
    .nm_modname = "locationcapi20",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterModule(void) { napi_module_register(&demoModule); }
