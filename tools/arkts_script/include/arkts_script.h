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

#include "js_runtime.h"
#include "napi/native_api.h"

namespace OHOS {
namespace AppExecFwk {
class EventRunner;
} // namespace AppExecFwk
} // namespace OHOS

namespace OHOS {
namespace ArktsScript {

enum class SkillType {
    APP_SKILL,
    STANDALONE_SKILL,
    UNKNOWN
};

struct ScriptArgs {
    std::string abcPath;
    std::string scriptClass;
    std::string funName;
    std::vector<std::string> arguments;
    SkillType skillType = SkillType::UNKNOWN;
    bool showHelp = false;

    void Print() const;
};

struct ExecutionContext {
    std::atomic<bool> completed{false};
    std::atomic<bool> isPromise{false};
    std::atomic<bool> timedOut{false};
    std::string result;
    std::string error;
};

using ResultCallback = std::function<void(bool success, const std::string& result, const std::string& error)>;

bool ParseArguments(int argc, char* argv[], ScriptArgs& args);

SkillType ResolveSkillType(const std::string& abcPath);

OHOS::AbilityRuntime::JsRuntime* GetExistingJsRuntime();

OHOS::AbilityRuntime::JsRuntime* CreateRuntimeForSkill(SkillType skillType);

bool LoadAbcFile(OHOS::AbilityRuntime::JsRuntime* runtime, const std::string& path);

bool BindContextToGlobal(napi_env env, OHOS::AbilityRuntime::JsRuntime* runtime);

bool BindCompleteArkTsTool(napi_env env, ResultCallback callback,
    std::shared_ptr<OHOS::AppExecFwk::EventRunner> runner);

napi_value ResolveFunctionForSkill(napi_env env, SkillType skillType,
    const std::string& scriptClass, const std::string& funName);

napi_value ConvertArgumentsToNapi(napi_env env, const std::vector<std::string>& arguments);

bool HandlePromiseResult(napi_env env, napi_value promise, ResultCallback callback,
    const std::weak_ptr<OHOS::AppExecFwk::EventRunner>& runner);

std::string NapiValueToJsonString(napi_env env, napi_value value);

void OutputResult(const std::string& result);
void OutputError(const std::string& error, const std::string& errorType = "EXECUTION_ERROR");

} // namespace ArktsScript
} // namespace OHOS

#endif // ARKTSSCRIPT_H
