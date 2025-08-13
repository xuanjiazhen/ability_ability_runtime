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
#include "nodeutilstest/NodeUtilsTest.h"
#include "arkui/native_type.h"
#include "focuscontrol/FocusControlTest.h"
#include "manager/PluginManagerTest.h"
#include <cstddef>
#include <cstdint>
static ArkUI_NodeHandle g_handle = nullptr;
static int32_t g_nodeutilsresult1 = 2;
static int32_t g_nodeutilsresult2 = 2;
static int32_t g_nodeutilsresult3 = 2;
static ArkUI_NodeCustomEvent* g_customevent;
static ArkUI_NativeNodeAPI_1 *g_nodeAPI = nullptr;
namespace NodeUtilsTest {
napi_value SetForceDarkConfigTest_001(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto component = g_nodeAPI->createNode(ARKUI_NODE_XCOMPONENT);
    ArkUI_ContextHandle uiContext = OH_ArkUI_GetContextByNode(component);
//    int result = OH_ArkUI_SetForceDarkConfig(uiContext, false, ARKUI_NODE_LIST, nullptr);
    napi_value retValue;
    napi_create_int32(env, 11, &retValue);
    return retValue;
}

napi_value SetForceDarkConfigTest_002(napi_env env, napi_callback_info info)
{
    ArkUI_ContextHandle uiContext = nullptr;
//    int result = OH_ArkUI_SetForceDarkConfig(nullptr, false, ARKUI_NODE_LIST, nullptr);
    napi_value retValue;
    napi_create_int32(env, 11, &retValue);
    return retValue;
}
napi_value SetForceDarkConfigTest_003(napi_env env, napi_callback_info info)
{
    ArkUI_ContextHandle uiContext = nullptr;
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto component = g_nodeAPI->createNode(ARKUI_NODE_XCOMPONENT);
//    int result = OH_ArkUI_SetForceDarkConfig(uiContext, false, ARKUI_NODE_LIST, nullptr);
    napi_value retValue;
    napi_create_int32(env, 11, &retValue);
    return retValue;
}

napi_value CreateCustomSpanNode(napi_env env, napi_callback_info info)
{
    size_t argc = PARAM_1;
    napi_value args[PARAM_1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    size_t length = PARAM_64;
    size_t strLength = PARAM_0;
    char xComponentID[PARAM_64] = {PARAM_0};
    napi_get_value_string_utf8(env, args[PARAM_0], xComponentID, length, &strLength);
    if ((env == nullptr) || (info == nullptr)) {
        return nullptr;
    }
    ArkUI_NativeNodeAPI_1* nodeAPI = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, nodeAPI);
    auto column = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    auto custom = nodeAPI->createNode(ARKUI_NODE_CUSTOM);

    ArkUI_NumberValue sizeValue[] = { { .f32 = SIZE_200 }, { .f32 = SIZE_200 } };
    ArkUI_AttributeItem sizeItem = { sizeValue, sizeof(sizeValue) / sizeof(ArkUI_NumberValue) };
    nodeAPI->setAttribute(custom, NODE_SIZE, &sizeItem);

    ArkUI_NumberValue backgroundColorValue[] = { { .u32 = COLOR_RED } };
    ArkUI_AttributeItem backgroundColorItem = { backgroundColorValue,
        sizeof(backgroundColorValue) / sizeof(ArkUI_NumberValue) };
    nodeAPI->setAttribute(custom, NODE_BACKGROUND_COLOR, &backgroundColorItem);

    ArkUI_AttributeItem id_item = {};
    id_item.string = "OnDrawTest";
    nodeAPI->setAttribute(custom, NODE_ID, &id_item);

    nodeAPI->addChild(column, custom);

    nodeAPI->registerNodeCustomEventReceiver([](ArkUI_NodeCustomEvent* event) {
        ArkUI_CustomSpanMeasureInfo *measureInfo = OH_ArkUI_CustomSpanMeasureInfo_Create();
        g_nodeutilsresult1 = OH_ArkUI_NodeCustomEvent_GetCustomSpanMeasureInfo(event, measureInfo);
        ArkUI_CustomSpanMetrics *customSpanMetrics = OH_ArkUI_CustomSpanMetrics_Create();
        g_nodeutilsresult2 = OH_ArkUI_NodeCustomEvent_SetCustomSpanMetrics(event, customSpanMetrics);
        ArkUI_CustomSpanDrawInfo *customSpanDrawInfo = OH_ArkUI_CustomSpanDrawInfo_Create();
        g_nodeutilsresult3 = OH_ArkUI_NodeCustomEvent_GetCustomSpanDrawInfo(event, customSpanDrawInfo);
    });
    nodeAPI->registerNodeCustomEvent(custom, ARKUI_NODE_CUSTOM_EVENT_ON_DRAW, ON_DRAW_EVENT_ID, nullptr);
    std::string id(xComponentID);
    if (OH_NativeXComponent_AttachNativeRootNode(ArkUICapiTest::PluginManager::GetInstance()->GetNativeXComponent(id),
                                                 column) == INVALID_PARAM) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "TouchEventXComponent",
                     "OH_NativeXComponent_AttachNativeRootNode failed");
    }
    napi_value exports;
    if (napi_create_object(env, &exports) != napi_ok) {
        napi_throw_type_error(env, NULL, "napi_create_object failed");
        return nullptr;
    }
    return exports;
}

napi_value GetCustomSpanMeasureInfoTest_001(napi_env env, napi_callback_info info)
{
    napi_value retValue;
    napi_create_int32(env, g_nodeutilsresult1, &retValue);
    return retValue;
}

napi_value GetCustomSpanMeasureInfoTest_002(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto customSpan = g_nodeAPI->createNode(ARKUI_NODE_CUSTOM_SPAN);
    ArkUI_CustomSpanMeasureInfo *measureInfo = OH_ArkUI_CustomSpanMeasureInfo_Create();
    int result = OH_ArkUI_NodeCustomEvent_GetCustomSpanMeasureInfo(nullptr, measureInfo);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value SetCustomSpanMetricsTest_001(napi_env env, napi_callback_info info)
{
    napi_value retValue;
    napi_create_int32(env, g_nodeutilsresult2, &retValue);
    return retValue;
}

napi_value SetCustomSpanMetricsTest_002(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto customSpan = g_nodeAPI->createNode(ARKUI_NODE_CUSTOM_SPAN);
    ArkUI_CustomSpanMetrics *customSpanMetrics = OH_ArkUI_CustomSpanMetrics_Create();
    int result = OH_ArkUI_NodeCustomEvent_SetCustomSpanMetrics(nullptr, customSpanMetrics);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetCustomSpanDrawInfoTest_001(napi_env env, napi_callback_info info)
{
    napi_value retValue;
    napi_create_int32(env, g_nodeutilsresult3, &retValue);
    return retValue;
}

napi_value GetCustomSpanDrawInfoTest_002(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto customSpan = g_nodeAPI->createNode(ARKUI_NODE_CUSTOM_SPAN);
    ArkUI_CustomSpanDrawInfo *customSpanDrawInfo = OH_ArkUI_CustomSpanDrawInfo_Create();
    int result = OH_ArkUI_NodeCustomEvent_GetCustomSpanDrawInfo(nullptr, customSpanDrawInfo);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CloseAllSwipeActionsTest_001(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto list = g_nodeAPI->createNode(ARKUI_NODE_LIST);
    int customData = 42;
    int result = OH_ArkUI_List_CloseAllSwipeActions(list, &customData, nullptr);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CloseAllSwipeActionsTest_002(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto text = g_nodeAPI->createNode(ARKUI_NODE_TEXT);
    int customData = 42;
    int result = OH_ArkUI_List_CloseAllSwipeActions(text, &customData, nullptr);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CloseAllSwipeActionsTest_003(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto list = nullptr;
    int result = OH_ArkUI_List_CloseAllSwipeActions(list, 0, nullptr);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterSystemColorModeChangeTest_001(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto node = g_nodeAPI->createNode(ARKUI_NODE_REFRESH);
    int customData = 42;
    int result = OH_ArkUI_RegisterSystemColorModeChangeEvent(node, &customData, nullptr);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterSystemColorModeChangeTest_002(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = nullptr;
    int result = OH_ArkUI_RegisterSystemColorModeChangeEvent(column, 0, nullptr);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterSystemFontStyleChangeTest_001(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto checkboxnode = g_nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    int customData = 42;
    int result = OH_ArkUI_RegisterSystemFontStyleChangeEvent(checkboxnode, &customData, nullptr);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterSystemFontStyleChangeTest_002(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = nullptr;
    int result = OH_ArkUI_RegisterSystemFontStyleChangeEvent(column, 0, nullptr);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterLayoutCallbackOnNodeTest_001(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = g_nodeAPI->createNode(ARKUI_NODE_COLUMN);
    int customData = 42;
    int result = OH_ArkUI_RegisterLayoutCallbackOnNodeHandle(column, &customData, nullptr);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterLayoutCallbackOnNodeTest_002(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = nullptr;
    int result = OH_ArkUI_RegisterLayoutCallbackOnNodeHandle(column, 0, nullptr);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterDrawCallbackOnNodeTest_001(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = g_nodeAPI->createNode(ARKUI_NODE_COLUMN);
    int customData = 42;
    int result = OH_ArkUI_RegisterDrawCallbackOnNodeHandle(column, &customData, nullptr);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterDrawCallbackOnNodeTest_002(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = nullptr;
    int result = OH_ArkUI_RegisterDrawCallbackOnNodeHandle(column, 0, nullptr);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value UnregisterLayoutCallbackOnNodeTest_001(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = g_nodeAPI->createNode(ARKUI_NODE_COLUMN);
    int result = OH_ArkUI_UnregisterLayoutCallbackOnNodeHandle(column);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value UnregisterLayoutCallbackOnNodeTest_002(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = nullptr;
    int result = OH_ArkUI_UnregisterLayoutCallbackOnNodeHandle(column);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value UnregisterDrawCallbackOnNodeTest_001(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = g_nodeAPI->createNode(ARKUI_NODE_COLUMN);
    int result = OH_ArkUI_UnregisterDrawCallbackOnNodeHandle(column);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value UnregisterDrawCallbackOnNodeTest_002(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = nullptr;
    int result = OH_ArkUI_UnregisterDrawCallbackOnNodeHandle(column);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetPositionToParentTest_001(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = g_nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_IntOffset *offset = new ArkUI_IntOffset[2];
    int result = OH_ArkUI_NodeUtils_GetPositionToParent(column, offset);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetPositionToParentTest_002(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = nullptr;
    ArkUI_IntOffset *offset = new ArkUI_IntOffset[2];
    int result = OH_ArkUI_NodeUtils_GetPositionToParent(column, offset);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetNodeHandleByUniqueIdTest_001(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = g_nodeAPI->createNode(ARKUI_NODE_COLUMN);
    auto text2 = g_nodeAPI->createNode(ARKUI_NODE_TEXT);
    ArkUI_AttributeItem text2_item = {.string = "GetNodeUniqueId 节点指针置空"};
    g_nodeAPI->setAttribute(text2, NODE_TEXT_CONTENT, &text2_item);
    ArkUI_AttributeItem text2_id = {.string = "11"};
    g_nodeAPI->setAttribute(text2, NODE_ID, &text2_id);
    auto uniqueId = 11;
    int result = OH_ArkUI_NodeUtils_GetNodeHandleByUniqueId(uniqueId, &text2);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetNodeHandleByUniqueIdTest_002(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = g_nodeAPI->createNode(ARKUI_NODE_COLUMN);
    auto text2 = g_nodeAPI->createNode(ARKUI_NODE_TEXT);
    ArkUI_AttributeItem text2_item = {.string = "GetNodeUniqueId 节点指针置空"};
    g_nodeAPI->setAttribute(text2, NODE_TEXT_CONTENT, &text2_item);
    ArkUI_AttributeItem text2_id = {.string = "text2_id"};
    g_nodeAPI->setAttribute(text2, NODE_ID, &text2_id);
    auto uniqueId = NULL;
    int result = OH_ArkUI_NodeUtils_GetNodeHandleByUniqueId(uniqueId, &column);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetNodeHandleByUniqueIdTest_003(napi_env env, napi_callback_info info)
{
    auto uniqueId = 1001;
    ArkUI_NodeHandle node;
    int result = OH_ArkUI_NodeUtils_GetNodeHandleByUniqueId(uniqueId, &node);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetNodeUniqueIdTest_001(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = g_nodeAPI->createNode(ARKUI_NODE_COLUMN);
    auto text2 = g_nodeAPI->createNode(ARKUI_NODE_TEXT);
    ArkUI_AttributeItem text2_item = {.string = "GetNodeUniqueId 节点指针置空"};
    g_nodeAPI->setAttribute(text2, NODE_TEXT_CONTENT, &text2_item);
    ArkUI_AttributeItem text2_id = {.string = "text2_id"};
    g_nodeAPI->setAttribute(text2, NODE_ID, &text2_id);
    auto uniqueId = 0;
    int result = OH_ArkUI_NodeUtils_GetNodeUniqueId(column, &uniqueId);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetNodeUniqueIdTest_002(napi_env env, napi_callback_info info)
{
    auto uniqueId = 0;
    int result = OH_ArkUI_NodeUtils_GetNodeUniqueId(nullptr, &uniqueId);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetNodeUniqueIdTest_003(napi_env env, napi_callback_info info)
{
    auto uniqueId = 0;
    int result = OH_ArkUI_NodeUtils_GetNodeUniqueId(nullptr, &uniqueId);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value AddSupportedUIStatesTest_001(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto column = g_nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NodeHandle text1 = g_nodeAPI->createNode(ARKUI_NODE_TEXT);
    int globalUserDataTest = 10;
    int32_t globalStates = static_cast <int32_t> (UI_STATE_PRESSED);
    int result = OH_ArkUI_AddSupportedUIStates(text1, globalStates, [](int currentStates, void* userData) {
                ArkUI_NodeHandle node = static_cast<ArkUI_NodeHandle>(userData);
                if ((currentStates & UI_STATE_PRESSED) == UI_STATE_PRESSED) {
                    ArkUI_AttributeItem logMsg1 = {.string = "PRESSEDREMOVE"};
                    g_nodeAPI->setAttribute(node, NODE_TEXT_CONTENT, &logMsg1);
                    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "FrameNodeTest", "style callback PRESSED");
                }
            }, true, static_cast<void*>(&globalUserDataTest));
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value AddSupportedUIStatesTest_002(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    ArkUI_NodeHandle text1 = nullptr;
    int globalUserDataTest = 10;
    int32_t globalStates = static_cast <int32_t> (UI_STATE_PRESSED);
    int result = OH_ArkUI_AddSupportedUIStates(text1, globalStates, [](int currentStates, void* userData) {
                ArkUI_NodeHandle node = static_cast<ArkUI_NodeHandle>(userData);
                if ((currentStates & UI_STATE_PRESSED) == UI_STATE_PRESSED) {
                    ArkUI_AttributeItem logMsg1 = {.string = "PRESSEDREMOVE"};
                    g_nodeAPI->setAttribute(node, NODE_TEXT_CONTENT, &logMsg1);
                    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "FrameNodeTest", "style callback PRESSED");
                }
            }, true, static_cast<void*>(&globalUserDataTest));
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RemoveSupportedUIStatesTest_001(napi_env env, napi_callback_info info)
{
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    ArkUI_NodeHandle text1 = g_nodeAPI->createNode(ARKUI_NODE_TEXT);
    int globalUserDataTest = 10;
    int32_t globalStatesToBeSupported = static_cast <int32_t> (UI_STATE_PRESSED);
    int32_t globalStatesToBeRemoved = static_cast <int32_t> (UI_STATE_PRESSED);
    OH_ArkUI_AddSupportedUIStates(text1, globalStatesToBeSupported, [](int currentStates, void* userData) {
                ArkUI_NodeHandle node = static_cast<ArkUI_NodeHandle>(userData);
                if ((currentStates & UI_STATE_PRESSED) == UI_STATE_PRESSED) {
                    ArkUI_AttributeItem logMsg1 = {.string = "PRESSEDREMOVE"};
                    g_nodeAPI->setAttribute(node, NODE_TEXT_CONTENT, &logMsg1);
                    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "FrameNodeTest", "style callback PRESSED");
                }
            }, true, static_cast<void*>(&globalUserDataTest));
    int result = OH_ArkUI_RemoveSupportedUIStates(text1, globalStatesToBeRemoved);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RemoveSupportedUIStatesTest_002(napi_env env, napi_callback_info info)
{
    ArkUI_NodeHandle text1 = nullptr;
    int32_t globalStatesToBeRemoved = static_cast<int32_t>(UI_STATE_PRESSED);
    int result = OH_ArkUI_RemoveSupportedUIStates(text1, globalStatesToBeRemoved);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}
}