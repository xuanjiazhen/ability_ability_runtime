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
#ifndef ARKUI_KEYCODE_TEST3_H
#define ARKUI_KEYCODE_TEST3_H

#include "napi/native_api.h"
#include <arkui/native_key_event.h>
#include <js_native_api_types.h>

namespace ArkUIKeyCode3 {
    bool g_arkuiKeyCodeSemicolonFlag = 0;
    bool g_arkuiKeyCodeApostropheFlag = 0;
    bool g_arkuiKeyCodeSlashFlag = 0;
    bool g_arkuiKeyCodeAtFlag = 0;
    bool g_arkuiKeyCodePlusFlag = 0;
    bool g_arkuiKeyCodeMenuFlag = 0;
    bool g_arkuiKeyCodePageUpFlag = 0;
    bool g_arkuiKeyCodePageDownFlag = 0;
    bool g_arkuiKeyCodeEscapeFlag = 0;
    bool g_arkuiKeyCodeForwardDelFlag = 0;
    bool g_arkuiKeyCodeCtrlLeftFlag = 0;
    bool g_arkuiKeyCodeCtrlRightFlag = 0;
    bool g_arkuiKeyCodeCapsLockFlag = 0;
    bool g_arkuiKeyCodeScrollLockFlag = 0;
    bool g_arkuiKeyCodeMetaLeftFlag = 0;
    bool g_arkuiKeyCodeMetaRightFlag = 0;
    bool g_arkuiKeyCodeFunctionFlag = 0;
    bool g_arkuiKeyCodeSysrqFlag = 0;
    bool g_arkuiKeyCodeBreakFlag = 0;
    bool g_arkuiKeyCodeMoveHomeFlag = 0;
    bool g_arkuiKeyCodeMoveEndFlag = 0;
    bool g_arkuiKeyCodeInsertFlag = 0;
    bool g_arkuiKeyCodeForwardFlag = 0;
    bool g_arkuiKeyCodeMediaPlayFlag = 0;
    bool g_arkuiKeyCodeMediaPauseFlag = 0;
    bool g_arkuiKeyCodeMediaCloseFlag = 0;
    bool g_arkuiKeyCodeMediaEjectFlag = 0;
    bool g_arkuiKeyCodeMediaRecordFlag = 0;
    bool g_arkuiKeyCodeF1Flag = 0;
    bool g_arkuiKeyCodeF2Flag = 0;
    bool g_arkuiKeyCodeF3Flag = 0;
    bool g_arkuiKeyCodeF4Flag = 0;
    bool g_arkuiKeyCodeF5Flag = 0;
    bool g_arkuiKeyCodeF6Flag = 0;
    bool g_arkuiKeyCodeF7Flag = 0;
    bool g_arkuiKeyCodeF8Flag = 0;
    bool g_arkuiKeyCodeF9Flag = 0;
} // namespace

napi_value GetArkUIKeyCodeEventSemicolon(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeSemicolonFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeSemicolonFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventApostrophe(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeApostropheFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeApostropheFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventSlash(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeSlashFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeSlashFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventAt(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeAtFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeAtFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventPlus(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodePlusFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodePlusFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventMenu(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeMenuFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeMenuFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventPageUp(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodePageUpFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodePageUpFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventPageDown(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodePageDownFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodePageDownFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventEscape(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeEscapeFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeEscapeFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventForwardDel(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeForwardDelFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeForwardDelFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventCtrlLeft(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeCtrlLeftFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeCtrlLeftFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventCtrlRight(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeCtrlRightFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeCtrlRightFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventCapsLock(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeCapsLockFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeCapsLockFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventScrollLock(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeScrollLockFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeScrollLockFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventMetaLeft(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeMetaLeftFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeMetaLeftFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventMetaRight(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeMetaRightFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeMetaRightFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventFunction(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeFunctionFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeFunctionFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventSysrq(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeSysrqFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeSysrqFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventBreak(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeBreakFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeBreakFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventMoveHome(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeMoveHomeFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeMoveHomeFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventMoveEnd(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeMoveEndFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeMoveEndFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventInsert(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeInsertFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeInsertFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventForward(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeForwardFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeForwardFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventMediaPlay(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeMediaPlayFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeMediaPlayFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventMediaPause(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeMediaPauseFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeMediaPauseFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventMediaClose(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeMediaCloseFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeMediaCloseFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventMediaEject(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeMediaEjectFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeMediaEjectFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventMediaRecord(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeMediaRecordFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeMediaRecordFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventF1(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeF1Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeF1Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventF2(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeF2Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeF2Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventF3(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeF3Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeF3Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventF4(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeF4Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeF4Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventF5(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeF5Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeF5Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventF6(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeF6Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeF6Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventF7(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeF7Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeF7Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventF8(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeF8Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeF8Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventF9(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode3::g_arkuiKeyCodeF9Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode3::g_arkuiKeyCodeF9Flag, &retValue);
        return retValue;
    }
    return 0;
}
typedef struct {
    int32_t keyCode;
    int* flagPtr;
} KeyCodeFlagPair;

napi_value InitArkUIKeyMonitor3(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    int32_t intValue;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    napi_get_value_int32(env, args[0], &intValue);
    struct { int key; bool* flag; } map[] = {
        { ARKUI_KEYCODE_SEMICOLON, &ArkUIKeyCode3::g_arkuiKeyCodeSemicolonFlag },
        { ARKUI_KEYCODE_APOSTROPHE, &ArkUIKeyCode3::g_arkuiKeyCodeApostropheFlag },
        { ARKUI_KEYCODE_SLASH, &ArkUIKeyCode3::g_arkuiKeyCodeSlashFlag },
        { ARKUI_KEYCODE_AT, &ArkUIKeyCode3::g_arkuiKeyCodeAtFlag },
        { ARKUI_KEYCODE_PLUS, &ArkUIKeyCode3::g_arkuiKeyCodePlusFlag },
        { ARKUI_KEYCODE_MENU, &ArkUIKeyCode3::g_arkuiKeyCodeMenuFlag },
        { ARKUI_KEYCODE_PAGE_UP, &ArkUIKeyCode3::g_arkuiKeyCodePageUpFlag },
        { ARKUI_KEYCODE_PAGE_DOWN, &ArkUIKeyCode3::g_arkuiKeyCodePageDownFlag },
        { ARKUI_KEYCODE_ESCAPE, &ArkUIKeyCode3::g_arkuiKeyCodeEscapeFlag },
        { ARKUI_KEYCODE_FORWARD_DEL, &ArkUIKeyCode3::g_arkuiKeyCodeForwardDelFlag },
        { ARKUI_KEYCODE_CTRL_LEFT, &ArkUIKeyCode3::g_arkuiKeyCodeCtrlLeftFlag },
        { ARKUI_KEYCODE_CTRL_RIGHT, &ArkUIKeyCode3::g_arkuiKeyCodeCtrlRightFlag },
        { ARKUI_KEYCODE_CAPS_LOCK, &ArkUIKeyCode3::g_arkuiKeyCodeCapsLockFlag },
        { ARKUI_KEYCODE_SCROLL_LOCK, &ArkUIKeyCode3::g_arkuiKeyCodeScrollLockFlag },
        { ARKUI_KEYCODE_META_LEFT, &ArkUIKeyCode3::g_arkuiKeyCodeMetaLeftFlag },
        { ARKUI_KEYCODE_META_RIGHT, &ArkUIKeyCode3::g_arkuiKeyCodeMetaRightFlag },
        { ARKUI_KEYCODE_FUNCTION, &ArkUIKeyCode3::g_arkuiKeyCodeFunctionFlag },
        { ARKUI_KEYCODE_SYSRQ, &ArkUIKeyCode3::g_arkuiKeyCodeSysrqFlag },
        { ARKUI_KEYCODE_BREAK, &ArkUIKeyCode3::g_arkuiKeyCodeBreakFlag },
        { ARKUI_KEYCODE_MOVE_HOME, &ArkUIKeyCode3::g_arkuiKeyCodeMoveHomeFlag },
        { ARKUI_KEYCODE_MOVE_END, &ArkUIKeyCode3::g_arkuiKeyCodeMoveEndFlag },
        { ARKUI_KEYCODE_INSERT, &ArkUIKeyCode3::g_arkuiKeyCodeInsertFlag },
        { ARKUI_KEYCODE_FORWARD, &ArkUIKeyCode3::g_arkuiKeyCodeForwardFlag },
        { ARKUI_KEYCODE_MEDIA_PLAY, &ArkUIKeyCode3::g_arkuiKeyCodeMediaPlayFlag },
        { ARKUI_KEYCODE_MEDIA_PAUSE, &ArkUIKeyCode3::g_arkuiKeyCodeMediaPauseFlag },
        { ARKUI_KEYCODE_MEDIA_CLOSE, &ArkUIKeyCode3::g_arkuiKeyCodeMediaCloseFlag },
        { ARKUI_KEYCODE_MEDIA_EJECT, &ArkUIKeyCode3::g_arkuiKeyCodeMediaEjectFlag },
        { ARKUI_KEYCODE_MEDIA_RECORD, &ArkUIKeyCode3::g_arkuiKeyCodeMediaRecordFlag }, { ARKUI_KEYCODE_F1,
        &ArkUIKeyCode3::g_arkuiKeyCodeF1Flag }, { ARKUI_KEYCODE_F2, &ArkUIKeyCode3::g_arkuiKeyCodeF2Flag },
        { ARKUI_KEYCODE_F3, &ArkUIKeyCode3::g_arkuiKeyCodeF3Flag }, { ARKUI_KEYCODE_F4,
        &ArkUIKeyCode3::g_arkuiKeyCodeF4Flag }, { ARKUI_KEYCODE_F5, &ArkUIKeyCode3::g_arkuiKeyCodeF5Flag },
        { ARKUI_KEYCODE_F6, &ArkUIKeyCode3::g_arkuiKeyCodeF6Flag }, { ARKUI_KEYCODE_F7,
        &ArkUIKeyCode3::g_arkuiKeyCodeF7Flag }, { ARKUI_KEYCODE_F8, &ArkUIKeyCode3::g_arkuiKeyCodeF8Flag },
        { ARKUI_KEYCODE_F9, &ArkUIKeyCode3::g_arkuiKeyCodeF9Flag }
    };
    for (size_t i = 0; i < sizeof(map) / sizeof(map[0]); i++)
        if (intValue == map[i].key) {
        *map[i].flag = true;
        break;
    }
    return nullptr;
}
#endif // ARKUI_KEYCODE_TEST3_H