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
    ~RegistryManager() = default;

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
     * @brief 加载工具描述文件
     * @param descriptorPath 描述文件路径
     * @return 错误码
     */
    int32_t LoadToolDescriptor(const std::string& descriptorPath);

private:
    /**
     * @brief 解析工具描述文件
     * @param filePath 文件路径
     * @param tool 输出参数，工具信息
     * @return 是否成功
     */
    bool ParseDescriptorFile(const std::string& filePath, ToolInfo& tool);

    /**
     * @brief 验证工具描述文件
     * @param tool 工具信息
     * @return 是否有效
     */
    bool ValidateToolInfo(const ToolInfo& tool);

private:
    // 工具注册表
    std::unordered_map<std::string, ToolInfo> tools_;
    std::mutex toolsMutex_;

    // 描述文件目录
    std::string descriptorDir_;

    static constexpr const char* TAG = "RegistryManager";
};

} // namespace CliMgr
} // namespace OHOS

#endif // REGISTRY_MANAGER_H
