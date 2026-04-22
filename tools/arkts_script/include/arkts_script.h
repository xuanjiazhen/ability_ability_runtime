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
#include <vector>
#include <functional>
#include <atomic>
#include <mutex>

#include "context_impl.h"
#include "js_runtime.h"
#include "napi/native_api.h"

namespace OHOS {
namespace ArktsScript {

using ContextImpl = OHOS::AbilityRuntime::ContextImpl;
using JsRuntime = OHOS::AbilityRuntime::JsRuntime;

struct ScriptArgs {
    std::string abcPath;
    std::string scriptName;
    std::string funName;
    std::vector<std::string> arguments;
    bool showHelp = false;

    void Print() const;
};

struct ExecutionContext {
    std::atomic<bool> completed{false};
    std::mutex mutex;
    std::string result;
    std::string error;
    std::string errorType;
};

using ResultCallback = std::function<void(bool success, const std::string& result,
    const std::string& error, const std::string& errorType)>;

bool ParseArguments(int argc, char* argv[], ScriptArgs& args);

std::shared_ptr<ContextImpl> CreateScriptContext();

JsRuntime* CreateJsRuntime(const std::shared_ptr<ContextImpl>& context);

bool LoadAbcFile(JsRuntime* runtime, const std::string& path);

bool BindContextToGlobal(napi_env env, const std::shared_ptr<ContextImpl>& context);

bool BindCompleteArkTsTool(napi_env env, const ResultCallback& callback);

bool ResolveFunction(napi_env env, const std::string& scriptName, const std::string& funName,
    napi_value& receiver, napi_value& func);

napi_value ConvertArgumentsToNapi(napi_env env, const std::vector<std::string>& arguments);

bool HandlePromiseResult(napi_env env, napi_value promise, const ResultCallback& callback);

std::string NapiValueToJsonString(napi_env env, napi_value value);

void OutputResult(const std::string& result);
void OutputError(const std::string& error, const std::string& errorType = "EXECUTION_ERROR");

} // namespace ArktsScript
} // namespace OHOS

#endif // ARKTSSCRIPT_H
