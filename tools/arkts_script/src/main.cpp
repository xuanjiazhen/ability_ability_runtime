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

struct ExecutionContext {
    std::atomic<bool> completed{false};
    std::atomic<bool> isPromise{false};
    std::atomic<bool> timedOut{false};
    std::string result;
    std::string error;
};

} // namespace ArktsScript
} // namespace OHOS

int main(int argc, char* argv[])
{
    using namespace OHOS::ArktsScript;
    using OHOS::AppExecFwk::EventRunner;

    constexpr int64_t EXECUTION_TIMEOUT_MS = 5 * 60 * 1000;
    constexpr const char* EXECUTION_TIMEOUT_TASK = "arktsScript_execution_timeout";

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

    TAG_LOGI(AAFwkTag::APPKIT, "Creating script context");
    auto context = CreateScriptContext();
    if (context == nullptr) {
        OutputError("failed to create script context", "CONTEXT_ERROR");
        return EXIT_FAILURE;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "Script context created successfully");

    TAG_LOGI(AAFwkTag::APPKIT, "Creating lite JsRuntime");
    auto runtime = CreateScriptJsRuntime(context);
    if (runtime == nullptr) {
        OutputError("failed to create JsRuntime", "RUNTIME_ERROR");
        return EXIT_FAILURE;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "JsRuntime created successfully");

    napi_env env = runtime->GetNapiEnv();
    if (env == nullptr) {
        OutputError("failed to get napi_env", "ENV_ERROR");
        return EXIT_FAILURE;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "Binding script context to globalThis");
    if (!BindContextToGlobal(env, context)) {
        OutputError("failed to bind scriptContext to globalThis", "BIND_ERROR");
        return EXIT_FAILURE;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "Script context bound successfully");

    TAG_LOGI(AAFwkTag::APPKIT, "Loading ABC file: %{private}s", args.path.c_str());
    if (!LoadAbcFile(runtime.get(), args.path)) {
        OutputError("failed to load ABC file: " + args.path, "LOAD_ERROR");
        return EXIT_FAILURE;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "ABC file loaded successfully");

    auto runner = EventRunner::GetMainEventRunner();
    if (runner == nullptr) {
        OutputError("failed to get main event runner", "RUNNER_ERROR");
        return EXIT_FAILURE;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "Calling function: %{public}s", args.funName.c_str());

    ExecutionContext execContext;
    bool isAsync = ExecuteFunction(env, args.funName, args.params,
        [&execContext, runner](bool success, const std::string& result, const std::string& error) {
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
        });
    execContext.isPromise.store(isAsync);

    if (!isAsync) {
        if (!execContext.error.empty()) {
            OutputError(execContext.error, "EXECUTION_ERROR");
            return EXIT_FAILURE;
        }
    } else if (!execContext.completed.load()) {
        TAG_LOGI(AAFwkTag::APPKIT, "Promise callbacks registered, starting event runner pump");
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
            TAG_LOGW(AAFwkTag::APPKIT, "Event runner exited before Promise completion");
            OutputError("event runner exited before Promise completion", "RUNNER_ERROR");
            return EXIT_FAILURE;
        }
    }

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

    TAG_LOGI(AAFwkTag::APPKIT, "arktsScript runner completed");
    return EXIT_SUCCESS;
}
