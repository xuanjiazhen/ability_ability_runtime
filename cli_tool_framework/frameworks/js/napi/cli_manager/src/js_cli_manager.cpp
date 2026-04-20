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

#include "cli_tool_mgr_client.h"
#include "hilog_tag_wrapper.h"
#include "js_cli_manager_utils.h"
#include "js_runtime.h"
#include "js_runtime_utils.h"
#include "napi/native_api.h"
#include "napi_common_util.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr size_t INDEX_ZERO = 0;
constexpr int32_t ERR_OK = 0;
}

class JsCliManager final {
public:
    JsCliManager() = default;
    ~JsCliManager() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "JsCliManager finalizer called");
        std::unique_ptr<JsCliManager>(static_cast<JsCliManager*>(data));
    }

    static napi_value QueryTools(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsCliManager, OnQueryTools);
    }

    static napi_value QueryToolSummaries(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsCliManager, OnQueryToolSummaries);
    }

    static napi_value GetToolInfoByName(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsCliManager, OnGetToolInfoByName);
    }

    static napi_value RegisterCliTool(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, JsCliManager, OnRegisterCliTool);
    }

private:
    napi_value OnQueryTools(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "OnQueryTools called");

        HandleEscape handleEscape(env);
        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        auto tools = std::make_shared<std::vector<ToolInfo>>();

        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, tools]() {
            *innerErrCode = CliToolMGRClient::GetInstance().QueryTools(*tools);
            TAG_LOGI(AAFwkTag::CLI_TOOL, "QueryTools returned errCode=%{public}d, tools.size()=%{public}zu",
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

    napi_value OnQueryToolSummaries(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "OnQueryToolSummaries called");

        HandleEscape handleEscape(env);
        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        auto summaries = std::make_shared<std::vector<ToolSummary>>();

        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, summaries]() {
            *innerErrCode = CliToolMGRClient::GetInstance().QueryToolSummaries(*summaries);
            TAG_LOGI(AAFwkTag::CLI_TOOL, "QueryToolSummaries returned errCode=%{public}d, summaries.size()=%{public}zu",
                *innerErrCode, summaries->size());
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrCode, summaries](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode == ERR_OK) {
                napi_value jsSummaries = CreateJsToolSummaryArray(env, *summaries);
                task.ResolveWithNoError(env, jsSummaries);
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };

        napi_value lastParam = (argc > 0) ? argv[argc - 1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsCliManager::OnQueryToolSummaries", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnGetToolInfoByName(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "OnGetToolInfoByName called");

        HandleEscape handleEscape(env);
        if (argc < 1) {
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        std::string name;
        if (!ConvertFromJsValue(env, argv[INDEX_ZERO], name) || name.empty()) {
            ThrowInvalidParamError(env, "Tool name is required");
            return CreateJsUndefined(env);
        }

        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        auto tool = std::make_shared<ToolInfo>();

        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, name, tool]() {
            *innerErrCode = CliToolMGRClient::GetInstance().GetToolInfoByName(name, *tool);
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrCode, tool](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode == ERR_OK) {
                napi_value jsTool = CreateJsToolInfo(env, *tool);
                task.ResolveWithNoError(env, jsTool);
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };

        napi_value lastParam = (argc > 1) ? argv[argc - 1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsCliManager::OnGetToolInfoByName", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }

    napi_value OnRegisterCliTool(napi_env env, size_t argc, napi_value *argv)
    {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "OnRegisterCliTool called");

        HandleEscape handleEscape(env);
        if (argc < 1) {
            ThrowTooFewParametersError(env);
            return CreateJsUndefined(env);
        }

        auto tool = std::make_shared<ToolInfo>();
        if (!ParseJsToolInfo(env, argv[INDEX_ZERO], *tool)) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "parse tool failed");
            ThrowInvalidParamError(env, "Parse param tool failed.");
            return CreateJsUndefined(env);
        }

        auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
        NapiAsyncTask::ExecuteCallback execute = [innerErrCode, tool]() {
            *innerErrCode = CliToolMGRClient::GetInstance().RegisterCliTool(*tool);
        };

        NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask &task, int32_t status) {
            HandleScope handleScope(env);
            if (*innerErrCode == ERR_OK) {
                task.ResolveWithNoError(env, CreateJsUndefined(env));
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };

        napi_value lastParam = (argc > 1) ? argv[argc - 1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("JsCliManager::OnRegisterCliTool", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }
};

napi_value JsCliManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "JsCliManagerInit called");

    std::unique_ptr<JsCliManager> jsCliManager = std::make_unique<JsCliManager>();
    napi_wrap(env, exportObj, jsCliManager.release(), JsCliManager::Finalizer, nullptr, nullptr);

    const char* moduleName = "JsCliManager";
    BindNativeFunction(env, exportObj, "queryTools", moduleName, JsCliManager::QueryTools);
    BindNativeFunction(env, exportObj, "queryToolSummaries", moduleName, JsCliManager::QueryToolSummaries);
    BindNativeFunction(env, exportObj, "getToolInfoByName", moduleName, JsCliManager::GetToolInfoByName);
    BindNativeFunction(env, exportObj, "registerCliTool", moduleName, JsCliManager::RegisterCliTool);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "JsCliManagerInit end");
    return CreateJsUndefined(env);
}

} // namespace CliTool
} // namespace OHOS
