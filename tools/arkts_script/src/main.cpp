int main(int argc, char* argv[])
{
    TAG_LOGI(AAFwkTag::APPKIT, "arktsScript runner start");
    
    // ========== 阶段1: 命令行解析 ==========
    ScriptArgs args;
    if (!ParseArguments(argc, argv, args)) {
        OutputError("failed to parse arguments", "ARGUMENT_ERROR");
        return EXIT_FAILURE;
    }
    if (args.showHelp) {
        return EXIT_SUCCESS;
    }
    
    // ========== 阶段2: 查询包管理器 ==========
    PackageInfo pkgInfo;
    if (!QueryPackageManager(args.path, pkgInfo)) {
        OutputError("failed to query package manager", "PACKAGE_ERROR");
        return EXIT_FAILURE;
    }
    
    // ========== 阶段3: 初始化 ==========
    // 创建 ExecutionContext
    g_execContext = std::make_shared<ExecutionContext>();
    
    // 创建 Runtime/Context
    std::shared_ptr<ContextImpl> context;
    std::unique_ptr<JsRuntime> ownedRuntime;
    std::shared_ptr<JsRuntime> runtime;
    
    if (pkgInfo.isStandaloneSkill) {
        context = CreateScriptContext();
        ownedRuntime = CreateScriptJsRuntime(context);
        runtime = ownedRuntime;
    } else {
        context = GetExistingContext();
        runtime = GetExistingRuntime();
    }
    
    napi_env env = runtime->GetNapiEnv();
    auto runner = EventRunner::GetMainEventRunner();
    g_execContext->runner = runner;
    
    // 绑定 completeArkTsTool（传入 OnCompleteArkTsTool 回调）
    if (!BindCompleteArkTsTool(env, OnCompleteArkTsTool)) {
        OutputError("failed to bind completeArkTsTool", "BIND_ERROR");
        return EXIT_FAILURE;
    }
    
    // 绑定 scriptContext
    if (!BindContextToGlobal(env, context)) {
        OutputError("failed to bind context", "BIND_ERROR");
        return EXIT_FAILURE;
    }
    
    // 加载 ABC
    if (!LoadAbcFile(runtime.get(), args.path)) {
        OutputError("failed to load ABC file", "LOAD_ERROR");
        return EXIT_FAILURE;
    }
    
    // ========== 阶段4: 获取并调用函数 ==========
    napi_value func = nullptr;
    if (pkgInfo.isStandaloneSkill) {
        if (!GetStandaloneFunction(env, args.scriptClass, args.funName, func)) {
            OutputError("failed to get standalone function", "FUNCTION_ERROR");
            return EXIT_FAILURE;
        }
    } else {
        if (!GetStaticFunction(env, args.funName, func)) {
            OutputError("failed to get static function", "FUNCTION_ERROR");
            return EXIT_FAILURE;
        }
    }
    
    // ========== 阶段5: 启动监听线程，调用函数 ==========
    constexpr int64_t TIMEOUT_MS = 300 * 1000;  // 300秒
    
    // 启动监听线程
    std::thread listenerThread(ListenerThread, g_execContext, TIMEOUT_MS);
    
    // 在 EventRunner 线程中调用 JS 函数
    // 使用 PostTask 将函数调用投递到 JS 线程
    bool callSuccess = false;
    runtime->PostTask([&]() {
        TAG_LOGI(AAFwkTag::APPKIT, "Calling function: %{public}s", args.funName.c_str());
        callSuccess = CallFunctionNoWait(env, func, args.params);
        if (!callSuccess) {
            // 调用失败，通知监听线程
            std::lock_guard<std::mutex> lock(g_execContext->mutex);
            g_execContext->error = "failed to call function";
            g_execContext->completed = true;
            g_execContext->cv.notify_one();
        }
    });
    
    // 启动 EventRunner（在单独线程中运行）
    std::thread jsThread([&runner]() {
        TAG_LOGI(AAFwkTag::APPKIT, "JS thread: EventRunner starting");
        runner->Run();
        TAG_LOGI(AAFwkTag::APPKIT, "JS thread: EventRunner stopped");
    });
    
    // ========== 阶段6: 主线程等待结果 ==========
    // 等待监听线程完成
    listenerThread.join();
    
    // 等待 JS 线程完成（应该很快，因为 runner 已被停止）
    jsThread.join();
    
    // ========== 阶段7: 输出结果 ==========
    if (g_execContext->timedOut) {
        OutputError("execution timeout", "TIMEOUT_ERROR");
        return EXIT_FAILURE;
    }
    
    if (!g_execContext->error.empty()) {
        OutputError(g_execContext->error, "EXECUTION_ERROR");
        return EXIT_FAILURE;
    }
    
    // 输出结果
    std::cout << g_execContext->result << std::endl;
    
    TAG_LOGI(AAFwkTag::APPKIT, "arktsScript runner completed");
    return EXIT_SUCCESS;
}