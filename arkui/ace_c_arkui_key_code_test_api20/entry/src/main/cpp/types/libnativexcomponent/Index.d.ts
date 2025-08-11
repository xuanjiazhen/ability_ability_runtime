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

export const XComponentCreate: (Node: Content) => void;
export const GetKeyEventAction: () => number;
export const GetKeyEventActionEnum: () => number;
export const GetKeyEventSourceType: () => number;
export const GetKeyEventDeviceId: () => number;
export const GetKeyEventTimestamp: () => number;
export const GetKeyEventModifierKeyStates: () => number;
export const GetKeyEventNumLockState: () => number;
export const GetKeyEventCapsLockState: () => number;
export const GetKeyEventScrollLockState: () => number;
export const AttachNativeRootNode001: () => number;
export const AttachNativeRootNode002: () => number;
export const DetachNativeRootNode001: () => number;
export const DetachNativeRootNode002: () => number;
export const SetExpectedFrameRateRange001: () => number;
export const SetExpectedFrameRateRange002: () => number;
export const SetArkUIExpectedFrameRateRange001: () => number;
export const SetArkUIExpectedFrameRateRange002: () => number;
export const RegisterOnFrameCallback001: () => number;
export const RegisterOnFrameCallback002: () => number;
export const ArkUIRegisterOnFrameCallback001: () => number;
export const ArkUIRegisterOnFrameCallback002: () => number;
export const UnregisterOnFrameCallback001: () => number;
export const UnregisterOnFrameCallback002: () => number;
export const ArkUIUnregisterOnFrameCallback001: () => number;
export const ArkUIUnregisterOnFrameCallback002: () => number;
export const RegisterUIInputEventCallback001: () => number;
export const RegisterUIInputEventCallback002: () => number;
export const RegisterOnTouchInterceptCallback001: () => number;
export const RegisterOnTouchInterceptCallback002: () => number;
export const GetNativeAccessibilityProvider001: () => number;
export const GetNativeAccessibilityProvider002: () => number;
export const RegisterKeyEventCallbackWithResult001: () => number;
export const RegisterKeyEventCallbackWithResult002: () => number;
export const XComponentInitialize001: () => number;
export const XComponentFinalize001: () => number;
export const XComponentSetNeedSoftKeyboard001: () => number;
export const XComponentSetNeedSoftKeyboard002: () => number;
export const GetExtraMouseEventInfo001: () => number;
export const GetExtraMouseEventInfo002: () => number;
export const GetMouseEventModifierKeyStates001: () => number;
export const GetMouseEventModifierKeyStates002: () => number;
export const RegisterFocusEventCallback001: () => number;
export const RegisterFocusEventCallback002: () => number;
export const RegisterBlurEventCallback001: () => number;
export const RegisterBlurEventCallback002: () => number;
export const GetEventSourceType: () => number;
export const ChooseTestFunction: (index: number) => void;

export const CreateTouchNativeNode: (Node: Content) => void;
export const GetTouchPointWindowX: (isNullptr: boolean) => number;
export const GetTouchPointWindowY: (isNullptr: boolean) => number;
export const GetTouchPointDisplayX: (isNullptr: boolean) => number;
export const GetTouchPointDisplayY: (isNullptr: boolean) => number;
export const GetHistoricalPoints: () => number;
export const HistoricalPointId: () => number;
export const HistoricalPointScreenX: () => number;
export const HistoricalPointScreenY: () => number;
export const HistoricalPointX: () => number;
export const HistoricalPointY: () => number;
export const HistoricalPointType: () => number;
export const HistoricalPointSize: () => number;
export const HistoricalPointForce: () => number;
export const HistoricalPointTimeStamp: () => number;
export const HistoricalPointTitlX: () => number;
export const HistoricalPointTitlY: () => number;
export const HistoricalPointSourceTool: () => number;
export const GetTouchEventSourceTool: (Index: number) => number;
export const GetTouchPointToolTypeResult: (Index: number) => number;
export const GetTouchEventSourceType: (Index: number) => number;
export const TouchPointId: () => number;
export const TouchPointSize: () => number;
export const TouchPointForce: () => number;
export const TouchPointTimeStamp: () => number;
export const TouchPointIsPressed: () => number;
export const TouchEventId: () => number;
export const TouchEventSize: () => number;
export const TouchEventForce: () => number;
export const TouchEventDeviceId: () => number;
export const TouchEventTimeStamp: () => number;

export const CreateMouseNativeNode: (Node: Content) => void;
export const MouseEventAction: (index: number) => number;
export const MouseEventButton: (index: number) => number;
export const MouseEventTimeStamp: () => number;

export const CreateAttributeNativeNode: (Node: Content) => void;
export const CheckNodeTextHalfLeading: () => number;
export const CheckNodeTextLineCount: () => number;
export const CheckNodeTextOptimizeTrailingSpace: () => number;
export const CheckNodeTextLinearGradient: () => number;
export const CheckNodeTextRadialGradient: () => number;
export const CheckNodeTextVerticalAlign: () => number;
export const CheckNodeTextInputHalfLeading: () => number;
export const CheckNodeTextInputKeyboardAppearance: () => number;
export const CheckNodeTextInputEnableFillAnimation: () => number;
export const CheckNodeTextInputLineHeight: () => number;
export const CheckNodeTextAreaInputFilter: () => number;
export const CheckNodeTextAreaContentType: () => number;
export const CheckNodeTextAreaHalfLeading: () => number;
export const CheckNodeTextAreaKeyboardAppearance: () => number;
export const CheckNodeTextAreaMaxLines: () => number;
export const CheckNodeTextAreaLineSpacing: () => number;
export const CheckNodeTextAreaMinLines: () => number;
export const CheckNodeTextAreaMaxLinesWithScroll: () => number;
export const CheckNodeTextAreaLineHeight: () => number;
export const CheckNodeTextPickSelectedBackgroundStyle: () => number;