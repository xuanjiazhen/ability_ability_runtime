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

#ifndef OHOS_ABILITY_RUNTIME_CLI_MANAGER_ERROR_UTILS_H
#define OHOS_ABILITY_RUNTIME_CLI_MANAGER_ERROR_UTILS_H

#include <native_engine/native_engine.h>
#include <string>

namespace OHOS {
namespace AbilityRuntime {

// CLI Manager 业务错误码（遵循 OpenHarmony businessError 规范）
enum class CliManagerErrorCode {
    // 参数错误 (401)
    ERROR_INVALID_PARAM = 401,

    // CLI Manager 业务错误 (15000000+)
    ERROR_TOOL_NOT_FOUND = 15000001,
    ERROR_PIPE_FAILED = 15000002,
    ERROR_FORK_FAILED = 15000003,
    ERROR_WAITPID_FAILED = 15000004,
    ERROR_SERVICE_NOT_INITIALIZED = 15000005,
    ERROR_MEMORY_ALLOC_FAILED = 15000006,
    ERROR_EXEC_TIMEOUT = 15000007,
    ERROR_EXEC_INTERRUPTED = 15000008,
    ERROR_INVALID_OPTIONS = 15000009,
    ERROR_WORKING_DIR_NOT_FOUND = 15000010,
    ERROR_CHDIR_FAILED = 15000011,
    ERROR_TOO_MANY_SESSIONS = 15000012,
    ERROR_SESSION_NOT_FOUND = 15000013,
    ERROR_SESSION_NOT_RUNNING = 15000014,
    ERROR_SCHEDULER_REGISTRATION_FAILED = 15000015,
    ERROR_SCHEDULER_NOT_FOUND = 15000016,
    ERROR_SUBSCRIBER_REGISTRATION_FAILED = 15000017,
    ERROR_IO_MONITOR_REGISTRATION_FAILED = 15000018,
    ERROR_PROCESS_MONITOR_REGISTRATION_FAILED = 15000019,

    // 通用内部错误
    ERROR_INNER = 16000050,
};

/**
 * @brief 创建 CLI Manager BusinessError 对象
 * @param env N-API 环境
 * @param errCode 错误码
 * @param errMsg 错误消息（可选，默认使用预定义消息）
 * @return napi_value BusinessError 对象
 */
napi_value CreateCliManagerError(napi_env env, int32_t errCode, const std::string& errMsg = "");

/**
 * @brief 根据内部错误码创建 CLI Manager BusinessError 对象
 * @param env N-API 环境
 * @param nativeErr 内部错误码（ERR_CLI_*）
 * @return napi_value BusinessError 对象
 */
napi_value CreateCliJsErrorByNativeErr(napi_env env, int32_t nativeErr);

/**
 * @brief 抛出 CLI Manager BusinessError 异常
 * @param env N-API 环境
 * @param errCode 错误码
 * @param errMsg 错误消息
 */
void ThrowCliManagerError(napi_env env, int32_t errCode, const std::string& errMsg = "");

/**
 * @brief 根据内部错误码抛出 CLI Manager BusinessError 异常
 * @param env N-API 环境
 * @param nativeErr 内部错误码
 */
void ThrowCliJsErrorByNativeErr(napi_env env, int32_t nativeErr);

}  // namespace AbilityRuntime
}  // namespace OHOS

#endif  // OHOS_ABILITY_RUNTIME_CLI_MANAGER_ERROR_UTILS_H
