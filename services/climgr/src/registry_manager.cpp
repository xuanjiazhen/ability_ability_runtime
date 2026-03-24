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
#include "cli_manager_error.h"
#include "hilog/log.h"
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <unistd.h>
#include <regex>
#include <fcntl.h>
#include <chrono>
#include <iomanip>
#include <ctime>
#include <thread>
#include <atomic>
#include <cerrno>
#include <cstring>
#include <sys/inotify.h>

namespace OHOS {
namespace CliMgr {

namespace {
    // Schema 版本
    constexpr const char* SUPPORTED_SCHEMA_VERSION = "https://schemas.openharmony.org/cli/tools-registry/v1";

    // 默认注册表路径
    constexpr const char* DEFAULT_REGISTRY_PATH = "/etc/cli/cli_tools.json";

    // 工具目录
    constexpr const char* TOOLS_DIR = "/etc/cli/tools";

    // 日志标签
    constexpr OHOS::HiviewDFX::HiLogLabel TAG = {LOG_CORE, 0, "RegistryManager"};
}

RegistryManager::RegistryManager()
{
}

bool RegistryManager::Initialize()
{
    HILOG_INFO(TAG, "Initializing RegistryManager");

    // 1. 检查注册表文件是否存在
    struct stat st;
    if (stat(registryPath_.c_str(), &st) != 0) {
        HILOG_WARN(TAG, "Registry file not found: %{public}s", registryPath_.c_str());
        // 创建空注册表
        return CreateEmptyRegistry(registryPath_);
    }

    // 2. 加载注册表
    int32_t ret = LoadRegistry(registryPath_);
    if (ret != ErrCode::OK) {
        HILOG_ERROR(TAG, "Failed to load registry: %{public}d", ret);
        return false;
    }

    // 3. 启动文件监控
    StartRegistryWatcher();

    HILOG_INFO(TAG, "RegistryManager initialized successfully");
    return true;
}

int32_t RegistryManager::ReloadRegistry()
{
    HILOG_INFO(TAG, "Reloading registry");

    return LoadRegistry(registryPath_);
}

int32_t RegistryManager::LoadRegistry(const std::string& registryPath)
{
    // 1. 读取文件内容
    std::ifstream file(registryPath);
    if (!file.is_open()) {
        HILOG_ERROR(TAG, "Failed to open registry file: %{public}s", registryPath.c_str());
        return ErrCode::ERR_INVALID_SCHEMA;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    // 2. 解析 JSON
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errs;

    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());
    if (!reader->parse(content.c_str(), content.c_str() + content.length(),
                       &root, &errs)) {
        HILOG_ERROR(TAG, "JSON parse error: %{public}s", errs.c_str());
        return ErrCode::ERR_INVALID_SCHEMA;
    }

    // 3. 验证版本
    std::string version = root.get("version", "").asString();
    if (version.empty()) {
        HILOG_ERROR(TAG, "Missing version field");
        return ErrCode::ERR_INVALID_SCHEMA;
    }

    HILOG_INFO(TAG, "Registry version: %{public}s", version.c_str());

    // 4. 解析工具数组
    if (!root.isMember("tools") || !root["tools"].isArray()) {
        HILOG_ERROR(TAG, "Missing or invalid 'tools' array");
        return ErrCode::ERR_INVALID_SCHEMA;
    }

    const Json::Value& toolsArray = root["tools"];
    std::vector<ToolInfo> validTools;

    for (const auto& toolJson : toolsArray) {
        ToolInfo tool;
        if (ParseToolFromJson(toolJson, tool)) {
            if (ValidateToolEntry(tool)) {
                validTools.push_back(tool);
                HILOG_INFO(TAG, "Loaded tool: %{public}s", tool.name.c_str());
            } else {
                HILOG_WARN(TAG, "Tool validation failed: %{public}s",
                         tool.name.c_str());
            }
        }
    }

    // 5. 更新内存注册表
    {
        std::lock_guard<std::mutex> lock(toolsMutex_);
        tools_.clear();
        for (const auto& tool : validTools) {
            tools_[tool.name] = tool;
        }
    }

    // 6. 更新最后修改时间
    struct stat st;
    if (stat(registryPath.c_str(), &st) == 0) {
        registryLastModified_ = st.st_mtime;
    }

    HILOG_INFO(TAG, "Registry loaded: %{public}zu tools", validTools.size());
    return ErrCode::OK;
}

bool RegistryManager::CreateEmptyRegistry(const std::string& registryPath)
{
    HILOG_INFO(TAG, "Creating empty registry: %{public}s", registryPath.c_str());

    // 创建目录
    std::string dir = registryPath.substr(0, registryPath.find_last_of('/'));
    if (mkdir(dir.c_str(), 0755) != 0 && errno != EEXIST) {
        HILOG_ERROR(TAG, "Failed to create directory: %{public}s", dir.c_str());
        return false;
    }

    // 创建空注册表
    Json::Value root;
    root["$schema"] = SUPPORTED_SCHEMA_VERSION;
    root["version"] = "1.0.0";
    root["lastUpdated"] = GetCurrentTimeISO();
    root["tools"] = Json::Value(Json::arrayValue);

    // 写入文件
    std::ofstream outFile(registryPath);
    if (!outFile.is_open()) {
        HILOG_ERROR(TAG, "Failed to create registry file");
        return false;
    }

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";
    std::string jsonStr = Json::writeString(builder, root);
    outFile << jsonStr;
    outFile.close();

    HILOG_INFO(TAG, "Empty registry created successfully");
    return true;
}

bool RegistryManager::ParseToolFromJson(const Json::Value& toolJson, ToolInfo& tool)
{
    // 提取基本信息
    if (!toolJson.isMember("name") || !toolJson["name"].isString()) {
        HILOG_ERROR(TAG, "Missing or invalid 'name' field");
        return false;
    }
    tool.name = toolJson["name"].asString();

    if (!ValidateToolName(tool.name)) {
        HILOG_ERROR(TAG, "Invalid tool name: %{public}s", tool.name.c_str());
        return false;
    }

    if (toolJson.isMember("version")) {
        tool.version = toolJson["version"].asString();
    } else {
        tool.version = "1.0.0";
    }

    if (toolJson.isMember("description")) {
        tool.description = toolJson["description"].asString();
    }

    if (toolJson.isMember("executable")) {
        tool.executablePath = toolJson["executable"].asString();
    } else {
        HILOG_ERROR(TAG, "Missing 'executable' field");
        return false;
    }

    // 解析权限列表
    if (toolJson.isMember("permissions") && toolJson["permissions"].isArray()) {
        for (const auto& perm : toolJson["permissions"]) {
            if (perm.isString()) {
                tool.permissions.push_back(perm.asString());
            }
        }
    }

    // 解析输入 Schema
    if (toolJson.isMember("input")) {
        tool.inputSchema = toolJson["input"];
    }

    // 解析输出 Schema
    if (toolJson.isMember("output")) {
        tool.outputSchema = toolJson["output"];
    }

    // 解析参数映射规则
    if (toolJson.isMember("argumentMapping")) {
        tool.argMapping = toolJson["argumentMapping"];
    }

    // 解析超时时间
    if (toolJson.isMember("timeout")) {
        tool.timeoutMs = toolJson["timeout"].asUInt();
    } else {
        tool.timeoutMs = 30000;  // 默认 30 秒
    }

    // 解析支持的事件类型
    if (toolJson.isMember("events") && toolJson["events"].isArray()) {
        for (const auto& event : toolJson["events"]) {
            if (event.isString()) {
                tool.eventTypes.push_back(event.asString());
            }
        }
    }

    // 解析子命令
    if (toolJson.isMember("hasSubcommands") && toolJson["hasSubcommands"].asBool()) {
        tool.hasSubcommands = true;

        if (toolJson.isMember("subcommands") && toolJson["subcommands"].isObject()) {
            const Json::Value& subcmds = toolJson["subcommands"];
            for (const auto& name : subcmds.getMemberNames()) {
                const Json::Value& subcmdJson = subcmds[name];
                SubCommandInfo subcmd;

                if (subcmdJson.isMember("description")) {
                    subcmd.description = subcmdJson["description"].asString();
                }

                if (subcmdJson.isMember("input")) {
                    subcmd.inputSchema = subcmdJson["input"];
                }

                if (subcmdJson.isMember("argumentMapping")) {
                    subcmd.argMapping = subcmdJson["argumentMapping"];
                }

                tool.subcommands[name] = subcmd;
                HILOG_INFO(TAG, "  Loaded subcommand: %{public}s", name.c_str());
            }
        }
    }

    return true;
}

bool RegistryManager::ValidateToolEntry(const ToolInfo& tool)
{
    // 1. 验证可执行文件路径
    if (tool.executablePath.find("/etc/cli/tools/") != 0) {
        HILOG_ERROR(TAG, "Executable must be in /etc/cli/tools/: %{public}s",
                 tool.executablePath.c_str());
        return false;
    }

    // 2. 验证文件存在
    struct stat st;
    if (stat(tool.executablePath.c_str(), &st) != 0) {
        HILOG_WARN(TAG, "Executable not found: %{public}s",
                 tool.executablePath.c_str());
        return false;
    }

    // 3. 验证是常规文件
    if (!S_ISREG(st.st_mode)) {
        HILOG_ERROR(TAG, "Not a regular file: %{public}s",
                 tool.executablePath.c_str());
        return false;
    }

    // 4. 验证不允许 setuid/setgid
    if ((st.st_mode & S_ISUID) || (st.st_mode & S_ISGID)) {
        HILOG_ERROR(TAG, "Setuid/setgid not allowed: %{public}s",
                 tool.executablePath.c_str());
        return false;
    }

    // 5. 验证所有者必须是 root
    if (st.st_uid != 0) {
        HILOG_ERROR(TAG, "Tool must be owned by root: %{public}s (uid=%{public}d)",
                 tool.executablePath.c_str(), st.st_uid);
        return false;
    }

    // 6. 验证权限（755 或更严格）
    if ((st.st_mode & 0777) > 0755) {
        HILOG_WARN(TAG, "Tool permissions too loose: %{public}s (mode=%{public}o)",
                 tool.executablePath.c_str(), st.st_mode & 0777);
    }

    // 7. 验证可执行
    if ((st.st_mode & S_IXUSR) == 0) {
        HILOG_ERROR(TAG, "Tool not executable: %{public}s",
                 tool.executablePath.c_str());
        return false;
    }

    return true;
}

bool RegistryManager::ValidateToolName(const std::string& name)
{
    // 1. 长度限制
    if (name.empty() || name.size() > 64) {
        return false;
    }

    // 2. 字符限制：只允许小写字母、数字、连字符
    std::regex pattern("^[a-z0-9-]+$");
    if (!std::regex_match(name, pattern)) {
        return false;
    }

    // 3. 不能以连字符开头或结尾
    if (name[0] == '-' || name[name.size()-1] == '-') {
        return false;
    }

    // 4. 不能有连续的连字符
    if (name.find("--") != std::string::npos) {
        return false;
    }

    return true;
}

bool RegistryManager::ValidateAllTools()
{
    std::lock_guard<std::mutex> lock(toolsMutex_);

    size_t validCount = 0;
    for (const auto& [name, tool] : tools_) {
        if (ValidateToolEntry(tool)) {
            validCount++;
        } else {
            HILOG_WARN(TAG, "Tool validation failed: %{public}s", name.c_str());
        }
    }

    HILOG_INFO(TAG, "Validation complete: %{public}zu/%{public}zu tools valid",
             validCount, tools_.size());

    return true;
}

int32_t RegistryManager::RegisterTool(const ToolInfo& tool)
{
    if (!ValidateToolName(tool.name)) {
        HILOG_ERROR(TAG, "Invalid tool name: %{public}s", tool.name.c_str());
        return ErrCode::ERR_INVALID_ARGS;
    }

    std::lock_guard<std::mutex> lock(toolsMutex_);

    // 检查工具是否已存在
    if (tools_.find(tool.name) != tools_.end()) {
        HILOG_WARN(TAG, "Tool already exists: %{public}s", tool.name.c_str());
        return ErrCode::ERR_ALREADY_EXISTS;
    }

    // 注册工具
    tools_[tool.name] = tool;
    tools_[tool.name].registeredTime =
        std::chrono::system_clock::now().time_since_epoch().count();

    HILOG_INFO(TAG, "Tool registered: %{public}s (version=%{public}s)",
             tool.name.c_str(), tool.version.c_str());

    return ErrCode::OK;
}

int32_t RegistryManager::UnregisterTool(const std::string& name)
{
    std::lock_guard<std::mutex> lock(toolsMutex_);

    auto it = tools_.find(name);
    if (it == tools_.end()) {
        HILOG_WARN(TAG, "Tool not found: %{public}s", name.c_str());
        return ErrCode::ERR_NOT_FOUND;
    }

    tools_.erase(it);
    HILOG_INFO(TAG, "Tool unregistered: %{public}s", name.c_str());

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

    HILOG_INFO(TAG, "Queried %{public}zu tools (enabled only)", tools.size());
    return ErrCode::OK;
}

int32_t RegistryManager::QueryToolByName(const std::string& name, ToolInfo& tool)
{
    std::lock_guard<std::mutex> lock(toolsMutex_);

    auto it = tools_.find(name);
    if (it == tools_.end()) {
        HILOG_WARN(TAG, "Tool not found: %{public}s", name.c_str());
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

    HILOG_INFO(TAG, "Queried %{public}zu tools with permission %{public}s",
             tools.size(), permission.c_str());

    return ErrCode::OK;
}

bool RegistryManager::ToolExists(const std::string& name)
{
    std::lock_guard<std::mutex> lock(toolsMutex_);
    return tools_.find(name) != tools_.end();
}

int32_t RegistryManager::GetRegistryStatus(RegistryStatus& status)
{
    status.registryPath = registryPath_;

    struct stat st;
    if (stat(registryPath_.c_str(), &st) == 0) {
        status.lastModified = st.st_mtime;
    } else {
        status.lastModified = 0;
    }

    std::lock_guard<std::mutex> lock(toolsMutex_);
    status.toolCount = tools_.size();
    status.isValid = true;
    status.errorMessage = "";

    return ErrCode::OK;
}

std::string RegistryManager::GetCurrentTimeISO()
{
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);

    std::stringstream ss;
    ss << std::put_time(std::gmtime(&time_t_now), "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

void RegistryManager::StartRegistryWatcher()
{
    std::lock_guard<std::mutex> lock(watcherMutex_);

    if (watcher_) {
        HILOG_WARN(TAG, "Registry watcher already started");
        return;
    }

    watcher_ = std::make_unique<RegistryWatcher>(this);
    if (!watcher_->Start(registryPath_)) {
        HILOG_WARN(TAG, "Failed to start registry watcher");
        watcher_.reset();
    } else {
        HILOG_INFO(TAG, "Registry watcher started successfully");
    }
}

void RegistryManager::StopRegistryWatcher()
{
    std::lock_guard<std::mutex> lock(watcherMutex_);

    if (watcher_) {
        watcher_->Stop();
        watcher_.reset();
    }
}

// ============================================================================
// RegistryWatcher 实现
// ============================================================================

/**
 * @brief 文件监控器类
 *
 * 使用 inotify 监控注册表文件变化，当文件修改时自动重新加载
 */
class RegistryManager::RegistryWatcher {
public:
    explicit RegistryWatcher(RegistryManager* manager)
        : manager_(manager),
          inotifyFd_(-1),
          watchFd_(-1),
          running_(false)
    {
        // 初始化日志标签
        tag_ = {LOG_CORE, 0, "RegistryWatcher"};
    }

    ~RegistryWatcher()
    {
        Stop();
    }

    /**
     * @brief 启动文件监控
     * @param registryPath 注册表文件路径
     * @return 是否成功
     */
    bool Start(const std::string& registryPath)
    {
        // 1. 初始化 inotify
        inotifyFd_ = inotify_init1(IN_NONBLOCK);
        if (inotifyFd_ < 0) {
            HILOG_ERROR(tag_, "Failed to init inotify: %{public}s", strerror(errno));
            return false;
        }

        // 2. 添加监控
        watchFd_ = inotify_add_watch(inotifyFd_, registryPath.c_str(),
                                      IN_MODIFY | IN_MOVED_TO | IN_DELETE_SELF);
        if (watchFd_ < 0) {
            HILOG_ERROR(tag_, "Failed to add watch: %{public}s", strerror(errno));
            close(inotifyFd_);
            inotifyFd_ = -1;
            return false;
        }

        // 3. 启动监控线程
        running_ = true;
        watchThread_ = std::thread(&RegistryWatcher::WatchThread, this);

        HILOG_INFO(tag_, "Registry watcher started for %{public}s", registryPath.c_str());
        return true;
    }

    /**
     * @brief 停止文件监控
     */
    void Stop()
    {
        if (!running_) {
            return;
        }

        running_ = false;

        // 唤醒 read 调用（通过写入一个事件）
        if (inotifyFd_ >= 0) {
            // 关闭 watch
            if (watchFd_ >= 0) {
                inotify_rm_watch(inotifyFd_, watchFd_);
                watchFd_ = -1;
            }
            close(inotifyFd_);
            inotifyFd_ = -1;
        }

        // 等待线程结束
        if (watchThread_.joinable()) {
            watchThread_.join();
        }

        HILOG_INFO(tag_, "Registry watcher stopped");
    }

private:
    /**
     * @brief 监控线程主循环
     */
    void WatchThread()
    {
        char buffer[4096] __attribute__((aligned(__alignof__(struct inotify_event))));

        HILOG_INFO(tag_, "Watch thread started");

        while (running_) {
            // 读取事件
            int length = read(inotifyFd_, buffer, sizeof(buffer));
            if (length < 0) {
                if (errno == EINTR || errno == EAGAIN) {
                    // 非阻塞返回，继续等待
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
                HILOG_ERROR(tag_, "Read error: %{public}s", strerror(errno));
                break;
            }

            // 处理事件
            int i = 0;
            while (i < length) {
                struct inotify_event* event =
                    (struct inotify_event*)&buffer[i];

                if (event->mask & (IN_MODIFY | IN_MOVED_TO)) {
                    HILOG_INFO(tag_, "Registry file modified, reloading...");
                    OnRegistryChanged();
                } else if (event->mask & IN_DELETE_SELF) {
                    HILOG_WARN(tag_, "Registry file deleted");
                    // 文件被删除，停止监控
                    running_ = false;
                    break;
                }

                i += sizeof(struct inotify_event) + event->len;
            }
        }

        HILOG_INFO(tag_, "Watch thread exited");
    }

    /**
     * @brief 注册表文件变化处理
     */
    void OnRegistryChanged()
    {
        // 延迟一小段时间，确保文件写入完成
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // 重新加载注册表
        if (manager_) {
            int32_t ret = manager_->ReloadRegistry();
            if (ret != ErrCode::OK) {
                HILOG_ERROR(tag_, "Failed to reload registry: %{public}d", ret);
            }
        }
    }

    RegistryManager* manager_;           // 注册表管理器指针
    int inotifyFd_;                      // inotify 文件描述符
    int watchFd_;                        // 监控描述符
    std::thread watchThread_;            // 监控线程
    std::atomic<bool> running_;          // 运行标志
    OHOS::HiviewDFX::HiLogLabel tag_;    // 日志标签
};

// ============================================================================
// 持久化方法实现
// ============================================================================

Json::Value RegistryManager::WriteToolToJson(const ToolInfo& tool)
{
    Json::Value toolJson;

    toolJson["name"] = tool.name;
    toolJson["version"] = tool.version;
    toolJson["description"] = tool.description;
    toolJson["executable"] = tool.executablePath;

    // 写入权限列表
    Json::Value permsArray(Json::arrayValue);
    for (const auto& perm : tool.permissions) {
        permsArray.append(perm);
    }
    toolJson["permissions"] = permsArray;

    // 写入 Schema
    if (!tool.inputSchema.isNull()) {
        toolJson["input"] = tool.inputSchema;
    }
    if (!tool.outputSchema.isNull()) {
        toolJson["output"] = tool.outputSchema;
    }

    // 写入参数映射
    if (!tool.argMapping.isNull()) {
        toolJson["argumentMapping"] = tool.argMapping;
    }

    // 写入超时时间
    toolJson["timeout"] = tool.timeoutMs;

    // 写入事件类型
    if (!tool.eventTypes.empty()) {
        Json::Value eventsArray(Json::arrayValue);
        for (const auto& event : tool.eventTypes) {
            eventsArray.append(event);
        }
        toolJson["events"] = eventsArray;
    }

    // 写入子命令
    if (tool.hasSubcommands && !tool.subcommands.empty()) {
        toolJson["hasSubcommands"] = true;
        Json::Value subcmdsJson;
        for (const auto& [name, subcmd] : tool.subcommands) {
            Json::Value subcmdJson;
            if (!subcmd.description.empty()) {
                subcmdJson["description"] = subcmd.description;
            }
            if (!subcmd.inputSchema.isNull()) {
                subcmdJson["input"] = subcmd.inputSchema;
            }
            if (!subcmd.argMapping.isNull()) {
                subcmdJson["argumentMapping"] = subcmd.argMapping;
            }
            subcmdsJson[name] = subcmdJson;
        }
        toolJson["subcommands"] = subcmdsJson;
    }

    return toolJson;
}

int32_t RegistryManager::PersistRegistry()
{
    HILOG_INFO(TAG, "Persisting registry to file");

    // 1. 读取现有注册表（如果存在）
    Json::Value root;
    struct stat st;
    if (stat(registryPath_.c_str(), &st) == 0) {
        // 文件存在，读取现有内容
        std::ifstream file(registryPath_);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();

            Json::CharReaderBuilder builder;
            std::string errs;
            std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

            if (reader->parse(buffer.str().c_str(),
                           buffer.str().c_str() + buffer.str().length(),
                           &root, &errs)) {
                HILOG_INFO(TAG, "Loaded existing registry for update");
            } else {
                HILOG_WARN(TAG, "Failed to parse existing registry, creating new");
                root.clear();
            }
        }
    } else {
        HILOG_INFO(TAG, "Registry file does not exist, creating new");
    }

    // 2. 更新基本信息
    if (!root.isMember("$schema")) {
        root["$schema"] = SUPPORTED_SCHEMA_VERSION;
    }
    root["version"] = "1.0.0";
    root["lastUpdated"] = GetCurrentTimeISO();

    // 3. 更新工具列表
    Json::Value toolsArray(Json::arrayValue);
    {
        std::lock_guard<std::mutex> lock(toolsMutex_);
        for (const auto& [name, tool] : tools_) {
            if (tool.enabled) {
                toolsArray.append(WriteToolToJson(tool));
            }
        }
    }
    root["tools"] = toolsArray;

    // 4. 写入文件
    std::ofstream outFile(registryPath_);
    if (!outFile.is_open()) {
        HILOG_ERROR(TAG, "Failed to open registry file for writing: %{public}s",
                   registryPath_.c_str());
        return ErrCode::ERR_IO_ERROR;
    }

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";
    builder["commentStyle"] = "None";
    std::string jsonStr = Json::writeString(builder, root);
    outFile << jsonStr;
    outFile.close();

    // 5. 更新文件修改时间
    if (stat(registryPath_.c_str(), &st) == 0) {
        registryLastModified_ = st.st_mtime;
    }

    HILOG_INFO(TAG, "Registry persisted successfully");
    return ErrCode::OK;
}

int32_t RegistryManager::AddToolToFile(const ToolInfo& tool)
{
    HILOG_INFO(TAG, "Adding tool to file: %{public}s", tool.name.c_str());

    // 1. 读取现有注册表
    Json::Value root;
    struct stat st;
    if (stat(registryPath_.c_str(), &st) == 0) {
        std::ifstream file(registryPath_);
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();

            Json::CharReaderBuilder builder;
            std::string errs;
            std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

            if (!reader->parse(buffer.str().c_str(),
                           buffer.str().c_str() + buffer.str().length(),
                           &root, &errs)) {
                HILOG_ERROR(TAG, "Failed to parse existing registry");
                return ErrCode::ERR_INVALID_SCHEMA;
            }
        }
    } else {
        // 创建新注册表
        root["$schema"] = SUPPORTED_SCHEMA_VERSION;
        root["version"] = "1.0.0";
    }

    // 2. 更新时间戳
    root["lastUpdated"] = GetCurrentTimeISO();

    // 3. 检查工具是否已存在
    if (!root.isMember("tools") || !root["tools"].isArray()) {
        root["tools"] = Json::Value(Json::arrayValue);
    }

    Json::Value& toolsArray = root["tools"];
    for (const auto& toolJson : toolsArray) {
        if (toolJson.isMember("name") && toolJson["name"].asString() == tool.name) {
            HILOG_WARN(TAG, "Tool already exists in file: %{public}s", tool.name.c_str());
            return ErrCode::ERR_ALREADY_EXISTS;
        }
    }

    // 4. 添加新工具
    toolsArray.append(WriteToolToJson(tool));

    // 5. 写入文件
    std::ofstream outFile(registryPath_);
    if (!outFile.is_open()) {
        HILOG_ERROR(TAG, "Failed to open registry file for writing");
        return ErrCode::ERR_IO_ERROR;
    }

    Json::StreamWriterBuilder builder;
    builder["indentation"] = "  ";
    std::string jsonStr = Json::writeString(builder, root);
    outFile << jsonStr;
    outFile.close();

    HILOG_INFO(TAG, "Tool added to file successfully");
    return ErrCode::OK;
}

int32_t RegistryManager::UpdateToolInFile(const ToolInfo& tool)
{
    HILOG_INFO(TAG, "Updating tool in file: %{public}s", tool.name.c_str());

    // 1. 读取现有注册表
    Json::Value root;
    struct stat st;
    if (stat(registryPath_.c_str(), &st) != 0) {
        HILOG_ERROR(TAG, "Registry file does not exist");
        return ErrCode::ERR_NOT_FOUND;
    }

    std::ifstream file(registryPath_);
    if (!file.is_open()) {
        HILOG_ERROR(TAG, "Failed to open registry file");
        return ErrCode::ERR_IO_ERROR;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    // 2. 解析 JSON
    Json::CharReaderBuilder builder;
    std::string errs;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

    if (!reader->parse(buffer.str().c_str(),
                       buffer.str().c_str() + buffer.str().length(),
                       &root, &errs)) {
        HILOG_ERROR(TAG, "Failed to parse registry file");
        return ErrCode::ERR_INVALID_SCHEMA;
    }

    // 3. 查找并更新工具
    if (!root.isMember("tools") || !root["tools"].isArray()) {
        HILOG_ERROR(TAG, "Invalid registry format");
        return ErrCode::ERR_INVALID_SCHEMA;
    }

    Json::Value& toolsArray = root["tools"];
    bool found = false;
    for (Json::ArrayIndex i = 0; i < toolsArray.size(); ++i) {
        if (toolsArray[i].isMember("name") &&
            toolsArray[i]["name"].asString() == tool.name) {
            // 更新工具信息（保留 name 和 executable）
            std::string name = toolsArray[i]["name"].asString();
            std::string executable = toolsArray[i]["executable"].asString();

            toolsArray[i] = WriteToolToJson(tool);
            toolsArray[i]["name"] = name;
            toolsArray[i]["executable"] = executable;
            found = true;
            break;
        }
    }

    if (!found) {
        HILOG_WARN(TAG, "Tool not found in file: %{public}s", tool.name.c_str());
        return ErrCode::ERR_NOT_FOUND;
    }

    // 4. 更新时间戳
    root["lastUpdated"] = GetCurrentTimeISO();

    // 5. 写入文件
    std::ofstream outFile(registryPath_);
    if (!outFile.is_open()) {
        HILOG_ERROR(TAG, "Failed to open registry file for writing");
        return ErrCode::ERR_IO_ERROR;
    }

    Json::StreamWriterBuilder writeBuilder;
    writeBuilder["indentation"] = "  ";
    std::string jsonStr = Json::writeString(writeBuilder, root);
    outFile << jsonStr;
    outFile.close();

    HILOG_INFO(TAG, "Tool updated in file successfully");
    return ErrCode::OK;
}

int32_t RegistryManager::RemoveToolFromFile(const std::string& name)
{
    HILOG_INFO(TAG, "Removing tool from file: %{public}s", name.c_str());

    // 1. 读取现有注册表
    Json::Value root;
    struct stat st;
    if (stat(registryPath_.c_str(), &st) != 0) {
        HILOG_WARN(TAG, "Registry file does not exist, nothing to remove");
        return ErrCode::OK;  // 文件不存在视为成功
    }

    std::ifstream file(registryPath_);
    if (!file.is_open()) {
        HILOG_ERROR(TAG, "Failed to open registry file");
        return ErrCode::ERR_IO_ERROR;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    // 2. 解析 JSON
    Json::CharReaderBuilder builder;
    std::string errs;
    std::unique_ptr<Json::CharReader> reader(builder.newCharReader());

    if (!reader->parse(buffer.str().c_str(),
                       buffer.str().c_str() + buffer.str().length(),
                       &root, &errs)) {
        HILOG_ERROR(TAG, "Failed to parse registry file");
        return ErrCode::ERR_INVALID_SCHEMA;
    }

    // 3. 查找并删除工具
    if (!root.isMember("tools") || !root["tools"].isArray()) {
        HILOG_WARN(TAG, "No tools in registry file");
        return ErrCode::OK;
    }

    Json::Value& toolsArray = root["tools"];
    Json::Value newToolsArray(Json::arrayValue);
    bool found = false;

    for (const auto& toolJson : toolsArray) {
        if (toolJson.isMember("name") && toolJson["name"].asString() == name) {
            found = true;
            continue;  // 跳过要删除的工具
        }
        newToolsArray.append(toolJson);
    }

    if (!found) {
        HILOG_WARN(TAG, "Tool not found in file: %{public}s", name.c_str());
        return ErrCode::ERR_NOT_FOUND;
    }

    // 4. 更新工具列表
    root["tools"] = newToolsArray;
    root["lastUpdated"] = GetCurrentTimeISO();

    // 5. 写入文件
    std::ofstream outFile(registryPath_);
    if (!outFile.is_open()) {
        HILOG_ERROR(TAG, "Failed to open registry file for writing");
        return ErrCode::ERR_IO_ERROR;
    }

    Json::StreamWriterBuilder writeBuilder;
    writeBuilder["indentation"] = "  ";
    std::string jsonStr = Json::writeString(writeBuilder, root);
    outFile << jsonStr;
    outFile.close();

    HILOG_INFO(TAG, "Tool removed from file successfully");
    return ErrCode::OK;
}

} // namespace CliMgr
} // namespace OHOS
