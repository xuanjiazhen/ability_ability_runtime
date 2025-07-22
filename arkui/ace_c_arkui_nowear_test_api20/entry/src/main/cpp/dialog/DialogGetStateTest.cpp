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

#include "DialogGetStateTest.h"
#include "../manager/plugin_manager.h"
#include <arkui/native_dialog.h>
#include <string>
 
namespace ArkUICapiTest {
    ArkUI_NativeDialogHandle globalCustomDialog = nullptr;
    ArkUI_NodeHandle text3 = nullptr;

    static auto SetWidth(ArkUI_NativeNodeAPI_1* nodeAPI, ArkUI_NodeHandle& nodeHandle, float width)
    {
        ArkUI_NumberValue width_value[] = { { .f32 = width } };
        ArkUI_AttributeItem width_item = { width_value, sizeof(width_value) / sizeof(ArkUI_NumberValue) };
        nodeAPI->setAttribute(nodeHandle, NODE_WIDTH, &width_item);
        return nodeHandle;
    }

    static auto SetHeight(ArkUI_NativeNodeAPI_1* nodeAPI, ArkUI_NodeHandle& nodeHandle, float height)
    {
        ArkUI_NumberValue height_value[] = { { .f32 = height } };
        ArkUI_AttributeItem height_item = { height_value, sizeof(height_value) / sizeof(ArkUI_NumberValue) };
        nodeAPI->setAttribute(nodeHandle, NODE_HEIGHT, &height_item);
        return nodeHandle;
    }

    static auto SetBackgroundColor(ArkUI_NativeNodeAPI_1* nodeAPI, ArkUI_NodeHandle nodeHandle, uint32_t color)
    {
        ArkUI_NumberValue color_value[] = { { .u32 = color } };
        ArkUI_AttributeItem color_item = { color_value, sizeof(color_value) / sizeof(ArkUI_NumberValue) };
        nodeAPI->setAttribute(nodeHandle, NODE_BACKGROUND_COLOR, &color_item);
        return nodeHandle;
    }

    static auto SetDialog(ArkUI_NativeNodeAPI_1* nodeAPI, ArkUI_NodeHandle& nodeHandle)
    {
        nodeHandle = SetWidth(nodeAPI, nodeHandle, SIZE_200);
        nodeHandle = SetHeight(nodeAPI, nodeHandle, SIZE_300);
        nodeHandle = SetBackgroundColor(nodeAPI, nodeHandle, COLOR_BLUE);
        return nodeHandle;
    }

    static auto createChildNode(ArkUI_NativeNodeAPI_1 *nodeAPI, bool enabled)
    {
        auto nodeHandle = nodeAPI->createNode(ARKUI_NODE_BUTTON);

        ArkUI_NumberValue enable_value[] = {{.i32 = enabled}};
        ArkUI_AttributeItem enable_item = {enable_value, sizeof(enable_value) / sizeof(ArkUI_NumberValue)};
        nodeAPI->setAttribute(nodeHandle, NODE_ENABLED, &enable_item);

        ArkUI_NumberValue width_value[] = {{.f32 = SIZE_100}};
        ArkUI_AttributeItem width_item = {width_value, sizeof(width_value) / sizeof(ArkUI_NumberValue)};
        nodeAPI->setAttribute(nodeHandle, NODE_WIDTH, &width_item);
        nodeAPI->setAttribute(nodeHandle, NODE_HEIGHT, &width_item);

        ArkUI_NumberValue margin_value[] = {{.f32 = DEFAULT_MARGIN}};
        ArkUI_AttributeItem margin_item = {margin_value, sizeof(margin_value) / sizeof(ArkUI_NumberValue)};
        nodeAPI->setAttribute(nodeHandle, NODE_MARGIN, &margin_item);

        ArkUI_NumberValue background_color_value[] = {{.u32 = COLOR_RED}};
        ArkUI_AttributeItem background_color_item = {background_color_value,
                                                     sizeof(background_color_value) / sizeof(ArkUI_NumberValue)};
        nodeAPI->setAttribute(nodeHandle, NODE_BACKGROUND_COLOR, &background_color_item);

        nodeAPI->registerNodeEvent(nodeHandle, NODE_ON_CLICK, ON_CLICK_EVENT_ID, nullptr);

        return nodeHandle;
    }

    static auto createChildNode1(ArkUI_NativeNodeAPI_1 *nodeAPI, bool enabled)
    {
        auto nodeHandle = nodeAPI->createNode(ARKUI_NODE_BUTTON);

        ArkUI_NumberValue enable_value[] = {{.i32 = enabled}};
        ArkUI_AttributeItem enable_item = {enable_value, sizeof(enable_value) / sizeof(ArkUI_NumberValue)};
        nodeAPI->setAttribute(nodeHandle, NODE_ENABLED, &enable_item);

        ArkUI_NumberValue width_value[] = {{.f32 = SIZE_100}};
        ArkUI_AttributeItem width_item = {width_value, sizeof(width_value) / sizeof(ArkUI_NumberValue)};
        nodeAPI->setAttribute(nodeHandle, NODE_WIDTH, &width_item);
        nodeAPI->setAttribute(nodeHandle, NODE_HEIGHT, &width_item);

        ArkUI_NumberValue margin_value[] = {{.f32 = DEFAULT_MARGIN}};
        ArkUI_AttributeItem margin_item = {margin_value, sizeof(margin_value) / sizeof(ArkUI_NumberValue)};
        nodeAPI->setAttribute(nodeHandle, NODE_MARGIN, &margin_item);

        ArkUI_NumberValue background_color_value[] = {{.u32 = COLOR_RED}};
        ArkUI_AttributeItem background_color_item = {background_color_value,
                                                     sizeof(background_color_value) / sizeof(ArkUI_NumberValue)};
        nodeAPI->setAttribute(nodeHandle, NODE_BACKGROUND_COLOR, &background_color_item);

        nodeAPI->registerNodeEvent(nodeHandle, NODE_ON_CLICK, ON_CLICK_EVENT_ID2, nullptr);

        return nodeHandle;
    }

    static void OnEventReceive(ArkUI_NodeEvent *event)
    {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "DialogGetStateTest", "OnEventReceive");
        if (event == nullptr) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "DialogGetStateTest", "OnEventReceive: event is null");
            return;
        }

        int32_t eventId = OH_ArkUI_NodeEvent_GetTargetId(event);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "DialogGetStateTest", "OnEventReceive eventId: %{public}d",
                     eventId);

        ArkUI_NativeNodeAPI_1 *nodeAPI = nullptr;
        OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, nodeAPI);
        ArkUI_NativeDialogAPI_1* dialogAPI = nullptr;
        OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_DIALOG, ArkUI_NativeDialogAPI_1, dialogAPI);
    
        if (eventId == ON_CLICK_EVENT_ID) {
            auto column1 = nodeAPI->createNode(ARKUI_NODE_COLUMN);
            auto textInput = nodeAPI->createNode(ARKUI_NODE_TEXT_INPUT);
            nodeAPI->addChild(column1, textInput);

            ArkUI_NumberValue a_value[] = { { .i32 = 1 } };
            ArkUI_AttributeItem a_item = { a_value, sizeof(a_value) / sizeof(ArkUI_NumberValue) };
            nodeAPI->setAttribute(textInput, NODE_TEXT_INPUT_CUSTOM_KEYBOARD, &a_item);

            ArkUI_AttributeItem b_item = {};
            b_item.string = "abc";
            nodeAPI->setAttribute(textInput, NODE_TEXT_INPUT_TEXT, &b_item);

            column1 = SetDialog(nodeAPI, column1);
            auto customDialog = dialogAPI->create();
            dialogAPI->setContent(customDialog, column1);
            dialogAPI->setBackgroundColor(customDialog, COLOR_GREEN);
            dialogAPI->setContentAlignment(customDialog, ARKUI_ALIGNMENT_TOP, PARAM_0, PARAM_0);
            dialogAPI->show(customDialog, true);

            ArkUI_DialogState state = DIALOG_APPEARED;
            int32_t result = OH_ArkUI_CustomDialog_GetState(customDialog, &state);
            std::string isState = std::to_string(state);
            const char* isState1 = isState.c_str();
            ArkUI_AttributeItem contentItem = {.string =  isState1};
            nodeAPI->setAttribute(text3, NODE_TEXT_CONTENT, &contentItem);
            globalCustomDialog = customDialog;
        }
    
        if (eventId == ON_CLICK_EVENT_ID2) {
            if (globalCustomDialog != nullptr) {
                ArkUI_DialogState state = DIALOG_DISAPPEARED;
                int32_t result1 = OH_ArkUI_CustomDialog_GetState(globalCustomDialog, &state);
                std::string isState = std::to_string(state);
                const char* isState1 = isState.c_str();
                ArkUI_AttributeItem contentItem = {.string =  isState1};
                nodeAPI->setAttribute(text3, NODE_TEXT_CONTENT, &contentItem);
        } else {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "CustomDialogContentTest",
                "Global custom dialog handle is null");
        }
        }
    }

    napi_value DialogGetStateTest::CreateNativeNode(napi_env env, napi_callback_info info)
    {
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "DialogGetStateTest", "CreateNativeNode");

        size_t argc = PARAM_1;
        napi_value args[PARAM_1] = {nullptr};
        napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
        size_t length = PARAM_64;
        size_t strLength = PARAM_0;
        char xComponentID[PARAM_64] = {PARAM_0};
        napi_get_value_string_utf8(env, args[PARAM_0], xComponentID, length, &strLength);

        if ((env == nullptr) || (info == nullptr)) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN,
                "DialogGetStateTest", "GetContext env or info is null");
            return nullptr;
        }

        ArkUI_NativeNodeAPI_1 *nodeAPI = nullptr;
        OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, nodeAPI);
        auto column = nodeAPI->createNode(ARKUI_NODE_COLUMN);
        auto button1 = createChildNode(nodeAPI, true);
        auto button2 = createChildNode1(nodeAPI, true);

        ArkUI_AttributeItem id_item = {};
        ArkUI_AttributeItem id_item1 = {};
        id_item.string = "GetStateButton";
        nodeAPI->setAttribute(button1, NODE_ID, &id_item);
        id_item1.string = "GetStateButton1";
        nodeAPI->setAttribute(button1, NODE_ID, &id_item);
        nodeAPI->setAttribute(button2, NODE_ID, &id_item1);
        nodeAPI->addChild(column, button1);
        nodeAPI->addChild(column, button2);
    
        text3 = nodeAPI->createNode(ARKUI_NODE_TEXT);
        ArkUI_AttributeItem id_item2 = {};
        id_item2.string = "textGetStates";
        nodeAPI->setAttribute(text3, NODE_ID, &id_item2);
        ArkUI_AttributeItem logMsg = {.string = "customDialogState"};
        nodeAPI->setAttribute(text3, NODE_TEXT_CONTENT, &logMsg);
        nodeAPI->addChild(column, text3);
        nodeAPI->registerNodeEventReceiver(&OnEventReceive);

        std::string id(xComponentID);
        if (OH_NativeXComponent_AttachNativeRootNode(PluginManager::GetInstance()->GetNativeXComponent(id), column)
        == INVALID_PARAM) {
            OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "DialogGetStateTest",
                "OH_NativeXComponent_AttachNativeRootNode failed");
        }

        napi_value exports;
        if (napi_create_object(env, &exports) != napi_ok) {
            napi_throw_type_error(env, nullptr, "napi_create_object failed");
            return nullptr;
        }

        return exports;
    }
} // namespace ArkUICapiTest