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

#include <hilog/log.h>

#include "mouse/xcomponentmouse.h"
#include "touch/touchpoint.h"
#include "node/nodeattribute.h"

namespace NativeXComponentTest {
EXTERN_C_START

static void InitPushTouchPoint(napi_property_descriptor* desc)
{
    napi_property_descriptor descTemp[] = {
        XL("CreateTouchNativeNode", TouchPoint::CreateTouchNativeNode),
        XL("GetTouchPointWindowX", TouchPoint::GetTouchPointWindowX),
        XL("GetTouchPointWindowY", TouchPoint::GetTouchPointWindowY),
        XL("GetTouchPointDisplayX", TouchPoint::GetTouchPointDisplayX),
        XL("GetTouchPointDisplayY", TouchPoint::GetTouchPointDisplayY),
        XL("GetHistoricalPoints", TouchPoint::GetHistoricalPoints),
        XL("HistoricalPointId", TouchPoint::HistoricalPointId),
        XL("HistoricalPointScreenX", TouchPoint::HistoricalPointScreenX),
        XL("HistoricalPointScreenY", TouchPoint::HistoricalPointScreenY),
        XL("HistoricalPointX", TouchPoint::HistoricalPointX),
        XL("HistoricalPointY", TouchPoint::HistoricalPointY),
        XL("HistoricalPointType", TouchPoint::HistoricalPointType),
        XL("HistoricalPointSize", TouchPoint::HistoricalPointSize),
        XL("HistoricalPointForce", TouchPoint::HistoricalPointForce),
        XL("HistoricalPointTimeStamp", TouchPoint::HistoricalPointTimeStamp),
        XL("HistoricalPointTitlX", TouchPoint::HistoricalPointTitlX),
        XL("HistoricalPointTitlY", TouchPoint::HistoricalPointTitlY),
        XL("HistoricalPointSourceTool", TouchPoint::HistoricalPointSourceTool),
        XL("GetTouchEventSourceTool", TouchPoint::GetTouchEventSourceTool),
        XL("GetTouchPointToolTypeResult", TouchPoint::GetTouchPointToolTypeResult),
        XL("TouchPointId",TouchPoint::TouchPointId),
        XL("TouchPointSize",TouchPoint::TouchPointSize),
        XL("TouchPointForce",TouchPoint::TouchPointForce),
        XL("TouchPointTimeStamp",TouchPoint::TouchPointTimeStamp),
        XL("TouchPointIsPressed",TouchPoint::TouchPointIsPressed),
        XL("TouchEventId",TouchPoint::TouchEventId),
        XL("TouchEventSize",TouchPoint::TouchEventSize),
        XL("TouchEventForce",TouchPoint::TouchEventForce),
        XL("TouchEventDeviceId",TouchPoint::TouchEventDeviceId),
        XL("TouchEventTimeStamp",TouchPoint::TouchEventTimeStamp),
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + 41] = descTemp[index];
    }
}

static void InitPushXComponentMouse(napi_property_descriptor* desc)
{
    napi_property_descriptor descTemp[] = {
        XL("CreateMouseNativeNode", XComponentMouse::CreateMouseNativeNode),
        XL("MouseEventAction", XComponentMouse::MouseEventAction),
        XL("MouseEventButton", XComponentMouse::MouseEventButton),
        XL("MouseEventTimeStamp", XComponentMouse::MouseEventTimeStamp),
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + 71] = descTemp[index];
    }
}

static void InitPushNodeAttribute(napi_property_descriptor* desc)
{
    napi_property_descriptor descTemp[] = {
        XL("CreateAttributeNativeNode", NodeAttribute::CreateAttributeNativeNode),
        XL("CheckNodeTextHalfLeading", NodeAttribute::CheckNodeTextHalfLeading),
        XL("CheckNodeTextLineCount", NodeAttribute::CheckNodeTextLineCount),
        XL("CheckNodeTextOptimizeTrailingSpace", NodeAttribute::CheckNodeTextOptimizeTrailingSpace),
        XL("CheckNodeTextLinearGradient", NodeAttribute::CheckNodeTextLinearGradient),
        XL("CheckNodeTextRadialGradient", NodeAttribute::CheckNodeTextRadialGradient),
        XL("CheckNodeTextVerticalAlign", NodeAttribute::CheckNodeTextVerticalAlign),
        XL("CheckNodeTextInputHalfLeading", NodeAttribute::CheckNodeTextInputHalfLeading),
        XL("CheckNodeTextInputKeyboardAppearance", NodeAttribute::CheckNodeTextInputKeyboardAppearance),
        XL("CheckNodeTextInputEnableFillAnimation", NodeAttribute::CheckNodeTextInputEnableFillAnimation),
        XL("CheckNodeTextInputLineHeight", NodeAttribute::CheckNodeTextInputLineHeight),
        XL("CheckNodeTextAreaInputFilter", NodeAttribute::CheckNodeTextAreaInputFilter),
        XL("CheckNodeTextAreaContentType", NodeAttribute::CheckNodeTextAreaContentType),
        XL("CheckNodeTextAreaHalfLeading", NodeAttribute::CheckNodeTextAreaHalfLeading),
        XL("CheckNodeTextAreaKeyboardAppearance", NodeAttribute::CheckNodeTextAreaKeyboardAppearance),
        XL("CheckNodeTextAreaMaxLines", NodeAttribute::CheckNodeTextAreaMaxLines),
        XL("CheckNodeTextAreaLineSpacing", NodeAttribute::CheckNodeTextAreaLineSpacing),
        XL("CheckNodeTextAreaMinLines", NodeAttribute::CheckNodeTextAreaMinLines),
        XL("CheckNodeTextAreaMaxLinesWithScroll", NodeAttribute::CheckNodeTextAreaMaxLinesWithScroll),
        XL("CheckNodeTextAreaLineHeight", NodeAttribute::CheckNodeTextAreaLineHeight),
        XL("CheckNodeTextPickSelectedBackgroundStyle", NodeAttribute::CheckNodeTextPickSelectedBackgroundStyle),
    };
    for (size_t index = 0; index < sizeof(descTemp) / sizeof(descTemp[0]); index++) {
        desc[index + 75] = descTemp[index];
    }
}

static napi_value Init(napi_env env, napi_value exports)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "Init","Init begins");
    if ((env == nullptr) || (exports == nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "Init","env or exports is null");
        return nullptr;
    }

    napi_property_descriptor desc[96] = {
        XL("XComponentCreate",ArkUICapiTest::CreateNativeNode),
        XL("ChooseTestFunction",ArkUICapiTest::ChooseTestFunction),
        XL("GetKeyEventAction",ArkUICapiTest::GetKeyEventAction),
        XL("GetKeyEventActionEnum",ArkUICapiTest::GetKeyEventActionEnum),
        XL("GetKeyEventSourceType",ArkUICapiTest::GetKeyEventSourceType),
        XL("GetKeyEventDeviceId",ArkUICapiTest::GetKeyEventDeviceId),
        XL("GetKeyEventTimestamp",ArkUICapiTest::GetKeyEventTimestamp),
        XL("GetKeyEventModifierKeyStates",ArkUICapiTest::GetKeyEventModifierKeyStates),
        XL("GetKeyEventNumLockState",ArkUICapiTest::GetKeyEventNumLockState),
        XL("GetKeyEventCapsLockState",ArkUICapiTest::GetKeyEventCapsLockState),
        XL("GetKeyEventScrollLockState",ArkUICapiTest::GetKeyEventScrollLockState),
        XL("AttachNativeRootNode001",ArkUICapiTest::AttachNativeRootNode001),
        XL("AttachNativeRootNode002",ArkUICapiTest::AttachNativeRootNode002),
        XL("DetachNativeRootNode001",ArkUICapiTest::DetachNativeRootNode001),
        XL("DetachNativeRootNode002",ArkUICapiTest::DetachNativeRootNode002),
        XL("SetExpectedFrameRateRange001",ArkUICapiTest::SetExpectedFrameRateRange001),
        XL("SetExpectedFrameRateRange002",ArkUICapiTest::SetExpectedFrameRateRange002),
        XL("RegisterOnFrameCallback001",ArkUICapiTest::RegisterOnFrameCallback001),
        XL("RegisterOnFrameCallback002",ArkUICapiTest::RegisterOnFrameCallback002),
        XL("UnregisterOnFrameCallback001",ArkUICapiTest::UnregisterOnFrameCallback001),
        XL("UnregisterOnFrameCallback002",ArkUICapiTest::UnregisterOnFrameCallback002),
        XL("RegisterUIInputEventCallback001",ArkUICapiTest::RegisterUIInputEventCallback001),
        XL("RegisterUIInputEventCallback002",ArkUICapiTest::RegisterUIInputEventCallback002),
        XL("RegisterOnTouchInterceptCallback001",ArkUICapiTest::RegisterOnTouchInterceptCallback001),
        XL("RegisterOnTouchInterceptCallback002",ArkUICapiTest::RegisterOnTouchInterceptCallback002),
        XL("GetNativeAccessibilityProvider001",ArkUICapiTest::GetNativeAccessibilityProvider001),
        XL("GetNativeAccessibilityProvider002",ArkUICapiTest::GetNativeAccessibilityProvider002),
        XL("RegisterKeyEventCallbackWithResult001",ArkUICapiTest::RegisterKeyEventCallbackWithResult001),
        XL("RegisterKeyEventCallbackWithResult002",ArkUICapiTest::RegisterKeyEventCallbackWithResult002),
        XL("XComponentInitialize001",ArkUICapiTest::XComponentInitialize001),
        XL("XComponentFinalize001",ArkUICapiTest::XComponentFinalize001),
        XL("XComponentSetNeedSoftKeyboard001",ArkUICapiTest::XComponentSetNeedSoftKeyboard001),
        XL("XComponentSetNeedSoftKeyboard002",ArkUICapiTest::XComponentSetNeedSoftKeyboard002),
        XL("GetExtraMouseEventInfo001",ArkUICapiTest::GetExtraMouseEventInfo001),
        XL("GetExtraMouseEventInfo002",ArkUICapiTest::GetExtraMouseEventInfo002),
        XL("GetMouseEventModifierKeyStates001",ArkUICapiTest::GetMouseEventModifierKeyStates001),
        XL("GetMouseEventModifierKeyStates002",ArkUICapiTest::GetMouseEventModifierKeyStates002),
        XL("RegisterFocusEventCallback001",ArkUICapiTest::RegisterFocusEventCallback001),
        XL("RegisterFocusEventCallback002",ArkUICapiTest::RegisterFocusEventCallback002),
        XL("RegisterBlurEventCallback001",ArkUICapiTest::RegisterBlurEventCallback001),
        XL("RegisterBlurEventCallback002",ArkUICapiTest::RegisterBlurEventCallback002),
    };
    InitPushTouchPoint(desc);
    InitPushXComponentMouse(desc);
    InitPushNodeAttribute(desc);
    if (napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "Init", "napi_define_properties failed");
        return nullptr;
    }
    
    return exports;
}
EXTERN_C_END

static napi_module nativexcomponentModule = { .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "nativexcomponent",
    .nm_priv = ((void*)0),
    .reserved = { 0 } };

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&nativexcomponentModule);
}
} // namespace NativeXComponentTest
