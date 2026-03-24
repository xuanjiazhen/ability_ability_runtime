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

#ifndef REGISTRY_MANAGER_H
#define REGISTRY_MANAGER_H

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <mutex>
#include "cli_tool_info.h"

namespace OHOS {
namespace CliMgr {

/**
 * @brief 工具注册管理器
 *
 * 负责工具的注册、查询和管理
 */
class RegistryManager {
public:
    RegistryManager();
    ~RegistryManager()
    {
        StopRegistryWatcher();
    }

    /**
     * @brief 初始化
     * @return 是否成功
     */
    bool Initialize();

    /**
     * @brief 注册工具
     * @param tool 工具信息
     * @return 错误码
     */
    int32_t RegisterTool(const ToolInfo& tool);

    /**
     * @brief 注销工具
     * @param name 工具名称
     * @return 错误码
     */
    int32_t UnregisterTool(const std::string& name);

    /**
     * @brief 查询所有工具
     * @param tools 输出参数，工具列表
     * @return 错误码
     */
    int32_t QueryTools(std::vector<ToolInfo>& tools);

    /**
     * @brief 根据名称查询工具
     * @param name 工具名称
     * @param tool 输出参数，工具信息
     * @return 错误码
     */
    int32_t QueryToolByName(const std::string& name, ToolInfo& tool);

    /**
     * @brief 根据权限过滤工具
     * @param permission 权限名称
     * @param tools 输出参数，工具列表
     * @return 错误码
     */
    int32_t QueryToolsByPermission(const std::string& permission,
                                   std::vector<ToolInfo>& tools);

    /**
     * @brief 检查工具是否存在
     * @param name 工具名称
     * @return 是否存在
     */
    bool ToolExists(const std::string& name);

    /**
     * @brief 重新加载注册表
     * @return 错误码
     */
    int32_t ReloadRegistry();

    /**
     * @brief 获取注册表状态
     */
    struct RegistryStatus {
        std::string registryPath;
        uint64_t lastModified;
        size_t toolCount;
        bool isValid;
        std::string errorMessage;
    };
    int32_t GetRegistryStatus(RegistryStatus& status);

    /**
     * @brief 启动文件监控
     */
    void StartRegistryWatcher();

    /**
     * @brief 停止文件监控
     */
    void StopRegistryWatcher();

private:
    /**
     * @brief 加载注册表文件
     * @param registryPath 注册表文件路径
     * @return 错误码
     */
    int32_t LoadRegistry(const std::string& registryPath);

    /**
     * @brief 创建空注册表
     * @param registryPath 注册表文件路径
     * @return 是否成功
     */
    bool CreateEmptyRegistry(const std::string& registryPath);

    /**
     * @brief 解析注册表文件
     * @param filePath 文件路径
     * @param tools 输出参数，工具列表
     * @return 是否成功
     */
    bool ParseRegistryFile(const std::string& filePath, std::vector<ToolInfo>& tools);

    /**
     * @brief 从 JSON 解析单个工具
     * @param toolJson 工具 JSON 对象
     * @param tool 输出参数，工具信息
     * @return 是否成功
     */
    bool ParseToolFromJson(const Json::Value& toolJson, ToolInfo& tool);

    /**
     * @brief 验证工具条目
     * @param tool 工具信息
     * @return 是否有效
     */
    bool ValidateToolEntry(const ToolInfo& tool);

    /**
     * @brief 验证工具名称
     * @param name 工具名称
     * @return 是否有效
     */
    bool ValidateToolName(const std::string& name);

    /**
     * @brief 验证所有工具
     * @return 是否全部有效
     */
    bool ValidateAllTools();

    /**
     * @brief 获取当前 ISO 格式时间
     * @return ISO 格式时间字符串
     */
    static std::string GetCurrentTimeISO();

    /**
     * @brief 将工具信息写入 JSON 对象
     * @param tool 工具信息
     * @return JSON 对象
     */
    Json::Value WriteToolToJson(const ToolInfo& tool);

    /**
     * @brief 持久化注册表到文件
     * @return 错误码
     */
    int32_t PersistRegistry();

    /**
     * @brief 从文件删除工具
     * @param name 工具名称
     * @return 错误码
     */
    int32_t RemoveToolFromFile(const std::string& name);

    /**
     * @brief 添加工具到文件
     * @param tool 工具信息
     * @return 错误码
     */
    int32_t AddToolToFile(const ToolInfo& tool);

    /**
     * @brief 更新文件中的工具
     * @param tool 工具信息
     * @return 错误码
     */
    int32_t UpdateToolInFile(const ToolInfo& tool);

private:
    // 工具注册表
    std::unordered_map<std::string, ToolInfo> tools_;
    std::mutex toolsMutex_;

    // 注册表路径
    std::string registryPath_ = "/etc/cli/cli_tools.json";

    // 注册表最后修改时间
    uint64_t registryLastModified_ = 0;

    // 文件监控相关
    class RegistryWatcher;
    std::unique_ptr<RegistryWatcher> watcher_;
    std::mutex watcherMutex_;

    static constexpr const char* TAG = "RegistryManager";
};

} // namespace CliMgr
} // namespace OHOS

#endif // REGISTRY_MANAGER_H
