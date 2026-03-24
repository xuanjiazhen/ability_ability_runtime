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
#include "cli_manager_client.h"
#include "hilog_tag_wrapper.h"
#include <memory>
#include <mutex>

namespace OHOS {
namespace CliMgr {
namespace JS {

// 静态成员初始化
napi_ref JSCliManager::sConstructor_ = nullptr;

// 模块名称
static constexpr const char* MODULE_NAME = "cliManager";
static constexpr const char* CLASS_NAME = "CLIManager";

// NAPI 方法名称
static constexpr const char* METHOD_QUERY_TOOLS = "queryTools";
static constexpr const char* METHOD_QUERY_TOOL_BY_NAME = "queryToolByName";
static constexpr const char* METHOD_QUERY_TOOLS_BY_PERMISSION = "queryToolsByPermission";
static constexpr const char* METHOD_EXEC_TOOL = "execTool";
static constexpr const char* METHOD_EXEC_TOOL_ASYNC = "execToolAsync";
static constexpr const char* METHOD_CANCEL_TASK = "cancelTask";
static constexpr const char* METHOD_QUERY_TASK_STATUS = "queryTaskStatus";

napi_value JSCliManager::Init(napi_env env, napi_value exports)
{
    TAG_LOGI("JSCliManager", "Init called");

    // 定义类属性
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_STATIC_METHOD(METHOD_QUERY_TOOLS, QueryTools),
        DECLARE_NAPI_STATIC_METHOD(METHOD_QUERY_TOOL_BY_NAME, QueryToolByName),
        DECLARE_NAPI_STATIC_METHOD(METHOD_QUERY_TOOLS_BY_PERMISSION, QueryToolsByPermission),
        DECLARE_NAPI_STATIC_METHOD(METHOD_EXEC_TOOL, ExecTool),
        DECLARE_NAPI_STATIC_METHOD(METHOD_EXEC_TOOL_ASYNC, ExecToolAsync),
        DECLARE_NAPI_STATIC_METHOD(METHOD_CANCEL_TASK, CancelTask),
        DECLARE_NAPI_STATIC_METHOD(METHOD_QUERY_TASK_STATUS, QueryTaskStatus),
    };

    // 定义类
    napi_value constructor;
    napi_status status = napi_define_class(
        env,
        CLASS_NAME,
        NAPI_AUTO_LENGTH,
        Constructor,
        nullptr,
        sizeof(properties) / sizeof(napi_property_descriptor),
        properties,
        &constructor
    );

    if (status != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to define class");
        return nullptr;
    }

    // 保存构造函数引用
    status = napi_create_reference(env, constructor, 1, &sConstructor_);
    if (status != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to create reference");
        return nullptr;
    }

    // 导出类
    status = napi_set_named_property(env, exports, CLASS_NAME, constructor);
    if (status != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to set named property");
        return nullptr;
    }

    TAG_LOGI("JSCliManager", "Init completed successfully");
    return exports;
}

napi_value JSCliManager::Constructor(napi_env env, napi_callback_info info)
{
    TAG_LOGD("JSCliManager", "Constructor called");

    napi_value thisVar = nullptr;
    napi_get_undefined(env, &thisVar);

    // 获取 this
    if (napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr) != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to get cb info");
        return nullptr;
    }

    return thisVar;
}

napi_value JSCliManager::QueryTools(napi_env env, napi_callback_info info)
{
    TAG_LOGD("JSCliManager", "QueryTools called");

    // 获取参数
    size_t argc = 1;
    napi_value argv[1];
    napi_value thisVar = nullptr;

    if (napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to get cb info");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Invalid callback info");
    }

    // 检查参数
    if (argc < 1) {
        TAG_LOGE("JSCliManager", "Missing callback parameter");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Missing callback parameter");
    }

    napi_valuetype valueType;
    if (napi_typeof(env, argv[0], &valueType) != napi_ok || valueType != napi_function) {
        TAG_LOGE("JSCliManager", "First parameter must be a function");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "First parameter must be a function");
    }

    // 创建异步工作
    napi_value resourceName;
    napi_create_string_utf8(env, "QueryTools", NAPI_AUTO_LENGTH, &resourceName);

    auto asyncContext = new (std::nothrow) AsyncContext();
    if (asyncContext == nullptr) {
        TAG_LOGE("JSCliManager", "Failed to create async context");
        return CreateError(env, ErrCode::ERR_NO_MEMORY, "Out of memory");
    }

    asyncContext->env = env;
    napi_create_reference(env, argv[0], 1, &asyncContext->callbackRef);

    napi_value asyncWork;
    napi_status status = napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void* data) {
            // 执行工作
            auto context = static_cast<AsyncContext*>(data);
            ::OHOS::CliMgr::CLIManagerClient client;
            std::vector<::OHOS::CliMgr::ToolInfo> tools;
            context->errorCode = client.QueryTools(tools);
        },
        [](napi_env env, napi_status status, void* data) {
            // 完成回调
            auto context = static_cast<AsyncContext*>(data);

            napi_value callback;
            napi_get_reference_value(env, context->callbackRef, &callback);

            napi_value argv[2];
            if (context->errorCode != ErrCode::OK) {
                argv[0] = CreateError(env, context->errorCode, "QueryTools failed");
                argv[1] = nullptr;
            } else {
                argv[0] = nullptr;  // 无错误
                // TODO: 创建工具数组
                napi_get_undefined(env, &argv[1]);
            }

            napi_value result;
            napi_call_function(env, nullptr, callback, 2, argv, &result);

            napi_delete_reference(env, context->callbackRef);
            delete context;
        },
        asyncContext,
        &asyncWork
    );

    if (status != napi_ok) {
        delete asyncContext;
        return CreateError(env, ErrCode::ERR_NO_MEMORY, "Failed to create async work");
    }

    napi_queue_async_work(env, asyncWork);

    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

napi_value JSCliManager::QueryToolByName(napi_env env, napi_callback_info info)
{
    TAG_LOGD("JSCliManager", "QueryToolByName called");

    size_t argc = 2;
    napi_value argv[2];
    napi_value thisVar = nullptr;

    if (napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to get cb info");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Invalid callback info");
    }

    // 检查参数
    if (argc < 2) {
        TAG_LOGE("JSCliManager", "Missing parameters");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Missing parameters");
    }

    // 读取工具名称
    size_t strSize = 0;
    if (napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strSize) != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to get string size");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Invalid tool name");
    }

    std::string toolName(strSize, '\0');
    if (napi_get_value_string_utf8(env, argv[0], &toolName[0], strSize + 1, &strSize) != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to get string value");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Invalid tool name");
    }

    // 检查回调函数
    napi_valuetype valueType;
    if (napi_typeof(env, argv[1], &valueType) != napi_ok || valueType != napi_function) {
        TAG_LOGE("JSCliManager", "Second parameter must be a function");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Second parameter must be a function");
    }

    // 创建异步工作
    napi_value resourceName;
    napi_create_string_utf8(env, "QueryToolByName", NAPI_AUTO_LENGTH, &resourceName);

    auto asyncContext = new (std::nothrow) AsyncContext();
    if (asyncContext == nullptr) {
        TAG_LOGE("JSCliManager", "Failed to create async context");
        return CreateError(env, ErrCode::ERR_NO_MEMORY, "Out of memory");
    }

    asyncContext->env = env;
    asyncContext->toolName = toolName;
    napi_create_reference(env, argv[1], 1, &asyncContext->callbackRef);

    napi_value asyncWork;
    napi_status status = napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void* data) {
            auto context = static_cast<AsyncContext*>(data);
            ::OHOS::CliMgr::CLIManagerClient client;
            ::OHOS::CliMgr::ToolInfo tool;
            context->errorCode = client.QueryToolByName(context->toolName, tool);
        },
        [](napi_env env, napi_status status, void* data) {
            auto context = static_cast<AsyncContext*>(data);

            napi_value callback;
            napi_get_reference_value(env, context->callbackRef, &callback);

            napi_value argv[2];
            if (context->errorCode != ErrCode::OK) {
                argv[0] = CreateError(env, context->errorCode, "QueryToolByName failed");
                argv[1] = nullptr;
            } else {
                argv[0] = nullptr;
                napi_get_undefined(env, &argv[1]);
            }

            napi_value result;
            napi_call_function(env, nullptr, callback, 2, argv, &result);

            napi_delete_reference(env, context->callbackRef);
            delete context;
        },
        asyncContext,
        &asyncWork
    );

    if (status != napi_ok) {
        delete asyncContext;
        return CreateError(env, ErrCode::ERR_NO_MEMORY, "Failed to create async work");
    }

    napi_queue_async_work(env, asyncWork);

    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

napi_value JSCliManager::QueryToolsByPermission(napi_env env, napi_callback_info info)
{
    TAG_LOGD("JSCliManager", "QueryToolsByPermission called");
    // 实现与 QueryTools 类似
    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

napi_value JSCliManager::ExecTool(napi_env env, napi_callback_info info)
{
    TAG_LOGD("JSCliManager", "ExecTool called");

    size_t argc = 3;
    napi_value argv[3];
    napi_value thisVar = nullptr;

    if (napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to get cb info");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Invalid callback info");
    }

    // 检查参数
    if (argc < 3) {
        TAG_LOGE("JSCliManager", "Missing parameters");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Missing parameters");
    }

    // 读取工具名称
    size_t strSize = 0;
    if (napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strSize) != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to get tool name");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Invalid tool name");
    }

    std::string toolName(strSize, '\0');
    napi_get_value_string_utf8(env, argv[0], &toolName[0], strSize + 1, &strSize);

    // 读取参数对象
    Json::Value arguments;
    if (!ReadArguments(env, argv[1], arguments)) {
        TAG_LOGE("JSCliManager", "Failed to read arguments");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Invalid arguments");
    }

    // 读取选项
    ::OHOS::CliMgr::ExecOptions options;
    napi_value optionsObj;
    bool hasOptions = false;
    napi_has_named_property(env, argv[2], "options", &hasOptions);
    if (hasOptions) {
        napi_get_named_property(env, argv[2], "options", &optionsObj);
        ReadExecOptions(env, optionsObj, options);
    }

    // 读取回调函数
    napi_value callback;
    napi_get_named_property(env, argv[2], "callback", &callback);

    // 创建异步工作
    napi_value resourceName;
    napi_create_string_utf8(env, "ExecTool", NAPI_AUTO_LENGTH, &resourceName);

    auto asyncContext = new (std::nothrow) AsyncContext();
    if (asyncContext == nullptr) {
        TAG_LOGE("JSCliManager", "Failed to create async context");
        return CreateError(env, ErrCode::ERR_NO_MEMORY, "Out of memory");
    }

    asyncContext->env = env;
    asyncContext->toolName = toolName;
    asyncContext->arguments = arguments;
    asyncContext->timeoutMs = options.timeoutMs;
    asyncContext->workingDir = options.workingDir;
    napi_create_reference(env, callback, 1, &asyncContext->callbackRef);

    napi_value asyncWork;
    napi_status status = napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void* data) {
            auto context = static_cast<AsyncContext*>(data);
            ::OHOS::CliMgr::CLIManagerClient client;
            ::OHOS::CliMgr::ExecutionResult result;
            ::OHOS::CliMgr::ExecOptions options;
            options.timeoutMs = context->timeoutMs;
            options.workingDir = context->workingDir;
            context->errorCode = client.ExecTool(context->toolName,
                                                context->arguments,
                                                result,
                                                options);
        },
        [](napi_env env, napi_status status, void* data) {
            auto context = static_cast<AsyncContext*>(data);

            napi_value callback;
            napi_get_reference_value(env, context->callbackRef, &callback);

            napi_value argv[2];
            if (context->errorCode != ErrCode::OK) {
                argv[0] = CreateError(env, context->errorCode, "ExecTool failed");
                argv[1] = nullptr;
            } else {
                argv[0] = nullptr;
                napi_get_undefined(env, &argv[1]);
            }

            napi_value result;
            napi_call_function(env, nullptr, callback, 2, argv, &result);

            napi_delete_reference(env, context->callbackRef);
            delete context;
        },
        asyncContext,
        &asyncWork
    );

    if (status != napi_ok) {
        delete asyncContext;
        return CreateError(env, ErrCode::ERR_NO_MEMORY, "Failed to create async work");
    }

    napi_queue_async_work(env, asyncWork);

    napi_value undefined;
    napi_get_undefined(env, &undefined);
    return undefined;
}

napi_value JSCliManager::ExecToolAsync(napi_env env, napi_callback_info info)
{
    TAG_LOGD("JSCliManager", "ExecToolAsync called");

    size_t argc = 4;
    napi_value argv[4];
    napi_value thisVar = nullptr;

    if (napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to get cb info");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Invalid callback info");
    }

    // 检查参数
    if (argc < 4) {
        TAG_LOGE("JSCliManager", "Missing parameters");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Missing parameters");
    }

    // 读取工具名称
    size_t strSize = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strSize);

    std::string toolName(strSize, '\0');
    napi_get_value_string_utf8(env, argv[0], &toolName[0], strSize + 1, &strSize);

    // 读取参数对象
    Json::Value arguments;
    ReadArguments(env, argv[1], arguments);

    // 读取事件回调
    napi_value eventCallback = argv[2];

    // 读取选项
    ::OHOS::CliMgr::ExecOptions options;
    ReadExecOptions(env, argv[3], options);

    // 创建 Promise
    napi_value promise;
    napi_deferred deferred;
    napi_create_promise(env, &deferred, &promise);

    auto asyncContext = new (std::nothrow) AsyncContext();
    if (asyncContext == nullptr) {
        return CreateError(env, ErrCode::ERR_NO_MEMORY, "Out of memory");
    }

    asyncContext->env = env;
    asyncContext->toolName = toolName;
    asyncContext->arguments = arguments;
    asyncContext->timeoutMs = options.timeoutMs;
    asyncContext->workingDir = options.workingDir;
    asyncContext->deferred = deferred;

    // 创建异步工作
    napi_value resourceName;
    napi_create_string_utf8(env, "ExecToolAsync", NAPI_AUTO_LENGTH, &resourceName);

    napi_value asyncWork;
    napi_status status = napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void* data) {
            auto context = static_cast<AsyncContext*>(data);
            ::OHOS::CliMgr::CLIManagerClient client;

            // 创建 C++ 回调
            auto callback = [context](const std::string& taskId,
                                     const ::OHOS::CliMgr::ToolEvent& event) {
                // TODO: 将事件发送到 JS 层
            };

            ::OHOS::CliMgr::ExecOptions options;
            options.timeoutMs = context->timeoutMs;
            options.workingDir = context->workingDir;

            context->errorCode = client.ExecToolAsync(context->toolName,
                                                     context->arguments,
                                                     callback,
                                                     context->taskId,
                                                     options);
        },
        [](napi_env env, napi_status status, void* data) {
            auto context = static_cast<AsyncContext*>(data);

            if (context->errorCode != ErrCode::OK) {
                napi_value error = CreateError(env, context->errorCode,
                                             "ExecToolAsync failed");
                napi_reject_deferred(env, context->deferred, error);
            } else {
                napi_value taskId;
                napi_create_string_utf8(env, context->taskId.c_str(),
                                       NAPI_AUTO_LENGTH, &taskId);
                napi_resolve_deferred(env, context->deferred, taskId);
            }

            napi_delete_async_work(env, context->deferred);
            delete context;
        },
        asyncContext,
        &asyncWork
    );

    if (status != napi_ok) {
        delete asyncContext;
        return CreateError(env, ErrCode::ERR_NO_MEMORY, "Failed to create async work");
    }

    napi_queue_async_work(env, asyncWork);

    return promise;
}

napi_value JSCliManager::CancelTask(napi_env env, napi_callback_info info)
{
    TAG_LOGD("JSCliManager", "CancelTask called");

    size_t argc = 2;
    napi_value argv[2];
    napi_value thisVar = nullptr;

    if (napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to get cb info");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Invalid callback info");
    }

    // 读取任务 ID
    size_t strSize = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strSize);

    std::string taskId(strSize, '\0');
    napi_get_value_string_utf8(env, argv[0], &taskId[0], strSize + 1, &strSize);

    // 调用客户端
    ::OHOS::CliMgr::CLIManagerClient client;
    int32_t ret = client.CancelTask(taskId);

    // 返回结果
    napi_value result;
    napi_get_boolean(env, ret == ErrCode::OK, &result);
    return result;
}

napi_value JSCliManager::QueryTaskStatus(napi_env env, napi_callback_info info)
{
    TAG_LOGD("JSCliManager", "QueryTaskStatus called");

    size_t argc = 2;
    napi_value argv[2];
    napi_value thisVar = nullptr;

    if (napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to get cb info");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Invalid callback info");
    }

    // 读取任务 ID
    size_t strSize = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strSize);

    std::string taskId(strSize, '\0');
    napi_get_value_string_utf8(env, argv[0], &taskId[0], strSize + 1, &strSize);

    // 调用客户端
    ::OHOS::CliMgr::CLIManagerClient client;
    ::OHOS::CliMgr::TaskStatus status;
    int32_t ret = client.QueryTaskStatus(taskId, status);

    if (ret != ErrCode::OK) {
        return CreateError(env, ret, "QueryTaskStatus failed");
    }

    return CreateTaskStatusObject(env, status);
}

bool JSCliManager::ReadExecOptions(napi_env env, napi_value options,
                                  ::OHOS::CliMgr::ExecOptions& execOptions)
{
    if (options == nullptr) {
        return false;
    }

    // 读取 timeout
    napi_value timeoutValue;
    if (napi_get_named_property(env, options, "timeout", &timeoutValue) == napi_ok) {
        uint32_t timeout = 0;
        if (napi_get_value_uint32(env, timeoutValue, &timeout) == napi_ok) {
            execOptions.timeoutMs = timeout;
        }
    }

    // 读取 workingDir
    napi_value cwdValue;
    if (napi_get_named_property(env, options, "cwd", &cwdValue) == napi_ok) {
        size_t strSize = 0;
        if (napi_get_value_string_utf8(env, cwdValue, nullptr, 0, &strSize) == napi_ok) {
            std::string cwd(strSize, '\0');
            napi_get_value_string_utf8(env, cwdValue, &cwd[0], strSize + 1, &strSize);
            execOptions.workingDir = cwd;
        }
    }

    return true;
}

bool JSCliManager::ReadArguments(napi_env env, napi_value args, Json::Value& outArgs)
{
    // TODO: 实现 JS 对象到 JSON 的转换
    outArgs = Json::objectValue;
    return true;
}

napi_value JSCliManager::CreateToolInfoObject(napi_env env,
                                             const ::OHOS::CliMgr::ToolInfo& tool)
{
    napi_value obj;
    napi_create_object(env, &obj);

    napi_value name;
    napi_create_string_utf8(env, tool.name.c_str(), NAPI_AUTO_LENGTH, &name);
    napi_set_named_property(env, obj, "name", name);

    napi_value version;
    napi_create_string_utf8(env, tool.version.c_str(), NAPI_AUTO_LENGTH, &version);
    napi_set_named_property(env, obj, "version", &version);

    napi_value description;
    napi_create_string_utf8(env, tool.description.c_str(), NAPI_AUTO_LENGTH, &description);
    napi_set_named_property(env, obj, "description", description);

    napi_value type;
    napi_create_string_utf8(env,
                           tool.type == ::OHOS::CliMgr::ToolType::SHORT_LIVED ?
                           "short-lived" : "long-lived",
                           NAPI_AUTO_LENGTH, &type);
    napi_set_named_property(env, obj, "type", type);

    return obj;
}

napi_value JSCliManager::CreateExecResultObject(napi_env env,
                                               const ::OHOS::CliMgr::ExecutionResult& result)
{
    napi_value obj;
    napi_create_object(env, &obj);

    napi_value taskId;
    napi_create_string_utf8(env, result.taskId.c_str(), NAPI_AUTO_LENGTH, &taskId);
    napi_set_named_property(env, obj, "taskId", taskId);

    napi_value exitCode;
    napi_create_int32(env, result.exitCode, &exitCode);
    napi_set_named_property(env, obj, "exitCode", exitCode);

    napi_value stdout;
    napi_create_string_utf8(env, result.stdout.c_str(), NAPI_AUTO_LENGTH, &stdout);
    napi_set_named_property(env, obj, "stdout", stdout);

    napi_value stderr;
    napi_create_string_utf8(env, result.stderr.c_str(), NAPI_AUTO_LENGTH, &stderr);
    napi_set_named_property(env, obj, "stderr", stderr);

    napi_value timedOut;
    napi_get_boolean(env, result.timedOut, &timedOut);
    napi_set_named_property(env, obj, "timedOut", timedOut);

    napi_value executionTime;
    napi_create_uint32(env, result.executionTimeMs, &executionTime);
    napi_set_named_property(env, obj, "executionTime", executionTime);

    return obj;
}

napi_value JSCliManager::CreateTaskStatusObject(napi_env env,
                                               const ::OHOS::CliMgr::TaskStatus& status)
{
    napi_value obj;
    napi_create_object(env, &obj);

    napi_value taskId;
    napi_create_string_utf8(env, status.taskId.c_str(), NAPI_AUTO_LENGTH, &taskId);
    napi_set_named_property(env, obj, "taskId", taskId);

    napi_value toolName;
    napi_create_string_utf8(env, status.toolName.c_str(), NAPI_AUTO_LENGTH, &toolName);
    napi_set_named_property(env, obj, "toolName", toolName);

    napi_value statusStr;
    napi_create_string_utf8(env, status.status.c_str(), NAPI_AUTO_LENGTH, &statusStr);
    napi_set_named_property(env, obj, "status", statusStr);

    napi_value startTime;
    napi_create_uint64(env, status.startTime, &startTime);
    napi_set_named_property(env, obj, "startTime", startTime);

    if (status.endTime > 0) {
        napi_value endTime;
        napi_create_uint64(env, status.endTime, &endTime);
        napi_set_named_property(env, obj, "endTime", endTime);
    }

    if (status.exitCode >= 0) {
        napi_value exitCode;
        napi_create_int32(env, status.exitCode, &exitCode);
        napi_set_named_property(env, obj, "exitCode", exitCode);
    }

    return obj;
}

napi_value JSCliManager::CreateError(napi_env env, int32_t errorCode,
                                    const std::string& errorMessage)
{
    napi_value code;
    napi_create_int32(env, errorCode, &code);

    napi_value message;
    napi_create_string_utf8(env, errorMessage.c_str(), NAPI_AUTO_LENGTH, &message);

    napi_value error;
    napi_create_error(env, code, message, &error);

    return error;
}

} // namespace JS
} // namespace CliMgr
} // namespace OHOS
