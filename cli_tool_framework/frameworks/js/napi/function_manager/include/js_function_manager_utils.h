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

#ifndef OHOS_FUNCTION_JS_FUNCTION_MANAGER_UTILS_H
#define OHOS_FUNCTION_JS_FUNCTION_MANAGER_UTILS_H

#include "native_engine/native_engine.h"
#include "function_info.h"
#include "want_params.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief Create JavaScript FunctionInfo object.
 * @param env The N-API environment.
 * @param function The FunctionInfo structure.
 * @return Returns the JavaScript object.
 */
napi_value CreateJsFunctionInfo(napi_env env, const FunctionInfo &function);

/**
 * @brief Create JavaScript InvokeResult object from explicit fields.
 *
 * @param env The N-API environment.
 * @param resultCode The result code driving InvokeResult.success/errorCode;
 *                   the caller passes executeResult.code (application-level).
 * @param result The WantParams data returned by the execution.
 * @param message The descriptive message (may be empty).
 * @param toolCallId The trace identifier echoed to the caller; an empty value
 *                   means the field was not passed and is not set on the object.
 * @param dmSessionId The device-manager session identifier echoed to the caller;
 *                   an empty value means the field was not passed.
 * @return Returns the JavaScript object.
 */
napi_value CreateJsInvokeResult(napi_env env, int32_t resultCode,
    const std::shared_ptr<AAFwk::WantParams> &result, const std::string &message,
    const std::string &toolCallId = "", const std::string &dmSessionId = "");

/**
 * @brief Parse the optional trace identifiers from InvokeOptions.
 *
 * Reads the optional toolCallId / dmSessionId fields. An absent field leaves
 * the output string empty. A present field must be a string of [A-Za-z0-9_-]
 * with length 1~256; an empty string or any other value is invalid.
 *
 * @param env The N-API environment.
 * @param options The InvokeOptions object (may be undefined/null).
 * @param toolCallId Output for the parsed toolCallId (empty when not passed).
 * @param dmSessionId Output for the parsed dmSessionId (empty when not passed).
 * @param msg Output error message when parsing fails.
 * @return Returns true on success, false otherwise.
 */
bool UnwrapInvokeOptions(napi_env env, napi_value options,
    std::string &toolCallId, std::string &dmSessionId, std::string &msg);

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_FUNCTION_JS_FUNCTION_MANAGER_UTILS_H
