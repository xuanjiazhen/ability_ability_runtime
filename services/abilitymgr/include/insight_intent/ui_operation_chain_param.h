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

#ifndef OHOS_ABILITY_RUNTIME_UI_OPERATION_CHAIN_PARAM_H
#define OHOS_ABILITY_RUNTIME_UI_OPERATION_CHAIN_PARAM_H

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
namespace AAFwk {

/**
 * @brief Single step in a UI operation chain.
 * Describes how to find a target component and what command to execute on it.
 *
 * Component matching priority (two-level, caller-controlled):
 *   1) compId + compType + compText  (preferred)
 *   2) compType + compText           (fallback when compId fails)
 */
struct UiOperationStep {
    // --- Component matching fields ---
    std::string compId;           // Component ID to match against $attrs.id (empty = skip)
    std::string compType;         // Component type, e.g. "Button", "Text"
    std::string compName;         // Component name (reserved)
    std::string compText;         // Component text/content to match against $attrs.content/$attrs.label
    int32_t compIndex = 0;        // Target index when multiple matches (list item position)

    // --- Action fields ---
    std::string command;          // Action to execute, e.g. "click"
    bool useAsync = false;        // true=SendCommandAsync, false=SendCommand(synchronous)
    int32_t timeoutMs = 5000;     // Per-step timeout in milliseconds

    // --- Legacy aliases (kept for backward compatibility with existing chain executor) ---
    std::string componentType;    // = compType (alias)
    std::string contentMatcher;   // = compText (alias)
    int32_t matchIndex = 0;       // = compIndex (alias)

    /**
     * @brief Sync legacy fields from the new fields (called after parsing).
     */
    void SyncLegacy()
    {
        componentType = compType;
        contentMatcher = compText;
        matchIndex = compIndex;
    }
};

/**
 * @brief A chain of UI operations to execute sequentially on a target window.
 * Each step depends on the success of the previous step.
 * Failure at any step interrupts the entire chain.
 */
struct UiOperationChain {
    int32_t windowId = -1;                        // Target window ID
    std::vector<UiOperationStep> steps;           // Ordered list of operation steps
    int32_t totalTimeoutMs = 30000;               // Overall chain timeout in milliseconds

    // --- Callback-wait fields (for page transition verification) ---
    bool waitForPageChange = false;               // If true, wait for RegisterContentChangeCallback after each step
    int32_t pageChangeTimeoutMs = 5000;           // Max wait for PAGE change callback (per step)

    /**
     * @brief Parse a UiOperationChain from a JSON string.
     * @param jsonStr JSON string describing the chain.
     * Expected format:
     * {
     *   "windowId": 1,
     *   "totalTimeoutMs": 30000,
     *   "steps": [
     *     { "compType": "Button", "compText": "Submit", "command": "click" },
     *     { "compType": "Text", "compText": "result", "command": "focus" }
     *   ]
     * }
     * @return Parsed UiOperationChain. windowId will be -1 if parsing fails.
     */
    static UiOperationChain ParseFromJson(const std::string& jsonStr);

    /**
     * @brief Serialize the chain to a JSON string.
     * @return JSON string representation.
     */
    std::string ToJsonString() const;

    /**
     * @brief Check if the chain is valid (has valid windowId and at least one step).
     * @return true if valid.
     */
    bool IsValid() const;
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_OPERATION_CHAIN_PARAM_H
