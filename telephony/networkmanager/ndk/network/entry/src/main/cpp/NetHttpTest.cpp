
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

#include "native_common.h"
#include <network/netstack/net_http.h>
#include "napi/native_api.h"
#include <js_native_api_types.h>
#include <string>
using namespace std;
static const uint32_t MAX_BUFFER_SIZE = 128;
static const uint32_t MAX_ARG_COUNT = 8;
static const uint32_t CASE_INDEX_1 = 1;
static const uint32_t CASE_INDEX_2 = 2;
static const uint32_t CASE_INDEX_3 = 3;
static const uint32_t CASE_INDEX_4 = 4;
static const uint32_t CASE_INDEX_5 = 5;
static const uint32_t CASE_INDEX_6 = 6;
static const uint32_t CASE_INDEX_7 = 7;
static const uint32_t CASE_INDEX_8 = 8;

static std::string GetString(napi_env env, napi_callback_info info, uint32_t index)
{
    size_t argc = MAX_ARG_COUNT;
    napi_value args[MAX_ARG_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    char output[MAX_BUFFER_SIZE] = {0};
    size_t size = 0;
    napi_get_value_string_utf8(env, args[index], output, MAX_BUFFER_SIZE, &size);
    return std::string(output);
}

static int32_t GetInt(napi_env env, napi_callback_info info, uint32_t index)
{
    size_t argc = MAX_ARG_COUNT;
    napi_value args[MAX_ARG_COUNT] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int32_t ret = -1;
    napi_get_value_int32(env, args[index], &ret);
    return ret;
}

napi_value Http_CreateHeaders(napi_env env, napi_callback_info info)
{
    Http_Headers *header = OH_Http_CreateHeaders();
    int32_t returnValue = header == nullptr ? -1 : 0;
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

napi_value Http_SetHeaderValue(napi_env env, napi_callback_info info)
{
    int32_t index = GetInt(env, info, 0);
    int32_t returnValue = -1; 
    Http_Headers *header = OH_Http_CreateHeaders();
    if ( CASE_INDEX_1 == index ) {
        returnValue = OH_Http_SetHeaderValue(header, nullptr, nullptr);
    } else if ( CASE_INDEX_2 == index ) {
        returnValue = OH_Http_SetHeaderValue(header, "key", nullptr);
    } else if ( CASE_INDEX_3 == index ) {
        returnValue = OH_Http_SetHeaderValue(header, "key", "value");
    } else if ( CASE_INDEX_4 == index ) {
        returnValue = OH_Http_SetHeaderValue(nullptr, nullptr, nullptr);
    } else if ( CASE_INDEX_5 == index ) {
        returnValue = OH_Http_SetHeaderValue(nullptr, "key", nullptr);
    } else if ( CASE_INDEX_6 == index ) {
        returnValue = OH_Http_SetHeaderValue(nullptr, "key", "value");
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

napi_value Http_GetHeaderValue(napi_env env, napi_callback_info info)
{
    int32_t index = GetInt(env, info, 0);
    int32_t returnValue = 0; 
    Http_Headers *header = OH_Http_CreateHeaders();
    if ( CASE_INDEX_1 == index ) {
        Http_HeaderValue *headValue = OH_Http_GetHeaderValue(header, nullptr);
    } else if ( CASE_INDEX_2 == index ) {
        Http_HeaderValue *headValue = OH_Http_GetHeaderValue(header, "key");
    } else if ( CASE_INDEX_3 == index ) {
        Http_HeaderValue *headValue = OH_Http_GetHeaderValue(nullptr, nullptr);
    }else if ( CASE_INDEX_4 == index ) {
        Http_HeaderValue *headValue = OH_Http_GetHeaderValue(nullptr, "key");
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

napi_value Http_GetHeaderEntries(napi_env env, napi_callback_info info)
{
    int32_t index = GetInt(env, info, 0);
    int32_t returnValue = 0; 
    if ( CASE_INDEX_1 == index ) {
        Http_Headers *header = OH_Http_CreateHeaders();
        OH_Http_SetHeaderValue(header, "key", "val");
        returnValue = OH_Http_GetHeaderEntries(header) ? 0 : -1;
    } else {
        returnValue = OH_Http_GetHeaderEntries(nullptr) ? -1 : 0;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

napi_value Http_DestroyHeaderEntries(napi_env env, napi_callback_info info)
{
    int32_t index = GetInt(env, info, 0);
    int32_t returnValue = 0; 
    if ( CASE_INDEX_1 == index ) {
        Http_Headers *header = OH_Http_CreateHeaders();
        OH_Http_SetHeaderValue(header, "key", "val");
        Http_HeaderEntry *headerEntries = OH_Http_GetHeaderEntries(header);
        OH_Http_DestroyHeaderEntries(&headerEntries);
    } else {
        OH_Http_GetHeaderEntries(nullptr);
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

napi_value Http_CreateRequest(napi_env env, napi_callback_info info)
{
    std::string url = GetString(env, info, 0);
    int32_t returnValue = 0; 
    if( url.empty() ) {
        Http_Request *request = OH_Http_CreateRequest(nullptr);
    } else {
        Http_Request *request = OH_Http_CreateRequest(url.c_str());
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

static void testResponseCallback(struct Http_Response *response, uint32_t errCode)
{
    return;
}

napi_value Http_RequestRequest(napi_env env, napi_callback_info info)
{
    int32_t index = GetInt(env, info, 0);
    std::string url = GetString(env, info, 1);
    Http_Request *request = OH_Http_CreateRequest(url.c_str());
    request->options = nullptr;
    int returnValue = 0;
    Http_EventsHandler handler;
    if ( index == CASE_INDEX_1 ) {
        request->options = (Http_RequestOptions *)calloc(1, sizeof(Http_RequestOptions));
        Http_ResponseCallback callback = testResponseCallback;
        returnValue = OH_Http_Request(request, callback, handler);
    } else if (index == CASE_INDEX_2 ) {
        request->options = (Http_RequestOptions *)calloc(1, sizeof(Http_RequestOptions));
        returnValue = OH_Http_Request(request, nullptr, handler);
    } else if ( index == CASE_INDEX_3 ) {
        request->options = (Http_RequestOptions *)calloc(1, sizeof(Http_RequestOptions));
        returnValue = OH_Http_Request(nullptr, nullptr, handler);
    }
    if( request->options != nullptr ) {
        free(request->options);
        request->options = nullptr;
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

napi_value Http_Destroy(napi_env env, napi_callback_info info)
{
    int32_t index = GetInt(env, info, 0);
    int32_t returnValue = 0; 
    if (index == CASE_INDEX_1) {
        OH_Http_Destroy(nullptr);
    } else{
        Http_Request *request = OH_Http_CreateRequest("https://www.baidu.com");
        OH_Http_Destroy(&request);
    }

    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}


EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        {"Http_CreateHeaders", nullptr, Http_CreateHeaders, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Http_SetHeaderValue", nullptr, Http_SetHeaderValue, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Http_GetHeaderValue", nullptr, Http_GetHeaderValue, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Http_GetHeaderEntries", nullptr, Http_GetHeaderEntries, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Http_DestroyHeaderEntries", nullptr, Http_DestroyHeaderEntries, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"Http_CreateRequest", nullptr, Http_CreateRequest, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"Http_RequestRequest", nullptr, Http_RequestRequest, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Http_Destroy", nullptr, Http_Destroy, nullptr, nullptr, nullptr, napi_default, nullptr},
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
    .nm_modname = "nethttptest",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterModule(void) { napi_module_register(&demoModule); }
