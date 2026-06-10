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
#include "ui_content_proxy.h"
#include "ui_content_service_interface.h"
#include "window_manager.h"

namespace OHOS {
namespace AAFwk {

namespace {
constexpr int32_t TREE_REQUEST_TIMEOUT_MS = 3000;
constexpr int32_t PAGE_CHANGE_POLL_INTERVAL_MS = 50;
} // anonymous namespace

GuiIntentExecutor::~GuiIntentExecutor()
{
    Cancel();
}

// ============================================================
// Public: Execute / Cancel
// ============================================================

void GuiIntentExecutor::Execute(int32_t windowId,
    const std::vector<GuiIntentInputItem>& inputItems,
    std::shared_ptr<AppExecFwk::EventHandler> handler,
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
        eventHandler_ = handler;
        onComplete_ = std::move(onComplete);
        cancelled_ = false;
        executing_ = true;
        currentStepIndex_ = 0;
    }

    // TODO: Load hardcoded decorator data from DB later
    // For now, use empty placeholders — matching is driven by input parameters directly
    hardcodedDecorators_.clear();

    ConnectWindow([this](bool success) {
        if (!success) {
            TAG_LOGE(AAFwkTag::INTENT, "GuiIntent connect window failed");
            OnFailed(UI_OPERATION_ERR_CONNECT_FAILED, "");
            return;
        }
        // Register content change callback once for the whole chain
        RegisterContentCallback();
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
        UnregisterContentCallback();
        auto cb = std::move(onComplete_);
        onComplete_ = nullptr;
        if (eventHandler_) {
            eventHandler_->PostTask([cb]() { cb(UI_OPERATION_ERR_CANCELLED, ""); }, "GuiIntentCancel");
        } else {
            cb(UI_OPERATION_ERR_CANCELLED, "");
        }
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
    }

    if (eventHandler_) {
        auto self = shared_from_this();
        eventHandler_->PostTask([self]() { self->ExecuteCurrentStep(); }, "GuiIntentStep");
    } else {
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
    GetInspectorTree([this, stepIndex, item](const std::string& treeJson, bool success) {
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

        // Reset callback flag before sending command
        {
            std::lock_guard<ffrt::mutex> cbLock(callbackMutex_);
            callbackReceived_ = false;
        }

        // Execute command on matched node
        int32_t cmdRet = -1;
        {
            std::lock_guard<ffrt::mutex> lock(mutex_);
            if (cachedService_ != nullptr) {
                cmdRet = cachedService_->SendCommand(nodeInfo.nodeId, item.actionType);
            }
        }

        if (cmdRet != 0) {
            TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] SendCommand failed, ret=%{public}d",
                stepIndex, cmdRet);
            OnStepCommandFailed(cmdRet);
            return;
        }

        TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] command sent, waiting for PAGE change...", stepIndex);

        // Wait for PAGE change callback
        if (!WaitForPageChange()) {
            TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] PAGE change timeout", stepIndex);
            OnStepPageChangeTimeout();
            return;
        }

        TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] PAGE change confirmed", stepIndex);

        // Advance to next step
        {
            std::lock_guard<ffrt::mutex> lock(mutex_);
            if (cancelled_) { return; }
            currentStepIndex_++;
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

    int32_t ret = service->Connect(connectCb, eventHandler_);
    if (ret != Ace::NO_ERROR) {
        TAG_LOGE(AAFwkTag::INTENT, "gatherWindow: Connect failed, ret=%{public}d", ret);
        if (onResult) { onResult(false); }
    }
}

void GuiIntentExecutor::GetInspectorTree(
    std::function<void(const std::string& treeJson, bool success)> onResult)
{
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
        if (isEnd) {
            std::string fullTree = std::move(accumulatedTree);
            if (onResult) { onResult(fullTree, true); }
        }
    };

    int32_t ret = service->GetVisibleInspectorTree(treeCallback, Ace::ParamConfig(),
        TREE_REQUEST_TIMEOUT_MS);
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
        TAG_LOGI(AAFwkTag::INTENT, "GuiIntent ContentChangeCallback: type=%{public}d, treeSize=%{public}zu",
            static_cast<int>(type), simpleTree.size());
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
    UnregisterContentCallback();

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
    UnregisterContentCallback();

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
