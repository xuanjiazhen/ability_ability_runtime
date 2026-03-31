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

#include "cli_tool_manager.h"
#include "cli_tool_data_manager.h"

namespace OHOS {
namespace AbilityRuntime {

namespace {
constexpr int32_t ERR_OK = 0;
constexpr int32_t ERR_NOT_INITIALIZED = -1;
constexpr int32_t ERR_NOT_FOUND = -2;
}

CliToolManager::CliToolManager()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliToolManager constructor called");

    // Load tools from default registry path in constructor
    int32_t ret = LoadToolsFromDefaultRegistry();
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to load tools from default registry: %{public}d", ret);
        // Don't fail construction, tools can be loaded later
        toolsLoaded_ = false;
    } else {
        toolsLoaded_ = true;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliToolManager constructed successfully");
}

CliToolManager::~CliToolManager()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliToolManager destructor called");
}

int32_t CliToolManager::LoadToolsFromRegistry(const std::string &filePath)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "LoadToolsFromRegistry: %{public}s", filePath.c_str());

    std::lock_guard<std::mutex> lock(mutex_);

    auto dataManager = DelayedSingleton<CliToolDataManager>::GetInstance();
    if (dataManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get CliToolDataManager instance");
        return -1;
    }

    int32_t ret = dataManager->LoadToolsFromFile(filePath);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to load tools from file: %{public}d", ret);
        return ret;
    }

    toolsLoaded_ = true;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Tools loaded successfully from: %{public}s", filePath.c_str());
    return ERR_OK;
}

int32_t CliToolManager::LoadToolsFromDefaultRegistry()
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "LoadToolsFromDefaultRegistry called");

    auto dataManager = DelayedSingleton<CliToolDataManager>::GetInstance();
    if (dataManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get CliToolDataManager instance");
        return -1;
    }

    return dataManager->LoadToolsFromFile(CliToolDataManager::DEFAULT_REGISTRY_PATH);
}

int32_t CliToolManager::QueryTools(std::vector<ToolInfo> &tools)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliToolManager QueryTools called");

    std::lock_guard<std::mutex> lock(mutex_);

    if (!toolsLoaded_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CliToolManager not initialized");
        return ERR_NOT_INITIALIZED;
    }

    auto dataManager = DelayedSingleton<CliToolDataManager>::GetInstance();
    if (dataManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get CliToolDataManager instance");
        return -1;
    }

    int32_t ret = dataManager->GetAllTools(tools);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get all tools: %{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "QueryTools returned %{public}zu tools", tools.size());
    return ERR_OK;
}

int32_t CliToolManager::QueryToolByName(const std::string &name, ToolInfo &tool)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliToolManager QueryToolByName: %{public}s", name.c_str());

    std::lock_guard<std::mutex> lock(mutex_);

    if (!toolsLoaded_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CliToolManager not initialized");
        return ERR_NOT_INITIALIZED;
    }

    auto dataManager = DelayedSingleton<CliToolDataManager>::GetInstance();
    if (dataManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get CliToolDataManager instance");
        return -1;
    }

    int32_t ret = dataManager->GetToolByName(name, tool);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Tool not found: %{public}s", name.c_str());
        return ERR_NOT_FOUND;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Found tool: %{public}s, version: %{public}s",
        tool.name.c_str(), tool.version.c_str());
    return ERR_OK;
}

int32_t CliToolManager::QueryToolsByPermission(const std::string &permission, std::vector<ToolInfo> &tools)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliToolManager QueryToolsByPermission: %{public}s", permission.c_str());

    std::lock_guard<std::mutex> lock(mutex_);

    if (!toolsLoaded_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CliToolManager not initialized");
        return ERR_NOT_INITIALIZED;
    }

    auto dataManager = DelayedSingleton<CliToolDataManager>::GetInstance();
    if (dataManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get CliToolDataManager instance");
        return -1;
    }

    int32_t ret = dataManager->GetToolsByPermission(permission, tools);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get tools by permission: %{public}d", ret);
        return ret;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "Found %{public}zu tools with permission: %{public}s",
        tools.size(), permission.c_str());
    return ERR_OK;
}

bool CliToolManager::ValidateTool(const std::string &name)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliToolManager ValidateTool: %{public}s", name.c_str());

    auto dataManager = DelayedSingleton<CliToolDataManager>::GetInstance();
    if (dataManager == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to get CliToolDataManager instance");
        return false;
    }

    return dataManager->ToolExists(name);
}

int32_t CliToolManager::GetToolTimeout(const std::string &name, int32_t &timeout)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CliToolManager GetToolTimeout: %{public}s", name.c_str());

    std::lock_guard<std::mutex> lock(mutex_);

    if (!toolsLoaded_) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "CliToolManager not initialized");
        return ERR_NOT_INITIALIZED;
    }

    ToolInfo tool;
    int32_t ret = QueryToolByName(name, tool);
    if (ret != ERR_OK) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "Tool not found: %{public}s", name.c_str());
        return ret;
    }

    timeout = tool.timeout;
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Tool timeout: %{public}s, timeout: %{public}d",
        name.c_str(), timeout);
    return ERR_OK;
}

bool CliToolManager::IsToolListLoaded()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return toolsLoaded_;
}

} // namespace AbilityRuntime
} // namespace OHOS
