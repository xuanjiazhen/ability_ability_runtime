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
static constexpr const char* METHOD_EXEC_TOOL = "execTool";
static constexpr const char* METHOD_SUBSCRIBE_SESSION = "subscribeSession";
static constexpr const char* METHOD_CANCEL_SESSION = "cancelSession";
static constexpr const char* METHOD_QUERY_SESSION_STATUS = "querySessionStatus";

napi_value JSCliManager::Init(napi_env env, napi_value exports)
{
    TAG_LOGI("JSCliManager", "Init called");

    // 定义类属性
    napi_property_descriptor properties[] = {
        DECLARE_NAPI_STATIC_METHOD(METHOD_QUERY_TOOLS, QueryTools),
        DECLARE_NAPI_STATIC_METHOD(METHOD_EXEC_TOOL, ExecTool),
        DECLARE_NAPI_STATIC_METHOD(METHOD_SUBSCRIBE_SESSION, SubscribeSession),
        DECLARE_NAPI_STATIC_METHOD(METHOD_CANCEL_SESSION, CancelSession),
        DECLARE_NAPI_STATIC_METHOD(METHOD_QUERY_SESSION_STATUS, QuerySessionStatus),
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
            context->errorCode = client.QueryTools(context->tools);
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

                // 创建工具数组
                napi_value toolsArray;
                napi_create_array(env, &toolsArray);

                for (size_t i = 0; i < context->tools.size(); ++i) {
                    napi_value toolObj = CreateToolInfoObject(env, context->tools[i]);
                    napi_set_element(env, toolsArray, i, toolObj);
                }

                argv[1] = toolsArray;
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

    // 参数: toolName (string), args (object), options (object, optional)
    if (argc < 2) {
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
    if (argc >= 3) {
        ReadExecOptions(env, argv[2], options);
    }

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
    asyncContext->background = options.background;
    asyncContext->yieldMs = options.yieldMs;
    asyncContext->timeoutMs = options.timeoutMs;
    asyncContext->workingDir = options.workingDir;
    asyncContext->deferred = deferred;

    // 创建异步工作
    napi_value resourceName;
    napi_create_string_utf8(env, "ExecTool", NAPI_AUTO_LENGTH, &resourceName);

    napi_value asyncWork;
    napi_status status = napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void* data) {
            auto context = static_cast<AsyncContext*>(data);
            ::OHOS::CliMgr::CLIManagerClient client;
            ::OHOS::CliMgr::ExecOptions options;
            options.background = context->background;
            options.yieldMs = context->yieldMs;
            options.timeoutMs = context->timeoutMs;
            options.workingDir = context->workingDir;
            context->errorCode = client.ExecTool(context->toolName,
                                                context->arguments,
                                                context->sessionInfo,
                                                options);
        },
        [](napi_env env, napi_status status, void* data) {
            auto context = static_cast<AsyncContext*>(data);

            if (context->errorCode != ErrCode::OK) {
                napi_value error = CreateError(env, context->errorCode, "ExecTool failed");
                napi_reject_deferred(env, context->deferred, error);
            } else {
                napi_value sessionObj = CreateSessionInfoObject(env, context->sessionInfo);
                napi_resolve_deferred(env, context->deferred, sessionObj);
            }

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

napi_value JSCliManager::SubscribeSession(napi_env env, napi_callback_info info)
{
    TAG_LOGD("JSCliManager", "SubscribeSession called");

    size_t argc = 2;
    napi_value argv[2];
    napi_value thisVar = nullptr;

    if (napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to get cb info");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Invalid callback info");
    }

    // 参数: sessionId (string), callback (function)
    if (argc < 2) {
        TAG_LOGE("JSCliManager", "Missing parameters");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Missing parameters");
    }

    // 读取会话 ID
    size_t strSize = 0;
    if (napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strSize) != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to get session ID");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Invalid session ID");
    }

    std::string sessionId(strSize, '\0');
    napi_get_value_string_utf8(env, argv[0], &sessionId[0], strSize + 1, &strSize);

    // 检查回调函数
    napi_valuetype valueType;
    if (napi_typeof(env, argv[1], &valueType) != napi_ok || valueType != napi_function) {
        TAG_LOGE("JSCliManager", "Second parameter must be a function");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Second parameter must be a function");
    }

    // 创建 Promise
    napi_value promise;
    napi_deferred deferred;
    napi_create_promise(env, &deferred, &promise);

    auto asyncContext = new (std::nothrow) AsyncContext();
    if (asyncContext == nullptr) {
        return CreateError(env, ErrCode::ERR_NO_MEMORY, "Out of memory");
    }

    asyncContext->env = env;
    asyncContext->sessionId = sessionId;
    asyncContext->deferred = deferred;

    // 保存回调引用
    napi_create_reference(env, argv[1], 1, &asyncContext->callbackRef);

    // TODO: 实现 C++ 回调，将事件发送到 JS 层
    // 需要维护一个 sessionId -> callbackRef 的映射

    // 调用客户端
    ::OHOS::CliMgr::CLIManagerClient client;
    auto callback = [](const std::string& sessionId, const ::OHOS::CliMgr::ToolEvent& event) {
        // TODO: 将事件发送到 JS 层
    };

    asyncContext->errorCode = client.SubscribeSession(sessionId, callback);

    // 模拟异步完成
    napi_value resourceName;
    napi_create_string_utf8(env, "SubscribeSession", NAPI_AUTO_LENGTH, &resourceName);

    napi_value asyncWork;
    napi_status status = napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void* data) {
            // 工作已在上面完成
        },
        [](napi_env env, napi_status status, void* data) {
            auto context = static_cast<AsyncContext*>(data);

            if (context->errorCode != ErrCode::OK) {
                napi_value error = CreateError(env, context->errorCode, "SubscribeSession failed");
                napi_reject_deferred(env, context->deferred, error);
            } else {
                napi_value success;
                napi_get_boolean(env, true, &success);
                napi_resolve_deferred(env, context->deferred, success);
            }

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

    return promise;
}

napi_value JSCliManager::CancelSession(napi_env env, napi_callback_info info)
{
    TAG_LOGD("JSCliManager", "CancelSession called");

    size_t argc = 2;
    napi_value argv[2];
    napi_value thisVar = nullptr;

    if (napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to get cb info");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Invalid callback info");
    }

    // 读取会话 ID
    size_t strSize = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strSize);

    std::string sessionId(strSize, '\0');
    napi_get_value_string_utf8(env, argv[0], &sessionId[0], strSize + 1, &strSize);

    // 调用客户端
    ::OHOS::CliMgr::CLIManagerClient client;
    int32_t ret = client.CancelSession(sessionId);

    // 返回结果
    napi_value result;
    napi_get_boolean(env, ret == ErrCode::OK, &result);
    return result;
}

napi_value JSCliManager::QuerySessionStatus(napi_env env, napi_callback_info info)
{
    TAG_LOGD("JSCliManager", "QuerySessionStatus called");

    size_t argc = 2;
    napi_value argv[2];
    napi_value thisVar = nullptr;

    if (napi_get_cb_info(env, info, &argc, argv, &thisVar, nullptr) != napi_ok) {
        TAG_LOGE("JSCliManager", "Failed to get cb info");
        return CreateError(env, ErrCode::ERR_INVALID_ARGS, "Invalid callback info");
    }

    // 读取会话 ID
    size_t strSize = 0;
    napi_get_value_string_utf8(env, argv[0], nullptr, 0, &strSize);

    std::string sessionId(strSize, '\0');
    napi_get_value_string_utf8(env, argv[0], &sessionId[0], strSize + 1, &strSize);

    // 调用客户端
    ::OHOS::CliMgr::CLIManagerClient client;
    ::OHOS::CliMgr::SessionInfo status;
    int32_t ret = client.QuerySessionStatus(sessionId, status);

    if (ret != ErrCode::OK) {
        return CreateError(env, ret, "QuerySessionStatus failed");
    }

    return CreateSessionInfoObject(env, status);
}

bool JSCliManager::ReadExecOptions(napi_env env, napi_value options,
                                  ::OHOS::CliMgr::ExecOptions& execOptions)
{
    if (options == nullptr) {
        return false;
    }

    // 读取 background
    napi_value backgroundValue;
    if (napi_get_named_property(env, options, "background", &backgroundValue) == napi_ok) {
        bool background = false;
        if (napi_get_value_bool(env, backgroundValue, &background) == napi_ok) {
            execOptions.background = background;
        }
    }

    // 读取 yieldMs
    napi_value yieldMsValue;
    if (napi_get_named_property(env, options, "yieldMs", &yieldMsValue) == napi_ok) {
        uint32_t yieldMs = 30000;
        if (napi_get_value_uint32(env, yieldMsValue, &yieldMs) == napi_ok) {
            execOptions.yieldMs = yieldMs;
        }
    }

    // 读取 timeout
    napi_value timeoutValue;
    if (napi_get_named_property(env, options, "timeout", &timeoutValue) == napi_ok) {
        uint32_t timeout = 0;
        if (napi_get_value_uint32(env, timeoutValue, &timeout) == napi_ok) {
            execOptions.timeoutMs = timeout;
        }
    }

    // 读取 workingDir (cwd)
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

/**
 * @brief 将 Json::Value 转换为 napi_value
 */
static napi_value JsonToNapiValue(napi_env env, const Json::Value& json)
{
    if (json.isNull()) {
        napi_value result;
        napi_get_null(env, &result);
        return result;
    }

    if (json.isBool()) {
        napi_value result;
        napi_get_boolean(env, json.asBool(), &result);
        return result;
    }

    if (json.isInt()) {
        napi_value result;
        napi_create_int32(env, json.asInt(), &result);
        return result;
    }

    if (json.isUInt()) {
        napi_value result;
        napi_create_uint32(env, json.asUInt(), &result);
        return result;
    }

    if (json.isInt64()) {
        napi_value result;
        napi_create_int64(env, json.asInt64(), &result);
        return result;
    }

    if (json.isUInt64()) {
        napi_value result;
        napi_create_double(env, json.asUInt64(), &result);
        return result;
    }

    if (json.isDouble()) {
        napi_value result;
        napi_create_double(env, json.asDouble(), &result);
        return result;
    }

    if (json.isString()) {
        napi_value result;
        std::string str = json.asString();
        napi_create_string_utf8(env, str.c_str(), str.length(), &result);
        return result;
    }

    if (json.isArray()) {
        napi_value result;
        napi_create_array(env, &result);

        for (Json::ArrayIndex i = 0; i < json.size(); ++i) {
            napi_value element = JsonToNapiValue(env, json[i]);
            napi_set_element(env, result, i, element);
        }

        return result;
    }

    if (json.isObject()) {
        napi_value result;
        napi_create_object(env, &result);

        for (const auto& key : json.getMemberNames()) {
            napi_value value = JsonToNapiValue(env, json[key]);
            napi_set_named_property(env, result, key.c_str(), value);
        }

        return result;
    }

    // 默认返回 undefined
    napi_value result;
    napi_get_undefined(env, &result);
    return result;
}

napi_value JSCliManager::CreateToolInfoObject(napi_env env,
                                             const ::OHOS::CliMgr::ToolInfo& tool)
{
    napi_value obj;
    napi_create_object(env, &obj);

    // 基础字段
    napi_value name;
    napi_create_string_utf8(env, tool.name.c_str(), NAPI_AUTO_LENGTH, &name);
    napi_set_named_property(env, obj, "name", name);

    napi_value version;
    napi_create_string_utf8(env, tool.version.c_str(), NAPI_AUTO_LENGTH, &version);
    napi_set_named_property(env, obj, "version", version);

    napi_value description;
    napi_create_string_utf8(env, tool.description.c_str(), NAPI_AUTO_LENGTH, &description);
    napi_set_named_property(env, obj, "description", description);

    napi_value executablePath;
    napi_create_string_utf8(env, tool.executablePath.c_str(), NAPI_AUTO_LENGTH, &executablePath);
    napi_set_named_property(env, obj, "executablePath", executablePath);

    // 权限数组
    napi_value permissions;
    napi_create_array(env, &permissions);
    for (size_t i = 0; i < tool.permissions.size(); ++i) {
        napi_value perm;
        napi_create_string_utf8(env, tool.permissions[i].c_str(), NAPI_AUTO_LENGTH, &perm);
        napi_set_element(env, permissions, i, perm);
    }
    napi_set_named_property(env, obj, "permissions", permissions);

    // inputSchema (JSON 对象)
    napi_value inputSchema = JsonToNapiValue(env, tool.inputSchema);
    napi_set_named_property(env, obj, "inputSchema", inputSchema);

    // outputSchema (JSON 对象)
    napi_value outputSchema = JsonToNapiValue(env, tool.outputSchema);
    napi_set_named_property(env, obj, "outputSchema", outputSchema);

    // argMapping (JSON 对象) - 新增
    napi_value argMapping = JsonToNapiValue(env, tool.argMapping);
    napi_set_named_property(env, obj, "argMapping", argMapping);

    // eventSchemas (JSON 对象) - 新增
    napi_value eventSchemas = JsonToNapiValue(env, tool.eventSchemas);
    napi_set_named_property(env, obj, "eventSchemas", eventSchemas);

    // timeoutMs
    napi_value timeoutMs;
    napi_create_uint32(env, tool.timeoutMs, &timeoutMs);
    napi_set_named_property(env, obj, "timeoutMs", timeoutMs);

    // eventTypes (数组)
    napi_value eventTypes;
    napi_create_array(env, &eventTypes);
    for (size_t i = 0; i < tool.eventTypes.size(); ++i) {
        napi_value event;
        napi_create_string_utf8(env, tool.eventTypes[i].c_str(), NAPI_AUTO_LENGTH, &event);
        napi_set_element(env, eventTypes, i, event);
    }
    napi_set_named_property(env, obj, "eventTypes", eventTypes);

    // registeredTime
    napi_value registeredTime;
    napi_create_uint64(env, tool.registeredTime, &registeredTime);
    napi_set_named_property(env, obj, "registeredTime", registeredTime);

    // enabled
    napi_value enabled;
    napi_get_boolean(env, tool.enabled, &enabled);
    napi_set_named_property(env, obj, "enabled", enabled);

    // hasSubcommands (布尔) - 新增
    napi_value hasSubcommands;
    napi_get_boolean(env, tool.hasSubcommands, &hasSubcommands);
    napi_set_named_property(env, obj, "hasSubcommands", hasSubcommands);

    // subcommands (对象) - 新增
    if (tool.hasSubcommands) {
        napi_value subcommands;
        napi_create_object(env, &subcommands);

        for (const auto& [name, subcmd] : tool.subcommands) {
            napi_value subcmdObj;
            napi_create_object(env, &subcmdObj);

            // 子命令描述
            napi_value subcmdDesc;
            napi_create_string_utf8(env, subcmd.description.c_str(), NAPI_AUTO_LENGTH, &subcmdDesc);
            napi_set_named_property(env, subcmdObj, "description", subcmdDesc);

            // 子命令 Schema
            napi_value subInputSchema = JsonToNapiValue(env, subcmd.inputSchema);
            napi_set_named_property(env, subcmdObj, "inputSchema", subInputSchema);

            napi_value subOutputSchema = JsonToNapiValue(env, subcmd.outputSchema);
            napi_set_named_property(env, subcmdObj, "outputSchema", subOutputSchema);

            napi_value subArgMapping = JsonToNapiValue(env, subcmd.argMapping);
            napi_set_named_property(env, subcmdObj, "argMapping", subArgMapping);

            napi_value subEventSchemas = JsonToNapiValue(env, subcmd.eventSchemas);
            napi_set_named_property(env, subcmdObj, "eventSchemas", subEventSchemas);

            // 子命令事件类型
            napi_value subEventTypes;
            napi_create_array(env, &subEventTypes);
            for (size_t i = 0; i < subcmd.eventTypes.size(); ++i) {
                napi_value event;
                napi_create_string_utf8(env, subcmd.eventTypes[i].c_str(), NAPI_AUTO_LENGTH, &event);
                napi_set_element(env, subEventTypes, i, event);
            }
            napi_set_named_property(env, subcmdObj, "eventTypes", subEventTypes);

            // 添加子命令到 subcommands 对象
            napi_set_named_property(env, subcommands, name.c_str(), subcmdObj);
        }

        napi_set_named_property(env, obj, "subcommands", subcommands);
    }

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

napi_value JSCliManager::CreateSessionInfoObject(napi_env env,
                                                const ::OHOS::CliMgr::SessionInfo& session)
{
    napi_value obj;
    napi_create_object(env, &obj);

    napi_value sessionId;
    napi_create_string_utf8(env, session.sessionId.c_str(), NAPI_AUTO_LENGTH, &sessionId);
    napi_set_named_property(env, obj, "sessionId", sessionId);

    napi_value toolName;
    napi_create_string_utf8(env, session.toolName.c_str(), NAPI_AUTO_LENGTH, &toolName);
    napi_set_named_property(env, obj, "toolName", toolName);

    napi_value status;
    napi_create_string_utf8(env, session.status.c_str(), NAPI_AUTO_LENGTH, &status);
    napi_set_named_property(env, obj, "status", status);

    napi_value startTime;
    napi_create_uint64(env, session.startTime, &startTime);
    napi_set_named_property(env, obj, "startTime", startTime);

    if (session.endTime > 0) {
        napi_value endTime;
        napi_create_uint64(env, session.endTime, &endTime);
        napi_set_named_property(env, obj, "endTime", endTime);
    }

    // 如果有执行结果，添加 result 字段
    if (session.result != nullptr) {
        napi_value resultObj = CreateExecResultObject(env, *session.result);
        napi_set_named_property(env, obj, "result", resultObj);
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
