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
#include "node/NodeAttribute.h"

namespace NodeAttribute {

napi_value CheckNodeTextHalfLeading(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    int32_t flag = mt() % 2; // 生成随机参数
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT);
    ArkUI_NumberValue value[] = { { .i32 = flag } };
    ArkUI_AttributeItem item = { .value = value, .size = PARAM_1 };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_HALF_LEADING, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_HALF_LEADING);
    if (itemptr->value[0].i32 == flag && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else {
        result = setResult;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextLineCount(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_LINE_COUNT);
    if (itemptr->value[0].i32 == 0) {
        result = 1;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextOptimizeTrailingSpace(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    int32_t flag = mt() % 2; // 生成随机参数
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT);
    ArkUI_NumberValue value[] = { { .i32 = flag } };
    ArkUI_AttributeItem item = { .value = value, .size = PARAM_1 };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_OPTIMIZE_TRAILING_SPACE, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_OPTIMIZE_TRAILING_SPACE);
    if (itemptr->value[0].i32 == flag && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else {
        result = setResult;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextLinearGradient(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    float angle = mt() % 3600 / 10; // 生成随机参数
    ArkUI_LinearGradientDirection direction = ArkUI_LinearGradientDirection(mt() % 10);
    int32_t flag = mt() % 2; // 生成随机参数
    int size = mt() % 4; // 生成随机参数
    uint32_t color1 = mt() % 4294967296; // 生成随机参数
    uint32_t color2 = mt() % 4294967296; // 生成随机参数
    uint32_t color3 = mt() % 4294967296; // 生成随机参数
    uint32_t colors[] = { color1, color2, color3 };
    float stop1 = mt() % 3 / 10; // 生成随机参数
    float stop2 = mt() % 4 / 10 + stop1; // 生成随机参数
    float stop3 = mt() % 4 / 10 + stop2; // 生成随机参数
    float stops[] = { stop1, stop2, stop3 };
    ArkUI_ColorStop object = { .colors = colors, .stops = stops, .size = size };
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT);
    ArkUI_NumberValue value[] = { { .f32 = angle }, { .i32 = direction }, { .i32 = flag } };
    ArkUI_AttributeItem item = { .value = value, .size = PARAM_3, .object = &object };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_LINEAR_GRADIENT, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_LINEAR_GRADIENT);
    bool condition = itemptr->value[1].i32 == direction && itemptr->value[2].i32 == flag;
    if (condition && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else if (condition) {
        result = setResult;
    } else {
        result = -9;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextRadialGradient(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    float x = mt() % 1000 / 10; // 生成随机参数
    float y = mt() % 1000 / 10; // 生成随机参数
    float radius = mt() % 3600 / 10; // 生成随机参数
    int32_t flag = mt() % 2; // 生成随机参数
    int size = mt() % 4; // 生成随机参数
    uint32_t color1 = mt() % 4294967296; // 生成随机参数
    uint32_t color2 = mt() % 4294967296; // 生成随机参数
    uint32_t color3 = mt() % 4294967296; // 生成随机参数
    uint32_t colors[] = { color1, color2, color3 };
    float stop1 = mt() % 3 / 10; // 生成随机参数
    float stop2 = mt() % 4 / 10 + stop1; // 生成随机参数
    float stop3 = mt() % 4 / 10 + stop2; // 生成随机参数
    float stops[] = { stop1, stop2, stop3 };
    ArkUI_ColorStop object = { .colors = colors, .stops = stops, .size = size };
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT);
    ArkUI_NumberValue value[] = { { .f32 = x }, { .f32 = y }, { .f32 = radius }, { .i32 = flag } };
    ArkUI_AttributeItem item = { .value = value, .size = PARAM_4, .object = &object };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_RADIAL_GRADIENT, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_RADIAL_GRADIENT);
    bool condition = itemptr->value[3].i32 == flag;
    if (condition && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else if (condition) {
        result = setResult;
    } else {
        result = -9;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextVerticalAlign(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    ArkUI_TextVerticalAlignment TextVerticalAlignment = ArkUI_TextVerticalAlignment(mt() % 4); // 生成随机参数
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT);
    ArkUI_NumberValue value[] = { { .i32 = TextVerticalAlignment } };
    ArkUI_AttributeItem item = { .value = value, .size = PARAM_1 };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_VERTICAL_ALIGN, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_VERTICAL_ALIGN);
    if (itemptr->value[0].i32 == TextVerticalAlignment && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else {
        result = setResult;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextInputHalfLeading(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    int32_t flag = mt() % 2; // 生成随机参数
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT_INPUT);
    ArkUI_NumberValue value[] = { { .i32 = flag } };
    ArkUI_AttributeItem item = { .value = value, .size = PARAM_1 };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_INPUT_HALF_LEADING, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_INPUT_HALF_LEADING);
    if (itemptr->value[0].i32 == flag && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else {
        result = setResult;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextInputKeyboardAppearance(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    ArkUI_KeyboardAppearance KeyboardAppearance = ArkUI_KeyboardAppearance(mt() % 4); // 生成随机参数
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT_INPUT);
    ArkUI_NumberValue value[] = { { .i32 = KeyboardAppearance } };
    ArkUI_AttributeItem item = { .value = value, .size = PARAM_1 };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_INPUT_KEYBOARD_APPEARANCE, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_INPUT_KEYBOARD_APPEARANCE);
    if (itemptr->value[0].i32 == KeyboardAppearance && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else {
        result = setResult;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextInputEnableFillAnimation(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    int32_t flag = mt() % 2; // 生成随机参数
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT_INPUT);
    ArkUI_NumberValue value[] = { { .i32 = flag } };
    ArkUI_AttributeItem item = { .value = value, .size = PARAM_1 };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_INPUT_ENABLE_FILL_ANIMATION, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_INPUT_ENABLE_FILL_ANIMATION);
    if (itemptr->value[0].i32 == flag && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else {
        result = setResult;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextInputLineHeight(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    int32_t lineHeight = mt() % 30; // 生成随机参数
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT_INPUT);
    ArkUI_NumberValue value[] = { { .i32 = lineHeight } };
    ArkUI_AttributeItem item = { .value = value, .size = PARAM_1 };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_INPUT_LINE_HEIGHT, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_INPUT_LINE_HEIGHT);
    if (itemptr->value[0].i32 == lineHeight && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else {
        result = setResult;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextAreaInputFilter(napi_env env, napi_callback_info info)
{
    char* regular = "/<.*>/"; // 生成随机参数
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT_AREA);
    ArkUI_AttributeItem item = { .string = regular };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_AREA_INPUT_FILTER, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_AREA_INPUT_FILTER);
    if (strcmp(item.string, regular) == 0 && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else {
        result = setResult;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextAreaContentType(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    ArkUI_TextInputContentType TextInputContentType = ArkUI_TextInputContentType(mt() % 33); // 生成随机参数
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT_AREA);
    ArkUI_NumberValue value[] = { { .i32 = TextInputContentType } };
    ArkUI_AttributeItem item = { .value = value, .size = PARAM_1 };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_AREA_CONTENT_TYPE, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_AREA_CONTENT_TYPE);
    if (itemptr->value[0].i32 == TextInputContentType && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else {
        result = setResult;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextAreaHalfLeading(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    int32_t flag = mt() % 2; // 生成随机参数
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT_AREA);
    ArkUI_NumberValue value[] = { { .i32 = flag } };
    ArkUI_AttributeItem item = { .value = value, .size = PARAM_1 };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_AREA_HALF_LEADING, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_AREA_HALF_LEADING);
    if (itemptr->value[0].i32 == flag && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else {
        result = setResult;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextAreaKeyboardAppearance(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    ArkUI_KeyboardAppearance KeyboardAppearance = ArkUI_KeyboardAppearance(mt() % 4); // 生成随机参数
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT_AREA);
    ArkUI_NumberValue value[] = { { .i32 = KeyboardAppearance } };
    ArkUI_AttributeItem item = { .value = value, .size = PARAM_1 };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_AREA_KEYBOARD_APPEARANCE, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_AREA_KEYBOARD_APPEARANCE);
    if (itemptr->value[0].i32 == KeyboardAppearance && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else {
        result = setResult;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextAreaMaxLines(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT_AREA);
    ArkUI_AttributeItem item;
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_AREA_MAX_LINES);
    if (itemptr->value[0].i32 == 3) {
        result = 1;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextAreaLineSpacing(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    int32_t lineSpacing = mt() % 10; // 生成随机参数
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT_AREA);
    ArkUI_NumberValue value[] = { { .i32 = lineSpacing } };
    ArkUI_AttributeItem item = { .value = value, .size = PARAM_1 };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_AREA_LINE_SPACING, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_AREA_LINE_SPACING);
    if (itemptr->value[0].i32 == lineSpacing && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else {
        result = setResult;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextAreaMinLines(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT_AREA);
    ArkUI_AttributeItem item;
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_AREA_MIN_LINES);
    if (itemptr->value[0].i32 == 0) {
        result = 1;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextAreaMaxLinesWithScroll(napi_env env, napi_callback_info info)
{
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT_AREA);
    ArkUI_AttributeItem item;
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_AREA_MAX_LINES_WITH_SCROLL);
    if (itemptr->value[0].i32 == 3) {
        result = 1;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextAreaLineHeight(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    int32_t lineHeight = mt() % 30; // 生成随机参数
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT_AREA);
    ArkUI_NumberValue value[] = { { .i32 = lineHeight } };
    ArkUI_AttributeItem item = { .value = value, .size = PARAM_1 };
    int32_t setResult = TextComponent.setAttribute(NODE_TEXT_AREA_LINE_HEIGHT, &item);
    const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_AREA_LINE_HEIGHT);
    if (itemptr->value[0].i32 == lineHeight && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
        result = 1;
    } else {
        result = setResult;
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CheckNodeTextPickSelectedBackgroundStyle(napi_env env, napi_callback_info info)
{
    std::random_device randomNumber; // 随机数种子生成器
    std::mt19937 mt(randomNumber()); // 随机数生成器
    int32_t flag = mt() % 2;
    uint32_t color = mt() % 4294967296; // 生成随机参数
    float radius1 = mt() % 3600 / 10; // 生成随机参数
    float radius2 = mt() % 3600 / 10; // 生成随机参数
    float radius3 = mt() % 3600 / 10; // 生成随机参数
    float radius4 = mt() % 3600 / 10; // 生成随机参数
    napi_value retValue = 0;
    int32_t result = 0;
    XC TextComponent = XC(ARKUI_NODE_TEXT_PICKER);
    if (flag) {
        ArkUI_NumberValue value[] = { { .u32 = color }, { .f32 = radius1 } };
        ArkUI_AttributeItem item = { .value = value, .size = PARAM_2 };
        int32_t setResult = TextComponent.setAttribute(NODE_TEXT_PICKER_SELECTED_BACKGROUND_STYLE, &item);
        const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_PICKER_SELECTED_BACKGROUND_STYLE);
        bool condition = itemptr->value[0].u32 == color;
        if (condition && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
            result = 1;
        } else if (condition) {
            result = setResult;
        } else {
            result = -9;
        }
    } else {
        ArkUI_NumberValue value[] = { { .u32 = color }, { .f32 = radius1 }, { .f32 = radius2 },
                                      { .f32 = radius3 }, { .f32 = radius4 } };
        ArkUI_AttributeItem item = { .value = value, .size = PARAM_2 };
        int32_t setResult = TextComponent.setAttribute(NODE_TEXT_PICKER_SELECTED_BACKGROUND_STYLE, &item);
        const ArkUI_AttributeItem* itemptr = TextComponent.getAttribute(NODE_TEXT_PICKER_SELECTED_BACKGROUND_STYLE);
        bool condition = itemptr->value[0].u32 == color;
        if (condition && setResult == ARKUI_ERROR_CODE_NO_ERROR) {
            result = 1;
        } else if (condition) {
            result = setResult;
        } else {
            result = -9;
        }
    }
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CreateAttributeNativeNode(napi_env env, napi_callback_info info)
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
    ArkUI_NodeContentHandle nodeContentHandle = nullptr;
    OH_ArkUI_GetNodeContentFromNapiValue(env, args[0], &nodeContentHandle);
    auto nodeContentEvent = [](ArkUI_NodeContentEvent *event) {
        ArkUI_NodeContentHandle handle = OH_ArkUI_NodeContentEvent_GetNodeContentHandle(event);
        if (OH_ArkUI_NodeContentEvent_GetEventType(event) == NODE_CONTENT_EVENT_ON_ATTACH_TO_WINDOW) {
            InitNode();
            OH_ArkUI_NodeContent_AddNode(handle, ColumnComponent.GetComponent());
        }
    };
    OH_ArkUI_NodeContent_RegisterCallback(nodeContentHandle, nodeContentEvent);
    return nullptr;
}

void InitNode()
{
    ColumnComponent.SetPercentWidth(0.5);
    ColumnComponent.SetPercentHeight(0.5);
    ColumnComponent.SetBackgroundColor(0xff3d8aed);
    ColumnComponent.SetId("rootnode");
}

}
