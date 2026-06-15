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

#include <map>

#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"
#include "napi_common_util.h"

namespace OHOS {
namespace CliTool {
namespace {
static const std::map<CliManagerErrorCode, std::string> ERROR_MSG_MAP = {
    {CliManagerErrorCode::ERROR_PERMISSION_DENIED, "Permission denied."},
    {CliManagerErrorCode::ERROR_NOT_SYSTEM_APP, "Not system application."},
    {CliManagerErrorCode::ERROR_INVALID_PARAM, "Invalid input parameter."},
    {CliManagerErrorCode::ERROR_TOOL_NOT_FOUND, "The tool does not exist."},
    {CliManagerErrorCode::ERROR_REACH_LIMIT, "Maximum number of processes has been reached."},
    {CliManagerErrorCode::ERROR_SESSION_NOT_FOUND, "The session not found."},
    {CliManagerErrorCode::ERROR_SEND_MESSAGE, "Failed to send message to the CLI session."},
    {CliManagerErrorCode::ERROR_INNER, "Internal error."},
};

static const std::map<int32_t, std::string> NATIVE_ERROR_MSG_MAP = {
    {GET_CLI_TOOL_MGR_SERVICE_FAILED,
        "Internal error. Failed to connect to the CLI tool manager service. Try again after the service is available."},
    {ERR_NO_INIT,
        "Internal error. A required CLI runtime component is not initialized or is unavailable. Try again later."},
    {ERR_NOT_HAP,
        "Internal error. Failed to build sandbox configuration. Ensure the caller is a HAP application."},
    {ERR_NOT_KILL, "Internal error. Failed to terminate the CLI tool process."},
    {ERR_JSON_PARSE_FAILED,
        "Internal error. Failed to parse CLI tool metadata JSON. Check the tool configuration JSON format."},
    {ERR_FILE_NOT_FOUND,
        "Internal error. CLI tool configuration file or directory was not found. Check the tool installation path."},
    {ERR_KVSTORE_NOT_READY, "Internal error. CLI tool database is not ready. Try again later."},
    {ERR_KVSTORE_ERROR, "Internal error. CLI tool database operation failed. Try again later."},
    {ERR_INVALID_VALUE,
        "Internal error. Failed to decode CLI tool data returned by the service. The data may be invalid or memory "
        "allocation may have failed."},
};

static const std::map<int32_t, CliManagerErrorCode> NATIVE_TO_BUSINESS_ERROR_MAP = {
    {ERR_PERMISSION_DENIED, CliManagerErrorCode::ERROR_PERMISSION_DENIED},
    {ERR_NOT_SYSTEM_APP, CliManagerErrorCode::ERROR_NOT_SYSTEM_APP},
    {ERR_INVALID_PARAM, CliManagerErrorCode::ERROR_INVALID_PARAM},
    {ERR_TOOL_NOT_EXIST, CliManagerErrorCode::ERROR_TOOL_NOT_FOUND},
    {ERR_SESSION_LIMIT_EXCEEDED, CliManagerErrorCode::ERROR_REACH_LIMIT},
    {ERR_CLI_SESSION_NOT_FOUND, CliManagerErrorCode::ERROR_SESSION_NOT_FOUND},
    {ERR_CLI_SEND_MESSAGE, CliManagerErrorCode::ERROR_SEND_MESSAGE},
};

std::string GetErrorMsg(CliManagerErrorCode errCode, int32_t nativeErr)
{
    auto nativeIt = NATIVE_ERROR_MSG_MAP.find(nativeErr);
    if (nativeIt != NATIVE_ERROR_MSG_MAP.end()) {
        return nativeIt->second;
    }

    if (errCode == CliManagerErrorCode::ERROR_INNER) {
        return "Internal error. Operation failed with native error code " + std::to_string(nativeErr) + ".";
    }

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

std::string AppendErrorContext(const std::string &errMsg, const std::string &errMsgContext)
{
    if (errMsgContext.empty()) {
        return errMsg;
    }
    return errMsg + " " + errMsgContext;
}
}  // namespace

napi_value CreateCliManagerError(napi_env env, int32_t errCode, const std::string& errMsg)
{
    AbilityRuntime::HandleEscape handleEscape(env);
    napi_value result = nullptr;
    napi_value codeValue = AbilityRuntime::CreateJsValue(env, errCode);
    napi_value msgValue = AbilityRuntime::CreateJsValue(env, errMsg);
    napi_status status = napi_create_error(env, codeValue, msgValue, &result);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "napi_create_error returned: %{public}d", status);
    return handleEscape.Escape(result);
}

napi_value CreateCliJsErrorByNativeErr(napi_env env, int32_t nativeErr, const std::string& errMsgContext)
{
    auto businessErrCode = GetBusinessErrorCode(nativeErr);
    auto errMsg = AppendErrorContext(GetErrorMsg(businessErrCode, nativeErr), errMsgContext);
    return CreateCliManagerError(env, static_cast<int32_t>(businessErrCode), errMsg);
}

} // namespace CliTool
} // namespace OHOS
