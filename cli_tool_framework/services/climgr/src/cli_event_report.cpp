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

#include "cli_event_report.h"

#include <unistd.h>

#include "hisysevent_report.h"
#include "hilog_tag_wrapper.h"
#include "cli_error_code.h"
#include "string_wrapper.h"
#include "int_wrapper.h"
#include "bool_wrapper.h"
#include "want_params.h"

namespace OHOS {
namespace CliTool {

void ReportExecuteCli(const std::string& bundleName, const std::string& cliName, const std::string& args)
{
    constexpr int32_t PARAM_COUNT = 4;
    AAFwk::HisyseventReport report(PARAM_COUNT);

    report.InsertParam("BUNDLE_NAME", bundleName);
    report.InsertParam("CLI_NAME", cliName);
    report.InsertParam("ARGS", args);
    report.InsertParam("PID", static_cast<int32_t>(getpid()));

    int32_t ret = report.Report(DOMAIN_CLI_TOOL, EVENT_EXECUTE_CLI, HISYSEVENT_BEHAVIOR);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Report EXECUTE_CLI: bundle=%{public}s, cli=%{public}s, ret=%{public}d",
        bundleName.c_str(), cliName.c_str(), ret);
}

void ReportExecuteCliFailed(const std::string& bundleName, const std::string& cliName, const std::string& reason)
{
    constexpr int32_t PARAM_COUNT = 4;
    AAFwk::HisyseventReport report(PARAM_COUNT);

    report.InsertParam("BUNDLE_NAME", bundleName);
    report.InsertParam("CLI_NAME", cliName);
    report.InsertParam("REASON", reason);
    report.InsertParam("PID", static_cast<int32_t>(getpid()));

    int32_t ret = report.Report(DOMAIN_CLI_TOOL, EVENT_EXECUTE_CLI_FAILED, HISYSEVENT_FAULT);
    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "Report EXECUTE_CLI_FAILED: bundle=%{public}s, cli=%{public}s, reason=%{public}s, ret=%{public}d",
        bundleName.c_str(), cliName.c_str(), reason.c_str(), ret);
}

void ReportCliRunTimeout(const std::string& bundleName, const std::string& cliName, int64_t durationMs)
{
    constexpr int32_t PARAM_COUNT = 4;
    AAFwk::HisyseventReport report(PARAM_COUNT);

    report.InsertParam("BUNDLE_NAME", bundleName);
    report.InsertParam("CLI_NAME", cliName);
    report.InsertParam("DURATION_MS", durationMs);
    report.InsertParam("PID", static_cast<int32_t>(getpid()));

    int32_t ret = report.Report(DOMAIN_CLI_TOOL, EVENT_CLI_RUN_TIMEOUT, HISYSEVENT_FAULT);
    TAG_LOGI(AAFwkTag::CLI_TOOL,
        "Report CLI_RUN_TIMEOUT: bundle=%{public}s, cli=%{public}s, duration=%{public}lld, ret=%{public}d",
        bundleName.c_str(), cliName.c_str(), durationMs, ret);
}

void ReportCliRunError(const std::string& cliName, int32_t signalNum)
{
    constexpr int32_t PARAM_COUNT = 3;
    AAFwk::HisyseventReport report(PARAM_COUNT);

    report.InsertParam("CLI_NAME", cliName);
    report.InsertParam("SIGNAL_NUM", signalNum);
    report.InsertParam("PID", static_cast<int32_t>(getpid()));

    int32_t ret = report.Report(DOMAIN_CLI_TOOL, EVENT_CLI_RUN_ERROR, HISYSEVENT_FAULT);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Report CLI_RUN_ERROR: cli=%{public}s, signal=%{public}d, ret=%{public}d",
        cliName.c_str(), signalNum, ret);
}

std::string GetFailureReason(int32_t errorCode)
{
    switch (errorCode) {
        case ERR_PERMISSION_DENIED:
            return REASON_PERMISSION_DENIED;
        case ERR_TOOL_NOT_EXIST:
            return REASON_TOOL_NOT_EXIST;
        case ERR_SESSION_LIMIT_EXCEEDED:
            return REASON_SESSION_LIMIT_EXCEEDED;
        case ERR_NO_INIT:
            return REASON_PROCESS_CREATE_FAILED;
        case ERR_INVALID_PARAM:
        case ERR_INNER_PARAM_INVALID:
            return REASON_INVALID_PARAM;
        case ERR_NOT_HAP:
            return REASON_NOT_HAP;
        case ERR_NOT_SYSTEM_APP:
            return REASON_NOT_SYSTEM_APP;
        default:
            return REASON_UNKNOWN;
    }
}

std::string FormatWantParamsToString(const AAFwk::WantParams& args)
{
    std::string result;
    for (const auto& [key, value] : args.GetParams()) {
        if (value == nullptr) {
            continue;
        }
        result += key + "=";

        // Try String
        if (auto strObj = AAFwk::IString::Query(value)) {
            std::string strValue;
            if (strObj->GetString(strValue) == ERR_OK) {
                result += strValue + " ";
                continue;
            }
        }

        // Try Boolean
        if (auto boolObj = AAFwk::IBoolean::Query(value)) {
            bool boolValue = false;
            if (boolObj->GetValue(boolValue) == ERR_OK) {
                result += (boolValue ? std::string("true") : std::string("false")) + " ";
                continue;
            }
        }

        // Try Integer
        if (auto intObj = AAFwk::IInteger::Query(value)) {
            int intValue = 0;
            if (intObj->GetValue(intValue) == ERR_OK) {
                result += std::to_string(intValue) + " ";
                continue;
            }
        }

        // Try Long
        if (auto longObj = AAFwk::ILong::Query(value)) {
            long longValue = 0;
            if (longObj->GetValue(longValue) == ERR_OK) {
                result += std::to_string(longValue) + " ";
                continue;
            }
        }

        // Try Double
        if (auto doubleObj = AAFwk::IDouble::Query(value)) {
            double doubleValue = 0.0;
            if (doubleObj->GetValue(doubleValue) == ERR_OK) {
                result += std::to_string(doubleValue) + " ";
                continue;
            }
        }

        // Default: empty string
        result += " ";
    }
    return result;
}

} // namespace CliTool
} // namespace OHOS
