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
napi_property_descriptor XXL(char* functionName, napi_callback func)
{
    return { functionName, nullptr, func, nullptr, nullptr, nullptr, napi_default, nullptr };
}
EXTERN_C_START
static void ArkUIKeycodeInitPush1(napi_property_descriptor* desc)
{
    napi_property_descriptor descTemp[] = {
        XXL("GetArkUIKeyCodeEventUnknown", GetArkUIKeyCodeEventUnknown),
        XXL("GetArkUIKeyCodeEventFn", GetArkUIKeyCodeEventFn),
        XXL("GetArkUIKeyCodeEventVolumeUp", GetArkUIKeyCodeEventVolumeUp),
        XXL("GetArkUIKeyCodeEventVolumeDown", GetArkUIKeyCodeEventVolumeDown),
        XXL("GetArkUIKeyCodeEventPower", GetArkUIKeyCodeEventPower),
        XXL("GetArkUIKeyCodeEventCamera", GetArkUIKeyCodeEventCamera),
        XXL("GetArkUIKeyCodeEventVolumeMute", GetArkUIKeyCodeEventVolumeMute),
        XXL("GetArkUIKeyCodeEventMute", GetArkUIKeyCodeEventMute),
        XXL("GetArkUIKeyCodeEventBrightnessUp", GetArkUIKeyCodeEventBrightnessUp),
        XXL("GetArkUIKeyCodeEventBrightnessDown", GetArkUIKeyCodeEventBrightnessDown),
        XXL("GetArkUIKeyCodeEvent0", GetArkUIKeyCodeEvent0),
        XXL("GetArkUIKeyCodeEvent1", GetArkUIKeyCodeEvent1),
        XXL("GetArkUIKeyCodeEvent2", GetArkUIKeyCodeEvent2),
        XXL("GetArkUIKeyCodeEvent3", GetArkUIKeyCodeEvent3),
        XXL("GetArkUIKeyCodeEvent4", GetArkUIKeyCodeEvent4),
        XXL("GetArkUIKeyCodeEvent5", GetArkUIKeyCodeEvent5),
        XXL("GetArkUIKeyCodeEvent6", GetArkUIKeyCodeEvent6),
        XXL("GetArkUIKeyCodeEvent7", GetArkUIKeyCodeEvent7),
        XXL("GetArkUIKeyCodeEvent8", GetArkUIKeyCodeEvent8),
        XXL("GetArkUIKeyCodeEvent9", GetArkUIKeyCodeEvent9),
        XXL("GetArkUIKeyCodeEventStar", GetArkUIKeyCodeEventStar),
        XXL("GetArkUIKeyCodeEventPound", GetArkUIKeyCodeEventPound),
        XXL("GetArkUIKeyCodeEventDpadUp", GetArkUIKeyCodeEventDpadUp),
        XXL("GetArkUIKeyCodeEventDpadDown", GetArkUIKeyCodeEventDpadDown),
        XXL("GetArkUIKeyCodeEventDpadLeft", GetArkUIKeyCodeEventDpadLeft),
        XXL("GetArkUIKeyCodeEventDpadRight", GetArkUIKeyCodeEventDpadRight),
        XXL("GetArkUIKeyCodeEventDpadCenter", GetArkUIKeyCodeEventDpadCenter),
        XXL("GetArkUIKeyCodeEventA", GetArkUIKeyCodeEventA),
        XXL("GetArkUIKeyCodeEventB", GetArkUIKeyCodeEventB),
        XXL("GetArkUIKeyCodeEventC", GetArkUIKeyCodeEventC),
        XXL("GetArkUIKeyCodeEventD", GetArkUIKeyCodeEventD),
        XXL("GetArkUIKeyCodeEventE", GetArkUIKeyCodeEventE),
        XXL("GetArkUIKeyCodeEventF", GetArkUIKeyCodeEventF),
        XXL("GetArkUIKeyCodeEventG", GetArkUIKeyCodeEventG),
        XXL("GetArkUIKeyCodeEventH", GetArkUIKeyCodeEventH),
        XXL("GetArkUIKeyCodeEventI", GetArkUIKeyCodeEventI),
        XXL("InitArkUIKeyMonitor1", InitArkUIKeyMonitor1),
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index] = descTemp[index];
    }
}
static void ArkUIKeycodeInitPush2(napi_property_descriptor* desc)
{
    const uint32_t offset = 37;
    napi_property_descriptor descTemp[] = {
        XXL("GetArkUIKeyCodeEventJ", GetArkUIKeyCodeEventJ),
        XXL("GetArkUIKeyCodeEventK", GetArkUIKeyCodeEventK),
        XXL("GetArkUIKeyCodeEventL", GetArkUIKeyCodeEventL),
        XXL("GetArkUIKeyCodeEventM", GetArkUIKeyCodeEventM),
        XXL("GetArkUIKeyCodeEventN", GetArkUIKeyCodeEventN),
        XXL("GetArkUIKeyCodeEventO", GetArkUIKeyCodeEventO),
        XXL("GetArkUIKeyCodeEventP", GetArkUIKeyCodeEventP),
        XXL("GetArkUIKeyCodeEventQ", GetArkUIKeyCodeEventQ),
        XXL("GetArkUIKeyCodeEventR", GetArkUIKeyCodeEventR),
        XXL("GetArkUIKeyCodeEventS", GetArkUIKeyCodeEventS),
        XXL("GetArkUIKeyCodeEventT", GetArkUIKeyCodeEventT),
        XXL("GetArkUIKeyCodeEventU", GetArkUIKeyCodeEventU),
        XXL("GetArkUIKeyCodeEventV", GetArkUIKeyCodeEventV),
        XXL("GetArkUIKeyCodeEventW", GetArkUIKeyCodeEventW),
        XXL("GetArkUIKeyCodeEventX", GetArkUIKeyCodeEventX),
        XXL("GetArkUIKeyCodeEventY", GetArkUIKeyCodeEventY),
        XXL("GetArkUIKeyCodeEventZ", GetArkUIKeyCodeEventZ),
        XXL("GetArkUIKeyCodeEventComma", GetArkUIKeyCodeEventComma),
        XXL("GetArkUIKeyCodeEventPeriod", GetArkUIKeyCodeEventPeriod),
        XXL("GetArkUIKeyCodeEventAltLeft", GetArkUIKeyCodeEventAltLeft),
        XXL("GetArkUIKeyCodeEventAltRight", GetArkUIKeyCodeEventAltRight),
        XXL("GetArkUIKeyCodeEventShiftLeft", GetArkUIKeyCodeEventShiftLeft),
        XXL("GetArkUIKeyCodeEventShiftRight", GetArkUIKeyCodeEventShiftRight),
        XXL("GetArkUIKeyCodeEventTab", GetArkUIKeyCodeEventTab),
        XXL("GetArkUIKeyCodeEventSpace", GetArkUIKeyCodeEventSpace),
        XXL("GetArkUIKeyCodeEventSym", GetArkUIKeyCodeEventSym),
        XXL("GetArkUIKeyCodeEventExplorer", GetArkUIKeyCodeEventExplorer),
        XXL("GetArkUIKeyCodeEventEnvelope", GetArkUIKeyCodeEventEnvelope),
        XXL("GetArkUIKeyCodeEventEnter", GetArkUIKeyCodeEventEnter),
        XXL("GetArkUIKeyCodeEventDel", GetArkUIKeyCodeEventDel),
        XXL("GetArkUIKeyCodeEventGrave", GetArkUIKeyCodeEventGrave),
        XXL("GetArkUIKeyCodeEventMinus", GetArkUIKeyCodeEventMinus),
        XXL("GetArkUIKeyCodeEventEquals", GetArkUIKeyCodeEventEquals),
        XXL("GetArkUIKeyCodeEventLeftBracket", GetArkUIKeyCodeEventLeftBracket),
        XXL("GetArkUIKeyCodeEventRightBracket", GetArkUIKeyCodeEventRightBracket),
        XXL("GetArkUIKeyCodeEventBackslash", GetArkUIKeyCodeEventBackslash),
        XXL("InitArkUIKeyMonitor2", InitArkUIKeyMonitor2)
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}
static void ArkUIKeycodeInitPush3(napi_property_descriptor* desc)
{
    const uint32_t offset = 74;
    napi_property_descriptor descTemp[] = {
        XXL("GetArkUIKeyCodeEventSemicolon", GetArkUIKeyCodeEventSemicolon),
        XXL("GetArkUIKeyCodeEventApostrophe", GetArkUIKeyCodeEventApostrophe),
        XXL("GetArkUIKeyCodeEventSlash", GetArkUIKeyCodeEventSlash),
        XXL("GetArkUIKeyCodeEventAt", GetArkUIKeyCodeEventAt),
        XXL("GetArkUIKeyCodeEventPlus", GetArkUIKeyCodeEventPlus),
        XXL("GetArkUIKeyCodeEventMenu", GetArkUIKeyCodeEventMenu),
        XXL("GetArkUIKeyCodeEventPageUp", GetArkUIKeyCodeEventPageUp),
        XXL("GetArkUIKeyCodeEventPageDown", GetArkUIKeyCodeEventPageDown),
        XXL("GetArkUIKeyCodeEventEscape", GetArkUIKeyCodeEventEscape),
        XXL("GetArkUIKeyCodeEventForwardDel", GetArkUIKeyCodeEventForwardDel),
        XXL("GetArkUIKeyCodeEventCtrlLeft", GetArkUIKeyCodeEventCtrlLeft),
        XXL("GetArkUIKeyCodeEventCtrlRight", GetArkUIKeyCodeEventCtrlRight),
        XXL("GetArkUIKeyCodeEventCapsLock", GetArkUIKeyCodeEventCapsLock),
        XXL("GetArkUIKeyCodeEventScrollLock", GetArkUIKeyCodeEventScrollLock),
        XXL("GetArkUIKeyCodeEventMetaLeft", GetArkUIKeyCodeEventMetaLeft),
        XXL("GetArkUIKeyCodeEventMetaRight", GetArkUIKeyCodeEventMetaRight),
        XXL("GetArkUIKeyCodeEventFunction", GetArkUIKeyCodeEventFunction),
        XXL("GetArkUIKeyCodeEventSysrq", GetArkUIKeyCodeEventSysrq),
        XXL("GetArkUIKeyCodeEventBreak", GetArkUIKeyCodeEventBreak),
        XXL("GetArkUIKeyCodeEventMoveHome", GetArkUIKeyCodeEventMoveHome),
        XXL("GetArkUIKeyCodeEventMoveEnd", GetArkUIKeyCodeEventMoveEnd),
        XXL("GetArkUIKeyCodeEventInsert", GetArkUIKeyCodeEventInsert),
        XXL("GetArkUIKeyCodeEventForward", GetArkUIKeyCodeEventForward),
        XXL("GetArkUIKeyCodeEventMediaPlay", GetArkUIKeyCodeEventMediaPlay),
        XXL("GetArkUIKeyCodeEventMediaPause", GetArkUIKeyCodeEventMediaPause),
        XXL("GetArkUIKeyCodeEventMediaClose", GetArkUIKeyCodeEventMediaClose),
        XXL("GetArkUIKeyCodeEventMediaEject", GetArkUIKeyCodeEventMediaEject),
        XXL("GetArkUIKeyCodeEventMediaRecord", GetArkUIKeyCodeEventMediaRecord),
        XXL("GetArkUIKeyCodeEventF1", GetArkUIKeyCodeEventF1),
        XXL("GetArkUIKeyCodeEventF2", GetArkUIKeyCodeEventF2),
        XXL("GetArkUIKeyCodeEventF3", GetArkUIKeyCodeEventF3),
        XXL("GetArkUIKeyCodeEventF4", GetArkUIKeyCodeEventF4),
        XXL("GetArkUIKeyCodeEventF5", GetArkUIKeyCodeEventF5),
        XXL("GetArkUIKeyCodeEventF6", GetArkUIKeyCodeEventF6),
        XXL("GetArkUIKeyCodeEventF7", GetArkUIKeyCodeEventF7),
        XXL("GetArkUIKeyCodeEventF8", GetArkUIKeyCodeEventF8),
        XXL("GetArkUIKeyCodeEventF9", GetArkUIKeyCodeEventF9),
        XXL("InitArkUIKeyMonitor3", InitArkUIKeyMonitor3),
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + offset] = descTemp[index];
    }
}
static void ArkUIKeycodeInitPush4(napi_property_descriptor* desc)
{
    const uint32_t offset = 112;
    napi_property_descriptor descTemp[] = {
        XXL("GetArkUIKeyCodeEventF10", GetArkUIKeyCodeEventF10),
        XXL("GetArkUIKeyCodeEventF11", GetArkUIKeyCodeEventF11),
        XXL("GetArkUIKeyCodeEventF12", GetArkUIKeyCodeEventF12),
        XXL("GetArkUIKeyCodeEventNumLock", GetArkUIKeyCodeEventNumLock),
        XXL("GetArkUIKeyCodeEventNumpad0", GetArkUIKeyCodeEventNumpad0),
        XXL("GetArkUIKeyCodeEventNumpad1", GetArkUIKeyCodeEventNumpad1),
        XXL("GetArkUIKeyCodeEventNumpad2", GetArkUIKeyCodeEventNumpad2),
        XXL("GetArkUIKeyCodeEventNumpad3", GetArkUIKeyCodeEventNumpad3),
        XXL("GetArkUIKeyCodeEventNumpad4", GetArkUIKeyCodeEventNumpad4),
        XXL("GetArkUIKeyCodeEventNumpad5", GetArkUIKeyCodeEventNumpad5),
        XXL("GetArkUIKeyCodeEventNumpad6", GetArkUIKeyCodeEventNumpad6),
        XXL("GetArkUIKeyCodeEventNumpad7", GetArkUIKeyCodeEventNumpad7),
        XXL("GetArkUIKeyCodeEventNumpad8", GetArkUIKeyCodeEventNumpad8),
        XXL("GetArkUIKeyCodeEventNumpad9", GetArkUIKeyCodeEventNumpad9),
        XXL("GetArkUIKeyCodeEventNumpadDivide", GetArkUIKeyCodeEventNumpadDivide),
        XXL("GetArkUIKeyCodeEventNumpadMultiply", GetArkUIKeyCodeEventNumpadMultiply),
        XXL("GetArkUIKeyCodeEventNumpadSubtract", GetArkUIKeyCodeEventNumpadSubtract),
        XXL("GetArkUIKeyCodeEventNumpadAdd", GetArkUIKeyCodeEventNumpadAdd),
        XXL("GetArkUIKeyCodeEventNumpadDot", GetArkUIKeyCodeEventNumpadDot),
        XXL("GetArkUIKeyCodeEventNumpadComma", GetArkUIKeyCodeEventNumpadComma),
        XXL("GetArkUIKeyCodeEventNumpadEnter", GetArkUIKeyCodeEventNumpadEnter),
        XXL("GetArkUIKeyCodeEventNumpadEquals", GetArkUIKeyCodeEventNumpadEquals),
        XXL("GetArkUIKeyCodeEventNumpadLeftParen", GetArkUIKeyCodeEventNumpadLeftParen),
        XXL("GetArkUIKeyCodeEventNumpadRightParen", GetArkUIKeyCodeEventNumpadRightParen),
        XXL("GetArkUIKeyCodeEventButtonA", GetArkUIKeyCodeEventButtonA),
        XXL("GetArkUIKeyCodeEventButtonB", GetArkUIKeyCodeEventButtonB),
        XXL("GetArkUIKeyCodeEventButtonX", GetArkUIKeyCodeEventButtonX),
        XXL("GetArkUIKeyCodeEventButtonY", GetArkUIKeyCodeEventButtonY),
        XXL("GetArkUIKeyCodeEventButtonL1", GetArkUIKeyCodeEventButtonL1),
        XXL("GetArkUIKeyCodeEventButtonR1", GetArkUIKeyCodeEventButtonR1),
        XXL("GetArkUIKeyCodeEventButtonL2", GetArkUIKeyCodeEventButtonL2),
        XXL("GetArkUIKeyCodeEventButtonR2", GetArkUIKeyCodeEventButtonR2),
        XXL("GetArkUIKeyCodeEventButtonSelect", GetArkUIKeyCodeEventButtonSelect),
        XXL("GetArkUIKeyCodeEventButtonStart", GetArkUIKeyCodeEventButtonStart),
        XXL("GetArkUIKeyCodeEventButtonMode", GetArkUIKeyCodeEventButtonMode),
        XXL("GetArkUIKeyCodeEventButtonThumbl", GetArkUIKeyCodeEventButtonThumbl),
        XXL("GetArkUIKeyCodeEventButtonThumbr", GetArkUIKeyCodeEventButtonThumbr),
        XXL("InitArkUIKeyMonitor4", InitArkUIKeyMonitor4)
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