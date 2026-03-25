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
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnQueryTools called");

        HandleEscape handleEscape(env);
        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        auto tools = std::make_shared<std::vector<ToolInfo>>();

        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, tools]() {
            *innerErrCode = CliManagerClient::GetInstance().QueryTools(*tools);
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

        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsCliManager::OnQueryTools", env,
            CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnExecTool(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGD(AAFwkTag::ABILITYMGR, "OnExecTool called");

        HandleEscape handleEscape(env);

        if (argc < ARGC_TWO) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Invalid argc, at least 2 arguments required");
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        // Parse tool name
        std::string name;
        if (!UnwrapStringFromJS2(env, argv[INDEX_ZERO], name)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to parse tool name");
            ThrowInvalidParamError(env, "Tool name is required");
            return CreateJsUndefined(env);
        }

        if (name.empty()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "Tool name is empty");
            ThrowInvalidParamError(env, "Tool name is required");
            return CreateJsUndefined(env);
        }

        // Parse options object
        napi_value jsOptions = argv[INDEX_ONE];
        std::map<std::string, std::string> args;
        ExecOptions options;

        if (jsOptions != nullptr) {
            napi_value jsArgs = nullptr;
            napi_get_named_property(env, jsOptions, "args", &jsArgs);
            if (jsArgs != nullptr) {
                UnwrapStringMap(env, jsArgs, args);
            }

            napi_value jsExecOptions = nullptr;
            napi_get_named_property(env, jsOptions, "options", &jsExecOptions);
            UnwrapExecOptions(env, jsExecOptions, options);
        }

        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        auto result = std::make_shared<ExecResult>();

        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, name, args, options, result]() {
            *innerErrCode = CliManagerClient::GetInstance().ExecTool(name, args, options, *result);
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrCode, result](
            napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode == ERR_OK) {
                napi_value jsResult = CreateJsExecResult(env, *result);
                task.ResolveWithNoError(env, jsResult);
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };

        napi_value lastParam = (argc >= ARGC_TWO) ? argv[INDEX_ONE] : nullptr;
        napi_value asyncResult = nullptr;
        NapiAsyncTask::Schedule("JsCliManager::OnExecTool", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &asyncResult));
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
