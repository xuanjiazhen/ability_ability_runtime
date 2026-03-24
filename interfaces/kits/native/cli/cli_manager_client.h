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

#ifndef CLI_MANAGER_CLIENT_H
#define CLI_MANAGER_CLIENT_H

#include <string>
#include <vector>
#include <functional>
#include "cli_tool_info.h"
#include "cli_execution_context.h"
#include "cli_manager_error.h"

namespace OHOS {
namespace CliMgr {

/**
 * @brief CLI 管理器客户端
 *
 * 对外暴露的客户端 API，供应用调用
 */
class CLIManagerClient {
public:
    /**
     * @brief 查询所有可用工具
     * @param tools 输出参数，工具列表
     * @return 错误码
     */
    static int32_t QueryTools(std::vector<ToolInfo>& tools);

    /**
     * @brief 根据名称查询工具
     * @param name 工具名称
     * @param tool 输出参数，工具信息
     * @return 错误码
     */
    static int32_t QueryToolByName(const std::string& name, ToolInfo& tool);

    /**
     * @brief 根据权限过滤工具
     * @param permission 权限名称
     * @param tools 输出参数，工具列表
     * @return 错误码
     */
    static int32_t QueryToolsByPermission(const std::string& permission,
                                         std::vector<ToolInfo>& tools);

    /**
     * @brief 同步执行工具
     * @param name 工具名称
     * @param arguments 执行参数 (JSON 格式)
     * @param result 输出参数，执行结果
     * @param options 执行选项
     * @return 错误码
     */
    static int32_t ExecTool(
        const std::string& name,
        const Json::Value& arguments,
        ExecutionResult& result,
        const ExecOptions& options = {}
    );

    /**
     * @brief 异步执行工具
     * @param name 工具名称
     * @param arguments 执行参数 (JSON 格式)
     * @param callback 事件回调
     * @param taskId 输出参数，任务 ID
     * @param options 执行选项
     * @return 错误码
     */
    using EventCallback = std::function<void(const std::string&, const ToolEvent&)>;
    static int32_t ExecToolAsync(
        const std::string& name,
        const Json::Value& arguments,
        EventCallback callback,
        std::string& taskId,
        const ExecOptions& options = {}
    );

    /**
     * @brief 取消异步任务
     * @param taskId 任务 ID
     * @return 错误码
     */
    static int32_t CancelTask(const std::string& taskId);

    /**
     * @brief 查询任务状态
     * @param taskId 任务 ID
     * @param status 输出参数，任务状态
     * @return 错误码
     */
    static int32_t QueryTaskStatus(const std::string& taskId, TaskStatus& status);

private:
    // 获取服务代理
    static sptr<ICLIManagerService> GetServiceProxy();

    // 连接服务
    static int32_t ConnectService();

    // 单例实例
    class CLIManagerClientImpl;
    static CLIManagerClientImpl* instance_;
};

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_MANAGER_CLIENT_H
