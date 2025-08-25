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
#include "NodeAdapterTest.h"
static int g_totalNodeCnt = 1000;
static uint32_t g_from = 1;
static uint32_t g_to = 2;
static uint32_t g_startPosition = 0;
static uint32_t g_itemCount = 10;
static int32_t g_ret1 = 2;
static int32_t g_ret2 = 2;
static int32_t g_ret3 = 2;
static int32_t g_ret4 = 2;
static int32_t g_ret5 = 2;
static int32_t g_ret6 = 2;
static int32_t g_ret7 = 2;
static int32_t g_ret8 = 2;
static ArkUI_NodeAdapterHandle g_adapter = nullptr;
static ArkUI_NativeNodeAPI_1 *nativenodeAPI = nullptr;
namespace NodeAdapterTest {
napi_value SetTotalNodeCountTest_001(napi_env env, napi_callback_info info)
{
    ArkUI_NodeAdapterHandle adapter = OH_ArkUI_NodeAdapter_Create();
    int result = OH_ArkUI_NodeAdapter_SetTotalNodeCount(adapter, g_totalNodeCnt);
    OH_ArkUI_NodeAdapter_Dispose(adapter);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value SetTotalNodeCountTest_002(napi_env env, napi_callback_info info)
{
    int result = OH_ArkUI_NodeAdapter_SetTotalNodeCount(nullptr, g_totalNodeCnt);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RegisterEventReceiverTest_001(napi_env env, napi_callback_info info)
{
    int result = OH_ArkUI_NodeAdapter_RegisterEventReceiver(nullptr, 0, nullptr);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value ReloadAllItemsTest_001(napi_env env, napi_callback_info info)
{
    // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, nativenodeAPI);
    // 创建指定类型的节点
    auto g_handle = nativenodeAPI->createNode(ARKUI_NODE_LIST);
    ArkUI_NodeAdapterHandle adapter = OH_ArkUI_NodeAdapter_Create();
    ArkUI_AttributeItem item{nullptr, 0, nullptr, adapter};
    nativenodeAPI->setAttribute(g_handle, NODE_LIST_NODE_ADAPTER, &item);
    g_ret1 = OH_ArkUI_NodeAdapter_ReloadAllItems(adapter);
    int result = OH_ArkUI_NodeAdapter_ReloadAllItems(adapter);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value ReloadAllItemsTest_002(napi_env env, napi_callback_info info)
{
    int result = OH_ArkUI_NodeAdapter_ReloadAllItems(nullptr);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value ReloadItemTest_001(napi_env env, napi_callback_info info)
{
    ArkUI_NodeAdapterHandle adapter = OH_ArkUI_NodeAdapter_Create();
    int result = OH_ArkUI_NodeAdapter_ReloadItem(adapter, g_startPosition, g_itemCount);
    OH_ArkUI_NodeAdapter_Dispose(adapter);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value ReloadItemTest_002(napi_env env, napi_callback_info info)
{
    int result = OH_ArkUI_NodeAdapter_ReloadItem(nullptr, g_startPosition, g_itemCount);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RemoveItemTest_001(napi_env env, napi_callback_info info)
{
    ArkUI_NodeAdapterHandle adapter = OH_ArkUI_NodeAdapter_Create();
    int result = OH_ArkUI_NodeAdapter_RemoveItem(adapter, g_startPosition, g_itemCount);
    OH_ArkUI_NodeAdapter_Dispose(adapter);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value RemoveItemTest_002(napi_env env, napi_callback_info info)
{
    int result = OH_ArkUI_NodeAdapter_RemoveItem(nullptr, g_startPosition, g_itemCount);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value InsertItemTest_001(napi_env env, napi_callback_info info)
{
    ArkUI_NodeAdapterHandle adapter = OH_ArkUI_NodeAdapter_Create();
    int result = OH_ArkUI_NodeAdapter_InsertItem(adapter, g_startPosition, g_itemCount);
    OH_ArkUI_NodeAdapter_Dispose(adapter);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value InsertItemTest_002(napi_env env, napi_callback_info info)
{
    int result = OH_ArkUI_NodeAdapter_InsertItem(nullptr, g_startPosition, g_itemCount);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value MoveItemTest_001(napi_env env, napi_callback_info info)
{
    ArkUI_NodeAdapterHandle adapter = OH_ArkUI_NodeAdapter_Create();
    int result = OH_ArkUI_NodeAdapter_MoveItem(adapter, g_from, g_to);
    OH_ArkUI_NodeAdapter_Dispose(adapter);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value MoveItemTest_002(napi_env env, napi_callback_info info)
{
    int result = OH_ArkUI_NodeAdapter_MoveItem(nullptr, g_from, g_to);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetAllItemsTest_001(napi_env env, napi_callback_info info)
{
    ArkUI_NodeAdapterHandle adapter = OH_ArkUI_NodeAdapter_Create();
    uint32_t size = 2;
    int result = OH_ArkUI_NodeAdapter_GetAllItems(adapter, nullptr, &size);
    OH_ArkUI_NodeAdapter_Dispose(adapter);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value GetAllItemsTest_002(napi_env env, napi_callback_info info)
{
    int result = OH_ArkUI_NodeAdapter_GetAllItems(nullptr, nullptr, 0);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

struct UserCallback {
    std::function<void(void *event)> callback;
};

napi_value CreateAdapterNode(napi_env env, napi_callback_info info)
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
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, nativenodeAPI);
    auto list = nativenodeAPI->createNode(ARKUI_NODE_LIST);
    ArkUI_NodeAdapterHandle g_adapter = OH_ArkUI_NodeAdapter_Create();
    struct UserCallback *adapterCallback = new UserCallback();
    adapterCallback->callback = [&](void *event) {
        auto *adapterEvent = reinterpret_cast<ArkUI_NodeAdapterEvent *>(event);
        auto type = OH_ArkUI_NodeAdapterEvent_GetType(adapterEvent);
        OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "UIInputTEvent", "Trigger");
        switch (type) {
            case NODE_ADAPTER_EVENT_WILL_ATTACH_TO_NODE:{
                // 通知Adapter进行局部元素插入，插入10个
                g_ret4 = OH_ArkUI_NodeAdapter_InsertItem(g_adapter, g_startPosition, g_itemCount);
                OH_LOG_Print(LOG_APP, LOG_INFO, LOG_PRINT_DOMAIN, "UIInputTEvent", "Trigger");
                // 通知Adapter进行局部元素移位
                g_ret5 = OH_ArkUI_NodeAdapter_MoveItem(g_adapter, g_from, g_to);
                // 通知Adapter进行全量元素变化
                g_ret1 = OH_ArkUI_NodeAdapter_ReloadAllItems(g_adapter);
                // 通知Adapter进行局部元素变化
                g_ret2 = OH_ArkUI_NodeAdapter_ReloadItem(g_adapter, g_startPosition, g_itemCount);
                // 通知Adapter进行局部元素删除
                g_ret3 = OH_ArkUI_NodeAdapter_RemoveItem(g_adapter, g_startPosition, g_itemCount);
            }
                break;
            default:
                break;
        }
    };
    // 注册事件回调函数
    int32_t ret = OH_ArkUI_NodeAdapter_RegisterEventReceiver(g_adapter, adapterCallback, nullptr);
    // 把adapter挂载到list上
    ArkUI_AttributeItem item{nullptr, 0, nullptr, g_adapter};
    ret = nativenodeAPI->setAttribute(list, NODE_LIST_NODE_ADAPTER, &item);
    napi_value exports;
    if (napi_create_object(env, &exports) != napi_ok) {
        napi_throw_type_error(env, nullptr, "napi_create_object failed");
        return nullptr;
    }
    return exports;
}

napi_value SetItemTest_001(napi_env env, napi_callback_info info)
{
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, nativenodeAPI);
    // 创建指定类型的节点
    auto g_handle = nativenodeAPI->createNode(ARKUI_NODE_LIST);
    ArkUI_NodeAdapterHandle adapter = OH_ArkUI_NodeAdapter_Create();
    ArkUI_NodeAdapterEvent* event = nullptr;
    int result = OH_ArkUI_NodeAdapterEvent_SetItem(event, g_handle);
    OH_ArkUI_NodeAdapter_Dispose(adapter);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value SetItemTest_002(napi_env env, napi_callback_info info)
{
    int result = OH_ArkUI_NodeAdapterEvent_SetItem(nullptr, nullptr);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value SetNodeIdTest_001(napi_env env, napi_callback_info info)
{
     // 获取ArkUI Native Node API接口
    OH_ArkUI_GetModuleInterface(ARKUI_NATIVE_NODE, ArkUI_NativeNodeAPI_1, nativenodeAPI);
    // 创建指定类型的节点
    auto g_handle = nativenodeAPI->createNode(ARKUI_NODE_LIST);
    ArkUI_NodeAdapterHandle adapter = OH_ArkUI_NodeAdapter_Create();
    ArkUI_NodeAdapterEvent* event = nullptr;
    int result = OH_ArkUI_NodeAdapterEvent_SetNodeId(event, 1);
    OH_ArkUI_NodeAdapter_Dispose(adapter);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}

napi_value SetNodeIdTest_002(napi_env env, napi_callback_info info)
{
    int result = OH_ArkUI_NodeAdapterEvent_SetNodeId(nullptr, 1);
    napi_value retValue;
    napi_create_int32(env, result, &retValue);
    return retValue;
}
}