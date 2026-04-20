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

#include "ets_cli_manager.h"

#include <cstdint>
#include <memory>

#include "cli_tool_mgr_client.h"
#include "ets_cli_manager_utils.h"
#include "hilog_tag_wrapper.h"
#include "native_engine/native_engine.h"
#include "native_engine/impl/ark/ark_native_engine.h"

namespace OHOS {
namespace CliTool {

namespace {
constexpr size_t INDEX_ZERO = 0;
constexpr int32_t ERR_OK = 0;
}

class EtsCliManager final {
public:
    EtsCliManager() = default;
    ~EtsCliManager() = default;

    static void Finalizer(napi_env env, void* data, void* hint)
    {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "EtsCliManager finalizer called");
        std::unique_ptr<EtsCliManager>(static_cast<EtsCliManager*>(data));
    }

    static napi_value QueryTools(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, EtsCliManager, OnQueryTools);
    }

    static napi_value QueryToolSummaries(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, EtsCliManager, OnQueryToolSummaries);
    }

    static napi_value GetToolInfoByName(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, EtsCliManager, OnGetToolInfoByName);
    }

    static napi_value RegisterCliTool(napi_env env, napi_callback_info info)
    {
        GET_CB_INFO_AND_CALL(env, info, EtsCliManager, OnRegisterCliTool);
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
                napi_value etsTools = CreateEtsToolInfoArray(env, *tools);
                task.ResolveWithNoError(env, etsTools);
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };

        napi_value lastParam = (argc > 0) ? argv[argc - 1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("EtsCliManager::OnQueryTools", env,
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
                napi_value etsSummaries = CreateEtsToolSummaryArray(env, *summaries);
                task.ResolveWithNoError(env, etsSummaries);
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };

        napi_value lastParam = (argc > 0) ? argv[argc - 1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("EtsCliManager::OnQueryToolSummaries", env,
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
                napi_value etsTool = CreateEtsToolInfo(env, *tool);
                task.ResolveWithNoError(env, etsTool);
            } else {
                task.Reject(env, CreateJsErrorByNativeErr(env, *innerErrCode));
            }
        };

        napi_value lastParam = (argc > 1) ? argv[argc - 1] : nullptr;
        napi_value result = nullptr;
        NapiAsyncTask::Schedule("EtsCliManager::OnGetToolInfoByName", env,
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
        if (!ParseEtsToolInfo(env, argv[INDEX_ZERO], *tool)) {
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
        NapiAsyncTask::Schedule("EtsCliManager::OnRegisterCliTool", env,
            CreateAsyncTaskWithLastParam(env, lastParam, std::move(execute), std::move(complete), &result));
        return handleEscape.Escape(result);
    }
};

napi_value EtsCliManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "EtsCliManagerInit called");

    std::unique_ptr<EtsCliManager> etsCliManager = std::make_unique<EtsCliManager>();
    napi_wrap(env, exportObj, etsCliManager.release(), EtsCliManager::Finalizer, nullptr, nullptr);

    const char* moduleName = "EtsCliManager";
    BindNativeFunction(env, exportObj, "queryTools", moduleName, EtsCliManager::QueryTools);
    BindNativeFunction(env, exportObj, "queryToolSummaries", moduleName, EtsCliManager::QueryToolSummaries);
    BindNativeFunction(env, exportObj, "getToolInfoByName", moduleName, EtsCliManager::GetToolInfoByName);
    BindNativeFunction(env, exportObj, "registerCliTool", moduleName, EtsCliManager::RegisterCliTool);

    TAG_LOGI(AAFwkTag::CLI_TOOL, "EtsCliManagerInit end");
    return CreateJsUndefined(env);
}

} // namespace CliTool
} // namespace OHOS
