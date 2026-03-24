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
        // 工具查询和执行
        QUERY_TOOLS = 1,
        QUERY_TOOL_SUMMARIES,  // 查询工具摘要列表 (新增)
        GET_TOOL_INFO_BY_NAME, // 根据名称查询工具详情 (新增)
        EXEC_TOOL,
        SUBSCRIBE_SESSION,
        CANCEL_SESSION,
        QUERY_SESSION_STATUS,

        // 动态工具注册
        REGISTER_TOOL = 100,
        REGISTER_TOOLS,
        UPDATE_TOOL,
        UNREGISTER_TOOL,
        QUERY_TOOL,
        RELOAD_REGISTRY,
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
     * @brief 查询全量工具摘要信息
     * 只返回 name, description, version 三个字段
     *
     * @param summaries 输出参数，工具摘要列表
     * @return 错误码
     *
     * @since 2025-03-28
     * @version 2.6
     */
    virtual int32_t QueryToolSummaries(std::vector<ToolSummary>& summaries) = 0;

    /**
     * @brief 根据名称查询单个工具的具体信息
     *
     * @param name 工具名称
     * @param tool 输出参数，工具完整信息
     * @return 错误码
     *   - ERR_OK: 成功
     *   - ERR_NOT_FOUND: 工具不存在
     *
     * @since 2025-03-28
     * @version 2.6
     */
    virtual int32_t GetToolInfoByName(const std::string& name, ToolInfo& tool) = 0;

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

    // ========================================================================
    // 动态工具注册接口
    // ========================================================================

    /**
     * @brief 动态注册 CLI 工具
     * @param tool 工具信息
     * @param persist 是否持久化到注册表文件
     * @return 错误码
     */
    virtual int32_t RegisterTool(const ToolInfo& tool, bool persist) = 0;

    /**
     * @brief 批量注册工具
     * @param tools 工具列表
     * @param persist 是否持久化
     * @return 错误码
     */
    virtual int32_t RegisterTools(const std::vector<ToolInfo>& tools, bool persist) = 0;

    /**
     * @brief 更新已注册的工具
     * @param tool 工具信息
     * @param persist 是否持久化
     * @return 错误码
     */
    virtual int32_t UpdateTool(const ToolInfo& tool, bool persist) = 0;

    /**
     * @brief 注销工具
     * @param name 工具名称
     * @param persist 是否从文件删除
     * @return 错误码
     */
    virtual int32_t UnregisterTool(const std::string& name, bool persist) = 0;

    /**
     * @brief 查询单个工具
     * @param toolName 工具名称
     * @param tool 输出参数，工具信息
     * @return 错误码
     */
    virtual int32_t QueryTool(const std::string& toolName, ToolInfo& tool) = 0;

    /**
     * @brief 重新加载注册表
     * @return 错误码
     */
    virtual int32_t ReloadRegistry() = 0;

    virtual ~ICLIManagerService() = default;
};

} // namespace CliMgr
} // namespace OHOS

#endif // CLI_MANAGER_SERVICE_INTERFACE_H
