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

#include "js_script_manager_local_callback.h"

#include <mutex>

namespace OHOS {
namespace AbilityRuntime {
namespace {
std::mutex g_localCompleteMutex;
ScriptManagerLocalCompleteCallback g_localCompleteCallback = nullptr;
} // namespace

void RegisterScriptManagerLocalCompleteCallback(const ScriptManagerLocalCompleteCallback &callback)
{
    std::lock_guard<std::mutex> lock(g_localCompleteMutex);
    g_localCompleteCallback = callback;
}

void ClearScriptManagerLocalCompleteCallback()
{
    std::lock_guard<std::mutex> lock(g_localCompleteMutex);
    g_localCompleteCallback = nullptr;
}

ScriptManagerLocalCompleteCallback TakeScriptManagerLocalCompleteCallback()
{
    std::lock_guard<std::mutex> lock(g_localCompleteMutex);
    auto callback = g_localCompleteCallback;
    g_localCompleteCallback = nullptr;
    return callback;
}

} // namespace AbilityRuntime
} // namespace OHOS
