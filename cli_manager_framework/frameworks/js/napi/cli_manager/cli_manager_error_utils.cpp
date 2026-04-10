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

#include "cli_manager_error_utils.h"
#include "cli_manager_errors.h"
#include "hilog_tag_wrapper.h"
#include "js_runtime_utils.h"
#include <map>

namespace OHOS {
namespace AbilityRuntime {

namespace {
static const std::map<CliManagerErrorCode, std::string> ERROR_MSG_MAP = {
    {CliManagerErrorCode::ERROR_INVALID_PARAM, "Invalid input parameter."},
    {CliManagerErrorCode::ERROR_TOOL_NOT_FOUND, "The specified tool does not exist."},
    {CliManagerErrorCode::ERROR_PIPE_FAILED, "Failed to create pipe for output redirection."},
    {CliManagerErrorCode::ERROR_FORK_FAILED, "Failed to create child process."},
    {CliManagerErrorCode::ERROR_WAITPID_FAILED, "Failed to wait for child process."},
    {CliManagerErrorCode::ERROR_SERVICE_NOT_INITIALIZED, "CLI Manager service is not initialized."},
    {CliManagerErrorCode::ERROR_MEMORY_ALLOC_FAILED, "Failed to allocate memory."},
    {CliManagerErrorCode::ERROR_EXEC_TIMEOUT, "Tool execution timed out."},
    {CliManagerErrorCode::ERROR_EXEC_INTERRUPTED, "Tool execution was interrupted."},
    {CliManagerErrorCode::ERROR_INVALID_OPTIONS, "Invalid execution options."},
    {CliManagerErrorCode::ERROR_WORKING_DIR_NOT_FOUND, "The working directory does not exist."},
    {CliManagerErrorCode::ERROR_CHDIR_FAILED, "Failed to change working directory."},
    {CliManagerErrorCode::ERROR_TOO_MANY_SESSIONS, "Too many concurrent sessions."},
    {CliManagerErrorCode::ERROR_SESSION_NOT_FOUND, "The session does not exist."},
    {CliManagerErrorCode::ERROR_SESSION_NOT_RUNNING, "The session is not running."},
    {CliManagerErrorCode::ERROR_SCHEDULER_REGISTRATION_FAILED, "Failed to register scheduler."},
    {CliManagerErrorCode::ERROR_SCHEDULER_NOT_FOUND, "The scheduler does not exist."},
    {CliManagerErrorCode::ERROR_SUBSCRIBER_REGISTRATION_FAILED, "Failed to register subscriber."},
    {CliManagerErrorCode::ERROR_IO_MONITOR_REGISTRATION_FAILED, "Failed to register IO monitor."},
    {CliManagerErrorCode::ERROR_PROCESS_MONITOR_REGISTRATION_FAILED, "Failed to register process monitor."},
    {CliManagerErrorCode::ERROR_INNER, "Internal error."},
};

static const std::map<int32_t, CliManagerErrorCode> NATIVE_TO_BUSINESS_ERROR_MAP = {
    {ERR_CLI_TOOL_NOT_FOUND, CliManagerErrorCode::ERROR_TOOL_NOT_FOUND},
    {ERR_CLI_CREATE_STDOUT_PIPE_FAILED, CliManagerErrorCode::ERROR_PIPE_FAILED},
    {ERR_CLI_CREATE_STDERR_PIPE_FAILED, CliManagerErrorCode::ERROR_PIPE_FAILED},
    {ERR_CLI_FORK_FAILED, CliManagerErrorCode::ERROR_FORK_FAILED},
    {ERR_CLI_WAITPID_FAILED, CliManagerErrorCode::ERROR_WAITPID_FAILED},
    {ERR_CLI_MANAGER_NOT_INITIALIZED, CliManagerErrorCode::ERROR_SERVICE_NOT_INITIALIZED},
    {ERR_CLI_MEMORY_ALLOC_FAILED, CliManagerErrorCode::ERROR_MEMORY_ALLOC_FAILED},
    {ERR_CLI_EXEC_TIMEOUT, CliManagerErrorCode::ERROR_EXEC_TIMEOUT},
    {ERR_CLI_EXEC_INTERRUPTED, CliManagerErrorCode::ERROR_EXEC_INTERRUPTED},
    {ERR_CLI_INVALID_EXEC_OPTIONS, CliManagerErrorCode::ERROR_INVALID_OPTIONS},
    {ERR_CLI_WORKING_DIR_NOT_FOUND, CliManagerErrorCode::ERROR_WORKING_DIR_NOT_FOUND},
    {ERR_CLI_CHDIR_FAILED, CliManagerErrorCode::ERROR_CHDIR_FAILED},
    {ERR_CLI_TOO_MANY_SESSIONS, CliManagerErrorCode::ERROR_TOO_MANY_SESSIONS},
    {ERR_CLI_SESSION_NOT_FOUND, CliManagerErrorCode::ERROR_SESSION_NOT_FOUND},
    {ERR_CLI_SESSION_NOT_RUNNING, CliManagerErrorCode::ERROR_SESSION_NOT_RUNNING},
    {ERR_CLI_INVALID_SCHEDULER_PARAMS, CliManagerErrorCode::ERROR_SCHEDULER_REGISTRATION_FAILED},
    {ERR_CLI_SCHEDULER_NOT_FOUND, CliManagerErrorCode::ERROR_SCHEDULER_NOT_FOUND},
    {ERR_CLI_INVALID_SUBSCRIBER_PARAMS, CliManagerErrorCode::ERROR_SUBSCRIBER_REGISTRATION_FAILED},
    {ERR_CLI_IO_MONITOR_REGISTER_FAILED, CliManagerErrorCode::ERROR_IO_MONITOR_REGISTRATION_FAILED},
    {ERR_CLI_PROCESS_MONITOR_REGISTER_FAILED, CliManagerErrorCode::ERROR_PROCESS_MONITOR_REGISTRATION_FAILED},
};

// 获取错误消息
std::string GetErrorMsg(CliManagerErrorCode errCode)
{
    auto it = ERROR_MSG_MAP.find(errCode);
    if (it != ERROR_MSG_MAP.end()) {
        return it->second;
    }
    return "Unknown error.";
}

CliManagerErrorCode GetBusinessErrorCode(int32_t nativeErr)
{
    auto it = NATIVE_TO_BUSINESS_ERROR_MAP.find(nativeErr);
    if (it != NATIVE_TO_BUSINESS_ERROR_MAP.end()) {
        return it->second;
    }
    return CliManagerErrorCode::ERROR_INNER;
}

}  // namespace

napi_value CreateCliManagerError(napi_env env, int32_t errCode, const std::string& errMsg)
{
    HandleEscape handleEscape(env);
    napi_value result = nullptr;
    std::string errorMsg = errMsg.empty() ?
        GetErrorMsg(static_cast<CliManagerErrorCode>(errCode)) : errMsg;

    TAG_LOGI(AAFwkTag::ABILITYMGR, "[CreateCliManagerError] errCode=%{public}d, errorMsg='%{public}s'",
        errCode, errorMsg.c_str());

    napi_value codeValue = CreateJsValue(env, errCode);
    napi_value msgValue = CreateJsValue(env, errorMsg);

    napi_status status = napi_create_error(env, codeValue, msgValue, &result);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "[CreateCliManagerError] napi_create_error failed: %{public}d", status);
        napi_create_object(env, &result);
        napi_set_named_property(env, result, "code", codeValue);
        napi_set_named_property(env, result, "msg", msgValue);
    } else {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "[CreateCliManagerError] napi_create_error succeeded");
    }

    return handleEscape.Escape(result);
}

napi_value CreateCliJsErrorByNativeErr(napi_env env, int32_t nativeErr)
{
    auto businessErrCode = GetBusinessErrorCode(nativeErr);
    auto errMsg = GetErrorMsg(businessErrCode);
    return CreateCliManagerError(env, static_cast<int32_t>(businessErrCode), errMsg);
}

void ThrowCliManagerError(napi_env env, int32_t errCode, const std::string& errMsg)
{
    napi_throw(env, CreateCliManagerError(env, errCode, errMsg));
}

void ThrowCliJsErrorByNativeErr(napi_env env, int32_t nativeErr)
{
    napi_throw(env, CreateCliJsErrorByNativeErr(env, nativeErr));
}

}  // namespace AbilityRuntime
}  // namespace OHOS
