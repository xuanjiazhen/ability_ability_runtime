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

#ifndef OHOS_CLI_TOOL_JS_CLI_HOOK_SCHEDULER_STUB_H
#define OHOS_CLI_TOOL_JS_CLI_HOOK_SCHEDULER_STUB_H

#include <atomic>
#include <future>
#include <memory>
#include <string>

#include "cli_hook_scheduler_stub.h"
#include "exec_cmd_param.h"
#include "exec_result.h"
#include "exec_tool_param.h"
#include "native_engine/native_engine.h"

namespace OHOS {
namespace CliTool {

/**
 * @brief IPC Stub that wraps a JS CliHook object.
 *
 * When the CliToolManagerService invokes a hook callback (BeforeCallTool etc.),
 * this Stub receives the IPC on a binder thread and synchronously dispatches
 * the call to the JS thread via napi_threadsafe_function. The JS callback
 * returns a (possibly modified) object which is parsed back to native data.
 *
 * Timeout: 500ms; on timeout the original parameter is left unchanged.
 */
class JsCliHookSchedulerStub : public CliHookSchedulerStub {
public:
    explicit JsCliHookSchedulerStub(napi_env env, napi_value jsHookObj);
    ~JsCliHookSchedulerStub() override;

    ErrCode BeforeCallTool(ExecToolParam& param) override;
    ErrCode AfterCallTool(ExecResult& execResult) override;
    ErrCode BeforeCallCmd(ExecCmdParam& param) override;
    ErrCode AfterCallCmd(ExecResult& execResult) override;

    napi_ref GetCallbackRef() const { return callbackRef_; }

private:
    struct HookCallData {
        std::string methodName;
        ExecToolParam* toolParam = nullptr;
        ExecCmdParam* cmdParam = nullptr;
        ExecResult* execResult = nullptr;
        std::shared_ptr<std::promise<void>> promise;
        std::atomic<bool> jsExecuted{false};
    };

    napi_env env_ = nullptr;
    napi_ref callbackRef_ = nullptr;
    napi_threadsafe_function tsfn_ = nullptr;
    std::atomic<bool> released_{false};

    static void CallJs(napi_env env, napi_value jsCb, void* context, void* data);
    static void Finalize(napi_env env, void* data, void* hint);

    ErrCode DispatchToJs(const std::string& method, ExecToolParam* toolParam,
        ExecCmdParam* cmdParam, ExecResult* execResult);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_CLI_TOOL_JS_CLI_HOOK_SCHEDULER_STUB_H
