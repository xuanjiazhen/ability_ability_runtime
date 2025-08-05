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
#include "native_common.h"
#include "nativekeycode/KeyCodeFunction3.h"
#include <ace/xcomponent/native_xcomponent_key_event.h>
#include <cstdint>
#include <js_native_api_types.h>
#include <napi/native_api.h>

static napi_value InitKeyMonitor(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    int32_t intValue;
    napi_value args[1]; // 存储参数数组

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    napi_get_value_int32(env, args[0], &intValue);
    struct KeyMap { int key; bool* flag; } map[] = {
        { KEY_APOSTROPHE, KEY(ApostropheIsTrue) }, { KEY_GRAVE, KEY(GraveIsTrue) },
        { KEY_MINUS, KEY(MinusIsTrue) }, { KEY_EQUALS, KEY(EqualsIsTrue) },
        { KEY_LEFT_BRACKET, KEY(LeftBracketIsTrue) }, { KEY_RIGHT_BRACKET, KEY(RightBracketIsTrue) },
        { KEY_BACKSLASH, KEY(BackslashIsTrue) }, { KEY_SEMICOLON, KEY(SemicolonIsTrue) },
        { KEY_SLASH, KEY(SlashIsTrue) }, { KEY_AT, KEY(AtIsTrue) }
    };
    for (size_t i = 0; i < sizeof(map) / sizeof(map[0]); i++) {
        if (intValue == map[i].key) {
            *(map[i].flag) = 1;
            break;
        }
    }
    return nullptr;
}

EXTERN_C_START
static void InitPush0(napi_property_descriptor* desc)
{
    napi_property_descriptor descTemp[] = {
        XL("GetKeyCodeEventApostrophe", GetKeyCodeEventApostrophe),
        XL("GetKeyCodeEventGrave", GetKeyCodeEventGrave),
        XL("GetKeyCodeEventMinus", GetKeyCodeEventMinus),
        XL("GetKeyCodeEventEquals", GetKeyCodeEventEquals),
        XL("GetKeyCodeEventLeftBracket", GetKeyCodeEventLeftBracket),
        XL("GetKeyCodeEventRightBracket", GetKeyCodeEventRightBracket),
        XL("GetKeyCodeEventBackslash", GetKeyCodeEventBackslash),
        XL("GetKeyCodeEventSemicolon", GetKeyCodeEventSemicolon),
        XL("GetKeyCodeEventSlash", GetKeyCodeEventSlash),
        XL("GetKeyCodeEventAt", GetKeyCodeEventAt),
        XL("InitKeyMonitor", InitKeyMonitor),
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index] = descTemp[index];
    }
}

static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[335] = { };
    InitPush0(desc);
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
#undef XL
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
