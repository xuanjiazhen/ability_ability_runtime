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

#ifndef OHOS_CLI_TOOL_INVOKE_FUNCTION_EXECUTOR_H
#define OHOS_CLI_TOOL_INVOKE_FUNCTION_EXECUTOR_H

#include <atomic>
#include <functional>
#include <memory>
#include <string>

#include "want_params.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief Pure-C++ outcome of an invokeFunction run.
 *
 * Carries everything the NAPI layer needs to build a JS result or reject the
 * promise. Deliberately holds NO napi types so the executor can run and be
 * tested without an N-API environment.
 */
struct InvokeFunctionResult {
    bool success = false;                              // framework resultCode == 0 (Promise decision)
    int32_t errorCode = 0;                             // native err code, used on the reject path
    int32_t resultCode = 0;                            // executeResult.code (app business level)
    std::shared_ptr<AAFwk::WantParams> result;         // business data carried into InvokeResult.data
    std::string message;                               // reserved description
};

/**
 * @brief Invoked EXACTLY ONCE on completion / failure / timeout.
 *
 * May be called from a worker thread or a binder thread, so the implementation
 * must be thread-safe (the NAPI layer bridges this through a threadsafe function).
 */
using InvokeResultCallback = std::function<void(const InvokeFunctionResult &)>;

/**
 * @class InvokeFunctionExecutor
 * @brief Pure C++ executor for invokeFunction, decoupled from NAPI.
 *
 * Owns the full business flow: function query, type validation, intent
 * execution, timeout, and race-guarded result reporting via the injected
 * callback. Does NOT depend on napi_env / napi types.
 *
 * Lifecycle: after Execute() returns the caller may drop its shared_ptr;
 * internal ffrt/binder closures keep this alive via shared_from_this until the
 * result is delivered exactly once.
 */
class InvokeFunctionExecutor
    : public std::enable_shared_from_this<InvokeFunctionExecutor> {
public:
    // Public default constructor so std::make_shared can allocate the object and
    // its control block in a single chunk; call sites should still prefer Create().
    InvokeFunctionExecutor() : completed_(std::make_shared<std::atomic<bool>>(false)) {}

    /**
     * @brief Create an executor instance.
     * @return shared_ptr to a new executor.
     */
    static std::shared_ptr<InvokeFunctionExecutor> Create();

    /**
     * @brief Kick off the async flow.
     *
     * @param bundleName Bundle name that owns the function.
     * @param functionName Function unique identifier.
     * @param wantParams Input arguments (Record -> WantParams).
     * @param callback Invoked exactly once with the outcome. Must be thread-safe.
     */
    void Execute(const std::string &bundleName, const std::string &functionName,
        const AAFwk::WantParams &wantParams, InvokeResultCallback callback);

private:
    // Step 1-3 core flow, runs on an ffrt worker thread.
    void DoExecute(const std::string &bundleName, const std::string &functionName,
        const AAFwk::WantParams &wantParams);
    // Report a failure outcome, guarded by the completed_ flag.
    void ReportError(int32_t errorCode);
    // Arm the 30s timeout task.
    void SetupTimeout();

    // Shared with InvokeFunctionCallbackClient and the timeout/failure paths so
    // they all race-guard on the same flag.
    std::shared_ptr<std::atomic<bool>> completed_;
    InvokeResultCallback callback_;
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_INVOKE_FUNCTION_EXECUTOR_H
