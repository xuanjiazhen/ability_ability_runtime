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
 * @version 2.6
 * @since 2025-03-27
 *
 * 更新内容：
 * - v2.6 (2025-03-28): 新增 ToolSummary 接口
 * - v2.6 (2025-03-28): 新增 queryToolSummaries() 接口（快速查询工具摘要）
 * - v2.6 (2025-03-28): 新增 getToolInfoByName() 接口（按名称查询工具详情）
 * - 新增 JSONSchema、TemplateDef、ArgMapping、SubCommandInfo 类型定义
 * - 完整更新 ToolInfo 接口，包含所有字段
 * - 新增 inputSchema、outputSchema、argMapping、eventSchemas 等关键字段
 * - 修正字段名称：timeout → timeoutMs，supportedEvents → eventTypes
 * - 新增 executablePath、registeredTime、enabled、hasSubcommands、subcommands 字段
 * - API 风格从回调改为 Promise/async-await
 */

/**
 * JSON Schema 类型定义
 * 遵循 JSON Schema 规范
 */
export type JSONSchema = object;

/**
 * 参数模板定义
 * 用于布尔类型参数的条件映射
 */
export interface TemplateDef {
  /**
   * 当值为 true 时的模板字符串
   * @example "--verbose"
   */
  if_true?: string;

  /**
   * 当值为 false 时的模板字符串
   * @example "" (空字符串表示不添加参数)
   */
  if_false?: string;
}

/**
 * 参数映射规则
 * 定义如何将抽象参数转换为命令行参数
 */
export interface ArgMapping {
  /**
   * 映射类型
   * - flag: 标志模式，使用模板字符串（如 --key=value）
   * - positional: 位置参数，按顺序排列
   * - flattened: 展平嵌套对象（如 parent.child=value）
   * - json-string: JSON 字符串传递
   * - javascript: 使用 JavaScript 自定义映射器
   */
  type: 'flag' | 'positional' | 'flattened' | 'json-string' | 'javascript';

  /**
   * 分隔符（仅用于 flattened 模式）
   * @default "."
   * @example "." 表示 parent.child
   * @example "-" 表示 parent-child
   */
  separator?: string;

  /**
   * 参数模板映射表
   * key 为参数名，value 为模板字符串或条件对象
   * @example { "target": "--target={value}" }
   * @example { "verbose": { "if_true": "-v", "if_false": "" } }
   */
  templates?: Record<string, string | TemplateDef>;

  /**
   * 位置参数顺序（仅用于 positional 模式）
   * @example ["inputFile", "outputFile", "options"]
   */
  order?: Array<string>;

  /**
   * JavaScript 自定义映射器代码（仅用于 javascript 模式）
   * @example "function(args) { return ['--custom=' + args.custom]; }"
   */
  mapper?: string;
}

/**
 * 子命令信息
 * 用于支持子命令的工具（如 pkg-manager）
 */
export interface SubCommandInfo {
  /**
   * 子命令描述
   */
  description: string;

  /**
   * 输入参数 Schema
   * 定义子命令的参数格式
   */
  inputSchema: JSONSchema;

  /**
   * 输出结果 Schema
   * 定义子命令的返回结果格式
   */
  outputSchema: JSONSchema;

  /**
   * 参数映射规则
   */
  argMapping: ArgMapping;

  /**
   * 支持的事件类型
   * @example ["progress", "result"]
   */
  eventTypes: Array<string>;

  /**
   * 事件 Schema 定义
   * key 为事件类型，value 为事件数据格式
   * @example { "progress": { ... } }
   */
  eventSchemas: Record<string, JSONSchema>;
}

/**
 * 工具摘要信息
 * 仅包含基础字段，用于工具列表快速展示
 *
 * @since 2025-03-28
 * @version 2.6
 */
export interface ToolSummary {
  /**
   * 工具名称
   * 唯一标识符
   */
  name: string;

  /**
   * 工具版本
   * 遵循语义化版本规范（SemVer）
   */
  version: string;

  /**
   * 工具描述
   * 简要说明工具的功能和用途
   */
  description: string;
}

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
   * 0 表示使用工具描述文件中的默认值
   */
  timeoutMs?: number;

  /**
   * 工作目录
   */
  workingDir?: string;

  /**
   * 环境变量
   */
  env?: Record<string, string>;
}

/**
 * 工具信息
 * 包含工具的完整元数据、参数定义和事件支持信息
 */
export interface ToolInfo {
  /**
   * 工具名称
   * 唯一标识符
   */
  name: string;

  /**
   * 工具版本
   * 遵循语义化版本规范（SemVer）
   */
  version: string;

  /**
   * 工具描述
   * 简要说明工具的功能和用途
   */
  description: string;

  /**
   * 可执行文件路径
   * 工具的绝对路径
   */
  executablePath: string;

  /**
   * 所需权限列表
   * 调用该工具需要的权限
   */
  permissions: Array<string>;

  /**
   * 输入参数 Schema
   * 定义工具的参数格式（JSON Schema）
   *
   * @example
   * ```typescript
   * {
   *   "type": "object",
   *   "properties": {
   *     "target": {
   *       "type": "string",
   *       "format": "ipv4",
   *       "description": "Target IP address"
   *     },
   *     "ports": {
   *       "type": "array",
   *       "items": {"type": "integer"},
   *       "description": "Ports to scan"
   *     }
   *   },
   *   "required": ["target"]
   * }
   * ```
   */
  inputSchema: JSONSchema;

  /**
   * 输出结果 Schema
   * 定义工具执行完成后的返回结果格式（JSON Schema）
   */
  outputSchema: JSONSchema;

  /**
   * 参数映射规则
   * 定义如何将 inputSchema 中的参数转换为命令行参数
   */
  argMapping: ArgMapping;

  /**
   * 事件 Schema 定义
   * key 为事件类型，value 为事件数据格式
   *
   * @example
   * ```typescript
   * {
   *   "progress": {
   *     "type": "object",
   *     "properties": {
   *       "percentage": {"type": "integer"},
   *       "status": {"type": "string"}
   *     }
   *   }
   * }
   * ```
   */
  eventSchemas: Record<string, JSONSchema>;

  /**
   * 默认超时时间（毫秒）
   * 工具执行的最大时长，0 表示无限制
   */
  timeoutMs: number;

  /**
   * 支持的事件类型列表
   * @example ["stdout", "stderr", "exit", "error", "progress", "result"]
   */
  eventTypes: Array<string>;

  /**
   * 注册时间戳（毫秒）
   * 工具注册到系统的时间
   */
  registeredTime: number;

  /**
   * 是否启用
   * false 表示工具已禁用，不可调用
   */
  enabled: boolean;

  /**
   * 是否支持子命令
   * true 表示该工具有多个子命令（如 pkg-manager install/remove/list）
   */
  hasSubcommands: boolean;

  /**
   * 子命令映射表
   * key 为子命令名称，value 为子命令信息
   * 仅当 hasSubcommands=true 时有效
   *
   * @example
   * ```typescript
   * {
   *   "install": {
   *     "description": "Install a package",
   *     "inputSchema": { ... },
   *     "outputSchema": { ... },
   *     "argMapping": { ... },
   *     "eventTypes": ["progress"],
   *     "eventSchemas": { ... }
   *   },
   *   "remove": { ... }
   * }
   * ```
   */
  subcommands?: Record<string, SubCommandInfo>;
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
export default class CliManager {
  /**
   * 查询全量工具摘要信息
   * 只返回 name, description, version 三个字段
   *
   * @returns Promise<Array<ToolSummary>> 返回工具摘要列表
   *
   * @example
   * ```typescript
   * const summaries = await CliManager.queryToolSummaries();
   * summaries.forEach(tool => {
   *   console.log(`- ${tool.name} v${tool.version}: ${tool.description}`);
   * });
   * ```
   *
   * @since 2025-03-28
   * @version 2.6
   */
  static queryToolSummaries(): Promise<Array<ToolSummary>>;

  /**
   * 查询全量工具信息
   * 返回 ToolInfo 完整信息
   *
   * @returns Promise<Array<ToolInfo>> 返回所有可用工具的列表
   *
   * @example
   * ```typescript
   * const tools = await CliManager.queryTools();
   * tools.forEach(tool => {
   *   console.log(`${tool.name}: ${tool.description}`);
   *   console.log('Input schema:', tool.inputSchema);
   * });
   * ```
   */
  static queryTools(): Promise<Array<ToolInfo>>;

  /**
   * 根据名称查询单个工具的具体信息
   *
   * @param name 工具名称
   *
   * @returns Promise<ToolInfo> 返回工具完整信息
   *
   * @throws {Error} 工具不存在时抛出异常
   *
   * @example
   * ```typescript
   * try {
   *   const scanner = await CliManager.getToolInfoByName('network-scanner');
   *   console.log('Scanner schema:', scanner.inputSchema);
   *   console.log('Scanner permissions:', scanner.permissions);
   * } catch (error) {
   *   console.error('Tool not found:', error);
   * }
   * ```
   *
   * @since 2025-03-28
   * @version 2.6
   */
  static getToolInfoByName(name: string): Promise<ToolInfo>;

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
   * const session = await CliManager.execTool('network-scanner', {
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
   * const session = await CliManager.execTool('log-monitor', {
   *   filter: 'ERROR'
   * }, {
   *   background: true
   * });
   *
   * // 订阅事件
   * CliManager.subscribeSession(session.sessionId, (event) => {
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
   * CliManager.subscribeSession(sessionId, (event) => {
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
   * const cancelled = await CliManager.cancelSession(sessionId);
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
   * const status = await CliManager.querySessionStatus(sessionId);
   * console.log(`Status: ${status.status}`);
   * if (status.result) {
   *   console.log(`Exit code: ${status.result.exitCode}`);
   * }
   * ```
   */
  static querySessionStatus(sessionId: string): Promise<SessionStatus>;
}
