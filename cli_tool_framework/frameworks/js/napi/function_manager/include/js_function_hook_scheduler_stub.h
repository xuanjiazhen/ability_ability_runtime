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

#ifndef OHOS_CLI_TOOL_JS_FUNCTION_HOOK_SCHEDULER_STUB_H
#define OHOS_CLI_TOOL_JS_FUNCTION_HOOK_SCHEDULER_STUB_H

#include <atomic>
#include <future>
#include <memory>
#include <string>

#include "function_hook_scheduler_stub.h"
#include "invoke_function_result.h"
#include "native_engine/native_engine.h"
#include "want_params.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief IPC Stub that wraps a JS FunctionHook object.
 *
 * When the CliToolManagerService dispatches a function hook callback
 * (BeforeInvokeFunction / AfterInvokeFunction), this Stub receives the IPC
 * on a binder thread and synchronously dispatches the call to the JS thread
 * via napi_threadsafe_function.
 *
 * Timeout: 500ms; on timeout the original parameter is left unchanged.
 */
class JsFunctionHookSchedulerStub : public FunctionHookSchedulerStub {
public:
    explicit JsFunctionHookSchedulerStub(napi_env env, napi_value jsHookObj);
    ~JsFunctionHookSchedulerStub() override;

    ErrCode BeforeInvokeFunction(
        const std::string& funcNamespace, const std::string& functionName,
        AAFwk::WantParams& param) override;

    ErrCode AfterInvokeFunction(
        const std::string& funcNamespace, const std::string& functionName,
        InvokeFunctionResult& invokeResult) override;

private:
    struct FunctionHookCallData {
        std::string methodName;
        std::string funcNamespace;
        std::string functionName;
        AAFwk::WantParams* param = nullptr;
        InvokeFunctionResult* invokeResult = nullptr;
        std::shared_ptr<std::promise<void>> promise;
    };

    napi_env env_ = nullptr;
    napi_ref callbackRef_ = nullptr;
    napi_threadsafe_function tsfn_ = nullptr;
    std::atomic<bool> released_{false};

    static void CallJs(napi_env env, napi_value jsCb, void* context, void* data);
    static void Finalize(napi_env env, void* data, void* hint);

    ErrCode DispatchToJs(const std::string& method, const std::string& funcNamespace,
        const std::string& functionName, AAFwk::WantParams* param,
        InvokeFunctionResult* invokeResult);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_JS_FUNCTION_HOOK_SCHEDULER_STUB_H
