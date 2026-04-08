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

#ifndef ARKTSSCRIPT_H
#define ARKTSSCRIPT_H

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include "context_impl.h"
#include "js_runtime.h"
#include "napi/native_api.h"

namespace OHOS {
namespace ArktsScript {

enum class ArgType {
    STRING,
    NUMBER,
    BOOLEAN,
    ARRAY,
    JSON_OBJECT,
    NULL_VAL,
    UNDEFINED,
};

struct ArgValue {
    ArgType type;
    std::variant<std::string, double, bool> value;

    ArgValue() : type(ArgType::UNDEFINED), value(std::string("undefined")) {}
    explicit ArgValue(const std::string& str) : type(ArgType::STRING), value(str) {}
    explicit ArgValue(double num) : type(ArgType::NUMBER), value(num) {}
    explicit ArgValue(bool b) : type(ArgType::BOOLEAN), value(b) {}
    explicit ArgValue(std::nullptr_t) : type(ArgType::NULL_VAL), value(std::string("null")) {}

    std::string GetString() const;
    double GetNumber() const;
    bool GetBool() const;
    std::string ToString() const;
};

struct ScriptArgs {
    std::string path;
    std::string funName;
    std::vector<ArgValue> argv;
    bool showHelp = false;

    void Print() const;
};

enum class CallStatus {
    SUCCESS = 0,
    INVALID_ENV,
    ARGUMENT_CONVERSION_FAILED,
    FUNCTION_NOT_FOUND,
    PROPERTY_ACCESS_FAILED,
    NOT_A_FUNCTION,
    CALL_FAILED,
};

struct CallResult {
    CallStatus status = CallStatus::CALL_FAILED;
    napi_value value = nullptr;
    std::string errorMessage;

    bool IsSuccess() const
    {
        return status == CallStatus::SUCCESS;
    }
};

bool ParseArguments(int argc, char* argv[], ScriptArgs& args);

std::unique_ptr<OHOS::AbilityRuntime::JsRuntime> CreateLiteJsRuntime(const std::string& scriptPath);

std::shared_ptr<OHOS::AbilityRuntime::ContextImpl> CreateScriptContext(const std::string& scriptPath);

bool BindContextToGlobal(
    napi_env env, const std::shared_ptr<OHOS::AbilityRuntime::ContextImpl>& context);

bool LoadAbcFile(OHOS::AbilityRuntime::JsRuntime* runtime, const std::string& path);

CallResult CallFunction(napi_env env, const std::string& funName, const std::vector<ArgValue>& argv);

napi_value ArgValueToNapi(napi_env env, const ArgValue& arg);

std::string NapiValueToString(napi_env env, napi_value value);

bool ParseTypedArg(const std::string& input, ArgValue& argValue, std::string& errorMessage);

} // namespace ArktsScript
} // namespace OHOS

#endif // ARKTSSCRIPT_H
