/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "insight_intent/gui_intent_executor.h"

#include <thread>

#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"
#include "ui_content_proxy.h"
#include "ui_content_service_interface.h"
#include "window_manager.h"

namespace OHOS {
namespace AAFwk {

namespace {
constexpr int32_t TREE_REQUEST_TIMEOUT_MS = 3000;
constexpr int32_t PAGE_CHANGE_POLL_INTERVAL_MS = 50;

// ArkUI Pattern::OnRecvCommand expects JSON: {"cmd":"<action>","params":{...}}
std::string BuildCommandJson(const std::string& action)
{
    return R"({"cmd":")" + action + R"("})";
}
} // anonymous namespace

// ============================================================
// GuiIntentInputItem::ParseFromJson
// Input format (as designed):
//   [{
//     "pageName": "main",
//     "intentName": "openXxx",
//     "parameters": {
//       "compId": "...", "compType": "Button", "compText": "...",
//       "actionType": "click", "nextPageName": "...", "compIndex": 0
//     }
//   }, ...]
// Also supports flat format (backward compat):
//   [{"pageName":"main","intentName":"x","compType":"Button",...}, ...]
// ============================================================

namespace {
constexpr const char* PARAM_FIELD_NAME = "parameters";

void ParseMatchingFields(const nlohmann::json& src, GuiIntentInputItem& item)
{
    if (src.contains("compId") && src["compId"].is_string()) {
        item.compId = src["compId"].get<std::string>();
    }
    if (src.contains("compType") && src["compType"].is_string()) {
        item.compType = src["compType"].get<std::string>();
    }
    if (src.contains("compName") && src["compName"].is_string()) {
        item.compName = src["compName"].get<std::string>();
    }
    if (src.contains("compText") && src["compText"].is_string()) {
        item.compText = src["compText"].get<std::string>();
    }
    if (src.contains("compIndex") && src["compIndex"].is_number_integer()) {
        item.compIndex = src["compIndex"].get<int32_t>();
    }
    if (src.contains("nextPageName") && src["nextPageName"].is_string()) {
        item.nextPageName = src["nextPageName"].get<std::string>();
    }
    if (src.contains("actionType") && src["actionType"].is_string()) {
        item.actionType = src["actionType"].get<std::string>();
    }
}
} // anonymous namespace

std::vector<GuiIntentInputItem> GuiIntentInputItem::ParseFromJson(const std::string& inputJson)
{
    std::vector<GuiIntentInputItem> results;
    if (inputJson.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "ParseFromJson: empty input");
        return results;
    }

    try {
        auto jsonArr = nlohmann::json::parse(inputJson);
        if (!jsonArr.is_array()) {
            TAG_LOGE(AAFwkTag::INTENT, "ParseFromJson: top-level is not an array");
            return results;
        }

        for (const auto& jsonItem : jsonArr) {
            GuiIntentInputItem inputItem;
            if (jsonItem.contains("pageName") && jsonItem["pageName"].is_string()) {
                inputItem.pageName = jsonItem["pageName"].get<std::string>();
            }
            if (jsonItem.contains("intentName") && jsonItem["intentName"].is_string()) {
                inputItem.intentName = jsonItem["intentName"].get<std::string>();
            }

            // "parameters" sub-object takes priority (as-designed format)
            if (jsonItem.contains(PARAM_FIELD_NAME) && jsonItem[PARAM_FIELD_NAME].is_object()) {
                ParseMatchingFields(jsonItem[PARAM_FIELD_NAME], inputItem);
            } else {
                // Fallback: read fields from top-level (flat compat format)
                ParseMatchingFields(jsonItem, inputItem);
            }

            // Validate required fields
            if (inputItem.pageName.empty() || inputItem.intentName.empty() ||
                inputItem.compType.empty() || inputItem.actionType.empty()) {
                TAG_LOGW(AAFwkTag::INTENT, "ParseFromJson: skipped item with missing required fields: "
                    "pageName=%{public}s, intentName=%{public}s, compType=%{public}s, actionType=%{public}s",
                    inputItem.pageName.c_str(), inputItem.intentName.c_str(),
                    inputItem.compType.c_str(), inputItem.actionType.c_str());
                continue;
            }
            results.push_back(inputItem);
        }

        TAG_LOGI(AAFwkTag::INTENT, "ParseFromJson: parsed %{public}zu items", results.size());
    } catch (const nlohmann::json::exception& e) {
        TAG_LOGE(AAFwkTag::INTENT, "ParseFromJson: JSON parse error: %{public}s", e.what());
    }

    return results;
}

// ============================================================
// BuildHardcodedData — hardcoded decorator info (replace with DB)
// ============================================================

std::vector<GuiIntentDecoratorInfo> GuiIntentExecutor::BuildHardcodedData(const std::string& intentName)
{
    std::vector<GuiIntentDecoratorInfo> data;

    // ==================================================
    // Intent: "openSettings" — 打开设置页面
    // ==================================================
    {
        // --- Page 1: 主页 (home) ---
        GuiIntentDecoratorInfo page1;
        page1.intentName = "openSettings";
        page1.domain = "system";
        page1.intentVersion = "1.0";
        page1.displayName = "打开系统设置";
        page1.llmDescription = "打开设备的系统设置页面";
        page1.keywords = "设置, settings, 系统配置";
        page1.pageName = "home";

        {
            GuiIntentDecoratorInfo::GuiIntentComponentEntry comp;
            comp.compId = "btn_settings";
            comp.compType = "Button";
            comp.description = "点击进入设置";
            comp.nextPageName = "settings";
            comp.defaultActionType = "click";
            comp.defaultCompText = "设置";
            page1.components.push_back(comp);
        }
        {
            GuiIntentDecoratorInfo::GuiIntentComponentEntry comp;
            comp.compId = "btn_app_mgr";
            comp.compType = "Button";
            comp.description = "点击进入应用管理";
            comp.nextPageName = "appMgr";
            comp.defaultActionType = "click";
            comp.defaultCompText = "应用管理";
            page1.components.push_back(comp);
        }
        data.push_back(page1);

        // --- Page 2: 设置列表页 (settings) ---
        GuiIntentDecoratorInfo page2;
        page2.intentName = "openSettings";
        page2.domain = "system";
        page2.intentVersion = "1.0";
        page2.displayName = "设置列表";
        page2.llmDescription = "设置菜单列表";
        page2.keywords = "设置列表";
        page2.pageName = "settings";

        {
            GuiIntentDecoratorInfo::GuiIntentComponentEntry comp;
            comp.compId = "list_settings";
            comp.compType = "List";
            comp.description = "设置项列表";
            comp.nextPageName = "";
            comp.defaultActionType = "click";
            comp.defaultCompText = "";

            // List items
            {
                GuiIntentDecoratorInfo::GuiIntentComponentEntry::ListItemEntry li;
                li.compId = "item_wifi";
                li.name = "WLAN";
                li.nextPageName = "wifiSettings";
                comp.listItems.push_back(li);
            }
            {
                GuiIntentDecoratorInfo::GuiIntentComponentEntry::ListItemEntry li;
                li.compId = "item_bluetooth";
                li.name = "蓝牙";
                li.nextPageName = "btSettings";
                comp.listItems.push_back(li);
            }
            {
                GuiIntentDecoratorInfo::GuiIntentComponentEntry::ListItemEntry li;
                li.compId = "item_display";
                li.name = "显示和亮度";
                li.nextPageName = "displaySettings";
                comp.listItems.push_back(li);
            }
            page2.components.push_back(comp);
        }
        {
            GuiIntentDecoratorInfo::GuiIntentComponentEntry comp;
            comp.compId = "nav_back";
            comp.compType = "Button";
            comp.description = "返回按钮";
            comp.nextPageName = "home";
            comp.defaultActionType = "click";
            comp.defaultCompText = "";
            page2.components.push_back(comp);
        }
        data.push_back(page2);

        // --- Page 3: WLAN 设置 ---
        GuiIntentDecoratorInfo page3;
        page3.intentName = "openSettings";
        page3.domain = "system";
        page3.intentVersion = "1.0";
        page3.displayName = "WLAN 设置";
        page3.llmDescription = "WLAN 网络设置页面";
        page3.keywords = "wifi, WLAN, 无线";
        page3.pageName = "wifiSettings";

        {
            GuiIntentDecoratorInfo::GuiIntentComponentEntry comp;
            comp.compId = "toggle_wifi";
            comp.compType = "Toggle";
            comp.description = "WLAN 开关";
            comp.nextPageName = "wifiSettings";
            comp.defaultActionType = "click";
            comp.defaultCompText = "WLAN";
            page3.components.push_back(comp);
        }
        {
            GuiIntentDecoratorInfo::GuiIntentComponentEntry comp;
            comp.compId = "nav_back";
            comp.compType = "Button";
            comp.description = "返回按钮";
            comp.nextPageName = "settings";
            comp.defaultActionType = "click";
            comp.defaultCompText = "";
            page3.components.push_back(comp);
        }
        data.push_back(page3);
    }

    // ==================================================
    // Intent: "submitOrder" — 下单流程
    // ==================================================
    {
        GuiIntentDecoratorInfo page1;
        page1.intentName = "submitOrder";
        page1.domain = "commerce";
        page1.intentVersion = "1.0";
        page1.displayName = "下单";
        page1.llmDescription = "商品下单流程";
        page1.keywords = "下单, 购买, 结算";
        page1.pageName = "productDetail";

        {
            GuiIntentDecoratorInfo::GuiIntentComponentEntry comp;
            comp.compId = "btn_buy";
            comp.compType = "Button";
            comp.description = "立即购买";
            comp.nextPageName = "confirmOrder";
            comp.defaultActionType = "click";
            comp.defaultCompText = "立即购买";
            page1.components.push_back(comp);
        }
        data.push_back(page1);

        GuiIntentDecoratorInfo page2;
        page2.intentName = "submitOrder";
        page2.domain = "commerce";
        page2.intentVersion = "1.0";
        page2.displayName = "确认订单";
        page2.llmDescription = "确认订单信息";
        page2.keywords = "确认订单";
        page2.pageName = "confirmOrder";

        {
            GuiIntentDecoratorInfo::GuiIntentComponentEntry comp;
            comp.compId = "btn_submit";
            comp.compType = "Button";
            comp.description = "提交订单";
            comp.nextPageName = "";
            comp.defaultActionType = "click";
            comp.defaultCompText = "提交订单";
            page2.components.push_back(comp);
        }
        data.push_back(page2);
    }

    // Filter by intentName if specified
    if (!intentName.empty()) {
        std::vector<GuiIntentDecoratorInfo> filtered;
        for (auto& info : data) {
            if (info.intentName == intentName) {
                filtered.push_back(info);
            }
        }
        TAG_LOGI(AAFwkTag::INTENT, "BuildHardcodedData: intent=%{public}s, filtered=%{public}zu",
            intentName.c_str(), filtered.size());
        return filtered;
    }

    TAG_LOGI(AAFwkTag::INTENT, "BuildHardcodedData: total=%{public}zu entries", data.size());
    return data;
}

GuiIntentExecutor::~GuiIntentExecutor()
{
    Cancel();
}

void GuiIntentExecutor::SetTaskHandler(std::shared_ptr<TaskHandlerWrap> handler)
{
    taskHandler_ = handler;
}

// ============================================================
// Public: Execute / Cancel
// ============================================================

void GuiIntentExecutor::Execute(int32_t windowId,
    const std::vector<GuiIntentInputItem>& inputItems,
    GuiIntentCompleteCallback onComplete)
{
    TAG_LOGI(AAFwkTag::INTENT, "GuiIntentExecutor::Execute windowId=%{public}d, items=%{public}zu",
        windowId, inputItems.size());

    if (windowId < 0) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid windowId=%{public}d", windowId);
        if (onComplete) { onComplete(UI_OPERATION_ERR_WINDOW_NOT_FOUND, ""); }
        return;
    }
    if (inputItems.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "empty input items");
        if (onComplete) { onComplete(UI_OPERATION_ERR_COMPONENT_NOT_FOUND, ""); }
        return;
    }

    // Validate: all items must share the same intentName
    const std::string& intentName = inputItems[0].intentName;
    for (const auto& item : inputItems) {
        if (item.intentName != intentName) {
            TAG_LOGE(AAFwkTag::INTENT, "intentName mismatch: expected %{public}s, got %{public}s",
                intentName.c_str(), item.intentName.c_str());
            if (onComplete) { onComplete(-1, ""); }
            return;
        }
    }

    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        if (executing_) {
            TAG_LOGE(AAFwkTag::INTENT, "already executing");
            if (onComplete) { onComplete(UI_OPERATION_ERR_CANCELLED, ""); }
            return;
        }
        windowId_ = windowId;
        inputItems_ = inputItems;
        onComplete_ = std::move(onComplete);
        cancelled_ = false;
        executing_ = true;
        currentStepIndex_ = 0;
    }

    // Load decorator data (hardcoded for now, replace with DB query later)
    decoratorInfos_ = BuildHardcodedData(intentName);

    ConnectWindow([this](bool success) {
        if (!success) {
            TAG_LOGE(AAFwkTag::INTENT, "GuiIntent connect window failed");
            OnFailed(UI_OPERATION_ERR_CONNECT_FAILED, "");
            return;
        }
        // TODO: RegisterContentCallback temporarily disabled — use SendCommand sync return instead
        // RegisterContentCallback();
        // Start first step
        StartNextStep();
    });
}

void GuiIntentExecutor::Cancel()
{
    TAG_LOGI(AAFwkTag::INTENT, "GuiIntentExecutor::Cancel");
    std::lock_guard<ffrt::mutex> lock(mutex_);
    cancelled_ = true;
    if (executing_ && onComplete_) {
        executing_ = false;
        // TODO: UnregisterContentCallback temporarily disabled
        // UnregisterContentCallback();
        auto cb = std::move(onComplete_);
        onComplete_ = nullptr;
        cb(UI_OPERATION_ERR_CANCELLED, "");
    }
}

// ============================================================
// Private: Step orchestration
// ============================================================

void GuiIntentExecutor::StartNextStep()
{
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        if (cancelled_) { return; }
        if (currentStepIndex_ >= inputItems_.size()) {
            OnAllStepsComplete();
            return;
        }
        TAG_LOGI(AAFwkTag::INTENT, "StartNextStep: currentStep=%{public}zu, totalSteps=%{public}zu",
            currentStepIndex_, inputItems_.size());
    }

    // Decouple from IPC callback thread: submit to task handler so the binder
    // thread is freed before the next GetVisibleInspectorTree call.
    // Delay 500ms to wait for the UI page transition to settle.
    auto handler = taskHandler_ ? taskHandler_ : TaskHandlerWrap::GetFfrtHandler();
    if (handler) {
        auto self = shared_from_this();
        handler->SubmitTask([self]() {
            self->ExecuteCurrentStep();
        }, "GuiIntentNextStep", 500);
    } else {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        ExecuteCurrentStep();
    }
}

void GuiIntentExecutor::ExecuteCurrentStep()
{
    size_t stepIndex;
    GuiIntentInputItem item;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        if (cancelled_) { return; }
        if (currentStepIndex_ >= inputItems_.size()) { return; }
        stepIndex = currentStepIndex_;
        item = inputItems_[stepIndex];
    }

    TAG_LOGI(AAFwkTag::INTENT, "GuiIntent step[%{public}zu]: pageName=%{public}s, "
        "compType=%{public}s, compText=%{public}s, action=%{public}s, compId=%{public}s, "
        "nextPage=%{public}s",
        stepIndex, item.pageName.c_str(), item.compType.c_str(), item.compText.c_str(),
        item.actionType.c_str(), item.compId.c_str(), item.nextPageName.c_str());

    // Get current inspector tree
    TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] requesting inspector tree...", stepIndex);
    GetInspectorTree([this, stepIndex, item](const std::string& treeJson, bool success) {
        TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] inspector tree callback: success=%{public}d, size=%{public}zu",
            stepIndex, success, treeJson.size());
        if (!success || treeJson.empty()) {
            TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] failed to get inspector tree", stepIndex);
            OnStepComponentNotFound("tree request failed");
            return;
        }

        // Two-level component matching
        auto matchedNodes = MatchComponent(treeJson, item);
        if (matchedNodes.empty()) {
            TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] component not found: compId=%{public}s, "
                "type=%{public}s, text=%{public}s",
                stepIndex, item.compId.c_str(), item.compType.c_str(), item.compText.c_str());
            OnStepComponentNotFound("tree match");
            return;
        }

        int32_t targetIndex = (item.compIndex >= 0 &&
            static_cast<size_t>(item.compIndex) < matchedNodes.size()) ? item.compIndex : 0;
        const auto& nodeInfo = matchedNodes[targetIndex];

        TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] matched node: id=%{public}d, compId=%{public}s, "
            "clickable=%{public}d (match %{public}d/%{public}zu)",
            stepIndex, nodeInfo.nodeId, nodeInfo.compId.c_str(),
            nodeInfo.clickable, targetIndex, matchedNodes.size());

        // Actionability check
        if (!CheckActionable(nodeInfo, item.actionType)) {
            TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] action not allowed: %{public}s",
                stepIndex, item.actionType.c_str());
            OnStepActionNotAllowed();
            return;
        }

        // TODO: callbackReceived reset temporarily disabled — use SendCommand sync return instead
        // {
        //     std::lock_guard<ffrt::mutex> cbLock(callbackMutex_);
        //     callbackReceived_ = false;
        // }

        // Execute command on matched node
        std::string commandJson = BuildCommandJson(item.actionType);
        TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] SendCommand nodeId=%{public}d, command=%{public}s",
            stepIndex, nodeInfo.nodeId, commandJson.c_str());
        int32_t cmdRet = -1;
        {
            std::lock_guard<ffrt::mutex> lock(mutex_);
            if (cachedService_ != nullptr) {
                cmdRet = cachedService_->SendCommand(nodeInfo.nodeId, commandJson);
            }
        }

        if (cmdRet != 0) {
            TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] SendCommand failed, ret=%{public}d",
                stepIndex, cmdRet);
            OnStepCommandFailed(cmdRet);
            return;
        }

        TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] SendCommand success, proceeding to next step", stepIndex);

        // TODO: WaitForPageChange temporarily disabled — use SendCommand sync return instead
        // if (!WaitForPageChange()) {
        //     TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] PAGE change timeout", stepIndex);
        //     OnStepPageChangeTimeout();
        //     return;
        // }
        // TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] PAGE change confirmed", stepIndex);

        // Verify page transition: check that nextPageName matches the next input element
        if (!item.nextPageName.empty()) {
            size_t nextIdx;
            std::string nextPageName;
            {
                std::lock_guard<ffrt::mutex> lock(mutex_);
                nextIdx = currentStepIndex_ + 1;
                if (nextIdx >= inputItems_.size()) {
                    TAG_LOGW(AAFwkTag::INTENT, "step[%{public}zu] nextPageName=%{public}s but no more input items",
                        stepIndex, item.nextPageName.c_str());
                } else {
                    nextPageName = inputItems_[nextIdx].pageName;
                }
            }
            if (nextIdx < inputItems_.size() && nextPageName != item.nextPageName) {
                TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] page transition mismatch: "
                    "expected=%{public}s, actual next input=%{public}s",
                    stepIndex, item.nextPageName.c_str(), nextPageName.c_str());
                OnFailed(-1, item.nextPageName);  // -1 = nextPageName mismatch
                return;
            }

            // Verify against decorator data
            const auto* decorator = FindDecoratorByPage(item.nextPageName);
            if (decorator == nullptr) {
                TAG_LOGW(AAFwkTag::INTENT, "step[%{public}zu] nextPageName=%{public}s not found in decorator set",
                    stepIndex, item.nextPageName.c_str());
            } else {
                TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] nextPageName=%{public}s matched decorator: "
                    "displayName=%{public}s, components=%{public}zu",
                    stepIndex, item.nextPageName.c_str(),
                    decorator->displayName.c_str(), decorator->components.size());
            }
        }

        // Advance to next step
        {
            std::lock_guard<ffrt::mutex> lock(mutex_);
            if (cancelled_) { return; }
            currentStepIndex_++;
            TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] advancing to step[%{public}zu]",
                stepIndex, currentStepIndex_);
        }
        StartNextStep();
    });
}

// ============================================================
// Private: Component matching (two-level priority)
// ============================================================

std::vector<MatchedNodeInfo> GuiIntentExecutor::MatchComponent(
    const std::string& treeJson, const GuiIntentInputItem& item)
{
    // Level 1: compId + compType + compText
    if (!item.compId.empty()) {
        auto results = UiOperationTreeParser::FindMatchingNodesWithAttributes(
            treeJson, item.compType, item.compId, item.compText);
        if (!results.empty()) {
            TAG_LOGI(AAFwkTag::INTENT, "match L1 (compId): found %{public}zu nodes", results.size());
            return results;
        }
        TAG_LOGI(AAFwkTag::INTENT, "match L1 (compId): no match, falling back to L2");
    }

    // Level 2: compType + compText
    auto results = UiOperationTreeParser::FindMatchingNodesWithAttributes(
        treeJson, item.compType, "", item.compText);
    TAG_LOGI(AAFwkTag::INTENT, "match L2 (type+text): found %{public}zu nodes", results.size());
    return results;
}

// ============================================================
// Private: Decorator lookup
// ============================================================

const GuiIntentDecoratorInfo* GuiIntentExecutor::FindDecoratorByPage(const std::string& pageName) const
{
    for (const auto& info : decoratorInfos_) {
        if (info.pageName == pageName) {
            return &info;
        }
    }
    return nullptr;
}

// ============================================================
// Private: Actionability check
// ============================================================

bool GuiIntentExecutor::CheckActionable(const MatchedNodeInfo& node, const std::string& actionType)
{
    if (actionType == "click") {
        return node.clickable;
    }
    if (actionType == "focus" || actionType == "blur") {
        return true;
    }
    if (actionType == "input" || actionType == "setText") {
        return true;
    }
    TAG_LOGW(AAFwkTag::INTENT, "CheckActionable: unknown action=%{public}s, allowing", actionType.c_str());
    return true;
}

// ============================================================
// Private: Window connection
// ============================================================

void GuiIntentExecutor::ConnectWindow(std::function<void(bool success)> onResult)
{
    sptr<IRemoteObject> remoteObj = nullptr;
    auto wmRet = Rosen::WindowManager::GetInstance().GetUIContentRemoteObj(windowId_, remoteObj);
    if (wmRet != Rosen::WMError::WM_OK || remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "gatherWindow: GetUIContentRemoteObj failed, windowId=%{public}d, ret=%{public}d",
            windowId_, static_cast<int32_t>(wmRet));
        if (onResult) { onResult(false); }
        return;
    }

    auto service = iface_cast<Ace::IUiContentService>(remoteObj);
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "gatherWindow: iface_cast failed for window %{public}d", windowId_);
        if (onResult) { onResult(false); }
        return;
    }

    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        cachedRemoteObj_ = remoteObj;
        cachedService_ = service;
    }

    if (service->IsConnect()) {
        TAG_LOGI(AAFwkTag::INTENT, "gatherWindow: already connected");
        if (onResult) { onResult(true); }
        return;
    }

    auto self = shared_from_this();
    auto connectCb = [self, onResult](std::string baseInfo) {
        TAG_LOGI(AAFwkTag::INTENT, "gatherWindow: connect success, baseInfo=%{public}s",
            baseInfo.substr(0, 100).c_str());
        if (onResult) { onResult(true); }
    };

    int32_t ret = service->Connect(connectCb, nullptr);
    if (ret != Ace::NO_ERROR) {
        TAG_LOGE(AAFwkTag::INTENT, "gatherWindow: Connect failed, ret=%{public}d", ret);
        if (onResult) { onResult(false); }
    }
}

void GuiIntentExecutor::GetInspectorTree(
    std::function<void(const std::string& treeJson, bool success)> onResult)
{
    TAG_LOGI(AAFwkTag::INTENT, "GetInspectorTree: enter");
    sptr<Ace::IUiContentService> service;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        service = cachedService_;
    }
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "GetInspectorTree: service is null");
        if (onResult) { onResult("", false); }
        return;
    }

    auto self = shared_from_this();
    std::string accumulatedTree;
    auto accumMutex = std::make_shared<ffrt::mutex>();

    auto treeCallback = [self, onResult, accumMutex, accumulatedTree](const std::string& data,
        int32_t index, bool isEnd) mutable {
        std::lock_guard<ffrt::mutex> lock(*accumMutex);
        accumulatedTree += data;
        TAG_LOGI(AAFwkTag::INTENT, "treeCallback: index=%{public}d, isEnd=%{public}d, "
            "partSize=%{public}zu, totalSize=%{public}zu",
            index, isEnd, data.size(), accumulatedTree.size());
        if (isEnd) {
            std::string fullTree = std::move(accumulatedTree);
            TAG_LOGI(AAFwkTag::INTENT, "treeCallback: full tree size=%{public}zu", fullTree.size());
            if (onResult) { onResult(fullTree, true); }
        }
    };

    Ace::ParamConfig paramConfig;
    paramConfig.interactionInfo = true;
    int32_t ret = service->GetVisibleInspectorTree(treeCallback, paramConfig,
        TREE_REQUEST_TIMEOUT_MS);
    TAG_LOGI(AAFwkTag::INTENT, "GetInspectorTree ret=%{public}d, interactionInfo=%{public}d",
        ret, paramConfig.interactionInfo);
    if (ret != Ace::NO_ERROR) {
        TAG_LOGE(AAFwkTag::INTENT, "GetInspectorTree failed, ret=%{public}d", ret);
        if (onResult) { onResult("", false); }
    }
}

// ============================================================
// Private: Content change callback
// ============================================================

void GuiIntentExecutor::RegisterContentCallback()
{
    sptr<Ace::IUiContentService> service;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        service = cachedService_;
    }
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "RegisterContentCallback: service is null");
        return;
    }

    Ace::ContentChangeConfig config;
    config.minReportTime = 0;

    auto self = shared_from_this();
    auto callback = [self](Ace::ChangeType type, const std::string& simpleTree) {
        TAG_LOGI(AAFwkTag::INTENT, "GuiIntent ContentChangeCallback: type=%{public}d, treeSize=%{public}zu, "
            "treeHead=%{public}s",
            static_cast<int>(type), simpleTree.size(), simpleTree.substr(0, 300).c_str());
        std::lock_guard<ffrt::mutex> lock(self->callbackMutex_);
        self->callbackReceived_ = true;
        self->receivedChangeType_ = type;
        self->receivedCallbackTree_ = simpleTree;
    };

    service->RegisterContentChangeCallback(config, callback);
    TAG_LOGI(AAFwkTag::INTENT, "RegisterContentCallback done for window %{public}d", windowId_);
}

void GuiIntentExecutor::UnregisterContentCallback()
{
    sptr<Ace::IUiContentService> service;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        service = cachedService_;
    }
    if (service == nullptr) { return; }
    service->UnregisterContentChangeCallback();
    TAG_LOGI(AAFwkTag::INTENT, "UnregisterContentCallback done");
}

bool GuiIntentExecutor::WaitForPageChange()
{
    constexpr int32_t DEFAULT_PAGE_CHANGE_TIMEOUT_MS = 5000;
    int32_t elapsed = 0;

    while (elapsed < DEFAULT_PAGE_CHANGE_TIMEOUT_MS) {
        {
            std::lock_guard<ffrt::mutex> lock(mutex_);
            if (cancelled_) {
                TAG_LOGI(AAFwkTag::INTENT, "WaitForPageChange cancelled");
                return false;
            }
        }

        {
            std::lock_guard<ffrt::mutex> cbLock(callbackMutex_);
            if (callbackReceived_) {
                if (receivedChangeType_ == Ace::ChangeType::PAGE) {
                    callbackReceived_ = false;
                    return true;
                }
                // Non-PAGE change — reset and keep waiting
                TAG_LOGI(AAFwkTag::INTENT, "WaitForPageChange got ChangeType=%{public}d, continuing",
                    static_cast<int>(receivedChangeType_));
                callbackReceived_ = false;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(PAGE_CHANGE_POLL_INTERVAL_MS));
        elapsed += PAGE_CHANGE_POLL_INTERVAL_MS;
    }

    TAG_LOGE(AAFwkTag::INTENT, "WaitForPageChange timeout after %{public}d ms",
        DEFAULT_PAGE_CHANGE_TIMEOUT_MS);
    return false;
}

// ============================================================
// Private: Step result handlers
// ============================================================

void GuiIntentExecutor::OnStepComponentNotFound(const std::string& failReason)
{
    std::string pageName;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        if (currentStepIndex_ < inputItems_.size()) {
            pageName = inputItems_[currentStepIndex_].pageName;
        }
    }
    TAG_LOGE(AAFwkTag::INTENT, "OnStepComponentNotFound: page=%{public}s, reason=%{public}s",
        pageName.c_str(), failReason.c_str());
    OnFailed(UI_OPERATION_ERR_COMPONENT_NOT_FOUND, pageName);
}

void GuiIntentExecutor::OnStepActionNotAllowed()
{
    std::string pageName;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        if (currentStepIndex_ < inputItems_.size()) {
            pageName = inputItems_[currentStepIndex_].pageName;
        }
    }
    OnFailed(UI_OPERATION_ERR_ACTION_NOT_ALLOWED, pageName);
}

void GuiIntentExecutor::OnStepCommandFailed(int32_t cmdRet)
{
    std::string pageName;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        if (currentStepIndex_ < inputItems_.size()) {
            pageName = inputItems_[currentStepIndex_].pageName;
        }
    }
    TAG_LOGE(AAFwkTag::INTENT, "OnStepCommandFailed: page=%{public}s, ret=%{public}d",
        pageName.c_str(), cmdRet);
    OnFailed(UI_OPERATION_ERR_COMMAND_FAILED, pageName);
}

void GuiIntentExecutor::OnStepPageChangeTimeout()
{
    std::string pageName;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        if (currentStepIndex_ < inputItems_.size()) {
            pageName = inputItems_[currentStepIndex_].pageName;
        }
    }
    OnFailed(UI_OPERATION_ERR_PAGE_CHANGE_TIMEOUT, pageName);
}

void GuiIntentExecutor::OnAllStepsComplete()
{
    TAG_LOGI(AAFwkTag::INTENT, "all steps completed successfully");
    // TODO: UnregisterContentCallback temporarily disabled
    // UnregisterContentCallback();

    GuiIntentCompleteCallback cb;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        executing_ = false;
        cb = std::move(onComplete_);
        onComplete_ = nullptr;
    }

    if (cb) {
        cb(UI_OPERATION_ERR_OK, "");
    }
}

void GuiIntentExecutor::OnFailed(int32_t resultCode, const std::string& failedPageName)
{
    // TODO: UnregisterContentCallback temporarily disabled
    // UnregisterContentCallback();

    GuiIntentCompleteCallback cb;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        executing_ = false;
        cb = std::move(onComplete_);
        onComplete_ = nullptr;
    }

    if (cb) {
        cb(resultCode, failedPageName);
    }
}

} // namespace AAFwk
} // namespace OHOS
