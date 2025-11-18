/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "json_utils.h"
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <regex>

#include "config_policy_utils.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
bool JsonUtils::LoadConfiguration(const std::string& path, nlohmann::json& jsonBuf,
    const std::string& defaultPath)
{
    std::string configPath = GetConfigPath(path, defaultPath);
    TAG_LOGD(AAFwkTag::ABILITYMGR, "config path is: %{public}s", configPath.c_str());
    if (!ReadFileInfoJson(configPath, jsonBuf)) {
        return false;
    }
    return true;
}

std::string JsonUtils::GetConfigPath(const std::string& path, const std::string& defaultPath)
{
    if (!IsValidConfigFileName(path)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid config path: %{public}s", path.c_str());
        return defaultPath;
    }

    char buf[MAX_PATH_LEN] = { 0 };
    char *configPath = GetOneCfgFile(path.c_str(), buf, MAX_PATH_LEN);
    if (configPath == nullptr || configPath[0] == '\0' || strlen(configPath) > MAX_PATH_LEN) {
        return defaultPath;
    }
    return configPath;
}

bool JsonUtils::IsValidConfigFileName(const std::string& fileName)
{
    const std::string dangerousChars = "../\\:*?\"<>|";
    if (fileName.find_first_of(dangerousChars) != std::string::npos) {
        return false;
    }

    std::vector<std::string> traversalPatterns = {
        "../", "..\\", "%2e%2e", "%2E%2E", "%2e%2e%2f", "%2E%2E%2F",
        "..;/", "..;\\", "%c0%af", "%c1%9c", "%c0%ae%c0%ae", "%c1%9c%c1%9c",
        "..%2f", "..%2F", "..%5c", "..%5C", "0x2e0x2e", "0x2E0x2E",
        "%uff0e", "%uFF0E", "%ef%bc%8e", "%EF%BC%8E"
    };

    for (const auto& pattern : traversalPatterns) {
        if (fileName.find(pattern) != std::string::npos) {
            return false;
        }
    }

    std::string decodedFileName = DecodeUnicodeEscapes(fileName);
    if (decodedFileName != fileName) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Unicode encoding detected, validating decoded string");
        return IsValidConfigFileName(decodedFileName);
    }

    return true;
}

bool JsonUtils::ReadFileInfoJson(const std::string &filePath, nlohmann::json &jsonBuf)
{
    if (access(filePath.c_str(), F_OK) != 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "deepLink config not exist");
        return false;
    }

    if (filePath.empty()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "filePath empty");
        return false;
    }

    char path[PATH_MAX] = {0};
    if (realpath(filePath.c_str(), path) == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "realpath error, errno: %{public}d", errno);
        return false;
    }

    std::fstream in;
    char errBuf[256];
    errBuf[0] = '\0';
    in.open(path, std::ios_base::in);
    if (!in.is_open()) {
        strerror_r(errno, errBuf, sizeof(errBuf));
        TAG_LOGE(AAFwkTag::ABILITYMGR, "file not open: %{public}s", errBuf);
        return false;
    }

    in.seekg(0, std::ios::end);
    int64_t size = in.tellg();
    if (size <= 0) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "empty file");
        in.close();
        return false;
    }

    in.seekg(0, std::ios::beg);
    jsonBuf = nlohmann::json::parse(in, nullptr, false);
    in.close();
    if (jsonBuf.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "bad profile file");
        return false;
    }

    return true;
}

bool JsonUtils::IsEqual(nlohmann::json &jsonObject, const std::string &key, const std::string &value, bool checkEmpty)
{
    if (jsonObject.contains(key) && jsonObject[key].is_string()) {
        std::string  jsonValue = jsonObject.at(key).get<std::string>();
        if (checkEmpty && !jsonValue.empty() && jsonValue != value) {
            return false;
        } else if (value != jsonValue) {
            return false;
        }
    }
    return true;
}

bool JsonUtils::IsEqual(nlohmann::json &jsonObject, const std::string &key, int32_t value)
{
    if (jsonObject.contains(key) && jsonObject[key].is_number()) {
        if (value != jsonObject.at(key).get<int32_t>()) {
            return false;
        }
    }
    return true;
}

std::optional<bool> JsonUtils::JsonToOptionalBool(const nlohmann::json &jsonObject, const std::string &key)
{
    if (jsonObject.contains(key) && jsonObject[key].is_boolean()) {
        return jsonObject[key].get<bool>();
    }
    return std::nullopt;
}

bool JsonUtils::JsonToBool(const nlohmann::json &jsonObject, const std::string &key, bool defaultValue)
{
    if (jsonObject.contains(key) && jsonObject[key].is_boolean()) {
        return jsonObject[key].get<bool>();
    }
    return defaultValue;
}

void JsonUtils::JsonToUnorderedStrSet(const nlohmann::json &jsonObject, const std::string &key,
    std::unordered_set<std::string> &set)
{
    if (!jsonObject.contains(key) || !jsonObject.at(key).is_array()) {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "%{public}s config null", key.c_str());
        return;
    }
    set.clear();
    const auto &jsonArray = jsonObject.at(key);
    for (auto &item : jsonArray.items()) {
        const nlohmann::json &itemJson = item.value();
        if (!itemJson.is_string()) {
            continue;
        }
        auto str = itemJson.get<std::string>();
        set.emplace(str);
    }
}

std::string JsonUtils::DecodeUnicodeEscapes(const std::string& input)
{
    std::string result = input;

    // 解析%uXXXX格式的Unicode转义
    std::regex unicodeRegex("%u([0-9a-fA-F]{4})");
    std::smatch match;

    while (std::regex_search(result, match, unicodeRegex)) {
        std::string hexStr = match[1].str();
        char32_t unicodeChar = std::stoul(hexStr, nullptr, 16);

        // 转换为UTF-8
        std::string utf8Char;
        if (unicodeChar <= 0x7F) {
            utf8Char = static_cast<char>(unicodeChar);
        } else if (unicodeChar <= 0x7FF) {
            utf8Char += static_cast<char>(0xC0 | (unicodeChar >> 6));
            utf8Char += static_cast<char>(0x80 | (unicodeChar & 0x3F));
        } else if (unicodeChar <= 0xFFFF) {
            utf8Char += static_cast<char>(0xE0 | (unicodeChar >> 12));
            utf8Char += static_cast<char>(0x80 | ((unicodeChar >> 6) & 0x3F));
            utf8Char += static_cast<char>(0x80 | (unicodeChar & 0x3F));
        }

        result.replace(match.position(), match.length(), utf8Char);
    }

    return result;
}
}  // namespace AAFwk
}  // namespace OHOS