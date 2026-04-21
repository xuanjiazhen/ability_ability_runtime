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

#ifndef SKILL_RESOLVER_H
#define SKILL_RESOLVER_H

#include <string>
#include <memory>

#include "application_info.h"
#include "hap_module_info.h"
#include "context_impl.h"

namespace OHOS {
namespace ArktsScript {

using namespace OHOS::AppExecFwk;

struct SkillResolutionResult {
    ApplicationInfo appInfo;
    HapModuleInfo hapModuleInfo;
    std::string srcPath;       // built .abc relative path
    std::string hapPath;       // HSP file absolute path
    std::string srcEntrance;   // original srcEntrance
    std::string moduleName;
    bool isEsModule = true;
};

bool ResolveSkillInfo(const std::string& bundleName,
    const std::string& skillName, SkillResolutionResult& result,
    std::string& errorMessage);

bool ResolveModuleInfo(const std::string& bundleName,
    const std::string& moduleName, SkillResolutionResult& result,
    std::string& errorMessage);

std::string BuildSrcPath(const std::string& moduleName,
    const std::string& srcEntrance);

std::shared_ptr<AbilityRuntime::ContextImpl> CreateScriptContextForBundle(
    const std::string& bundleName);

} // namespace ArktsScript
} // namespace OHOS

#endif // SKILL_RESOLVER_H
