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

#include <map>
#include "WebErrorCodeList.h"
#include "napi/native_api.h"
#include "web/arkweb_net_error_list.h"
#include "web/arkweb_scheme_handler.h"
#include <hilog/log.h>

static const int ERROR_NAME_SIZE = 200;

static char* GetArgsParamater(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    size_t netErrorSize = 0;
    char netErrorName [200] = "";
    napi_get_value_string_utf8(env, args[0], netErrorName, 200, &netErrorSize);
    return netErrorName;
}

static ArkWeb_NetError GetNetErrorByName(const char* name)
{
    std::map<const char *, ArkWeb_NetError> netErrorMap = {
        {"ARKWEB_ERR_DNS_NO_MATCHING_SUPPORTED_ALPN", ArkWeb_NetError::ARKWEB_ERR_DNS_NO_MATCHING_SUPPORTED_ALPN},
        {"ARKWEB_ERR_IO_PENDING", ArkWeb_NetError::ARKWEB_ERR_IO_PENDING},
        {"ARKWEB_ERR_FAILED", ArkWeb_NetError::ARKWEB_ERR_FAILED},
        {"ARKWEB_ERR_ABORTED", ArkWeb_NetError::ARKWEB_ERR_ABORTED},
        {"ARKWEB_ERR_INVALID_HANDLE", ArkWeb_NetError::ARKWEB_ERR_INVALID_HANDLE},
        {"ARKWEB_ERR_TIMED_OUT", ArkWeb_NetError::ARKWEB_ERR_TIMED_OUT},
        {"ARKWEB_ERR_FILE_TOO_LARGE", ArkWeb_NetError::ARKWEB_ERR_FILE_TOO_LARGE},
        {"ARKWEB_ERR_UNEXPECTED", ArkWeb_NetError::ARKWEB_ERR_UNEXPECTED},
        {"ARKWEB_ERR_ACCESS_DENIED", ArkWeb_NetError::ARKWEB_ERR_ACCESS_DENIED},
        {"ARKWEB_ERR_NOT_IMPLEMENTED", ArkWeb_NetError::ARKWEB_ERR_NOT_IMPLEMENTED},
        {"ARKWEB_ERR_INSUFFICIENT_RESOURCES", ArkWeb_NetError::ARKWEB_ERR_INSUFFICIENT_RESOURCES},
        {"ARKWEB_ERR_OUT_OF_MEMORY", ArkWeb_NetError::ARKWEB_ERR_OUT_OF_MEMORY},
        {"ARKWEB_ERR_UPLOAD_FILE_CHANGED", ArkWeb_NetError::ARKWEB_ERR_UPLOAD_FILE_CHANGED},
        {"ARKWEB_ERR_SOCKET_NOT_CONNECTED", ArkWeb_NetError::ARKWEB_ERR_SOCKET_NOT_CONNECTED},
        {"ARKWEB_ERR_FILE_EXISTS", ArkWeb_NetError::ARKWEB_ERR_FILE_EXISTS},
        {"ARKWEB_ERR_FILE_PATH_TOO_LONG", ArkWeb_NetError::ARKWEB_ERR_FILE_PATH_TOO_LONG},
        {"ARKWEB_ERR_FILE_NO_SPACE", ArkWeb_NetError::ARKWEB_ERR_FILE_NO_SPACE},
        {"ARKWEB_ERR_FILE_VIRUS_INFECTED", ArkWeb_NetError::ARKWEB_ERR_FILE_VIRUS_INFECTED},
        {"ARKWEB_ERR_BLOCKED_BY_CLIENT", ArkWeb_NetError::ARKWEB_ERR_BLOCKED_BY_CLIENT},
        {"ARKWEB_ERR_NETWORK_CHANGED", ArkWeb_NetError::ARKWEB_ERR_NETWORK_CHANGED},
        {"ARKWEB_ERR_BLOCKED_BY_ADMINISTRATOR", ArkWeb_NetError::ARKWEB_ERR_BLOCKED_BY_ADMINISTRATOR},
        {"ARKWEB_ERR_SOCKET_CONNECTED", ArkWeb_NetError::ARKWEB_ERR_SOCKET_CONNECTED},
        {"ARKWEB_ERR_UPLOAD_STREAM_REWIND_NOT_SUPPORTED",
         ArkWeb_NetError::ARKWEB_ERR_UPLOAD_STREAM_REWIND_NOT_SUPPORTED},
    };

    auto it = netErrorMap.find(name);
    if (it != netErrorMap.end()) {
        return it->second;
    }
    return ARKWEB_NET_OK;
}

napi_value WebNetErrorDidFailWithError(napi_env env, napi_callback_info info)
{
    OH_LOG_DEBUG(LOG_APP, "WebNetErrorDidFailWithError start..");
    char* netErrorName = GetArgsParamater(env, info);
    ArkWeb_NetError errorCode = GetNetErrorByName(netErrorName);

    ArkWeb_ResourceHandler* resourceHandler = nullptr;
    napi_value result;
    if (OH_ArkWebResourceHandler_DidFailWithError(resourceHandler, errorCode) == ARKWEB_ERROR_UNKNOWN) {
        napi_create_int32(env, 0, &result);
    }

    OH_LOG_DEBUG(LOG_APP, "WebNetErrorDidFailWithError end..");
    return result;
}

napi_value WebArkWebResponseGetError(napi_env env, napi_callback_info info)
{
    OH_LOG_DEBUG(LOG_APP, "WebArkWebResponseGetError start..");

    ArkWeb_Response* response = nullptr;
    OH_ArkWeb_DestroyResponse(response);
    const char* url = "www.example.com";
    int32_t ret = OH_ArkWebResponse_SetUrl(response, url);

    char* netErrorName = GetArgsParamater(env, info);
    ArkWeb_NetError errorCode = GetNetErrorByName(netErrorName); 
    ret = OH_ArkWebResponse_SetError(response, errorCode);
    ArkWeb_NetError errorCodeRes = OH_ArkWebResponse_GetError(response);
    
    napi_value result;
    if (errorCodeRes == ARKWEB_ERR_FAILED) {
        napi_create_int32(env, 0, &result);
    }

    OH_LOG_DEBUG(LOG_APP, "WebArkWebResponseGetError end..");
    return result;
}
