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

#include "json_safe_util.h"

#include <functional>
#include <utility>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {
namespace {
class JsonDepthLimitSax : public nlohmann::json_sax<nlohmann::json> {
public:
    explicit JsonDepthLimitSax(size_t maxDepth) : maxDepth_(maxDepth) {}
    bool null() override { return true; }
    bool boolean(bool) override { return true; }
    bool number_integer(nlohmann::json::number_integer_t) override { return true; }
    bool number_unsigned(nlohmann::json::number_unsigned_t) override { return true; }
    bool number_float(nlohmann::json::number_float_t, const nlohmann::json::string_t&) override { return true; }
    bool string(nlohmann::json::string_t&) override { return true; }
    bool binary(nlohmann::json::binary_t&) override { return true; }
    bool start_object(size_t) override { return ++depth_ <= maxDepth_; }
    bool end_object() override
    {
        if (depth_ > 0) {
            --depth_;
        }
        return true;
    }
    bool start_array(size_t) override { return ++depth_ <= maxDepth_; }
    bool end_array() override
    {
        if (depth_ > 0) {
            --depth_;
        }
        return true;
    }
    bool key(nlohmann::json::string_t&) override { return true; }
    bool parse_error(size_t, const std::string&, const nlohmann::detail::exception&) override { return false; }

private:
    size_t depth_ = 0;
    size_t maxDepth_ = 0;
};

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
}  // namespace

bool SafeParse(const std::string &jsonStr, nlohmann::json &jsonObject)
{
    if (jsonStr.size() > JSON_SAFE_PARSE_MAX_LEN) {
        HiLogError(JSON_SAFE_UTIL_LABEL, "json size exceeds limit: %{public}zu", jsonStr.size());
        return false;
    }
    JsonDepthLimitSax handler(JSON_SAFE_MAX_DEPTH);
    if (!nlohmann::json::sax_parse(jsonStr, &handler)) {
        HiLogError(JSON_SAFE_UTIL_LABEL, "json depth exceeds limit or parse error");
        return false;
    }
    jsonObject = nlohmann::json::parse(jsonStr, nullptr, false);
    if (jsonObject.is_discarded()) {
        HiLogError(JSON_SAFE_UTIL_LABEL, "json parse discarded");
        return false;
    }
    return true;
}

bool SafeDump(const nlohmann::json &jsonObject, std::string &out, size_t maxLen)
{
    if (!IsJsonDepthOk(jsonObject, JSON_SAFE_MAX_DEPTH)) {
        HiLogError(JSON_SAFE_UTIL_LABEL, "json depth exceeds limit %{public}zu", JSON_SAFE_MAX_DEPTH);
        return false;
    }
    std::string dumped = jsonObject.dump();
    out = (maxLen > 0 && dumped.size() > maxLen) ? dumped.substr(0, maxLen) + "..." : dumped;
    return true;
}

std::string SafeDump(const nlohmann::json &jsonObject, size_t maxLen)
{
    std::string out;
    if (!SafeDump(jsonObject, out, maxLen)) {
        return "";
    }
    return out;
}
}  // namespace AbilityRuntime
}  // namespace OHOS