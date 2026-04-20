/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
 * @version 1.0
 * @since 2026
 */

/**
 * @namespace cliManager
 * @description cliManager 工具管理命名空间
 */
declare namespace cliManager {
  /**
   * JSON Schema 类型定义
   */
  export type JSONSchema = object;

  /**
   * 参数模板定义
   */
  export interface TemplateDef {
    if_true?: string;
    if_false?: string;
  }

  /**
   * 参数映射规则
   */
  export interface ArgMapping {
    type: 'flag' | 'positional' | 'flattened' | 'json-string';
    separator?: string;
    templates?: Record<string, string | TemplateDef>;
    order?: Array<string>;
    mapper?: string;
  }

  /**
   * 子命令信息
   */
  export interface SubCommandInfo {
    description: string;
    inputSchema: JSONSchema;
    outputSchema: JSONSchema;
    argMapping: ArgMapping;
    eventTypes: Array<string>;
    eventSchemas: Record<string, JSONSchema>;
  }

  /**
   * 工具摘要信息
   */
  export interface ToolSummary {
    name: string;
    version: string;
    description: string;
  }

  /**
   * 工具信息
   */
  export interface ToolInfo {
    name: string;
    version: string;
    description: string;
    executablePath: string;
    permissions: Array<string>;
    inputSchema: JSONSchema;
    outputSchema: JSONSchema;
    argMapping: ArgMapping;
    eventSchemas: Record<string, JSONSchema>;
    timeout: number;
    eventTypes: Array<string>;
    registeredTime: number;
    enabled: boolean;
    hasSubcommands: boolean;
    subcommands?: Record<string, SubCommandInfo>;
  }

  /**
   * 查询全量工具摘要信息
   * @returns Promise<Array<ToolSummary>> 返回工具摘要列表
   */
  function queryToolSummaries(): Promise<Array<ToolSummary>>;

  /**
   * 查询全量工具信息
   * @returns Promise<Array<ToolInfo>> 返回所有可用工具的列表
   */
  function queryTools(): Promise<Array<ToolInfo>>;

  /**
   * 根据名称查询单个工具的具体信息
   * @param name 工具名称
   * @returns Promise<ToolInfo> 返回工具完整信息
   * @throws {Error} 工具不存在时抛出异常
   */
  function getToolInfoByName(name: string): Promise<ToolInfo>;

  /**
   * 注册 CLI 工具
   * @param tool 工具信息
   * @returns Promise<void>
   */
  function registerCliTool(tool: ToolInfo): Promise<void>;
}

export default cliManager;
