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

#ifndef OHOS_ABILITY_RUNTIME_CLI_EVENT_REPORT_H
#define OHOS_ABILITY_RUNTIME_CLI_EVENT_REPORT_H

#include <string>

namespace OHOS {
namespace AAFwk {
class WantParams;
}

namespace CliTool {

// Event names
constexpr const char* EVENT_EXECUTE_CLI = "EXECUTE_CLI";
constexpr const char* EVENT_EXECUTE_CLI_FAILED = "EXECUTE_CLI_FAILED";
constexpr const char* EVENT_CLI_RUN_TIMEOUT = "CLI_RUN_TIMEOUT";
constexpr const char* EVENT_CLI_RUN_ERROR = "CLI_RUN_ERROR";

// Event domain
constexpr const char* DOMAIN_CLI_TOOL = "AAFWK";

// Failure reasons
constexpr const char* REASON_PERMISSION_DENIED = "PERMISSION_DENIED";
constexpr const char* REASON_TOOL_NOT_EXIST = "TOOL_NOT_EXIST";
constexpr const char* REASON_SESSION_LIMIT_EXCEEDED = "SESSION_LIMIT_EXCEEDED";
constexpr const char* REASON_PROCESS_CREATE_FAILED = "PROCESS_CREATE_FAILED";
constexpr const char* REASON_INVALID_PARAM = "INVALID_PARAM";
constexpr const char* REASON_NOT_HAP = "NOT_HAP";
constexpr const char* REASON_NOT_SYSTEM_APP = "NOT_SYSTEM_APP";
constexpr const char* REASON_UNKNOWN = "UNKNOWN";

/**
 * @brief Report CLI execution event
 * @param bundleName Caller bundle name
 * @param cliName CLI tool name
 * @param args CLI arguments (optional)
 */
void ReportExecuteCli(const std::string& bundleName, const std::string& cliName, const std::string& args = "");

/**
 * @brief Report CLI execution failure event
 * @param bundleName Caller bundle name
 * @param cliName CLI tool name
 * @param reason Failure reason
 */
void ReportExecuteCliFailed(const std::string& bundleName, const std::string& cliName, const std::string& reason);

/**
 * @brief Report CLI timeout event
 * @param bundleName Caller bundle name
 * @param cliName CLI tool name
 * @param durationMs Execution duration in milliseconds
 */
void ReportCliRunTimeout(const std::string& bundleName, const std::string& cliName, int64_t durationMs);

/**
 * @brief Report CLI error exit event
 * @param cliName CLI tool name
 * @param signalNum Signal number that caused the error exit
 */
void ReportCliRunError(const std::string& cliName, int32_t signalNum);

/**
 * @brief Get failure reason string from error code
 * @param errorCode Error code
 * @return Failure reason string
 */
std::string GetFailureReason(int32_t errorCode);

/**
 * @brief Format WantParams to string for event reporting
 * @param args WantParams to format
 * @return Formatted string
 */
std::string FormatWantParamsToString(const AAFwk::WantParams& args);

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_EVENT_REPORT_H
