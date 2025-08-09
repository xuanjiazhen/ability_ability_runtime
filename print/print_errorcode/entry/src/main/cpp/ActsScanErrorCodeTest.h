//
// Created on 2025/8/9.
//
// Node APIs are not fully supported. To solve the compilation error of the interface cannot be found,
// please include "napi/native_api.h".

#ifndef PRINT_ERRORCODE_ACTSSCANERRORCODETEST_H
#define PRINT_ERRORCODE_ACTSSCANERRORCODETEST_H
#include "napi/native_api.h"
#include <js_native_api_types.h>
#include <napi/native_api.h>
#include <vector>
void addScanErrorCodeTest(std::vector<napi_property_descriptor>& descData);
napi_value OHScanNoError(napi_env env, napi_callback_info info);
napi_value OHScanInvalidParameter(napi_env env, napi_callback_info info);
napi_value OHScanUnSupported(napi_env env, napi_callback_info info);
napi_value OHPrintINVALID(napi_env env, napi_callback_info info);
#endif //PRINT_ERRORCODE_ACTSSCANERRORCODETEST_H
