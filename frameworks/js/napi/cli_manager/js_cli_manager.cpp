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
        std::map<std::string, std::string> args;
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
            napi_value jsExecOptions = nullptr;
            napi_get_named_property(env, jsOptions, "options", &jsExecOptions);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 3.2 - calling UnwrapExecOptions");
            UnwrapExecOptions(env, jsExecOptions, options);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 3.3 - parsed options: mode='%{public}s', timeout=%{public}d",
                options.mode.c_str(), options.timeout);
        } else {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 3.1 - jsOptions is null");
        }

        // Extract callback and create napi_ref for lifecycle management
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 4 - extracting callback from argv[2]");
        napi_value callback = nullptr;
        if (argc >= ARGC_THREE && argv[INDEX_TWO] != nullptr) {
            napi_get_named_property(env, argv[INDEX_TWO], "callback", &callback);
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: callback=%{public}s", callback ? "not null" : "null");

        // Create napi_ref to manage callback lifetime
        auto callbackRef = std::make_shared<napi_ref>(nullptr);
        if (callback != nullptr) {
            napi_create_reference(env, callback, 1, &(*callbackRef));
            TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: created napi_ref for callback");
        }

        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        auto result = std::make_shared<ExecResult>();

        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 5 - creating async callbacks");
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: step 5.1 - captured name='%{public}s', args.count=%{public}zu",
            name.c_str(), args.size());

        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, name, args, options, result]() {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback(ExecTool): starting, name='%{public}s'", name.c_str());
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback(ExecTool): args count=%{public}zu", args.size());
            for (const auto &[key, value] : args) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback(ExecTool):   arg[%{public}s]='%{public}s'",
                    key.c_str(), value.c_str());
            }
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback(ExecTool): calling CliManagerClient::ExecTool");
            *innerErrCode = CliManagerClient::GetInstance().ExecTool(name, args, options, *result);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback(ExecTool): returned errCode=%{public}d", *innerErrCode);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback(ExecTool): result.exitCode=%{public}d, stdout.len=%{public}zu",
                result->exitCode, result->stdOut.length());
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrCode, result, callbackRef](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "========== CompleteCallback(ExecTool) START ==========");
            TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback(ExecTool): errCode=%{public}d", *innerErrCode);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback(ExecTool): result.exitCode=%{public}d, stdout.len=%{public}zu, stderr.len=%{public}zu",
                result->exitCode, result->stdOut.length(), result->stdErr.length());

            HandleScope handleScope(env);

            // Call user's callback if it exists
            if (*callbackRef != nullptr) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback(ExecTool): calling user callback");
                napi_value callbackFunc = nullptr;
                napi_get_reference_value(env, *callbackRef, &callbackFunc);

                if (callbackFunc != nullptr) {
                    napi_value argv[2];
                    if (*innerErrCode == ERR_OK) {
                        TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback(ExecTool): calling callback with success result");
                        argv[0] = nullptr; // error = null
                        argv[1] = CreateJsExecResult(env, *result); // result
                    } else {
                        TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback(ExecTool): calling callback with error");
                        argv[0] = CreateJsErrorByNativeErr(env, *innerErrCode); // error
                        argv[1] = nullptr; // result = undefined
                    }

                    napi_value result = nullptr;
                    napi_call_function(env, nullptr, callbackFunc, 2, argv, &result);
                    TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback(ExecTool): user callback called");
                }
            }

            // Resolve/reject promise
            if (*innerErrCode == ERR_OK) {
                TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback(ExecTool): Creating JS result object for promise");
                napi_value jsResult = CreateJsExecResult(env, *result);
                TAG_LOGI(AAFwkTag::ABILITYMGR, "CompleteCallback(ExecTool): Calling ResolveWithNoError");
                task.ResolveWithNoError(env, jsResult);
                TAG_LOGI(AAFwkTag::ABILITYMGR, "========== CompleteCallback(ExecTool) RESOLVED ==========");
            } else {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "CompleteCallback(ExecTool): Rejecting promise with errCode=%{public}d", *innerErrCode);
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
                TAG_LOGI(AAFwkTag::ABILITYMGR, "========== CompleteCallback(ExecTool) REJECTED ==========");
            }
            TAG_LOGI(AAFwkTag::ABILITYMGR, "========== CompleteCallback(ExecTool) END ==========");
        };

        napi_value asyncResult = nullptr;
        NapiAsyncTask::Schedule("JsCliManager::OnExecTool", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: Async task scheduled");
        return handleEscape.Escape(asyncResult);
    }
};

napi_value JsCliManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::ABILITYMGR, "JsCliManagerInit called");

    std::unique_ptr<JsCliManager> jsCliManager = std::make_unique<JsCliManager>();
    napi_wrap(env, exportObj, jsCliManager.release(), JsCliManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "JsCliManager";
    BindNativeFunction(env, exportObj, "queryTools", moduleName, JsCliManager::QueryTools);
    BindNativeFunction(env, exportObj, "execTool", moduleName, JsCliManager::ExecTool);

    TAG_LOGD(AAFwkTag::ABILITYMGR, "JsCliManagerInit end");
    return exportObj;
}

}  // namespace AbilityRuntime
}  // namespace OHOS
