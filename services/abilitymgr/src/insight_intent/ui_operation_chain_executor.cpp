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

#include "insight_intent/ui_operation_chain_executor.h"

#include <thread>

#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"
#include "ui_content_proxy.h"
#include "ui_content_service_interface.h"
#include "window_manager.h"

namespace OHOS {
namespace AAFwk {

namespace {
constexpr int32_t MAX_CHAIN_STEPS = 100;
constexpr int32_t TREE_TIMEOUT_MS = 3000;
} // anonymous namespace

// ============================================================
// Private: Inspector tree unwrapping
// ============================================================

std::string UiOperationChainExecutor::UnwrapInspectorTree(const std::string& wrappedJson)
{
    if (wrappedJson.empty()) {
        return wrappedJson;
    }
    try {
        auto outer = nlohmann::json::parse(wrappedJson);
        if (!outer.is_object() || !outer.contains("0")) {
            // Not wrapped — return as-is (already raw Inspector tree JSON).
            return wrappedJson;
        }
        // Case 1: {"0": "<string>"} — string-wrapped format
        if (outer["0"].is_string()) {
            return outer["0"].get<std::string>();
        }
        // Case 2: {"0": {<object>}} — nested object format
        if (outer["0"].is_object()) {
            return outer["0"].dump();
        }
        // Unknown format — return as-is
        return wrappedJson;
    } catch (const nlohmann::json::exception& e) {
        TAG_LOGW(AAFwkTag::INTENT, "UnwrapInspectorTree parse failed, use raw: %{public}s", e.what());
        return wrappedJson;
    }
}

// ============================================================
// Private: Service management
// ============================================================

sptr<Ace::IUiContentService> UiOperationChainExecutor::GetUiContentService(int32_t windowId)
{
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        auto it = serviceCache_.find(windowId);
        if (it != serviceCache_.end() && it->second.service != nullptr) {
            return it->second.service;
        }
    }

    sptr<IRemoteObject> remoteObj = nullptr;
    auto wmRet = Rosen::WindowManager::GetInstance().GetUIContentRemoteObj(windowId, remoteObj);
    if (wmRet != Rosen::WMError::WM_OK || remoteObj == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "GetUIContentRemoteObj failed, windowId=%{public}d, ret=%{public}d",
            windowId, static_cast<int32_t>(wmRet));
        return nullptr;
    }

    auto service = iface_cast<Ace::IUiContentService>(remoteObj);
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "iface_cast failed for window %{public}d", windowId);
        return nullptr;
    }

    auto self = shared_from_this();
    auto recipient = sptr<Ace::UiContentProxyRecipient>::MakeSptr(
        [self, windowId]() {
            TAG_LOGW(AAFwkTag::INTENT, "remote died for window %{public}d", windowId);
            std::lock_guard<ffrt::mutex> lock(self->mutex_);
            self->serviceCache_.erase(windowId);
        });
    remoteObj->AddDeathRecipient(recipient);

    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        ServiceCache cache{remoteObj, service};
        serviceCache_[windowId] = cache;
    }

    TAG_LOGI(AAFwkTag::INTENT, "cached service for window %{public}d", windowId);
    return service;
}

// ============================================================
// Private: Connect / Inspector tree
// ============================================================

void UiOperationChainExecutor::DoConnect(sptr<Ace::IUiContentService> service,
    std::function<void(bool success)> onConnected)
{
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "service is null");
        if (onConnected) { onConnected(false); }
        return;
    }
    if (service->IsConnect()) {
        TAG_LOGI(AAFwkTag::INTENT, "already connected");
        if (onConnected) { onConnected(true); }
        return;
    }

    auto callback = [onConnected](std::string baseInfo) {
        TAG_LOGI(AAFwkTag::INTENT, "connect success, baseInfo=%{public}s",
            baseInfo.substr(0, 100).c_str());
        if (onConnected) { onConnected(true); }
    };

    int32_t ret = service->Connect(callback, eventHandler_);
    if (ret != Ace::NO_ERROR) {
        TAG_LOGE(AAFwkTag::INTENT, "Connect failed, ret=%{public}d", ret);
        if (onConnected) { onConnected(false); }
    }
}

void UiOperationChainExecutor::DoRequestInspectorTree(sptr<Ace::IUiContentService> service,
    const Ace::ParamConfig& config,
    std::function<void(std::string, bool)> onTreeReady)
{
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "service is null");
        if (onTreeReady) { onTreeReady("", false); }
        return;
    }

    auto self = shared_from_this();
    auto treeCallback = [self, onTreeReady](const std::string& data, int32_t index, bool isEnd) {
        std::lock_guard<ffrt::mutex> lock(self->mutex_);
        self->accumulatedTree_ += data;
        TAG_LOGI(AAFwkTag::INTENT, "tree part %{public}d, isEnd=%{public}d, partSize=%{public}zu",
            index, isEnd, data.size());
        if (isEnd) {
            std::string fullTree = std::move(self->accumulatedTree_);
            self->accumulatedTree_.clear();
            TAG_LOGI(AAFwkTag::INTENT, "raw callback head: %{public}s",
                fullTree.substr(0, 500).c_str());
            // ArkUI wraps Inspector tree in {"0": "<escaped-json>"}.
            // Unwrap here so the parser receives the raw Inspector tree JSON.
            std::string unwrappedTree = UnwrapInspectorTree(fullTree);
            TAG_LOGI(AAFwkTag::INTENT, "unwrapped head: %{public}s",
                unwrappedTree.substr(0, 500).c_str());
            if (onTreeReady) { onTreeReady(unwrappedTree, true); }
        }
    };

    int32_t ret = service->GetVisibleInspectorTree(treeCallback, config, TREE_TIMEOUT_MS);
    TAG_LOGI(AAFwkTag::INTENT, "GetVisibleInspectorTree ret=%{public}d, config.interactionInfo=%{public}d",
        ret, config.interactionInfo);
    if (ret != Ace::NO_ERROR) {
        TAG_LOGE(AAFwkTag::INTENT, "GetVisibleInspectorTree failed, ret=%{public}d", ret);
        if (onTreeReady) { onTreeReady("", false); }
    }
}

// ============================================================
// Standalone operations (mirroring ui_sa_sample)
// ============================================================

void UiOperationChainExecutor::ConnectToWindow(int32_t windowId,
    std::shared_ptr<AppExecFwk::EventHandler> handler,
    std::function<void(bool success, const std::string& baseInfo)> onResult)
{
    TAG_LOGI(AAFwkTag::INTENT, "ConnectToWindow windowId=%{public}d", windowId);
    if (windowId < 0) {
        if (onResult) { onResult(false, ""); }
        return;
    }

    eventHandler_ = handler;
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        if (onResult) { onResult(false, ""); }
        return;
    }

    auto self = shared_from_this();
    auto connectCb = [self, windowId, onResult](std::string baseInfo) {
        TAG_LOGI(AAFwkTag::INTENT, "connect callback window=%{public}d, info=%{public}s",
            windowId, baseInfo.substr(0, 100).c_str());
        if (onResult) { onResult(true, baseInfo); }
    };

    if (service->IsConnect()) {
        if (onResult) { onResult(true, "already connected"); }
        return;
    }

    int32_t ret = service->Connect(connectCb, handler);
    if (ret != Ace::NO_ERROR) {
        TAG_LOGE(AAFwkTag::INTENT, "Connect failed, ret=%{public}d", ret);
        if (onResult) { onResult(false, ""); }
    }
}

void UiOperationChainExecutor::GetVisibleInspectorTree(int32_t windowId,
    const Ace::ParamConfig& config,
    std::function<void(bool success, const std::string& treeJson)> onResult)
{
    TAG_LOGI(AAFwkTag::INTENT, "GetVisibleInspectorTree windowId=%{public}d", windowId);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        if (onResult) { onResult(false, ""); }
        return;
    }
    if (!service->IsConnect()) {
        TAG_LOGE(AAFwkTag::INTENT, "not connected, connect first");
        if (onResult) { onResult(false, ""); }
        return;
    }

    auto self = shared_from_this();
    DoRequestInspectorTree(service, config, [onResult](std::string tree, bool success) {
        if (onResult) { onResult(success, tree); }
    });
}

void UiOperationChainExecutor::GetCurrentPageName(int32_t windowId,
    std::function<void(bool success, const std::string& pageName)> onResult)
{
    TAG_LOGI(AAFwkTag::INTENT, "GetCurrentPageName windowId=%{public}d", windowId);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        if (onResult) { onResult(false, ""); }
        return;
    }

    auto cb = [onResult](std::string pageName) {
        TAG_LOGI(AAFwkTag::INTENT, "pageName=%{public}s", pageName.c_str());
        if (onResult) { onResult(true, pageName); }
    };
    service->GetCurrentPageName(cb);
}

int32_t UiOperationChainExecutor::SendCommand(int32_t windowId, int32_t nodeId,
    const std::string& command)
{
    TAG_LOGI(AAFwkTag::INTENT, "SendCommand window=%{public}d, node=%{public}d, cmd=%{public}s",
        windowId, nodeId, command.c_str());
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return Ace::FAILED;
    }
    return service->SendCommand(nodeId, command);
}

int32_t UiOperationChainExecutor::SendCommand(int32_t windowId, const std::string& command)
{
    TAG_LOGI(AAFwkTag::INTENT, "SendCommand(global) window=%{public}d, cmd=%{public}s",
        windowId, command.c_str());
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return Ace::FAILED;
    }
    return service->SendCommand(command);
}

int32_t UiOperationChainExecutor::SendCommandAsync(int32_t windowId, int32_t nodeId,
    const std::string& command)
{
    TAG_LOGI(AAFwkTag::INTENT, "SendCommandAsync window=%{public}d, node=%{public}d, cmd=%{public}s",
        windowId, nodeId, command.c_str());
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return Ace::FAILED;
    }
    return service->SendCommandAsync(nodeId, command);
}

void UiOperationChainExecutor::RegisterContentChangeCallback(int32_t windowId,
    const Ace::ContentChangeConfig& config,
    std::function<void(Ace::ChangeType, const std::string&)> callback)
{
    TAG_LOGI(AAFwkTag::INTENT, "RegisterContentChangeCallback window=%{public}d", windowId);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return;
    }
    service->RegisterContentChangeCallback(config, callback);
    TAG_LOGI(AAFwkTag::INTENT, "RegisterContentChangeCallback done");
}

int32_t UiOperationChainExecutor::UnregisterContentChangeCallback(int32_t windowId)
{
    TAG_LOGI(AAFwkTag::INTENT, "UnregisterContentChangeCallback window=%{public}d", windowId);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return Ace::FAILED;
    }
    return service->UnregisterContentChangeCallback();
}

void UiOperationChainExecutor::GetCurrentImagesShowing(int32_t windowId,
    std::function<void(std::vector<std::pair<int32_t, std::shared_ptr<Media::PixelMap>>>)> onResult)
{
    TAG_LOGI(AAFwkTag::INTENT, "GetCurrentImagesShowing window=%{public}d", windowId);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        if (onResult) { onResult({}); }
        return;
    }
    service->GetCurrentImagesShowing(onResult);
}

void UiOperationChainExecutor::GetImagesById(int32_t windowId,
    const std::vector<int32_t>& arkUIIds,
    std::function<void(int32_t,
        const std::unordered_map<int32_t, std::shared_ptr<Media::PixelMap>>&,
        Ace::MultiImageQueryErrorCode)> arkUICallback,
    const std::map<int32_t, std::vector<int32_t>>& arkWebs,
    std::function<void(int32_t,
        const std::map<int32_t, std::map<int32_t, std::shared_ptr<Media::PixelMap>>>&,
        Ace::MultiImageQueryErrorCode)> arkWebCallback)
{
    TAG_LOGI(AAFwkTag::INTENT, "GetImagesById window=%{public}d, arkUI=%{public}zu, arkWeb=%{public}zu",
        windowId, arkUIIds.size(), arkWebs.size());
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return;
    }
    service->GetImagesById(arkUIIds, arkUICallback, arkWebs, arkWebCallback);
}

void UiOperationChainExecutor::GetWebInfoByRequest(int32_t windowId, int32_t webId,
    const std::string& request, Ace::GetWebInfoByRequestCallback callback)
{
    TAG_LOGI(AAFwkTag::INTENT, "GetWebInfoByRequest window=%{public}d, webId=%{public}d, req=%{public}s",
        windowId, webId, request.c_str());
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return;
    }
    service->GetWebInfoByRequest(webId, request, callback);
}

void UiOperationChainExecutor::RegisterComponentChangeEventCallback(int32_t windowId,
    uint32_t mask, std::function<void(const std::string&)> callback)
{
    TAG_LOGI(AAFwkTag::INTENT, "RegisterComponentChangeEventCallback window=%{public}d, mask=%{public}u",
        windowId, mask);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return;
    }
    service->RegisterComponentChangeEventCallback(callback, mask);
}

int32_t UiOperationChainExecutor::UnregisterComponentChangeEventCallback(int32_t windowId)
{
    TAG_LOGI(AAFwkTag::INTENT, "UnregisterComponentChangeEventCallback window=%{public}d", windowId);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return Ace::FAILED;
    }
    return service->UnregisterComponentChangeEventCallback();
}

void UiOperationChainExecutor::ExeAppAIFunction(int32_t windowId,
    const std::string& funcName, const std::string& params,
    std::function<void(uint32_t)> onResult)
{
    TAG_LOGI(AAFwkTag::INTENT, "ExeAppAIFunction window=%{public}d, func=%{public}s",
        windowId, funcName.c_str());
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return;
    }
    service->ExeAppAIFunction(funcName, params, onResult);
}

void UiOperationChainExecutor::GetWebViewCurrentLanguage(int32_t windowId,
    std::function<void(const std::string&)> onResult)
{
    TAG_LOGI(AAFwkTag::INTENT, "GetWebViewCurrentLanguage window=%{public}d", windowId);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return;
    }
    service->GetWebViewCurrentLanguage([onResult](std::string lang) {
        TAG_LOGI(AAFwkTag::INTENT, "currentLanguage=%{public}s", lang.c_str());
        if (onResult) { onResult(lang); }
    });
}

void UiOperationChainExecutor::StartWebViewTranslate(int32_t windowId,
    const std::string& data,
    std::function<void(int32_t code, const std::string& result)> onResult)
{
    TAG_LOGI(AAFwkTag::INTENT, "StartWebViewTranslate window=%{public}d", windowId);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return;
    }
    service->StartWebViewTranslate(data, [onResult](int32_t code, std::string result) {
        TAG_LOGI(AAFwkTag::INTENT, "translate result code=%{public}d", code);
        if (onResult) { onResult(code, result); }
    });
}

void UiOperationChainExecutor::GetStateMgmtInfo(int32_t windowId,
    const std::string& componentName, const std::string& propertyName,
    const std::string& jsonPath,
    std::function<void(std::vector<std::string>)> onResult, bool onlyVisible)
{
    TAG_LOGI(AAFwkTag::INTENT, "GetStateMgmtInfo window=%{public}d, comp=%{public}s, prop=%{public}s",
        windowId, componentName.c_str(), propertyName.c_str());
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return;
    }
    service->GetStateMgmtInfo(componentName, propertyName, jsonPath, onResult, onlyVisible);
}

void UiOperationChainExecutor::RegisterTextChangeEventCallback(int32_t windowId,
    std::function<void(const std::string&)> callback)
{
    TAG_LOGI(AAFwkTag::INTENT, "RegisterTextChangeEventCallback window=%{public}d", windowId);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return;
    }
    service->RegisterTextChangeEventCallback(callback);
}

int32_t UiOperationChainExecutor::UnregisterTextChangeEventCallback(int32_t windowId)
{
    TAG_LOGI(AAFwkTag::INTENT, "UnregisterTextChangeEventCallback window=%{public}d", windowId);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return Ace::FAILED;
    }
    return service->UnregisterTextChangeEventCallback();
}

void UiOperationChainExecutor::RegisterSelectTextEventCallback(int32_t windowId,
    std::function<void(const std::string&)> callback)
{
    TAG_LOGI(AAFwkTag::INTENT, "RegisterSelectTextEventCallback window=%{public}d", windowId);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return;
    }
    service->RegisterSelectTextEventCallback(callback);
}

int32_t UiOperationChainExecutor::UnregisterSelectTextEventCallback(int32_t windowId)
{
    TAG_LOGI(AAFwkTag::INTENT, "UnregisterSelectTextEventCallback window=%{public}d", windowId);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return Ace::FAILED;
    }
    return service->UnregisterSelectTextEventCallback();
}

void UiOperationChainExecutor::GetSpecifiedContentOffsets(int32_t windowId,
    int32_t id, const std::string& content,
    std::function<void(std::vector<std::pair<float, float>>)> onResult)
{
    TAG_LOGI(AAFwkTag::INTENT, "GetSpecifiedContentOffsets window=%{public}d, id=%{public}d",
        windowId, id);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return;
    }
    service->GetSpecifiedContentOffsets(id, content, onResult);
}

int32_t UiOperationChainExecutor::HighlightSpecifiedContent(int32_t windowId,
    int32_t id, const std::string& content,
    const std::vector<std::string>& nodeIds, const std::string& configs)
{
    TAG_LOGI(AAFwkTag::INTENT, "HighlightSpecifiedContent window=%{public}d, id=%{public}d",
        windowId, id);
    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        return Ace::FAILED;
    }
    return service->HighlightSpecifiedContent(id, content, nodeIds, configs);
}

// ============================================================
// Chain execution
// ============================================================

void UiOperationChainExecutor::ExecuteChain(const UiOperationChain& chain,
    std::shared_ptr<AppExecFwk::EventHandler> handler, ChainCompleteCallback onComplete)
{
    TAG_LOGI(AAFwkTag::INTENT, "ExecuteChain windowId=%{public}d, steps=%{public}zu, ",
        chain.windowId, chain.steps.size());

    if (!chain.IsValid()) {
        TAG_LOGE(AAFwkTag::INTENT, "invalid chain params");
        if (onComplete) { onComplete(UI_OPERATION_ERR_WINDOW_NOT_FOUND, -1); }
        return;
    }
    if (chain.steps.size() > MAX_CHAIN_STEPS) {
        TAG_LOGE(AAFwkTag::INTENT, "too many steps: %{public}zu", chain.steps.size());
        if (onComplete) { onComplete(UI_OPERATION_ERR_WINDOW_NOT_FOUND, -1); }
        return;
    }

    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        if (executing_) {
            TAG_LOGE(AAFwkTag::INTENT, "already executing a chain");
            if (onComplete) { onComplete(UI_OPERATION_ERR_CANCELLED, -1); }
            return;
        }
        chain_ = chain;
        onComplete_ = std::move(onComplete);
        eventHandler_ = handler;
        cancelled_ = false;
        executing_ = true;
        accumulatedTree_.clear();
    }

    {
        std::lock_guard<ffrt::mutex> lock(callbackMutex_);
        contentCallbackReceived_ = false;
        receivedCallbackTree_.clear();
    }

    auto service = GetUiContentService(chain.windowId);
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "cannot get service for window %{public}d", chain.windowId);
        OnChainComplete(UI_OPERATION_ERR_WINDOW_NOT_FOUND, -1);
        return;
    }

    auto self = shared_from_this();
    DoConnect(service, [self](bool success) {
        if (!success) {
            self->OnChainComplete(UI_OPERATION_ERR_CONNECT_FAILED, -1);
            return;
        }
        // Register content change callback once for the whole chain
        if (self->chain_.waitForPageChange) {
            self->RegisterChainContentCallback();
        }
        self->ProceedToStep(0);
    });
}

void UiOperationChainExecutor::Cancel()
{
    TAG_LOGI(AAFwkTag::INTENT, "Cancel");
    std::lock_guard<ffrt::mutex> lock(mutex_);
    cancelled_ = true;
    if (executing_ && onComplete_) {
        executing_ = false;
        auto callback = std::move(onComplete_);
        onComplete_ = nullptr;
        UnregisterChainContentCallback();
        if (eventHandler_) {
            eventHandler_->PostTask([callback]() {
                callback(UI_OPERATION_ERR_CANCELLED, -1);
            }, "UiOpChainCancel");
        } else {
            callback(UI_OPERATION_ERR_CANCELLED, -1);
        }
    }
}

// ============================================================
// Private: Step execution
// ============================================================

void UiOperationChainExecutor::ExecuteStep(size_t stepIndex)
{
    UiOperationStep step;
    bool waitForPageChange;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        if (cancelled_) {
            TAG_LOGI(AAFwkTag::INTENT, "chain cancelled, skip step %{public}zu", stepIndex);
            return;
        }
        if (stepIndex >= chain_.steps.size()) {
            TAG_LOGI(AAFwkTag::INTENT, "all steps completed");
            OnChainComplete(UI_OPERATION_ERR_OK, -1);
            return;
        }
        step = chain_.steps[stepIndex];
        waitForPageChange = chain_.waitForPageChange;
    }

    if (waitForPageChange) {
        ExecuteStepWithPageWait(stepIndex);
        return;
    }

    // --- Legacy path: no page-change callback-wait ---
    TAG_LOGI(AAFwkTag::INTENT, "ExecuteStep[%{public}zu]: type=%{public}s, matcher=%{public}s, cmd=%{public}s",
        stepIndex, step.compType.c_str(), step.compText.c_str(), step.command.c_str());

    auto service = GetUiContentService(chain_.windowId);
    if (service == nullptr) {
        OnChainComplete(UI_OPERATION_ERR_WINDOW_NOT_FOUND, static_cast<int32_t>(stepIndex));
        return;
    }

    auto self = shared_from_this();
    auto treeReady = [self, step, stepIndex](std::string treeJson, bool success) {
        if (!success || treeJson.empty()) {
            TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] tree request failed", stepIndex);
            self->OnChainComplete(UI_OPERATION_ERR_TREE_TIMEOUT, static_cast<int32_t>(stepIndex));
            return;
        }

        auto matchingIds = UiOperationTreeParser::FindMatchingNodes(
            treeJson, step.compType, step.compText);

        if (matchingIds.empty()) {
            TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] component not found: type=%{public}s, matcher=%{public}s",
                stepIndex, step.compType.c_str(), step.compText.c_str());
            self->OnChainComplete(UI_OPERATION_ERR_COMPONENT_NOT_FOUND, static_cast<int32_t>(stepIndex));
            return;
        }

        int32_t targetIndex = (step.compIndex >= 0 &&
            static_cast<size_t>(step.compIndex) < matchingIds.size()) ? step.compIndex : 0;
        int32_t nodeId = matchingIds[targetIndex];
        TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] found node id=%{public}d (match %{public}d/%{public}zu)",
            stepIndex, nodeId, targetIndex, matchingIds.size());

        int32_t cmdRet = Ace::FAILED;
        auto svc = self->GetUiContentService(self->chain_.windowId);
        if (svc == nullptr) {
            self->OnChainComplete(UI_OPERATION_ERR_WINDOW_NOT_FOUND, static_cast<int32_t>(stepIndex));
            return;
        }
        if (step.useAsync) {
            cmdRet = svc->SendCommandAsync(nodeId, step.command);
        } else {
            cmdRet = svc->SendCommand(nodeId, step.command);
        }

        if (cmdRet != Ace::NO_ERROR) {
            TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] SendCommand failed, ret=%{public}d", stepIndex, cmdRet);
            self->OnChainComplete(UI_OPERATION_ERR_COMMAND_FAILED, static_cast<int32_t>(stepIndex));
            return;
        }

        TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] success", stepIndex);
        self->ProceedToStep(stepIndex + 1);
    };

    Ace::ParamConfig paramConfig;
    paramConfig.interactionInfo = true;
    DoRequestInspectorTree(service, paramConfig, treeReady);
}

void UiOperationChainExecutor::ProceedToStep(size_t stepIndex)
{
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        if (cancelled_) { return; }
        if (stepIndex >= chain_.steps.size()) {
            UnregisterChainContentCallback();
            OnChainComplete(UI_OPERATION_ERR_OK, -1);
            return;
        }
    }

    if (eventHandler_) {
        auto self = shared_from_this();
        eventHandler_->PostTask([self, stepIndex]() {
            self->ExecuteStep(stepIndex);
        }, "UiOpChainStep");
    } else {
        ExecuteStep(stepIndex);
    }
}

void UiOperationChainExecutor::OnChainComplete(int32_t resultCode, int32_t failedStepIndex)
{
    ChainCompleteCallback callback;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        executing_ = false;
        callback = std::move(onComplete_);
        onComplete_ = nullptr;
    }

    UnregisterChainContentCallback();

    TAG_LOGI(AAFwkTag::INTENT, "chain complete: code=%{public}d, failedStep=%{public}d",
        resultCode, failedStepIndex);

    if (callback) {
        callback(resultCode, failedStepIndex);
    }
}

// ============================================================
// Private: Extended step execution (two-level matching + actionability + callback-wait)
// ============================================================

void UiOperationChainExecutor::ExecuteStepWithPageWait(size_t stepIndex)
{
    UiOperationStep step;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        if (cancelled_) { return; }
        if (stepIndex >= chain_.steps.size()) {
            OnChainComplete(UI_OPERATION_ERR_OK, -1);
            return;
        }
        step = chain_.steps[stepIndex];
    }

    TAG_LOGI(AAFwkTag::INTENT, "ExecuteStepWithPageWait[%{public}zu]: compId=%{public}s, "
        "compType=%{public}s, compText=%{public}s, cmd=%{public}s",
        stepIndex, step.compId.c_str(), step.compType.c_str(), step.compText.c_str(), step.command.c_str());

    auto service = GetUiContentService(chain_.windowId);
    if (service == nullptr) {
        OnChainComplete(UI_OPERATION_ERR_WINDOW_NOT_FOUND, static_cast<int32_t>(stepIndex));
        return;
    }

    auto self = shared_from_this();
    auto treeReady = [self, step, stepIndex](std::string treeJson, bool success) {
        if (!success || treeJson.empty()) {
            TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] tree request failed", stepIndex);
            self->OnChainComplete(UI_OPERATION_ERR_TREE_TIMEOUT, static_cast<int32_t>(stepIndex));
            return;
        }

        // Two-level matching
        auto matchingNodes = self->ResolveMatchingNodes(treeJson, step);
        if (matchingNodes.empty()) {
            TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] component not found: compId=%{public}s, "
                "type=%{public}s, text=%{public}s",
                stepIndex, step.compId.c_str(), step.compType.c_str(), step.compText.c_str());
            self->OnChainComplete(UI_OPERATION_ERR_COMPONENT_NOT_FOUND, static_cast<int32_t>(stepIndex));
            return;
        }

        int32_t targetIndex = (step.compIndex >= 0 &&
            static_cast<size_t>(step.compIndex) < matchingNodes.size()) ? step.compIndex : 0;
        const auto& nodeInfo = matchingNodes[targetIndex];
        TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] found node: id=%{public}d, compId=%{public}s, "
            "clickable=%{public}d (match %{public}d/%{public}zu)",
            stepIndex, nodeInfo.nodeId, nodeInfo.compId.c_str(),
            nodeInfo.clickable, targetIndex, matchingNodes.size());

        // Actionability check
        if (!self->CheckActionability(nodeInfo, step.command)) {
            TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] action not allowed: cmd=%{public}s, "
                "clickable=%{public}d",
                stepIndex, step.command.c_str(), nodeInfo.clickable);
            self->OnChainComplete(UI_OPERATION_ERR_ACTION_NOT_ALLOWED, static_cast<int32_t>(stepIndex));
            return;
        }

        // Reset callback flag before sending command
        {
            std::lock_guard<ffrt::mutex> cbLock(self->callbackMutex_);
            self->contentCallbackReceived_ = false;
        }

        // Execute command
        auto svc = self->GetUiContentService(self->chain_.windowId);
        if (svc == nullptr) {
            self->OnChainComplete(UI_OPERATION_ERR_WINDOW_NOT_FOUND, static_cast<int32_t>(stepIndex));
            return;
        }
        int32_t cmdRet = step.useAsync ?
            svc->SendCommandAsync(nodeInfo.nodeId, step.command) :
            svc->SendCommand(nodeInfo.nodeId, step.command);

        if (cmdRet != Ace::NO_ERROR) {
            TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] SendCommand failed, ret=%{public}d", stepIndex, cmdRet);
            self->OnChainComplete(UI_OPERATION_ERR_COMMAND_FAILED, static_cast<int32_t>(stepIndex));
            return;
        }

        TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] command sent, waiting for PAGE change callback...", stepIndex);

        // Wait for PAGE change callback
        if (!self->WaitForPageChange(stepIndex)) {
            TAG_LOGE(AAFwkTag::INTENT, "step[%{public}zu] PAGE change timeout or wrong type", stepIndex);
            self->OnChainComplete(UI_OPERATION_ERR_PAGE_CHANGE_TIMEOUT, static_cast<int32_t>(stepIndex));
            return;
        }

        TAG_LOGI(AAFwkTag::INTENT, "step[%{public}zu] PAGE change confirmed", stepIndex);
        self->ProceedToStep(stepIndex + 1);
    };

    Ace::ParamConfig paramConfig;
    paramConfig.interactionInfo = true;
    DoRequestInspectorTree(service, paramConfig, treeReady);
}

std::vector<MatchedNodeInfo> UiOperationChainExecutor::ResolveMatchingNodes(
    const std::string& treeJson, const UiOperationStep& step)
{
    if (!step.compId.empty()) {
        // Level 1a: compId + compType + compText (most precise)
        auto results = UiOperationTreeParser::FindMatchingNodesWithAttributes(
            treeJson, step.compType, step.compId, step.compText);
        if (!results.empty()) {
            TAG_LOGI(AAFwkTag::INTENT, "resolve L1a (compId+type+text): found %{public}zu nodes", results.size());
            return results;
        }

        // Level 1b: compId + compType (compText as precision hint only, not a rejection criterion)
        auto resultsNoText = UiOperationTreeParser::FindMatchingNodesWithAttributes(
            treeJson, step.compType, step.compId, "");
        if (!resultsNoText.empty()) {
            TAG_LOGI(AAFwkTag::INTENT, "resolve L1b (compId+type, text mismatch): found %{public}zu nodes",
                resultsNoText.size());
            return resultsNoText;
        }

        TAG_LOGI(AAFwkTag::INTENT, "resolve L1 (compId): no match, falling back to L2");
    }

    // Level 2: compType + compText (fallback, discard compId)
    auto results = UiOperationTreeParser::FindMatchingNodesWithAttributes(
        treeJson, step.compType, "", step.compText);
    TAG_LOGI(AAFwkTag::INTENT, "resolve L2 (type+text): found %{public}zu nodes", results.size());
    return results;
}

bool UiOperationChainExecutor::CheckActionability(const MatchedNodeInfo& nodeInfo, const std::string& command)
{
    // "click" requires clickable=true
    if (command == "click") {
        return nodeInfo.clickable;
    }
    // "focus" — always allowed (any visible node can receive focus)
    if (command == "focus" || command == "blur") {
        return true;
    }
    // "input" / "setText" — allowed for now (text input validation layer TBD)
    if (command == "input" || command == "setText") {
        return true;
    }
    // Unknown command — allow by default (extensible)
    TAG_LOGW(AAFwkTag::INTENT, "CheckActionability: unknown command=%{public}s, allowing", command.c_str());
    return true;
}

// ============================================================
// Private: Content change callback registration (once per chain)
// ============================================================

void UiOperationChainExecutor::RegisterChainContentCallback()
{
    auto service = GetUiContentService(chain_.windowId);
    if (service == nullptr) {
        TAG_LOGE(AAFwkTag::INTENT, "RegisterChainContentCallback: service null");
        return;
    }

    Ace::ContentChangeConfig config;
    config.minReportTime = 0; // Report immediately

    auto self = shared_from_this();
    auto callback = [self](Ace::ChangeType type, const std::string& simpleTree) {
        TAG_LOGI(AAFwkTag::INTENT, "ContentChangeCallback: type=%{public}d, treeSize=%{public}zu",
            static_cast<int>(type), simpleTree.size());
        std::lock_guard<ffrt::mutex> lock(self->callbackMutex_);
        self->contentCallbackReceived_ = true;
        self->receivedChangeType_ = type;
        self->receivedCallbackTree_ = simpleTree;
    };

    service->RegisterContentChangeCallback(config, callback);
    TAG_LOGI(AAFwkTag::INTENT, "RegisterChainContentCallback done for window %{public}d", chain_.windowId);
}

void UiOperationChainExecutor::UnregisterChainContentCallback()
{
    auto service = GetUiContentService(chain_.windowId);
    if (service == nullptr) {
        return;
    }
    service->UnregisterContentChangeCallback();
    TAG_LOGI(AAFwkTag::INTENT, "UnregisterChainContentCallback done for window %{public}d", chain_.windowId);
}

bool UiOperationChainExecutor::WaitForPageChange(size_t stepIndex)
{
    int32_t timeoutMs;
    {
        std::lock_guard<ffrt::mutex> lock(mutex_);
        timeoutMs = chain_.pageChangeTimeoutMs > 0 ? chain_.pageChangeTimeoutMs : 5000;
    }

    constexpr int32_t POLL_INTERVAL_MS = 50;
    int32_t elapsed = 0;

    while (elapsed < timeoutMs) {
        {
            std::lock_guard<ffrt::mutex> lock(mutex_);
            if (cancelled_) {
                TAG_LOGI(AAFwkTag::INTENT, "WaitForPageChange step[%{public}zu] cancelled", stepIndex);
                return false;
            }
        }

        {
            std::lock_guard<ffrt::mutex> cbLock(callbackMutex_);
            if (contentCallbackReceived_) {
                if (receivedChangeType_ == Ace::ChangeType::PAGE) {
                    contentCallbackReceived_ = false;
                    return true;
                }
                // Received a non-PAGE change — reset and keep waiting
                TAG_LOGI(AAFwkTag::INTENT, "WaitForPageChange step[%{public}zu] received ChangeType=%{public}d, "
                    "not PAGE, continuing to wait", stepIndex, static_cast<int>(receivedChangeType_));
                contentCallbackReceived_ = false;
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(POLL_INTERVAL_MS));
        elapsed += POLL_INTERVAL_MS;
    }

    TAG_LOGE(AAFwkTag::INTENT, "WaitForPageChange step[%{public}zu] timeout after %{public}d ms",
        stepIndex, timeoutMs);
    return false;
}

// ============================================================
// Dump dispatch (mirrors UiSaService::Dump + DUMP_MAP)
// ============================================================

namespace {
constexpr int32_t SEND_COMMAND_WITH_NODEID = 3;
constexpr int32_t SEND_COMMAND_WITHOUT_NODEID = 2;
constexpr int32_t START_WEB_VIEW_TRANSLATE = 2;
constexpr int32_t GET_WEB_INFO_BY_REQUEST_PARAMS = 3;
constexpr int32_t EXE_APP_AI_FUNCTION_PARAMS = 3;
constexpr int32_t GET_STATE_MGMT_INFO_PARAMS = 5;
constexpr int32_t GET_SPECIFIED_CONTENT_OFFSETS_PARAMS = 3;
constexpr int32_t HIGHLIGHT_SPECIFIED_CONTENT_PARAMS = 3;
constexpr size_t BITS_UINT32 = sizeof(uint32_t) * 8;

uint32_t ParseComponentChangeEventMask(const std::vector<std::string>& params)
{
    int32_t paramSize = static_cast<int32_t>(params.size());
    if (paramSize <= 1) {
        return Ace::ComponentEventType::COMPONENT_EVENT_ALL;
    }
    if (params[1] == "x") {
        return Ace::ComponentEventType::COMPONENT_EVENT_NONE;
    }
    uint32_t mask = 0;
    for (int32_t i = 1; i < paramSize; i++) {
        int32_t bit = std::atoi(params[i].c_str());
        mask |= (1 << bit);
    }
    return mask;
}

std::string UnescapeContent(const std::string& param)
{
    std::string result;
    result.reserve(param.size());
    for (size_t i = 0; i < param.size(); i++) {
        if (param[i] == '\\' && i + 1 < param.size() && param[i + 1] == 'w') {
            result += ' ';
            i++;
        } else {
            result += param[i];
        }
    }
    return result;
}

Ace::ContentChangeConfig ParseContentChangeConfig(const std::vector<std::string>& params)
{
    Ace::ContentChangeConfig config;
    if (params.size() < 7) { // cmd + 6 config fields
        return config;
    }
    config.minReportTime = std::atoi(params[1].c_str());
    config.textContentRatio = std::atof(params[2].c_str());
    config.ignoreEventType = params[3];
    config.minWidth = std::atoi(params[4].c_str());
    config.minHeight = std::atoi(params[5].c_str());
    config.reportDelayTime = std::atoi(params[6].c_str());
    return config;
}
} // anonymous namespace

const std::map<std::string, UiOperationChainExecutor::DumpHandler>
    UiOperationChainExecutor::DUMP_HANDLER_MAP = {
    { "Connect", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        auto cb = [&i](std::string res) {
            i.push_back("Connect success, baseInfo: " + res.substr(0, 200));
        };
        if (!s->IsConnect()) {
            s->Connect(cb, nullptr);
            i.push_back("Connect called");
        } else {
            i.push_back("Already connected");
        }
    }},
    { "GetVisibleInspectorTree", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        auto cb = [&i](const std::string& data, int32_t index, bool isEnd) {
            if (isEnd) {
                i.push_back("Tree received, size=" + std::to_string(data.size()));
                i.push_back(data.substr(0, 500));
            }
        };
        Ace::ParamConfig paramConfig;
        paramConfig.interactionInfo = true;
        s->GetVisibleInspectorTree(cb, paramConfig);
    }},
    { "GetCurrentPageName", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        auto cb = [&i](std::string name) {
            i.push_back("CurrentPageName: " + name);
        };
        s->GetCurrentPageName(cb);
    }},
    { "SendCommand", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        if (p.size() == SEND_COMMAND_WITH_NODEID) {
            int32_t id = std::atoi(p[1].c_str());
            int32_t ret = s->SendCommand(id, p[2]);
            i.push_back("SendCommand nodeId=" + p[1] + " cmd=" + p[2] + " ret=" + std::to_string(ret));
        } else if (p.size() == SEND_COMMAND_WITHOUT_NODEID) {
            int32_t ret = s->SendCommand(p[1]);
            i.push_back("SendCommand cmd=" + p[1] + " ret=" + std::to_string(ret));
        }
    }},
    { "SendCommandAsync", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        if (p.size() >= SEND_COMMAND_WITH_NODEID) {
            int32_t id = std::atoi(p[1].c_str());
            int32_t ret = s->SendCommandAsync(id, p[2]);
            i.push_back("SendCommandAsync nodeId=" + p[1] + " cmd=" + p[2] + " ret=" + std::to_string(ret));
        }
    }},
    { "RegisterContentChangeCallback", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        auto cb = [&i](Ace::ChangeType type, const std::string& tree) {
            i.push_back("ContentChange type=" + std::to_string(static_cast<int>(type)) +
                " tree=" + tree.substr(0, 200));
        };
        auto config = ParseContentChangeConfig(p);
        s->RegisterContentChangeCallback(config, cb);
        i.push_back("RegisterContentChangeCallback done");
    }},
    { "UnregisterContentChangeCallback", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        s->UnregisterContentChangeCallback();
        i.push_back("UnregisterContentChangeCallback done");
    }},
    { "GetCurrentImagesShowing", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        auto cb = [&i](std::vector<std::pair<int32_t, std::shared_ptr<Media::PixelMap>>> imgs) {
            i.push_back("ImagesShowing count=" + std::to_string(imgs.size()));
            for (auto& [id, pm] : imgs) {
                i.push_back("  id=" + std::to_string(id) + " hasPixelMap=" + std::to_string(pm != nullptr));
            }
        };
        s->GetCurrentImagesShowing(cb);
    }},
    { "GetImagesById", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        int32_t idx = 1;
        int32_t paramSize = static_cast<int32_t>(p.size());
        if (idx >= paramSize) { return; }
        int32_t arkUISize = std::atoi(p[idx++].c_str());
        if (idx + arkUISize > paramSize) { return; }
        std::vector<int32_t> arkUIIds;
        for (int32_t j = 0; j < arkUISize; j++) {
            arkUIIds.emplace_back(std::atoi(p[idx++].c_str()));
        }
        auto arkUICb = [&i](int32_t winId,
            const std::unordered_map<int32_t, std::shared_ptr<Media::PixelMap>>& res,
            Ace::MultiImageQueryErrorCode code) {
            i.push_back("ArkUI images winId=" + std::to_string(winId)
                + " code=" + std::to_string(static_cast<int32_t>(code)));
        };
        if (idx >= paramSize) {
            s->GetImagesById(arkUIIds, arkUICb, {}, nullptr);
            return;
        }
        int32_t arkWebSize = std::atoi(p[idx++].c_str());
        std::map<int32_t, std::vector<int32_t>> arkWebs;
        for (int32_t j = 0; j < arkWebSize; j++) {
            if (idx >= paramSize) { break; }
            int32_t webId = std::atoi(p[idx++].c_str());
            if (idx >= paramSize) { break; }
            int32_t vecSize = std::atoi(p[idx++].c_str());
            std::vector<int32_t> vec;
            for (int32_t k = 0; k < vecSize && idx < paramSize; k++) {
                vec.emplace_back(std::atoi(p[idx++].c_str()));
            }
            arkWebs[webId] = vec;
        }
        auto arkWebCb = [&i](int32_t winId,
            const std::map<int32_t, std::map<int32_t, std::shared_ptr<Media::PixelMap>>>& res,
            Ace::MultiImageQueryErrorCode code) {
            i.push_back("ArkWeb images winId=" + std::to_string(winId)
                + " code=" + std::to_string(static_cast<int32_t>(code)));
        };
        s->GetImagesById(arkUIIds, arkUICb, arkWebs, arkWebCb);
    }},
    { "GetWebInfoByRequest", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        if (p.size() >= GET_WEB_INFO_BY_REQUEST_PARAMS) {
            int32_t webId = std::atoi(p[1].c_str());
            auto cb = [&i](int32_t winId, int32_t wbId, const std::string& req,
                const std::string& result, Ace::WebRequestErrorCode code) {
                i.push_back("WebInfo winId=" + std::to_string(winId) + " webId=" + std::to_string(wbId) +
                    " code=" + std::to_string(static_cast<int32_t>(code))
                    + " result=" + result.substr(0, 200));
            };
            s->GetWebInfoByRequest(webId, p[2], cb);
        }
    }},
    { "RegisterComponentChangeEventCallback", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        uint32_t mask = ParseComponentChangeEventMask(p);
        auto cb = [&i](std::string data) {
            i.push_back("ComponentChange: " + data.substr(0, 200));
        };
        s->RegisterComponentChangeEventCallback(cb, mask);
        i.push_back("RegisterComponentChangeEventCallback mask=" + std::bitset<BITS_UINT32>(mask).to_string());
    }},
    { "UnregisterComponentChangeEventCallback", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        s->UnregisterComponentChangeEventCallback();
        i.push_back("UnregisterComponentChangeEventCallback done");
    }},
    { "ExeAppAIFunction", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        if (p.size() >= EXE_APP_AI_FUNCTION_PARAMS) {
            auto cb = [&i](uint32_t result) {
                i.push_back("AIFunction result=" + std::to_string(result));
            };
            s->ExeAppAIFunction(p[1], p[2], cb);
        }
    }},
    { "GetWebViewCurrentLanguage", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        auto cb = [&i](std::string lang) {
            i.push_back("CurrentLanguage: " + lang);
        };
        s->GetWebViewCurrentLanguage(cb);
    }},
    { "StartWebViewTranslate", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        if (p.size() >= START_WEB_VIEW_TRANSLATE) {
            auto cb = [&i](int32_t code, std::string result) {
                i.push_back("WebViewTranslate code=" + std::to_string(code) + " result=" + result.substr(0, 200));
            };
            s->StartWebViewTranslate(p[1], cb);
        }
    }},
    { "GetStateMgmtInfo", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        if (p.size() >= GET_STATE_MGMT_INFO_PARAMS) {
            auto cb = [&i](std::vector<std::string> results) {
                i.push_back("StateMgmtInfo count=" + std::to_string(results.size()));
                for (auto& r : results) { i.push_back("  " + r); }
            };
            int32_t onlyVisible = std::atoi(p[4].c_str());
            s->GetStateMgmtInfo(p[1], p[2], p[3], cb, onlyVisible != 0);
        }
    }},
    { "RegisterTextChangeEventCallback", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        auto cb = [&i](std::string data) {
            i.push_back("TextChange: " + data.substr(0, 200));
        };
        s->RegisterTextChangeEventCallback(cb);
        i.push_back("RegisterTextChangeEventCallback done");
    }},
    { "UnregisterTextChangeEventCallback", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        s->UnregisterTextChangeEventCallback();
        i.push_back("UnregisterTextChangeEventCallback done");
    }},
    { "RegisterSelectTextEventCallback", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        auto cb = [&i](std::string data) {
            i.push_back("SelectText: " + data.substr(0, 200));
        };
        s->RegisterSelectTextEventCallback(cb);
        i.push_back("RegisterSelectTextEventCallback done");
    }},
    { "UnregisterSelectTextEventCallback", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        s->UnregisterSelectTextEventCallback();
        i.push_back("UnregisterSelectTextEventCallback done");
    }},
    { "GetSpecifiedContentOffsets", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        if (p.size() >= GET_SPECIFIED_CONTENT_OFFSETS_PARAMS) {
            int32_t id = std::atoi(p[1].c_str());
            std::string content = UnescapeContent(p[2]);
            auto cb = [&i](std::vector<std::pair<float, float>> offsets) {
                i.push_back("Offsets count=" + std::to_string(offsets.size()));
                for (auto& [x, y] : offsets) {
                    i.push_back("  (" + std::to_string(x) + ", " + std::to_string(y) + ")");
                }
            };
            s->GetSpecifiedContentOffsets(id, content, cb);
        }
    }},
    { "HighlightSpecifiedContent", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        if (p.size() >= HIGHLIGHT_SPECIFIED_CONTENT_PARAMS) {
            int32_t id = std::atoi(p[1].c_str());
            std::string content = UnescapeContent(p[2]);
            int32_t ret = s->HighlightSpecifiedContent(id, content, {}, "");
            i.push_back("HighlightSpecifiedContent id=" + p[1] + " ret=" + std::to_string(ret));
        }
    }},
    { "ExecuteChain", [](sptr<Ace::IUiContentService> s, auto& p, auto& i) {
        if (p.size() < 2) {
            i.push_back("Usage: ExecuteChain <chainJson>");
            return;
        }
        auto chain = UiOperationChain::ParseFromJson(p[1]);
        if (!chain.IsValid()) {
            i.push_back("Invalid chain JSON");
            return;
        }
        i.push_back("Chain parsed: windowId=" + std::to_string(chain.windowId) +
            " steps=" + std::to_string(chain.steps.size()));
        i.push_back("(Use ExecuteChain via programmatic API for full async execution)");
    }},
};

void UiOperationChainExecutor::HandleDump(const std::vector<std::string>& params,
    std::vector<std::string>& info)
{
    TAG_LOGI(AAFwkTag::INTENT, "HandleDump params=%{public}zu", params.size());

    if (params.empty()) {
        ShowDumpHelp(info);
        return;
    }

    if (params.size() < 2) {
        info.push_back("Usage: -o <windowId> <command> [args...]");
        ShowDumpHelp(info);
        return;
    }

    int32_t windowId = std::atoi(params[0].c_str());
    const std::string& cmd = params[1];

    TAG_LOGI(AAFwkTag::INTENT, "HandleDump windowId=%{public}d cmd=%{public}s", windowId, cmd.c_str());

    auto service = GetUiContentService(windowId);
    if (service == nullptr) {
        info.push_back("Error: cannot get UIContentService for window " + std::to_string(windowId));
        return;
    }

    if (cmd != "Connect" && !service->IsConnect()) {
        info.push_back("Not connected, auto-connecting...");
        auto connectCb = [](std::string) {};
        service->Connect(connectCb);
    }

    std::vector<std::string> subParams(params.begin() + 1, params.end());

    auto it = DUMP_HANDLER_MAP.find(cmd);
    if (it == DUMP_HANDLER_MAP.end()) {
        info.push_back("Unknown command: " + cmd);
        ShowDumpHelp(info);
        return;
    }

    it->second(service, subParams, info);
    info.push_back("Done.");
}

void UiOperationChainExecutor::ShowDumpHelp(std::vector<std::string>& info)
{
    info.push_back("UI Operation Dump Commands:");
    info.push_back("  Usage: -o <windowId> <command> [args...]");
    info.push_back("  Commands:");
    info.push_back("    Connect");
    info.push_back("    GetVisibleInspectorTree");
    info.push_back("    GetCurrentPageName");
    info.push_back("    SendCommand <nodeId> <command>");
    info.push_back("    SendCommand <command>               (global, no nodeId)");
    info.push_back("    SendCommandAsync <nodeId> <command>");
    info.push_back("    RegisterContentChangeCallback [minReportTime textRatio ignoreEvt minW minH delayMs]");
    info.push_back("    UnregisterContentChangeCallback");
    info.push_back("    GetCurrentImagesShowing");
    info.push_back("    GetImagesById <arkUISize> [ids...] <arkWebSize> [webId vecSize [ids...]]");
    info.push_back("    GetWebInfoByRequest <webId> <request>");
    info.push_back("    RegisterComponentChangeEventCallback [bit0 bit1 ...]  (x=none, empty=all)");
    info.push_back("    UnregisterComponentChangeEventCallback");
    info.push_back("    ExeAppAIFunction <funcName> <paramsJson>");
    info.push_back("    GetWebViewCurrentLanguage");
    info.push_back("    StartWebViewTranslate <data>");
    info.push_back("    GetStateMgmtInfo <component> <property> <jsonPath> <onlyVisible>");
    info.push_back("    RegisterTextChangeEventCallback");
    info.push_back("    UnregisterTextChangeEventCallback");
    info.push_back("    RegisterSelectTextEventCallback");
    info.push_back("    UnregisterSelectTextEventCallback");
    info.push_back("    GetSpecifiedContentOffsets <id> <content>");
    info.push_back("    HighlightSpecifiedContent <id> <content>");
    info.push_back("    ExecuteChain <chainJson>");
}

} // namespace AAFwk
} // namespace OHOS
