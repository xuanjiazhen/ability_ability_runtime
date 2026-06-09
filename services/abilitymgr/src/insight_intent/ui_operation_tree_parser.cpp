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
// ACE Inspector tree JSON field names
constexpr const char* FIELD_ID = "$ID";
constexpr const char* FIELD_TYPE = "$type";
constexpr const char* FIELD_CHILDREN = "$children";
constexpr const char* FIELD_ATTRS = "$attrs";
constexpr const char* ATTR_CONTENT = "content";
constexpr const char* ATTR_LABEL = "label";

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

void TraverseNode(const nlohmann::json& node, const std::string& componentType,
    const std::string& contentMatcher, std::vector<int32_t>& results, int32_t& totalCount)
{
    if (!node.is_object()) {
        return;
    }
    totalCount++;

    // Check $type match (case-insensitive)
    bool typeMatched = false;
    if (node.contains(FIELD_TYPE) && node[FIELD_TYPE].is_string()) {
        std::string nodeType = node[FIELD_TYPE].get<std::string>();
        typeMatched = CaseInsensitiveEqual(nodeType, componentType);
    }

    // Check content match if type matched
    if (typeMatched) {
        bool contentMatched = true;
        if (!contentMatcher.empty()) {
            contentMatched = false;
            // Check $attrs.content and $attrs.label
            if (node.contains(FIELD_ATTRS) && node[FIELD_ATTRS].is_object()) {
                const auto& attrs = node[FIELD_ATTRS];
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

        if (contentMatched && node.contains(FIELD_ID) && node[FIELD_ID].is_number_integer()) {
            results.push_back(node[FIELD_ID].get<int32_t>());
        }
    }

    // Recurse into children
    if (node.contains(FIELD_CHILDREN) && node[FIELD_CHILDREN].is_array()) {
        for (const auto& child : node[FIELD_CHILDREN]) {
            TraverseNode(child, componentType, contentMatcher, results, totalCount);
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
