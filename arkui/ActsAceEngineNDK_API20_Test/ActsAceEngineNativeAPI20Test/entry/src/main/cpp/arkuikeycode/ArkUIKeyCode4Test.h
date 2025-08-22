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
#ifndef ARKUI_KEYCODE_TEST4_H
#define ARKUI_KEYCODE_TEST4_H


#include "napi/native_api.h"
#include <ace/xcomponent/native_xcomponent_key_event.h>
#include <arkui/native_key_event.h>
#include <js_native_api_types.h>

namespace ArkUIKeyCode4 {
    bool g_arkuiKeyCodeF10Flag = 0;
    bool g_arkuiKeyCodeF11Flag = 0;
    bool g_arkuiKeyCodeF12Flag = 0;
    bool g_arkuiKeyCodeNumLockFlag = 0;
    bool g_arkuiKeyCodeNumpad0Flag = 0;
    bool g_arkuiKeyCodeNumpad1Flag = 0;
    bool g_arkuiKeyCodeNumpad2Flag = 0;
    bool g_arkuiKeyCodeNumpad3Flag = 0;
    bool g_arkuiKeyCodeNumpad4Flag = 0;
    bool g_arkuiKeyCodeNumpad5Flag = 0;
    bool g_arkuiKeyCodeNumpad6Flag = 0;
    bool g_arkuiKeyCodeNumpad7Flag = 0;
    bool g_arkuiKeyCodeNumpad8Flag = 0;
    bool g_arkuiKeyCodeNumpad9Flag = 0;
    bool g_arkuiKeyCodeNumpadDivideFlag = 0;
    bool g_arkuiKeyCodeNumpadMultiplyFlag = 0;
    bool g_arkuiKeyCodeNumpadSubtractFlag = 0;
    bool g_arkuiKeyCodeNumpadAddFlag = 0;
    bool g_arkuiKeyCodeNumpadDotFlag = 0;
    bool g_arkuiKeyCodeNumpadCommaFlag = 0;
    bool g_arkuiKeyCodeNumpadEnterFlag = 0;
    bool g_arkuiKeyCodeNumpadEqualsFlag = 0;
    bool g_arkuiKeyCodeNumpadLeftParenFlag = 0;
    bool g_arkuiKeyCodeNumpadRightParenFlag = 0;
    bool g_arkuiKeyCodeButtonAFlag = 0;
    bool g_arkuiKeyCodeButtonBFlag = 0;
    bool g_arkuiKeyCodeButtonXFlag = 0;
    bool g_arkuiKeyCodeButtonYFlag = 0;
    bool g_arkuiKeyCodeButtonL1Flag = 0;
    bool g_arkuiKeyCodeButtonR1Flag = 0;
    bool g_arkuiKeyCodeButtonL2Flag = 0;
    bool g_arkuiKeyCodeButtonR2Flag = 0;
    bool g_arkuiKeyCodeButtonSelectFlag = 0;
    bool g_arkuiKeyCodeButtonStartFlag = 0;
    bool g_arkuiKeyCodeButtonModeFlag = 0;
    bool g_arkuiKeyCodeButtonThumblFlag = 0;
    bool g_arkuiKeyCodeButtonThumbrFlag = 0;
} // namespace

napi_value GetArkUIKeyCodeEventF10(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeF10Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeF10Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventF11(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeF11Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeF11Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventF12(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeF12Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeF12Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumLock(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumLockFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumLockFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpad0(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpad0Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpad0Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpad1(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpad1Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpad1Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpad2(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpad2Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpad2Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpad3(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpad3Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpad3Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpad4(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpad4Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpad4Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpad5(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpad5Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpad5Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpad6(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpad6Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpad6Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpad7(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpad7Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpad7Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpad8(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpad8Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpad8Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpad9(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpad9Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpad9Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpadDivide(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpadDivideFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpadDivideFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpadMultiply(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpadMultiplyFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpadMultiplyFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpadSubtract(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpadSubtractFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpadSubtractFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpadAdd(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpadAddFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpadAddFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpadDot(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpadDotFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpadDotFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpadComma(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpadCommaFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpadCommaFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpadEnter(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpadEnterFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpadEnterFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpadEquals(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpadEqualsFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpadEqualsFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpadLeftParen(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpadLeftParenFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpadLeftParenFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventNumpadRightParen(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeNumpadRightParenFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeNumpadRightParenFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventButtonA(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeButtonAFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeButtonAFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventButtonB(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeButtonBFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeButtonBFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventButtonX(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeButtonXFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeButtonXFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventButtonY(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeButtonYFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeButtonYFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventButtonL1(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeButtonL1Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeButtonL1Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventButtonR1(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeButtonR1Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeButtonR1Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventButtonL2(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeButtonL2Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeButtonL2Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventButtonR2(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeButtonR2Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeButtonR2Flag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventButtonSelect(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeButtonSelectFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeButtonSelectFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventButtonStart(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeButtonStartFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeButtonStartFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventButtonMode(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeButtonModeFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeButtonModeFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventButtonThumbl(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeButtonThumblFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeButtonThumblFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value GetArkUIKeyCodeEventButtonThumbr(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode4::g_arkuiKeyCodeButtonThumbrFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode4::g_arkuiKeyCodeButtonThumbrFlag, &retValue);
        return retValue;
    }
    return 0;
}

napi_value InitArkUIKeyMonitor4(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    int32_t intValue;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    napi_get_value_int32(env, args[0], &intValue);
    struct { int key; bool* flag; } map[] = {
        { ARKUI_KEYCODE_NUM_LOCK, &ArkUIKeyCode4::g_arkuiKeyCodeNumLockFlag }, { ARKUI_KEYCODE_NUMPAD_0,
        &ArkUIKeyCode4::g_arkuiKeyCodeNumpad0Flag }, { ARKUI_KEYCODE_F10, &ArkUIKeyCode4::g_arkuiKeyCodeF10Flag },
        { ARKUI_KEYCODE_NUMPAD_1, &ArkUIKeyCode4::g_arkuiKeyCodeNumpad1Flag }, { ARKUI_KEYCODE_F11,
        &ArkUIKeyCode4::g_arkuiKeyCodeF11Flag }, { ARKUI_KEYCODE_NUMPAD_2, &ArkUIKeyCode4::g_arkuiKeyCodeNumpad2Flag },
        { ARKUI_KEYCODE_NUMPAD_3, &ArkUIKeyCode4::g_arkuiKeyCodeNumpad3Flag }, { ARKUI_KEYCODE_F12,
        &ArkUIKeyCode4::g_arkuiKeyCodeF12Flag }, { ARKUI_KEYCODE_NUMPAD_4, &ArkUIKeyCode4::g_arkuiKeyCodeNumpad4Flag },
        { ARKUI_KEYCODE_NUMPAD_5, &ArkUIKeyCode4::g_arkuiKeyCodeNumpad5Flag },
        { ARKUI_KEYCODE_NUMPAD_6, &ArkUIKeyCode4::g_arkuiKeyCodeNumpad6Flag },
        { ARKUI_KEYCODE_NUMPAD_7, &ArkUIKeyCode4::g_arkuiKeyCodeNumpad7Flag },
        { ARKUI_KEYCODE_NUMPAD_8, &ArkUIKeyCode4::g_arkuiKeyCodeNumpad8Flag },
        { ARKUI_KEYCODE_NUMPAD_9, &ArkUIKeyCode4::g_arkuiKeyCodeNumpad9Flag },
        { ARKUI_KEYCODE_NUMPAD_DIVIDE, &ArkUIKeyCode4::g_arkuiKeyCodeNumpadDivideFlag },
        { ARKUI_KEYCODE_NUMPAD_MULTIPLY, &ArkUIKeyCode4::g_arkuiKeyCodeNumpadMultiplyFlag },
        { ARKUI_KEYCODE_NUMPAD_SUBTRACT, &ArkUIKeyCode4::g_arkuiKeyCodeNumpadSubtractFlag },
        { ARKUI_KEYCODE_NUMPAD_ADD, &ArkUIKeyCode4::g_arkuiKeyCodeNumpadAddFlag },
        { ARKUI_KEYCODE_NUMPAD_DOT, &ArkUIKeyCode4::g_arkuiKeyCodeNumpadDotFlag },
        { ARKUI_KEYCODE_NUMPAD_COMMA, &ArkUIKeyCode4::g_arkuiKeyCodeNumpadCommaFlag },
        { ARKUI_KEYCODE_NUMPAD_ENTER, &ArkUIKeyCode4::g_arkuiKeyCodeNumpadEnterFlag },
        { ARKUI_KEYCODE_NUMPAD_EQUALS, &ArkUIKeyCode4::g_arkuiKeyCodeNumpadEqualsFlag },
        { ARKUI_KEYCODE_NUMPAD_LEFT_PAREN, &ArkUIKeyCode4::g_arkuiKeyCodeNumpadLeftParenFlag },
        { ARKUI_KEYCODE_NUMPAD_RIGHT_PAREN, &ArkUIKeyCode4::g_arkuiKeyCodeNumpadRightParenFlag },
        { ARKUI_KEYCODE_BUTTON_A, &ArkUIKeyCode4::g_arkuiKeyCodeButtonAFlag },
        { ARKUI_KEYCODE_BUTTON_B, &ArkUIKeyCode4::g_arkuiKeyCodeButtonBFlag },
        { ARKUI_KEYCODE_BUTTON_X, &ArkUIKeyCode4::g_arkuiKeyCodeButtonXFlag },
        { ARKUI_KEYCODE_BUTTON_Y, &ArkUIKeyCode4::g_arkuiKeyCodeButtonYFlag },
        { ARKUI_KEYCODE_BUTTON_L1, &ArkUIKeyCode4::g_arkuiKeyCodeButtonL1Flag },
        { ARKUI_KEYCODE_BUTTON_R1, &ArkUIKeyCode4::g_arkuiKeyCodeButtonR1Flag },
        { ARKUI_KEYCODE_BUTTON_L2, &ArkUIKeyCode4::g_arkuiKeyCodeButtonL2Flag },
        { ARKUI_KEYCODE_BUTTON_R2, &ArkUIKeyCode4::g_arkuiKeyCodeButtonR2Flag },
        { ARKUI_KEYCODE_BUTTON_SELECT, &ArkUIKeyCode4::g_arkuiKeyCodeButtonSelectFlag },
        { ARKUI_KEYCODE_BUTTON_START, &ArkUIKeyCode4::g_arkuiKeyCodeButtonStartFlag },
        { ARKUI_KEYCODE_BUTTON_MODE, &ArkUIKeyCode4::g_arkuiKeyCodeButtonModeFlag },
        { ARKUI_KEYCODE_BUTTON_THUMBL, &ArkUIKeyCode4::g_arkuiKeyCodeButtonThumblFlag },
        { ARKUI_KEYCODE_BUTTON_THUMBR, &ArkUIKeyCode4::g_arkuiKeyCodeButtonThumbrFlag }
    };
    for (size_t i = 0; i < sizeof(map) / sizeof(map[0]); i++)
        if (intValue == map[i].key) {
        *map[i].flag = true;
        break;
        }
    return nullptr;
}
#endif // ARKUI_KEYCODE_TEST4_H