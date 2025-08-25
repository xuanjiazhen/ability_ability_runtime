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
#include "manager/PluginManagerTest.h"
#include "RunTaskInScopeTest.h"
#include <arkui/native_node.h>
#include <string>
#include <arkui/native_node_napi.h>
static ArkUI_NativeNodeAPI_1 *g_nodeAPI_ = nullptr;
static int NUMBER_3 = 3;
static int NUMBER_4 = 4;
static int NUMBER_5 = 5;
static int NUMBER_6 = 6;
static int g_snapresult1 = 2;
static int g_snapresult2 = 2;
static int g_snapresult3 = 2;
static int g_snapresult4 = 2;
struct Test {
    ArkUI_NodeHandle pageOneButton1 = nullptr;
    ArkUI_NodeHandle button5 = nullptr;
    ArkUI_NodeHandle column = nullptr;
};
static Test *g_test1 = new Test();
namespace RunTaskInScopeTest {

static ArkUI_NodeHandle CreateChildNode(ArkUI_NativeNodeAPI_1 *nodeAPI)
{
    ArkUI_NodeHandle column = nodeAPI->createNode(ARKUI_NODE_COLUMN);
    ArkUI_NodeHandle button2 = nodeAPI->createNode(ARKUI_NODE_BUTTON);
    nodeAPI->registerNodeEvent(button2, NODE_ON_CLICK, NUMBER_3, nullptr);
    ArkUI_AttributeItem LABEL_Item2 = {.string = "uiContext"};
    nodeAPI->setAttribute(button2, NODE_BUTTON_LABEL, &LABEL_Item2);
    ArkUI_AttributeItem id2 = {.string = "btn2_id"};
    nodeAPI->setAttribute(button2, NODE_ID, &id2);
    
    ArkUI_NodeHandle button3 = nodeAPI->createNode(ARKUI_NODE_BUTTON);
    nodeAPI->registerNodeEvent(button3, NODE_ON_CLICK, NUMBER_4, nullptr);
    ArkUI_AttributeItem LABEL_Item3 = {.string = "uiContext 为空"};
    nodeAPI->setAttribute(button3, NODE_BUTTON_LABEL, &LABEL_Item3);
    ArkUI_AttributeItem id3 = {.string = "btn3_id"};
    nodeAPI->setAttribute(button3, NODE_ID, &id3);
    
    ArkUI_NodeHandle button4 = nodeAPI->createNode(ARKUI_NODE_BUTTON);
    nodeAPI->registerNodeEvent(button4, NODE_ON_CLICK, NUMBER_5, nullptr);
    ArkUI_AttributeItem LABEL_Item4 = {.string = "callBack 为空"};
    nodeAPI->setAttribute(button4, NODE_BUTTON_LABEL, &LABEL_Item4);
    ArkUI_AttributeItem id4 = {.string = "btn4_id"};
    nodeAPI->setAttribute(button4, NODE_ID, &id4);

    ArkUI_NodeHandle button5 = nodeAPI->createNode(ARKUI_NODE_BUTTON);
    nodeAPI->registerNodeEvent(button5, NODE_ON_CLICK, NUMBER_6, nullptr);
    ArkUI_AttributeItem LABEL_Item5 = {.string = "change uiContext2"};
    nodeAPI->setAttribute(button5, NODE_BUTTON_LABEL, &LABEL_Item5);
    ArkUI_AttributeItem id0 = {.string = "uiContext2"};
    nodeAPI->setAttribute(button5, NODE_ID, &id0);

    nodeAPI->addChild(column, button3);
    nodeAPI->addChild(column, button4);
    nodeAPI->addChild(column, button5);
    
    ArkUI_NodeHandle pageOneButton1 = nullptr;
    auto ec = OH_ArkUI_NodeUtils_GetAttachedNodeHandleById("pageOneButton1", &pageOneButton1);
    ec = OH_ArkUI_NodeUtils_GetAttachedNodeHandleById("uiContext2", &button5);
    g_test1->pageOneButton1 = pageOneButton1;
    g_test1->button5 = button5;
    g_test1->column = column;
    return column;
}
napi_value testRunTaskInScope(napi_env env, napi_callback_info info)
{
    OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "testRunTaskInScope", "CreateNativeNode");
    size_t argc = PARAM_1;
    napi_value args[PARAM_1] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    size_t length = PARAM_64;
    size_t strLength = PARAM_0;
    char xComponentID[PARAM_64] = {PARAM_0};
    napi_get_value_string_utf8(env, args[PARAM_0], xComponentID, length, &strLength);
    if ((env == nullptr) || (info == nullptr)) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "testRunTaskInScope",
            "GetContext env or info is null");
        return nullptr;
    }
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI_);
    ArkUI_NodeHandle column = CreateChildNode(g_nodeAPI_);
 
    std::string id(xComponentID);
    if (OH_NativeXComponent_AttachNativeRootNode(ArkUICapiTest::PluginManager::GetInstance()->
                                                 GetNativeXComponent(id), column) ==INVALID_PARAM) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "testRunTaskInScope",
                     "OH_NativeXComponent_AttachNativeRootNode failed");
    }
    napi_value exports;
    if (napi_create_object(env, &exports) != napi_ok) {
        napi_throw_type_error(env, nullptr, "napi_create_object failed");
        return nullptr;
    }
    return exports;
}

napi_value RunTaskInScopeTest_001(napi_env env, napi_callback_info info)
{
    auto uiContext = OH_ArkUI_GetContextByNode(g_test1->button5);
    g_snapresult1 = OH_ArkUI_RunTaskInScope(uiContext, g_test1, [](void *userData) {
        auto test = (Test *)userData;
        ArkUI_AttributeItem LABEL_Item = {.string = "success"};
        g_nodeAPI_->setAttribute(test->pageOneButton1, NODE_BUTTON_LABEL, &LABEL_Item);
    });
    napi_value retValue;
    napi_create_int32(env, g_snapresult1, &retValue);
    return retValue;
}

napi_value RunTaskInScopeTest_002(napi_env env, napi_callback_info info)
{
    auto uiContext = OH_ArkUI_GetContextByNode(g_test1->button5);
    g_snapresult2 = OH_ArkUI_RunTaskInScope(uiContext, nullptr, nullptr);
    napi_value retValue;
    napi_create_int32(env, g_snapresult2, &retValue);
    return retValue;
}

napi_value RunTaskInScopeTest_003(napi_env env, napi_callback_info info)
{
    auto uiContext = OH_ArkUI_GetContextByNode(g_test1->button5);
    g_snapresult3 = OH_ArkUI_RunTaskInScope(uiContext, g_test1, nullptr);
    napi_value retValue;
    napi_create_int32(env, g_snapresult3, &retValue);
    return retValue;
}

napi_value RunTaskInScopeTest_004(napi_env env, napi_callback_info info)
{
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, g_nodeAPI_);
    // 创建指定类型的节点
    auto uiContext = nullptr;
    g_snapresult4 = OH_ArkUI_RunTaskInScope(uiContext, g_test1, [](void *userData) {
                auto test = (Test *)userData;
                ArkUI_AttributeItem LABEL_Item = {.string = "success"};
                g_nodeAPI_->setAttribute(test->pageOneButton1, NODE_BUTTON_LABEL, &LABEL_Item);
            });
    napi_value retValue;
    napi_create_int32(env, g_snapresult4, &retValue);
    return retValue;
}

}