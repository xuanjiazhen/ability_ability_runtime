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

#include "standalone_skill_utils.h"

#include <mutex>

namespace OHOS {
namespace AbilityRuntime {
namespace {

std::mutex g_callbackMutex;
StandaloneScriptCompleteCallback g_callback;

} // namespace

bool RegisterStandaloneScriptCompleteCallback(StandaloneScriptCompleteCallback callback)
{
    if (!callback) {
        return false;
    }

    std::lock_guard<std::mutex> lock(g_callbackMutex);
    g_callback = std::move(callback);
    return true;
}

void ClearStandaloneScriptCompleteCallback()
{
    std::lock_guard<std::mutex> lock(g_callbackMutex);
    g_callback = nullptr;
}

bool CompleteStandaloneScript(const StandaloneScriptExecuteResult& result)
{
    StandaloneScriptCompleteCallback callback;
    {
        std::lock_guard<std::mutex> lock(g_callbackMutex);
        if (!g_callback) {
            return false;
        }
        callback = g_callback;
    }

    callback(result);
    return true;
}

} // namespace AbilityRuntime
} // namespace OHOS
