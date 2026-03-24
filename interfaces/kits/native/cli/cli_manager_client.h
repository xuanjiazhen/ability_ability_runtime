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
     * @brief 查询全量工具摘要信息
     * 只返回 name, description, version 三个字段
     *
     * @param summaries 输出参数，工具摘要列表
     * @return 错误码
     *
     * @since 2025-03-28
     * @version 2.6
     */
    static int32_t QueryToolSummaries(std::vector<ToolSummary>& summaries);

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
    static int32_t GetToolInfoByName(const std::string& name, ToolInfo& tool);

    /**
     * @brief 统一的执行接口
     * @param name 工具名称
     * @param arguments 执行参数 (JSON 格式)
     * @param session 输出参数，会话信息
     * @param options 执行选项
     * @return 错误码
     *
     * 执行行为：
     * - background=false: 在 yieldMs 内等待结果，超时后自动转入后台
     *   - 完成时: session.status="completed", session.result 有值
     *   - 超时时: session.status="running", session.result 为 nullptr
     * - background=true: 立即返回
     *   - session.status="running", session.result 为 nullptr
     */
    static int32_t ExecTool(
        const std::string& name,
        const Json::Value& arguments,
        SessionInfo& session,
        const ExecOptions& options = {}
    );

    /**
     * @brief 订阅会话事件
     * @param sessionId 会话 ID
     * @param callback 事件回调
     * @return 错误码
     */
    using EventCallback = std::function<void(const std::string&, const ToolEvent&)>;
    static int32_t SubscribeSession(
        const std::string& sessionId,
        EventCallback callback
    );

    /**
     * @brief 取消会话
     * @param sessionId 会话 ID
     * @return 错误码
     */
    static int32_t CancelSession(const std::string& sessionId);

    /**
     * @brief 查询会话状态
     * @param sessionId 会话 ID
     * @param status 输出参数，会话状态
     * @return 错误码
     */
    static int32_t QuerySessionStatus(const std::string& sessionId, SessionInfo& status);

    /**
     * @brief 向运行中的工具发送输入数据
     *
     * 通过 stdin 管道向工具进程发送数据。
     *
     * @param sessionId 会话 ID
     * @param input 输入数据（建议末尾添加换行符 '\n'）
     * @return 错误码
     *   - ERR_OK: 成功
     *   - ERR_NOT_FOUND: 会话不存在
     *   - ERR_INVALID_STATE: 工具未运行或 stdin 已关闭
     *   - ERR_IO_ERROR: 写入失败
     *
     * @since 2025-04-03
     */
    static int32_t SendInput(const std::string& sessionId, const std::string& input);

    /**
     * @brief 关闭工具的 stdin（发送 EOF）
     *
     * 关闭 stdin 管道的写端，向工具进程发送 EOF。
     *
     * @param sessionId 会话 ID
     * @return 错误码
     *   - ERR_OK: 成功
     *   - ERR_NOT_FOUND: 会话不存在
     *
     * @since 2025-04-03
     */
    static int32_t CloseInput(const std::string& sessionId);

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
