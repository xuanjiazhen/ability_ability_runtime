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

#ifndef OHOS_ABILITY_RUNTIME_CLI_MANAGER_ERRORS_H
#define OHOS_ABILITY_RUNTIME_CLI_MANAGER_ERRORS_H

#include <map>
#include "errors.h"

namespace OHOS {
namespace AbilityRuntime {

// Module type for CLI Manager
enum {
    /**
     * Module type: CLI Manager Service side
     */
    CLI_MANAGER_MODULE_TYPE_SERVICE = 0,
};

// Offset of CLI Manager error
constexpr ErrCode CLI_MANAGER_SERVICE_ERR_OFFSET = ErrCodeOffset(SUBSYS_ABILITY_RUNTIME, CLI_MANAGER_MODULE_TYPE_SERVICE);

enum {
    /**
     * Result(2100000) Base error code for CLI Manager
     */
    CLI_MANAGER_ERR_BASE = CLI_MANAGER_SERVICE_ERR_OFFSET,

    /**
     * Result(2100001) Tool not found or invalid
     * The specified tool name does not exist in the registered tools list
     */
    ERR_CLI_TOOL_NOT_FOUND = CLI_MANAGER_ERR_BASE,

    /**
     * Result(2100002) Failed to create stdout pipe
     * System pipe creation failed for stdout redirection
     */
    ERR_CLI_CREATE_STDOUT_PIPE_FAILED,

    /**
     * Result(2100003) Failed to create stderr pipe
     * System pipe creation failed for stderr redirection
     */
    ERR_CLI_CREATE_STDERR_PIPE_FAILED,

    /**
     * Result(2100004) Failed to fork process
     * Unable to create child process for tool execution
     */
    ERR_CLI_FORK_FAILED,

    /**
     * Result(2100005) Failed to wait for child process
     * waitpid system call failed
     */
    ERR_CLI_WAITPID_FAILED,

    /**
     * Result(2100006) CLI Manager service not initialized
     * CliToolManager instance is null
     */
    ERR_CLI_MANAGER_NOT_INITIALIZED,

    /**
     * Result(2100007) Invalid tool name
     * Tool name is empty or contains invalid characters
     */
    ERR_CLI_INVALID_TOOL_NAME,

    /**
     * Result(2100008) Failed to allocate memory
     * Memory allocation failed for ExecResult or other structures
     */
    ERR_CLI_MEMORY_ALLOC_FAILED,

    /**
     * Result(2100009) Tool execution timeout
     * Tool execution exceeded the specified timeout limit
     */
    ERR_CLI_EXEC_TIMEOUT,

    /**
     * Result(2100010) Tool execution interrupted
     * Tool execution was interrupted by signal
     */
    ERR_CLI_EXEC_INTERRUPTED,

    /**
     * Result(2100011) Invalid execution options
     * Provided ExecOptions contains invalid parameters
     */
    ERR_CLI_INVALID_EXEC_OPTIONS,

    /**
     * Result(2100012) Working directory not found
     * Specified working directory does not exist
     */
    ERR_CLI_WORKING_DIR_NOT_FOUND,

    /**
     * Result(2100013) Failed to change working directory
     * chdir system call failed
     */
    ERR_CLI_CHDIR_FAILED,
};

}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CLI_MANAGER_ERRORS_H
