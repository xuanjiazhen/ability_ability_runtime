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
#include <thread>
#include <chrono>
#include "native_common.h"
#include <network/netmanager/net_connection.h>
#include <network/netmanager/net_connection_type.h>
#include <network/netstack/net_ssl/net_ssl_c.h>
#include <network/netstack/net_http.h>
#include <network/netstack/net_http_type.h>
#include <network/netstack/net_websocket_type.h>
#include <network/netstack/net_websocket.h>

#include "napi/native_api.h"
#include <js_native_api_types.h>
#include <cstring>
#include <string>
#include <thread>
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
static const uint32_t CLOSE_STATUS_NORMAL = 1000;
static int32_t g_httpErrorCode = -1;
static int32_t g_respErrorCode = 0;
static int32_t g_sleepSecs = 1;
static int32_t g_maxHeaderLen = 8192;
static int32_t g_maxLoopNum = 256;
static int32_t g_maxDataLength = (4 * 1024 * 1024);

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

static uint32_t ReadResponseCode(uint32_t responseCode)
{
    return responseCode;
}

static void ResponseCallback(struct Http_Response *response, uint32_t errCode)
{
    if (response == nullptr) {
        return;
    }
    g_httpErrorCode = ReadResponseCode(errCode);
}

napi_value Http_RequestRequest(napi_env env, napi_callback_info info)
{
    int32_t index = GetInt(env, info, 0);
    std::string url = GetString(env, info, 1);
    Http_Request *request = OH_Http_CreateRequest(url.c_str());
    int32_t returnValue = 0;
    Http_EventsHandler handler;
    if (index == CASE_INDEX_1 || index == CASE_INDEX_2) {
        returnValue = OH_Http_Request(request, ResponseCallback, handler);
    } else if (index == CASE_INDEX_3) {
        returnValue = OH_Http_Request(request, ResponseCallback, handler);
        std::this_thread::sleep_for(std::chrono::seconds(g_sleepSecs));
        returnValue = g_httpErrorCode;
    } else if (index == CASE_INDEX_4) {
        returnValue = OH_Http_Request(request, ResponseCallback, handler);
        std::this_thread::sleep_for(std::chrono::seconds(g_sleepSecs));
        returnValue = g_httpErrorCode;
    } else if (index == CASE_INDEX_5) {
        Http_RequestOptions reqOptions;
        request->options = &reqOptions;
        reqOptions.method = NET_HTTP_METHOD_POST;
        returnValue = OH_Http_Request(request, ResponseCallback, handler);
        std::this_thread::sleep_for(std::chrono::seconds(g_sleepSecs));
        returnValue = g_httpErrorCode;
    } else {
        Http_RequestOptions reqOptions;
        returnValue = ReadResponseCode(index);
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

void HttpOnUploadProgress(uint64_t totalSize, uint64_t transferredSize)
{
    return;
}

void HttpOnDownloadProgress(uint64_t totalSize, uint64_t transferredSize)
{
    return;
}

void HttpOnEndCall()
{
    return;
}

void HttpOnCancel()
{
    return;
}

static void OnOpen(struct WebSocket *client, WebSocket_OpenResult openResult)
{
    return;
}

static void OnMessage(struct WebSocket *client, char *data, uint32_t length)
{
    return;
}

static void OnError(struct WebSocket *client, WebSocket_ErrorResult error)
{
    return;
}

static void OnClose(struct WebSocket *client, WebSocket_CloseResult closeResult)
{
    return;
}

static napi_value Web_Socket(napi_env env, napi_callback_info info)
{
    int32_t index = GetInt(env, info, 0);
    std::string url = GetString(env, info, 1);
    int32_t returnValue = 0;
    struct WebSocket *websocket = OH_WebSocketClient_Constructor(OnOpen, OnMessage, OnError, OnClose);
    if (index == CASE_INDEX_1) {
        WebSocket_CloseOption CloseOptions;
        CloseOptions.code = CLOSE_STATUS_NORMAL;
        CloseOptions.reason = "";
        returnValue = OH_WebSocketClient_Close(websocket, CloseOptions);
    } else if (index == CASE_INDEX_2) {
        returnValue = OH_WebSocketClient_Destroy(websocket);
    } else if (index == CASE_INDEX_3) {
        struct WebSocket_RequestOptions options;
        WebSocket_Header header;
        header.fieldName = strdup("0");
        header.fieldValue = strdup("0");
        options.headers = &header;
        WebSocket_Header* tail = &header;
        for (int32_t i = 0; i <= g_maxHeaderLen; i++) {
            WebSocket_Header* tmpHeader = new WebSocket_Header();
            tmpHeader->fieldName = strdup(std::to_string(i+1).c_str());
            tmpHeader->fieldValue = strdup(std::to_string(i+1).c_str());
            tail->next = tmpHeader;
            tail = tmpHeader;
        }
        tail->next = nullptr;
        returnValue = OH_WebSocketClient_Connect(websocket, url.c_str(), options);
    } else if (index == CASE_INDEX_4) {
        returnValue = OH_WebSocketClient_Connect(websocket, url.c_str(), websocket->requestOptions);
    } else if (index == CASE_INDEX_5) {
        WebSocket_Header header;
        header.fieldName = "Content-Type";
        header.fieldValue = "application/json";
        header.next = nullptr;
        OH_WebSocketClient_AddHeader(websocket, header);
        OH_WebSocketClient_Connect(websocket, url.c_str(), websocket->requestOptions);
        std::string data;
        data.resize(g_maxDataLength + 1, 'a');
        int sendLength = data.size();
        returnValue = OH_WebSocketClient_Send(websocket, const_cast<char *>(data.c_str()), sendLength);
    }
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        {"Http_RequestRequest", nullptr, Http_RequestRequest, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"Web_Socket", nullptr, Web_Socket, nullptr, nullptr, nullptr, napi_default, nullptr},
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
    .nm_modname = "netconnectionauthtest",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterModule(void) { napi_module_register(&demoModule); }
