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
constexpr size_t ARGC_TWO = 2;
constexpr size_t INDEX_ZERO = 0;
constexpr size_t INDEX_ONE = 1;
constexpr size_t INDEX_TWO = 2;
constexpr size_t INDEX_THREE = 3;
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
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnQueryTools called");

        HandleEscape handleEscape(env);
        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        auto tools = std::make_shared<std::vector<ToolInfo>>();

        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, tools]() {
            *innerErrCode = CliManagerClient::GetInstance().QueryTools(*tools);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "QueryTools returned errCode=%{public}d, tools.size()=%{public}zu",
                *innerErrCode, tools->size());
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrCode, tools](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode == ERR_OK) {
                napi_value jsTools = CreateJsToolInfoArray(env, *tools);
                task.ResolveWithNoError(env, jsTools);
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };

        napi_value lastParam = (argc > 0) ? argv[argc - 1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsCliManager::OnQueryTools", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnExecTool(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool called");

        HandleEscape handleEscape(env);

        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid argc, requires at least 2 arguments (name, args)");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        // Parse tool name (argv[0]) - required
        std::string name;
        if (!UnwrapStringFromJS2(env, argv[INDEX_ZERO], name)) {
            ThrowInvalidParamError(env, "Tool name is required");
            return CreateJsUndefined(env);
        }

        if (name.empty()) {
            ThrowInvalidParamError(env, "Tool name cannot be empty");
            return CreateJsUndefined(env);
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR, "Tool name: '%{public}s'", name.c_str());

        // Parse args (argv[1]) - required, plain JS object
        std::vector<std::pair<std::string, std::string>> args;
        napi_value jsArgs = argv[INDEX_ONE];
        if (jsArgs != nullptr) {
            UnwrapStringMap(env, jsArgs, args);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "Args count: %{public}zu", args.size());
        }

        // Parse optional parameters based on d.ts signature:
        // execTool(name, args, challenge?, options?)
        ExecOptions options;
        std::string challenge;  // Default empty

        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: argc=%{public}zu", argc);

        if (argc >= INDEX_THREE + 1 && argv[INDEX_TWO] != nullptr) {
            napi_valuetype valueType;
            napi_status status = napi_typeof(env, argv[INDEX_TWO], &valueType);

            TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: argv[2] valueType=%{public}d", valueType);

            if (status == napi_ok) {
                if (valueType == napi_string) {
                    // argv[2] is challenge (string)
                    UnwrapStringFromJS2(env, argv[INDEX_TWO], challenge);
                    TAG_LOGI(AAFwkTag::ABILITYMGR, "Challenge parameter provided: '%{public}s'", challenge.c_str());

                    // argv[3] might be options
                    if (argc >= INDEX_THREE + 1 && argv[INDEX_THREE] != nullptr) {
                        napi_valuetype valueType3;
                        napi_status status3 = napi_typeof(env, argv[INDEX_THREE], &valueType3);
                        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool: argv[3] exists, valueType=%{public}d", valueType3);

                        if (status3 == napi_ok && valueType3 == napi_object) {
                            UnwrapExecOptions(env, argv[INDEX_THREE], options);
                            TAG_LOGI(AAFwkTag::ABILITYMGR, "Options parsed from argv[3]: timeout=%{public}d, background=%{public}d",
                                options.timeout, options.background);
                        } else {
                            TAG_LOGW(AAFwkTag::ABILITYMGR, "argv[3] is not an object, cannot parse options");
                        }
                    } else {
                        TAG_LOGI(AAFwkTag::ABILITYMGR, "No argv[3] provided, using default options");
                    }
                } else if (valueType == napi_object) {
                    // argv[2] is options (object) - 3参数模式：execTool(name, args, options)
                    UnwrapExecOptions(env, argv[INDEX_TWO], options);
                    TAG_LOGI(AAFwkTag::ABILITYMGR, "Options parsed from argv[2]: timeout=%{public}d, background=%{public}d",
                        options.timeout, options.background);
                } else {
                    TAG_LOGW(AAFwkTag::ABILITYMGR, "argv[2] has unexpected type: %{public}d", valueType);
                }
            }
        } else {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "No optional parameters provided, using defaults");
        }

        TAG_LOGI(AAFwkTag::ABILITYMGR, "OnExecTool final: timeout=%{public}d, background=%{public}d, challenge='%{public}s'",
            options.timeout, options.background, challenge.c_str());

        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        auto session = std::make_shared<SessionInfo>();

        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, name, args, options, challenge, session]() {
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback: ExecTool name='%{public}s', challenge='%{public}s'",
                name.c_str(), challenge.c_str());
            *innerErrCode = CliManagerClient::GetInstance().ExecTool(name, args, options, challenge, *session);
            TAG_LOGI(AAFwkTag::ABILITYMGR, "ExecuteCallback: errCode=%{public}d, status='%{public}s",
                *innerErrCode, session->status.c_str());
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrCode, session](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);

            // If IPC call failed, reject promise with error
            if (*innerErrCode != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "ExecTool failed with errCode=%{public}d, rejecting promise", *innerErrCode);
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
                return;
            }

            // Return session info from service
            napi_value jsSession = CreateJsSessionInfo(env, *session);
            if (jsSession == nullptr) {
                task.Reject(env, CreateJsUndefined(env));
                return;
            }

            task.ResolveWithNoError(env, jsSession);
        };

        // Find last parameter (callback if present)
        napi_value lastParam = nullptr;
        for (size_t i = argc; i > 0; i--) {
            napi_value param = argv[i - 1];
            if (param != nullptr) {
                napi_valuetype valueType;
                if (napi_typeof(env, param, &valueType) == napi_ok && valueType == napi_function) {
                    lastParam = param;
                    break;
                }
            }
        }

        napi_value asyncResult = nullptr;
        NapiAsyncTask::Schedule("JsCliManager::OnExecTool", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &asyncResult));
        return handleEscape.Escape(asyncResult);
    }
};

napi_value JsCliManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "JsCliManagerInit called");

    // Wrap JsCliManager instance to exportObj (like JsAbilityManagerInit)
    std::unique_ptr<JsCliManager> jsCliManager = std::make_unique<JsCliManager>();
    napi_wrap(env, exportObj, jsCliManager.release(), JsCliManager::Finalizer, nullptr, nullptr);

    // Bind methods directly to exportObj (like JsAbilityManagerInit)
    const char* moduleName = "JsCliManager";
    BindNativeFunction(env, exportObj, "queryTools", moduleName, JsCliManager::QueryTools);
    BindNativeFunction(env, exportObj, "execTool", moduleName, JsCliManager::ExecTool);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "JsCliManagerInit end");
    return CreateJsUndefined(env);
}

}  // namespace AbilityRuntime
}  // namespace OHOS
