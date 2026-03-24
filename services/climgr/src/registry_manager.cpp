/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "registry_manager.h"
#include "hilog_tag_wrapper.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cstring>

namespace OHOS {
namespace CliMgr {

namespace {
    // 默认工具描述文件目录
    constexpr const char* DEFAULT_DESCRIPTOR_DIR = "/etc/cli/tools";

    // 描述文件后缀
    constexpr const char* DESCRIPTOR_FILE_EXT = ".json";

    // Schema 版本
    constexpr const char* SUPPORTED_SCHEMA_VERSION = "https://schemas.openharmony.org/cli/tool/v1";
}

RegistryManager::RegistryManager()
    : descriptorDir_(DEFAULT_DESCRIPTOR_DIR)
{
}

bool RegistryManager::Initialize()
{
    TAG_LOGI(TAG, "Initializing RegistryManager with descriptor dir: %{public}s",
             descriptorDir_.c_str());

    // 检查目录是否存在
    struct stat st;
    if (stat(descriptorDir_.c_str(), &st) != 0) {
        TAG_LOGW(TAG, "Descriptor directory does not exist: %{public}s",
                 descriptorDir_.c_str());
        // 尝试创建目录
        if (mkdir(descriptorDir_.c_str(), 0755) != 0) {
            TAG_LOGE(TAG, "Failed to create descriptor directory: %{public}s",
                     descriptorDir_.c_str());
            return false;
        }
        TAG_LOGI(TAG, "Created descriptor directory: %{public}s",
                 descriptorDir_.c_str());
    }

    // 扫描并加载所有工具描述文件
    return LoadAllTools();
}

bool RegistryManager::LoadAllTools()
{
    DIR* dir = opendir(descriptorDir_.c_str());
    if (!dir) {
        TAG_LOGE(TAG, "Failed to open descriptor directory: %{public}s",
                 descriptorDir_.c_str());
        return false;
    }

    struct dirent* entry;
    int loadedCount = 0;
    int failedCount = 0;

    while ((entry = readdir(dir)) != nullptr) {
        // 跳过 . 和 ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // 检查是否为 .json 文件
        std::string filename = entry->d_name;
        if (filename.size() < strlen(DESCRIPTOR_FILE_EXT) ||
            filename.substr(filename.size() - strlen(DESCRIPTOR_FILE_EXT)) != DESCRIPTOR_FILE_EXT) {
            continue;
        }

        // 构建完整路径
        std::string fullPath = descriptorDir_ + "/" + filename;

        // 加载工具描述文件
        int32_t ret = LoadToolDescriptor(fullPath);
        if (ret == ErrCode::OK) {
            loadedCount++;
        } else {
            failedCount++;
            TAG_LOGW(TAG, "Failed to load tool descriptor: %{public}s", filename.c_str());
        }
    }

    closedir(dir);

    TAG_LOGI(TAG, "Tool loading completed: loaded=%{public}d, failed=%{public}d",
             loadedCount, failedCount);

    return (loadedCount > 0 || failedCount == 0);
}

int32_t RegistryManager::RegisterTool(const ToolInfo& tool)
{
    if (!ValidateToolInfo(tool)) {
        TAG_LOGE(TAG, "Invalid tool info for %{public}s", tool.name.c_str());
        return ErrCode::ERR_INVALID_SCHEMA;
    }

    std::lock_guard<std::mutex> lock(toolsMutex_);

    // 检查工具是否已存在
    if (tools_.find(tool.name) != tools_.end()) {
        TAG_LOGW(TAG, "Tool already exists: %{public}s", tool.name.c_str());
        return ErrCode::ERR_ALREADY_EXISTS;
    }

    // 注册工具
    tools_[tool.name] = tool;
    tools_[tool.name].registeredTime =
        std::chrono::system_clock::now().time_since_epoch().count();

    TAG_LOGI(TAG, "Tool registered: %{public}s (version=%{public}s)",
             tool.name.c_str(), tool.version.c_str());

    return ErrCode::OK;
}

int32_t RegistryManager::UnregisterTool(const std::string& name)
{
    std::lock_guard<std::mutex> lock(toolsMutex_);

    auto it = tools_.find(name);
    if (it == tools_.end()) {
        TAG_LOGW(TAG, "Tool not found: %{public}s", name.c_str());
        return ErrCode::ERR_NOT_FOUND;
    }

    tools_.erase(it);
    TAG_LOGI(TAG, "Tool unregistered: %{public}s", name.c_str());

    return ErrCode::OK;
}

int32_t RegistryManager::QueryTools(std::vector<ToolInfo>& tools)
{
    std::lock_guard<std::mutex> lock(toolsMutex_);

    tools.clear();
    tools.reserve(tools_.size());

    for (const auto& [name, tool] : tools_) {
        if (tool.enabled) {
            tools.push_back(tool);
        }
    }

    TAG_LOGI(TAG, "Queried %{public}zu tools (enabled only)", tools.size());
    return ErrCode::OK;
}

int32_t RegistryManager::QueryToolByName(const std::string& name, ToolInfo& tool)
{
    std::lock_guard<std::mutex> lock(toolsMutex_);

    auto it = tools_.find(name);
    if (it == tools_.end()) {
        TAG_LOGW(TAG, "Tool not found: %{public}s", name.c_str());
        return ErrCode::ERR_NOT_FOUND;
    }

    tool = it->second;
    return ErrCode::OK;
}

int32_t RegistryManager::QueryToolsByPermission(const std::string& permission,
                                               std::vector<ToolInfo>& tools)
{
    std::lock_guard<std::mutex> lock(toolsMutex_);

    tools.clear();

    for (const auto& [name, tool] : tools_) {
        if (!tool.enabled) {
            continue;
        }

        // 检查工具是否需要该权限
        bool hasPermission = false;
        for (const auto& perm : tool.permissions) {
            if (perm == permission) {
                hasPermission = true;
                break;
            }
        }

        if (hasPermission) {
            tools.push_back(tool);
        }
    }

    TAG_LOGI(TAG, "Queried %{public}zu tools with permission %{public}s",
             tools.size(), permission.c_str());

    return ErrCode::OK;
}

bool RegistryManager::ToolExists(const std::string& name)
{
    std::lock_guard<std::mutex> lock(toolsMutex_);
    return tools_.find(name) != tools_.end();
}

int32_t RegistryManager::LoadToolDescriptor(const std::string& descriptorPath)
{
    TAG_LOGI(TAG, "Loading tool descriptor: %{public}s", descriptorPath.c_str());

    ToolInfo tool;

    // 解析描述文件
    if (!ParseDescriptorFile(descriptorPath, tool)) {
        TAG_LOGE(TAG, "Failed to parse descriptor file: %{public}s",
                 descriptorPath.c_str());
        return ErrCode::ERR_INVALID_SCHEMA;
    }

    // 验证工具信息
    if (!ValidateToolInfo(tool)) {
        TAG_LOGE(TAG, "Invalid tool info in %{public}s", descriptorPath.c_str());
        return ErrCode::ERR_INVALID_SCHEMA;
    }

    // 检查可执行文件是否存在
    struct stat st;
    if (stat(tool.executablePath.c_str(), &st) != 0) {
        TAG_LOGW(TAG, "Executable not found: %{public}s",
                 tool.executablePath.c_str());
        // 仍然允许注册，但标记为禁用
        tool.enabled = false;
    }

    // 检查是否可执行
    if (tool.enabled && (st.st_mode & S_IXUSR) == 0) {
        TAG_LOGW(TAG, "Executable not executable: %{public}s",
                 tool.executablePath.c_str());
        tool.enabled = false;
    }

    // 注册工具
    return RegisterTool(tool);
}

bool RegistryManager::ParseDescriptorFile(const std::string& filePath, ToolInfo& tool)
{
    // 读取文件内容
    std::ifstream file(filePath);
    if (!file.is_open()) {
        TAG_LOGE(TAG, "Failed to open file: %{public}s", filePath.c_str());
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    // 解析 JSON
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;

    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(content.c_str(), content.c_str() + content.length(),
                       &root, &errs)) {
        TAG_LOGE(TAG, "JSON parse error: %{public}s", errs.c_str());
        return false;
    }

    // 验证 Schema 版本
    if (root.isMember("$schema")) {
        std::string schema = root["$schema"].asString();
        if (schema != SUPPORTED_SCHEMA_VERSION) {
            TAG_LOGW(TAG, "Unsupported schema version: %{public}s", schema.c_str());
        }
    }

    // 提取基本信息
    if (!root.isMember("name") || !root["name"].isString()) {
        TAG_LOGE(TAG, "Missing or invalid 'name' field");
        return false;
    }
    tool.name = root["name"].asString();

    if (root.isMember("version")) {
        tool.version = root["version"].asString();
    } else {
        tool.version = "1.0.0";
    }

    if (root.isMember("description")) {
        tool.description = root["description"].asString();
    }

    if (root.isMember("executable")) {
        tool.executablePath = root["executable"].asString();
    } else {
        TAG_LOGE(TAG, "Missing 'executable' field");
        return false;
    }

    // 解析工具类型
    if (root.isMember("type")) {
        std::string typeStr = root["type"].asString();
        if (typeStr == "short-lived") {
            tool.type = ToolType::SHORT_LIVED;
        } else if (typeStr == "long-lived") {
            tool.type = ToolType::LONG_LIVED;
        } else {
            TAG_LOGW(TAG, "Unknown tool type: %{public}s, defaulting to short-lived",
                     typeStr.c_str());
            tool.type = ToolType::SHORT_LIVED;
        }
    } else {
        tool.type = ToolType::SHORT_LIVED;
    }

    // 解析权限列表
    if (root.isMember("permissions") && root["permissions"].isArray()) {
        for (const auto& perm : root["permissions"]) {
            if (perm.isString()) {
                tool.permissions.push_back(perm.asString());
            }
        }
    }

    // 解析输入 Schema
    if (root.isMember("input")) {
        tool.inputSchema = root["input"];
    }

    // 解析输出 Schema
    if (root.isMember("output")) {
        tool.outputSchema = root["output"];
    }

    // 解析超时时间
    if (root.isMember("timeout")) {
        tool.timeoutMs = root["timeout"].asUInt();
    } else {
        tool.timeoutMs = 30000;  // 默认 30 秒
    }

    // 解析支持的事件类型
    if (root.isMember("events") && root["events"].isArray()) {
        for (const auto& event : root["events"]) {
            if (event.isString()) {
                tool.eventTypes.push_back(event.asString());
            }
        }
    }

    TAG_LOGI(TAG, "Parsed tool descriptor: %{public}s", tool.name.c_str());
    return true;
}

bool RegistryManager::ValidateToolInfo(const ToolInfo& tool)
{
    // 验证工具名称
    if (tool.name.empty() || tool.name.size() > 256) {
        TAG_LOGE(TAG, "Invalid tool name: empty or too long");
        return false;
    }

    // 验证可执行文件路径
    if (tool.executablePath.empty() || tool.executablePath.size() > 512) {
        TAG_LOGE(TAG, "Invalid executable path");
        return false;
    }

    // 验证超时时间（最大 5 分钟）
    if (tool.timeoutMs > 300000) {
        TAG_LOGW(TAG, "Timeout too large, clamping to 5 minutes");
        return false;
    }

    // 验证版本号格式
    if (tool.version.empty()) {
        TAG_LOGE(TAG, "Missing version");
        return false;
    }

    return true;
}

} // namespace CliMgr
} // namespace OHOS
