/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "cli_tool_data_manager.h"
#include "hilog_tag_wrapper.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <unistd.h>

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_FILE_NOT_FOUND = -1;
constexpr int32_t ERR_JSON_PARSE_FAILED = -2;
constexpr int32_t ERR_KVSTORE_NOT_READY = -3;
constexpr int32_t CHECK_INTERVAL = 100000; // 100ms
constexpr int32_t MAX_TIMES = 5;           // 5 * 100ms = 500ms
}

CliToolDataManager::CliToolDataManager()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliToolDataManager constructor called");
}

CliToolDataManager::~CliToolDataManager()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliToolDataManager destructor called");
    if (kvStorePtr_ != nullptr) {
        dataManager_.CloseKvStore(appId_, kvStorePtr_);
    }
}

DistributedKv::Status CliToolDataManager::GetKvStore()
{
    DistributedKv::Options options = { .createIfMissing = true,
        .encrypt = false,
        .autoSync = false,
        .syncable = false,
        .securityLevel = DistributedKv::SecurityLevel::S2,
        .area = DistributedKv::EL1,
        .kvStoreType = DistributedKv::KvStoreType::SINGLE_VERSION,
        .baseDir = CLI_TOOLS_STORAGE_DIR };

    DistributedKv::Status status = dataManager_.GetSingleKvStore(options, appId_, storeId_, kvStorePtr_);

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get KVStore: %{public}d", static_cast<int>(status));
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "KVStore initialized successfully");
    }

    return status;
}

bool CliToolDataManager::CheckKvStore()
{
    if (kvStorePtr_ != nullptr) {
        return true;
    }

    int32_t tryTimes = MAX_TIMES;
    while (tryTimes > 0) {
        DistributedKv::Status status = GetKvStore();
        if (status == DistributedKv::Status::SUCCESS && kvStorePtr_ != nullptr) {
            return true;
        }
        TAG_LOGW(AAFwkTag::ABILITYMGR, "CheckKvStore failed, try times: %{public}d", tryTimes);
        usleep(CHECK_INTERVAL);
        tryTimes--;
    }

    return kvStorePtr_ != nullptr;
}

int32_t CliToolDataManager::LoadToolsFromFile(const std::string &filePath)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "LoadToolsFromFile called: %{public}s", filePath.c_str());

    // Parse JSON file
    std::vector<ToolInfo> tools;
    int32_t ret = ParseJsonFile(filePath, tools);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to parse JSON file: %{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Parsed %{public}zu tools from file", tools.size());

    // Ensure KVStore is ready
    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "KVStore not ready");
        return ERR_KVSTORE_NOT_READY;
    }

    // Clear existing tools and store new ones
    DistributedKv::Key toolsListKey(TOOLS_LIST_KEY);
    kvStorePtr_->Delete(toolsListKey);

    // Store all tools
    for (const auto &tool : tools) {
        ret = StoreTool(tool);
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "Failed to store tool: %{public}s", tool.name.c_str());
        }
    }

    // Store tools list as JSON array
    std::string toolsJson = ToolsToJsonArray(tools);
    DistributedKv::Value toolsJsonValue(toolsJson);
    DistributedKv::Status status = kvStorePtr_->Put(toolsListKey, toolsJsonValue);

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to store tools list: %{public}d", static_cast<int>(status));
        return -1;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Successfully loaded and stored %{public}zu tools", tools.size());
    return ERR_OK;
}

int32_t CliToolDataManager::GetAllTools(std::vector<ToolInfo> &tools)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetAllTools called");

    if (!CheckKvStore()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "KVStore not ready");
        return ERR_KVSTORE_NOT_READY;
    }

    DistributedKv::Key toolsListKey(TOOLS_LIST_KEY);
    DistributedKv::Value value;
    DistributedKv::Status status = kvStorePtr_->Get(toolsListKey, value);

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get tools list: %{public}d", static_cast<int>(status));
        return -1;
    }

    std::string jsonStr(value.ToString());
    int32_t ret = JsonArrayToTools(jsonStr, tools);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to parse tools JSON: %{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Retrieved %{public}zu tools", tools.size());
    return ERR_OK;
}

int32_t CliToolDataManager::GetToolByName(const std::string &name, ToolInfo &tool)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetToolByName called: %{public}s", name.c_str());

    std::vector<ToolInfo> tools;
    int32_t ret = GetAllTools(tools);
    if (ret != ERR_OK) {
        return ret;
    }

    auto it = std::find_if(tools.begin(), tools.end(),
        [&name](const ToolInfo &t) { return t.name == name; });

    if (it != tools.end()) {
        tool = *it;
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Found tool: %{public}s", name.c_str());
        return ERR_OK;
    }

    TAG_LOGW(AAFwkTag::ABILITYMGR, "Tool not found: %{public}s", name.c_str());
    return -1;
}

int32_t CliToolDataManager::GetToolsByPermission(const std::string &permission, std::vector<ToolInfo> &tools)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "GetToolsByPermission called: %{public}s", permission.c_str());

    std::vector<ToolInfo> allTools;
    int32_t ret = GetAllTools(allTools);
    if (ret != ERR_OK) {
        return ret;
    }

    tools.clear();
    for (const auto &tool : allTools) {
        auto it = std::find(tool.permissions.begin(), tool.permissions.end(), permission);
        if (it != tool.permissions.end()) {
            tools.push_back(tool);
        }
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Found %{public}zu tools with permission: %{public}s",
        tools.size(), permission.c_str());
    return ERR_OK;
}

bool CliToolDataManager::ToolExists(const std::string &name)
{
    ToolInfo tool;
    return GetToolByName(name, tool) == ERR_OK;
}

int32_t CliToolDataManager::ParseJsonFile(const std::string &filePath, std::vector<ToolInfo> &tools)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "ParseJsonFile: %{public}s", filePath.c_str());

    std::ifstream file(filePath);
    if (!file.is_open()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to open file: %{public}s", filePath.c_str());
        return ERR_FILE_NOT_FOUND;
    }

    // Parse JSON without exceptions
    nlohmann::json root = nlohmann::json::parse(file, nullptr, false);
    if (root.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "JSON parse failed: invalid JSON format");
        return ERR_JSON_PARSE_FAILED;
    }

    if (!root.is_array()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "JSON root is not an array");
        return ERR_JSON_PARSE_FAILED;
    }

    for (const auto &item : root) {
        ToolInfo tool;

        if (item.contains("name") && item["name"].is_string()) {
            tool.name = item["name"];
        }
        if (item.contains("version") && item["version"].is_string()) {
            tool.version = item["version"];
        }
        if (item.contains("description") && item["description"].is_string()) {
            tool.description = item["description"];
        }
        if (item.contains("permissions") && item["permissions"].is_array()) {
            for (const auto &perm : item["permissions"]) {
                if (perm.is_string()) {
                    tool.permissions.push_back(perm);
                }
            }
        }
        if (item.contains("timeout") && item["timeout"].is_number()) {
            tool.timeout = item["timeout"];
        }
        if (item.contains("supportedEvents") && item["supportedEvents"].is_array()) {
            for (const auto &event : item["supportedEvents"]) {
                if (event.is_string()) {
                    tool.supportedEvents.push_back(event);
                }
            }
        }

        tools.push_back(tool);
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Parsed tool: %{public}s", tool.name.c_str());
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Successfully parsed %{public}zu tools", tools.size());
    return ERR_OK;
}

std::string CliToolDataManager::ToolInfoToJson(const ToolInfo &tool)
{
    nlohmann::json j;
    j["name"] = tool.name;
    j["version"] = tool.version;
    j["description"] = tool.description;
    j["permissions"] = tool.permissions;
    j["timeout"] = tool.timeout;
    j["supportedEvents"] = tool.supportedEvents;
    return j.dump();
}

int32_t CliToolDataManager::JsonToToolInfo(const std::string &jsonStr, ToolInfo &tool)
{
    // Parse JSON without exceptions
    nlohmann::json j = nlohmann::json::parse(jsonStr, nullptr, false);
    if (j.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "JSON parse failed: invalid JSON format");
        return ERR_JSON_PARSE_FAILED;
    }

    if (j.contains("name") && j["name"].is_string()) {
        tool.name = j["name"];
    }
    if (j.contains("version") && j["version"].is_string()) {
        tool.version = j["version"];
    }
    if (j.contains("description") && j["description"].is_string()) {
        tool.description = j["description"];
    }
    if (j.contains("permissions") && j["permissions"].is_array()) {
        tool.permissions = j["permissions"];
    }
    if (j.contains("timeout") && j["timeout"].is_number()) {
        tool.timeout = j["timeout"];
    }
    if (j.contains("supportedEvents") && j["supportedEvents"].is_array()) {
        tool.supportedEvents = j["supportedEvents"];
    }

    return ERR_OK;
}

std::string CliToolDataManager::ToolsToJsonArray(const std::vector<ToolInfo> &tools)
{
    nlohmann::json j = nlohmann::json::array();
    for (const auto &tool : tools) {
        nlohmann::json item;
        item["name"] = tool.name;
        item["version"] = tool.version;
        item["description"] = tool.description;
        item["permissions"] = tool.permissions;
        item["timeout"] = tool.timeout;
        item["supportedEvents"] = tool.supportedEvents;
        j.push_back(item);
    }
    return j.dump();
}

int32_t CliToolDataManager::JsonArrayToTools(const std::string &jsonStr, std::vector<ToolInfo> &tools)
{
    // Parse JSON without exceptions
    nlohmann::json j = nlohmann::json::parse(jsonStr, nullptr, false);
    if (j.is_discarded()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "JSON parse failed: invalid JSON format");
        return ERR_JSON_PARSE_FAILED;
    }

    if (!j.is_array()) {
        return ERR_JSON_PARSE_FAILED;
    }

    tools.clear();
    for (const auto &item : j) {
        ToolInfo tool;
        int32_t ret = JsonToToolInfo(item.dump(), tool);
        if (ret != ERR_OK) {
            TAG_LOGW(AAFwkTag::ABILITYMGR, "Failed to parse tool item");
            continue;
        }
        tools.push_back(tool);
    }

    return ERR_OK;
}

int32_t CliToolDataManager::StoreTool(const ToolInfo &tool)
{
    DistributedKv::Key key(tool.name);
    DistributedKv::Value value(ToolInfoToJson(tool));
    DistributedKv::Status status = kvStorePtr_->Put(key, value);

    if (status != DistributedKv::Status::SUCCESS) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to store tool: %{public}s, status: %{public}d",
            tool.name.c_str(), static_cast<int>(status));
        return -1;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Stored tool: %{public}s", tool.name.c_str());
    return ERR_OK;
}

} // namespace AbilityRuntime
} // namespace OHOS
