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
#include "common/Common.h"
#include "./ArkUIKeyCode1Test.h"
#include "./ArkUIKeyCode2Test.h"
#include "./ArkUIKeyCode3Test.h"
#include "./ArkUIKeyCode4Test.h"

EXTERN_C_START
static void ArkUIKeycodeInitPush1(napi_property_descriptor* desc)
{
    napi_property_descriptor descTemp[] = {
        XL("GetArkUIKeyCodeEventUnknown", GetArkUIKeyCodeEventUnknown),
        XL("GetArkUIKeyCodeEventFn", GetArkUIKeyCodeEventFn),
        XL("GetArkUIKeyCodeEventVolumeUp", GetArkUIKeyCodeEventVolumeUp),
        XL("GetArkUIKeyCodeEventVolumeDown", GetArkUIKeyCodeEventVolumeDown),
        XL("GetArkUIKeyCodeEventPower", GetArkUIKeyCodeEventPower),
        XL("GetArkUIKeyCodeEventCamera", GetArkUIKeyCodeEventCamera),
        XL("GetArkUIKeyCodeEventVolumeMute", GetArkUIKeyCodeEventVolumeMute),
        XL("GetArkUIKeyCodeEventMute", GetArkUIKeyCodeEventMute),
        XL("GetArkUIKeyCodeEventBrightnessUp", GetArkUIKeyCodeEventBrightnessUp),
        XL("GetArkUIKeyCodeEventBrightnessDown", GetArkUIKeyCodeEventBrightnessDown),
        XL("GetArkUIKeyCodeEvent0", GetArkUIKeyCodeEvent0),
        XL("GetArkUIKeyCodeEvent1", GetArkUIKeyCodeEvent1),
        XL("GetArkUIKeyCodeEvent2", GetArkUIKeyCodeEvent2),
        XL("GetArkUIKeyCodeEvent3", GetArkUIKeyCodeEvent3),
        XL("GetArkUIKeyCodeEvent4", GetArkUIKeyCodeEvent4),
        XL("GetArkUIKeyCodeEvent5", GetArkUIKeyCodeEvent5),
        XL("GetArkUIKeyCodeEvent6", GetArkUIKeyCodeEvent6),
        XL("GetArkUIKeyCodeEvent7", GetArkUIKeyCodeEvent7),
        XL("GetArkUIKeyCodeEvent8", GetArkUIKeyCodeEvent8),
        XL("GetArkUIKeyCodeEvent9", GetArkUIKeyCodeEvent9),
        XL("GetArkUIKeyCodeEventStar", GetArkUIKeyCodeEventStar),
        XL("GetArkUIKeyCodeEventPound", GetArkUIKeyCodeEventPound),
        XL("GetArkUIKeyCodeEventDpadUp", GetArkUIKeyCodeEventDpadUp),
        XL("GetArkUIKeyCodeEventDpadDown", GetArkUIKeyCodeEventDpadDown),
        XL("GetArkUIKeyCodeEventDpadLeft", GetArkUIKeyCodeEventDpadLeft),
        XL("GetArkUIKeyCodeEventDpadRight", GetArkUIKeyCodeEventDpadRight),
        XL("GetArkUIKeyCodeEventDpadCenter", GetArkUIKeyCodeEventDpadCenter),
        XL("GetArkUIKeyCodeEventA", GetArkUIKeyCodeEventA),
        XL("GetArkUIKeyCodeEventB", GetArkUIKeyCodeEventB),
        XL("GetArkUIKeyCodeEventC", GetArkUIKeyCodeEventC),
        XL("GetArkUIKeyCodeEventD", GetArkUIKeyCodeEventD),
        XL("GetArkUIKeyCodeEventE", GetArkUIKeyCodeEventE),
        XL("GetArkUIKeyCodeEventF", GetArkUIKeyCodeEventF),
        XL("GetArkUIKeyCodeEventG", GetArkUIKeyCodeEventG),
        XL("GetArkUIKeyCodeEventH", GetArkUIKeyCodeEventH),
        XL("GetArkUIKeyCodeEventI", GetArkUIKeyCodeEventI),
        XL("InitArkUIKeyMonitor1", InitArkUIKeyMonitor1),
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index] = descTemp[index];
    }
}
static void ArkUIKeycodeInitPush2(napi_property_descriptor* desc)
{
    const uint32_t offset = 37;
    napi_property_descriptor descTemp[] = {
        XL("GetArkUIKeyCodeEventJ", GetArkUIKeyCodeEventJ),
        XL("GetArkUIKeyCodeEventK", GetArkUIKeyCodeEventK),
        XL("GetArkUIKeyCodeEventL", GetArkUIKeyCodeEventL),
        XL("GetArkUIKeyCodeEventM", GetArkUIKeyCodeEventM),
        XL("GetArkUIKeyCodeEventN", GetArkUIKeyCodeEventN),
        XL("GetArkUIKeyCodeEventO", GetArkUIKeyCodeEventO),
        XL("GetArkUIKeyCodeEventP", GetArkUIKeyCodeEventP),
        XL("GetArkUIKeyCodeEventQ", GetArkUIKeyCodeEventQ),
        XL("GetArkUIKeyCodeEventR", GetArkUIKeyCodeEventR),
        XL("GetArkUIKeyCodeEventS", GetArkUIKeyCodeEventS),
        XL("GetArkUIKeyCodeEventT", GetArkUIKeyCodeEventT),
        XL("GetArkUIKeyCodeEventU", GetArkUIKeyCodeEventU),
        XL("GetArkUIKeyCodeEventV", GetArkUIKeyCodeEventV),
        XL("GetArkUIKeyCodeEventW", GetArkUIKeyCodeEventW),
        XL("GetArkUIKeyCodeEventX", GetArkUIKeyCodeEventX),
        XL("GetArkUIKeyCodeEventY", GetArkUIKeyCodeEventY),
        XL("GetArkUIKeyCodeEventZ", GetArkUIKeyCodeEventZ),
        XL("GetArkUIKeyCodeEventComma", GetArkUIKeyCodeEventComma),
        XL("GetArkUIKeyCodeEventPeriod", GetArkUIKeyCodeEventPeriod),
        XL("GetArkUIKeyCodeEventAltLeft", GetArkUIKeyCodeEventAltLeft),
        XL("GetArkUIKeyCodeEventAltRight", GetArkUIKeyCodeEventAltRight),
        XL("GetArkUIKeyCodeEventShiftLeft", GetArkUIKeyCodeEventShiftLeft),
        XL("GetArkUIKeyCodeEventShiftRight", GetArkUIKeyCodeEventShiftRight),
        XL("GetArkUIKeyCodeEventTab", GetArkUIKeyCodeEventTab),
        XL("GetArkUIKeyCodeEventSpace", GetArkUIKeyCodeEventSpace),
        XL("GetArkUIKeyCodeEventSym", GetArkUIKeyCodeEventSym),
        XL("GetArkUIKeyCodeEventExplorer", GetArkUIKeyCodeEventExplorer),
        XL("GetArkUIKeyCodeEventEnvelope", GetArkUIKeyCodeEventEnvelope),
        XL("GetArkUIKeyCodeEventEnter", GetArkUIKeyCodeEventEnter),
        XL("GetArkUIKeyCodeEventDel", GetArkUIKeyCodeEventDel),
        XL("GetArkUIKeyCodeEventGrave", GetArkUIKeyCodeEventGrave),
        XL("GetArkUIKeyCodeEventMinus", GetArkUIKeyCodeEventMinus),
        XL("GetArkUIKeyCodeEventEquals", GetArkUIKeyCodeEventEquals),
        XL("GetArkUIKeyCodeEventLeftBracket", GetArkUIKeyCodeEventLeftBracket),
        XL("GetArkUIKeyCodeEventRightBracket", GetArkUIKeyCodeEventRightBracket),
        XL("GetArkUIKeyCodeEventBackslash", GetArkUIKeyCodeEventBackslash),
        XL("InitArkUIKeyMonitor2", InitArkUIKeyMonitor2)
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}
static void ArkUIKeycodeInitPush3(napi_property_descriptor* desc)
{
    const uint32_t offset = 74;
    napi_property_descriptor descTemp[] = {
        XL("GetArkUIKeyCodeEventSemicolon", GetArkUIKeyCodeEventSemicolon),
        XL("GetArkUIKeyCodeEventApostrophe", GetArkUIKeyCodeEventApostrophe),
        XL("GetArkUIKeyCodeEventSlash", GetArkUIKeyCodeEventSlash),
        XL("GetArkUIKeyCodeEventAt", GetArkUIKeyCodeEventAt),
        XL("GetArkUIKeyCodeEventPlus", GetArkUIKeyCodeEventPlus),
        XL("GetArkUIKeyCodeEventMenu", GetArkUIKeyCodeEventMenu),
        XL("GetArkUIKeyCodeEventPageUp", GetArkUIKeyCodeEventPageUp),
        XL("GetArkUIKeyCodeEventPageDown", GetArkUIKeyCodeEventPageDown),
        XL("GetArkUIKeyCodeEventEscape", GetArkUIKeyCodeEventEscape),
        XL("GetArkUIKeyCodeEventForwardDel", GetArkUIKeyCodeEventForwardDel),
        XL("GetArkUIKeyCodeEventCtrlLeft", GetArkUIKeyCodeEventCtrlLeft),
        XL("GetArkUIKeyCodeEventCtrlRight", GetArkUIKeyCodeEventCtrlRight),
        XL("GetArkUIKeyCodeEventCapsLock", GetArkUIKeyCodeEventCapsLock),
        XL("GetArkUIKeyCodeEventScrollLock", GetArkUIKeyCodeEventScrollLock),
        XL("GetArkUIKeyCodeEventMetaLeft", GetArkUIKeyCodeEventMetaLeft),
        XL("GetArkUIKeyCodeEventMetaRight", GetArkUIKeyCodeEventMetaRight),
        XL("GetArkUIKeyCodeEventFunction", GetArkUIKeyCodeEventFunction),
        XL("GetArkUIKeyCodeEventSysrq", GetArkUIKeyCodeEventSysrq),
        XL("GetArkUIKeyCodeEventBreak", GetArkUIKeyCodeEventBreak),
        XL("GetArkUIKeyCodeEventMoveHome", GetArkUIKeyCodeEventMoveHome),
        XL("GetArkUIKeyCodeEventMoveEnd", GetArkUIKeyCodeEventMoveEnd),
        XL("GetArkUIKeyCodeEventInsert", GetArkUIKeyCodeEventInsert),
        XL("GetArkUIKeyCodeEventForward", GetArkUIKeyCodeEventForward),
        XL("GetArkUIKeyCodeEventMediaPlay", GetArkUIKeyCodeEventMediaPlay),
        XL("GetArkUIKeyCodeEventMediaPause", GetArkUIKeyCodeEventMediaPause),
        XL("GetArkUIKeyCodeEventMediaClose", GetArkUIKeyCodeEventMediaClose),
        XL("GetArkUIKeyCodeEventMediaEject", GetArkUIKeyCodeEventMediaEject),
        XL("GetArkUIKeyCodeEventMediaRecord", GetArkUIKeyCodeEventMediaRecord),
        XL("GetArkUIKeyCodeEventF1", GetArkUIKeyCodeEventF1),
        XL("GetArkUIKeyCodeEventF2", GetArkUIKeyCodeEventF2),
        XL("GetArkUIKeyCodeEventF3", GetArkUIKeyCodeEventF3),
        XL("GetArkUIKeyCodeEventF4", GetArkUIKeyCodeEventF4),
        XL("GetArkUIKeyCodeEventF5", GetArkUIKeyCodeEventF5),
        XL("GetArkUIKeyCodeEventF6", GetArkUIKeyCodeEventF6),
        XL("GetArkUIKeyCodeEventF7", GetArkUIKeyCodeEventF7),
        XL("GetArkUIKeyCodeEventF8", GetArkUIKeyCodeEventF8),
        XL("GetArkUIKeyCodeEventF9", GetArkUIKeyCodeEventF9),
        XL("InitArkUIKeyMonitor3", InitArkUIKeyMonitor3),
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}
static void ArkUIKeycodeInitPush4(napi_property_descriptor* desc)
{
    const uint32_t offset = 112;
    napi_property_descriptor descTemp[] = {
        XL("GetArkUIKeyCodeEventF10", GetArkUIKeyCodeEventF10),
        XL("GetArkUIKeyCodeEventF11", GetArkUIKeyCodeEventF11),
        XL("GetArkUIKeyCodeEventF12", GetArkUIKeyCodeEventF12),
        XL("GetArkUIKeyCodeEventNumLock", GetArkUIKeyCodeEventNumLock),
        XL("GetArkUIKeyCodeEventNumpad0", GetArkUIKeyCodeEventNumpad0),
        XL("GetArkUIKeyCodeEventNumpad1", GetArkUIKeyCodeEventNumpad1),
        XL("GetArkUIKeyCodeEventNumpad2", GetArkUIKeyCodeEventNumpad2),
        XL("GetArkUIKeyCodeEventNumpad3", GetArkUIKeyCodeEventNumpad3),
        XL("GetArkUIKeyCodeEventNumpad4", GetArkUIKeyCodeEventNumpad4),
        XL("GetArkUIKeyCodeEventNumpad5", GetArkUIKeyCodeEventNumpad5),
        XL("GetArkUIKeyCodeEventNumpad6", GetArkUIKeyCodeEventNumpad6),
        XL("GetArkUIKeyCodeEventNumpad7", GetArkUIKeyCodeEventNumpad7),
        XL("GetArkUIKeyCodeEventNumpad8", GetArkUIKeyCodeEventNumpad8),
        XL("GetArkUIKeyCodeEventNumpad9", GetArkUIKeyCodeEventNumpad9),
        XL("GetArkUIKeyCodeEventNumpadDivide", GetArkUIKeyCodeEventNumpadDivide),
        XL("GetArkUIKeyCodeEventNumpadMultiply", GetArkUIKeyCodeEventNumpadMultiply),
        XL("GetArkUIKeyCodeEventNumpadSubtract", GetArkUIKeyCodeEventNumpadSubtract),
        XL("GetArkUIKeyCodeEventNumpadAdd", GetArkUIKeyCodeEventNumpadAdd),
        XL("GetArkUIKeyCodeEventNumpadDot", GetArkUIKeyCodeEventNumpadDot),
        XL("GetArkUIKeyCodeEventNumpadComma", GetArkUIKeyCodeEventNumpadComma),
        XL("GetArkUIKeyCodeEventNumpadEnter", GetArkUIKeyCodeEventNumpadEnter),
        XL("GetArkUIKeyCodeEventNumpadEquals", GetArkUIKeyCodeEventNumpadEquals),
        XL("GetArkUIKeyCodeEventNumpadLeftParen", GetArkUIKeyCodeEventNumpadLeftParen),
        XL("GetArkUIKeyCodeEventNumpadRightParen", GetArkUIKeyCodeEventNumpadRightParen),
        XL("GetArkUIKeyCodeEventButtonA", GetArkUIKeyCodeEventButtonA),
        XL("GetArkUIKeyCodeEventButtonB", GetArkUIKeyCodeEventButtonB),
        XL("GetArkUIKeyCodeEventButtonX", GetArkUIKeyCodeEventButtonX),
        XL("GetArkUIKeyCodeEventButtonY", GetArkUIKeyCodeEventButtonY),
        XL("GetArkUIKeyCodeEventButtonL1", GetArkUIKeyCodeEventButtonL1),
        XL("GetArkUIKeyCodeEventButtonR1", GetArkUIKeyCodeEventButtonR1),
        XL("GetArkUIKeyCodeEventButtonL2", GetArkUIKeyCodeEventButtonL2),
        XL("GetArkUIKeyCodeEventButtonR2", GetArkUIKeyCodeEventButtonR2),
        XL("GetArkUIKeyCodeEventButtonSelect", GetArkUIKeyCodeEventButtonSelect),
        XL("GetArkUIKeyCodeEventButtonStart", GetArkUIKeyCodeEventButtonStart),
        XL("GetArkUIKeyCodeEventButtonMode", GetArkUIKeyCodeEventButtonMode),
        XL("GetArkUIKeyCodeEventButtonThumbl", GetArkUIKeyCodeEventButtonThumbl),
        XL("GetArkUIKeyCodeEventButtonThumbr", GetArkUIKeyCodeEventButtonThumbr),
        XL("InitArkUIKeyMonitor4", InitArkUIKeyMonitor4)
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}
static napi_value ArkUIKeycodeInit(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[330] = { };
    ArkUIKeycodeInitPush1(desc);
    ArkUIKeycodeInitPush2(desc);
    ArkUIKeycodeInitPush3(desc);
    ArkUIKeycodeInitPush4(desc);
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END
static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = ArkUIKeycodeInit,
    .nm_modname = "arkuikeycodendk",
    .nm_priv = ((void *)0),
    .reserved = {0}
};

extern "C" __attribute__((constructor)) void RegisterArkUIKeyCodeModule(void) { napi_module_register(&demoModule); }