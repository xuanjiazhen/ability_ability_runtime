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

#ifndef ARKTSSCRIPT_H
#define ARKTSSCRIPT_H

#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <functional>
#include <map>

#include "context_impl.h"
#include "js_runtime.h"
#include "napi/native_api.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace AbilityRuntime {
class ContextImpl;
} // namespace AbilityRuntime
} // namespace OHOS

namespace OHOS {
namespace ArktsScript {

using JsonValue = nlohmann::json;

struct ScriptArgs {
    std::string path;
    std::string funName;
    std::map<std::string, JsonValue> params;
    bool showHelp = false;

    void Print() const;
};

using ResultCallback = std::function<void(bool success, const std::string& result, const std::string& error)>;

bool ParseArguments(int argc, char* argv[], ScriptArgs& args);

std::unique_ptr<OHOS::AbilityRuntime::JsRuntime> CreateScriptJsRuntime(
    const std::shared_ptr<OHOS::AbilityRuntime::ContextImpl>& context);

std::shared_ptr<OHOS::AbilityRuntime::ContextImpl> CreateScriptContext();

bool BindContextToGlobal(
    napi_env env, const std::shared_ptr<OHOS::AbilityRuntime::ContextImpl>& context);

bool LoadAbcFile(OHOS::AbilityRuntime::JsRuntime* runtime, const std::string& path);

bool ExecuteFunction(napi_env env, const std::string& funName,
    const std::map<std::string, JsonValue>& params, ResultCallback callback);

std::string NapiValueToJsonString(napi_env env, napi_value value);

void OutputResult(const std::string& result);
void OutputError(const std::string& error, const std::string& errorType = "EXECUTION_ERROR");

bool ParseParamsFromJson(const std::string& jsonStr, std::map<std::string, JsonValue>& params,
    std::string& errorMessage);

} // namespace ArktsScript
} // namespace OHOS

#endif // ARKTSSCRIPT_H
