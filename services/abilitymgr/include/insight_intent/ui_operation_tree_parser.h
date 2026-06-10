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

#ifndef OHOS_ABILITY_RUNTIME_UI_OPERATION_TREE_PARSER_H
#define OHOS_ABILITY_RUNTIME_UI_OPERATION_TREE_PARSER_H

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
namespace AAFwk {

/**
 * @brief Extended match result containing node attributes.
 * Used by GuiIntentExecutor for compId matching and actionability checks.
 */
struct MatchedNodeInfo {
    int32_t nodeId = -1;       // $ID (ARKUI real node id for SendCommand)
    std::string compId;        // $attrs.id  (component's own compId)
    std::string compType;      // $type      (component type, e.g. "Button")
    std::string content;       // $attrs.content
    std::string label;         // $attrs.label
    bool clickable = false;    // $attrs.clickable
};

/**
 * @brief Utility class for parsing ACE Inspector tree JSON and finding components.
 *
 * The ACE Inspector tree JSON uses these fields:
 * - "$ID": integer node ID (used for SendCommand)
 * - "$type": component type string, e.g. "Button", "Text", "Column"
 * - "$attrs": object containing attributes like "content", "id", "label", "clickable"
 * - "$children": array of child nodes
 *
 * This parser performs depth-first traversal to find nodes matching
 * given component type and content criteria.
 */
class UiOperationTreeParser {
public:
    /**
     * @brief Find all node IDs whose type and content match the given criteria.
     * @param treeJson Full inspector tree JSON string (may be concatenated from multi-part callback).
     * @param componentType Component type to match (case-insensitive), e.g. "Button".
     * @param contentMatcher Substring to match against node content (empty = match all).
     *                        Searches "$attrs.content" and "$attrs.label" fields.
     * @return Vector of matching node IDs (ordered by depth-first traversal).
     */
    static std::vector<int32_t> FindMatchingNodes(const std::string& treeJson,
        const std::string& componentType, const std::string& contentMatcher);

    /**
     * @brief Find nodes with full attribute extraction, supporting compId matching.
     *
     * Matching rules (caller-controlled priority):
     * - compId non-empty: match $attrs.id == compId (exact, case-insensitive)
     * - compId empty:     match type only (or type+content)
     *
     * @param treeJson        Inspector tree JSON.
     * @param componentType   Component type to match (case-insensitive).
     * @param compId          Component ID to match against $attrs.id (empty = skip compId check).
     * @param contentMatcher  Substring to match against $attrs.content / $attrs.label (empty = skip).
     * @return Vector of MatchedNodeInfo (ordered by depth-first traversal).
     */
    static std::vector<MatchedNodeInfo> FindMatchingNodesWithAttributes(const std::string& treeJson,
        const std::string& componentType, const std::string& compId,
        const std::string& contentMatcher);

    /**
     * @brief Get the total number of nodes in the tree (for logging/diagnostics).
     * @param treeJson Full inspector tree JSON string.
     * @return Number of nodes, or 0 on parse failure.
     */
    static int32_t GetNodeCount(const std::string& treeJson);
};

} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_UI_OPERATION_TREE_PARSER_H
