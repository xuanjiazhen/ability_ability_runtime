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

#include "cli_manager_data.h"
#include "native_engine/native_engine.h"
#include <utility>
#include <vector>

namespace OHOS {
namespace AbilityRuntime {

// Create JS objects from native structures
napi_value CreateJsToolSummary(napi_env env, const ToolSummary &summary);
napi_value CreateJsToolSummaryArray(napi_env env, const std::vector<ToolSummary> &summaries);
napi_value CreateJsToolInfo(napi_env env, const ToolInfo &info);
napi_value CreateJsToolInfoArray(napi_env env, const std::vector<ToolInfo> &infos);
napi_value CreateJsExecResult(napi_env env, const ExecResult &result);
napi_value CreateJsSessionInfo(napi_env env, const SessionInfo &session);
napi_value CreateJsSessionStatus(napi_env env, const SessionStatus &status);
napi_value CreateJsToolEvent(napi_env env, const ToolEvent &event);

// Unwrap JS objects to native structures
bool UnwrapExecOptions(napi_env env, napi_value jsOptions, ExecOptions &options);
bool UnwrapStringMap(napi_env env, napi_value jsMap, std::vector<std::pair<std::string, std::string>> &map);

}  // namespace AbilityRuntime
}  // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_CLI_MANAGER_UTILS_H
