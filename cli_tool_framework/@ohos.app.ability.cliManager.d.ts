/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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
 * @file
 * @kit AbilityKit
 */
import { ToolInfo, ToolSummary } from './application/ToolInfo';
import { ToolEventCallback } from './application/ToolEventCallback';

/**
 * The module provides the capability to interact with cli tools in the system.
 *
 * @namespace cliManager
 * @syscap SystemCapability.Ability.AgentRuntime.Core
 * @systemapi
 * @stagemodelonly
 * @since 26.0.0 dynamic&static
 */
declare namespace cliManager {
  /**
   * Tool execution options.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  interface ExecOptions {
    /**
     * Indicates whether the tool is executed in the background
     *
     * @syscap SystemCapability.Ability.AgentRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    background?: boolean;

    /**
     * Indicates the foreground waiting timeout in milliseconds.
     *
     * @syscap SystemCapability.Ability.AgentRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    yieldMs?: int;

    /**
     * Indicates the maximum execution time of the tool, in seconds.
     *
     * @syscap SystemCapability.Ability.AgentRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    timeout?: int;
  }

  /**
   * Exexute result of a tool execution.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  interface ExecResult {
    /**
     * Indicates the exit code, 0 means success.
     *
     * @syscap SystemCapability.Ability.AgentRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    exitCode: number;

    /**
     * Indicates the standard output of the tool.
     *
     * @syscap SystemCapability.Ability.AgentRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    outputText: string;

    /**
     * Indicates the error output of the tool.
     *
     * @syscap SystemCapability.Ability.AgentRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    errorText: string;

    /**
     * Indicates the termination signal (if the tool process was terminated by a signal).
     *
     * @syscap SystemCapability.Ability.AgentRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    signalNumber: number;

    /**
     * Indicates whether it timed out.
     *
     * @syscap SystemCapability.Ability.AgentRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    timeOut: boolean;

    /**
     * Indicates the execution duration in milliseconds.
     *
     * @syscap SystemCapability.Ability.AgentRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    executionTime: number;
  }

  /**
   * Enum for session status.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  enum SessionStatus {
    /**
     * Indicates that the status is running.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    RUNNING = 'running',

    /**
     * Indicates that the status is completed.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    COMPLETED = 'completed',

    /**
     * Indicates that the status is failed.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    FAILED = 'failed'
  }

  /**
   * Session information of a tool execution.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  interface CliSessionInfo {
    /**
     * Indicates id of this session.
     *
     * @syscap SystemCapability.Ability.AgentRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    sessionId: string;

    /**
     * Indicates name of tool.
     *
     * @syscap SystemCapability.Ability.AgentRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    toolName: string;

    /**
     * Indicates status of session.
     *
     * @syscap SystemCapability.Ability.AgentRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    status: SessionStatus;

    /**
     * Indicates the execution result, only has a value when stats is completed.
     *
     * @syscap SystemCapability.Ability.AgentRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    result?: ExecResult;
  }

  /**
   * Query all tool summary information. The summary information only contains the fields: name, description, version.
   *
   * @permission ohos.permission.QUERY_AND_EXEC_CLI_TOOL
   * @returns { Promise<Array<ToolSummary>> } List of full tool summaries.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  function queryToolSummaries(): Promise<Array<ToolSummary>>;

  /**
   * Query all detailed information of tools
   *
   * @permission ohos.permission.QUERY_AND_EXEC_CLI_TOOL
   * @returns { Promise<Array<ToolInfo>> } List of full tool summaries.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  function queryTools(): Promise<Array<ToolInfo>>;

  /**
   * Get detailed information of a single tool by its name
   *
   * @permission ohos.permission.QUERY_AND_EXEC_CLI_TOOL
   * @returns { Promise<ToolInfo> } detailed information of tool.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  function getToolInfoByName(toolName: string): Promise<ToolInfo>;

  /**
   * Execute tool.
   *
   * @permission ohos.permission.QUERY_AND_EXEC_CLI_TOOL
   * @param { string } tool - The name of target tool.
   * @param { string } subCommand- The subCommand of this execute action.
   * @param { string } args - The input args of tool.
   * @param { string } challenge - The unique identifier get from access token manager.
   * @param { ExecOptions } execOptions - The options of this action.
   * @returns { Promise<CliSessionInfo> } execute result.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @throws { BusinessError } 35600030 - The tool not exist.
   * @throws { BusinessError } 35600031 - Maximum number of processes has been reached.
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic
   */
  function execTool(toolName: string, subCommand: string, args: Record<string, Object>, challenge: string,
    execOptions?: ExecOptions): Promise<CliSessionInfo>;

  /**
   * Execute tool.
   *
   * @permission ohos.permission.QUERY_AND_EXEC_CLI_TOOL
   * @param { string } tool - The name of target tool.
   * @param { string } subCommand- The subCommand of this execute action.
   * @param { string } args - The input args of tool.
   * @param { string } challenge - The unique identifier get from access token manager.
   * @param { ExecOptions } execOptions - The options of this action.
   * @returns { Promise<CliSessionInfo> } execute result.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @throws { BusinessError } 35600030 - The tool does not exist.
   * @throws { BusinessError } 35600031 - Maximum number of processes has been reached.
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 static
   */
  function execTool(toolName: string, subCommand: string, args: Record<string, RecordData>, challenge: string,
    execOptions?: ExecOptions): Promise<CliSessionInfo>;

  /**
   * Subscribe session event.
   *
   * @permission ohos.permission.QUERY_AND_EXEC_CLI_TOOL
   * @param { string } sessionId - The session id of target tool process.
   * @param { ToolEventCallback } callback - The input args of tool.
   * @returns { Promise<void> } The promise returned by the function.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @throws { BusinessError } 35600032 - The session does not exist.
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  function subscribeSession(sessionId: string, callback: ToolEventCallback): Promise<void>;

  /**
   * Close session and force kill tool process.
   *
   * @permission ohos.permission.QUERY_AND_EXEC_CLI_TOOL
   * @param { string } sessionId - The session id of target tool process..
   * @returns { Promise<void> } The promise returned by the function.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @throws { BusinessError } 35600032 - The session does not exist.
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  function clearSession(sessionId: string): Promise<void>;

  /**
   * Query session status.
   *
   * @permission ohos.permission.QUERY_AND_EXEC_CLI_TOOL
   * @param { string } sessionId - The session id of target tool process.
   * @returns { Promise<void> } The promise returned by the function.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @throws { BusinessError } 35600032 - The session does not exist.
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  function querySession(sessionId: string): Promise<CliSessionInfo>;

  /**
   * Send event to target process.
   *
   * @permission ohos.permission.QUERY_AND_EXEC_CLI_TOOL
   * @param { string } sessionId - The session id of target tool process.
   * @returns { Promise<void> } The promise returned by the function.
   * @throws { BusinessError } 201 - Permission denied.
   * @throws { BusinessError } 202 - Not system application.
   * @throws { BusinessError } 35600032 - The session does not exist.
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  function sendInput(sessionId: string): Promise<void>;
}

export default cliManager;
