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
static ArkUI_NodeHandle g_handle = nullptr;
static int32_t g_nodeutilsresult1 = 2;
static int32_t g_nodeutilsresult2 = 2;
static int32_t g_nodeutilsresult3 = 2;
static ArkUI_NodeCustomEvent* g_customevent;
static ArkUI_NativeNodeAPI_1 *g_nodeAPI = nullptr;
namespace NodeUtilsTest {
napi_value GetNodeTypeTest_001(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_CUSTOM) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_002(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_TEXT);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_TEXT) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_003(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_SPAN);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_SPAN) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_004(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_IMAGE_SPAN);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_IMAGE_SPAN) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_005(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_IMAGE);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_IMAGE) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_006(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_TOGGLE);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_TOGGLE) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_007(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_LOADING_PROGRESS);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_LOADING_PROGRESS) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_008(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_TEXT_INPUT);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_TEXT_INPUT) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_009(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_TEXT_AREA);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_TEXT_AREA) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_010(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_BUTTON);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_BUTTON) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_011(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_PROGRESS);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_PROGRESS) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_012(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_CHECKBOX);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_CHECKBOX) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_013(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_XCOMPONENT);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_XCOMPONENT) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_014(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_DATE_PICKER);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_DATE_PICKER) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_015(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_TIME_PICKER);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_TIME_PICKER) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_016(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_TEXT_PICKER);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_TEXT_PICKER) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_017(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_CALENDAR_PICKER);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_CALENDAR_PICKER) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_018(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_SLIDER);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_SLIDER) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_019(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_RADIO);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_RADIO) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_020(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_IMAGE_ANIMATOR);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_IMAGE_ANIMATOR) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}
napi_value GetNodeTypeTest_021(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_XCOMPONENT_TEXTURE);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_XCOMPONENT_TEXTURE) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_022(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_CHECKBOX_GROUP);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_CHECKBOX_GROUP) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_023(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_STACK);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_STACK) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_024(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_SWIPER);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_SWIPER) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_025(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_SCROLL);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_SCROLL) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_026(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_LIST);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_LIST) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_027(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_LIST_ITEM);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_LIST_ITEM) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_028(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_LIST_ITEM_GROUP);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_LIST_ITEM_GROUP) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_029(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_COLUMN);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_COLUMN) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_030(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_ROW);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_ROW) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_031(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_FLEX);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_FLEX) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_032(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_REFRESH);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_REFRESH) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_033(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_WATER_FLOW);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_WATER_FLOW) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_034(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_FLOW_ITEM);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_FLOW_ITEM) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_035(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_RELATIVE_CONTAINER);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_RELATIVE_CONTAINER) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_036(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_GRID);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_GRID) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_037(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_GRID_ITEM);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_GRID_ITEM) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_038(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_CUSTOM_SPAN);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_CUSTOM_SPAN) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_039(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_EMBEDDED_COMPONENT);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_EMBEDDED_COMPONENT) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}

napi_value GetNodeTypeTest_040(napi_env env, napi_callback_info info)
{
    napi_value retValue;
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI);
    // 创建指定类型的节点
    auto g_handle = g_nodeAPI->createNode(ARKUI_NODE_CUSTOM);
    int result = OH_ArkUI_NodeUtils_GetNodeType(g_handle);
    if (result == ARKUI_NODE_UNDEFINED) {
        napi_create_int32(env, 0, &retValue);
        return retValue;
    }
    napi_create_int32(env, 1, &retValue);
    return retValue;
}
}