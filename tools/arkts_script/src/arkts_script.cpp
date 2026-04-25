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

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <thread>

#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "js_arkts_script.h"
#include "nlohmann/json.hpp"

namespace OHOS {
namespace ArktsScript {
using OHOS::AbilityRuntime::JsRuntime;
namespace {

constexpr char SCRIPT_EXTENSION[] = ".ets";
constexpr size_t SCRIPT_EXTENSION_LENGTH = sizeof(SCRIPT_EXTENSION) - 1;
constexpr int MIN_REQUIRED_ARGC = 4;
constexpr int MIN_SCRIPT_AND_FUNCTION_ARGS = 2;
constexpr uint64_t SIGNAL_VALUE = 1;
constexpr int MAX_EVENTS = 1;
constexpr size_t CALL_ARGC = 1;

constexpr int32_t SCRIPT_CONTEXT_BUNDLE_INFO_FLAGS =
    static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) +
    static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION);

bool InitScriptContextFromBundleInfo(const std::shared_ptr<ContextImpl>& scriptContext,
    const AppExecFwk::BundleInfo& bundleInfo)
{
    if (scriptContext == nullptr) {
        return false;
    }
    if (bundleInfo.applicationInfo.name.empty() && bundleInfo.applicationInfo.bundleName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "caller applicationInfo empty");
        return false;
    }

    scriptContext->SetApplicationInfo(std::make_shared<AppExecFwk::ApplicationInfo>(bundleInfo.applicationInfo));
    scriptContext->SetProcessName(bundleInfo.applicationInfo.process);

    if (bundleInfo.hapModuleInfos.empty()) {
        TAG_LOGW(AAFwkTag::APPKIT, "caller has no hapModuleInfos");
        return true;
    }

    auto hapModuleInfo = std::find_if(bundleInfo.hapModuleInfos.begin(), bundleInfo.hapModuleInfos.end(),
        [](const AppExecFwk::HapModuleInfo& info) {
            return !info.moduleName.empty();
        });
    if (hapModuleInfo == bundleInfo.hapModuleInfos.end()) {
        hapModuleInfo = bundleInfo.hapModuleInfos.begin();
    }
    scriptContext->InitHapModuleInfo(*hapModuleInfo);
    TAG_LOGI(AAFwkTag::APPKIT,
        "script context initialized from bundleName=%{public}s, moduleName=%{public}s",
        bundleInfo.applicationInfo.bundleName.c_str(), hapModuleInfo->moduleName.c_str());
    return true;
}

bool InitScriptContextFromCaller(const std::shared_ptr<ContextImpl>& scriptContext)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "bundleMgrHelper nullptr");
        return false;
    }

    AppExecFwk::BundleInfo bundleInfo;
    ErrCode ret = bundleMgrHelper->GetBundleInfoForSelf(SCRIPT_CONTEXT_BUNDLE_INFO_FLAGS, bundleInfo);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::APPKIT, "getBundleInfo failed ret=%{public}d", ret);
        return false;
    }
    return InitScriptContextFromBundleInfo(scriptContext, bundleInfo);
}

} // namespace

bool ArktsScript::IsScriptFile(const std::string& value)
{
    if (value.empty()) {
        return false;
    }
    return value.size() > SCRIPT_EXTENSION_LENGTH &&
        value.substr(value.size() - SCRIPT_EXTENSION_LENGTH) == SCRIPT_EXTENSION;
}

bool ArktsScript::ParseHelpOption(int argc, char* argv[], ScriptArgs& args)
{
    for (int i = 1; i < argc; i++) {
        if (argv[i] == nullptr) {
            continue;
        }
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            PrintUsage(argv[0]);
            args.showHelp = true;
            return true;
        }
    }
    return false;
}

bool ArktsScript::MoveToAbcOption(int argc, char* argv[], int& index)
{
    while (index < argc && argv[index][0] == '-' && strcmp(argv[index], "-abc") != 0) {
        std::string option = argv[index];
        TAG_LOGE(AAFwkTag::APPKIT, "Unknown option: %{public}s", option.c_str());
        PrintUsage(argv[0]);
        return false;
    }
    return true;
}

bool ArktsScript::ParseAbcPath(int argc, char* argv[], int& index, ScriptArgs& args)
{
    if (index >= argc || strcmp(argv[index], "-abc") != 0) {
        TAG_LOGE(AAFwkTag::APPKIT, "Missing -abc option");
        PrintUsage(argv[0]);
        return false;
    }
    index++;
    if (index >= argc) {
        TAG_LOGE(AAFwkTag::APPKIT, "Missing value for -abc");
        return false;
    }
    args.abcPath = argv[index++];
    return true;
}

bool ArktsScript::ParseFunctionAndArguments(int argc, char* argv[], int& index, ScriptArgs& args)
{
    if (index >= argc) {
        TAG_LOGE(AAFwkTag::APPKIT, "Missing functionName");
        PrintUsage(argv[0]);
        return false;
    }
    if ((argc - index) >= MIN_SCRIPT_AND_FUNCTION_ARGS && IsScriptFile(argv[index])) {
        args.scriptName = argv[index++];
    }
    if (index >= argc) {
        TAG_LOGE(AAFwkTag::APPKIT, "Missing functionName");
        PrintUsage(argv[0]);
        return false;
    }
    args.funName = argv[index++];
    while (index < argc) {
        args.arguments.push_back(argv[index++]);
    }
    return true;
}

void ArktsScript::PrintUsage(const char* execName)
{
    const std::string usage = std::string("Usage: ") + execName +
        " [options] -abc module.abc [script.ets] functionName [arguments]\n\n"
        "Options:\n"
        "  -h, --help    Show this help message\n\n"
        "Arguments:\n"
        "  -abc module.abc    ABC file path (required)\n"
        "  [script.ets]      Script class name for standalone skill (optional)\n"
        "  functionName       Function name to execute (required)\n"
        "  [arguments]        Function arguments as string array (optional)\n\n";

    TAG_LOGI(AAFwkTag::APPKIT, "%{public}s", usage.c_str());
}

bool ArktsScript::ParseArguments(int argc, char* argv[], ScriptArgs& args)
{
    if (ParseHelpOption(argc, argv, args)) {
        return true;
    }

    int i = 1;
    if (!MoveToAbcOption(argc, argv, i) || argc < MIN_REQUIRED_ARGC) {
        PrintUsage(argv[0]);
        return false;
    }

    return ParseAbcPath(argc, argv, i, args) && ParseFunctionAndArguments(argc, argv, i, args);
}

std::shared_ptr<ContextImpl> ArktsScript::CreateScriptContext()
{
    auto scriptContext = std::make_shared<ContextImpl>();
    if (scriptContext == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to allocate ContextImpl");
        return nullptr;
    }

    if (!InitScriptContextFromCaller(scriptContext)) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to initialize script context from caller");
        return nullptr;
    }

    return scriptContext;
}

std::unique_ptr<JsRuntime> ArktsScript::CreateJsRuntime(const std::shared_ptr<ContextImpl>& context)
{
    TAG_LOGI(AAFwkTag::APPKIT, "Creating new Runtime");

    AbilityRuntime::Runtime::Options options;
    options.lang = AbilityRuntime::Runtime::Language::JS;
    options.isBundle = false;
    options.isStageModel = true;
    if (context != nullptr) {
        options.bundleName = context->GetBundleName();
        options.codePath = context->GetBundleCodePath();
        options.bundleCodeDir = context->GetBundleCodeDir();
    }

    auto runtime = JsRuntime::Create(options);
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to create JsRuntime");
        return nullptr;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "JsRuntime created successfully");
    return runtime;
}

bool ArktsScript::LoadAbcFile(JsRuntime* runtime, const std::string& path)
{
    if (runtime == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "runtime is null when loading abc");
        return false;
    }

    bool loaded = runtime->RunScript(path, "", false);
    if (!loaded) {
        TAG_LOGE(AAFwkTag::APPKIT, "RunScript failed for %{private}s", path.c_str());
    }
    return loaded;
}

void ArktsScript::OutputResult(const std::string& result)
{
    std::cout << result << std::endl;
}

void ArktsScript::OutputError(const std::string& error, const std::string& errorType)
{
    nlohmann::json errorOutput;
    errorOutput["success"] = false;
    errorOutput["errorType"] = errorType;
    errorOutput["error"] = error;
    std::cerr << errorOutput.dump() << std::endl;
}

void ArktsScript::ExitFromMonitor(const CompletionChannel& channel, int exitCode)
{
    CompletionChannel localChannel = channel;
    CloseCompletionChannel(localChannel);
    _exit(exitCode);
}

bool ArktsScript::CreateCompletionChannel(CompletionChannel& channel)
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

void ArktsScript::CloseCompletionChannel(CompletionChannel& channel)
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

bool ArktsScript::SignalCompletion(int eventFd)
{
    if (eventFd < 0) {
        return false;
    }

    ssize_t ret = -1;
    do {
        ret = write(eventFd, &SIGNAL_VALUE, sizeof(SIGNAL_VALUE));
    } while (ret < 0 && errno == EINTR);
    if (ret != static_cast<ssize_t>(sizeof(SIGNAL_VALUE))) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to signal eventfd, errno=%{public}d", errno);
        return false;
    }
    return true;
}

bool ArktsScript::PublishResult(const std::shared_ptr<ExecutionContext>& execContext, int eventFd,
    const std::string& result)
{
    if (execContext == nullptr) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(execContext->mutex);
        if (execContext->resultReady.load(std::memory_order_acquire)) {
            TAG_LOGW(AAFwkTag::APPKIT, "duplicate result ignored");
            return false;
        }
        execContext->result = result;
        execContext->error.clear();
        execContext->errorType.clear();
        execContext->resultReady.store(true, std::memory_order_release);
    }

    if (!SignalCompletion(eventFd)) {
        TAG_LOGE(AAFwkTag::APPKIT, "result recorded but signal failed");
    }
    return true;
}

bool ArktsScript::PublishFailure(const std::shared_ptr<ExecutionContext>& execContext, int eventFd,
    const std::string& error, const std::string& errorType)
{
    if (execContext == nullptr) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(execContext->mutex);
        if (!execContext->error.empty()) {
            TAG_LOGW(AAFwkTag::APPKIT, "duplicate failure ignored");
            return false;
        }
        execContext->result.clear();
        execContext->error = error;
        execContext->errorType = errorType;
        execContext->scriptDone.store(true, std::memory_order_release);
    }

    if (!SignalCompletion(eventFd)) {
        TAG_LOGE(AAFwkTag::APPKIT, "failure recorded but signal failed");
    }
    return true;
}

bool ArktsScript::MarkScriptDone(const std::shared_ptr<ExecutionContext>& execContext, int eventFd)
{
    if (execContext == nullptr) {
        return false;
    }

    execContext->scriptDone.store(true, std::memory_order_release);
    if (!SignalCompletion(eventFd)) {
        TAG_LOGE(AAFwkTag::APPKIT, "scriptDone recorded but signal failed");
    }
    return true;
}

void ArktsScript::ReadCompletionSignal(const CompletionChannel& channel)
{
    uint64_t count = 0;
    ssize_t readSize = -1;
    do {
        readSize = read(channel.eventFd, &count, sizeof(count));
    } while (readSize < 0 && errno == EINTR);
    if (readSize < 0 && errno != EAGAIN) {
        TAG_LOGE(AAFwkTag::APPKIT, "failed to read eventfd, errno=%{public}d", errno);
        ArktsScript::OutputError("failed to read completion signal", "SYSTEM_ERROR");
        ExitFromMonitor(channel, EXIT_FAILURE);
    }
}

ExecutionSnapshot ArktsScript::TakeExecutionSnapshot(const std::shared_ptr<ExecutionContext>& execContext)
{
    ExecutionSnapshot snapshot;
    if (execContext == nullptr) {
        return snapshot;
    }

    std::lock_guard<std::mutex> lock(execContext->mutex);
    snapshot.result = execContext->result;
    snapshot.error = execContext->error;
    snapshot.errorType = execContext->errorType;
    snapshot.resultReady = execContext->resultReady.load(std::memory_order_acquire);
    snapshot.scriptDone = execContext->scriptDone.load(std::memory_order_acquire);
    return snapshot;
}

void ArktsScript::HandleExecutionSnapshot(const ExecutionSnapshot& snapshot, const CompletionChannel& channel)
{
    if (!snapshot.error.empty()) {
        ArktsScript::OutputError(snapshot.error, snapshot.errorType.empty() ? "EXECUTION_ERROR" : snapshot.errorType);
        ExitFromMonitor(channel, EXIT_FAILURE);
    }

    if (snapshot.resultReady && snapshot.scriptDone) {
        ArktsScript::OutputResult(snapshot.result);
        ExitFromMonitor(channel, EXIT_SUCCESS);
    }
}

void ArktsScript::MonitorCompletion(ExitState state, CompletionChannel channel)
{
    epoll_event events[MAX_EVENTS] {};

    while (channel.epollFd >= 0 && channel.eventFd >= 0) {
        int readyCount = epoll_wait(channel.epollFd, events, MAX_EVENTS, -1);
        if (readyCount < 0) {
            if (errno == EINTR) {
                continue;
            }
            TAG_LOGE(AAFwkTag::APPKIT, "epoll_wait failed, errno=%{public}d", errno);
            ArktsScript::OutputError("failed to wait completion signal", "SYSTEM_ERROR");
            ExitFromMonitor(channel, EXIT_FAILURE);
        }

        if (readyCount == 0 || events[0].data.fd != channel.eventFd) {
            continue;
        }

        ReadCompletionSignal(channel);
        HandleExecutionSnapshot(TakeExecutionSnapshot(state.execContext), channel);
    }
}

bool ArktsScript::StartMonitor(CompletionChannel& channel, std::shared_ptr<ExecutionContext>& execContext,
    std::thread& monitorThread)
{
    if (!CreateCompletionChannel(channel)) {
        ArktsScript::OutputError("failed to create completion channel", "SYSTEM_ERROR");
        return false;
    }

    execContext = std::make_shared<ExecutionContext>();
    ExitState exitState { execContext };
    monitorThread = std::thread(MonitorCompletion, exitState, channel);
    return true;
}

int ArktsScript::FinalizeAndJoin(const std::shared_ptr<ExecutionContext>& execContext, CompletionChannel& channel,
    std::thread& monitorThread, const std::string& error, const std::string& errorType)
{
    if (execContext != nullptr) {
        PublishFailure(execContext, channel.eventFd, error, errorType);
    }
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
    return EXIT_FAILURE;
}

ResultCallback ArktsScript::CreateCompleteCallback(const std::shared_ptr<ExecutionContext>& execContext,
    CompletionChannel channel)
{
    return [execContext, channel](bool success, const std::string& result, const std::string& error,
        const std::string& errorType) {
        if (success) {
            PublishResult(execContext, channel.eventFd, result);
            return;
        }
        PublishFailure(execContext, channel.eventFd, error, errorType);
    };
}

bool ArktsScript::PrepareRuntimeEnvironment(const ResultCallback& completeCallback, std::shared_ptr<ContextImpl>& context,
    std::unique_ptr<JsRuntime>& runtimeOwner, napi_env& env, std::string& error, std::string& errorType)
{
    context = ArktsScript::CreateScriptContext();
    if (context == nullptr) {
        error = "failed to create script context";
        errorType = "CONTEXT_ERROR";
        return false;
    }

    runtimeOwner = ArktsScript::CreateJsRuntime(context);
    if (runtimeOwner == nullptr) {
        error = "failed to create js runtime";
        errorType = "RUNTIME_ERROR";
        return false;
    }

    env = runtimeOwner->GetNapiEnv();
    if (env == nullptr) {
        error = "failed to get napi_env";
        errorType = "ENV_ERROR";
        return false;
    }
    if (!JsArktsScript::BindContextToGlobal(env, context)) {
        error = "failed to bind context to globalThis";
        errorType = "BIND_ERROR";
        return false;
    }
    if (!JsArktsScript::BindCompleteArkTsTool(env, completeCallback)) {
        error = "failed to bind completeArkTsTool";
        errorType = "BIND_ERROR";
        return false;
    }
    return true;
}

bool ArktsScript::LoadAndResolveFunction(const ScriptArgs& args, JsRuntime* runtime, napi_env env,
    napi_value& receiver, napi_value& func, std::string& error, std::string& errorType)
{
    TAG_LOGI(AAFwkTag::APPKIT, "loading ABC file: %{private}s", args.abcPath.c_str());
    if (!ArktsScript::LoadAbcFile(runtime, args.abcPath)) {
        error = "failed to load ABC file: " + args.abcPath;
        errorType = "LOAD_ERROR";
        return false;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "resolving function: %{public}s", args.funName.c_str());
    if (!JsArktsScript::ResolveFunction(env, args.scriptName, args.funName, receiver, func)) {
        error = "failed to resolve function: " + args.funName;
        errorType = "FUNCTION_ERROR";
        return false;
    }
    return true;
}

bool ArktsScript::CallResolvedFunction(const ScriptArgs& args, napi_env env, napi_value receiver, napi_value func,
    std::string& error, std::string& errorType)
{
    napi_value argsArray = JsArktsScript::ConvertArgumentsToNapi(env, args.arguments);
    if (argsArray == nullptr) {
        error = "failed to convert arguments";
        errorType = "ARGUMENT_ERROR";
        return false;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "calling function: %{public}s", args.funName.c_str());
    napi_value callResult = nullptr;
    napi_status status = napi_call_function(env, receiver, func, CALL_ARGC, &argsArray, &callResult);
    if (status != napi_ok) {
        error = "failed to call function: " + args.funName + ", napi_status: " + std::to_string(status);
        errorType = "CALL_ERROR";
        return false;
    }

    callResult;
    return true;
}

int ArktsScript::RunArkTsScript(int argc, char* argv[])
{
    ScriptArgs args;
    if (!ArktsScript::ParseArguments(argc, argv, args)) {
        ArktsScript::OutputError("failed to parse arguments", "ARGUMENT_ERROR");
        return EXIT_FAILURE;
    }
    if (args.showHelp) {
        return EXIT_SUCCESS;
    }

    CompletionChannel channel;
    std::shared_ptr<ExecutionContext> execContext;
    std::thread monitorThread;
    if (!StartMonitor(channel, execContext, monitorThread)) {
        return EXIT_FAILURE;
    }

    std::string error;
    std::string errorType;
    std::shared_ptr<ContextImpl> context;
    std::unique_ptr<JsRuntime> runtimeOwner;
    napi_env env = nullptr;
    ResultCallback completeCallback = CreateCompleteCallback(execContext, channel);
    if (!PrepareRuntimeEnvironment(completeCallback, context, runtimeOwner, env, error, errorType)) {
        return FinalizeAndJoin(execContext, channel, monitorThread, error, errorType);
    }

    napi_value receiver = nullptr;
    napi_value func = nullptr;
    if (!LoadAndResolveFunction(args, runtimeOwner.get(), env, receiver, func, error, errorType) ||
        !CallResolvedFunction(args, env, receiver, func, error, errorType)) {
        return FinalizeAndJoin(execContext, channel, monitorThread, error, errorType);
    }

    MarkScriptDone(execContext, channel.eventFd);
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
    return EXIT_FAILURE;
}

} // namespace ArktsScript
} // namespace OHOS
