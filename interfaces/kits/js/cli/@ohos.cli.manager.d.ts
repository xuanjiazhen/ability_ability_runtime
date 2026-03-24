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
 */

/**
 * 工具执行选项
 */
export interface ExecOptions {
  /**
   * 执行模式
   * - 'sync': 同步执行，等待结果返回
   * - 'async': 异步执行，通过回调接收事件
   */
  mode?: 'sync' | 'async';

  /**
   * 超时时间（毫秒）
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
  name: string;
  version: string;
  description: string;
  type: 'short-lived' | 'long-lived';
  permissions: string[];
  timeout: number;
  supportedEvents: string[];
}

/**
 * 执行结果（同步模式）
 */
export interface ExecResult {
  taskId: string;
  exitCode: number;
  stdout: string;
  stderr: string;
  signal: number;
  timedOut: boolean;
  executionTime: number;
}

/**
 * 工具输出事件（异步模式）
 */
export interface ToolEvent {
  type: 'stdout' | 'stderr' | 'exit' | 'error';
  data: string;
  exitCode?: number;
  timestamp: number;
}

/**
 * 任务状态
 */
export interface TaskStatus {
  taskId: string;
  toolName: string;
  status: 'running' | 'completed' | 'failed' | 'timeout' | 'cancelled';
  startTime: number;
  endTime?: number;
  exitCode?: number;
}

/**
 * CLI 管理器
 */
export class CLIManager {
  /**
   * 查询所有可用工具
   * @param callback 回调函数 (error: Error, tools: ToolInfo[]) => void
   */
  static queryTools(callback: (error: Error | null, tools?: ToolInfo[]) => void): void;

  /**
   * 根据名称查询工具
   * @param name 工具名称
   * @param callback 回调函数 (error: Error, tool: ToolInfo) => void
   */
  static queryToolByName(name: string,
                        callback: (error: Error | null, tool?: ToolInfo) => void): void;

  /**
   * 根据权限过滤工具
   * @param permission 权限名称
   * @param callback 回调函数 (error: Error, tools: ToolInfo[]) => void
   */
  static queryToolsByPermission(permission: string,
                                callback: (error: Error | null, tools?: ToolInfo[]) => void): void;

  /**
   * 同步执行工具
   * @param name 工具名称
   * @param args 执行参数
   * @param options 选项（包含 callback）
   */
  static execTool(name: string,
                  args: Record<string, any>,
                  options: {
                    options?: ExecOptions;
                    callback: (error: Error | null, result?: ExecResult) => void;
                  }): void;

  /**
   * 异步执行工具
   * @param name 工具名称
   * @param args 执行参数
   * @param eventCallback 事件回调函数
   * @param options 执行选项
   * @returns Promise<string> 返回任务 ID
   */
  static execToolAsync(name: string,
                      args: Record<string, any>,
                      eventCallback: (event: ToolEvent) => void,
                      options?: ExecOptions): Promise<string>;

  /**
   * 取消异步任务
   * @param taskId 任务 ID
   * @returns 是否成功取消
   */
  static cancelTask(taskId: string): boolean;

  /**
   * 查询任务状态
   * @param taskId 任务 ID
   * @returns 任务状态
   * @throws Error 如果查询失败
   */
  static queryTaskStatus(taskId: string): TaskStatus;
}

export default CLIManager;
