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

#ifndef OHOS_ABILITY_RUNTIME_GUI_INTENT_EXECUTOR_H
#define OHOS_ABILITY_RUNTIME_GUI_INTENT_EXECUTOR_H

#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "cpp/mutex.h"
#include "insight_intent/ui_operation_chain_executor.h"
#include "insight_intent/ui_operation_chain_param.h"
#include "insight_intent/ui_operation_tree_parser.h"
#include "param_config.h"

#include "event_handler.h"

namespace OHOS {
namespace AAFwk {

/**
 * @brief Single page's parameters from the input array.
 *
 * Input format: [{pageName, parameters?, intentName}, ...]
 * All elements share the same intentName.
 * First element is the home/main page.
 */
struct GuiIntentInputItem {
    std::string pageName;           // Target page (unique within the array)
    std::string intentName;         // Intent identifier (same across all items)
    std::string compId;             // Component ID
    std::string compType;           // Component type, e.g. "Button"
    std::string compName;           // Component name (reserved)
    std::string compText;           // Component text/content
    int32_t compIndex = 0;          // Component index (list item position)
    std::string nextPageName;       // Expected next page after action
    std::string actionType;         // Action type, e.g. "click"

    /**
     * @brief Parse input items from JSON string.
     * Expected format: [{"pageName":"main","intentName":"...","compId":"...",...}, ...]
     * @return Vector of GuiIntentInputItem, empty on parse failure.
     */
    static std::vector<GuiIntentInputItem> ParseFromJson(const std::string& inputJson);
};

/**
 * @brief GuiIntentDecoratorInfo — as defined in the design spec.
 * Maps an intent to a set of pages, each containing components with action descriptions.
 *
 * Each element in the array represents one page in the intent flow.
 * The first element is the home/entry page.
 * During execution, input's pageName is used to locate the matching element.
 */
struct GuiIntentDecoratorInfo {
    // --- Intent metadata ---
    std::string intentName;               // Intent unique identifier (same across all elements)
    std::string domain;                   // Intent domain
    std::string intentVersion;            // Intent version
    std::string displayName;              // Display name
    std::string llmDescription;           // LLM description (optional)
    std::string keywords;                 // Search keywords (optional)

    // --- Page binding ---
    std::string pageName;                 // Current page (unique within the collection)

    // --- Component entries for this page ---
    struct GuiIntentComponentEntry {
        std::string compId;               // Component ID for matching
        std::string componentIdWildcard;  // Wildcard pattern (optional)
        std::string description;          // Human-readable description
        std::string compType;             // Component type, e.g. "Button", "List"
        std::string nextPageName;         // Target page after interacting with this component
        std::string defaultActionType;    // Default action, e.g. "click"
        std::string defaultCompText;      // Default text matcher

        struct ListItemEntry {
            std::string compId;
            std::string name;
            std::string nextPageName;
        };
        std::vector<ListItemEntry> listItems;   // List sub-components

        struct TabItemEntry {
            std::string compId;
            std::string name;
            std::string nextPageName;
        };
        std::vector<TabItemEntry> tabItems;     // Tabs sub-components
    };
    std::vector<GuiIntentComponentEntry> components;
};

/**
 * @brief Callback for GuiIntent execution completion.
 * @param resultCode 0 on success, error code on failure.
 * @param failedPageName Page where execution failed (empty on success).
 */
using GuiIntentCompleteCallback = std::function<void(int32_t resultCode, const std::string& failedPageName)>;

/**
 * @brief GuiIntentExecutor: multi-page GUI intent orchestration layer.
 *
 * Execution flow:
 *   1. Receive ordered [{pageName, parameters, intentName}] array
 *   2. Resolve GuiIntentDecoratorInfo for each page (hardcoded for now)
 *   3. Register ContentChangeCallback once
 *   4. For each input item:
 *      a) Get visible inspector tree via window
 *      b) Two-level component matching (compId+type+text > type+text)
 *      c) Actionability check (e.g. click → clickable=true)
 *      d) sendCommand(actionType) on matched ARKUI node id
 *      e) Wait for ContentChangeCallback(PAGE) → verify page transition
 *      f) On failure → interrupt chain
 *   5. Unregister ContentChangeCallback
 *   6. Report result
 */
class GuiIntentExecutor : public std::enable_shared_from_this<GuiIntentExecutor> {
public:
    GuiIntentExecutor() = default;
    ~GuiIntentExecutor();

    /**
     * @brief Execute a GUI intent.
     * @param windowId      Target window ID.
     * @param inputItems    Ordered execution plan.
     * @param handler       EventHandler for thread scheduling.
     * @param onComplete    Completion callback.
     */
    void Execute(int32_t windowId,
        const std::vector<GuiIntentInputItem>& inputItems,
        std::shared_ptr<AppExecFwk::EventHandler> handler,
        GuiIntentCompleteCallback onComplete);

    /**
     * @brief Cancel an in-progress execution.
     */
    void Cancel();

    /**
     * @brief Build hardcoded GuiIntentDecoratorInfo[] for testing.
     * Called before Execute to populate the decorator data.
     * Replace with DB query later.
     *
     * @param intentName Target intent to load (empty = load all hardcoded).
     * @return Vector of GuiIntentDecoratorInfo for the given intent.
     */
    static std::vector<GuiIntentDecoratorInfo> BuildHardcodedData(const std::string& intentName = "");

private:
    void StartNextStep();
    void ExecuteCurrentStep();
    void OnStepComponentNotFound(const std::string& failReason);
    void OnStepActionNotAllowed();
    void OnStepCommandFailed(int32_t cmdRet);
    void OnStepPageChangeTimeout();
    void OnAllStepsComplete();
    void OnFailed(int32_t resultCode, const std::string& failedPageName);

    // Window & session management
    void ConnectWindow(std::function<void(bool success)> onResult);
    void GetInspectorTree(std::function<void(const std::string& treeJson, bool success)> onResult);

    // Component matching (two-level priority)
    std::vector<MatchedNodeInfo> MatchComponent(const std::string& treeJson, const GuiIntentInputItem& item);

    // Look up decorator info by pageName from loaded decoratorInfos_
    const GuiIntentDecoratorInfo* FindDecoratorByPage(const std::string& pageName) const;

    // Actionability check
    bool CheckActionable(const MatchedNodeInfo& node, const std::string& actionType);

    // Content change callback (registered once, persists across steps)
    void RegisterContentCallback();
    void UnregisterContentCallback();
    bool WaitForPageChange();

    // --- State ---
    ffrt::mutex mutex_;
    bool cancelled_ = false;
    bool executing_ = false;

    int32_t windowId_ = -1;
    std::vector<GuiIntentInputItem> inputItems_;
    size_t currentStepIndex_ = 0;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
    GuiIntentCompleteCallback onComplete_;

    // Decorator data — resolves intentName to pages and components.
    // Currently hardcoded; will be replaced by DB query later.
    std::vector<GuiIntentDecoratorInfo> decoratorInfos_;

    // Content change callback state
    ffrt::mutex callbackMutex_;
    bool callbackReceived_ = false;
    Ace::ChangeType receivedChangeType_ = Ace::ChangeType::PAGE;
    std::string receivedCallbackTree_;

    sptr<IRemoteObject> cachedRemoteObj_;
    sptr<Ace::IUiContentService> cachedService_;
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_GUI_INTENT_EXECUTOR_H
