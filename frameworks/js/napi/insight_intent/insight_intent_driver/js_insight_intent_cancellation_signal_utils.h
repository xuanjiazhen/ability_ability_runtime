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

#ifndef OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_CANCELLATION_SIGNAL_UTILS_H
#define OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_CANCELLATION_SIGNAL_UTILS_H

#include <atomic>
#include <memory>
#include <string>

#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
// Caller-side CancellationSignal data shared by the insightIntentDriver and
// functionManager cancellation paths. It owns a NAPI-generated signalId plus two
// shared flags:
//   - canceled: flipped locally by cancel() before the AMS IPC, so isCancelled()
//     turns true synchronously;
//   - bound: true while the signal is attached to an in-flight execution, so a
//     second execute() on the same signal can be rejected with a busy error.
struct JsCancellationSignalData {
    std::string signalId;
    std::shared_ptr<std::atomic<bool>> canceled = nullptr;
    std::shared_ptr<std::atomic<bool>> bound = nullptr;
};

std::string GenerateSignalId();

// Extracts the signalId from a caller-side CancellationSignal JS object. Returns
// false when value is null/undefined or not a CancellationSignal instance.
//
// Header-inline so both the insightIntentDriver and the functionManager native
// modules (which live in separate shared libraries) share a single cross-module
// implementation without linking against each other's library.
inline bool GetCancellationSignalId(napi_env env, napi_value value, std::string &signalId)
{
    if (value == nullptr) {
        return false;
    }
    napi_valuetype type = napi_undefined;
    napi_typeof(env, value, &type);
    if (type == napi_undefined || type == napi_null) {
        return false;
    }
    JsCancellationSignalData *data = nullptr;
    napi_unwrap(env, value, reinterpret_cast<void **>(&data));
    if (data == nullptr) {
        return false;
    }
    signalId = data->signalId;
    return true;
}
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_INSIGHT_INTENT_CANCELLATION_SIGNAL_UTILS_H
