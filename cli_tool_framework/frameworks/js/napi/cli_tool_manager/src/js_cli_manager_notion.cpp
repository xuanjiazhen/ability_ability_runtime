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

/*
 * 文件整体作用：
 * 这个文件是 CliManager 的 JS/ArkTS 到 C++ 的 N-API 适配层。
 *
 * 可以把它理解成三层中的“中间翻译层”：
 *
 *   ArkTS/JS 侧调用：
 *     cliManager.execTool(...)
 *     cliManager.execCmd(...)
 *     cliManager.querySession(...)
 *
 *          ↓ 通过 N-API 进入 C++
 *
 *   本文件 JSCliManager：
 *     1. 接收 JS 传进来的 napi_value 参数；
 *     2. 校验参数数量和类型；
 *     3. 把 JS 参数转换成 C++ 结构体；
 *     4. 调用 CliToolMGRClient；
 *     5. 把 C++ 返回结果转换成 JS 对象；
 *     6. 通过 Promise resolve/reject 返回给 JS。
 *
 *          ↓ 调用真正的客户端层
 *
 *   CliToolMGRClient：
 *     真正负责和底层 CLI Tool Manager 服务通信。
 */

#include "js_cli_manager.h" // 当前 JS 绑定类 JSCliManager 的声明，一般包含静态入口函数和 OnXXX 实现声明。

#include <string> // 使用 std::string。

#include "cli_error_code.h"                 // CLI 工具相关错误码，例如 ERR_OK。
#include "cli_manager_error_utils.h"        // 将 native 层错误码转换成 JS Error 的工具函数。
#include "cli_session_info.h"               // CliSessionInfo，表示一次 CLI 会话的信息。
#include "cli_tool_mgr_client.h"            // CliToolMGRClient，真正调用底层 CLI Manager 服务的客户端。
#include "exec_cmd_param.h"                 // ExecCmdParam，执行命令 execCmd 时使用的参数结构体。
#include "exec_result.h"                    // 执行结果相关结构定义。
#include "hilog_tag_wrapper.h"              // 日志宏 TAG_LOGD/TAG_LOGE/TAG_LOGW 等。
#include "js_cli_event_handler_manager.h"   // JS 事件线程/任务投递管理器，用于把回调切回 JS 可安全执行的线程。
#include "js_cli_manager_utils.h"           // CliManager JS 转换工具，例如 CreateJsToolInfo、UnwrapExecOptions 等。
#include "js_cli_session_event_callback.h"  // JS session 事件回调的 C++ 封装类。
#include "js_error_utils.h"                 // JS 错误抛出工具，例如 ThrowInvalidParamError。
#include "napi_common_util.h"               // N-API 通用工具，例如 CreateJsUndefined、NapiAsyncTask。
#include "napi_common_want.h"               // Want/WantParams 的 JS 与 C++ 转换工具。

// 使用 AbilityRuntime 命名空间中的一些 N-API 辅助类，比如 NapiAsyncTask、HandleScope、HandleEscape。
using namespace OHOS::AbilityRuntime;

namespace OHOS {
namespace CliTool {

// 匿名 namespace：
// 这里面的内容只在当前 cpp 文件可见，相当于“文件私有”。
namespace {

// 用具名常量代替裸数字，避免代码中直接写 argv[0]、argv[1] 这种魔法数字。
// INDEX_ZERO 表示第 0 个参数，INDEX_ONE 表示第 1 个参数，以此类推。
constexpr int32_t INDEX_ZERO = 0;
constexpr int32_t INDEX_ONE = 1;
constexpr int32_t INDEX_TWO = 2;
constexpr int32_t INDEX_THREE = 3;
constexpr int32_t INDEX_FOUR = 4;

/*
 * DispatchCliTool：派发 execTool 调用。
 *
 * 参数说明：
 *   param：已经从 JS 参数解析出来的 C++ 执行参数，类型是 ExecToolParam。
 *   env：N-API 环境句柄，代表当前 JS 运行环境。
 *   asyncTask：Promise 对应的异步任务对象，用于 resolve 或 reject。
 *
 * 返回值：
 *   int32_t 错误码。ERR_OK 表示调用底层接口成功发起；非 ERR_OK 表示发起失败。
 *
 * 注意：
 *   这个函数本身不是最终执行工具的地方，它只是调用 CliToolMGRClient::ExecTool。
 *   真正结果会通过 replyCallback 异步回来。
 */
int32_t DispatchCliTool(const ExecToolParam &param, napi_env env,
    std::shared_ptr<NapiAsyncTask> asyncTask)
{
    /*
     * 定义底层执行完成后的回调。
     *
     * CliToolMGRClient::ExecTool 执行结束后，会调用这个 replyCallback，
     * 并传入：
     *   resultCode：执行结果错误码；
     *   session：执行后创建/返回的 CLI 会话信息。
     *
     * [env, asyncTask] 是 lambda 捕获列表：
     *   env 用于后续创建 JS 对象；
     *   asyncTask 用于 Promise resolve/reject。
     */
    CliToolMGRClient::ExecToolReplyCallback replyCallback =
        [env, asyncTask](int32_t resultCode, const CliSessionInfo &session) {
            /*
             * 注意：底层回调不一定运行在 JS 主线程。
             * N-API 操作通常需要在正确的 JS 线程/事件队列中执行。
             * 所以这里通过 JsCliEventHandlerManager::PostTask 把任务投递回 JS 事件处理线程。
             * 底层 C++ 回调可能发生在别的线程里，但 JS 的 Promise、JS 对象只能在 JS 运行的线程里安全操作，所以要把后续任务“投递”回 JS 线程执行。
             */
            JsCliEventHandlerManager::GetInstance().PostTask(
                [env, asyncTask, resultCode, session]() {
                    // HandleScope：管理本次回调中创建的 napi_value 生命周期，防止局部 JS 引用泄露。
                    HandleScope handleScope(env);

                    // 如果底层返回错误，就把 Promise reject 掉，并转换成 JS Error。
                    if (resultCode != ERR_OK) {
                        asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, resultCode));
                        return;
                    }

                    // 把 C++ 的 CliSessionInfo 转换成 JS 对象，方便 ArkTS/JS 使用。
                    napi_value jsSession = CreateJsCliSessionInfo(env, session);
                    if (jsSession == nullptr) {
                        // 如果转换失败，这里用 undefined reject。更严格的实现可以返回具体错误。
                        asyncTask->Reject(env, CreateJsUndefined(env));
                        return;
                    }

                    // 成功：Promise resolve(jsSession)。把结果通过 Promise 的 resolve / reject 返回给 JS
                    asyncTask->ResolveWithNoError(env, jsSession);
                });
        };

    // 调用真正的客户端接口，把 param 和回调交给底层处理。
    return CliToolMGRClient::GetInstance().ExecTool(param, replyCallback);
}

/*
 * DispatchCliCmd：派发 execCmd 调用。
 *
 * 和 DispatchCliTool 很像，区别是：
 *   1. 参数类型是 ExecCmdParam；
 *   2. 调用的是 CliToolMGRClient::ExecCmd；
 *   3. 多了 callback 参数，用于接收命令执行过程中的 session 事件。
 */
int32_t DispatchCliCmd(const ExecCmdParam &param, napi_env env,
    std::shared_ptr<NapiAsyncTask> asyncTask,
    std::shared_ptr<JsCliSessionEventCallbackImpl> callback)
{
    // 定义 execCmd 执行完成后的回调。
    CliToolMGRClient::ExecToolReplyCallback replyCallback =
        [env, asyncTask](int32_t resultCode, const CliSessionInfo &session) {
            JsCliEventHandlerManager::GetInstance().PostTask(
                [env, asyncTask, resultCode, session]() {
                    HandleScope handleScope(env);

                    // 错误时 reject Promise。
                    if (resultCode != ERR_OK) {
                        asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, resultCode));
                        return;
                    }

                    // 成功时把 session 转成 JS 对象。
                    napi_value jsSession = CreateJsCliSessionInfo(env, session);
                    if (jsSession == nullptr) {
                        asyncTask->Reject(env, CreateJsUndefined(env));
                        return;
                    }

                    // Promise resolve(jsSession)。
                    asyncTask->ResolveWithNoError(env, jsSession);
                });
        };

    // 调用底层客户端执行命令。
    // callback 用于接收执行过程事件，比如输出、状态变化等，具体取决于 JsCliSessionEventCallbackImpl 的实现。
    return CliToolMGRClient::GetInstance().ExecCmd(param, replyCallback, callback);
}
} // namespace

/*
 * Finalizer：JSCliManager 对象被 JS GC 回收时调用。
 *
 * napi_wrap 会把一个 C++ 对象绑定到 JS 对象上。
 * 当 JS 对象被回收时，N-API 会调用 Finalizer，让 C++ 层释放对应对象。
 */
void JSCliManager::Finalizer(napi_env env, void *data, void *hint)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::Finalizer is called");

    // data 是之前 napi_wrap 绑定进去的 JSCliManager*。
    // 这里用 unique_ptr 接管裸指针，函数结束时自动 delete。
    std::unique_ptr<JSCliManager>(static_cast<JSCliManager*>(data));
}

/*
 * 下面这些静态函数是暴露给 JS 的 N-API 入口。
 *
 * 它们通常不直接写业务逻辑，而是通过 GET_CB_INFO_AND_CALL 宏：
 *   1. 获取 JS this 对应的 C++ JSCliManager 实例；
 *   2. 解析 argc/argv；
 *   3. 调用对应的 OnXXX 成员函数。
 *
 * 例如 JS 调用 cliManager.execTool(...)，最终会进入：
 *   JSCliManager::ExecTool -> JSCliManager::OnExecTool
 */
napi_value JSCliManager::ExecTool(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnExecTool);
}

napi_value JSCliManager::ExecCmd(napi_env env, napi_callback_info info)
{
    GET_CB_INFO_AND_CALL(env, info, JSCliManager, OnExecCmd);
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

/*
 * OnExecTool：实现 JS 侧 execTool(...) 的核心逻辑。
 *
 * 预期 JS 参数大概是：
 *   execTool(toolName, subcommand, args, challenge, options?)
 *
 * 参数含义：
 *   toolName：工具名称，必填字符串。
 *   subcommand：子命令，必填字符串。允许为空字符串与否取决于业务，这里只要求能解析出 string。
 *   args：参数对象，转换为 WantParams。
 *   challenge：挑战/校验信息，必填字符串且不能为空。
 *   options：可选执行选项。
 *
 * 返回：
 *   JS Promise。成功 resolve CliSessionInfo，失败 reject JS Error。
 */
napi_value JSCliManager::OnExecTool(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::OnExecTool called");

    // HandleEscape 用于把当前作用域里的 napi_value 安全返回到外层。
    HandleEscape handleEscape(env);

    // execTool 至少需要 4 个参数：toolName、subcommand、args、challenge。
    if (argc < INDEX_FOUR) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // ExecToolParam 是 native 层需要的参数结构体。
    ExecToolParam param;

    // 解析第 0 个 JS 参数：toolName。
    // UnwrapStringFromJS2：把 napi_value 转成 std::string。
    // 如果解析失败，或者 toolName 是空字符串，就抛参数错误。
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], param.toolName) || param.toolName.empty()) {
        ThrowInvalidParamError(env, "Tool toolName is required");
        return CreateJsUndefined(env);
    }

    // 解析第 1 个 JS 参数：subcommand。
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ONE], param.subcommand)) {
        ThrowInvalidParamError(env, "Tool subcommand is required");
        return CreateJsUndefined(env);
    }

    // 解析第 2 个 JS 参数：args。
    // 这里不是普通字符串，而是 WantParams，适合传递 key-value 参数集合。
    if (!AppExecFwk::UnwrapWantParams(env, argv[INDEX_TWO], param.args)) {
        ThrowInvalidParamError(env, "Tool args is required");
        return CreateJsUndefined(env);
    }

    // 解析第 3 个 JS 参数：challenge。
    // 这里要求 challenge 不仅能解析为 string，还必须非空。
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_THREE], param.challenge) || param.challenge.empty()) {
        ThrowInvalidParamError(env, "Tool challenge is required");
        return CreateJsUndefined(env);
    }

    // 解析第 4 个 JS 参数：options，可选。
    // 只有当调用方真的传了第 5 个参数，并且不是 nullptr，才会解析。
    if (argc > INDEX_FOUR && argv[INDEX_FOUR] != nullptr) {
        if (!UnwrapExecOptions(env, argv[INDEX_FOUR], param.options)) {
            ThrowInvalidParamError(env, "Tool options is required");
            return CreateJsUndefined(env);
        }
    }

    // result 用来保存返回给 JS 的 Promise。
    napi_value result = nullptr;

    // 创建异步任务。
    // 这里 execute/complete 都传 nullptr，因为本函数的异步结果不是通过 NapiAsyncTask::Schedule 的 execute/complete 模式完成，
    // 而是由 DispatchCliTool 里的底层 replyCallback 主动 resolve/reject。
    auto uasyncTask = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);

    // 转成 shared_ptr，是因为 asyncTask 要被 lambda 回调捕获，必须保证异步回调回来之前对象不被释放。
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);

    // 调用底层执行工具。
    int32_t errCode = DispatchCliTool(param, env, asyncTask);

    // 如果发起调用本身就失败，直接 reject Promise。
    // 注意这不是工具执行失败，而是“请求没成功发出去”。
    if (errCode != ERR_OK) {
        asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, errCode));
    }

    // 返回 Promise 给 JS。
    return handleEscape.Escape(result);
}

/*
 * OnExecCmd：实现 JS 侧 execCmd(...) 的核心逻辑。
 *
 * 预期 JS 参数大概是：
 *   execCmd(cmd, execCmdOptions?)
 *
 * 参数含义：
 *   cmd：要执行的命令字符串，必填且不能为空。
 *   execCmdOptions：可选对象，里面可能包含 cwd、env、callback 等执行选项。
 *
 * 返回：
 *   JS Promise。成功 resolve CliSessionInfo，失败 reject JS Error。
 */
napi_value JSCliManager::OnExecCmd(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::OnExecCmd called");
    HandleEscape handleEscape(env);

    // execCmd 至少需要一个参数：cmd。
    if (argc < INDEX_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // ExecCmdParam 是 native 层执行命令需要的参数结构体。
    ExecCmdParam param;

    // 解析第 0 个 JS 参数：cmd。
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], param.cmd) || param.cmd.empty()) {
        ThrowInvalidParamError(env, "cmd is required");
        return CreateJsUndefined(env);
    }

    // 解析可选参数 execCmdOptions，即 argv[1]。
    // callback 用于保存 JS 侧传入的事件回调封装对象。
    std::shared_ptr<JsCliSessionEventCallbackImpl> callback = nullptr;
    if (argc > INDEX_ONE && argv[INDEX_ONE] != nullptr) {
        napi_valuetype valueType = napi_undefined;

        // napi_typeof：获取 JS 值的类型，比如 object、function、string、undefined 等。
        napi_status status = napi_typeof(env, argv[INDEX_ONE], &valueType);

        // 只有第二个参数是对象时，才按照 options 解析。
        if (status == napi_ok && valueType == napi_object) {
            std::string cmdOptionsMsg;

            // 把 JS options 对象解析进 param，并可能创建 callback。
            if (!UnwrapExecCmdOptions(env, argv[INDEX_ONE], param, callback, cmdOptionsMsg)) {
                // 如果工具函数给出了具体错误消息，就用具体错误；否则用默认错误。
                ThrowInvalidParamError(env, cmdOptionsMsg.empty() ? "Invalid ExecCmdOptions" : cmdOptionsMsg.c_str());
                return CreateJsUndefined(env);
            }
        }
    }

    // 创建返回给 JS 的 Promise。
    napi_value result = nullptr;
    auto uasyncTask = CreateAsyncTaskWithLastParam(env, nullptr, nullptr, nullptr, &result);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);

    // 调用底层执行命令。
    int32_t errCode = DispatchCliCmd(param, env, asyncTask, callback);
    if (errCode != ERR_OK) {
        asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, errCode));
    }

    return handleEscape.Escape(result);
}

/*
 * OnSubscribeSession：订阅某个 session 的事件。
 *
 * 预期 JS 参数大概是：
 *   subscribeSession(sessionId, callback)
 *
 * 参数含义：
 *   sessionId：要订阅的 CLI 会话 ID。
 *   callback：JS 回调函数，用于接收 session 事件。
 *
 * 返回：
 *   JS Promise。成功 resolve undefined，失败 reject JS Error。
 */
napi_value JSCliManager::OnSubscribeSession(napi_env env, size_t argc, napi_value *argv)
{
    HandleEscape handleEscape(env);

    // 至少需要两个参数：sessionId 和 callback。
    if (argc < INDEX_TWO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // 解析 sessionId。
    std::string sessionId;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
        ThrowInvalidParamError(env, "sessionId is required");
        return CreateJsUndefined(env);
    }

    // 校验第二个参数是不是合法的工具事件回调。
    if (!IsValidToolEventCallback(env, argv[INDEX_ONE])) {
        ThrowInvalidParamError(env, "tool event callback is required");
        return CreateJsUndefined(env);
    }

    // innerErrCode 用 shared_ptr，是因为 execute 和 complete 两个 lambda 都要访问它。
    // execute 在线程池/异步阶段写入错误码，complete 在 JS 线程读取错误码并 resolve/reject。
    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);

    // 把 JS callback 包装成 C++ 对象。
    auto callback = std::make_shared<JsCliSessionEventCallbackImpl>(env, argv[INDEX_ONE]);
    if (callback == nullptr || !callback->IsValid()) {
        ThrowInvalidParamError(env, "callback must be a function");
        return CreateJsUndefined(env);
    }

    // execute：真正执行订阅逻辑的异步函数。
    // 注意这里不直接操作 JS 对象，只调用 native client。
    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, callback, sessionId]() {
        std::string subscriptionId;
        *innerErrCode = CliToolMGRClient::GetInstance().SubscribeSession(sessionId, callback, subscriptionId);
    };

    // complete：execute 完成后回到 JS 线程执行，负责处理 Promise 结果。
    NapiAsyncTask::CompleteCallback complete = [innerErrCode](napi_env env,
        NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        // 订阅成功没有额外返回值，所以 resolve undefined。
        task.ResolveWithNoError(env, CreateJsUndefined(env));
    };

    // 创建并调度异步任务，返回 Promise。
    napi_value result = nullptr;
    NapiAsyncTask::Schedule("JSCliManager::OnSubscribeSession", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &result));
    return handleEscape.Escape(result);
}

/*
 * OnClearSession：清理一个 CLI session。
 *
 * 预期 JS 参数大概是：
 *   clearSession(sessionId)
 *
 * 返回：
 *   JS Promise。成功 resolve undefined，失败 reject JS Error。
 */
napi_value JSCliManager::OnClearSession(napi_env env, size_t argc, napi_value *argv)
{
    HandleEscape handleEscape(env);

    // 至少需要一个参数：sessionId。
    if (argc < INDEX_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // 解析 sessionId。
    std::string sessionId;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
        ThrowInvalidParamError(env, "sessionId is required");
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);

    // 异步执行清理 session。
    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, sessionId]() {
        *innerErrCode = CliToolMGRClient::GetInstance().ClearSession(sessionId);
    };

    // 根据清理结果 resolve/reject Promise。
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

/*
 * OnSendMessage：向指定 session 发送一条输入文本。
 *
 * 预期 JS 参数大概是：
 *   sendMessage(sessionId, inputText)
 *
 * 参数含义：
 *   sessionId：目标 CLI 会话 ID。
 *   inputText：发送给该 session 的文本，例如交互式命令的输入。
 *
 * 返回：
 *   JS Promise。成功 resolve undefined，失败 reject JS Error。
 */
napi_value JSCliManager::OnSendMessage(napi_env env, size_t argc, napi_value *argv)
{
    HandleEscape handleEscape(env);

    // 至少需要两个参数：sessionId 和 inputText。
    if (argc < INDEX_TWO) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // 解析 sessionId。
    std::string sessionId;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
        ThrowInvalidParamError(env, "sessionId is required");
        return CreateJsUndefined(env);
    }

    // 解析 inputText。
    // 注意这里没有要求 inputText 非空，所以空字符串也可能是合法输入，取决于底层业务。
    std::string inputText;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ONE], inputText)) {
        ThrowInvalidParamError(env, "inputText is required");
        return CreateJsUndefined(env);
    }

    // 创建 Promise 对应的异步任务。
    napi_value asyncResult = nullptr;
    napi_value lastParam = nullptr;
    std::unique_ptr<NapiAsyncTask> uasyncTask =
        CreateAsyncTaskWithLastParam(env, lastParam, nullptr, nullptr, &asyncResult);
    std::shared_ptr<NapiAsyncTask> asyncTask = std::move(uasyncTask);

    // SendMessage 的底层结果通过 EventReplyCallback 返回。
    CliToolMGRClient::EventReplyCallback replyCallback = [env, asyncTask](int32_t resultCode) {
        // 同样要投递回 JS 线程再操作 Promise。
        JsCliEventHandlerManager::GetInstance().PostTask([env, asyncTask, resultCode]() {
            HandleScope handleScope(env);
            if (resultCode != ERR_OK) {
                asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, resultCode));
                return;
            }
            asyncTask->ResolveWithNoError(env, CreateJsUndefined(env));
        });
    };

    // 发起发送消息请求。
    int32_t errCode = CliToolMGRClient::GetInstance().SendMessage(sessionId, inputText, replyCallback);
    if (errCode != ERR_OK) {
        asyncTask->Reject(env, CreateCliJsErrorByNativeErr(env, errCode));
    }

    return handleEscape.Escape(asyncResult);
}

/*
 * OnQuerySession：查询一个 session 的当前信息。
 *
 * 预期 JS 参数大概是：
 *   querySession(sessionId)
 *
 * 返回：
 *   JS Promise。成功 resolve CliSessionInfo，失败 reject JS Error。
 */
napi_value JSCliManager::OnQuerySession(napi_env env, size_t argc, napi_value *argv)
{
    HandleEscape handleEscape(env);

    // 至少需要一个参数：sessionId。
    if (argc < INDEX_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // 解析 sessionId。
    std::string sessionId;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], sessionId) || sessionId.empty()) {
        ThrowInvalidParamError(env, "sessionId is required");
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);

    // session 用 shared_ptr，因为 execute 写入查询结果，complete 读取并转成 JS 对象。
    auto session = std::make_shared<CliSessionInfo>();

    // 异步查询 session 信息。
    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, sessionId, session]() {
        *innerErrCode = CliToolMGRClient::GetInstance().QuerySession(sessionId, *session);
    };

    // 查询完成后，回到 JS 线程处理结果。
    NapiAsyncTask::CompleteCallback complete = [innerErrCode, session](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        // C++ session 结构转 JS 对象。
        napi_value jsSession = CreateJsCliSessionInfo(env, *session);
        if (jsSession == nullptr) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Fail to create js CliSessionInfo");
            task.Reject(env, CreateJsUndefined(env));
            return;
        }

        // Promise resolve(jsSession)。
        task.ResolveWithNoError(env, jsSession);
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JSCliManager::OnQuerySession", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

/*
 * OnGetToolInfoByName：根据工具名称查询工具完整信息。
 *
 * 预期 JS 参数大概是：
 *   getToolInfoByName(toolName)
 *
 * 返回：
 *   JS Promise。成功 resolve ToolInfo，失败 reject JS Error。
 */
napi_value JSCliManager::OnGetToolInfoByName(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::OnGetToolInfoByName called");
    HandleEscape handleEscape(env);

    // 至少需要一个参数：toolName。
    if (argc < INDEX_ONE) {
        ThrowTooFewParametersError(env);
        return CreateJsUndefined(env);
    }

    // 解析 toolName。
    std::string toolName;
    if (!AppExecFwk::UnwrapStringFromJS2(env, argv[INDEX_ZERO], toolName) || toolName.empty()) {
        ThrowInvalidParamError(env, "Tool name is required");
        return CreateJsUndefined(env);
    }

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);

    // ToolInfo 保存底层查询出来的工具详细信息。
    auto tool = std::make_shared<ToolInfo>();

    // 异步查询工具信息。
    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, toolName, tool]() {
        *innerErrCode = CliToolMGRClient::GetInstance().GetToolInfoByName(toolName, *tool);
    };

    // 查询完成后转成 JS 对象返回。
    NapiAsyncTask::CompleteCallback complete = [innerErrCode, tool](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "GetToolInfoByName error: %{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        // ToolInfo -> JS object。
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

/*
 * OnQueryToolSummaries：查询所有工具的摘要信息。
 *
 * 预期 JS 参数：
 *   queryToolSummaries()
 *
 * 返回：
 *   JS Promise。成功 resolve ToolSummary[]，失败 reject JS Error。
 *
 * ToolSummary 一般比 ToolInfo 更轻量，适合列表展示。
 */
napi_value JSCliManager::OnQueryToolSummaries(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::OnQueryToolSummaries called");
    HandleEscape handleEscape(env);

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);

    // 保存底层返回的工具摘要列表。
    auto summaries = std::make_shared<std::vector<ToolSummary>>();

    // 异步查询所有工具摘要。
    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, summaries]() {
        *innerErrCode = CliToolMGRClient::GetInstance().GetAllToolSummaries(*summaries);
    };

    // 查询完成后，将 vector<ToolSummary> 转成 JS array。
    NapiAsyncTask::CompleteCallback complete = [innerErrCode, summaries](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "QueryToolSummaries error: %{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        // 创建 JS 数组。
        napi_value jsArray = nullptr;
        napi_create_array(env, &jsArray);

        // 遍历 C++ vector，把每个 ToolSummary 转成 JS 对象，然后放进 JS 数组。
        for (size_t i = 0; i < summaries->size(); i++) {
            napi_value jsSummary = CreateJsToolSummary(env, (*summaries)[i]);
            if (jsSummary != nullptr) {
                napi_set_element(env, jsArray, i, jsSummary);
            }
        }

        // Promise resolve(jsArray)。
        task.ResolveWithNoError(env, jsArray);
    };

    napi_value asyncResult = nullptr;
    NapiAsyncTask::Schedule("JsCliManager::OnQueryToolSummaries", env,
        CreateAsyncTaskWithLastParam(env, nullptr, std::move(execute), std::move(complete), &asyncResult));
    return handleEscape.Escape(asyncResult);
}

/*
 * OnQueryTools：查询所有工具的完整信息。
 *
 * 预期 JS 参数：
 *   queryTools()
 *
 * 返回：
 *   JS Promise。成功 resolve ToolInfo[]，失败 reject JS Error。
 *
 * 与 queryToolSummaries 的区别：
 *   queryToolSummaries 返回摘要，通常字段更少；
 *   queryTools 返回完整 ToolInfo，字段更多。
 */
napi_value JSCliManager::OnQueryTools(napi_env env, size_t argc, napi_value *argv)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManager::OnQueryTools called");
    HandleEscape handleEscape(env);

    auto innerErrCode = std::make_shared<int32_t>(ERR_OK);

    // 保存底层返回的完整工具信息列表。
    auto tools = std::make_shared<std::vector<ToolInfo>>();

    // 异步查询所有工具完整信息。
    NapiAsyncTask::ExecuteCallback execute = [innerErrCode, tools]() {
        *innerErrCode = CliToolMGRClient::GetInstance().GetAllToolInfos(*tools);
    };

    // 查询完成后，将 vector<ToolInfo> 转成 JS array。
    NapiAsyncTask::CompleteCallback complete = [innerErrCode, tools](
        napi_env env, NapiAsyncTask &task, int32_t status) {
        HandleScope handleScope(env);
        if (*innerErrCode != ERR_OK) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "QueryTools error: %{public}d", *innerErrCode);
            task.Reject(env, CreateCliJsErrorByNativeErr(env, *innerErrCode));
            return;
        }

        // 创建 JS 数组。
        napi_value jsArray = nullptr;
        napi_create_array(env, &jsArray);

        // 每个 ToolInfo 转成 JS 对象后，设置到数组对应位置。
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

/*
 * JSCliManagerInit：模块初始化函数。
 *
 * 这个函数通常会在 native 模块加载时被调用。
 * 它的职责是：
 *   1. 创建 JSCliManager C++ 对象；
 *   2. 将 C++ 对象绑定到 JS exportObj 上；
 *   3. 把 C++ 函数绑定成 JS 可调用的方法。
 *
 * 简单理解：
 *   没有这个函数，JS 侧就看不到 execTool、execCmd 等方法。
 */
napi_value JSCliManagerInit(napi_env env, napi_value exportObj)
{
    TAG_LOGD(AAFwkTag::CLI_TOOL, "Init JSCliManager");

    // env 是 N-API 环境，exportObj 是要导出的 JS 对象。
    // 二者任何一个为空，都无法继续初始化。
    if (env == nullptr || exportObj == nullptr) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Null env or exportObj");
        return nullptr;
    }

    // 创建 C++ JSCliManager 实例。
    std::unique_ptr<JSCliManager> jsCliManager = std::make_unique<JSCliManager>();

    /*
     * napi_wrap：把 C++ 对象挂到 JS 对象上。
     *
     * 参数解释：
     *   env：N-API 环境；
     *   exportObj：JS 导出对象；
     *   jsCliManager.release()：释放 unique_ptr 所有权，把裸指针交给 N-API 管理；
     *   JSCliManager::Finalizer：JS 对象销毁时调用，用于 delete C++ 对象；
     *   nullptr：finalizer 的 hint 参数，这里没用；
     *   nullptr：如果需要保存 wrapper 引用可以传地址，这里不保存。
     */
    napi_wrap(env, exportObj, jsCliManager.release(), JSCliManager::Finalizer, nullptr, nullptr);

    // moduleName 用于日志/错误信息/函数绑定标识。
    const char *moduleName = "CliManager";

    /*
     * BindNativeFunction：把 C++ 静态函数绑定为 JS 方法。
     *
     * 绑定后 JS 侧大概可以这样调用：
     *   CliManager.execTool(...)
     *   CliManager.execCmd(...)
     *   CliManager.subscribeSession(...)
     */
    BindNativeFunction(env, exportObj, "execTool", moduleName, JSCliManager::ExecTool);
    BindNativeFunction(env, exportObj, "execCmd", moduleName, JSCliManager::ExecCmd);
    BindNativeFunction(env, exportObj, "subscribeSession", moduleName, JSCliManager::SubscribeSession);
    BindNativeFunction(env, exportObj, "clearSession", moduleName, JSCliManager::ClearSession);
    BindNativeFunction(env, exportObj, "querySession", moduleName, JSCliManager::QuerySession);
    BindNativeFunction(env, exportObj, "sendMessage", moduleName, JSCliManager::SendMessage);
    BindNativeFunction(env, exportObj, "getToolInfoByName", moduleName, JSCliManager::GetToolInfoByName);
    BindNativeFunction(env, exportObj, "queryToolSummaries", moduleName, JSCliManager::QueryToolSummaries);
    BindNativeFunction(env, exportObj, "queryTools", moduleName, JSCliManager::QueryTools);

    TAG_LOGD(AAFwkTag::CLI_TOOL, "JSCliManagerInit end");

    // 初始化函数成功结束，这里返回 JS undefined。
    return CreateJsUndefined(env);
}

} // namespace CliTool
} // namespace OHOS
