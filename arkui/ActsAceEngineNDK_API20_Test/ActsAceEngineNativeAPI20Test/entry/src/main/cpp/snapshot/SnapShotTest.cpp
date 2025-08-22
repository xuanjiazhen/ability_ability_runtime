/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "SnapShotTest.h"
#include <arkui/native_node.h>
#include <arkui/drag_and_drop.h>
#include <multimedia/image_framework/image/pixelmap_native.h>

static OH_PixelmapNative *g_pixelMap = nullptr;
static ArkUI_NodeHandle g_image;
static ArkUI_NativeNodeAPI_1 *g_nodeAPi = nullptr;
static ArkUI_AttributeItem g_NODE_IMAGE_SRC_Item;

namespace SnapShotTest {
static auto CreateColumn(bool dropDataPrefetchOnNode)
{
    ArkUI_NativeNodeAPI_1 *nodeAPI = nullptr;
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, nodeAPI);
    auto column = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    g_image = nodeAPI->createNode(ARKUI_NODE_IMAGE);
    ArkUI_AttributeItem NODE_IMAGE_SRC_Item = {.string = "./resources/base/media/icon.png"};
    ArkUI_NumberValue imageWidthValue[] = {100};
    ArkUI_AttributeItem imageWidthItem = {imageWidthValue, 1};
    ArkUI_NumberValue imageHeightValue[] = {100};
    ArkUI_AttributeItem imageHeightItem = {imageHeightValue, 1};
    ArkUI_NumberValue marginValue[] = {20};
    ArkUI_AttributeItem marginItem = {marginValue, 1};
    nodeAPI->setAttribute(g_image, NODE_WIDTH, &imageWidthItem);
    nodeAPI->setAttribute(g_image, NODE_HEIGHT, &imageHeightItem);
    nodeAPI->setAttribute(g_image, NODE_IMAGE_SRC, &NODE_IMAGE_SRC_Item);
    nodeAPI->setAttribute(g_image, NODE_MARGIN, &marginItem);
    auto dragArea = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NumberValue heightValue1[] = {150};
    ArkUI_AttributeItem heightItem = {heightValue1, 1};
    ArkUI_NumberValue widthValue[] = {200};
    ArkUI_AttributeItem widthItem = {widthValue, 1};
    ArkUI_NumberValue borderValue[] = {1};
    ArkUI_AttributeItem borderItem = {borderValue, 1};
    nodeAPI->setAttribute(dragArea, NODE_WIDTH, &widthItem);
    nodeAPI->setAttribute(dragArea, NODE_HEIGHT, &heightItem);
    nodeAPI->setAttribute(dragArea, NODE_BORDER_WIDTH, &borderItem);
    nodeAPI->registerNodeEvent(g_image, NODE_ON_CLICK, ON_CLICK_EVENT_ID, nullptr);
    OH_ArkUI_AllowNodeAllDropDataTypes(dragArea);
    OH_ArkUI_DisableDropDataPrefetchOnNode(dragArea, dropDataPrefetchOnNode);
    ArkUI_AttributeItem id_item = {};
    id_item.string = "g_image";
    nodeAPI->setAttribute(g_image, NODE_ID, &id_item);
    id_item.string = "dragArea";
    nodeAPI->setAttribute(dragArea, NODE_ID, &id_item);
    nodeAPI->addChild(column, g_image);
    nodeAPI->addChild(column, dragArea);
    return column;
}

napi_value GetNodeSnapshotTest_001(napi_env env, napi_callback_info info)
{
    auto column = CreateColumn(true);
    auto g_options = OH_ArkUI_CreateSnapshotOptions();
    uint8_t data[PARAM_96];
    for (auto i = 0; i < PARAM_96; i = i + PARAM_4) {
        data[i] = uint8_t(0);
        data[i + PARAM_1] = uint8_t(0);
        data[i + PARAM_2] = uint8_t(0);
        data[i + PARAM_3] = uint8_t(PARAM_255);
    };
    int result = OH_ArkUI_GetNodeSnapshot(column, g_options, &g_pixelMap);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetNodeSnapshotTest_002(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPi);
    // 创建指定类型的节点
    auto column = nullptr;
    auto options = OH_ArkUI_CreateSnapshotOptions();
    int result = OH_ArkUI_GetNodeSnapshot(column, options, &g_pixelMap);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetNodeSnapshotTest_003(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPi);
    // 创建指定类型的节点
    auto column = g_nodeAPi->createNode(ARKUI_NODE_COLUMN);
    auto options = OH_ArkUI_CreateSnapshotOptions();
    int result = OH_ArkUI_GetNodeSnapshot(column, options, &g_pixelMap);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetNodeSnapshotTest_004(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPi);
    // 创建指定类型的节点
    auto column = nullptr;
    auto options = OH_ArkUI_CreateSnapshotOptions();
    int result = OH_ArkUI_GetNodeSnapshot(column, options, &g_pixelMap);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}
}
 