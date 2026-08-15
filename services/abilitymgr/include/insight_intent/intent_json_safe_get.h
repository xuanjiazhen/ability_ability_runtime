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

#ifndef OHOS_ABILITY_RUNTIME_INTENT_JSON_SAFE_GET_H
#define OHOS_ABILITY_RUNTIME_INTENT_JSON_SAFE_GET_H

#include <exception>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {

constexpr size_t DUMP_TOKEN_MAX_LEN = 50;   // single short token; longest ExecuteMode value is 26 chars
constexpr size_t DUMP_LOG_MAX_LEN = 1000;   // full JSON debug dump, bounded for hilog readability
// nlohmann::json::dump() recurses on nested objects/arrays; super-deep input triggers
// stack overflow (SIGSEGV) that try/catch cannot intercept. This bound keeps dump stack
// usage bounded. 100 is far above any legitimate intent payload depth.
constexpr size_t JSON_DUMP_MAX_DEPTH = 100;

template<typename T>
bool SafeJsonGet(const nlohmann::json &jsonObject, T &out, const char *tag)
{
    try {
        out = jsonObject.get<T>();
        return true;
    } catch (const nlohmann::json::exception &e) {
        TAG_LOGE(AAFwkTag::INTENT, "%{public}s json exception: %{public}s", tag, e.what());
        return false;
    } catch (const std::exception &e) {
        TAG_LOGE(AAFwkTag::INTENT, "%{public}s std exception: %{public}s", tag, e.what());
        return false;
    } catch (...) {
        TAG_LOGE(AAFwkTag::INTENT, "%{public}s unknown exception", tag);
        return false;
    }
}

// NOTE: the helpers below MUST stay inline in this header. Their consumers are compiled
// into libabilityms.so, the ability_manager innerkit (which cannot depend on abilityms),
// and numerous test/fuzz targets that compile the calling TUs directly. Out-of-line
// definitions would be hidden by the libabilityms version script (local: *) and left
// unresolved inside the innerkit, breaking both build and runtime linkage.

// Iterative depth walker. Uses an explicit stack so that checking an
// already-deep json object cannot itself stack-overflow.
inline bool IsJsonDepthOk(const nlohmann::json &jsonObject, size_t maxDepth)
{
    std::vector<std::pair<std::reference_wrapper<const nlohmann::json>, size_t>> stack;
    stack.emplace_back(std::cref(jsonObject), 1);
    while (!stack.empty()) {
        auto back = stack.back();
        stack.pop_back();
        const nlohmann::json &node = back.first.get();
        size_t depth = back.second;
        if (depth > maxDepth) {
            return false;
        }
        if (node.is_object()) {
            for (auto it = node.begin(); it != node.end(); ++it) {
                stack.emplace_back(std::cref(it.value()), depth + 1);
            }
        } else if (node.is_array()) {
            for (const auto &item : node) {
                stack.emplace_back(std::cref(item), depth + 1);
            }
        }
    }
    return true;
}

inline std::string SafeDump(const nlohmann::json &jsonObject, size_t maxLen = 0)
{
    if (!IsJsonDepthOk(jsonObject, JSON_DUMP_MAX_DEPTH)) {
        TAG_LOGE(AAFwkTag::INTENT, "json depth exceeds limit %{public}zu", JSON_DUMP_MAX_DEPTH);
        return "";
    }
    try {
        auto s = jsonObject.dump();
        return (maxLen > 0 && s.size() > maxLen) ? s.substr(0, maxLen) + "..." : s;
    } catch (...) {
        return "";
    }
}

inline bool SafeDumpTo(const nlohmann::json &jsonObject, std::string &out)
{
    if (!IsJsonDepthOk(jsonObject, JSON_DUMP_MAX_DEPTH)) {
        TAG_LOGE(AAFwkTag::INTENT, "json depth exceeds limit %{public}zu", JSON_DUMP_MAX_DEPTH);
        return false;
    }
    try {
        out = jsonObject.dump();
    } catch (...) {
        return false;
    }
    return true;
}

} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_INTENT_JSON_SAFE_GET_H
