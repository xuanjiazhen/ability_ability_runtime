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

#include "arktsScript.h"
#include "hilog_tag_wrapper.h"

int main(int argc, char* argv[])
{
    using namespace OHOS::ArktsScript;

    TAG_LOGI(AAFwkTag::APPKIT, "arktsScript runner start");

    ScriptArgs args;
    if (!ParseArguments(argc, argv, args)) {
        return 1;
    }
    if (args.showHelp) {
        return 0;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "Arguments parsed successfully");
    args.Print();

    TAG_LOGI(AAFwkTag::APPKIT, "Creating lite JsRuntime");
    auto runtime = CreateLiteJsRuntime(args.path);
    if (runtime == nullptr) {
        return 1;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "JsRuntime created successfully");

    napi_env env = runtime->GetNapiEnv();
    if (env == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to get napi_env");
        return 1;
    }

    TAG_LOGI(AAFwkTag::APPKIT, "Creating script context");
    auto context = CreateScriptContext(args.path);
    if (context == nullptr) {
        return 1;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "Script context created successfully");

    TAG_LOGI(AAFwkTag::APPKIT, "Binding script context to globalThis");
    if (!BindContextToGlobal(env, context)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to bind scriptContext to globalThis");
        return 1;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "Script context bound successfully");

    TAG_LOGI(AAFwkTag::APPKIT, "Loading ABC file: %{private}s", args.path.c_str());
    if (!LoadAbcFile(runtime.get(), args.path)) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to load ABC file: %{private}s", args.path.c_str());
        return 1;
    }
    TAG_LOGI(AAFwkTag::APPKIT, "ABC file loaded successfully");

    TAG_LOGI(AAFwkTag::APPKIT, "Calling function: %{public}s", args.funName.c_str());
    CallResult callResult = CallFunction(env, args.funName, args.argv);
    if (!callResult.IsSuccess()) {
        TAG_LOGE(AAFwkTag::APPKIT, "Failed to call function '%{public}s': %{public}s",
            args.funName.c_str(), callResult.errorMessage.c_str());
        return 1;
    }

    const std::string resultString = NapiValueToString(env, callResult.value);
    TAG_LOGI(AAFwkTag::APPKIT, "Function returned: %{public}s", resultString.c_str());
    TAG_LOGI(AAFwkTag::APPKIT, "arktsScript runner completed");
    return 0;
}
