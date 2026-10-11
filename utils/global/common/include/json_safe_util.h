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

#ifndef OHOS_ABILITY_RUNTIME_JSON_SAFE_UTIL_H
#define OHOS_ABILITY_RUNTIME_JSON_SAFE_UTIL_H

#include <cstddef>
#include <exception>
#include <string>

#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {
constexpr size_t JSON_SAFE_PARSE_MAX_LEN = 128 * 1024 * 1024;
constexpr size_t JSON_SAFE_MAX_DEPTH = 100;
constexpr size_t DUMP_TOKEN_MAX_LEN = 50;
constexpr size_t DUMP_LOG_MAX_LEN = 1000;

enum class JsonType {
    NULLABLE,
    BOOLEAN,
    NUMBER,
    OBJECT,
    ARRAY,
    STRING,
};

enum class ArrayType {
    NUMBER,
    OBJECT,
    STRING,
    NOT_ARRAY,
};

bool SafeParse(const std::string &jsonStr, nlohmann::json &jsonObject);
bool SafeDump(const nlohmann::json &jsonObject, std::string &out, size_t maxLen = 0);
std::string SafeDump(const nlohmann::json &jsonObject, size_t maxLen = 0);

void GetStrValueIfFindKey(const nlohmann::json &jsonObject,
    const nlohmann::json::const_iterator &end,
    const std::string &key, std::string &value,
    bool isNecessary, int32_t &parseResult);

template<typename T, typename dataType>
void CheckArrayType(const nlohmann::json &jsonObject, const std::string &key,
    dataType &data, ArrayType arrayType, int32_t &parseResult)
{
    auto arrays = jsonObject.at(key);
    if (arrays.empty()) {
        return;
    }
    for (const auto &array : arrays) {
        switch (arrayType) {
            case ArrayType::STRING:
                if (!array.is_string()) {
                    TAG_LOGE(AAFwkTag::DEFAULT, "array %{public}s not string", key.c_str());
                    parseResult = ERR_INVALID_VALUE;
                }
                break;
            case ArrayType::OBJECT:
                if (!array.is_object()) {
                    TAG_LOGE(AAFwkTag::DEFAULT, "array %{public}s not object", key.c_str());
                    parseResult = ERR_INVALID_VALUE;
                }
                break;
            case ArrayType::NUMBER:
                if (!array.is_number()) {
                    TAG_LOGE(AAFwkTag::DEFAULT, "array %{public}s not number", key.c_str());
                    parseResult = ERR_INVALID_VALUE;
                }
                break;
            case ArrayType::NOT_ARRAY:
            default:
                break;
        }
        if (parseResult != ERR_OK) {
            break;
        }
    }
    if (parseResult == ERR_OK) {
        data = jsonObject.at(key).get<T>();
    }
}

template<typename T, typename dataType>
void GetValueIfFindKey(const nlohmann::json &jsonObject,
    const nlohmann::json::const_iterator &end,
    const std::string &key, dataType &data, JsonType jsonType,
    bool isNecessary, int32_t &parseResult, ArrayType arrayType)
{
    if (parseResult != ERR_OK) {
        return;
    }
    if (jsonObject.find(key) != end) {
        switch (jsonType) {
            case JsonType::BOOLEAN:
                if (!jsonObject.at(key).is_boolean()) {
                    TAG_LOGE(AAFwkTag::DEFAULT, "type:%{public}s not bool", key.c_str());
                    parseResult = ERR_INVALID_VALUE;
                    break;
                }
                data = jsonObject.at(key).get<T>();
                break;
            case JsonType::NUMBER:
                if (!jsonObject.at(key).is_number()) {
                    TAG_LOGE(AAFwkTag::DEFAULT, "type:%{public}s not number", key.c_str());
                    parseResult = ERR_INVALID_VALUE;
                    break;
                }
                data = jsonObject.at(key).get<T>();
                break;
            case JsonType::OBJECT:
                if (!jsonObject.at(key).is_object()) {
                    TAG_LOGE(AAFwkTag::DEFAULT, "type:%{public}s not object", key.c_str());
                    parseResult = ERR_INVALID_VALUE;
                    break;
                }
                data = jsonObject.at(key).get<T>();
                break;
            case JsonType::ARRAY:
                if (!jsonObject.at(key).is_array()) {
                    TAG_LOGE(AAFwkTag::DEFAULT, "type:%{public}s not array", key.c_str());
                    parseResult = ERR_INVALID_VALUE;
                    break;
                }
                CheckArrayType<T>(jsonObject, key, data, arrayType, parseResult);
                break;
            case JsonType::STRING:
                if (!jsonObject.at(key).is_string()) {
                    TAG_LOGE(AAFwkTag::DEFAULT, "type:%{public}s not string", key.c_str());
                    parseResult = ERR_INVALID_VALUE;
                    break;
                }
                data = jsonObject.at(key).get<T>();
                break;
            case JsonType::NULLABLE:
                break;
            default:
                TAG_LOGE(AAFwkTag::DEFAULT, "type:%{public}s not jsonType", key.c_str());
                parseResult = ERR_INVALID_VALUE;
        }
        return;
    }
    if (isNecessary) {
        TAG_LOGE(AAFwkTag::DEFAULT, "profile prop %{public}s is missing", key.c_str());
        parseResult = ERR_INVALID_VALUE;
    }
}

// Header-only template using try/catch: any translation unit that includes
// this header must be built with exceptions enabled (use_exceptions = true).
template<typename T>
bool SafeJsonGet(const nlohmann::json &jsonObject, T &out, const char *tag)
{
    try {
        out = jsonObject.get<T>();
        return true;
    } catch (const nlohmann::json::exception &e) {
        TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s json exception: %{public}s", tag, e.what());
        return false;
    } catch (const std::exception &e) {
        TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s std exception: %{public}s", tag, e.what());
        return false;
    } catch (...) {
        TAG_LOGE(AAFwkTag::DEFAULT, "%{public}s unknown exception", tag);
        return false;
    }
}
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_JSON_SAFE_UTIL_H
