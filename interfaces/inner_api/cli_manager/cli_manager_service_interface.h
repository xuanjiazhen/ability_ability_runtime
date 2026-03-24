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
        EXEC_TOOL,
        SUBSCRIBE_SESSION,
        CANCEL_SESSION,
        QUERY_SESSION_STATUS,
    };

    /**
     * @brief 会话信息
     */
    struct SessionInfo {
        std::string sessionId;              // 会话/任务 ID
        std::string toolName;                // 工具名称
        std::string status;                  // "running" | "completed" | "failed"
        uint64_t startTime;                  // 开始时间戳
        uint64_t endTime = 0;                // 结束时间戳
        ExecutionResult* result = nullptr;   // 执行结果（仅 completed 时有值）
    };

    /**
     * @brief 查询所有工具
     * @param tools 输出参数，工具列表
     * @return 错误码
     */
    virtual int32_t QueryTools(std::vector<ToolInfo>& tools) = 0;

    /**
     * @brief 统一的执行接口
     * @param name 工具名称
     * @param arguments 执行参数 (JSON 格式)
     * @param options 执行选项
     * @param session 输出参数，会话信息
     * @return 错误码
     *
     * 执行行为：
     * - background=false: 在 yieldMs 内等待结果，超时后自动转入后台
     *   - 完成时: session.status="completed", session.result 有值
     *   - 超时时: session.status="running", session.result 为 nullptr
     * - background=true: 立即返回
     *   - session.status="running", session.result 为 nullptr
     */
    virtual int32_t ExecTool(
        const std::string& name,
        const Json::Value& arguments,
        const ExecOptions& options,
        SessionInfo& session
    ) = 0;

    /**
     * @brief 订阅会话事件
     * @param sessionId 会话 ID
     * @param callback IPC 回调对象
     * @return 错误码
     */
    virtual int32_t SubscribeSession(
        const std::string& sessionId,
        const sptr<ICLICallback>& callback
    ) = 0;

    /**
     * @brief 取消会话
     * @param sessionId 会话 ID
     * @return 错误码
     */
    virtual int32_t CancelSession(const std::string& sessionId) = 0;

    /**
     * @brief 查询会话状态
     * @param sessionId 会话 ID
     * @param status 输出参数，会话状态
     * @return 错误码
     */
    virtual int32_t QuerySessionStatus(const std::string& sessionId, SessionInfo& status) = 0;

    virtual ~ICLIManagerService() = default;
};

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_MANAGER_SERVICE_INTERFACE_H
