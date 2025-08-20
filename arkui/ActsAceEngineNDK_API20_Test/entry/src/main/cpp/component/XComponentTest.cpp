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

#include "component/XComponent.h"

namespace ArkUICapiTest {

static XComponent g_columnComponent = XComponent(ARKUI_NODE_COLUMN);
static XComponent g_textComponent = XComponent(ARKUI_NODE_TEXT);
static XComponent* g_xComponent = new XComponent(ARKUI_NODE_XCOMPONENT);
static bool g_flag = false;
static int32_t test = -2;

napi_value InitTextComponent(napi_env env, napi_callback_info info)
{
    ArkUI_AttributeItem item = { .string = "init" };
    g_textComponent.setAttribute(NODE_TEXT_CONTENT, &item);
    return nullptr;
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
    g_columnComponent.SetPercentWidth(PARAM_0_POINT_6);
    g_columnComponent.SetPercentHeight(PARAM_0_POINT_6);
    g_columnComponent.SetBackgroundColor(0xffff0000);
    g_textComponent.SetPercentWidth(PARAM_1);
    g_textComponent.SetHeight(200);
    g_textComponent.SetBackgroundColor(0xff00ff00);
    ArkUI_AttributeItem item = { .string = "init" };
    g_textComponent.setAttribute(NODE_TEXT_CONTENT, &item);
    g_textComponent.SetId("NativeXComponentKeyCodeText");
    g_xComponent->SetXComponentType(ARKUI_XCOMPONENT_TYPE_SURFACE);
    g_xComponent->SetXComponentId("TestXComponent");
    g_xComponent->SetPercentWidth(PARAM_1);
    g_xComponent->SetPercentHeight(PARAM_1);
    g_xComponent->SetId("NativeXComponentKeyTest");
    g_xComponent->SetFocusable(true);
    g_xComponent->SetFocusDefault(true);
    g_xComponent->PrintXComponentInfo();
    g_xComponent->RegisterOnKey(std::function<void (ArkUI_NodeEvent *)> (OnKeyEvent));
    std::shared_ptr<Component> ptr = std::make_shared<XComponent>(*g_xComponent);
    g_columnComponent.AddChild(ptr);
    ptr = std::make_shared<XComponent>(g_textComponent);
    g_columnComponent.AddChild(ptr);
    int32_t ret = OH_ArkUI_NodeContent_SetUserData(nodeContentHandle_, &g_xComponent);
    if (ret != ARKUI_ERROR_CODE_NO_ERROR) {
        OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "PluginManager", "setUserData failed error=%{public}d", ret);
    }
    ret = OH_ArkUI_NodeContent_AddNode(nodeContentHandle_, g_columnComponent.GetComponent());
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
    OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "OnFocusEvent", "OnFocusEvent");
    return;
}

void OnBlurEvent(OH_NativeXComponent* component, void* window)
{
    OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "OnFocusEvent", "OnTouchInterceptCallback");
    return;
}

void OnKeyEvent(ArkUI_NodeEvent* event)
{
    OH_NativeXComponent_KeyCode keyCode = OH_NativeXComponent_KeyCode(-2);
    OH_LOG_Print(LOG_APP, LOG_ERROR, LOG_PRINT_DOMAIN, "OnKeyEvent", "OH_NativeXComponent_KeyCode keyCode: %{public}d", keyCode);
    OH_NativeXComponent_KeyEvent* keyEvent;
    OH_NativeXComponent* triggerObject = static_cast<OH_NativeXComponent*>(OH_ArkUI_NodeEvent_GetUserData(event));
    // 对比XComopnet指针确定是否为目标组件触发
    if (triggerObject == g_xComponent->GetXComponent()) {
        OH_NativeXComponent_GetKeyEvent(triggerObject, &keyEvent);
        OH_NativeXComponent_GetKeyEventCode(keyEvent, &keyCode);
        ArkUI_AttributeItem item = { .string = std::to_string(keyCode).data() };
        g_textComponent.setAttribute(NODE_TEXT_CONTENT, &item);
    }
    g_flag = true;
}

}
