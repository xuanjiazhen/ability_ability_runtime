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

#include "ActsScanErrorCodeTest.h"
#include "napi/native_api.h"
#include <js_native_api_types.h>
#include <BasicServicesKit/ohscan.h>
#define SCANNER_ID "12"
#define TEST_PRINTER "testPrinter"
void addScanErrorCodeTest(std::vector<napi_property_descriptor>& descData)
{
    descData.push_back({"OHScanNoError", nullptr, OHScanNoError,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHScanInvalidParameter", nullptr, OHScanInvalidParameter,
            nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHScanUnSupported", nullptr, OHScanUnSupported,
             nullptr, nullptr, nullptr, napi_default, nullptr});
    descData.push_back({"OHPrintINVALID", nullptr, OHPrintINVALID,
            nullptr, nullptr, nullptr, napi_default, nullptr});
}

napi_value OHScanNoError(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    int32_t errorCode = OH_Scan_Init();
    napi_create_int32(env, static_cast<int32_t>(errorCode), &result);
    return result;
}

napi_value OHScanInvalidParameter(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    int32_t errorCode = OH_Scan_CloseScanner(nullptr);
    napi_create_int32(env, static_cast<int32_t>(errorCode), &result);
    return result;
}

napi_value OHScanUnSupported(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    int32_t errorCode = OH_Scan_Init();
    errorCode = OH_Scan_OpenScanner(SCANNER_ID);
    napi_create_int32(env, static_cast<int32_t>(errorCode), &result);
    return result;
}

napi_value OHPrintINVALID(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    int32_t errorCode = OH_Scan_StartScan(SCANNER_ID, false);
    napi_create_int32(env, static_cast<int32_t>(errorCode), &result);
    return result;
}