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
#include <ace/xcomponent/native_xcomponent_key_event.h>
#include <js_native_api_types.h>
#include <napi/native_api.h>
namespace {
static const uint32_t MAX_NAME_LENGTH = 1024;
static bool g_keyApostropheIsTrue = 0;
static bool g_keyGraveIsTrue = 0;
static bool g_keyMinusIsTrue = 0;
static bool g_keyEqualsIsTrue = 0;
static bool g_keyLeftBracket = 0;
static bool g_keyRightBracketIsTrue = 0;
static bool g_keyBackslashIsTrue = 0;
static bool g_keySemicolonIsTrue = 0;
static bool g_keySlashIsTrue = 0;
static bool g_keyAtIsTrue = 0;
}

static napi_value GetKeyCodeEventKeyApostrophe(napi_env env, napi_callback_info info)
{
    if (g_keyApostropheIsTrue) {
        napi_value retValue;
        napi_create_int32(env, g_keyApostropheIsTrue, &retValue);
        return retValue;
    }
    return 0;
}

static napi_value GetKeyCodeEventKeyGrave(napi_env env, napi_callback_info info)
{
    if (g_keyGraveIsTrue) {
        napi_value retValue;
        napi_create_int32(env, g_keyGraveIsTrue, &retValue);
        return retValue;
    }
    return 0;
}
static napi_value GetKeyCodeEventKeyMinus(napi_env env, napi_callback_info info)
{
    if (g_keyMinusIsTrue) {
        napi_value retValue;
        napi_create_int32(env, g_keyMinusIsTrue, &retValue);
        return retValue;
    }
    return 0;
}
static napi_value GetKeyCodeEventKeyEquals(napi_env env, napi_callback_info info)
{
    if (g_keyEqualsIsTrue) {
        napi_value retValue;
        napi_create_int32(env, g_keyEqualsIsTrue, &retValue);
        return retValue;
    }
    return 0;
}
static napi_value GetKeyCodeEventKeyLeftbracket(napi_env env, napi_callback_info info)
{
    if (g_keyLeftBracket) {
        napi_value retValue;
        napi_create_int32(env, g_keyLeftBracket, &retValue);
        return retValue;
    }
    return 0;
}
static napi_value GetKeyCodeEventKeyRightbracket(napi_env env, napi_callback_info info)
{
    if (g_keyRightBracketIsTrue) {
        napi_value retValue;
        napi_create_int32(env, g_keyRightBracketIsTrue, &retValue);
        return retValue;
    }
    return 0;
}
static napi_value GetKeyCodeEventKeyBackslash(napi_env env, napi_callback_info info)
{
    if (g_keyBackslashIsTrue) {
        napi_value retValue;
        napi_create_int32(env, g_keyBackslashIsTrue, &retValue);
        return retValue;
    }
    return 0;
}
static napi_value GetKeyCodeEventKeySemicolon(napi_env env, napi_callback_info info)
{
    if (g_keySemicolonIsTrue) {
        napi_value retValue;
        napi_create_int32(env, g_keySemicolonIsTrue, &retValue);
        return retValue;
    }
    return 0;
}
static napi_value GetKeyCodeEventKeySlash(napi_env env, napi_callback_info info)
{
    if (g_keySlashIsTrue) {
        napi_value retValue;
        napi_create_int32(env, g_keySlashIsTrue, &retValue);
        return retValue;
    }
    return 0;
}
static napi_value GetKeyCodeEventKeyAt(napi_env env, napi_callback_info info)
{
    if (g_keyAtIsTrue) {
        napi_value retValue;
        napi_create_int32(env, g_keyAtIsTrue, &retValue);
        return retValue;
    }
    return 0;
}

static napi_value InitKeyMonitor(napi_env env, napi_callback_info info)
{
    char str[MAX_NAME_LENGTH] = {0};
    size_t argc = 1;
    int32_t intValue;
    napi_value args[1]; // 存储参数数组

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    napi_get_value_int32(env, args[0], &intValue);
    if (KEY_APOSTROPHE == intValue) {
        g_keyApostropheIsTrue = 1;
    } else if (KEY_GRAVE == intValue) {
        g_keyGraveIsTrue = 1;
    } else if (KEY_MINUS == intValue) {
        g_keyMinusIsTrue = 1;
    } else if (KEY_EQUALS == intValue) {
        g_keyEqualsIsTrue = 1;
    } else if (KEY_LEFT_BRACKET == intValue) {
        g_keyLeftBracket = 1;
    } else if (KEY_RIGHT_BRACKET == intValue) {
        g_keyRightBracketIsTrue = 1;
    } else if (KEY_BACKSLASH == intValue) {
        g_keyBackslashIsTrue = 1;
    } else if (KEY_SEMICOLON == intValue) {
        g_keySemicolonIsTrue = 1;
    } else if (KEY_SLASH == intValue) {
        g_keySlashIsTrue = 1;
    } else if (KEY_AT == intValue) {
        g_keyAtIsTrue = 1;
    }
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        { "GetKeyCodeEventKeyApostrophe", nullptr, GetKeyCodeEventKeyApostrophe, nullptr, nullptr, nullptr,
          napi_default, nullptr },
        { "GetKeyCodeEventKeyGrave", nullptr, GetKeyCodeEventKeyGrave, nullptr, nullptr, nullptr, napi_default,
          nullptr },
        { "GetKeyCodeEventKeyMinus", nullptr, GetKeyCodeEventKeyMinus, nullptr, nullptr, nullptr, napi_default,
          nullptr },
        { "GetKeyCodeEventKeyEquals", nullptr, GetKeyCodeEventKeyEquals, nullptr, nullptr, nullptr, napi_default,
          nullptr },
        { "GetKeyCodeEventKeyLeftbracket", nullptr, GetKeyCodeEventKeyLeftbracket, nullptr, nullptr, nullptr,
          napi_default, nullptr },
        { "GetKeyCodeEventKeyRightbracket", nullptr, GetKeyCodeEventKeyRightbracket, nullptr, nullptr, nullptr,
          napi_default, nullptr },
        { "GetKeyCodeEventKeyBackslash", nullptr, GetKeyCodeEventKeyBackslash, nullptr, nullptr, nullptr,
          napi_default, nullptr },
        { "GetKeyCodeEventKeySemicolon", nullptr, GetKeyCodeEventKeySemicolon, nullptr, nullptr, nullptr,
          napi_default, nullptr },
        { "GetKeyCodeEventKeySlash", nullptr, GetKeyCodeEventKeySlash, nullptr, nullptr, nullptr, napi_default,
          nullptr },
        { "GetKeyCodeEventKeyAt", nullptr, GetKeyCodeEventKeyAt, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "InitKeyMonitor", nullptr, InitKeyMonitor, nullptr, nullptr, nullptr, napi_default, nullptr }
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
    .nm_modname = "keycodendk",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterModule(void) { napi_module_register(&demoModule); }
