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
#ifndef ARKUI_KEYCODE_TEST2_H
#define ARKUI_KEYCODE_TEST2_H

#include "napi/native_api.h"
#include <arkui/native_key_event.h>
#include <js_native_api_types.h>
namespace ArkUIKeyCode2 {
    const uint32_t MAX_NAME_LENGTH = 1024;
    bool g_arkuiKeyCodeJFlag = 0;
    bool g_arkuiKeyCodeKFlag = 0;
    bool g_arkuiKeyCodeLFlag = 0;
    bool g_arkuiKeyCodeMFlag = 0;
    bool g_arkuiKeyCodeNFlag = 0;
    bool g_arkuiKeyCodeOFlag = 0;
    bool g_arkuiKeyCodePFlag = 0;
    bool g_arkuiKeyCodeQFlag = 0;
    bool g_arkuiKeyCodeRFlag = 0;
    bool g_arkuiKeyCodeSFlag = 0;
    bool g_arkuiKeyCodeTFlag = 0;
    bool g_arkuiKeyCodeUFlag = 0;
    bool g_arkuiKeyCodeVFlag = 0;
    bool g_arkuiKeyCodeWFlag = 0;
    bool g_arkuiKeyCodeXFlag = 0;
    bool g_arkuiKeyCodeYFlag = 0;
    bool g_arkuiKeyCodeZFlag = 0;
    bool g_arkuiKeyCodeCommaFlag = 0;
    bool g_arkuiKeyCodePeriodFlag = 0;
    bool g_arkuiKeyCodeAltLeftFlag = 0;
    bool g_arkuiKeyCodeAltRightFlag = 0;
    bool g_arkuiKeyCodeShiftLeftFlag = 0;
    bool g_arkuiKeyCodeShiftRightFlag = 0;
    bool g_arkuiKeyCodeTabFlag = 0;
    bool g_arkuiKeyCodeSpaceFlag = 0;
    bool g_arkuiKeyCodeSymFlag = 0;
    bool g_arkuiKeyCodeExplorerFlag = 0;
    bool g_arkuiKeyCodeEnvelopeFlag = 0;
    bool g_arkuiKeyCodeEnterFlag = 0;
    bool g_arkuiKeyCodeDelFlag = 0;
    bool g_arkuiKeyCodeGraveFlag = 0;
    bool g_arkuiKeyCodeMinusFlag = 0;
    bool g_arkuiKeyCodeEqualsFlag = 0;
    bool g_arkuiKeyCodeLeftBracketFlag = 0;
    bool g_arkuiKeyCodeRightBracketFlag = 0;
    bool g_arkuiKeyCodeBackslashFlag = 0;

}

napi_value GetArkUIKeyCodeEventJ(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeJFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeJFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventK(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeKFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeKFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventL(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeLFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeLFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventM(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeMFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeMFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventN(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeNFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeNFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventO(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeOFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeOFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventP(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodePFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodePFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventQ(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeQFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeQFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventR(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeRFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeRFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventS(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeSFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeSFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventT(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeTFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeTFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventU(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeUFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeUFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventV(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeVFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeVFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventW(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeWFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeWFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventX(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeXFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeXFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventY(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeYFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeYFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventZ(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeZFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeZFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventComma(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeCommaFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeCommaFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventPeriod(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodePeriodFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodePeriodFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventAltLeft(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeAltLeftFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeAltLeftFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventAltRight(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeAltRightFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeAltRightFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventShiftLeft(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeShiftLeftFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeShiftLeftFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventShiftRight(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeShiftRightFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeShiftRightFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventTab(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeTabFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeTabFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventSpace(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeSpaceFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeSpaceFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventSym(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeSymFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeSymFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventExplorer(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeExplorerFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeExplorerFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventEnvelope(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeEnvelopeFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeEnvelopeFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventEnter(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeEnterFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeEnterFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventDel(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeDelFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeDelFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventGrave(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeGraveFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeGraveFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventMinus(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeMinusFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeMinusFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventEquals(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeEqualsFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeEqualsFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventLeftBracket(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeLeftBracketFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeLeftBracketFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventRightBracket(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeRightBracketFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeRightBracketFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventBackslash(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode2::g_arkuiKeyCodeBackslashFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode2::g_arkuiKeyCodeBackslashFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value InitArkUIKeyMonitor2(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    int32_t intValue;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    napi_get_value_int32(env, args[0], &intValue);
    struct { int key; bool* flag; } map[] = {
    { ARKUI_KEYCODE_J, &ArkUIKeyCode2::g_arkuiKeyCodeJFlag },
    { ARKUI_KEYCODE_K, &ArkUIKeyCode2::g_arkuiKeyCodeKFlag },
    { ARKUI_KEYCODE_L, &ArkUIKeyCode2::g_arkuiKeyCodeLFlag },
    { ARKUI_KEYCODE_M, &ArkUIKeyCode2::g_arkuiKeyCodeMFlag },
    { ARKUI_KEYCODE_N, &ArkUIKeyCode2::g_arkuiKeyCodeNFlag },
    { ARKUI_KEYCODE_O, &ArkUIKeyCode2::g_arkuiKeyCodeOFlag },
    { ARKUI_KEYCODE_P, &ArkUIKeyCode2::g_arkuiKeyCodePFlag },
    { ARKUI_KEYCODE_Q, &ArkUIKeyCode2::g_arkuiKeyCodeQFlag },
    { ARKUI_KEYCODE_R, &ArkUIKeyCode2::g_arkuiKeyCodeRFlag },
    { ARKUI_KEYCODE_S, &ArkUIKeyCode2::g_arkuiKeyCodeSFlag },
    { ARKUI_KEYCODE_T, &ArkUIKeyCode2::g_arkuiKeyCodeTFlag },
    { ARKUI_KEYCODE_U, &ArkUIKeyCode2::g_arkuiKeyCodeUFlag },
    { ARKUI_KEYCODE_V, &ArkUIKeyCode2::g_arkuiKeyCodeVFlag }, { ARKUI_KEYCODE_W, &ArkUIKeyCode2::g_arkuiKeyCodeWFlag },
    { ARKUI_KEYCODE_X, &ArkUIKeyCode2::g_arkuiKeyCodeXFlag }, { ARKUI_KEYCODE_Y, &ArkUIKeyCode2::g_arkuiKeyCodeYFlag },
    { ARKUI_KEYCODE_Z, &ArkUIKeyCode2::g_arkuiKeyCodeZFlag },
    { ARKUI_KEYCODE_COMMA, &ArkUIKeyCode2::g_arkuiKeyCodeCommaFlag },
    { ARKUI_KEYCODE_PERIOD, &ArkUIKeyCode2::g_arkuiKeyCodePeriodFlag },
    { ARKUI_KEYCODE_ALT_LEFT, &ArkUIKeyCode2::g_arkuiKeyCodeAltLeftFlag },
    { ARKUI_KEYCODE_ALT_RIGHT, &ArkUIKeyCode2::g_arkuiKeyCodeAltRightFlag },
    { ARKUI_KEYCODE_SHIFT_LEFT, &ArkUIKeyCode2::g_arkuiKeyCodeShiftLeftFlag },
    { ARKUI_KEYCODE_SHIFT_RIGHT, &ArkUIKeyCode2::g_arkuiKeyCodeShiftRightFlag },
    { ARKUI_KEYCODE_TAB, &ArkUIKeyCode2::g_arkuiKeyCodeTabFlag },
    { ARKUI_KEYCODE_SPACE, &ArkUIKeyCode2::g_arkuiKeyCodeSpaceFlag },
    { ARKUI_KEYCODE_SYM, &ArkUIKeyCode2::g_arkuiKeyCodeSymFlag },
    { ARKUI_KEYCODE_EXPLORER, &ArkUIKeyCode2::g_arkuiKeyCodeExplorerFlag },
    { ARKUI_KEYCODE_ENVELOPE, &ArkUIKeyCode2::g_arkuiKeyCodeEnvelopeFlag },
    { ARKUI_KEYCODE_ENTER, &ArkUIKeyCode2::g_arkuiKeyCodeEnterFlag },
    { ARKUI_KEYCODE_DEL, &ArkUIKeyCode2::g_arkuiKeyCodeDelFlag },
    { ARKUI_KEYCODE_GRAVE, &ArkUIKeyCode2::g_arkuiKeyCodeGraveFlag },
    { ARKUI_KEYCODE_MINUS, &ArkUIKeyCode2::g_arkuiKeyCodeMinusFlag },
    { ARKUI_KEYCODE_EQUALS, &ArkUIKeyCode2::g_arkuiKeyCodeEqualsFlag },
    { ARKUI_KEYCODE_LEFT_BRACKET, &ArkUIKeyCode2::g_arkuiKeyCodeLeftBracketFlag },
    { ARKUI_KEYCODE_RIGHT_BRACKET, &ArkUIKeyCode2::g_arkuiKeyCodeRightBracketFlag },
    { ARKUI_KEYCODE_BACKSLASH, &ArkUIKeyCode2::g_arkuiKeyCodeBackslashFlag }
    };
    for (size_t i = 0; i < sizeof(map) / sizeof(map[0]); i++)
        if  (intValue == map[i].key) {
        *map[i].flag = true;
        break;
        }
    return nullptr;
}
#endif // ARKUI_KEYCODE_TEST2_H