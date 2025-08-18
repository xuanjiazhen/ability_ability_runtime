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

#include "NodeEventTest.h"
#include "manager/PluginManagerTest.h"
#include <cstdint>

static ArkUI_NativeNodeAPI_1 *g_nativenodeAPI = nullptr;
static ArkUI_NumberValue values[2] = {{1.0}, {2.0}};
static int32_t g_test1ret1 = 2;
static int32_t g_test1ret2 = 2;
static int32_t g_test2ret1 = 2;
static int32_t g_test2ret2 = 2;
static int32_t g_test3ret1 = 2;
static int32_t g_test3ret2 = 2;

namespace NodeEventTest {
napi_value GetNumberValueTest_001(napi_env env, napi_callback_info info)
{
    napi_value retValue;
    napi_create_int32(env, g_test1ret1, &retValue);
    return retValue;
}

napi_value GetNumberValueTest_002(napi_env env, napi_callback_info info)
{
    napi_value retValue;
    napi_create_int32(env, g_test1ret2, &retValue);
    return retValue;
}

napi_value GetNumberValueTest_003(napi_env env, napi_callback_info info)
{
    ArkUI_NodeEvent* event = nullptr;
    ArkUI_NumberValue numberValue;
    int result = OH_ArkUI_NodeEvent_GetNumberValue(event, 0, &numberValue);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetStringValueTest_001(napi_env env, napi_callback_info info)
{
    napi_value retValue;
    napi_create_int32(env, g_test2ret1, &retValue);
    return retValue;
}

napi_value GetStringValueTest_002(napi_env env, napi_callback_info info)
{
    napi_value retValue;
    napi_create_int32(env, g_test2ret2, &retValue);
    return retValue;
}

napi_value GetStringValueTest_003(napi_env env, napi_callback_info info)
{
    ArkUI_NodeEvent* event = nullptr;
    char* str = nullptr;
    int32_t size = 0;
    int32_t result = OH_ArkUI_NodeEvent_GetStringValue(event, 0, &str, &size);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value SetReturnNumberValueTest_001(napi_env env, napi_callback_info info)
{
    napi_value retValue;
    napi_create_int32(env, g_test3ret1, &retValue);
    return retValue;
}

napi_value SetReturnNumberValueTest_002(napi_env env, napi_callback_info info)
{
    ArkUI_NumberValue values[2] = {{1.0}, {2.0}};
    int32_t result = OH_ArkUI_NodeEvent_SetReturnNumberValue(nullptr, values, 1);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value SetReturnNumberValueTest_003(napi_env env, napi_callback_info info)
{
    ArkUI_NumberValue values[1] = {{0}};
    int32_t result = OH_ArkUI_NodeEvent_SetReturnNumberValue(nullptr, values, 1);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value CreateNode(napi_env env, napi_callback_info info)
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
    // 1. 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nativenodeAPI);
    auto nodeHandle = g_nativenodeAPI->createNode(ARKUI_NODE_BUTTON);
    ArkUI_NumberValue enable_value[] = {{.i32 = true}};
    ArkUI_AttributeItem enable_item = {enable_value, sizeof(true) / sizeof(ArkUI_NumberValue)};
    g_nativenodeAPI->setAttribute(nodeHandle, NODE_ENABLED, &enable_item);
    ArkUI_NumberValue width_value[] = {{.f32 = SIZE_100}};
    ArkUI_AttributeItem width_item = {width_value, sizeof(width_value) / sizeof(ArkUI_NumberValue)};
    g_nativenodeAPI->setAttribute(nodeHandle, NODE_WIDTH, &width_item);
    g_nativenodeAPI->setAttribute(nodeHandle, NODE_HEIGHT, &width_item);
    ArkUI_NumberValue margin_value[] = {{.f32 = DEFAULT_MARGIN}};
    ArkUI_AttributeItem margin_item = {margin_value, sizeof(margin_value) / sizeof(ArkUI_NumberValue)};
    g_nativenodeAPI->setAttribute(nodeHandle, NODE_MARGIN, &margin_item);
    ArkUI_NumberValue background_color_value[] = {{.u32 = COLOR_RED}};
    ArkUI_AttributeItem background_color_item = {background_color_value,
        sizeof(background_color_value) / sizeof(ArkUI_NumberValue)};
    g_nativenodeAPI->setAttribute(nodeHandle, NODE_BACKGROUND_COLOR, &background_color_item);
    g_nativenodeAPI->registerNodeEventReceiver([](ArkUI_NodeEvent *event) {
        ArkUI_NumberValue numberValue;
        int32_t* stringSize;
        g_test1ret1 = OH_ArkUI_NodeEvent_GetNumberValue(event, 1, &numberValue);
        g_test1ret2 = OH_ArkUI_NodeEvent_GetNumberValue(event, -1, nullptr);
        g_test2ret1 = OH_ArkUI_NodeEvent_GetStringValue(event, 1, nullptr, stringSize);
        g_test2ret2 = OH_ArkUI_NodeEvent_GetStringValue(event, -1, nullptr, stringSize);
        g_test3ret1 = OH_ArkUI_NodeEvent_SetReturnNumberValue(event, values, 1);
    });
    g_nativenodeAPI->registerNodeEvent(nodeHandle, NODE_ON_KEY_EVENT, 1, nullptr);
    std::string id(xComponentID);
    if (OH_NativeXComponent_AttachNativeRootNode(ArkUICapiTest::PluginManager::GetInstance()->GetNativeXComponent(id),
                                                 nodeHandle) == INVALID_PARAM) {
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
}