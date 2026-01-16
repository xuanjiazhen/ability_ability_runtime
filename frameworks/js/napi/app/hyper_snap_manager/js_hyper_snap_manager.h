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

#ifndef OHOS_ABILITY_RUNTIME_JS_HYPER_SNAP_MANAGER_H
#define OHOS_ABILITY_RUNTIME_JS_HYPER_SNAP_MANAGER_H

#include <unordered_map>

#include "native_engine/native_engine.h"

namespace OHOS {
namespace AbilityRuntime {
using ReportSyncEventFunc = int32_t (*)(const uint32_t resType, const int64_t value,
    const unordered_map<std::string, std::string> &payload, const unordered_map<std::string, std::string> &reply);
ReportSyncEventFunc LoadHyperSnapManagerReportSynvEventFunc();
napi_value JsHyperSnapManagerInit(napi_env env, napi_value ecportObj);
} // namespace AbilityRuntime
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_JS_HYPER_SNAP_MANAGER_H
