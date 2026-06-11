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

#ifndef OHOS_ABILITY_RUNTIME_UI_OPERATION_CHAIN_EXECUTOR_H
#define OHOS_ABILITY_RUNTIME_UI_OPERATION_CHAIN_EXECUTOR_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "cpp/mutex.h"
#include "insight_intent/ui_operation_chain_param.h"
#include "insight_intent/ui_operation_tree_parser.h"
#include "iremote_object.h"
#include "param_config.h"
#include "ui_content_errors.h"
#include "ui_content_proxy_error_code.h"
#include "ui_content_service_interface.h"

class EventHandler;

namespace OHOS {
namespace AppExecFwk {
class EventHandler;
} // namespace AppExecFwk

namespace Media {
class PixelMap;
} // namespace Media

namespace Ace {
class IUiContentService;
} // namespace Ace
} // namespace OHOS

namespace OHOS {
namespace AAFwk {

/**
 * @brief Error codes for UI operation chain execution.
 */
enum UiOperationErrorCode {
    UI_OPERATION_ERR_OK = 0,
    UI_OPERATION_ERR_CONNECT_FAILED = 100,
    UI_OPERATION_ERR_TREE_TIMEOUT = 101,
    UI_OPERATION_ERR_COMPONENT_NOT_FOUND = 102,
    UI_OPERATION_ERR_COMMAND_FAILED = 103,
    UI_OPERATION_ERR_CHAIN_TIMEOUT = 104,
    UI_OPERATION_ERR_WINDOW_NOT_FOUND = 105,
    UI_OPERATION_ERR_CANCELLED = 106,
    UI_OPERATION_ERR_ACTION_NOT_ALLOWED = 107,     // Component not actionable (e.g. click on non-clickable)
    UI_OPERATION_ERR_PAGE_CHANGE_TIMEOUT = 108,    // ContentChangeCallback PAGE not received within timeout
};

/**
 * @brief Executor that drives a UiOperationChain step by step.
 *
 * Supports:
 * - Two-level component matching (compId+type+text > type+text)
 * - Actionability checks (e.g. clickable=true for "click" command)
 * - RegisterContentChangeCallback once per chain (before first step)
 * - Callback-wait after each step (waitForPageChange mode)
 * - UnregisterContentChangeCallback on chain completion
 * - All standalone UI session operations for dump-based testing.
 */
class UiOperationChainExecutor : public std::enable_shared_from_this<UiOperationChainExecutor> {
public:
    using ChainCompleteCallback = std::function<void(int32_t resultCode, int32_t failedStepIndex)>;

    UiOperationChainExecutor() = default;
    ~UiOperationChainExecutor() = default;

    // ================================================================
    // Chain execution
    // ================================================================

    /**
     * @brief Start executing the operation chain.
     * @param chain The chain definition (windowId, steps, timeout, waitForPageChange).
     * @param handler EventHandler for posting callbacks to the caller's thread.
     * @param onComplete Called when chain finishes (all steps done or error).
     */
    void ExecuteChain(const UiOperationChain& chain,
        std::shared_ptr<AppExecFwk::EventHandler> handler, ChainCompleteCallback onComplete);

    /**
     * @brief Cancel an in-progress chain execution.
     */
    void Cancel();

    // ================================================================
    // Standalone UI session operations (mirroring ui_sa_sample)
    // All methods accept windowId to identify the target window.
    // ================================================================

    /** @brief Connect to a window's UIContentService. */
    void ConnectToWindow(int32_t windowId, std::shared_ptr<AppExecFwk::EventHandler> handler,
        std::function<void(bool success, const std::string& baseInfo)> onResult);

    /** @brief Get the visible inspector tree. Optionally pass ParamConfig. */
    void GetVisibleInspectorTree(int32_t windowId, const Ace::ParamConfig& config,
        std::function<void(bool success, const std::string& treeJson)> onResult);

    /** @brief Get current page name. */
    void GetCurrentPageName(int32_t windowId,
        std::function<void(bool success, const std::string& pageName)> onResult);

    /** @brief Send command to a node (sync). */
    int32_t SendCommand(int32_t windowId, int32_t nodeId, const std::string& command);

    /** @brief Send global command (async, no node target). */
    int32_t SendCommand(int32_t windowId, const std::string& command);

    /** @brief Send command to a node (async). */
    int32_t SendCommandAsync(int32_t windowId, int32_t nodeId, const std::string& command);

    /** @brief Register content change callback with config. */
    void RegisterContentChangeCallback(int32_t windowId, const Ace::ContentChangeConfig& config,
        std::function<void(Ace::ChangeType, const std::string&)> callback);

    /** @brief Unregister content change callback. */
    int32_t UnregisterContentChangeCallback(int32_t windowId);

    /** @brief Get current images showing in the window. */
    void GetCurrentImagesShowing(int32_t windowId,
        std::function<void(std::vector<std::pair<int32_t, std::shared_ptr<Media::PixelMap>>>)> onResult);

    /** @brief Get images by ID (ArkUI + ArkWeb). */
    void GetImagesById(int32_t windowId,
        const std::vector<int32_t>& arkUIIds,
        std::function<void(int32_t,
            const std::unordered_map<int32_t, std::shared_ptr<Media::PixelMap>>&,
            Ace::MultiImageQueryErrorCode)> arkUICallback,
        const std::map<int32_t, std::vector<int32_t>>& arkWebs,
        std::function<void(int32_t,
            const std::map<int32_t, std::map<int32_t, std::shared_ptr<Media::PixelMap>>>&,
            Ace::MultiImageQueryErrorCode)> arkWebCallback);

    /** @brief Get web info by request (e.g. DOM tree). */
    void GetWebInfoByRequest(int32_t windowId, int32_t webId, const std::string& request,
        Ace::GetWebInfoByRequestCallback callback);

    /** @brief Register component change event callback with event mask. */
    void RegisterComponentChangeEventCallback(int32_t windowId, uint32_t mask,
        std::function<void(const std::string&)> callback);

    /** @brief Unregister component change event callback. */
    int32_t UnregisterComponentChangeEventCallback(int32_t windowId);

    /** @brief Execute an AI function on the app. */
    void ExeAppAIFunction(int32_t windowId, const std::string& funcName, const std::string& params,
        std::function<void(uint32_t)> onResult);

    /** @brief Get web view current language. */
    void GetWebViewCurrentLanguage(int32_t windowId,
        std::function<void(const std::string&)> onResult);

    /** @brief Start web view translate. */
    void StartWebViewTranslate(int32_t windowId, const std::string& data,
        std::function<void(int32_t code, const std::string& result)> onResult);

    /** @brief Get state management info. */
    void GetStateMgmtInfo(int32_t windowId, const std::string& componentName,
        const std::string& propertyName, const std::string& jsonPath,
        std::function<void(std::vector<std::string>)> onResult, bool onlyVisible = false);

    /** @brief Register text change event callback. */
    void RegisterTextChangeEventCallback(int32_t windowId,
        std::function<void(const std::string&)> callback);

    /** @brief Unregister text change event callback. */
    int32_t UnregisterTextChangeEventCallback(int32_t windowId);

    /** @brief Register select text event callback. */
    void RegisterSelectTextEventCallback(int32_t windowId,
        std::function<void(const std::string&)> callback);

    /** @brief Unregister select text event callback. */
    int32_t UnregisterSelectTextEventCallback(int32_t windowId);

    /** @brief Get specified content offsets. */
    void GetSpecifiedContentOffsets(int32_t windowId, int32_t id, const std::string& content,
        std::function<void(std::vector<std::pair<float, float>>)> onResult);

    /** @brief Highlight specified content. */
    int32_t HighlightSpecifiedContent(int32_t windowId, int32_t id, const std::string& content,
        const std::vector<std::string>& nodeIds = {}, const std::string& configs = "");

    // ================================================================
    // Dump entry point
    // ================================================================

    void HandleDump(const std::vector<std::string>& params,
        std::vector<std::string>& info);

    void ShowDumpHelp(std::vector<std::string>& info);

private:
    sptr<Ace::IUiContentService> GetUiContentService(int32_t windowId);

    void DoConnect(sptr<Ace::IUiContentService> service,
        std::function<void(bool success)> onConnected);

    void DoRequestInspectorTree(sptr<Ace::IUiContentService> service,
        const Ace::ParamConfig& config,
        std::function<void(std::string, bool)> onTreeReady);

    /**
     * @brief Unwrap the {"0": "<escaped-json>"} envelope from ArkUI GetVisibleInspectorTree.
     * Returns the inner Inspector tree JSON string, or the original string if unwrapping fails.
     */
    static std::string UnwrapInspectorTree(const std::string& wrappedJson);

    /** @brief Execute a single chain step. */
    void ExecuteStep(size_t stepIndex);

    /** @brief Drive the next step in the chain. */
    void ProceedToStep(size_t stepIndex);

    /** @brief Handle chain completion. */
    void OnChainComplete(int32_t resultCode, int32_t failedStepIndex);

    // --- Extended step execution (for GuiIntent) ---

    /**
     * @brief Execute step with two-level matching + actionability check + callback-wait.
     * Called by ExecuteStep when chain.waitForPageChange is true.
     */
    void ExecuteStepWithPageWait(size_t stepIndex);

    /**
     * @brief Resolve matching nodes with two-level priority:
     *   1) compId + compType + compText
     *   2) compType + compText (fallback)
     */
    std::vector<MatchedNodeInfo> ResolveMatchingNodes(const std::string& treeJson,
        const UiOperationStep& step);

    /**
     * @brief Check if the actionType is allowed on the given node.
     * e.g. "click" requires clickable=true.
     */
    bool CheckActionability(const MatchedNodeInfo& nodeInfo, const std::string& command);

    /**
     * @brief Register content change callback for the current chain window.
     */
    void RegisterChainContentCallback();

    /**
     * @brief Unregister content change callback for the current chain window.
     */
    void UnregisterChainContentCallback();

    /**
     * @brief Wait for a PAGE change callback within timeout.
     * @return true if PAGE callback received within timeout.
     */
    bool WaitForPageChange(size_t stepIndex);

    // --- Dump sub-handlers ---
    using DumpHandler = std::function<void(sptr<Ace::IUiContentService>, const std::vector<std::string>&,
        std::vector<std::string>&)>;
    static const std::map<std::string, DumpHandler> DUMP_HANDLER_MAP;

    void DumpConnect(sptr<Ace::IUiContentService> service, const std::vector<std::string>& params,
        std::vector<std::string>& info);
    void DumpGetVisibleInspectorTree(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpGetCurrentPageName(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpSendCommand(sptr<Ace::IUiContentService> service, const std::vector<std::string>& params,
        std::vector<std::string>& info);
    void DumpSendCommandAsync(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpRegisterContentChangeCallback(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpUnregisterContentChangeCallback(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpGetCurrentImagesShowing(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpGetImagesById(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpGetWebInfoByRequest(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpRegisterComponentChangeEventCallback(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpUnregisterComponentChangeEventCallback(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpExeAppAIFunction(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpGetWebViewCurrentLanguage(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpStartWebViewTranslate(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpGetStateMgmtInfo(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpRegisterTextChangeEventCallback(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpUnregisterTextChangeEventCallback(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpRegisterSelectTextEventCallback(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpUnregisterSelectTextEventCallback(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpGetSpecifiedContentOffsets(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpHighlightSpecifiedContent(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);
    void DumpExecuteChain(sptr<Ace::IUiContentService> service,
        const std::vector<std::string>& params, std::vector<std::string>& info);

    // --- State ---
    ffrt::mutex mutex_;
    UiOperationChain chain_;
    ChainCompleteCallback onComplete_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    std::string accumulatedTree_;

    // Cached proxies: windowId -> (remoteObj, service)
    struct ServiceCache {
        sptr<IRemoteObject> remoteObj;
        sptr<Ace::IUiContentService> service;
    };
    std::map<int32_t, ServiceCache> serviceCache_;

    bool cancelled_ = false;
    bool executing_ = false;

    // --- Content change callback state ---
    ffrt::mutex callbackMutex_;
    bool contentCallbackReceived_ = false;
    Ace::ChangeType receivedChangeType_ = Ace::ChangeType::PAGE;
    std::string receivedCallbackTree_;
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_OPERATION_CHAIN_EXECUTOR_H
