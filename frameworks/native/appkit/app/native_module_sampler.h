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

#ifndef OHOS_ABILITY_RUNTIME_NATIVE_MODULE_SAMPLER_H
#define OHOS_ABILITY_RUNTIME_NATIVE_MODULE_SAMPLER_H

#include <vector>

#include "native_module_collector.h"

namespace OHOS {
namespace AppExecFwk {
/**
 * @brief Samples the currently loaded native modules from NativeModuleManager.
 *
 * Lives outside the collector so that the collector (and its unit tests) stay
 * free of the napi dependency. The preload (pre-fork) filter is applied inside
 * NativeModuleCollector::AccumulateModules, not here.
 */
class NativeModuleSampler {
public:
    /**
     * @brief Take a snapshot of currently loaded native modules.
     *
     * The module key is NativeModuleInfo.moduleName, falling back to the
     * register name when moduleName is empty (same rule as the runtime-side
     * preload snapshot, so both are comparable).
     *
     * @return sampled modules; empty when the module manager is unavailable
     */
    static std::vector<CollectedModule> Sample();
};
} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_NATIVE_MODULE_SAMPLER_H
