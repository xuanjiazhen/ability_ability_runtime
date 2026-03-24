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

#ifndef CLI_TOOL_REGISTRY_H
#define CLI_TOOL_REGISTRY_H

#include <string>
#include <vector>
#include <memory>
#include "cli_tool_info.h"

namespace OHOS {
namespace CliMgr {

/**
 * @brief CLI 工具注册客户端
 *
 * 提供给系统服务和底层组件使用，用于动态注册和管理 CLI 工具。
 * 通过 IPC 与 CLI Manager Service 通信。
 *
 * 使用场景：
 * 1. 系统服务启动时注册自己提供的 CLI 工具
 * 2. 第三方服务动态注册工具
 * 3. 工具的版本更新和注销
 *
 * @note 此接口仅供 Native C++ 代码使用，不提供 JS 绑定
 */
class CliToolRegistry {
public:
    /**
     * @brief 获取 CliToolRegistry 单例
     * @return CliToolRegistry 实例的指针
     */
    static CliToolRegistry* GetInstance();

    /**
     * @brief 初始化注册客户端
     *
     * 连接到 CLI Manager Service，准备进行工具注册操作。
     *
     * @return 是否成功初始化
     * @retval true 初始化成功
     * @retval false 初始化失败（服务未启动或连接失败）
     */
    bool Initialize();

    /**
     * @brief 释放资源
     *
     * 断开与服务连接，释放相关资源。
     */
    void Release();

    /**
     * @brief 注册 CLI 工具
     *
     * 将一个新的 CLI 工具注册到系统中。注册成功后，工具可以被其他组件查询和调用。
     *
     * @param tool 工具信息（包含名称、路径、权限、Schema 等）
     * @param persist 是否持久化到注册表文件
     *               - true: 将工具信息写入 /etc/cli/cli_tools.json
     *               - false: 仅在内存中注册，系统重启后失效
     * @return 错误码
     * @retval ErrCode::OK 注册成功
     * @retval ErrCode::ERR_INVALID_ARGS 参数无效
     * @retval ErrCode::ERR_ALREADY_EXISTS 工具已存在
     * @retval ErrCode::ERR_PERMISSION_DENIED 权限不足
     * @retval ErrCode::ERR_SERVICE_NOT_READY 服务未就绪
     *
     * @note 调用者需要有 ohos.permission.MANAGE_CLI_TOOLS 权限
     * @note 如果工具已存在且 forceUpdate 为 false，返回 ERR_ALREADY_EXISTS
     *
     * 示例：
     * @code
     * ToolInfo tool;
     * tool.name = "my-scanner";
     * tool.version = "1.0.0";
     * tool.description = "Network scanner";
     * tool.executablePath = "/etc/cli/tools/my-scanner";
     * tool.permissions.push_back("ohos.permission.INTERNET");
     *
     * int32_t ret = CliToolRegistry::GetInstance()->RegisterTool(tool, true);
     * if (ret == ErrCode::OK) {
     *     HILOG_INFO("Tool registered successfully");
     * }
     * @endcode
     */
    int32_t RegisterTool(const ToolInfo& tool, bool persist = true);

    /**
     * @brief 批量注册 CLI 工具
     *
     * 一次性注册多个工具，原子操作。如果任一工具注册失败，所有注册都会回滚。
     *
     * @param tools 工具列表
     * @param persist 是否持久化到注册表文件
     * @return 错误码
     * @retval ErrCode::OK 全部注册成功
     * @retval ErrCode::ERR_INVALID_ARGS 参数无效
     * @retval ErrCode::ERR_ALREADY_EXISTS 部分工具已存在
     * @retval ErrCode::ERR_PERMISSION_DENIED 权限不足
     */
    int32_t RegisterTools(const std::vector<ToolInfo>& tools, bool persist = true);

    /**
     * @brief 更新已注册的工具
     *
     * 更新工具的元数据（版本、描述、权限等）。工具名称和可执行路径不能修改。
     *
     * @param tool 工具信息（必须包含有效的工具名称）
     * @param persist 是否持久化更新到注册表文件
     * @return 错误码
     * @retval ErrCode::OK 更新成功
     * @retval ErrCode::ERR_NOT_FOUND 工具不存在
     * @retval ErrCode::ERR_INVALID_ARGS 参数无效
     * @retval ErrCode::ERR_PERMISSION_DENIED 权限不足
     */
    int32_t UpdateTool(const ToolInfo& tool, bool persist = true);

    /**
     * @brief 注销 CLI 工具
     *
     * 从系统中移除已注册的工具。注销后工具将无法被查询和调用。
     *
     * @param name 工具名称
     * @param persist 是否从注册表文件中删除
     * @return 错误码
     * @retval ErrCode::OK 注销成功
     * @retval ErrCode::ERR_NOT_FOUND 工具不存在
     * @retval ErrCode::ERR_PERMISSION_DENIED 权限不足
     */
    int32_t UnregisterTool(const std::string& name, bool persist = true);

    /**
     * @brief 检查工具是否已注册
     *
     * @param name 工具名称
     * @param exists 输出参数，工具是否存在
     * @return 错误码
     */
    int32_t IsToolRegistered(const std::string& name, bool& exists);

    /**
     * @brief 查询已注册的工具
     *
     * @param toolName 工具名称
     * @param tool 输出参数，工具信息
     * @return 错误码
     * @retval ErrCode::OK 查询成功
     * @retval ErrCode::ERR_NOT_FOUND 工具不存在
     */
    int32_t QueryTool(const std::string& toolName, ToolInfo& tool);

    /**
     * @brief 查询所有已注册的工具
     *
     * @param tools 输出参数，工具列表
     * @return 错误码
     */
    int32_t QueryAllTools(std::vector<ToolInfo>& tools);

    /**
     * @brief 重新加载注册表
     *
     * 从文件重新加载工具注册信息。这将丢弃所有内存中动态注册但未持久化的工具。
     *
     * @return 错误码
     */
    int32_t ReloadRegistry();

    /**
     * @brief 设置回调监听器
     *
     * 监听工具注册表变化事件。
     *
     * @param callback 回调接口
     * @return 错误码
     */
    int32_t SetRegistryChangeListener(std::shared_ptr<RegistryChangeListener> callback);

    /**
     * @brief 注册表变化监听器
     */
    class RegistryChangeListener {
    public:
        virtual ~RegistryChangeListener() = default;

        /**
         * @brief 工具注册事件
         */
        virtual void OnToolRegistered(const ToolInfo& tool) = 0;

        /**
         * @brief 工具更新事件
         */
        virtual void OnToolUpdated(const ToolInfo& tool) = 0;

        /**
         * @brief 工具注销事件
         */
        virtual void OnToolUnregistered(const std::string& toolName) = 0;

        /**
         * @brief 注册表重新加载事件
         */
        virtual void OnRegistryReloaded() = 0;
    };

private:
    CliToolRegistry();
    ~CliToolRegistry();

    // 禁止拷贝和赋值
    CliToolRegistry(const CliToolRegistry&) = delete;
    CliToolRegistry& operator=(const CliToolRegistry&) = delete;

    // IPC 代理实现
    class CliToolRegistryImpl;
    std::unique_ptr<CliToolRegistryImpl> impl_;

    static std::mutex instanceMutex_;
    static CliToolRegistry* instance_;
};

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_TOOL_REGISTRY_H
