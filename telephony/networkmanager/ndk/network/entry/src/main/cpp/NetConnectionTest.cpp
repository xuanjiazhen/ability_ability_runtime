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
#include <network/netmanager/net_connection.h>
#include "napi/native_api.h"
#include <js_native_api_types.h>
#include <string>
using namespace std;

static const uint32_t MAX_NAME_LENGTH = 1024;
static const uint32_t DEFAULT_HOST_LEN = 20;
static const uint32_t DEFAULT_SERV_LEN = 30;
static const uint32_t MAX_BUFFER_SIZE = 128;
static const uint32_t MAX_ARG_COUNT = 8;
static const uint32_t TIMEOUT_3000 = 3000;

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

napi_value NetConn_HasDefaultNet(napi_env env, napi_callback_info info)
{
    int32_t hasDefaultNet;
    int32_t returnValue = OH_NetConn_HasDefaultNet(&hasDefaultNet);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

napi_value NetConn_GetDefaultNet(napi_env env, napi_callback_info info)
{
    NetConn_NetHandle netHandle;
    int32_t returnValue = OH_NetConn_GetDefaultNet(&netHandle);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

napi_value NetConn_IsDefaultNetMetered(napi_env env, napi_callback_info info)
{
    int32_t isMetered;
    int32_t returnValue = OH_NetConn_IsDefaultNetMetered(&isMetered);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

napi_value NetConn_GetConnectionProperties(napi_env env, napi_callback_info info)
{
    NetConn_ConnectionProperties netProp;
    NetConn_NetHandle netHandle;
    OH_NetConn_GetDefaultNet(&netHandle);
    int32_t returnValue = OH_NetConn_GetConnectionProperties(&netHandle, &netProp);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

napi_value NetConn_GetNetCapabilities(napi_env env, napi_callback_info info)
{
    NetConn_NetHandle netHandle;
    OH_NetConn_GetDefaultNet(&netHandle);
    NetConn_NetCapabilities netCapacities;
    int32_t returnValue = OH_NetConn_GetNetCapabilities(&netHandle, &netCapacities);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

napi_value NetConn_GetAddrInfo(napi_env env, napi_callback_info info)
{
    std::string host = GetString(env, info, 0);
    std::string serv = GetString(env, info, 1);
    struct addrinfo *res = nullptr;
    int32_t returnValue = OH_NetConn_GetAddrInfo(const_cast<char*>(host.c_str()),
        const_cast<char*>(serv.c_str()), nullptr, &res, 0);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

napi_value NetConn_GetAllNets(napi_env env, napi_callback_info info)
{
    NetConn_NetHandleList netHandles;
    int32_t returnValue = OH_NetConn_GetAllNets(&netHandles);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

napi_value NetConn_SetPacUrl(napi_env env, napi_callback_info info)
{
    std::string pacUrl = GetString(env, info, 0);
    int32_t returnValue = OH_NetConn_SetPacUrl(pacUrl.c_str());
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

napi_value NetConn_GetPacUrl(napi_env env, napi_callback_info info)
{
    int32_t index = GetInt(env, info, 0);
    char tmpBuffer[MAX_BUFFER_SIZE];
    char *pac = tmpBuffer;
    if ( index == 0) {
        pac = nullptr;
    }
    int32_t returnValue = OH_NetConn_GetPacUrl(pac);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, returnValue, &result));
    return result;
}

void MyNetCapabilitiesChange(NetConn_NetHandle *netHandle, NetConn_NetCapabilities *netCapabilities)
{
    return;
}

napi_value NetConn_NetCapabilities_netCaps(napi_env env, napi_callback_info info)
{
    int32_t index = GetInt(env, info, 0);
    NetConn_NetHandle netHandle;
    NetConn_NetCapabilities netCapabilities = {};
    netCapabilities.linkUpBandwidthKbps = MAX_NAME_LENGTH;
    netCapabilities.linkDownBandwidthKbps = MAX_NAME_LENGTH;
    int32_t val = CASE_INDEX_1;
    switch (index) {
        case CASE_INDEX_1:
            val = NetConn_NetCap::NETCONN_NET_CAPABILITY_MMS;
            break;
        case CASE_INDEX_2:
            val = NetConn_NetCap::NETCONN_NET_CAPABILITY_NOT_METERED;
            break;
        case CASE_INDEX_3:
            val = NetConn_NetCap::NETCONN_NET_CAPABILITY_INTERNET;
            break;
        case CASE_INDEX_4:
            val = NetConn_NetCap::NETCONN_NET_CAPABILITY_NOT_VPN;
            break;
        case CASE_INDEX_5:
            val = NetConn_NetCap::NETCONN_NET_CAPABILITY_VALIDATED;
            break;
        case CASE_INDEX_6:
            val = NetConn_NetCap::NETCONN_NET_CAPABILITY_PORTAL;
            break;
        case CASE_INDEX_7:
            val = NetConn_NetCap::NETCONN_NET_CAPABILITY_CHECKING_CONNECTIVITY;
            break;
        default:
            break;
    }
    netCapabilities.netCaps[0] = static_cast<NetConn_NetCap>(val);
    OH_NetConn_NetCapabilitiesChange capabilitiesChangeFunc = MyNetCapabilitiesChange;
    capabilitiesChangeFunc(&netHandle, &netCapabilities);
    napi_value result = nullptr;
    NAPI_CALL(env, napi_create_int32(env, 0, &result));
    return result;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        {"NetConn_HasDefaultNet", nullptr, NetConn_HasDefaultNet, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"NetConn_GetDefaultNet", nullptr, NetConn_GetDefaultNet, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"NetConn_IsDefaultNetMetered", nullptr, NetConn_IsDefaultNetMetered, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"NetConn_GetConnectionProperties", nullptr, NetConn_GetConnectionProperties, nullptr, nullptr,
         nullptr, napi_default, nullptr},
        {"NetConn_GetNetCapabilities", nullptr, NetConn_GetNetCapabilities, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"NetConn_GetAddrInfo", nullptr, NetConn_GetAddrInfo, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"NetConn_GetAllNets", nullptr, NetConn_GetAllNets, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"NetConn_SetPacUrl", nullptr, NetConn_SetPacUrl, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"NetConn_NetCapabilities_netCaps", nullptr, NetConn_NetCapabilities_netCaps, nullptr, nullptr,
         nullptr, napi_default, nullptr},
        {"NetConn_GetPacUrl", nullptr, NetConn_GetPacUrl, nullptr, nullptr, nullptr, napi_default,
         nullptr},
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
    .nm_modname = "netconnectiontest",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterModule(void) { napi_module_register(&demoModule); }
