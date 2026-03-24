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

/**
 * @fileoverview CLI Manager TypeScript 类型定义
 * @version 2.0
 * @since 2025-03-26
 */

/**
 * @namespace CLI
 * @description CLI 工具管理命名空间
 */
declare namespace CLI {
  /**
   * 工具执行选项
   */
  export interface ExecOptions {
    /**
     * 是否后台执行
     * - false: 前台执行，在 yieldMs 内等待结果返回，超时后自动转入后台
     * - true: 立即进入后台执行，马上返回 sessionId
     *
     * @default false
     */
    background?: boolean;

    /**
     * 前台等待超时时间（毫秒）
     * 仅当 background=false 时有效
     *
     * @default 30000 (30秒)
     */
    yieldMs?: number;

    /**
     * 总超时时间（毫秒）
     * 进程运行超过此时间会被强制终止
     * 默认使用工具描述文件中的 timeout
     */
    timeout?: number;

    /**
     * 环境变量
     */
    env?: Record<string, string>;

    /**
     * 工作目录
     */
    cwd?: string;
  }

  /**
   * 工具信息
   */
  export interface ToolInfo {
    /**
     * 工具名称
     */
    name: string;

    /**
     * 工具版本
     */
    version: string;

    /**
     * 工具描述
     */
    description: string;

    /**
     * 工具类型
     * - 'short-lived': 短时运行，快速退出
     * - 'long-lived': 长时运行，持续输出
     */
    type: 'short-lived' | 'long-lived';

    /**
     * 所需权限列表
     */
    permissions: string[];

    /**
     * 默认超时时间（毫秒）
     */
    timeout: number;

    /**
     * 支持的事件类型列表
     */
    supportedEvents: string[];
  }

  /**
   * 执行结果
   * 包含工具执行的输出和状态信息
   */
  export interface ExecResult {
    /**
     * 任务 ID
     */
    taskId: string;

    /**
     * 退出码
     * 0 表示成功
     */
    exitCode: number;

    /**
     * 标准输出
     */
    stdout: string;

    /**
     * 标准错误
     */
    stderr: string;

    /**
     * 终止信号（如果被信号终止）
     */
    signal: number;

    /**
     * 是否超时
     */
    timedOut: boolean;

    /**
     * 执行时长（毫秒）
     */
    executionTime: number;
  }

  /**
   * 会话信息
   * 表示一次工具执行的会话
   */
  export interface SessionInfo {
    /**
     * 会话/任务 ID
     */
    sessionId: string;

    /**
     * 工具名称
     */
    toolName: string;

    /**
     * 会话状态
     * - 'running': 正在运行
     * - 'completed': 已完成
     * - 'failed': 失败
     */
    status: 'running' | 'completed' | 'failed';

    /**
     * 开始时间戳（毫秒）
     */
    startTime: number;

    /**
     * 结束时间戳（毫秒）
     * 仅在 status 为 'completed' 或 'failed' 时有值
     */
    endTime?: number;

    /**
     * 执行结果
     * 仅在 status 为 'completed' 时有值
     */
    result?: ExecResult;
  }

  /**
   * 会话状态（查询返回）
   */
  export interface SessionStatus {
    /**
     * 会话 ID
     */
    sessionId: string;

    /**
     * 工具名称
     */
    toolName: string;

    /**
     * 会话状态
     */
    status: 'running' | 'completed' | 'failed';

    /**
     * 开始时间戳（毫秒）
     */
    startTime: number;

    /**
     * 结束时间戳（毫秒）
     * 仅在 status 为 'completed' 或 'failed' 时有值
     */
    endTime?: number;

    /**
     * 执行结果
     * 仅在 status 为 'completed' 时有值
     */
    result?: ExecResult;
  }

  /**
   * 工具输出事件
   * 订阅后台会话时触发的事件
   */
  export interface ToolEvent {
    /**
     * 事件类型
     * - 'stdout': 标准输出
     * - 'stderr': 标准错误
     * - 'exit': 进程退出
     * - 'error': 错误事件
     */
    type: 'stdout' | 'stderr' | 'exit' | 'error';

    /**
     * 事件数据
     */
    data: string;

    /**
     * 退出码
     * 仅在 type 为 'exit' 时有值
     */
    exitCode?: number;

    /**
     * 时间戳（毫秒）
     */
    timestamp: number;
  }

  /**
   * CLI 管理器
   * 提供 CLI 工具的查询、执行和管理功能
   */
  export class CLIManager {
    /**
     * 查询所有可用工具
     *
     * @returns Promise<ToolInfo[]> 返回所有可用工具的列表
     *
     * @example
     * ```typescript
     * const tools = await CLI.CLIManager.queryTools();
     * tools.forEach(tool => {
     *   console.log(`${tool.name}: ${tool.description}`);
     * });
     * ```
     */
    static queryTools(): Promise<ToolInfo[]>;

    /**
     * 统一的执行接口
     *
     * 根据 background 参数决定执行模式：
     * - background=false: 在 yieldMs 内等待结果，超时后自动转入后台
     * - background=true: 立即进入后台执行
     *
     * @param name 工具名称
     * @param args 执行参数（JSON 对象）
     * @param options 执行选项
     *
     * @returns Promise<SessionInfo> 返回会话信息
     * - 前台完成: status='completed', result 有值
     * - 后台运行: status='running', result 为 undefined
     *
     * @example
     * ```typescript
     * // 前台执行
     * const session = await CLI.CLIManager.execTool('network-scanner', {
     *   target: '192.168.1.1',
     *   ports: [80, 443]
     * }, {
     *   background: false,
     *   yieldMs: 30000
     * });
     *
     * if (session.status === 'completed' && session.result) {
     *   console.log('Result:', session.result.stdout);
     * }
     * ```
     *
     * @example
     * ```typescript
     * // 后台执行
     * const session = await CLI.CLIManager.execTool('log-monitor', {
     *   filter: 'ERROR'
     * }, {
     *   background: true
     * });
     *
     * // 订阅事件
     * CLI.CLIManager.subscribeSession(session.sessionId, (event) => {
     *   console.log(event.data);
     * });
     * ```
     */
    static execTool(
      name: string,
      args: Record<string, any>,
      options?: ExecOptions
    ): Promise<SessionInfo>;

    /**
     * 订阅后台会话事件
     *
     * @param sessionId 会话 ID
     * @param callback 事件回调函数
     *
     * @returns Promise<boolean> 是否订阅成功
     *
     * @example
     * ```typescript
     * CLI.CLIManager.subscribeSession(sessionId, (event) => {
     *   if (event.type === 'stdout') {
     *     console.log('Output:', event.data);
     *   } else if (event.type === 'exit') {
     *     console.log('Exit code:', event.exitCode);
     *   } else if (event.type === 'error') {
     *     console.error('Error:', event.data);
     *   }
     * });
     * ```
     */
    static subscribeSession(
      sessionId: string,
      callback: (event: ToolEvent) => void
    ): Promise<boolean>;

    /**
     * 取消后台会话
     *
     * @param sessionId 会话 ID
     *
     * @returns Promise<boolean> 是否取消成功
     *
     * @example
     * ```typescript
     * const cancelled = await CLI.CLIManager.cancelSession(sessionId);
     * if (cancelled) {
     *   console.log('Session cancelled');
     * }
     * ```
     */
    static cancelSession(sessionId: string): Promise<boolean>;

    /**
     * 查询会话状态
     *
     * @param sessionId 会话 ID
     *
     * @returns Promise<SessionStatus> 会话状态
     *
     * @example
     * ```typescript
     * const status = await CLI.CLIManager.querySessionStatus(sessionId);
     * console.log(`Status: ${status.status}`);
     * if (status.result) {
     *   console.log(`Exit code: ${status.result.exitCode}`);
     * }
     * ```
     */
    static querySessionStatus(sessionId: string): Promise<SessionStatus>;
  }
}

export default CLI;
