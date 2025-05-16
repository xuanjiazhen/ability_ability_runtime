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

#ifndef OHOS_ABILITY_RUNTIME_CALL_COMMON_UTIL_H
#define OHOS_ABILITY_RUNTIME_CALL_COMMON_UTIL_H

#include <chrono>
#include <condition_variable>
#include "memory.h"
#include <mutex>

#include "caller_callback.h"

namespace OHOS {
namespace AbilityRuntime {
struct StartAbilityByCallData {
    sptr<IRemoteObject> remoteCallee;
    std::mutex mutexlock;
    std::condition_variable condition;
};

class CallCommonUtil {
public:
    static void GenerateCallerCallBack(std::shared_ptr<StartAbilityByCallData> calls,
        std::shared_ptr<CallerCallBack> callerCallBack);

    static void WaitForCalleeObj(std::shared_ptr<StartAbilityByCallData> callData);
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif