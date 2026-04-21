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

#include "arkts_script.h"
#include "skill_resolver.h"
#include "hilog_tag_wrapper.h"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>

#include "ability_manager_client.h"
#include "event_runner.h"
#include "skill/skill_execute_param.h"

namespace OHOS {
namespace ArktsScript {

struct ExecutionContext {
    std::atomic<bool> completed{false};
    std::atomic<bool> isPromise{false};
    std::atomic<bool> timedOut{false};
    std::string result;
    std::string error;
};

} // namespace ArktsScript
} // namespace OHOS

namespace {

using namespace OHOS::ArktsScript;
using OHOS::AppExecFwk::EventRunner;

constexpr int64_t EXECUTION_TIMEOUT_MS = 5 * 60 * 1000;
constexpr const char* EXECUTION_TIMEOUT_TASK = "arktsScript_execution_timeout";

void MakeResultCallback(ExecutionContext& execContext,
    const std::shared_ptr<EventRunner>& runner, bool success,
    const std::string& result, const std::string& error)
{
    bool expected = false;
    if (!execContext.completed.compare_exchange_strong(expected, true)) {
        TAG_LOGW(AAFwkTag::APPKIT, "Execution result already finalized, ignoring duplicate callback");
        return;
    }

    if (success) {
        execContext.result = result;
        TAG_LOGI(AAFwkTag::APPKIT, "Execution callback completed successfully");
    } else {
        execContext.error = error;
        TAG_LOGW(AAFwkTag::APPKIT, "Execution callback completed with error: %{public}s", error.c_str());
    }

    if (execContext.isPromise.load()) {
        TAG_LOGI(AAFwkTag::APPKIT, "Stopping event runner after Promise completion");
        runner->Stop();
    }
}

bool ResolveSkillForArgs(const ScriptArgs& args, SkillResolutionResult& skillResult)
{
    std::string errMsg;
    if (args.mode == ExecutionMode::SKILL_BUNDLE) {
        return ResolveSkillInfo(args.bundleName, args.skillName, skillResult, errMsg);
    }
    return ResolveModuleInfo(args.bundleName, args.moduleName, skillResult, errMsg);
}

bool ResolveAndExecuteSkill(const ScriptArgs& args, const std::shared_ptr<EventRunner>& runner,
    ExecutionContext& execContext, std::unique_ptr<JsRuntime>& outRuntime)
{
    SkillResolutionResult skillResult;
    if (!ResolveSkillForArgs(args, skillResult)) {
        OutputError("failed to resolve skill/module info", "RESOLVE_ERROR");
        return false;
    }

    auto context = CreateScriptContextForBundle(args.bundleName);
    if (context == nullptr) {
        OutputError("failed to create script context for bundle: " + args.bundleName, "CONTEXT_ERROR");
        return false;
    }

    outRuntime = CreateScriptJsRuntime(context);
    if (outRuntime == nullptr) {
        OutputError("failed to create JsRuntime", "RUNTIME_ERROR");
        return false;
    }

    napi_env env = outRuntime->GetNapiEnv();
    if (env == nullptr) {
        OutputError("failed to get napi_env", "ENV_ERROR");
        return false;
    }

    if (!BindContextToGlobal(env, context)) {
        OutputError("failed to bind scriptContext to globalThis", "BIND_ERROR");
        return false;
    }

    auto cb = [&execContext, runner](bool success, const std::string& res, const std::string& err) {
        MakeResultCallback(execContext, runner, success, res, err);
    };

    bool isAsync = LoadAndExecuteModule(outRuntime.get(),
        skillResult.moduleName, skillResult.srcPath,
        skillResult.hapPath, skillResult.srcEntrance,
        skillResult.isEsModule, args.funName, args.params, cb);
    execContext.isPromise.store(isAsync);
    return true;
}

int RunDirectAbcMode(const ScriptArgs& args, const std::shared_ptr<EventRunner>& runner,
    ExecutionContext& execContext, std::unique_ptr<JsRuntime>& outRuntime)
{
    auto context = CreateScriptContext();
    if (context == nullptr) {
        OutputError("failed to create script context", "CONTEXT_ERROR");
        return EXIT_FAILURE;
    }

    outRuntime = CreateScriptJsRuntime(context);
    if (outRuntime == nullptr) {
        OutputError("failed to create JsRuntime", "RUNTIME_ERROR");
        return EXIT_FAILURE;
    }

    napi_env env = outRuntime->GetNapiEnv();
    if (env == nullptr) {
        OutputError("failed to get napi_env", "ENV_ERROR");
        return EXIT_FAILURE;
    }

    if (!BindContextToGlobal(env, context)) {
        OutputError("failed to bind scriptContext to globalThis", "BIND_ERROR");
        return EXIT_FAILURE;
    }

    if (!LoadAbcFile(outRuntime.get(), args.path)) {
        OutputError("failed to load ABC file: " + args.path, "LOAD_ERROR");
        return EXIT_FAILURE;
    }

    auto cb = [&execContext, runner](bool success, const std::string& res, const std::string& err) {
        MakeResultCallback(execContext, runner, success, res, err);
    };

    bool isAsync = ExecuteFunction(env, args.funName, args.params, cb);
    execContext.isPromise.store(isAsync);
    return EXIT_SUCCESS;
}

int RunSkillMode(const ScriptArgs& args, const std::shared_ptr<EventRunner>& runner,
    ExecutionContext& execContext, std::unique_ptr<JsRuntime>& outRuntime)
{
    if (!ResolveAndExecuteSkill(args, runner, execContext, outRuntime)) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

int RunHdcSkillMode(const ScriptArgs& args, const std::shared_ptr<EventRunner>& runner,
    ExecutionContext& execContext)
{
    using namespace OHOS::AAFwk;

    // Resolve skill info to get module loading paths
    SkillResolutionResult skillResult;
    if (!ResolveSkillForArgs(args, skillResult)) {
        OutputError("failed to resolve skill info for HDC mode", "RESOLVE_ERROR");
        return EXIT_FAILURE;
    }

    auto params = std::make_shared<WantParams>();
    uint64_t skillId = static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());

    auto client = AbilityManagerClient::GetInstance();
    if (client == nullptr) {
        OutputError("failed to get AbilityManagerClient", "CLIENT_ERROR");
        return EXIT_FAILURE;
    }

    TAG_LOGI(AAFwkTag::APPKIT,
        "HDC skill mode: bundle=%{public}s module=%{public}s skill=%{public}s",
        args.bundleName.c_str(), args.moduleName.c_str(), args.skillName.c_str());

    auto ret = client->StartSkillByHDC(
        args.bundleName, args.moduleName, args.skillName, params, skillId,
        skillResult.srcEntrance, skillResult.hapPath, skillResult.isEsModule);
    if (ret != ERR_OK) {
        OutputError("StartSkillByHDC failed with code: " + std::to_string(ret), "IPC_ERROR");
        return EXIT_FAILURE;
    }

    execContext.isPromise.store(true);
    return EXIT_SUCCESS;
}

int WaitForAsyncResult(const std::unique_ptr<JsRuntime>& runtime,
    const std::shared_ptr<EventRunner>& runner, ExecutionContext& execContext)
{
    if (!execContext.isPromise.load()) {
        if (!execContext.error.empty()) {
            OutputError(execContext.error, "EXECUTION_ERROR");
            return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
    }

    if (execContext.completed.load()) {
        return CheckExecResult(execContext);
    }

    runtime->PostTask([&execContext, runner]() {
        bool expected = false;
        if (!execContext.completed.compare_exchange_strong(expected, true)) {
            TAG_LOGI(AAFwkTag::APPKIT, "Timeout task skipped because execution already completed");
            return;
        }

        execContext.timedOut.store(true);
        execContext.error = "execution timeout";
        TAG_LOGW(AAFwkTag::APPKIT, "Execution timed out, stopping event runner");
        runner->Stop();
    }, EXECUTION_TIMEOUT_TASK, EXECUTION_TIMEOUT_MS);

    int32_t ret = runner->Run();
    runtime->RemoveTask(EXECUTION_TIMEOUT_TASK);
    TAG_LOGI(AAFwkTag::APPKIT,
        "Event runner pump finished, ret=%{public}d, completed=%{public}d, timedOut=%{public}d",
        ret, execContext.completed.load(), execContext.timedOut.load());
    if (ret != 0) {
        OutputError("event runner failed while waiting for async result", "RUNNER_ERROR");
        return EXIT_FAILURE;
    }
    if (!execContext.completed.load()) {
        OutputError("event runner exited before Promise completion", "RUNNER_ERROR");
        return EXIT_FAILURE;
    }
    return CheckExecResult(execContext);
}

int CheckExecResult(const ExecutionContext& execContext)
{
    if (execContext.timedOut.load()) {
        TAG_LOGW(AAFwkTag::APPKIT, "Returning timeout after async wait");
        OutputError("execution timeout", "TIMEOUT_ERROR");
        return EXIT_FAILURE;
    }

    if (!execContext.error.empty()) {
        OutputError(execContext.error, "EXECUTION_ERROR");
        return EXIT_FAILURE;
    }

    OutputResult(execContext.result);
    return EXIT_SUCCESS;
}

} // namespace

int main(int argc, char* argv[])
{
    using namespace OHOS::ArktsScript;

    TAG_LOGI(AAFwkTag::APPKIT, "arktsScript runner start");

    ScriptArgs args;
    if (!ParseArguments(argc, argv, args)) {
        OutputError("failed to parse arguments", "ARGUMENT_ERROR");
        return EXIT_FAILURE;
    }
    if (args.showHelp) {
        return EXIT_SUCCESS;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "Arguments parsed successfully");
    args.Print();

    auto runner = EventRunner::GetMainEventRunner();
    if (runner == nullptr) {
        OutputError("failed to get main event runner", "RUNNER_ERROR");
        return EXIT_FAILURE;
    }

    ExecutionContext execContext;
    std::unique_ptr<JsRuntime> runtime;
    int execRet = EXIT_FAILURE;

    if (args.mode == ExecutionMode::DIRECT_ABC) {
        TAG_LOGI(AAFwkTag::APPKIT, "Running in DIRECT_ABC mode");
        execRet = RunDirectAbcMode(args, runner, execContext, runtime);
    } else if (args.mode == ExecutionMode::HDC_SKILL) {
        TAG_LOGI(AAFwkTag::APPKIT, "Running in HDC_SKILL mode");
        execRet = RunHdcSkillMode(args, runner, execContext);
    } else {
        TAG_LOGI(AAFwkTag::APPKIT, "Running in SKILL/MODULE mode");
        execRet = RunSkillMode(args, runner, execContext, runtime);
    }

    if (execRet != EXIT_SUCCESS) {
        return execRet;
    }

    // HDC mode: the skill executes asynchronously in the target process.
    // The IPC call returns immediately after triggering execution.
    // Result is delivered back via StartSkillByHDCDone IPC callback.
    if (args.mode == ExecutionMode::HDC_SKILL) {
        TAG_LOGI(AAFwkTag::APPKIT, "HDC skill dispatched successfully");
        OutputResult("{\"status\":\"dispatched\"}");
        return EXIT_SUCCESS;
    }

    int result = WaitForAsyncResult(runtime, runner, execContext);

    TAG_LOGI(AAFwkTag::APPKIT, "arktsScript runner completed");
    return result;
}
