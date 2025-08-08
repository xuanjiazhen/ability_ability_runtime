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

#include "xcomponent.h"

namespace ArkUICapiTest {

napi_value ChooseTestFunction(napi_env env, napi_callback_info info)
{
    if ((env == nullptr) || (info == nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "PluginManager", "CreateNativeNode env or info is null");
        return nullptr;
    }
    size_t argCnt = 1;
    napi_value args[1] = { nullptr };
    if (napi_get_cb_info(env, info, &argCnt, args, nullptr, nullptr) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "PluginManager", "CreateNativeNode napi_get_cb_info failed");
    }
    if (argCnt != PARAM_1) {
        napi_throw_type_error(env, NULL, "Wrong number of arguments");
        return nullptr;
    }
    napi_get_value_int32(env, args[PARAM_0], &g_testFunctionIndex);
    return nullptr;
}

napi_value GetKeyEventAction(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = -2;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventAction");
    if (g_action == OH_NATIVEXCOMPONENT_KEY_ACTION_UNKNOWN) {
        result = -1;
    } else if (g_action == OH_NATIVEXCOMPONENT_KEY_ACTION_DOWN) {
        result = 0;
    } else if (g_action == OH_NATIVEXCOMPONENT_KEY_ACTION_UP) {
        result = 1;
    }
    g_action = OH_NATIVEXCOMPONENT_KEY_ACTION_UNKNOWN;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetKeyEventActionEnum(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventAction");
    napi_create_int32(env, g_action_result, &retValue);
    g_action_result = 0;
    return retValue;
}

napi_value GetKeyEventSourceType(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = -2;
    if (g_eventSourceType == OH_NATIVEXCOMPONENT_SOURCE_TYPE_UNKNOWN) {
        result = 0;
    } else if (g_eventSourceType == OH_NATIVEXCOMPONENT_TOOL_TYPE_FINGER) {
        result = 1;
    } else if (g_eventSourceType == OH_NATIVEXCOMPONENT_TOOL_TYPE_PEN) {
        result = 2;
    } else if (g_eventSourceType == OH_NATIVEXCOMPONENT_TOOL_TYPE_RUBBER) {
        result = 3;
    } else if (g_eventSourceType == OH_NATIVEXCOMPONENT_TOOL_TYPE_BRUSH) {
        result = 4;
    } else if (g_eventSourceType == OH_NATIVEXCOMPONENT_TOOL_TYPE_PENCIL) {
        result = 5;
    } else if (g_eventSourceType == OH_NATIVEXCOMPONENT_TOOL_TYPE_AIRBRUSH) {
        result = 6;
    } else if (g_eventSourceType == OH_NATIVEXCOMPONENT_TOOL_TYPE_MOUSE) {
        result = 7;
    } else if (g_eventSourceType == OH_NATIVEXCOMPONENT_TOOL_TYPE_LENS) {
        result = 8;
    }
    g_eventSourceType = OH_NATIVEXCOMPONENT_SOURCE_TYPE_UNKNOWN;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetKeyEventDeviceId(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int64_t result = -2;
    if (g_deviceId == -2) {
        result = 0;
    } else {
        result = 1;
    }
    g_deviceId = -2;
    napi_create_int64(env, result, &retValue);
    return retValue;
}

napi_value GetKeyEventTimestamp(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int64_t result = -2;
    if (g_timestamp == -2) {
        result = 0;
    } else {
        result = 1;
    }
    g_timestamp = -2;
    napi_create_int64(env, result, &retValue);
    return retValue;
}

napi_value GetKeyEventModifierKeyStates(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    napi_create_int32(env, g_result, &retValue);
    return retValue;
}

napi_value GetKeyEventNumLockState(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    napi_create_int32(env, g_result, &retValue);
    return retValue;
}
napi_value GetKeyEventCapsLockState(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    napi_create_int32(env, g_result, &retValue);
    return retValue;
}
napi_value GetKeyEventScrollLockState(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    napi_create_int32(env, g_result, &retValue);
    return retValue;
}

napi_value AttachNativeRootNode001(napi_env env, napi_callback_info info)
{
    Column.SetBackgroundColor(0xffff0000);
    Column.SetWidth(ColumnWidthAndHeight);
    Column.SetHeight(ColumnWidthAndHeight);
    ArkUI_NativeNodeAPI_1* _nodeAPI;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, _nodeAPI);
    _nodeAPI->addChild(ChildXComponent->GetComponent(), Column.GetComponent());
    napi_value retValue = 0;
    int32_t result = 401;
    result = OH_NativeXComponent_AttachNativeRootNode(ChildXComponent->GetXComponent(), Column.GetComponent());
    OH_NativeXComponent_DetachNativeRootNode(ChildXComponent->GetXComponent(), Column.GetComponent());
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value AttachNativeRootNode002(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    if (OH_NativeXComponent_AttachNativeRootNode(g_xComponent.GetXComponent(), nullptr) ==
        OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER) {
        result = 401;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value DetachNativeRootNode001(napi_env env, napi_callback_info info)
{
    Column.SetBackgroundColor(0xffff0000);
    Column.SetWidth(210);
    Column.SetHeight(210);
    ArkUI_NativeNodeAPI_1* _nodeAPI;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, _nodeAPI);
    _nodeAPI->addChild(ChildXComponent->GetComponent(), Column.GetComponent());
    napi_value retValue = 0;
    int32_t result = 401;
    OH_NativeXComponent_AttachNativeRootNode(ChildXComponent->GetXComponent(), Column.GetComponent());
    result = OH_NativeXComponent_DetachNativeRootNode(ChildXComponent->GetXComponent(), Column.GetComponent());
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value DetachNativeRootNode002(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    if (OH_NativeXComponent_DetachNativeRootNode(g_xComponent.GetXComponent(), nullptr) ==
        OH_NATIVEXCOMPONENT_RESULT_BAD_PARAMETER) {
        result = 401;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value SetExpectedFrameRateRange001(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    OH_NativeXComponent_ExpectedRateRange ExpectedRateRange = { .min = 30, .max = 60, .expected = 45 };
    int32_t result = 0;
    result = OH_NativeXComponent_SetExpectedFrameRateRange(ChildXComponent->GetXComponent(), &ExpectedRateRange);
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value SetExpectedFrameRateRange002(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    OH_NativeXComponent_ExpectedRateRange ExpectedRateRange = { .min = 30, .max = 60, .expected = 45 };
    if (OH_NativeXComponent_SetExpectedFrameRateRange(nullptr, &ExpectedRateRange) == 0) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    if (OH_NativeXComponent_SetExpectedFrameRateRange(ChildXComponent->GetXComponent(), nullptr) == 0) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 401, &retValue);
    return retValue;
}

napi_value RegisterOnFrameCallback001(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 401;
    result = OH_NativeXComponent_RegisterOnFrameCallback(ChildXComponent->GetXComponent(), OnFrameCallback);
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterOnFrameCallback002(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    result = OH_NativeXComponent_RegisterOnFrameCallback(nullptr, OnFrameCallback) == -2 ? 401 : 0;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

void OnFrameCallback(OH_NativeXComponent* component, uint64_t timestamp, uint64_t targetTimestamp)
{
    return;
}

napi_value UnregisterOnFrameCallback001(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 401;
    result = OH_NativeXComponent_UnregisterOnFrameCallback(ChildXComponent->GetXComponent());
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value UnregisterOnFrameCallback002(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    result = OH_NativeXComponent_UnregisterOnFrameCallback(nullptr) == -2 ? 401 : 0;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterUIInputEventCallback001(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    OH_NativeXComponent_ExpectedRateRange ExpectedRateRange = { .min = 30, .max = 60, .expected = 45 };
    int32_t result = 401;
    result = OH_NativeXComponent_RegisterUIInputEventCallback(ChildXComponent->GetXComponent(), UIInputEventCallback, ARKUI_UIINPUTEVENT_TYPE_AXIS);
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterUIInputEventCallback002(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    OH_NativeXComponent_ExpectedRateRange ExpectedRateRange = { .min = 30, .max = 60, .expected = 45 };
    if (OH_NativeXComponent_RegisterUIInputEventCallback(ChildXComponent->GetXComponent(),
                                                         UIInputEventCallback, ARKUI_UIINPUTEVENT_TYPE_KEY) == 0) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    if (OH_NativeXComponent_RegisterUIInputEventCallback(ChildXComponent->GetXComponent(),
                                                         UIInputEventCallback, ARKUI_UIINPUTEVENT_TYPE_MOUSE) == 0) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    if (OH_NativeXComponent_RegisterUIInputEventCallback(ChildXComponent->GetXComponent(),
                                                         UIInputEventCallback, ARKUI_UIINPUTEVENT_TYPE_TOUCH) == 0) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    if (OH_NativeXComponent_RegisterUIInputEventCallback(ChildXComponent->GetXComponent(),
                                                         UIInputEventCallback, ARKUI_UIINPUTEVENT_TYPE_UNKNOWN) == 0) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 401, &retValue);
    return retValue;
}

void UIInputEventCallback(OH_NativeXComponent* component, ArkUI_UIInputEvent* event, ArkUI_UIInputEvent_Type type)
{
    return;
}

napi_value RegisterOnTouchInterceptCallback001(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    result = OH_NativeXComponent_RegisterOnTouchInterceptCallback(ChildXComponent->GetXComponent(), OnTouchInterceptCallback);
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterOnTouchInterceptCallback002(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    if (OH_NativeXComponent_RegisterOnTouchInterceptCallback(nullptr, OnTouchInterceptCallback) == 0) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    if (OH_NativeXComponent_RegisterOnTouchInterceptCallback(ChildXComponent->GetXComponent(), nullptr) == 0) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 401, &retValue);
    return retValue;
}

HitTestMode OnTouchInterceptCallback(OH_NativeXComponent* component, ArkUI_UIInputEvent* event)
{
    OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "TouchPoint", "OnTouchInterceptCallback");
    return HTM_BLOCK;
}

napi_value GetNativeAccessibilityProvider001(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    ArkUI_AccessibilityProvider* handle;
    result = OH_NativeXComponent_GetNativeAccessibilityProvider(ChildXComponent->GetXComponent(), &handle);
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetNativeAccessibilityProvider002(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    ArkUI_AccessibilityProvider* handle;
    if (OH_NativeXComponent_GetNativeAccessibilityProvider(nullptr, &handle) == 0) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    if (OH_NativeXComponent_GetNativeAccessibilityProvider(ChildXComponent->GetXComponent(), nullptr) == 0) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 401, &retValue);
    return retValue;
}

napi_value RegisterKeyEventCallbackWithResult001(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    ArkUI_AccessibilityProvider* handle;
    result = OH_NativeXComponent_RegisterKeyEventCallbackWithResult(ChildXComponent->GetXComponent(), OnKeyEventCallbackWithResult);
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterKeyEventCallbackWithResult002(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    ArkUI_AccessibilityProvider* handle;
    if (OH_NativeXComponent_RegisterKeyEventCallbackWithResult(nullptr, OnKeyEventCallbackWithResult) == 0) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    if (OH_NativeXComponent_RegisterKeyEventCallbackWithResult(ChildXComponent->GetXComponent(), nullptr) == 0) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 401, &retValue);
    return retValue;
}

bool OnKeyEventCallbackWithResult(OH_NativeXComponent* component, void* window)
{
    return false;
}

napi_value XComponentInitialize001(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    Component temp = Component(ARKUI_NODE_XCOMPONENT);
    int32_t result = 0;
    result = OH_ArkUI_XComponent_Initialize(temp.GetComponent());
    result = OH_ArkUI_XComponent_Initialize(temp.GetComponent());
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value XComponentFinalize001(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    Component temp = Component(ARKUI_NODE_XCOMPONENT);
    if (OH_ArkUI_XComponent_Finalize(temp.GetComponent()) == ARKUI_ERROR_CODE_XCOMPONENT_STATE_INVALID) {
        napi_create_int32(env, ARKUI_ERROR_CODE_XCOMPONENT_STATE_INVALID, &retValue);
        return retValue;
    }
    if (OH_ArkUI_XComponent_Finalize(temp.GetComponent()) == ARKUI_ERROR_CODE_XCOMPONENT_STATE_INVALID) {
        napi_create_int32(env, ARKUI_ERROR_CODE_XCOMPONENT_STATE_INVALID, &retValue);
        return retValue;
    }
    napi_create_int32(env, 0, &retValue);
    return retValue;
}

napi_value XComponentSetNeedSoftKeyboard001(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    result = OH_ArkUI_XComponent_SetNeedSoftKeyboard(ChildXComponent->GetComponent(), false);
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value XComponentSetNeedSoftKeyboard002(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    if (OH_ArkUI_XComponent_SetNeedSoftKeyboard(nullptr, true) == 0) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    if (OH_ArkUI_XComponent_SetNeedSoftKeyboard(nullptr, false) == 0) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 401, &retValue);
    return retValue;
}



napi_value GetExtraMouseEventInfo001(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 401;
    OH_NativeXComponent_ExtraMouseEventInfo* ExtraMouseEventInfo;
    result = OH_NativeXComponent_GetExtraMouseEventInfo(ChildXComponent->GetXComponent(), &ExtraMouseEventInfo);
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetExtraMouseEventInfo002(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    OH_NativeXComponent_ExtraMouseEventInfo* ExtraMouseEventInfo;
    result = OH_NativeXComponent_GetExtraMouseEventInfo(nullptr, &ExtraMouseEventInfo);
    result = OH_NativeXComponent_GetExtraMouseEventInfo(ChildXComponent->GetXComponent(), nullptr) == 401 && result == 401 ? 401 : 0;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetMouseEventModifierKeyStates001(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 401;
    OH_NativeXComponent_ExtraMouseEventInfo* ExtraMouseEventInfo;
    OH_NativeXComponent_GetExtraMouseEventInfo(ChildXComponent->GetXComponent(), &ExtraMouseEventInfo);
    result = OH_NativeXComponent_GetMouseEventModifierKeyStates(ExtraMouseEventInfo, &g_keys);
    napi_create_int32(env, g_result, &retValue);
    return retValue;
}

napi_value GetMouseEventModifierKeyStates002(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    OH_NativeXComponent_ExtraMouseEventInfo* ExtraMouseEventInfo;
    OH_NativeXComponent_GetExtraMouseEventInfo(ChildXComponent->GetXComponent(), &ExtraMouseEventInfo);
    result = OH_NativeXComponent_GetMouseEventModifierKeyStates(nullptr, &g_keys);
    result = OH_NativeXComponent_GetMouseEventModifierKeyStates(ExtraMouseEventInfo, nullptr) == 401 && result == 401 ? 401 : 0;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterFocusEventCallback001(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 401;
    result = OH_NativeXComponent_RegisterFocusEventCallback(ChildXComponent->GetXComponent(), OnFocusEvent);
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterFocusEventCallback002(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    result = OH_NativeXComponent_RegisterFocusEventCallback(nullptr, OnFocusEvent);
    result = OH_NativeXComponent_RegisterFocusEventCallback(ChildXComponent->GetXComponent(), nullptr) == -2 && result == -2 ? 401 : 0;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterBlurEventCallback001(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 401;
    result = OH_NativeXComponent_RegisterBlurEventCallback(ChildXComponent->GetXComponent(), OnBlurEvent);
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterBlurEventCallback002(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    result = OH_NativeXComponent_RegisterBlurEventCallback(nullptr, OnBlurEvent);
    result = OH_NativeXComponent_RegisterBlurEventCallback(ChildXComponent->GetXComponent(), nullptr) == -2 && result == -2 ? 401 : 0;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CreateNativeNode(napi_env env, napi_callback_info info)
{
    if ((env == nullptr) || (info == nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "PluginManager", "CreateNativeNode env or info is null");
        return nullptr;
    }
    size_t argCnt = 1;
    napi_value args[1] = { nullptr };
    if (napi_get_cb_info(env, info, &argCnt, args, nullptr, nullptr) != napi_ok) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "PluginManager", "CreateNativeNode napi_get_cb_info failed");
    }
    if (argCnt != PARAM_1) {
        napi_throw_type_error(env, NULL, "Wrong number of arguments");
        return nullptr;
    }
    ArkUI_NodeContentHandle nodeContentHandle_ = nullptr;
    OH_ArkUI_GetNodeContentFromNapiValue(env, args[0], &nodeContentHandle_);
    g_xComponent.SetPercentWidth(80);
    g_xComponent.SetPercentHeight(80);
    g_xComponent.SetBackgroundColor(0xffff0000);
    ChildXComponent->SetXComponentType(ARKUI_XCOMPONENT_TYPE_SURFACE);
    ChildXComponent->SetXComponentId("TestXComponent");
//    ChildXComponent->SetXComponentSurfaceSize(480, 320);
    ChildXComponent->SetPercentWidth(100);
    ChildXComponent->SetPercentHeight(100);
    ChildXComponent->SetId("NativeXComponentTest");
    ChildXComponent->SetFocusable(true);
    ChildXComponent->SetFocusDefault(true);
    ChildXComponent->PrintXComponentInfo();
    std::shared_ptr<Component> ptr = std::make_shared<XComponent>(*ChildXComponent);
    g_xComponent.AddChild(ptr);
    int32_t ret = OH_ArkUI_NodeContent_SetUserData(nodeContentHandle_, &ChildXComponent);
    OH_NativeXComponent_RegisterKeyEventCallback(ChildXComponent->GetXComponent(), OnKeyEvent);
    if (ret != ARKUI_ERROR_CODE_NO_ERROR) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "PluginManager", "setUserData failed error=%{public}d", ret);
    }
    ret = OH_ArkUI_NodeContent_AddNode(nodeContentHandle_, g_xComponent.GetComponent());
    OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "NodeAttach", "setUserData failed error=%{public}d", ret);
    return nullptr;
}

void XComponent::SetXComponentType(ArkUI_XComponentType type)
{
    ArkUI_NumberValue value[] = {0};
    value[0].i32 = type;
    ArkUI_AttributeItem item = {value, 1};
    _nodeAPI->setAttribute(_component, NODE_XCOMPONENT_TYPE, &item);
}

void XComponent::SetXComponentId(char* id)
{
    ArkUI_NumberValue value[] = {0};
    ArkUI_AttributeItem item = {value, 1, id};
    _nodeAPI->setAttribute(_component, NODE_XCOMPONENT_ID, &item);
}

void XComponent::SetXComponentSurfaceSize(uint32_t width, uint32_t height)
{
    ArkUI_NumberValue value[] = {{.u32 = width}, {.u32 = height}};
    ArkUI_AttributeItem item = {value, 2};
    _nodeAPI->setAttribute(_component, NODE_XCOMPONENT_SURFACE_SIZE, &item);
}

void XComponent::SetFocusable(bool flag)
{
    ArkUI_NumberValue FocusValue[] = {{.i32 = flag?1:0}};
    ArkUI_AttributeItem FocusItem = {FocusValue, 1};
    _nodeAPI->setAttribute(_component, NODE_FOCUSABLE, &FocusItem);
}

void XComponent::SetFocusDefault(bool flag)
{
    ArkUI_NumberValue FocusValue[] = {{.i32 = flag?1:0}};
    ArkUI_AttributeItem FocusItem = {FocusValue, 1};
    _nodeAPI->setAttribute(_component, NODE_DEFAULT_FOCUS, &FocusItem);
}

void XComponent::PrintXComponentInfo()
{
    auto *nativeXComponent = OH_NativeXComponent_GetNativeXComponent(_component);
    if (!nativeXComponent) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "PluginManager", "GetNativeXComponent error");
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "PluginManager", "GetNativeXComponent success");
    auto typeRet = _nodeAPI->getAttribute(_component, NODE_XCOMPONENT_TYPE);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "PluginManager", "xcomponent type: %{public}d",
                typeRet->value[0].i32);
    auto idRet = _nodeAPI->getAttribute(_component, NODE_XCOMPONENT_ID);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "PluginManager", "xcomponent id: %{public}s",
                idRet->string);
}

void OnFocusEvent(OH_NativeXComponent* component, void* window)
{
    OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "TouchPoint", "OnFocusEvent");
    return;
}

void OnBlurEvent(OH_NativeXComponent* component, void* window)
{
    OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "TouchPoint", "OnTouchInterceptCallback");
    return;
}

void OnKeyEvent(OH_NativeXComponent* component, void* window)
{
    // 2. 获取键盘事件数据
    int32_t ret = OH_NativeXComponent_GetKeyEvent(component, &g_keyEvent);
    if (ret != OH_NATIVEXCOMPONENT_RESULT_SUCCESS) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "XComponent", "GetKeyEventError");
        return;
    }

    // 3. 处理按键事件
    switch (g_testFunctionIndex) {
        case 0:
            XComponent::GetKeyEventAction001();
            break;
        case 1:
            XComponent::GetKeyEventAction002();
            break;
        case 2:
            XComponent::GetKeyEventSourceType001();
            break;
        case 3:
            XComponent::GetKeyEventSourceType002();
            break;
        case 4:
            XComponent::GetKeyEventDeviceId001();
            break;
        case 5:
            XComponent::GetKeyEventDeviceId002();
            break;
        case 6:
            XComponent::GetKeyEventTimestamp001();
            break;
        case 7:
            XComponent::GetKeyEventTimestamp002();
            break;
        case 8:
            XComponent::GetKeyEventModifierKeyStates001();
            break;
        case 9:
            XComponent::GetKeyEventModifierKeyStates002();
            break;
        case 10:
            XComponent::GetKeyEventNumLockState001();
            break;
        case 11:
            XComponent::GetKeyEventNumLockState002();
            break;
        case 12:
            XComponent::GetKeyEventCapsLockState001();
            break;
        case 13:
            XComponent::GetKeyEventCapsLockState002();
            break;
        case 14:
            XComponent::GetKeyEventScrollLockState001();
            break;
        case 15:
            XComponent::GetKeyEventScrollLockState002();
            break;
        case 16:
            XComponent::GetKeyEventActionEnum001();
            break;
        case 17:
            XComponent::GetKeyEventActionEnum002();
            break;
        case 18:
            XComponent::GetKeyEventActionEnum003();
            break;
    }
}

void XComponent::GetKeyEventAction001()
{
    OH_NativeXComponent_GetKeyEventAction(g_keyEvent, &g_action);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventAction001");
}

void XComponent::GetKeyEventAction002()
{
    OH_NativeXComponent_GetKeyEventAction(nullptr, &g_action);
    OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventAction001");
}

void XComponent::GetKeyEventActionEnum001()
{
    OH_NativeXComponent_GetKeyEventAction(g_keyEvent, &g_action);
    if (g_action == OH_NATIVEXCOMPONENT_KEY_ACTION_DOWN) {
        g_action_result = 1;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventAction001");
}

void XComponent::GetKeyEventActionEnum002()
{
    OH_NativeXComponent_GetKeyEventAction(g_keyEvent, &g_action);
    if (g_action == OH_NATIVEXCOMPONENT_KEY_ACTION_UP) {
        g_action_result = 1;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventAction001");
}

void XComponent::GetKeyEventActionEnum003()
{
    OH_NativeXComponent_GetKeyEventAction(g_keyEvent, &g_action);
    if (g_action == OH_NATIVEXCOMPONENT_KEY_ACTION_UNKNOWN) {
        g_action_result = 1;
    }
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventActionEnum003${public}%d",g_action);
}

void XComponent::GetKeyEventSourceType001()
{
    OH_NativeXComponent_GetKeyEventSourceType(g_keyEvent, &g_eventSourceType);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventSourceType001");
}

void XComponent::GetKeyEventSourceType002()
{
    OH_NativeXComponent_GetKeyEventSourceType(nullptr, &g_eventSourceType);
    OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventSourceType001");
}

void XComponent::GetKeyEventDeviceId001()
{
    OH_NativeXComponent_GetKeyEventDeviceId(g_keyEvent, &g_deviceId);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventSourceType001");
}

void XComponent::GetKeyEventDeviceId002()
{
    OH_NativeXComponent_GetKeyEventDeviceId(nullptr, &g_deviceId);
    OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventSourceType001");
}

void XComponent::GetKeyEventTimestamp001()
{
    OH_NativeXComponent_GetKeyEventTimestamp(g_keyEvent, &g_timestamp);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventSourceType001");
}

void XComponent::GetKeyEventTimestamp002()
{
    OH_NativeXComponent_GetKeyEventTimestamp(nullptr, &g_deviceId);
    OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventSourceType001");
}

void XComponent::GetKeyEventModifierKeyStates001()
{
    g_result = OH_NativeXComponent_GetKeyEventModifierKeyStates(g_keyEvent, &g_keys);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventSourceType001");
}

void XComponent::GetKeyEventModifierKeyStates002()
{
    g_result = OH_NativeXComponent_GetKeyEventModifierKeyStates(nullptr, &g_keys);
    OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventSourceType001");
}

void XComponent::GetKeyEventNumLockState001()
{
    g_result = OH_NativeXComponent_GetKeyEventNumLockState(g_keyEvent, &isNumLockOn);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventNumLockState001");
}

void XComponent::GetKeyEventNumLockState002()
{
    g_result = OH_NativeXComponent_GetKeyEventNumLockState(nullptr, &isNumLockOn);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventNumLockState002");
}

void XComponent::GetKeyEventCapsLockState001()
{
    g_result = OH_NativeXComponent_GetKeyEventCapsLockState(g_keyEvent, &isCapsLockOn);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventCapsLockState001");
}

void XComponent::GetKeyEventCapsLockState002()
{
    g_result = OH_NativeXComponent_GetKeyEventCapsLockState(nullptr, &isCapsLockOn);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventCapsLockState002");
}

void XComponent::GetKeyEventScrollLockState001()
{
    g_result = OH_NativeXComponent_GetKeyEventScrollLockState(g_keyEvent, &isScrollLockOn);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventScrollLockState001");
}

void XComponent::GetKeyEventScrollLockState002()
{
    g_result = OH_NativeXComponent_GetKeyEventScrollLockState(nullptr, &isScrollLockOn);
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN,
                 "KeyEventXComponent", "Choose Function GetKeyEventScrollLockState002");
}


}
