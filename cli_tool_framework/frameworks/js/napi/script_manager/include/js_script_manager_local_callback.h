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

#ifndef OHOS_ABILITY_RUNTIME_JS_SCRIPT_MANAGER_LOCAL_CALLBACK_H
#define OHOS_ABILITY_RUNTIME_JS_SCRIPT_MANAGER_LOCAL_CALLBACK_H

#include <functional>
#include <string>

#ifdef _WIN32
#define JS_SCRIPT_MANAGER_LOCAL_CALLBACK_EXPORT __declspec(dllexport)
#else
#define JS_SCRIPT_MANAGER_LOCAL_CALLBACK_EXPORT __attribute__((visibility("default")))
#endif

namespace OHOS {
namespace AbilityRuntime {

using ScriptManagerLocalCompleteCallback = std::function<void(bool success, const std::string &result,
    const std::string &errorMessage, const std::string &errorType)>;

JS_SCRIPT_MANAGER_LOCAL_CALLBACK_EXPORT
void RegisterScriptManagerLocalCompleteCallback(const ScriptManagerLocalCompleteCallback &callback);
JS_SCRIPT_MANAGER_LOCAL_CALLBACK_EXPORT
void ClearScriptManagerLocalCompleteCallback();
JS_SCRIPT_MANAGER_LOCAL_CALLBACK_EXPORT
ScriptManagerLocalCompleteCallback TakeScriptManagerLocalCompleteCallback();

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_JS_SCRIPT_MANAGER_LOCAL_CALLBACK_H
