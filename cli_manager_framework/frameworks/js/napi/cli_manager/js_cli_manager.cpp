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

#include "js_cli_manager.h"

#include <cstdint>
#include <memory>

#include "ability_business_error.h"
#include "cli_manager_client.h"
#include "hilog_tag_wrapper.h"
#include "js_cli_manager_utils.h"
#include "js_cli_subscription_manager.h"
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

    static napi_value CancelSession(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsCliManager, OnCancelSession);
    }

    static napi_value SubscribeSession(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsCliManager, OnSubscribeSession);
    }

    static napi_value QuerySessionStatus(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsCliManager, OnQuerySessionStatus);
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
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string name;
        if (!UnwrapStringFromJS2(env, argv[INDEX_ZERO], name) || name.empty()) {
            ThrowInvalidParamError(env, "Tool name is required");
            return CreateJsUndefined(env);
        }

        std::vector<std::pair<std::string, std::string>> args;
        UnwrapStringMap(env, argv[INDEX_ONE], args);

        ExecOptions options;
        std::string challenge;
        if (argc > INDEX_TWO && argv[INDEX_TWO] != nullptr) {
            napi_valuetype valueType = napi_undefined;
            if (napi_typeof(env, argv[INDEX_TWO], &valueType) == napi_ok) {
                if (valueType == napi_string) {
                    UnwrapStringFromJS2(env, argv[INDEX_TWO], challenge);
                } else if (valueType == napi_object) {
                    UnwrapExecOptions(env, argv[INDEX_TWO], options);
                }
            }
        }
        if (argc > INDEX_THREE && argv[INDEX_THREE] != nullptr) {
            UnwrapExecOptions(env, argv[INDEX_THREE], options);
        }

        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        auto session = std::make_shared<SessionInfo>();

        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, name, args, options, challenge, session]() {
            *innerErrCode = CliManagerClient::GetInstance().ExecTool(name, args, options, challenge, *session);
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrCode, session](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);

            if (*innerErrCode != ERR_OK) {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
                return;
            }

            napi_value jsSession = CreateJsSessionInfo(env, *session);
            if (jsSession == nullptr) {
                task.Reject(env, CreateJsUndefined(env));
                return;
            }

            task.ResolveWithNoError(env, jsSession);
        };

        napi_value lastParam = nullptr;
        for (size_t i = argc; i > 0; i--) {
            napi_value param = argv[i - 1];
            if (param != nullptr) {
                napi_valuetype valueType = napi_undefined;
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

    napi_value OnCancelSession(napi_env env, size_t argc, napi_value *argv)
    {
        HandleEscape handleEscape(env);
        if (argc < 1) {
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string sessionId;
        if (!UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
            ThrowInvalidParamError(env, "sessionId is required");
            return CreateJsUndefined(env);
        }

        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        auto cancelled = std::make_shared<bool>(false);
        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, cancelled, sessionId]() {
            *innerErrCode = CliManagerClient::GetInstance().CancelSession(sessionId);
            *cancelled = (*innerErrCode == ERR_OK);
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrCode, cancelled](napi_env env, NapiAsyncTask &task,
            int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode != ERR_OK) {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
                return;
            }
            task.ResolveWithNoError(env, CreateJsValue(env, *cancelled));
        };

        napi_value lastParam = (argc > 1) ? argv[argc - 1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsCliManager::OnCancelSession", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnSubscribeSession(napi_env env, size_t argc, napi_value *argv)
    {
        HandleEscape handleEscape(env);
        if (argc < ARGC_TWO) {
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string sessionId;
        if (!UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
            ThrowInvalidParamError(env, "sessionId is required");
            return CreateJsUndefined(env);
        }

        napi_valuetype callbackType = napi_undefined;
        if (napi_typeof(env, argv[INDEX_ONE], &callbackType) != napi_ok || callbackType != napi_function) {
            ThrowInvalidParamError(env, "callback must be a function");
            return CreateJsUndefined(env);
        }

        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        auto subscribed = std::make_shared<bool>(false);
        std::string subscriptionId =
            CliSessionSubscriptionManager::GetInstance().AddProvisionalSubscription(env, sessionId, argv[INDEX_ONE]);
        if (subscriptionId.empty()) {
            ThrowInvalidParamError(env, "failed to create session callback bridge");
            return CreateJsUndefined(env);
        }

        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, subscribed, sessionId, subscriptionId]() {
            *innerErrCode = CliManagerClient::GetInstance().SubscribeSession(sessionId, subscriptionId);
            *subscribed = (*innerErrCode == ERR_OK);
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrCode, subscribed, subscriptionId](napi_env env,
            NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode != ERR_OK) {
                CliSessionSubscriptionManager::GetInstance().RemoveSubscription(subscriptionId);
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
                return;
            }
            task.ResolveWithNoError(env, CreateJsValue(env, *subscribed));
            std::unique_ptr<NapiAsyncTask::CompleteCallback> activate =
                std::make_unique<NapiAsyncTask::CompleteCallback>(
                    [subscriptionId](napi_env activateEnv, NapiAsyncTask &activateTask, int32_t activateStatus) {
                        CliSessionSubscriptionManager::GetInstance().ActivateSubscription(subscriptionId);
                    });
            napi_ref callback = nullptr;
            std::unique_ptr<NapiAsyncTask::ExecuteCallback> execute = nullptr;
            NapiAsyncTask::Schedule("JsCliManager::ActivateSubscription", env,
                std::make_unique<NapiAsyncTask>(callback, std::move(execute), std::move(activate)));
        };

        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsCliManager::OnSubscribeSession", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnQuerySessionStatus(napi_env env, size_t argc, napi_value *argv)
    {
        HandleEscape handleEscape(env);
        if (argc < 1) {
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string sessionId;
        if (!UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
            ThrowInvalidParamError(env, "sessionId is required");
            return CreateJsUndefined(env);
        }

        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        auto sessionStatus = std::make_shared<SessionStatus>();
        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, sessionId, sessionStatus]() {
            *innerErrCode = CliManagerClient::GetInstance().QuerySessionStatus(sessionId, *sessionStatus);
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrCode, sessionStatus](napi_env env, NapiAsyncTask &task,
            int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode != ERR_OK) {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
                return;
            }
            task.ResolveWithNoError(env, CreateJsSessionStatus(env, *sessionStatus));
        };

        napi_value lastParam = (argc > 1) ? argv[argc - 1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsCliManager::OnQuerySessionStatus", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
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
    BindNativeFunction(env, exportObj, "subscribeSession", moduleName, JsCliManager::SubscribeSession);
    BindNativeFunction(env, exportObj, "cancelSession", moduleName, JsCliManager::CancelSession);
    BindNativeFunction(env, exportObj, "querySessionStatus", moduleName, JsCliManager::QuerySessionStatus);

    TAG_LOGI(AAFwkTag::ABILITYMGR, "JsCliManagerInit end");
    return CreateJsUndefined(env);
}

}  // namespace AbilityRuntime
}  // namespace OHOS
