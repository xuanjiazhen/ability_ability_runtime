/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "js_cli_manager.h"

#include <cstdint>
#include <memory>

#include "ability_business_error.h"
#include "cli_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "js_cli_manager_utils.h"
#include "js_error_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_util.h"

namespace OHOS {
namespace AbilityRuntime {

using namespace OHOS::AppExecFwk;

namespace {
constexpr size_t ARGC_THREE = 3;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
}

class JsCliManager final {
public:
    JsCliManager() = default;
    ~JsCliManager() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "JsCliManager finalizer called");
        std::unique_ptr<JsCliManager>(static_cast<JsCliManager*>(data));
    }

    static napi_value QueryTools(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsCliManager, OnQueryTools);
    }

    static napi_value ExecTool(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsCliManager, OnExecTool);
    }

private:
    napi_value OnQueryTools(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnQueryTools called (NAPI layer), argc=%{public}zu", argc);

        HandleEscape handleEscape(env);
        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        auto tools = std::make_shared<std::vector<ToolInfo>>();

        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, tools]() {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback: calling CliManagerClient::QueryTools");
            *innerErrCode = CliManagerClient::GetInstance().QueryTools(*tools);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback: QueryTools returned errCode=%{public}d, tools.size()=%{public}zu",
                *innerErrCode, tools->size());
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrCode, tools](napi_env env, NapiAsyncTask &task, int32_t status) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback: errCode=%{public}d, tools.size()=%{public}zu",
                *innerErrCode, tools->size());
            HandleScope handleScope(env);
            if (*innerErrCode == ERR_OK) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback: Creating JS array with %{public}zu tools", tools->size());
                napi_value jsTools = CreateJsToolInfoArray(env, *tools);
                task.ResolveWithNoError(env, jsTools);
                TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback: Resolved promise successfully");
            } else {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "CompleteCallback: Rejecting promise with errCode=%{public}d", *innerErrCode);
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };

        // Get last parameter (callback)
        napi_value lastParam = (argc > 0) ? argv[argc - 1] : nullptr;
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnQueryTools: lastParam=%{public}s", lastParam ? "not null" : "null");

        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsCliManager::OnQueryTools", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnQueryTools: Async task scheduled");
        return handleEscape.Escape(result);
    }

    napi_value OnExecTool(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool called (NAPI layer), argc=%{public}zu", argc);

        HandleEscape handleEscape(env);

        if (argc < ARGC_THREE) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid argc, requires 3 arguments (name, args, options)");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        // Parse tool name (argv[0])
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 1 - parsing tool name from argv[0]");
        std::string name;
        if (!UnwrapStringFromJS2(env, argv[INDEX_ZERO], name)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to parse tool name from argv[0]");
            ThrowInvalidParamError(env, "Tool name is required");
            return CreateJsUndefined(env);
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 1.1 - tool name parsed: '%{public}s'", name.c_str());

        if (name.empty()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Tool name is empty");
            ThrowInvalidParamError(env, "Tool name is required");
            return CreateJsUndefined(env);
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 1.2 - tool name validated: '%{public}s'", name.c_str());

        // Parse args (argv[1]) - plain JS object, not array
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 2 - parsing args from argv[1]");
        std::vector<std::pair<std::string, std::string>> args;
        napi_value jsArgs = argv[INDEX_ONE];
        if (jsArgs != nullptr) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 2.1 - jsArgs is not null, calling UnwrapStringMap");
            UnwrapStringMap(env, jsArgs, args);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 2.2 - parsed args, count=%{public}zu", args.size());
            for (const auto &[key, value] : args) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool:   arg[%{public}s]='%{public}s'",
                    key.c_str(), value.c_str());
            }
        } else {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 2.1 - jsArgs is null");
        }

        // Parse options object (argv[2])
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 3 - parsing options from argv[2]");
        napi_value jsOptions = argv[INDEX_TWO];
        ExecOptions options;
        if (jsOptions != nullptr) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 3.1 - jsOptions is not null");
            TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 3.2 - calling UnwrapExecOptions");
            UnwrapExecOptions(env, jsOptions, options);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 3.3 - parsed options: background=%{public}d, yieldMs=%{public}d, timeoutMs=%{public}d",
                options.background, options.yieldMs, options.timeoutMs);
        } else {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 3.1 - jsOptions is null");
        }

        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        auto session = std::make_shared<SessionInfo>();

        // 预先初始化 SessionInfo 的基本信息，以便在失败时也能返回有效的 sessionId
        auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        session->sessionId = std::to_string(std::hash<std::string>{}(
            name + std::to_string(nowMs)));
        session->toolName = name;
        session->status = "running";
        session->startTime = nowMs;
        session->endTime = 0;
        session->result = nullptr;

        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 4 - creating async callbacks");
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 4.1 - captured sessionId='%{public}s', name='%{public}s', args.count=%{public}zu",
            session->sessionId.c_str(), name.c_str(), args.size());

        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, name, args, options, session]() {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback(ExecTool): starting, name='%{public}s'", name.c_str());
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback(ExecTool): args count=%{public}zu", args.size());
            for (const auto &[key, value] : args) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback(ExecTool):   arg[%{public}s]='%{public}s'",
                    key.c_str(), value.c_str());
            }
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback(ExecTool): calling CliManagerClient::ExecTool");
            *innerErrCode = CliManagerClient::GetInstance().ExecTool(name, args, options, *session);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback(ExecTool): returned errCode=%{public}d", *innerErrCode);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback(ExecTool): session.status='%{public}s', sessionId='%{public}s'",
                session->status.c_str(), session->sessionId.c_str());
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrCode, session](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "========== CompleteCallback(ExecTool) START ==========");
            TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback(ExecTool): errCode=%{public}d", *innerErrCode);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback(ExecTool): session.status='%{public}s', sessionId='%{public}s'",
                session->status.c_str(), session->sessionId.c_str());

            HandleScope handleScope(env);

            // 如果 IPC 调用失败，设置 session 状态为 failed
            if (*innerErrCode != ERR_OK) {
                session->status = "failed";
                // 如果还没有结束时间，设置当前时间
                if (session->endTime == 0) {
                    session->endTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::steady_clock::now().time_since_epoch()).count();
                }
                TAG_LOGW(AAFwkTag::ABILITYMGR, "CompleteCallback(ExecTool): IPC failed, setting status to 'failed'");
            }

            // 总是返回 SessionInfo，通过 status 字段判断成功/失败
            TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback(ExecTool): Creating JS session object for promise");
            napi_value jsSession = CreateJsSessionInfo(env, *session);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback(ExecTool): Calling ResolveWithNoError");
            task.ResolveWithNoError(env, jsSession);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "========== CompleteCallback(ExecTool) RESOLVED ==========");
            TAG_LOGI(AAFwkTag::ABILITYMGR, "========== CompleteCallback(ExecTool) END ==========");
        };

        napi_value asyncResult = nullptr;
        NapiAsyncTask::Schedule("JsCliManager::OnExecTool", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: Async task scheduled");
        return handleEscape.Escape(asyncResult);
    }
};

napi_value JsCliManagerInit(napi_env env, napi_value exports)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "JsCliManagerInit called");

    // Define CLIManager class
    napi_value cliManagerClass = nullptr;
    napi_status status = napi_define_class(
        env,
        "CLIManager",
        NAPI_AUTO_LENGTH,
        [](napi_env env, napi_callback_info info) -> napi_value {
            napi_throw_error(env, nullptr, "CLIManager is a static class and cannot be instantiated");
            return nullptr;
        },
        nullptr,
        0,
        nullptr,
        &cliManagerClass
    );

    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to define CLIManager class: %{public}d", status);
        return nullptr;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "CLIManager class defined successfully");

    // Attach static methods to the class (constructor function)
    napi_property_descriptor staticMethods[] = {
        { "queryTools", nullptr, JsCliManager::QueryTools, nullptr, nullptr, nullptr, napi_default, nullptr },
        { "execTool", nullptr, JsCliManager::ExecTool, nullptr, nullptr, nullptr, napi_default, nullptr },
    };

    status = napi_define_properties(
        env, cliManagerClass, sizeof(staticMethods) / sizeof(staticMethods[0]), staticMethods);

    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to define static methods: %{public}d", status);
        return nullptr;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "Static methods attached successfully");

    // Export CLIManager as property of exports
    napi_property_descriptor cliNamespaceDesc[] = {
        { "CLIManager", nullptr, nullptr, nullptr, nullptr, cliManagerClass, napi_default, nullptr },
    };

    status = napi_define_properties(
        env, exports, sizeof(cliNamespaceDesc) / sizeof(cliNamespaceDesc[0]), cliNamespaceDesc);

    if (status != napi_ok) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to define CLI namespace properties: %{public}d", status);
        return nullptr;
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR, "JsCliManagerInit end - exported CLI.CLIManager successfully");
    return exports;
}

}  // namespace AbilityRuntime
}  // namespace OHOS
