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

#ifndef OHOS_ABILITY_RUNTIME_JS_CLI_MANAGER_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_CLI_MANAGER_UTILS_H

#include <vector>
#include "cli_tool_manager_data.h"
#include "napi/native_api.h"

namespace OHOS {
namespace CliTool {

napi_value CreateJsToolInfo(napi_env env, const ToolInfo &tool);
napi_value CreateJsToolInfoArray(napi_env env, const std::vector<ToolInfo> &tools);
napi_value CreateJsToolSummary(napi_env env, const ToolSummary &summary);
napi_value CreateJsToolSummaryArray(napi_env env, const std::vector<ToolSummary> &summaries);
bool ParseJsToolInfo(napi_env env, napi_value jsTool, ToolInfo &tool);

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_CLI_MANAGER_UTILS_H
