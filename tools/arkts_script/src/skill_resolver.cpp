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

#include "skill_resolver.h"

#include "bundle_mgr_helper.h"
#include "bundle_constants.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace ArktsScript {

using namespace OHOS::AppExecFwk;
using namespace OHOS::AbilityRuntime;

namespace {

int32_t GetBundleInfoFlags()
{
    return static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_APPLICATION) |
        static_cast<int32_t>(GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE);
}

bool QueryBundleInfo(const std::string& bundleName, BundleInfo& bundleInfo)
{
    auto bundleMgrHelper = DelayedSingleton<BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "[skill] failed to create BundleMgrHelper");
        return false;
    }

    bool ret = bundleMgrHelper->GetBundleInfo(
        bundleName, GetBundleInfoFlags(), bundleInfo, Constants::ALL_USERID);
    if (!ret) {
        TAG_LOGE(AAFwkTag::APPKIT,
            "[skill] GetBundleInfo failed for bundleName:%{public}s", bundleName.c_str());
        return false;
    }

    TAG_LOGI(AAFwkTag::APPKIT,
        "[skill] bundle info resolved, bundleName:%{public}s, hapModuleCount:%{public}zu",
        bundleName.c_str(), bundleInfo.hapModuleInfos.size());
    return true;
}

bool FindModuleByName(const std::vector<HapModuleInfo>& modules,
    const std::string& moduleName, HapModuleInfo& matched)
{
    for (const auto& moduleInfo : modules) {
        if (moduleInfo.moduleName == moduleName) {
            matched = moduleInfo;
            return true;
        }
    }
    return false;
}

bool FillSkillResult(const ApplicationInfo& appInfo, const HapModuleInfo& moduleInfo,
    SkillResolutionResult& result)
{
    result.appInfo = appInfo;
    result.hapModuleInfo = moduleInfo;
    result.hapPath = moduleInfo.hapPath;
    result.srcEntrance = moduleInfo.srcEntrance;
    result.moduleName = moduleInfo.moduleName;
    result.isEsModule = (moduleInfo.compileMode == CompileMode::ES_MODULE);

    if (moduleInfo.srcEntrance.empty()) {
        TAG_LOGE(AAFwkTag::APPKIT,
            "[skill] srcEntrance is empty for module:%{public}s", moduleInfo.moduleName.c_str());
        return false;
    }

    result.srcPath = BuildSrcPath(moduleInfo.moduleName, moduleInfo.srcEntrance);
    TAG_LOGI(AAFwkTag::APPKIT,
        "[skill] skill resolved, moduleName:%{public}s, srcPath:%{private}s, "
        "hapPath:%{private}s, isEsModule:%{public}d",
        result.moduleName.c_str(), result.srcPath.c_str(),
        result.hapPath.c_str(), result.isEsModule);
    return true;
}

} // namespace

std::string BuildSrcPath(const std::string& moduleName, const std::string& srcEntrance)
{
    std::string srcPath;
    srcPath.append(moduleName + "/");
    srcPath.append(srcEntrance);
    auto pos = srcPath.rfind(".");
    if (pos != std::string::npos) {
        srcPath.erase(pos);
    }
    srcPath.append(".abc");
    return srcPath;
}

bool ResolveSkillInfo(const std::string& bundleName,
    const std::string& skillName, SkillResolutionResult& result,
    std::string& errorMessage)
{
    BundleInfo bundleInfo;
    if (!QueryBundleInfo(bundleName, bundleInfo)) {
        errorMessage = "failed to get bundle info for " + bundleName;
        return false;
    }

    // TODO: skillProfiles are not yet exposed in HapModuleInfo public interface.
    // Currently falls back to matching by skillName against module mainElementName.
    // Once BMS exposes skillProfiles, replace this with proper skill profile lookup.
    for (const auto& moduleInfo : bundleInfo.hapModuleInfos) {
        if (moduleInfo.mainElementName == skillName) {
            if (FillSkillResult(bundleInfo.applicationInfo, moduleInfo, result)) {
                return true;
            }
        }
    }

    // Fallback: try matching skillName against moduleName
    for (const auto& moduleInfo : bundleInfo.hapModuleInfos) {
        if (moduleInfo.moduleName == skillName) {
            if (FillSkillResult(bundleInfo.applicationInfo, moduleInfo, result)) {
                return true;
            }
        }
    }

    errorMessage = "skill '" + skillName + "' not found in bundle '" + bundleName + "'";
    TAG_LOGE(AAFwkTag::APPKIT, "%{public}s", errorMessage.c_str());
    return false;
}

bool ResolveModuleInfo(const std::string& bundleName,
    const std::string& moduleName, SkillResolutionResult& result,
    std::string& errorMessage)
{
    BundleInfo bundleInfo;
    if (!QueryBundleInfo(bundleName, bundleInfo)) {
        errorMessage = "failed to get bundle info for " + bundleName;
        return false;
    }

    HapModuleInfo matchedModule;
    if (!FindModuleByName(bundleInfo.hapModuleInfos, moduleName, matchedModule)) {
        errorMessage = "module '" + moduleName + "' not found in bundle '" + bundleName + "'";
        TAG_LOGE(AAFwkTag::APPKIT, "%{public}s", errorMessage.c_str());
        return false;
    }

    if (!FillSkillResult(bundleInfo.applicationInfo, matchedModule, result)) {
        errorMessage = "failed to resolve module info for '" + moduleName + "'";
        return false;
    }
    return true;
}

std::shared_ptr<ContextImpl> CreateScriptContextForBundle(const std::string& bundleName)
{
    auto contextImpl = std::make_shared<ContextImpl>();
    if (contextImpl == nullptr) {
        TAG_LOGE(AAFwkTag::APPKIT, "[skill] failed to create ContextImpl");
        return nullptr;
    }

    BundleInfo bundleInfo;
    if (!QueryBundleInfo(bundleName, bundleInfo)) {
        TAG_LOGE(AAFwkTag::APPKIT,
            "[skill] failed to get bundle info while creating context for %{public}s",
            bundleName.c_str());
        return nullptr;
    }

    contextImpl->SetApplicationInfo(
        std::make_shared<ApplicationInfo>(bundleInfo.applicationInfo));
    contextImpl->SetProcessName(bundleInfo.applicationInfo.process);
    return contextImpl;
}

} // namespace ArktsScript
} // namespace OHOS
