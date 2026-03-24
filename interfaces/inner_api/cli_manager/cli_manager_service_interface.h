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

#ifndef CLI_MANAGER_SERVICE_INTERFACE_H
#define CLI_MANAGER_SERVICE_INTERFACE_H

#include <string>
#include <vector>
#include "iremote_broker.h"
#include "cli_tool_info.h"
#include "cli_execution_context.h"
#include "cli_callback.h"

namespace OHOS {
namespace CliMgr {

/**
 * @brief CLI Manager Service 接口
 *
 * SystemAbility 接口定义，用于客户端和服务器端通信
 */
class ICLIManagerService : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(CLI_MANAGER_SERVICE);

    // 消息 ID 定义
    enum {
        QUERY_TOOLS = 1,
        QUERY_TOOL_BY_NAME,
        EXEC_TOOL,
        EXEC_TOOL_ASYNC,
        CANCEL_TASK,
        QUERY_TASK_STATUS,
    };

    /**
     * @brief 查询所有工具
     * @param tools 输出参数，工具列表
     * @return 错误码
     */
    virtual int32_t QueryTools(std::vector<ToolInfo>& tools) = 0;

    /**
     * @brief 查询单个工具
     * @param name 工具名称
     * @param tool 输出参数，工具信息
     * @return 错误码
     */
    virtual int32_t QueryToolByName(const std::string& name, ToolInfo& tool) = 0;

    /**
     * @brief 同步执行工具
     * @param name 工具名称
     * @param arguments 执行参数 (JSON 格式)
     * @param options 执行选项
     * @param result 输出参数，执行结果
     * @return 错误码
     */
    virtual int32_t ExecTool(
        const std::string& name,
        const Json::Value& arguments,
        const ExecOptions& options,
        ExecutionResult& result
    ) = 0;

    /**
     * @brief 异步执行工具
     * @param name 工具名称
     * @param arguments 执行参数 (JSON 格式)
     * @param callback IPC 回调对象
     * @param options 执行选项
     * @param taskId 输出参数，任务 ID
     * @return 错误码
     */
    virtual int32_t ExecToolAsync(
        const std::string& name,
        const Json::Value& arguments,
        const sptr<ICLICallback>& callback,
        const ExecOptions& options,
        std::string& taskId
    ) = 0;

    /**
     * @brief 取消任务
     * @param taskId 任务 ID
     * @return 错误码
     */
    virtual int32_t CancelTask(const std::string& taskId) = 0;

    /**
     * @brief 查询任务状态
     * @param taskId 任务 ID
     * @param status 输出参数，任务状态
     * @return 错误码
     */
    virtual int32_t QueryTaskStatus(const std::string& taskId, TaskStatus& status) = 0;

    virtual ~ICLIManagerService() = default;
};

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_MANAGER_SERVICE_INTERFACE_H
