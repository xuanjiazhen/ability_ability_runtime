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
#include <cctype>
#include <charconv>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <map>
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

constexpr uint64_t SIGNAL_VALUE = 1;
constexpr int MAX_EVENTS = 1;
constexpr size_t CALL_ARGC = 1;
constexpr char ABC_PATH_OPTION[] = "--abcPath";
constexpr char SCRIPT_PATH_OPTION[] = "--scriptPath";
constexpr char FUNCTION_NAME_OPTION[] = "--functionName";
constexpr char ARGS_OPTION[] = "--args";
constexpr char ARG_NAME_PREFIX[] = "arg";
constexpr size_t ARG_NAME_PREFIX_LENGTH = sizeof(ARG_NAME_PREFIX) - 1;

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

bool IsArgName(const std::string& name)
{
    if (name.size() <= ARG_NAME_PREFIX_LENGTH ||
        name.compare(0, ARG_NAME_PREFIX_LENGTH, ARG_NAME_PREFIX) != 0) {
        return false;
    }
    return std::all_of(name.begin() + ARG_NAME_PREFIX_LENGTH, name.end(),
        [](unsigned char ch) { return std::isdigit(ch) != 0; });
}

bool ParseArgNameIndex(const std::string& name, uint32_t& index)
{
    if (!IsArgName(name)) {
        return false;
    }
    const std::string indexText = name.substr(ARG_NAME_PREFIX_LENGTH);
    auto ret = std::from_chars(indexText.data(), indexText.data() + indexText.size(), index);
    return ret.ec == std::errc() && ret.ptr == indexText.data() + indexText.size();
}

bool ParseJsonValueToScriptArg(const nlohmann::json& value, ScriptArg& arg)
{
    if (value.is_string()) {
        arg.type = ScriptArgType::STRING;
        arg.value = value.get<std::string>();
        return true;
    }
    if (value.is_boolean()) {
        arg.type = ScriptArgType::BOOLEAN;
        arg.value = value.get<bool>() ? "1" : "0";
        return true;
    }
    if (value.is_number_integer()) {
        if (value.is_number_unsigned()) {
            uint64_t number = value.get<uint64_t>();
            if (number <= static_cast<uint64_t>(std::numeric_limits<int32_t>::max())) {
                arg.type = ScriptArgType::INT32;
            } else {
                arg.type = ScriptArgType::DOUBLE;
            }
        } else {
            int64_t number = value.get<int64_t>();
            if (number >= std::numeric_limits<int32_t>::min() && number <= std::numeric_limits<int32_t>::max()) {
                arg.type = ScriptArgType::INT32;
            } else {
                arg.type = ScriptArgType::DOUBLE;
            }
        }
        arg.value = value.dump();
        return true;
    }
    if (value.is_number_float()) {
        arg.type = ScriptArgType::DOUBLE;
        arg.value = value.dump();
        return true;
    }
    if (value.is_object() || value.is_array()) {
        arg.type = ScriptArgType::JSON_VALUE;
        arg.value = value.dump();
        return true;
    }
    return false;
}

bool ParseArgsJson(const std::string& rawValue, std::map<uint32_t, ScriptArg>& indexedArgs)
{
    nlohmann::json argsJson = nlohmann::json::parse(rawValue, nullptr, false);
    if (argsJson.is_discarded() || !argsJson.is_object()) {
        return false;
    }

    for (auto it = argsJson.begin(); it != argsJson.end(); ++it) {
        uint32_t argIndex = 0;
        if (!ParseArgNameIndex(it.key(), argIndex)) {
            return false;
        }
        ScriptArg arg;
        if (!ParseJsonValueToScriptArg(it.value(), arg)) {
            return false;
        }
        if (!indexedArgs.emplace(argIndex, std::move(arg)).second) {
            return false;
        }
    }
    return true;
}

bool ReadOptionValue(int argc, char* argv[], int& index, std::string& value)
{
    if (index + 1 >= argc || argv[index + 1] == nullptr) {
        return false;
    }
    value = argv[++index];
    return true;
}

} // namespace

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

bool ArktsScript::ParseFunctionAndArguments(int argc, char* argv[], int& index, ScriptArgs& args)
{
    std::map<uint32_t, ScriptArg> indexedArgs;
    bool hasArgsJson = false;
    while (index < argc) {
        if (argv[index] == nullptr) {
            index++;
            continue;
        }

        std::string option = argv[index];
        std::string value;
        if (option == ABC_PATH_OPTION) {
            if (!ReadOptionValue(argc, argv, index, value)) {
                TAG_LOGE(AAFwkTag::APPKIT, "Missing value for --abcPath");
                return false;
            }
            args.abcPath = value;
        } else if (option == SCRIPT_PATH_OPTION) {
            if (!ReadOptionValue(argc, argv, index, value)) {
                TAG_LOGE(AAFwkTag::APPKIT, "Missing value for --scriptPath");
                return false;
            }
            args.scriptName = value;
        } else if (option == FUNCTION_NAME_OPTION) {
            if (!ReadOptionValue(argc, argv, index, value)) {
                TAG_LOGE(AAFwkTag::APPKIT, "Missing value for --functionName");
                return false;
            }
            args.funName = value;
        } else if (option == ARGS_OPTION) {
            if (hasArgsJson || !ReadOptionValue(argc, argv, index, value)) {
                TAG_LOGE(AAFwkTag::APPKIT, "Invalid --args option");
                return false;
            }
            if (!ParseArgsJson(value, indexedArgs)) {
                TAG_LOGE(AAFwkTag::APPKIT, "Invalid value for --args");
                return false;
            }
            hasArgsJson = true;
        } else {
            TAG_LOGE(AAFwkTag::APPKIT, "Unknown option: %{public}s", option.c_str());
            PrintUsage(argv[0]);
            return false;
        }
        index++;
    }

    if (args.abcPath.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Missing --abcPath option");
        PrintUsage(argv[0]);
        return false;
    }
    if (args.funName.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Missing --functionName option");
        PrintUsage(argv[0]);
        return false;
    }

    args.arguments.clear();
    if (!indexedArgs.empty()) {
        const uint32_t maxArgIndex = indexedArgs.rbegin()->first;
        args.arguments.resize(static_cast<size_t>(maxArgIndex) + 1);
        for (const auto& item : indexedArgs) {
            args.arguments[item.first] = item.second;
        }
    }
    return true;
}

void ArktsScript::PrintUsage(const char* execName)
{
    const std::string usage = std::string("Usage: ") + execName +
        " [options] --abcPath module.abc --scriptPath [script.ets] --functionName functionName "
        "[--args '{\"arg0\":10}']\n\n"
        "Options:\n"
        "  -h, --help    Show this help message\n\n"
        "Arguments:\n"
        "  --abcPath module.abc          ABC file path (required)\n"
        "  --scriptPath script.ets       Script class name for standalone skill (optional)\n"
        "  --functionName functionName   Function name to execute (required)\n"
        "  --args jsonObject             JSON object arguments, keys must be argN, such as "
        "{\"arg0\":10,\"arg1\":20}\n\n";

    TAG_LOGI(AAFwkTag::APPKIT, "%{public}s", usage.c_str());
}

bool ArktsScript::ParseArguments(int argc, char* argv[], ScriptArgs& args)
{
    if (argc <= 0 || argv == nullptr || argv[0] == nullptr) {
        return false;
    }
    if (ParseHelpOption(argc, argv, args)) {
        return true;
    }

    int i = 1;
    if (argc <= i) {
        PrintUsage(argv[0]);
        return false;
    }

    return ParseFunctionAndArguments(argc, argv, i, args);
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

    TAG_LOGI(AAFwkTag::APPKIT, "RunScript start, runScriptPath: %{public}s, targetPath: %{public}s",
        path.c_str(), path.c_str());
    bool loaded = runtime->RunScript(path, "", false);
    if (!loaded) {
        TAG_LOGE(AAFwkTag::APPKIT, "RunScript failed, runScriptPath: %{public}s, targetPath: %{public}s",
            path.c_str(), path.c_str());
    }
    return loaded;
}

void ArktsScript::OutputResult(const std::string& result)
{
    std::cout << result << std::endl;
}

void ArktsScript::OutputError(const ScriptError& error)
{
    nlohmann::json errorOutput;
    errorOutput["success"] = false;
    errorOutput["errorType"] = error.type.empty() ? "EXECUTION_ERROR" : error.type;
    errorOutput["error"] = error.message;
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
        execContext->error = {};
        execContext->resultReady.store(true, std::memory_order_release);
    }

    if (!SignalCompletion(eventFd)) {
        TAG_LOGE(AAFwkTag::APPKIT, "result recorded but signal failed");
    }
    return true;
}

bool ArktsScript::PublishFailure(const std::shared_ptr<ExecutionContext>& execContext, int eventFd,
    const ScriptError& error)
{
    if (execContext == nullptr) {
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(execContext->mutex);
        if (!execContext->error.message.empty()) {
            TAG_LOGW(AAFwkTag::APPKIT, "duplicate failure ignored");
            return false;
        }
        execContext->result.clear();
        execContext->error = error;
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
        ArktsScript::OutputError({"failed to read completion signal", "SYSTEM_ERROR"});
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
    snapshot.resultReady = execContext->resultReady.load(std::memory_order_acquire);
    snapshot.scriptDone = execContext->scriptDone.load(std::memory_order_acquire);
    return snapshot;
}

void ArktsScript::HandleExecutionSnapshot(const ExecutionSnapshot& snapshot, const CompletionChannel& channel)
{
    if (!snapshot.error.message.empty()) {
        ArktsScript::OutputError(snapshot.error);
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
            ArktsScript::OutputError({"failed to wait completion signal", "SYSTEM_ERROR"});
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
        ArktsScript::OutputError({"failed to create completion channel", "SYSTEM_ERROR"});
        return false;
    }

    execContext = std::make_shared<ExecutionContext>();
    ExitState exitState { execContext };
    monitorThread = std::thread(MonitorCompletion, exitState, channel);
    return true;
}

int ArktsScript::FinalizeAndJoin(const std::shared_ptr<ExecutionContext>& execContext, CompletionChannel& channel,
    std::thread& monitorThread, const ScriptError& error)
{
    if (execContext != nullptr) {
        PublishFailure(execContext, channel.eventFd, error);
    }
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
    return EXIT_FAILURE;
}

ResultCallback ArktsScript::CreateCompleteCallback(const std::shared_ptr<ExecutionContext>& execContext,
    CompletionChannel channel)
{
    return [execContext, channel](bool success, const std::string& result, const ScriptError& error) {
        if (success) {
            PublishResult(execContext, channel.eventFd, result);
            return;
        }
        PublishFailure(execContext, channel.eventFd, error);
    };
}

bool ArktsScript::PrepareRuntimeEnvironment(const ResultCallback& completeCallback,
    std::shared_ptr<ContextImpl>& context, std::unique_ptr<JsRuntime>& runtimeOwner,
    napi_env& env, ScriptError& error)
{
    context = ArktsScript::CreateScriptContext();
    if (context == nullptr) {
        error.message = "failed to create script context";
        error.type = "CONTEXT_ERROR";
        return false;
    }

    runtimeOwner = ArktsScript::CreateJsRuntime(context);
    if (runtimeOwner == nullptr) {
        error.message = "failed to create js runtime";
        error.type = "RUNTIME_ERROR";
        return false;
    }

    env = runtimeOwner->GetNapiEnv();
    if (env == nullptr) {
        error.message = "failed to get napi_env";
        error.type = "ENV_ERROR";
        return false;
    }
    if (!JsArktsScript::BindContextToGlobal(env, context)) {
        error.message = "failed to bind context to globalThis";
        error.type = "BIND_ERROR";
        return false;
    }
    if (!JsArktsScript::BindCompletearktsScript(env, completeCallback)) {
        error.message = "failed to bind CompletearktsScript";
        error.type = "BIND_ERROR";
        return false;
    }
    return true;
}

bool ArktsScript::LoadScriptFile(const ScriptArgs& args, JsRuntime* runtime, ScriptError& error)
{
    TAG_LOGI(AAFwkTag::APPKIT,
        "loading ABC file, runScriptPath: %{public}s, targetPath: %{public}s, scriptName: %{public}s",
        args.abcPath.c_str(), args.abcPath.c_str(), args.scriptName.c_str());
    if (!ArktsScript::LoadAbcFile(runtime, args.abcPath)) {
        error.message = "failed to load ABC file, runScriptPath: " + args.abcPath +
            ", targetPath: " + args.abcPath;
        error.type = "LOAD_ERROR";
        return false;
    }
    return true;
}

bool ArktsScript::ResolveScriptFunction(const ScriptArgs& args, JsRuntime* runtime, napi_env env,
    napi_value& receiver, napi_value& func, ScriptError& error)
{
    TAG_LOGI(AAFwkTag::APPKIT, "resolving function: %{public}s", args.funName.c_str());
    if (!JsArktsScript::ResolveFunction(runtime, env, args.abcPath, args.scriptName, args.funName, receiver, func)) {
        error.message = "failed to resolve function: " + args.funName +
            ", loaded abcPath: " + args.abcPath +
            ", targetPath: " + args.abcPath +
            ", scriptName: " + args.scriptName;
        error.type = "FUNCTION_ERROR";
        return false;
    }
    return true;
}

bool ArktsScript::CallResolvedFunction(const ScriptArgs& args, napi_env env, napi_value receiver, napi_value func,
    ScriptError& error)
{
    napi_value argsArray = JsArktsScript::ConvertArgumentsToNapi(env, args.arguments);
    if (argsArray == nullptr) {
        error.message = "failed to convert arguments";
        error.type = "ARGUMENT_ERROR";
        return false;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "calling function: %{public}s", args.funName.c_str());
    napi_value callResult = nullptr;
    napi_status status = napi_call_function(env, receiver, func, CALL_ARGC, &argsArray, &callResult);
    if (status != napi_ok) {
        error.message = "failed to call function: " + args.funName + ", napi_status: " + std::to_string(status);
        error.type = "CALL_ERROR";
        return false;
    }
    return true;
}

int ArktsScript::RunArkTsScript(int argc, char* argv[])
{
    ScriptArgs args;
    if (!ArktsScript::ParseArguments(argc, argv, args)) {
        ArktsScript::OutputError({"failed to parse arguments", "ARGUMENT_ERROR"});
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

    ScriptError error;
    std::shared_ptr<ContextImpl> context;
    std::unique_ptr<JsRuntime> runtimeOwner;
    napi_env env = nullptr;
    ResultCallback completeCallback = CreateCompleteCallback(execContext, channel);
    if (!PrepareRuntimeEnvironment(completeCallback, context, runtimeOwner, env, error)) {
        return FinalizeAndJoin(execContext, channel, monitorThread, error);
    }

    napi_value receiver = nullptr;
    napi_value func = nullptr;
    if (!LoadScriptFile(args, runtimeOwner.get(), error) ||
        !ResolveScriptFunction(args, runtimeOwner.get(), env, receiver, func, error) ||
        !CallResolvedFunction(args, env, receiver, func, error)) {
        return FinalizeAndJoin(execContext, channel, monitorThread, error);
    }

    MarkScriptDone(execContext, channel.eventFd);
    if (monitorThread.joinable()) {
        monitorThread.join();
    }
    return EXIT_FAILURE;
}

} // namespace ArktsScript
} // namespace OHOS
