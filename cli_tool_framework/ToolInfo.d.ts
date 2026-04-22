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


/*** if arkts static */
import { RecordData } from './@ohos.base';
/*** endif */

/**
 * ToolInfo describes the basic information of a cli tool.
 *
 * @typedef ToolInfo
 * @syscap SystemCapability.Ability.AgentRuntime.Core
 * @systemapi
 * @stagemodelonly
 * @since 26.0.0 dynamic&static
 */
export interface ToolInfo {
  /**
   * The name of the CLI tool.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  readonly name: string;

  /**
   * The version of the CLI tool (format defined by provider, e.g., "1.0.0").
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  readonly version: string;

  /**
   * The description of the CLI tool.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic&static
   */
  readonly description: string;

  /**
   * The executable path of the CLI tool.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic&static
   */
  readonly executablePath: string;

  /**
   * The require permissions of the CLI tool.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic&static
   */
  readonly requirePermissions?: Array<string>;

  /**
   * The input schema of the CLI tool.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic
   */
  readonly inputSchema: Record<string, Object>;

  /**
   * The input schema of the CLI tool.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 static
   */
  readonly inputSchema: Record<string, RecordData>;

  /**
   * The output schema of the CLI tool.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic
   */
  readonly outputSchema: Record<string, Object>;

  /**
   * The output schema of the CLI tool.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 static
   */
  readonly outputSchema: Record<string, RecordData>;

  /**
   * Indicates parameter mapping rules.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic&static
   */
  readonly argMapping: ArgMapping;

  /**
   * Supported event types.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic&static
   */
  readonly eventTypes?: Array<string>;

  /**
   * Supported event types.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic&static
   */
  readonly eventTypes?: Array<string>;

  /**
   * Schemas about event.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic
   */
  readonly eventSchemas?: Record<string, Record<string, Object>>;

  /**
   * Schemas about event.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 static
   */
  readonly eventSchemas?: Record<string, Record<string, RecordData>>;

  /**
   * Timeout in seconds.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic&static
   */
  readonly timeout?: int;

  /**
   * Whether this tool has subcommand.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic&static
   */
  readonly hasSubCommand?: boolean;

  /**
   * SubCommandInfo list.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly

   * @since 26.0.0 dynamic&static
   */
  readonly subcommands?: Record<string, SubCommandInfo>;
}

/**
 * ToolInfo describes the basic summary information of a cli tool.
 *
 * @syscap SystemCapability.Ability.AgentRuntime.Core
 * @systemapi
 * @stagemodelonly
 * @since 26.0.0 dynamic&static
 */
export interface ToolSummary {
  /**
   * The name of the CLI tool.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  readonly name: string;

  /**
   * The version of the CLI tool (format defined by provider, e.g., "1.0.0").
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  readonly version: string;

  /**
   * The description of the CLI tool.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic&static
   */
  readonly description: string;
}

/**
 * Represents mapping rules for tool parameters.
 *
 * @syscap SystemCapability.Ability.AgentRuntime.Core
 * @systemapi
 * @stagemodelonly
 * @since 26.0.0 dynamic&static
 */
export interface ArgMapping {
  /**
   * Mapping type.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  readonly type: ArgMappingType;

  /**
   * separator for flag type.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  readonly separator?: string;

  /**
   * order for positional type.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  readonly order?: string;

  /**
   * Parameter template mapping table
   * The key is the parameter name, and the value is one of the following two formats:
   * 1. String template, e.g.: "--target={value}"
   * 2. Conditional object, e.g.: { "if_true": "--verbose", "if_false": "" }
   *
   * Example:
   * templates: {
   *   "target": "--target={value}",
   *   "verbose": { "if_true": "-v", "if_false": "" }
   * }
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic
   */
  readonly templates?: Record<string, Object>;

  /**
   * Parameter template mapping table
   * The key is the parameter name, and the value is one of the following two formats:
   * 1. String template, e.g.: "--target={value}"
   * 2. Conditional object, e.g.: { "if_true": "--verbose", "if_false": "" }
   *
   * Example:
   * templates: {
   *   "target": "--target={value}",
   *   "verbose": { "if_true": "-v", "if_false": "" }
   * }
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 static
   */
  readonly templates?: Record<string, RecordData>;
}

/**
   * Enum for query entity mode.
   *
   * @syscap SystemCapability.Ability.AbilityRuntime.Core
   * @StageModelOnly
   * @atomicservice
   * @since 26.0.0 dynamic&static
   */
export enum ArgMappingType {
    /**
     * flag pattern.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    FLAG = 'flag',

    /**
     * positional pattern.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    POSITIONAL = 'positional',

    /**
     * flattened pattern.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    FLATTENED = 'flattened',

    /**
     * json-string pattern.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    JSONSTRING = 'jsonString',

    /**
     * mixed pattern.
     *
     * @syscap SystemCapability.Ability.AbilityRuntime.Core
     * @systemapi
     * @stagemodelonly
     * @since 26.0.0 dynamic&static
     */
    MIXED = 'mixed'
}

/**
 * Subcommand information
 *
 * @syscap SystemCapability.Ability.AgentRuntime.Core
 * @systemapi
 * @stagemodelonly
 * @since 26.0.0 dynamic&static
 */
export interface SubCommandInfo {
  /**
   * The description of the subcommand.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @since 26.0.0 dynamic&static
   */
  readonly description: string;

  /**
   * The require permissions of the subcommand.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic&static
   */
  readonly requirePermissions?: Array<string>;

  /**
   * The input schema of the subcommand.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic
   */
  readonly inputSchema: Record<string, Object>;

  /**
   * The input schema of the subcommand.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 static
   */
  readonly inputSchema: Record<string, RecordData>;

  /**
   * The output schema of the subcommand.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic
   */
  readonly outputSchema: Record<string, Object>;

  /**
   * The output schema of the subcommand.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 static
   */
  readonly outputSchema: Record<string, RecordData>;

  /**
   * Indicates parameter mapping rules for subcommand.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic&static
   */
  readonly argMapping: ArgMapping;

  /**
   * Supported event types for subcommand.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic&static
   */
  readonly eventTypes?: Array<string>;

  /**
   * Supported event types for subcommand.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic&static
   */
  readonly eventTypes?: Array<string>;

  /**
   * Schemas about event for subcommand.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 dynamic
   */
  readonly eventSchemas?: Record<string, Record<string, Object>>;

  /**
   * Schemas about event for subcommand.
   *
   * @syscap SystemCapability.Ability.AgentRuntime.Core
   * @systemapi
   * @stagemodelonly
   * @atomicservice
   * @since 26.0.0 static
   */
  readonly eventSchemas?: Record<string, Record<string, RecordData>>;
}
