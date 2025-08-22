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
#include "hilog/log.h"
#include "napi/native_api.h"
#include "web/arkweb_interface.h"
#include "web/native_interface_arkweb.h"
#include <iostream>
#include <new>
#include <string>
using namespace std;
constexpr unsigned int LOG_PRINT_DOMAIN = 0xFF00;
const char *g_tag = "YB";
const char *g_doblanklessGetInfoValue2048Key = "";
const char *g_doblanklessGetInfoValueWithKey = "GetBlanklessInfoWithKey";
const char *g_doblanklessINVALID_ARGS = "";
const char *g_setBlanklessLoadingWithKey = "SetBlanklessInfoWithKey";
const char *g_doblanklessClearWithKey = "ClearBlankLoadingCache";
const int g_errCode = 801;
const double g_similarity = 0.33;

static napi_value Doblankless_Success(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // 获取第一个参数 webTag
    size_t webTagSize = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &webTagSize);
    char *webTagValue = new (std::nothrow) char[webTagSize + 1];
    size_t webTagLength = 0;
    napi_get_value_string_utf8(env, args[0], webTagValue, webTagSize + 1, &webTagLength);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "%{public}s, length = %{public}d", webTagValue,
                 (int)webTagSize);
    napi_value Capacity;
    ArkWeb_BlanklessInfo blanklessInfo =
        OH_NativeArkWeb_GetBlanklessInfoWithKey(webTagValue, g_doblanklessGetInfoValueWithKey);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag,
                 "mmx1 similarity:%{public}f,loadingTime:%{public}d,errCode:%{public}d", blanklessInfo.similarity,
                 blanklessInfo.loadingTime, blanklessInfo.errCode);
    if (blanklessInfo.errCode == ARKWEB_BLANKLESS_SUCCESS) {
        if (blanklessInfo.similarity >= g_similarity) {
            OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag,
                         "mmx2 similarity:%{public}f,loadingTime:%{public}d", blanklessInfo.similarity,
                         blanklessInfo.loadingTime);
            ArkWeb_BlanklessErrorCode err =
                OH_NativeArkWeb_SetBlanklessLoadingWithKey(webTagValue, g_doblanklessGetInfoValueWithKey, true);
            napi_create_double(env, blanklessInfo.errCode, &Capacity);
        } else {
            ArkWeb_BlanklessErrorCode err =
                OH_NativeArkWeb_SetBlanklessLoadingWithKey(webTagValue, g_doblanklessGetInfoValueWithKey, true);
            napi_create_double(env, err, &Capacity);
        }
    } else if (blanklessInfo.errCode == g_errCode) {
        napi_create_double(env, blanklessInfo.errCode, &Capacity);
    }
    return Capacity;
}

static napi_value ClearBlanklessLoadingCache(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // 获取第一个参数 webTag
    size_t webTagSize = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &webTagSize);
    char *webTagValue = new (std::nothrow) char[webTagSize + 1];
    size_t webTagLength = 0;
    napi_get_value_string_utf8(env, args[0], webTagValue, webTagSize + 1, &webTagLength);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "%{public}s, length = %{public}d", webTagValue,
                 (int)webTagSize);
    const char *key[1] = {g_doblanklessGetInfoValueWithKey};
    OH_NativeArkWeb_ClearBlanklessLoadingCache(key, 1);
    return nullptr;
}
static napi_value Doblankless_ERR_KEY_NOT_MATCH(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // 获取第一个参数 webTag
    size_t webTagSize = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &webTagSize);
    char *webTagValue = new (std::nothrow) char[webTagSize + 1];
    size_t webTagLength = 0;
    napi_get_value_string_utf8(env, args[0], webTagValue, webTagSize + 1, &webTagLength);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "%{public}s, length = %{public}d", webTagValue,
                 (int)webTagSize);

    napi_value Capacity;
    ArkWeb_BlanklessInfo blanklessInfo =
        OH_NativeArkWeb_GetBlanklessInfoWithKey(webTagValue, g_doblanklessGetInfoValueWithKey);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "mmx errCode:%{public}d", blanklessInfo.errCode);
    if (blanklessInfo.errCode == ARKWEB_BLANKLESS_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "mmx similarity:%{public}f,loadingTime:%{public}d",
                     blanklessInfo.similarity, blanklessInfo.loadingTime);
        ArkWeb_BlanklessErrorCode err =
            OH_NativeArkWeb_SetBlanklessLoadingWithKey(webTagValue, g_setBlanklessLoadingWithKey, false);
        if (err == ARKWEB_BLANKLESS_ERR_KEY_NOT_MATCH) {
            napi_create_double(env, err, &Capacity);
        }
    } else if (blanklessInfo.errCode == g_errCode) {
        napi_create_double(env, blanklessInfo.errCode, &Capacity);
    }
    return Capacity;
}

static napi_value Doblankless_CONTROLLER_NOT_INITED(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // 获取第一个参数 webTag
    size_t webTagSize = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &webTagSize);
    char *webTagValue = new (std::nothrow) char[webTagSize + 1];
    size_t webTagLength = 0;
    napi_get_value_string_utf8(env, args[0], webTagValue, webTagSize + 1, &webTagLength);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "%{public}s, length = %{public}d", webTagValue,
                 (int)webTagSize);

    napi_value Capacity;
    ArkWeb_BlanklessInfo blanklessInfo = OH_NativeArkWeb_GetBlanklessInfoWithKey("", g_doblanklessGetInfoValueWithKey);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "mmx errCode:%{public}d", blanklessInfo.errCode);
    if (blanklessInfo.errCode == ARKWEB_BLANKLESS_ERR_CONTROLLER_NOT_INITED) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "mmx similarity:%{public}f,loadingTime:%{public}d",
                     blanklessInfo.similarity, blanklessInfo.loadingTime);
        napi_create_double(env, blanklessInfo.errCode, &Capacity);
    } else if (blanklessInfo.errCode == g_errCode) {
        napi_create_double(env, blanklessInfo.errCode, &Capacity);
    }
    return Capacity;
}

static napi_value Doblankless_INVALID_ARGS(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // 获取第一个参数 webTag
    size_t webTagSize = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &webTagSize);
    char *webTagValue = new (std::nothrow) char[webTagSize + 1];
    size_t webTagLength = 0;
    napi_get_value_string_utf8(env, args[0], webTagValue, webTagSize + 1, &webTagLength);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "%{public}s, length = %{public}d", webTagValue,
                 (int)webTagSize);

    napi_value Capacity;
    ArkWeb_BlanklessInfo blanklessInfo =
        OH_NativeArkWeb_GetBlanklessInfoWithKey(webTagValue, g_doblanklessINVALID_ARGS);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "mmx errCode:%{public}d", blanklessInfo.errCode);
    if (blanklessInfo.errCode == ARKWEB_BLANKLESS_ERR_INVALID_ARGS) {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "mmx similarity:%{public}f,loadingTime:%{public}d",
                     blanklessInfo.similarity, blanklessInfo.loadingTime);
        napi_create_double(env, blanklessInfo.errCode, &Capacity);
    } else if (blanklessInfo.errCode == g_errCode) {
        napi_create_double(env, blanklessInfo.errCode, &Capacity);
    }
    return Capacity;
}

static napi_value SetCacheCapacity_1(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // 获取第一个参数 webTag
    size_t webTagSize = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &webTagSize);
    char *webTagValue = new (std::nothrow) char[webTagSize + 1];
    size_t webTagLength = 0;
    napi_value Capacity;
    napi_get_value_string_utf8(env, args[0], webTagValue, webTagSize + 1, &webTagLength);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "%{public}s, length = %{public}d", webTagValue,
                 (int)webTagSize);
    uint32_t capacity = OH_NativeArkWeb_SetBlanklessLoadingCacheCapacity(-1);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "testLog: SetCacheCapacity0:capacity:%{public}d",
                 capacity);
    napi_create_uint32(env, capacity, &Capacity);
    return Capacity;
}

static napi_value SetCacheCapacity0(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // 获取第一个参数 webTag
    size_t webTagSize = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &webTagSize);
    char *webTagValue = new (std::nothrow) char[webTagSize + 1];
    size_t webTagLength = 0;
    napi_value Capacity;
    napi_get_value_string_utf8(env, args[0], webTagValue, webTagSize + 1, &webTagLength);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "%{public}s, length = %{public}d", webTagValue,
                 (int)webTagSize);
    uint32_t capacity = OH_NativeArkWeb_SetBlanklessLoadingCacheCapacity(0);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "testLog: SetCacheCapacity0:capacity:%{public}d",
                 capacity);
    napi_create_uint32(env, capacity, &Capacity);
    return Capacity;
}

static napi_value SetCacheCapacity50(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // 获取第一个参数 webTag
    size_t webTagSize = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &webTagSize);
    char *webTagValue = new (std::nothrow) char[webTagSize + 1];
    size_t webTagLength = 0;
    napi_value Capacity;
    napi_get_value_string_utf8(env, args[0], webTagValue, webTagSize + 1, &webTagLength);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "%{public}s, length = %{public}d", webTagValue,
                 (int)webTagSize);
    uint32_t capacity = OH_NativeArkWeb_SetBlanklessLoadingCacheCapacity(50);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "testLog: SetCacheCapacity0:capacity:%{public}d",
                 capacity);
    napi_create_uint32(env, capacity, &Capacity);
    return Capacity;
}

static napi_value SetCacheCapacity101(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    napi_value args[1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    // 获取第一个参数 webTag
    size_t webTagSize = 0;
    napi_get_value_string_utf8(env, args[0], nullptr, 0, &webTagSize);
    char *webTagValue = new (std::nothrow) char[webTagSize + 1];
    size_t webTagLength = 0;
    napi_value Capacity;
    napi_get_value_string_utf8(env, args[0], webTagValue, webTagSize + 1, &webTagLength);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "%{public}s, length = %{public}d", webTagValue,
                 (int)webTagSize);
    uint32_t capacity = OH_NativeArkWeb_SetBlanklessLoadingCacheCapacity(101);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, g_tag, "testLog: SetCacheCapacity0:capacity:%{public}d",
                 capacity);
    napi_create_uint32(env, capacity, &Capacity);
    return Capacity;
}

static napi_value Add(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    napi_valuetype valuetype0;
    napi_typeof(env, args[0], &valuetype0);

    napi_valuetype valuetype1;
    napi_typeof(env, args[1], &valuetype1);

    double value0;
    napi_get_value_double(env, args[0], &value0);

    double value1;
    napi_get_value_double(env, args[1], &value1);

    napi_value sum;
    napi_create_double(env, value0 + value1, &sum);

    return sum;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        {"add", nullptr, Add, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"doblankless_Success", nullptr, Doblankless_Success, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"clearBlanklessLoadingCache", nullptr, ClearBlanklessLoadingCache, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"doblankless_ERR_KEY_NOT_MATCH", nullptr, Doblankless_ERR_KEY_NOT_MATCH, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"doblankless_CONTROLLER_NOT_INITED", nullptr, Doblankless_CONTROLLER_NOT_INITED, nullptr, nullptr, nullptr,
         napi_default, nullptr},
        {"doblankless_INVALID_ARGS", nullptr, Doblankless_INVALID_ARGS, nullptr, nullptr, nullptr, napi_default,
         nullptr},
        {"setCacheCapacity_1", nullptr, SetCacheCapacity_1, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setCacheCapacity0", nullptr, SetCacheCapacity0, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setCacheCapacity50", nullptr, SetCacheCapacity50, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"setCacheCapacity101", nullptr, SetCacheCapacity101, nullptr, nullptr, nullptr, napi_default, nullptr}

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
    .nm_modname = "entry",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void) { napi_module_register(&demoModule); }
