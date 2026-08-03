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

#include "native_module_sampler.h"

#include "hilog_tag_wrapper.h"
#include "native_module_manager.h"

namespace OHOS {
namespace AppExecFwk {
std::vector<CollectedModule> NativeModuleSampler::Sample()
{
    std::vector<CollectedModule> modules;
    NativeModuleManager* moduleManager = NativeModuleManager::GetInstance();
    if (moduleManager == nullptr) {
        return modules;
    }
    std::vector<NativeModuleInfo> infos;
    moduleManager->GetLoadedModuleInfos(infos);
    TAG_LOGD(AAFwkTag::APPKIT, "sample: loaded=%{public}zu", infos.size());
    modules.reserve(infos.size());
    for (const auto& info : infos) {
        std::string key = !info.moduleName.empty() ? info.moduleName : info.name;
        if (key.empty()) {
            continue;
        }
        CollectedModule module;
        module.moduleName = std::move(key);
        module.fileName = info.fileName;
        module.relativePath = info.relativePath;
        module.isAppModule = info.isAppModule;
        module.loadCount = info.loadCount;
        modules.push_back(std::move(module));
    }
    return modules;
}
} // namespace AppExecFwk
} // namespace OHOS
