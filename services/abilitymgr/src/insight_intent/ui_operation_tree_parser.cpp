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

#include "insight_intent/ui_operation_tree_parser.h"

#include <algorithm>
#include <cctype>

#include "hilog_tag_wrapper.h"

#include "nlohmann/json.hpp"

namespace OHOS {
namespace AAFwk {

namespace {
// ACE Inspector tree JSON field names (ArkUI may output with or without "$" prefix)
constexpr const char* FIELD_ID = "$ID";
constexpr const char* FIELD_ID_ALT = "ID";
constexpr const char* FIELD_TYPE = "$type";
constexpr const char* FIELD_TYPE_ALT = "type";
constexpr const char* FIELD_CHILDREN = "$children";
constexpr const char* FIELD_CHILDREN_ALT = "children";
constexpr const char* FIELD_ATTRS = "$attrs";
constexpr const char* FIELD_ATTRS_ALT = "attrs";
constexpr const char* ATTR_CONTENT = "content";
constexpr const char* ATTR_LABEL = "label";
constexpr const char* NODE_COMP_ID = "compid";
constexpr const char* ATTR_COMP_ID = "id";
constexpr const char* ATTR_CLICKABLE = "clickable";
constexpr size_t MAX_TREE_DUMP_LEN = 2000;

// Lookup helper: try primary key first (with "$"), fallback to alternate (without "$")
const nlohmann::json* LookupField(const nlohmann::json& node, const char* primary, const char* alt)
{
    if (node.contains(primary)) {
        return &node[primary];
    }
    if (alt && node.contains(alt)) {
        return &node[alt];
    }
    return nullptr;
}

bool HasField(const nlohmann::json& node, const char* primary, const char* alt)
{
    return node.contains(primary) || (alt && node.contains(alt));
}

bool IsStringField(const nlohmann::json& node, const char* primary, const char* alt)
{
    auto* val = LookupField(node, primary, alt);
    return val != nullptr && val->is_string();
}

bool IsIntField(const nlohmann::json& node, const char* primary, const char* alt)
{
    auto* val = LookupField(node, primary, alt);
    return val != nullptr && val->is_number_integer();
}

bool IsArrayField(const nlohmann::json& node, const char* primary, const char* alt)
{
    auto* val = LookupField(node, primary, alt);
    return val != nullptr && val->is_array();
}

bool IsObjectField(const nlohmann::json& node, const char* primary, const char* alt)
{
    auto* val = LookupField(node, primary, alt);
    return val != nullptr && val->is_object();
}

std::string GetStringField(const nlohmann::json& node, const char* primary, const char* alt,
    const std::string& def = "")
{
    auto* val = LookupField(node, primary, alt);
    if (val != nullptr && val->is_string()) {
        return val->get<std::string>();
    }
    return def;
}

int32_t GetIntField(const nlohmann::json& node, const char* primary, const char* alt, int32_t def = -1)
{
    auto* val = LookupField(node, primary, alt);
    if (val != nullptr && val->is_number_integer()) {
        return val->get<int32_t>();
    }
    return def;
}

const nlohmann::json& GetFieldRef(const nlohmann::json& node, const char* primary, const char* alt)
{
    if (node.contains(primary)) {
        return node[primary];
    }
    if (alt && node.contains(alt)) {
        return node[alt];
    }
    static const nlohmann::json nullJson;
    return nullJson;
}

bool CaseInsensitiveEqual(const std::string& a, const std::string& b)
{
    if (a.size() != b.size()) {
        return false;
    }
    return std::equal(a.begin(), a.end(), b.begin(),
        [](unsigned char ca, unsigned char cb) { return std::tolower(ca) == std::tolower(cb); });
}

bool CaseInsensitiveContains(const std::string& haystack, const std::string& needle)
{
    if (needle.empty()) {
        return true;
    }
    if (haystack.size() < needle.size()) {
        return false;
    }
    auto it = std::search(haystack.begin(), haystack.end(), needle.begin(), needle.end(),
        [](unsigned char ch1, unsigned char ch2) { return std::tolower(ch1) == std::tolower(ch2); });
    return it != haystack.end();
}

void ExtractNodeAttrs(const nlohmann::json& node, MatchedNodeInfo& info)
{
    // Priority 1: node["compid"] — DumpSimplifyCommonInfo format (frame_node.cpp:1385)
    if (node.contains(NODE_COMP_ID) && node[NODE_COMP_ID].is_string()) {
        info.compId = node[NODE_COMP_ID].get<std::string>();
    }

    // Priority 2: node["id"] — old inspector tree format (simplified_inspector FillInspectorAttrs)
    if (info.compId.empty() && node.contains(ATTR_COMP_ID) && node[ATTR_COMP_ID].is_string()) {
        info.compId = node[ATTR_COMP_ID].get<std::string>();
    }

    // Clickable is written to node top-level by DumpSimplifyCommonInfoOnlyForParamConfig
    // (frame_node.cpp:1408), not inside $attrs.
    if (node.contains(ATTR_CLICKABLE) && node[ATTR_CLICKABLE].is_boolean()) {
        info.clickable = node[ATTR_CLICKABLE].get<bool>();
    }

    // $attrs / attrs — try both "$attrs" and "attrs" keys
    if (!IsObjectField(node, FIELD_ATTRS, FIELD_ATTRS_ALT)) {
        return;
    }
    const auto& attrs = GetFieldRef(node, FIELD_ATTRS, FIELD_ATTRS_ALT);
    // Priority 3: $attrs.id — new inspector tree (ToTreeJson) or old fullAttrs format
    if (info.compId.empty() && attrs.contains(ATTR_COMP_ID) && attrs[ATTR_COMP_ID].is_string()) {
        info.compId = attrs[ATTR_COMP_ID].get<std::string>();
    }
    if (attrs.contains(ATTR_CONTENT) && attrs[ATTR_CONTENT].is_string()) {
        info.content = attrs[ATTR_CONTENT].get<std::string>();
    }
    if (attrs.contains(ATTR_LABEL) && attrs[ATTR_LABEL].is_string()) {
        info.label = attrs[ATTR_LABEL].get<std::string>();
    }
    // Fallback: clickable may also appear in $attrs for some formats
    if (!info.clickable && attrs.contains(ATTR_CLICKABLE) && attrs[ATTR_CLICKABLE].is_boolean()) {
        info.clickable = attrs[ATTR_CLICKABLE].get<bool>();
    }
}

void TraverseNode(const nlohmann::json& node, const std::string& componentType,
    const std::string& contentMatcher, std::vector<int32_t>& results, int32_t& totalCount)
{
    if (!node.is_object()) {
        return;
    }
    totalCount++;

    // Check type match (case-insensitive), try "$type" then "type"
    bool typeMatched = false;
    std::string nodeType = GetStringField(node, FIELD_TYPE, FIELD_TYPE_ALT);
    if (!nodeType.empty()) {
        typeMatched = CaseInsensitiveEqual(nodeType, componentType);
    }

    // Check content match if type matched
    if (typeMatched) {
        bool contentMatched = true;
        if (!contentMatcher.empty()) {
            contentMatched = false;
            if (IsObjectField(node, FIELD_ATTRS, FIELD_ATTRS_ALT)) {
                const auto& attrs = GetFieldRef(node, FIELD_ATTRS, FIELD_ATTRS_ALT);
                if (attrs.contains(ATTR_CONTENT) && attrs[ATTR_CONTENT].is_string()) {
                    std::string content = attrs[ATTR_CONTENT].get<std::string>();
                    contentMatched = CaseInsensitiveContains(content, contentMatcher);
                }
                if (!contentMatched && attrs.contains(ATTR_LABEL) && attrs[ATTR_LABEL].is_string()) {
                    std::string label = attrs[ATTR_LABEL].get<std::string>();
                    contentMatched = CaseInsensitiveContains(label, contentMatcher);
                }
            }
        }

        if (contentMatched && IsIntField(node, FIELD_ID, FIELD_ID_ALT)) {
            results.push_back(GetIntField(node, FIELD_ID, FIELD_ID_ALT));
        }
    }

    // Recurse into children
    if (IsArrayField(node, FIELD_CHILDREN, FIELD_CHILDREN_ALT)) {
        const auto& children = GetFieldRef(node, FIELD_CHILDREN, FIELD_CHILDREN_ALT);
        for (const auto& child : children) {
            TraverseNode(child, componentType, contentMatcher, results, totalCount);
        }
    }
}

void TraverseNodeWithAttrs(const nlohmann::json& node, const std::string& componentType,
    const std::string& compId, const std::string& contentMatcher,
    std::vector<MatchedNodeInfo>& results, int32_t& totalCount)
{
    if (!node.is_object()) {
        return;
    }
    totalCount++;

    // Extract raw fields for every node in DFS
    int32_t nodeId = GetIntField(node, FIELD_ID, FIELD_ID_ALT);
    std::string nodeType = GetStringField(node, FIELD_TYPE, FIELD_TYPE_ALT);
    std::string nodeCompId;
    if (node.contains(NODE_COMP_ID) && node[NODE_COMP_ID].is_string()) {
        nodeCompId = node[NODE_COMP_ID].get<std::string>();
    }
    TAG_LOGI(AAFwkTag::INTENT, "DFS[%{public}d]: type=%{public}s, ID=%{public}d, compid=%{public}s",
        totalCount, nodeType.c_str(), nodeId, nodeCompId.c_str());

    // Check type match (case-insensitive)
    bool typeMatched = !nodeType.empty() && CaseInsensitiveEqual(nodeType, componentType);
    if (!typeMatched) {
        // Recurse into children
        if (IsArrayField(node, FIELD_CHILDREN, FIELD_CHILDREN_ALT)) {
            const auto& children = GetFieldRef(node, FIELD_CHILDREN, FIELD_CHILDREN_ALT);
            for (const auto& child : children) {
                TraverseNodeWithAttrs(child, componentType, compId, contentMatcher, results, totalCount);
            }
        }
        return;
    }

    // Type matched -- extract full attributes
    MatchedNodeInfo info;
    info.compType = nodeType;
    info.nodeId = nodeId;
    ExtractNodeAttrs(node, info);

    TAG_LOGI(AAFwkTag::INTENT, "type matched: node[%{public}s] ID=%{public}d, "
        "compid=%{public}s, content=%{public}s, clickable=%{public}d, "
        "expect compId=%{public}s, matcher=%{public}s",
        nodeType.c_str(), info.nodeId, info.compId.c_str(), info.content.c_str(),
        info.clickable, compId.c_str(), contentMatcher.c_str());

    // Check compId match (if specified)
    bool compIdMatched = true;
    if (!compId.empty()) {
        compIdMatched = CaseInsensitiveEqual(info.compId, compId);
        if (!compIdMatched) {
            TAG_LOGI(AAFwkTag::INTENT, "-> rejected: compId mismatch");
        }
    }

    // Check content match
    bool contentMatched = true;
    if (!contentMatcher.empty()) {
        contentMatched = CaseInsensitiveContains(info.content, contentMatcher);
        if (!contentMatched) {
            contentMatched = CaseInsensitiveContains(info.label, contentMatcher);
        }
        if (!contentMatched) {
            TAG_LOGI(AAFwkTag::INTENT, "-> rejected: content mismatch");
        }
    }

    if (info.nodeId < 0) {
        TAG_LOGI(AAFwkTag::INTENT, "-> rejected: nodeId invalid");
    }

    if (compIdMatched && contentMatched && info.nodeId >= 0) {
        TAG_LOGI(AAFwkTag::INTENT, "-> matched");
        results.push_back(info);
    }

    // Recurse into children
    if (IsArrayField(node, FIELD_CHILDREN, FIELD_CHILDREN_ALT)) {
        const auto& children = GetFieldRef(node, FIELD_CHILDREN, FIELD_CHILDREN_ALT);
        for (const auto& child : children) {
            TraverseNodeWithAttrs(child, componentType, compId, contentMatcher, results, totalCount);
        }
    }
}
} // anonymous namespace

std::vector<int32_t> UiOperationTreeParser::FindMatchingNodes(const std::string& treeJson,
    const std::string& componentType, const std::string& contentMatcher)
{
    std::vector<int32_t> results;
    if (treeJson.empty() || componentType.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "treeJson or componentType is empty");
        return results;
    }

    try {
        auto json = nlohmann::json::parse(treeJson);
        int32_t totalCount = 0;
        TraverseNode(json, componentType, contentMatcher, results, totalCount);
        TAG_LOGI(AAFwkTag::INTENT, "FindMatchingNodes: type=%{public}s, matcher=%{public}s, "
            "found=%{public}zu, total=%{public}d",
            componentType.c_str(), contentMatcher.c_str(), results.size(), totalCount);
    } catch (const nlohmann::json::exception& e) {
        TAG_LOGE(AAFwkTag::INTENT, "JSON parse error: %{public}s", e.what());
    }

    return results;
}

std::vector<MatchedNodeInfo> UiOperationTreeParser::FindMatchingNodesWithAttributes(
    const std::string& treeJson, const std::string& componentType,
    const std::string& compId, const std::string& contentMatcher)
{
    std::vector<MatchedNodeInfo> results;
    if (treeJson.empty() || componentType.empty()) {
        TAG_LOGE(AAFwkTag::INTENT, "treeJson or componentType is empty");
        return results;
    }

    try {
        auto json = nlohmann::json::parse(treeJson);
        // Always print raw tree head for diagnosis
        TAG_LOGI(AAFwkTag::INTENT, "parsed tree head: %{public}s",
            treeJson.substr(0, MAX_TREE_DUMP_LEN).c_str());
        int32_t totalCount = 0;
        TraverseNodeWithAttrs(json, componentType, compId, contentMatcher, results, totalCount);
        TAG_LOGI(AAFwkTag::INTENT, "FindMatchingNodesWithAttributes: type=%{public}s, "
            "compId=%{public}s, matcher=%{public}s, found=%{public}zu, total=%{public}d",
            componentType.c_str(), compId.c_str(), contentMatcher.c_str(), results.size(), totalCount);
    } catch (const nlohmann::json::exception& e) {
        TAG_LOGE(AAFwkTag::INTENT, "JSON parse error: %{public}s", e.what());
    }

    return results;
}

int32_t UiOperationTreeParser::GetNodeCount(const std::string& treeJson)
{
    if (treeJson.empty()) {
        return 0;
    }
    try {
        auto json = nlohmann::json::parse(treeJson);
        int32_t totalCount = 0;
        std::vector<int32_t> unused;
        TraverseNode(json, "", "", unused, totalCount);
        return totalCount;
    } catch (const nlohmann::json::exception& e) {
        TAG_LOGE(AAFwkTag::INTENT, "JSON parse error: %{public}s", e.what());
        return 0;
    }
}

} // namespace AAFwk
} // namespace OHOS
