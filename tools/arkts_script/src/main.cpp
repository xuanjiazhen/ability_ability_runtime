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

#include "arkts_script.h"

#include <atomic>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <thread>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace ArktsScript {
namespace {

struct CompletionChannel {
    int eventFd = -1;
    int epollFd = -1;
};

struct ExitState {
    std::shared_ptr<ExecutionContext> execContext;
};

void CloseCompletionChannel(CompletionChannel& channel);

[[noreturn]] void ExitFromMonitor(const CompletionChannel& channel, int exitCode)
{
    CompletionChannel localChannel = channel;
    CloseCompletionChannel(localChannel);
    _exit(exitCode);
}

bool CreateCompletionChannel(CompletionChannel& channel)
{
    channel.eventFd = eventfd(0, EFD_CLOEXEC);
    if (channel.eventFd < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create eventfd, errno=%{public}d", errno);
        return false;
    }

    channel.epollFd = epoll_create1(EPOLL_CLOEXEC);
    if (channel.epollFd < 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to create epoll fd, errno=%{public}d", errno);
        close(channel.eventFd);
        channel.eventFd = -1;
        return false;
    }

    epoll_event event {};
    event.events = EPOLLIN;
    event.data.fd = channel.eventFd;
    if (epoll_ctl(channel.epollFd, EPOLL_CTL_ADD, channel.eventFd, &event) != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to register eventfd into epoll, errno=%{public}d", errno);
        close(channel.epollFd);
        close(channel.eventFd);
        channel.epollFd = -1;
        channel.eventFd = -1;
        return false;
    }

    return true;
}

void CloseCompletionChannel(CompletionChannel& channel)
{
    if (channel.epollFd >= 0) {
        close(channel.epollFd);
        channel.epollFd = -1;
    }
    if (channel.eventFd >= 0) {
        close(channel.eventFd);
        channel.eventFd = -1;
    }
}

bool SignalCompletion(int eventFd)
{
    if (eventFd < 0) {
        return false;
    }

    constexpr uint64_t VALUE = 1;
    ssize_t ret = -1;
    do {
        ret = write(eventFd, &VALUE, sizeof(VALUE));
    } while (ret < 0 && errno == EINTR);
    if (ret != static_cast<ssize_t>(sizeof(VALUE))) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to signal eventfd, errno=%{public}d", errno);
        return false;
    }
    return true;
}

bool PublishCompletion(const std::shared_ptr<ExecutionContext>& execContext, int eventFd,
    bool success, const std::string& result, const std::string& error, const std::string& errorType)
{
    if (execContext == nullptr) {
        return false;
    }

    std::lock_guard<std::mutex> lock(execContext->mutex);
    if (execContext->completed.load()) {
        TAG_LOGW(AAFwkTag::APPKIT, "Duplicate completion ignored");
        return false;
    }

    if (success) {
        execContext->result = result;
        execContext->errorType.clear();
    } else {
        execContext->error = error;
        execContext->errorType = errorType;
    }
    execContext->completed.store(true, std::memory_order_release);

    if (!SignalCompletion(eventFd)) {
        TAG_LOGE(AAFwkTag::APPKIT, "completion recorded but signal failed");
    }
    return true;
}

void MonitorCompletion(ExitState state, CompletionChannel channel)
{
    constexpr int MAX_EVENTS = 1;
    epoll_event events[MAX_EVENTS] {};

    while (true) {
        int readyCount = epoll_wait(channel.epollFd, events, MAX_EVENTS, -1);
        if (readyCount < 0) {
            if (errno == EINTR) {
                continue;
            }
            TAG_LOGE(AAFwkTag::APPKIT, "epoll_wait failed, errno=%{public}d", errno);
            OutputError("failed to wait completion signal", "SYSTEM_ERROR");
            ExitFromMonitor(channel, EXIT_FAILURE);
        }

        if (readyCount == 0) {
            continue;
        }

        if (events[0].data.fd != channel.eventFd) {
            continue;
        }

        uint64_t count = 0;
        ssize_t readSize = -1;
        do {
            readSize = read(channel.eventFd, &count, sizeof(count));
        } while (readSize < 0 && errno == EINTR);
        if (readSize < 0 && errno != EAGAIN) {
            TAG_LOGE(AAFwkTag::APPKIT, "failed to read eventfd, errno=%{public}d", errno);
            OutputError("failed to read completion signal", "SYSTEM_ERROR");
            ExitFromMonitor(channel, EXIT_FAILURE);
        }

        if (state.execContext == nullptr || !state.execContext->completed.load(std::memory_order_acquire)) {
            continue;
        }

        std::string result;
        std::string error;
        std::string errorType;
        {
            std::lock_guard<std::mutex> lock(state.execContext->mutex);
            result = state.execContext->result;
            error = state.execContext->error;
            errorType = state.execContext->errorType;
        }

        if (!error.empty()) {
            OutputError(error, errorType.empty() ? "EXECUTION_ERROR" : errorType);
            ExitFromMonitor(channel, EXIT_FAILURE);
        } else {
            OutputResult(result);
            ExitFromMonitor(channel, EXIT_SUCCESS);
        }
    }
}

} // namespace

int main(int argc, char* argv[])
{
    TAG_LOGI(AAFwkTag::APPKIT, "ohos-arkts runner start");

    // ========== 阶段1: 命令行解析 ==========
    ScriptArgs args;
    if (!ParseArguments(argc, argv, args)) {
        OutputError("failed to parse arguments", "ARGUMENT_ERROR");
        return EXIT_FAILURE;
    }
    if (args.showHelp) {
        return EXIT_SUCCESS;
    }

    // ========== 阶段2: 创建CompletionChannel + Monitor子线程 ==========
    CompletionChannel channel;
    if (!CreateCompletionChannel(channel)) {
        OutputError("failed to create completion channel", "SYSTEM_ERROR");
        return EXIT_FAILURE;
    }

    auto execContext = std::make_shared<ExecutionContext>();
    ExitState exitState { execContext };
    std::thread monitorThread(MonitorCompletion, exitState, channel);

    ResultCallback completeAndSignal = [execContext, channel](bool success, const std::string& result,
                                         const std::string& error, const std::string& errorType) {
        PublishCompletion(execContext, channel.eventFd, success, result, error, errorType);
    };

    auto finalizeAndJoin = [&](const std::string& error, const std::string& errorType) -> int {
        if (execContext != nullptr && !execContext->completed.load()) {
            completeAndSignal(false, "", error, errorType);
        }
        if (monitorThread.joinable()) {
            monitorThread.join();
        }
        return EXIT_FAILURE;
    };

    // ========== 阶段3: 创建ScriptContext → 创建JsRuntime → 绑定 → 加载ABC ==========
    // 需要其他文件提供: std::shared_ptr<ContextImpl> CreateScriptContext()
    std::shared_ptr<ContextImpl> context = CreateScriptContext();
    if (context == nullptr) {
        return finalizeAndJoin("failed to create script context", "CONTEXT_ERROR");
    }

    // 需要其他文件提供: JsRuntime* CreateJsRuntime(std::shared_ptr<ContextImpl>)
    // 或: std::unique_ptr<JsRuntime> CreateJsRuntime(std::shared_ptr<ContextImpl>)
    JsRuntime* runtime = CreateJsRuntime(context);
    if (runtime == nullptr) {
        return finalizeAndJoin("failed to create js runtime", "RUNTIME_ERROR");
    }

    std::unique_ptr<JsRuntime> runtimeOwner;
    runtimeOwner.reset(runtime);

    napi_env env = runtime->GetNapiEnv();
    if (env == nullptr) {
        return finalizeAndJoin("failed to get napi_env", "ENV_ERROR");
    }

    // 需要其他文件提供: bool BindContextToGlobal(napi_env, std::shared_ptr<ContextImpl>)
    if (!BindContextToGlobal(env, context)) {
        return finalizeAndJoin("failed to bind context to globalThis", "BIND_ERROR");
    }

    // 绑定 completeArkTsTool 到 JS 全局
    // 需要其他文件提供: bool BindCompleteArkTsTool(napi_env, std::function<...>)
    if (!BindCompleteArkTsTool(env, completeAndSignal)) {
        return finalizeAndJoin("failed to bind completeArkTsTool", "BIND_ERROR");
    }

    // 需要其他文件提供: bool LoadAbcFile(JsRuntime*, const std::string&)
    TAG_LOGI(AAFwkTag::APPKIT, "loading ABC file: %{private}s", args.abcPath.c_str());
    if (!LoadAbcFile(runtime, args.abcPath)) {
        return finalizeAndJoin("failed to load ABC file: " + args.abcPath, "LOAD_ERROR");
    }

    // 需要其他文件提供: bool ResolveFunction(napi_env, const std::string&, const std::string&, ...)
    TAG_LOGI(AAFwkTag::APPKIT, "resolving function: %{public}s", args.funName.c_str());
    napi_value receiver = nullptr;
    napi_value func = nullptr;
    if (!ResolveFunction(env, args.scriptName, args.funName, receiver, func)) {
        return finalizeAndJoin("failed to resolve function: " + args.funName, "FUNCTION_ERROR");
    }

    // 需要其他文件提供: napi_value ConvertArgumentsToNapi(napi_env, const std::vector<std::string>&)
    napi_value argsArray = ConvertArgumentsToNapi(env, args.arguments);
    if (argsArray == nullptr) {
        return finalizeAndJoin("failed to convert arguments", "ARGUMENT_ERROR");
    }


    // ========== 阶段4: 调用目标函数 ==========
    TAG_LOGI(AAFwkTag::APPKIT, "calling function: %{public}s", args.funName.c_str());
    napi_value callResult = nullptr;
    napi_status status = napi_call_function(env, receiver, func, 1, &argsArray, &callResult);
    if (status != napi_ok) {
        return finalizeAndJoin("failed to call function: " + args.funName +
            ", napi_status: " + std::to_string(status), "CALL_ERROR");
    }

    // ========== 阶段5: 统一等待 complete 回调 ==========
    // 方案C: 所有结果必须由 JS 函数内部调用 completeArkTsTool 返回
    // 同步函数: JS 代码执行到 completeArkTsTool(result) 时触发 NAPI 回调
    // 异步函数: Promise resolve 后 JS 调用 completeArkTsTool，或 HandlePromiseResult 的 then 回调触发
    // 主线程不再区分 sync/promise，不再兜底调用 completeAndSignal
    // 如果 JS 函数未调用 completeArkTsTool，Monitor 永远 epoll_wait 阻塞

    bool isPromise = false;
    napi_is_promise(env, callResult, &isPromise);
    TAG_LOGI(AAFwkTag::APPKIT, "function return type: %{public}s", isPromise ? "Promise" : "sync");

    if (isPromise) {
        // 需要其他文件提供: bool HandlePromiseResult(napi_env, napi_value, std::function<...>)
        if (!HandlePromiseResult(env, callResult, completeAndSignal)) {
            return finalizeAndJoin("failed to handle Promise result", "PROMISE_ERROR");
        }
    }

    // ========== 阶段6: 主线程阻塞等待 Monitor 输出并退出 ==========
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
    // Monitor 线程已 _exit 结束进程，以下代码不会执行
    return EXIT_FAILURE;
    return EXIT_FAILURE;
}

} // namespace ArktsScript
} // namespace OHOS
