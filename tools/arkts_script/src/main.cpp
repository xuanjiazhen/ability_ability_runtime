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
#include "hilog_tag_wrapper.h"

#include <atomic>
#include <cstdlib>
#include <iostream>

#include "event_runner.h"

namespace OHOS {
namespace ArktsScript {

class ExecutionContext {
public:
    ExecutionContext() = default;
    ~ExecutionContext() = default;

    bool IsCompleted() const { return completed_.load(); }
    bool IsPromise() const { return isPromise_.load(); }
    bool IsTimedOut() const { return timedOut_.load(); }

    bool SetCompleted()
    {
        bool expected = false;
        return completed_.compare_exchange_strong(expected, true);
    }

    void SetPromise(bool isPromise) { isPromise_.store(isPromise); }

    void SetTimedOut()
    {
        timedOut_.store(true);
    }

    void SetResult(const std::string& result)
    {
        result_ = result;
    }

    void SetError(const std::string& error)
    {
        error_ = error;
    }

    std::string GetResult() const { return result_; }
    std::string GetError() const { return error_; }

private:
    std::atomic<bool> completed_{false};
    std::atomic<bool> isPromise_{false};
    std::atomic<bool> timedOut_{false};
    std::string result_;
    std::string error_;
};

} // namespace ArktsScript
} // namespace OHOS

int main(int argc, char* argv[])
{
    using namespace OHOS::ArktsScript;
    using OHOS::AppExecFwk::EventRunner;

    constexpr int64_t EXECUTION_TIMEOUT_MS = 300 * 1000;
    constexpr const char* EXECUTION_TIMEOUT_TASK = "ohos_arkts_timeout";

    TAG_LOGI(AAFwkTag::APPKIT, "ohos-arkts runner start");

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

    args.skillType = ResolveSkillType(args.abcPath);
    TAG_LOGI(AAFwkTag::APPKIT, "Skill type resolved: %{public}d", static_cast<int>(args.skillType));

    JsRuntime* runtime = CreateRuntimeForSkill(args.skillType);
    if (runtime == nullptr) {
        OutputError("failed to get Runtime", "RUNTIME_ERROR");
        return EXIT_FAILURE;
    }

    std::unique_ptr<JsRuntime> runtimeOwner;
    if (args.skillType == SkillType::STANDALONE_SKILL) {
        runtimeOwner.reset(runtime);
    } else {
        TAG_LOGI(AAFwkTag::APPKIT, "App Skill mode: not releasing runtime ownership");
    }

    napi_env env = runtime->GetNapiEnv();
    if (env == nullptr) {
        OutputError("failed to get napi_env", "ENV_ERROR");
        return EXIT_FAILURE;
    }

    if (!BindContextToGlobal(env, runtime)) {
        OutputError("failed to bind context to globalThis", "BIND_ERROR");
        return EXIT_FAILURE;
    }

    auto runner = EventRunner::GetMainEventRunner();
    if (runner == nullptr) {
        OutputError("failed to get main event runner", "RUNNER_ERROR");
        return EXIT_FAILURE;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "Loading ABC file: %{private}s", args.abcPath.c_str());
    if (!LoadAbcFile(runtime, args.abcPath)) {
        OutputError("failed to load ABC file: " + args.abcPath, "LOAD_ERROR");
        return EXIT_FAILURE;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "ABC file loaded successfully");

    ExecutionContext execContext;

    if (!BindCompleteArkTsTool(env,
        [&execContext](bool success, const std::string& result, const std::string& error) {
            bool expected = false;
            if (!execContext.completed.compare_exchange_strong(expected, true)) {
                TAG_LOGW(AAFwkTag::APPKIT, "Duplicate completeArkTsTool callback, ignoring");
                return;
            }
            if (success) {
                execContext.result = result;
            } else {
                execContext.error = error;
            }
        }, runner)) {
        OutputError("failed to bind completeArkTsTool", "BIND_ERROR");
        return EXIT_FAILURE;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "completeArkTsTool bound to globalThis");

    TAG_LOGI(AAFwkTag::APPKIT, "Resolving function: %{public}s", args.funName.c_str());
    napi_value func = ResolveFunctionForSkill(env, args.skillType, args.scriptClass, args.funName);
    if (func == nullptr) {
        OutputError("failed to resolve function: " + args.funName, "FUNCTION_ERROR");
        return EXIT_FAILURE;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "Function resolved successfully");

    napi_value argsArray = ConvertArgumentsToNapi(env, args.arguments);
    if (argsArray == nullptr) {
        OutputError("failed to convert arguments", "ARGUMENT_ERROR");
        return EXIT_FAILURE;
    }

    napi_value global = nullptr;
    napi_get_global(env, &global);

    TAG_LOGI(AAFwkTag::APPKIT, "Calling function: %{public}s", args.funName.c_str());
    napi_value callResult = nullptr;
    napi_status status = napi_call_function(env, global, func, 1, &argsArray, &callResult);
    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::APPKIT, "napi_call_function failed: %{public}d", status);
        OutputError("failed to call function: " + args.funName + ", napi_status: " + std::to_string(status), "CALL_ERROR");
        return EXIT_FAILURE;
    }

    bool isPromise = false;
    napi_is_promise(env, callResult, &isPromise);
    execContext.isPromise.store(isPromise);
    TAG_LOGI(AAFwkTag::APPKIT, "Function return type: %{public}s", isPromise ? "Promise" : "sync");

    if (isPromise) {
        if (!HandlePromiseResult(env, callResult,
            [&execContext, runner](bool success, const std::string& result, const std::string& error) {
                bool expected = false;
                if (!execContext.completed.compare_exchange_strong(expected, true)) {
                    TAG_LOGW(AAFwkTag::APPKIT, "Duplicate Promise callback, ignoring");
                    return;
                }
                if (success) {
                    execContext.result = result;
                    TAG_LOGI(AAFwkTag::APPKIT, "Promise resolved successfully");
                } else {
                    execContext.error = error;
                    TAG_LOGW(AAFwkTag::APPKIT, "Promise rejected: %{public}s", error.c_str());
                }
                auto r = runner.lock();
                if (r) {
                    r->Stop();
                }
            }, runner)) {
            OutputError("failed to handle Promise result", "PROMISE_ERROR");
            return EXIT_FAILURE;
        }
    } else {
        execContext.completed.store(true);
        execContext.result = NapiValueToJsonString(env, callResult);
    }

    if (!execContext.completed.load()) {
        TAG_LOGI(AAFwkTag::APPKIT, "Starting EventRunner pump, timeout=%{public}lld ms",
            static_cast<long long>(EXECUTION_TIMEOUT_MS));

        runtime->PostTask([&execContext, runner]() {
            bool expected = false;
            if (!execContext.completed.compare_exchange_strong(expected, true)) {
                TAG_LOGI(AAFwkTag::APPKIT, "Timeout task skipped: execution already completed");
                return;
            }

            execContext.timedOut.store(true);
            execContext.error = "execution timeout";
            TAG_LOGW(AAFwkTag::APPKIT, "Execution timed out, stopping EventRunner");
            auto r = runner.lock();
            if (r) {
                r->Stop();
            }
        }, EXECUTION_TIMEOUT_TASK, EXECUTION_TIMEOUT_MS);

        runner->Run();

        // 清理超时任务：如果已完成，移除任务；如果超时，任务已执行无需移除
        if (!execContext.timedOut.load()) {
            runtime->RemoveTask(EXECUTION_TIMEOUT_TASK);
            TAG_LOGI(AAFwkTag::APPKIT, "Timeout task removed: execution completed before timeout");
        }

        TAG_LOGI(AAFwkTag::APPKIT,
            "EventRunner pump finished, completed=%{public}d, timedOut=%{public}d",
            execContext.completed.load(), execContext.timedOut.load());
    } else {
        // 同步执行已完成，无需调度超时任务
        TAG_LOGI(AAFwkTag::APPKIT, "Synchronous execution completed, no timeout task scheduled");
    }

    if (execContext.timedOut.load()) {
        OutputError("execution timeout", "TIMEOUT_ERROR");
        return EXIT_FAILURE;
    }

    if (!execContext.error.empty()) {
        OutputError(execContext.error, "EXECUTION_ERROR");
        return EXIT_FAILURE;
    }

    OutputResult(execContext.result);

    TAG_LOGI(AAFwkTag::APPKIT, "ohos-arkts runner completed successfully");
    return EXIT_SUCCESS;
}
