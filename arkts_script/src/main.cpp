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

#include <iostream>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <cstdlib>

namespace OHOS {
namespace ArktsScript {

// 用于同步等待异步结果
struct ExecutionContext {
    std::atomic<bool> completed{false};
    std::atomic<bool> isPromise{false};
    std::string result;
    std::string error;
    std::mutex mutex;
    std::condition_variable cv;
};

} // namespace ArktsScript
} // namespace OHOS

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

    TAG_LOGI(AAFwkTag::APPKIT, "Calling function: %{public}s", args.funName.c_str());

    // 执行上下文
    ExecutionContext execContext;

    // 执行函数，传递 params map
    bool isAsync = ExecuteFunction(env, args.funName, args.params,
        [&execContext](bool success, const std::string& result, const std::string& error) {
            std::lock_guard<std::mutex> lock(execContext.mutex);
            if (success) {
                execContext.result = result;
            } else {
                execContext.error = error;
            }
            execContext.completed.store(true);
            execContext.cv.notify_one();
        });
    
    if (!isAsync) {
        // 同步执行失败（不是 Promise 的情况）
        // 错误已经在回调中设置
        std::lock_guard<std::mutex> lock(execContext.mutex);
        if (!execContext.error.empty()) {
            OutputError(execContext.error, "EXECUTION_ERROR");
            return EXIT_FAILURE;
        }
    }

    // 等待执行完成（对于 Promise，需要等待异步回调）
    {
        std::unique_lock<std::mutex> lock(execContext.mutex);
        // 使用超时机制，避免无限等待
        auto status = execContext.cv.wait_for(lock, std::chrono::minutes(5), [&execContext]() {
            return execContext.completed.load();
        });

        if (!status) {
            OutputError("execution timeout", "TIMEOUT_ERROR");
            return EXIT_FAILURE;
        }
    }

    // 检查执行结果
    if (!execContext.error.empty()) {
        OutputError(execContext.error, "EXECUTION_ERROR");
        return EXIT_FAILURE;
    }

    // 输出结果
    OutputResult(execContext.result);

    TAG_LOGI(AAFwkTag::APPKIT, "arktsScript runner completed");
    return EXIT_SUCCESS;
}
