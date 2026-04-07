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

#include <iostream>

#include "arkTSScript.h"

int main(int argc, char* argv[])
{
    using namespace ArkTSScript;

    std::cout << "=== ArkTSScript Runner ===" << std::endl;

    ScriptArgs args;
    if (!ParseArguments(argc, argv, args)) {
        return 1;
    }
    if (args.showHelp) {
        return 0;
    }

    std::cout << "\nParsed arguments:" << std::endl;
    args.Print();
    std::cout << std::endl;

    std::cout << "Creating lite JsRuntime..." << std::endl;
    auto runtime = CreateLiteJsRuntime(args.path);
    if (runtime == nullptr) {
        return 1;
    }
    std::cout << "JsRuntime created successfully" << std::endl;

    napi_env env = runtime->GetNapiEnv();
    if (env == nullptr) {
        std::cerr << "Failed to get napi_env" << std::endl;
        return 1;
    }

    std::cout << "Creating script context..." << std::endl;
    auto context = CreateScriptContext(args.path);
    if (context == nullptr) {
        return 1;
    }
    std::cout << "Context created successfully" << std::endl;

    std::cout << "Binding context to globalThis..." << std::endl;
    if (!BindContextToGlobal(env, context)) {
        std::cerr << "Failed to bind scriptContext to globalThis" << std::endl;
        return 1;
    }
    std::cout << "Context bound successfully" << std::endl;

    std::cout << "Loading ABC file: " << args.path << std::endl;
    if (!LoadAbcFile(runtime.get(), args.path)) {
        std::cerr << "Failed to load ABC file: " << args.path << std::endl;
        return 1;
    }
    std::cout << "ABC file loaded successfully" << std::endl;

    std::cout << "Calling function: " << args.funName << "..." << std::endl;
    CallResult callResult = CallFunction(env, args.funName, args.argv);
    if (!callResult.IsSuccess()) {
        std::cerr << "Failed to call function '" << args.funName << "': "
                  << callResult.errorMessage << std::endl;
        return 1;
    }

    std::cout << "Function returned: " << NapiValueToString(env, callResult.value) << std::endl;
    std::cout << "\n=== Execution completed ===" << std::endl;
    return 0;
}
