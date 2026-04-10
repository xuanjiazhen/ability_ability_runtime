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

namespace OHOS {
namespace AbilityRuntime {

enum {
    /**
     * Result(2100001) Tool not found or invalid
     * The specified tool name does not exist in the registered tools list
     */
    ERR_CLI_TOOL_NOT_FOUND = 2100001,

    /**
     * Result(2100002) Failed to create stdout pipe
     * System pipe creation failed for stdout redirection
     */
    ERR_CLI_CREATE_STDOUT_PIPE_FAILED = 2100002,

    /**
     * Result(2100003) Failed to create stderr pipe
     * System pipe creation failed for stderr redirection
     */
    ERR_CLI_CREATE_STDERR_PIPE_FAILED = 2100003,

    /**
     * Result(2100004) Failed to fork process
     * Unable to create child process for tool execution
     */
    ERR_CLI_FORK_FAILED = 2100004,

    /**
     * Result(2100005) Failed to wait for child process
     * waitpid system call failed
     */
    ERR_CLI_WAITPID_FAILED = 2100005,

    /**
     * Result(2100006) CLI Manager service not initialized
     * CliToolManager instance is null
     */
    ERR_CLI_MANAGER_NOT_INITIALIZED = 2100006,

    /**
     * Result(2100007) Invalid tool name
     * Tool name is empty or contains invalid characters
     */
    ERR_CLI_INVALID_TOOL_NAME = 2100007,

    /**
     * Result(2100008) Failed to allocate memory
     * Memory allocation failed for ExecResult or other structures
     */
    ERR_CLI_MEMORY_ALLOC_FAILED = 2100008,

    /**
     * Result(2100009) Tool execution timeout
     * Tool execution exceeded the specified timeout limit
     */
    ERR_CLI_EXEC_TIMEOUT = 2100009,

    /**
     * Result(2100010) Tool execution interrupted
     * Tool execution was interrupted by signal
     */
    ERR_CLI_EXEC_INTERRUPTED = 2100010,

    /**
     * Result(2100011) Invalid execution options
     * Provided ExecOptions contains invalid parameters
     */
    ERR_CLI_INVALID_EXEC_OPTIONS = 2100011,

    /**
     * Result(2100012) Working directory not found
     * Specified working directory does not exist
     */
    ERR_CLI_WORKING_DIR_NOT_FOUND = 2100012,

    /**
     * Result(2100013) Failed to change working directory
     * chdir system call failed
     */
    ERR_CLI_CHDIR_FAILED = 2100013,

    /**
     * Result(2100014) Too many concurrent sessions
     * Maximum number of concurrent sessions (64) has been reached
     */
    ERR_CLI_TOO_MANY_SESSIONS = 2100014,

    /**
     * Result(2100015) Session not found
     * The specified session does not exist or has already been cleaned up
     */
    ERR_CLI_SESSION_NOT_FOUND = 2100015,

    /**
     * Result(2100016) Session not running
     * The specified session cannot be cancelled because it is not active
     */
    ERR_CLI_SESSION_NOT_RUNNING = 2100016,

    /**
     * Result(2100017) Invalid scheduler registration parameters
     * RegisterScheduler called with invalid callerPid or null scheduler
     */
    ERR_CLI_INVALID_SCHEDULER_PARAMS = 2100017,

    /**
     * Result(2100018) Scheduler not found
     * No scheduler registered for the given callerPid
     */
    ERR_CLI_SCHEDULER_NOT_FOUND = 2100018,

    /**
     * Result(2100019) Invalid subscriber registration parameters
     * RegisterSubscriber called with invalid sessionId, subscriptionId, or callerPid
     */
    ERR_CLI_INVALID_SUBSCRIBER_PARAMS = 2100019,

    /**
     * Result(2100020) IO monitor registration failed
     * Failed to register session with IO monitor
     */
    ERR_CLI_IO_MONITOR_REGISTER_FAILED = 2100020,

    /**
     * Result(2100021) Process monitor registration failed
     * Failed to register session with process monitor
     */
    ERR_CLI_PROCESS_MONITOR_REGISTER_FAILED = 2100021,
};

}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CLI_MANAGER_ERRORS_H
