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

#include <set>
#include <string>

#include "ability_manager_client.h"
#include "cli_error_code.h"
#include "cli_manager_error_utils.h"
#include "cli_session_info.h"
#include "cli_tool_mgr_client.h"
#include "exec_result.h"
#include "hilog_tag_wrapper.h"
#include "js_cli_event_handler_manager.h"
#include "js_cli_manager_utils.h"
#include "js_cli_session_event_callback.h"
#include "js_error_utils.h"
#include "napi_common_util.h"
#include "napi_common_want.h"
#include "skill/skill_execute_callback_stub.h"

#include <nlohmann/json.hpp>
#include "bool_wrapper.h"
#include "int_wrapper.h"
#include "string_wrapper.h"
#include "want_params_wrapper.h"

using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t INDEX_THREE = 3;
constexpr int32_t INDEX_FOUR = 4;
constexpr const char *ARKTS_SCRIPT_TOOL_NAME = "ohos-arkTSScript";
constexpr int32_t SKILL_TYPE_INDEPENDENT = -1;

const std::set<std::string> SKILL_RESERVED_KEYS = {
    "bundleName", "moduleName", "skillName", "arkTSPath", "funcName"
};

std::shared_ptr<AAFwk::WantParams> FilterSkillArgs(const AAFwk::WantParams &args)
{
    auto skillArgs = std::make_shared<AAFwk::WantParams>();
    auto &params = args.GetParams();
    for (auto &[key, value] : params) {
        if (SKILL_RESERVED_KEYS.count(key) == 0) {
            skillArgs->SetParam(key, value);
        }
    }
    return skillArgs;
}

void ExpandArgsJsonString(AAFwk::WantParams &args)
{
    auto argsStr = args.GetStringParam("args");
    if (!argsStr.empty()) {
        auto jsonObj = nlohmann::json::parse(argsStr, nullptr, false);
        if (jsonObj.is_object() && !jsonObj.is_discarded()) {
            args.Remove("args");
            for (auto &[key, val] : jsonObj.items()) {
                if (SKILL_RESERVED_KEYS.count(key) > 0) {
                    continue;
                }
                if (val.is_string()) {
                    args.SetParam(key, AAFwk::String::Box(val.get<std::string>()));
                } else if (val.is_number_integer()) {
                    args.SetParam(key, AAFwk::Integer::Box(val.get<int32_t>()));
                } else if (val.is_boolean()) {
                    args.SetParam(key, AAFwk::Boolean::Box(val.get<bool>()));
                }
            }
            return;
        }
    }

    // Handle "args" as nested WantParams (WantParamWrapper)
    auto &params = args.GetParams();
    auto it = params.find("args");
    if (it == params.end()) {
        return;
    }
    auto *wantParams = AAFwk::IWantParams::Query(it->second);
    if (wantParams == nullptr) {
        return;
    }
    AAFwk::WantParams nestedParams;
    if (wantParams->GetValue(nestedParams) != ERR_OK) {
        return;
    }
    args.Remove("args");
    for (auto &[key, value] : nestedParams.GetParams()) {
        if (SKILL_RESERVED_KEYS.count(key) > 0) {
            continue;
        }
        args.SetParam(key, value);
    }
}

CliSessionInfo BuildSkillSessionInfo(const std::string &requestCode,
    int32_t resultCode, const AppExecFwk::SkillExecuteResult &skillResult)
{
    CliSessionInfo session;
    session.sessionId = requestCode;
    session.toolName = ARKTS_SCRIPT_TOOL_NAME;
    session.status = (resultCode == ERR_OK) ? "completed" : "failed";
    session.result = std::make_shared<ExecResult>();
    session.result->exitCode = skillResult.code;
    if (skillResult.result != nullptr) {
        session.result->outputText = skillResult.result->ToString();
    }
    return session;
}

class SkillExecuteCallbackImpl : public AAFwk::SkillExecuteCallbackStub {
public:
    SkillExecuteCallbackImpl(napi_env env, std::shared_ptr<NapiAsyncTask> asyncTask)
        : env_(env), asyncTask_(std::move(asyncTask)) {}

    void OnExecuteDone(const std::string &requestCode, int32_t resultCode,
        const AppExecFwk::SkillExecuteResult &result) override
    {
        TAG_LOGD(AAFwkTag::CLI_TOOL,
            "SkillExecuteCallbackImpl::OnExecuteDone req:%{public}s code:%{public}d",
            requestCode.c_str(), resultCode);
        auto asyncTask = asyncTask_;
        auto session = BuildSkillSessionInfo(requestCode, resultCode, result);
        JsCliEventHandlerManager::GetInstance().PostTask(
            [env = env_, asyncTask, session]() {
                HandleScope handleScope(env);
                napi_value jsSession = CreateJsCliSessionInfo(env, session);
                if (jsSession == nullptr) {
                    asyncTask->Reject(env, CreateJsUndefined(env));
                    return;
                }
                asyncTask->ResolveWithNoError(env, jsSession);
            });
    }

private:
    napi_env env_ = nullptr;
    std::shared_ptr<NapiAsyncTask> asyncTask_;
};

struct SkillExecuteParam {
    std::string bundleName;
    std::string moduleName;
    std::string skillName;
    std::string arkTSPath;
    std::string funcName;
    std::shared_ptr<AAFwk::WantParams> skillArgs;
    sptr<SkillExecuteCallbackImpl> callback;
};

int32_t DispatchExecuteSkill(const SkillExecuteParam &param)
{
    int32_t skillType = 0;
    auto queryRet = AAFwk::AbilityManagerClient::GetInstance()->QuerySkillType(
        param.bundleName, param.moduleName, param.skillName, skillType);
    if (queryRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "querySkillType failed:%{public}d", queryRet);
        return queryRet;
    }
    if (skillType == SKILL_TYPE_INDEPENDENT) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "independent skill not supported yet");
        return ERR_TOOL_NOT_EXIST;
    }
    return AAFwk::AbilityManagerClient::GetInstance()->ExecuteInAppSkill(
        param.bundleName, param.moduleName, param.skillName,
        param.arkTSPath, param.funcName, param.skillArgs, param.callback);
}

int32_t ExecuteSkillTool(ExecToolParam &param, napi_env env,
    std::shared_ptr<NapiAsyncTask> asyncTask)
{
    auto &args = param.args;
    auto bundleName = args.GetStringParam("bundleName");
    auto moduleName = args.GetStringParam("moduleName");
    auto skillName = args.GetStringParam("skillName");
    if (skillName.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "skillName is required in args");
        return ERR_INVALID_VALUE;
    }
    auto arkTSPath = args.GetStringParam("arkTSPath");
    auto funcName = args.GetStringParam("funcName");
    ExpandArgsJsonString(args);
    auto skillArgs = FilterSkillArgs(args);

    TAG_LOGD(AAFwkTag::CLI_TOOL,
        "execTool skill path bundle:%{public}s module:%{public}s skill:%{public}s",
        bundleName.c_str(), moduleName.c_str(), skillName.c_str());

    auto callback = sptr<SkillExecuteCallbackImpl>::MakeSptr(env, std::move(asyncTask));
    SkillExecuteParam skillParam = {
        .bundleName = std::move(bundleName),
        .moduleName = std::move(moduleName),
        .skillName = std::move(skillName),
        .arkTSPath = std::move(arkTSPath),
        .funcName = std::move(funcName),
        .skillArgs = std::move(skillArgs),
        .callback = std::move(callback),
    };
    return DispatchExecuteSkill(skillParam);
}

int32_t DispatchCliTool(const ExecToolParam &param, napi_env env,
    std::shared_ptr<NapiAsyncTask> asyncTask)
{
    CliToolMGRClient::ExecToolReplyCallback replyCallback =
        [env, asyncTask](int32_t resultCode, const CliSessionInfo &session) {
            JsCliEventHandlerManager::GetInstance().PostTask(
                [env, asyncTask, resultCode, session]() {
                    HandleScope handleScope(env);
                    if (resultCode != ERR_OK) {
                        asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, resultCode));
                        return;
                    }
                    napi_value jsSession = CreateJsCliSessionInfo(env, session);
                    if (jsSession == nullptr) {
                        asyncTask->Reject(env, CreateJsUndefined(env));
                        return;
                    }
                    asyncTask->ResolveWithNoError(env, jsSession);
                });
        };
    return CliToolMGRClient::GetInstance().ExecTool(param, replyCallback);
}
} // namespace

void JSCliManager::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::Finalizer is called");
    std::unique_ptr<JSCliManager>(static_cast<JSCliManager*>(data));
}

napi_value JSCliManager::ExecTool(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnExecTool);
}

napi_value JSCliManager::SubscribeSession(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnSubscribeSession);
}

napi_value JSCliManager::ClearSession(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnClearSession);
}

napi_value JSCliManager::QuerySession(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnQuerySession);
}

napi_value JSCliManager::SendMessage(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnSendMessage);
}

napi_value JSCliManager::GetToolInfoByName(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnGetToolInfoByName);
}

napi_value JSCliManager::QueryToolSummaries(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnQueryToolSummaries);
}

napi_value JSCliManager::QueryTools(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnQueryTools);
}

napi_value JSCliManager::OnExecTool(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::OnExecTool called");
    HandleEscape handleEscape(env);
    if (argc < INDEX_FOUR) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    ExecToolParam param;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], param.toolName) || param.toolName.empty()) {
        ThrowInvalidParamError(env, "Tool toolName is required");
        return CreateJsUndefined(env);
    }

    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ONE], param.subcommand)) {
        ThrowInvalidParamError(env, "Tool subcommand is required");
        return CreateJsUndefined(env);
    }

    if (!AppExecFwk::UnwrapWantParams(env, argv[INDEX_TWO], param.args)) {
        ThrowInvalidParamError(env, "Tool args is required");
        return CreateJsUndefined(env);
    }

    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_THREE], param.challenge) || param.challenge.empty()) {
        ThrowInvalidParamError(env, "Tool challenge is required");
        return CreateJsUndefined(env);
    }

    if (argc > INDEX_FOUR && argv[INDEX_FOUR] != nullptr) {
        if (!UnwrapExecOptions(env, argv[INDEX_FOUR], param.options)) {
            ThrowInvalidParamError(env, "Tool options is required");
            return CreateJsUndefined(env);
        }
    }

    napi_value result = nullptr;
    auto uasyncTask = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);

    int32_t errCode = ERR_OK;
    if (param.toolName == ARKTS_SCRIPT_TOOL_NAME) {
        errCode = ExecuteSkillTool(param, env, asyncTask);
    } else {
        errCode = DispatchCliTool(param, env, asyncTask);
    }
    if (errCode != ERR_OK) {
        asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, errCode));
    }
    return handleEscape.Escape(result);
}

napi_value JSCliManager::OnSubscribeSession(napi_env env, size_t argc, napi_value *argv)
{
    HandleEscape handleEscape(env);
    if (argc < INDEX_TWO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string sessionId;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
        ThrowInvalidParamError(env, "sessionId is required");
        return CreateJsUndefined(env);
    }
    if (!IsValidToolEventCallback(env, argv[INDEX_ONE])) {
        ThrowInvalidParamError(env, "tool event callback is required");
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto callback = std::make_shared<JsCliSessionEventCallbackImpl>(env, argv[INDEX_ONE]);
    if (callback == nullptr || !callback->IsValid()) {
        ThrowInvalidParamError(env, "callback must be a function");
        return CreateJsUndefined(env);
    }
    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, callback, sessionId]() {
        std::string subscriptionId;
        *innerErrCode = CliToolMGRClient::GetInstance().SubscribeSession(sessionId, callback, subscriptionId);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env,
        NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSCliManager::OnSubscribeSession", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JSCliManager::OnClearSession(napi_env env, size_t argc, napi_value *argv)
{
    HandleEscape handleEscape(env);
    if (argc < INDEX_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string sessionId;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
        ThrowInvalidParamError(env, "sessionId is required");
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, sessionId]() {
        *innerErrCode = CliToolMGRClient::GetInstance().ClearSession(sessionId);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env, NapiAsyncTask &task,
        int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSCliManager::OnClearSession", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

napi_value JSCliManager::OnSendMessage(napi_env env, size_t argc, napi_value *argv)
{
    HandleEscape handleEscape(env);
    if (argc < INDEX_TWO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    std::string sessionId;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
        ThrowInvalidParamError(env, "sessionId is required");
        return CreateJsUndefined(env);
    }

    std::string inputText;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ONE], inputText)) {
        ThrowInvalidParamError(env, "inputText is required");
        return CreateJsUndefined(env);
    }

    napi_value asyncResult = nullptr;
    napi_value lastParam = nullptr;
    std::unique_ptr<NapiAsyncTask> uasyncTask =
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &asyncResult);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);

    CliToolMGRClient::EventReplyCallback replyCallback = [env, asyncTask](int32_t resultCode) {
        JsCliEventHandlerManager::GetInstance().PostTask([env, asyncTask, resultCode]() {
            HandleScope handleScope(env);
            if (resultCode != ERR_OK) {
                asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, resultCode));
                return;
            }
            asyncTask->ResolveWithNoError(env, CreateJsUndefined(env));
        });
    };
    int32_t errCode = CliToolMGRClient::GetInstance().SendMessage(sessionId, inputText, replyCallback);
    if (errCode != ERR_OK) {
        asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, errCode));
    }
    return handleEscape.Escape(asyncResult);
}

napi_value JSCliManager::OnQuerySession(napi_env env, size_t argc, napi_value *argv)
{
    HandleEscape handleEscape(env);
    if (argc < INDEX_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    std::string sessionId;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
        ThrowInvalidParamError(env, "sessionId is required");
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto session = std::make_shared<CliSessionInfo>();
    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, sessionId, session]() {
        *innerErrCode = CliToolMGRClient::GetInstance().QuerySession(sessionId, *session);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode, session](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }
        napi_value jsSession = CreateJsCliSessionInfo(env, *session);
        if (jsSession == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Fail to create js CliSessionInfo");
            task.Reject(env, CreateJsUndefined(env));
            return;
        }
        task.ResolveWithNoError(env, jsSession);
    };
    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JSCliManager::OnQuerySession", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSCliManager::OnGetToolInfoByName(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::OnGetToolInfoByName called");
    HandleEscape handleEscape(env);
    if (argc < INDEX_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }
    std::string toolName;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], toolName) || toolName.empty()) {
        ThrowInvalidParamError(env, "Tool name is required");
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto tool = std::make_shared<ToolInfo>();

    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, toolName, tool]() {
        *innerErrCode = CliToolMGRClient::GetInstance().GetToolInfoByName(toolName, *tool);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode, tool](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "GetToolInfoByName error: %{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        napi_value jsTool = CreateJsToolInfo(env, *tool);
        if (jsTool == nullptr) {
            task.Reject(env, CreateJsUndefined(env));
            return;
        }

        task.ResolveWithNoError(env, jsTool);
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JsCliManager::OnGetToolInfoByName", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSCliManager::OnQueryToolSummaries(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::OnQueryToolSummaries called");
    HandleEscape handleEscape(env);

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto summaries = std::make_shared<std::vector<ToolSummary>>();

    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, summaries]() {
        *innerErrCode = CliToolMGRClient::GetInstance().GetAllToolSummaries(*summaries);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode, summaries](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "QueryToolSummaries error: %{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        napi_value jsArray = nullptr;
        napi_create_array(env, &jsArray);
        for (size_t i = 0; i < summaries->size(); i++) {
            napi_value jsSummary = CreateJsToolSummary(env, (*summaries)[i]);
            if (jsSummary != nullptr) {
                napi_set_element(env, jsArray, i, jsSummary);
            }
        }
        task.ResolveWithNoError(env, jsArray);
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JsCliManager::OnQueryToolSummaries", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSCliManager::OnQueryTools(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::OnQueryTools called");
    HandleEscape handleEscape(env);

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);
    auto tools = std::make_shared<std::vector<ToolInfo>>();

    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, tools]() {
        *innerErrCode = CliToolMGRClient::GetInstance().GetAllToolInfos(*tools);
    };

    NapiAsyncTask::CompleteCallback complete = [innerErrCode, tools](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "QueryTools error: %{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        napi_value jsArray = nullptr;
        napi_create_array(env, &jsArray);
        for (size_t i = 0; i < tools->size(); i++) {
            napi_value jsTool = CreateJsToolInfo(env, (*tools)[i]);
            if (jsTool != nullptr) {
                napi_set_element(env, jsArray, i, jsTool);
            }
        }
        task.ResolveWithNoError(env, jsArray);
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JsCliManager::OnQueryTools", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

napi_value JSCliManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "Init JSCliManager");

    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Null env or exportObj");
        return nullptr;
    }

    std::unique_ptr<JSCliManager> jsCliManager = std::make_unique<JSCliManager>();
    napi_wrap(env, exportObj, jsCliManager.release(), JSCliManager::Finalizer, nullptr, nullptr);

    const char *moduleName = "CliManager";
    BindNativeFunction(env, exportObj, "execTool", moduleName, JSCliManager::ExecTool);
    BindNativeFunction(env, exportObj, "subscribeSession", moduleName, JSCliManager::SubscribeSession);
    BindNativeFunction(env, exportObj, "clearSession", moduleName, JSCliManager::ClearSession);
    BindNativeFunction(env, exportObj, "querySession", moduleName, JSCliManager::QuerySession);
    BindNativeFunction(env, exportObj, "sendMessage", moduleName, JSCliManager::SendMessage);
    BindNativeFunction(env, exportObj, "getToolInfoByName", moduleName, JSCliManager::GetToolInfoByName);
    BindNativeFunction(env, exportObj, "queryToolSummaries", moduleName, JSCliManager::QueryToolSummaries);
    BindNativeFunction(env, exportObj, "queryTools", moduleName, JSCliManager::QueryTools);

    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManagerInit end");
    return CreateJsUndefined(env);
}

} // namespace CliTool
} // namespace OHOS
