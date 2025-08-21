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

#ifndef ARKUI_KEYCODE_TEST1_H
#define ARKUI_KEYCODE_TEST1_H

#include "napi/native_api.h"
#include <arkui/native_key_event.h>
#include <js_native_api_types.h>

namespace ArkUIKeyCode1 {
    bool g_arkuiKeyCodeUnknownFlag = 0;
    bool g_arkuiKeyCodeFnFlag = 0;
    bool g_arkuiKeyCodeVolumeUpFlag = 0;
    bool g_arkuiKeyCodeVolumeDownFlag = 0;
    bool g_arkuiKeyCodePowerFlag = 0;
    bool g_arkuiKeyCodeCameraFlag = 0;
    bool g_arkuiKeyCodeVolumeMuteFlag = 0;
    bool g_arkuiKeyCodeMuteFlag = 0;
    bool g_arkuiKeyCodeBrightnessUpFlag = 0;
    bool g_arkuiKeyCodeBrightnessDownFlag = 0;
    bool g_arkuiKeyCode0Flag = 0;
    bool g_arkuiKeyCode1Flag = 0;
    bool g_arkuiKeyCode2Flag = 0;
    bool g_arkuiKeyCode3Flag = 0;
    bool g_arkuiKeyCode4Flag = 0;
    bool g_arkuiKeyCode5Flag = 0;
    bool g_arkuiKeyCode6Flag = 0;
    bool g_arkuiKeyCode7Flag = 0;
    bool g_arkuiKeyCode8Flag = 0;
    bool g_arkuiKeyCode9Flag = 0;
    bool g_arkuiKeyCodeStarFlag = 0;
    bool g_arkuiKeyCodePoundFlag = 0;
    bool g_arkuiKeyCodeDpadUpFlag = 0;
    bool g_arkuiKeyCodeDpadDownFlag = 0;
    bool g_arkuiKeyCodeDpadLeftFlag = 0;
    bool g_arkuiKeyCodeDpadRightFlag = 0;
    bool g_arkuiKeyCodeDpadCenterFlag = 0;
    bool g_arkuiKeyCodeAFlag = 0;
    bool g_arkuiKeyCodeBFlag = 0;
    bool g_arkuiKeyCodeCFlag = 0;
    bool g_arkuiKeyCodeDFlag = 0;
    bool g_arkuiKeyCodeEFlag = 0;
    bool g_arkuiKeyCodeFFlag = 0;
    bool g_arkuiKeyCodeGFlag = 0;
    bool g_arkuiKeyCodeHFlag = 0;
    bool g_arkuiKeyCodeIFlag = 0;
} // namespace

napi_value GetArkUIKeyCodeEventUnknown(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeUnknownFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeUnknownFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventFn(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeFnFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeFnFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventVolumeUp(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeVolumeUpFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeVolumeUpFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventVolumeDown(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeVolumeDownFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeVolumeDownFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventPower(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodePowerFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodePowerFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventCamera(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeCameraFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeCameraFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventVolumeMute(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeVolumeMuteFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeVolumeMuteFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventMute(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeMuteFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeMuteFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventBrightnessUp(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeBrightnessUpFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeBrightnessUpFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventBrightnessDown(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeBrightnessDownFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeBrightnessDownFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEvent0(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCode0Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCode0Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEvent1(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCode1Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCode1Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEvent2(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCode2Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCode2Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEvent3(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCode3Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCode3Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEvent4(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCode4Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCode4Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEvent5(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCode5Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCode5Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEvent6(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCode6Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCode6Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEvent7(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCode7Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCode7Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEvent8(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCode8Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCode8Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEvent9(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCode9Flag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCode9Flag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventStar(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeStarFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeStarFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventPound(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodePoundFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodePoundFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventDpadUp(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeDpadUpFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeDpadUpFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventDpadDown(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeDpadDownFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeDpadDownFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventDpadLeft(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeDpadLeftFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeDpadLeftFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventDpadRight(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeDpadRightFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeDpadRightFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventDpadCenter(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeDpadCenterFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeDpadCenterFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventA(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeAFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeAFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventB(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeBFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeBFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventC(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeCFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeCFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventD(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeDFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeDFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventE(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeEFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeEFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventF(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeFFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeFFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventG(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeGFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeGFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventH(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeHFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeHFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value GetArkUIKeyCodeEventI(napi_env env, napi_callback_info info)
{
    if (ArkUIKeyCode1::g_arkuiKeyCodeIFlag) {
        napi_value retValue;
        napi_create_int32(env, ArkUIKeyCode1::g_arkuiKeyCodeIFlag, &retValue);
        return retValue;
    }
    return 0;
}
napi_value InitArkUIKeyMonitor1(napi_env env, napi_callback_info info)
{
    size_t argc = 1;
    int32_t intValue;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    napi_get_value_int32(env, args[0], &intValue);
    struct { int key; bool* flag; } map[] = {
        { ARKUI_KEYCODE_UNKNOWN, &ArkUIKeyCode1::g_arkuiKeyCodeUnknownFlag },
        { ARKUI_KEYCODE_FN, &ArkUIKeyCode1::g_arkuiKeyCodeFnFlag },
        { ARKUI_KEYCODE_VOLUME_UP, &ArkUIKeyCode1::g_arkuiKeyCodeVolumeUpFlag },
        { ARKUI_KEYCODE_VOLUME_DOWN, &ArkUIKeyCode1::g_arkuiKeyCodeVolumeDownFlag },
        { ARKUI_KEYCODE_POWER, &ArkUIKeyCode1::g_arkuiKeyCodePowerFlag },
        { ARKUI_KEYCODE_CAMERA, &ArkUIKeyCode1::g_arkuiKeyCodeCameraFlag },
        { ARKUI_KEYCODE_VOLUME_MUTE, &ArkUIKeyCode1::g_arkuiKeyCodeVolumeMuteFlag },
        { ARKUI_KEYCODE_MUTE, &ArkUIKeyCode1::g_arkuiKeyCodeMuteFlag },
        { ARKUI_KEYCODE_BRIGHTNESS_UP, &ArkUIKeyCode1::g_arkuiKeyCodeBrightnessUpFlag },
        { ARKUI_KEYCODE_BRIGHTNESS_DOWN, &ArkUIKeyCode1::g_arkuiKeyCodeBrightnessDownFlag },
        { ARKUI_KEYCODE_0, &ArkUIKeyCode1::g_arkuiKeyCode0Flag },
        { ARKUI_KEYCODE_1, &ArkUIKeyCode1::g_arkuiKeyCode1Flag },
        { ARKUI_KEYCODE_2, &ArkUIKeyCode1::g_arkuiKeyCode2Flag },
        { ARKUI_KEYCODE_3, &ArkUIKeyCode1::g_arkuiKeyCode3Flag }, { ARKUI_KEYCODE_4,
        &ArkUIKeyCode1::g_arkuiKeyCode4Flag }, { ARKUI_KEYCODE_5, &ArkUIKeyCode1::g_arkuiKeyCode5Flag },
        { ARKUI_KEYCODE_6, &ArkUIKeyCode1::g_arkuiKeyCode6Flag }, { ARKUI_KEYCODE_7,
        &ArkUIKeyCode1::g_arkuiKeyCode7Flag }, { ARKUI_KEYCODE_8, &ArkUIKeyCode1::g_arkuiKeyCode8Flag },
        { ARKUI_KEYCODE_9, &ArkUIKeyCode1::g_arkuiKeyCode9Flag },
        { ARKUI_KEYCODE_STAR, &ArkUIKeyCode1::g_arkuiKeyCodeStarFlag },
        { ARKUI_KEYCODE_POUND, &ArkUIKeyCode1::g_arkuiKeyCodePoundFlag },
        { ARKUI_KEYCODE_DPAD_UP, &ArkUIKeyCode1::g_arkuiKeyCodeDpadUpFlag },
        { ARKUI_KEYCODE_DPAD_DOWN, &ArkUIKeyCode1::g_arkuiKeyCodeDpadDownFlag },
        { ARKUI_KEYCODE_DPAD_LEFT, &ArkUIKeyCode1::g_arkuiKeyCodeDpadLeftFlag },
        { ARKUI_KEYCODE_DPAD_RIGHT, &ArkUIKeyCode1::g_arkuiKeyCodeDpadRightFlag },
        { ARKUI_KEYCODE_DPAD_CENTER, &ArkUIKeyCode1::g_arkuiKeyCodeDpadCenterFlag },
        { ARKUI_KEYCODE_A, &ArkUIKeyCode1::g_arkuiKeyCodeAFlag },
        { ARKUI_KEYCODE_B, &ArkUIKeyCode1::g_arkuiKeyCodeBFlag },
        { ARKUI_KEYCODE_C, &ArkUIKeyCode1::g_arkuiKeyCodeCFlag },
        { ARKUI_KEYCODE_D, &ArkUIKeyCode1::g_arkuiKeyCodeDFlag },
        { ARKUI_KEYCODE_E, &ArkUIKeyCode1::g_arkuiKeyCodeEFlag },
        { ARKUI_KEYCODE_F, &ArkUIKeyCode1::g_arkuiKeyCodeFFlag },
        { ARKUI_KEYCODE_G, &ArkUIKeyCode1::g_arkuiKeyCodeGFlag },
        { ARKUI_KEYCODE_H, &ArkUIKeyCode1::g_arkuiKeyCodeHFlag },
        { ARKUI_KEYCODE_I, &ArkUIKeyCode1::g_arkuiKeyCodeIFlag }
    };
    for (size_t i = 0; i < sizeof(map) / sizeof(map[0]); i++)
        if (intValue == map[i].key) {
        *map[i].flag = true;
        break;
    }
    return nullptr;
}
#endif //ARKUI_KEYCODE_TEST1_H
