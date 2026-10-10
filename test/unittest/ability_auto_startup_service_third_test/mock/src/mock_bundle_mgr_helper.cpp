nt32_t &appIndex)
/*
ppIndex = 0;
nt32_t &appIndex)
 * Copyright (c) 2025 Huawei Device Co., Ltd.
ppIndex = 0;
nt32_t &appIndex)
 * Licensed under the Apache License, Version 2.0 (the "License");
ppIndex = 0;
nt32_t &appIndex)
 * you may not use this file except in compliance with the License.
ppIndex = 0;
nt32_t &appIndex)
 * You may obtain a copy of the License at
ppIndex = 0;
nt32_t &appIndex)
 *
ppIndex = 0;
nt32_t &appIndex)
 *     http://www.apache.org/licenses/LICENSE-2.0
ppIndex = 0;
nt32_t &appIndex)
 *
ppIndex = 0;
nt32_t &appIndex)
 * Unless required by applicable law or agreed to in writing, software
ppIndex = 0;
nt32_t &appIndex)
 * distributed under the License is distributed on an "AS IS" BASIS,
ppIndex = 0;
nt32_t &appIndex)
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
ppIndex = 0;
nt32_t &appIndex)
 * See the License for the specific language governing permissions and
ppIndex = 0;
nt32_t &appIndex)
 * limitations under the License.
ppIndex = 0;
nt32_t &appIndex)
 */
ppIndex = 0;
nt32_t &appIndex)

ppIndex = 0;
nt32_t &appIndex)
#include "mock_bundle_mgr_helper.h"
ppIndex = 0;
nt32_t &appIndex)

ppIndex = 0;
nt32_t &appIndex)
namespace OHOS {
ppIndex = 0;
nt32_t &appIndex)
namespace AppExecFwk {
ppIndex = 0;
nt32_t &appIndex)

ppIndex = 0;
nt32_t &appIndex)
BundleMgrHelper::BundleMgrHelper() {}
ppIndex = 0;
nt32_t &appIndex)

ppIndex = 0;
nt32_t &appIndex)
BundleMgrHelper::~BundleMgrHelper() {}
ppIndex = 0;
nt32_t &appIndex)

ppIndex = 0;
nt32_t &appIndex)
ErrCode BundleMgrHelper::GetSandboxBundleInfo(
ppIndex = 0;
nt32_t &appIndex)
    const std::string& bundleName, int32_t appIndex, int32_t userId, BundleInfo& info)
ppIndex = 0;
nt32_t &appIndex)
{
ppIndex = 0;
nt32_t &appIndex)
    return ERR_OK;
ppIndex = 0;
nt32_t &appIndex)
}
ppIndex = 0;
nt32_t &appIndex)

ppIndex = 0;
nt32_t &appIndex)
bool BundleMgrHelper::GetBundleInfo(
ppIndex = 0;
nt32_t &appIndex)
    const std::string& bundleName, const BundleFlag flags, BundleInfo& bundleInfo, int32_t userId)
ppIndex = 0;
nt32_t &appIndex)
{
ppIndex = 0;
nt32_t &appIndex)
    AbilityInfo abilityInfo;
ppIndex = 0;
nt32_t &appIndex)
    bundleInfo.applicationInfo.accessTokenId = 1;
ppIndex = 0;
nt32_t &appIndex)
    abilityInfo.bundleName = "bundleNameTest";
ppIndex = 0;
nt32_t &appIndex)
    abilityInfo.name = "nameTest";
ppIndex = 0;
nt32_t &appIndex)
    abilityInfo.moduleName = "moduleName";
ppIndex = 0;
nt32_t &appIndex)
    bundleInfo.abilityInfos.emplace_back(abilityInfo);
ppIndex = 0;
nt32_t &appIndex)
    if ("hapModuleInfosModuleNameIsEmpty" == bundleName) {
ppIndex = 0;
nt32_t &appIndex)
        abilityInfo.bundleName = bundleName;
ppIndex = 0;
nt32_t &appIndex)
        abilityInfo.moduleName = "";
ppIndex = 0;
nt32_t &appIndex)
        HapModuleInfo hapModuleInfo;
ppIndex = 0;
nt32_t &appIndex)
        hapModuleInfo.abilityInfos.emplace_back(abilityInfo);
ppIndex = 0;
nt32_t &appIndex)
        bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo);
ppIndex = 0;
nt32_t &appIndex)
    }
ppIndex = 0;
nt32_t &appIndex)
    if ("hapAbilityInfoVisible" == bundleName) {
ppIndex = 0;
nt32_t &appIndex)
        abilityInfo.bundleName = bundleName;
ppIndex = 0;
nt32_t &appIndex)
        abilityInfo.moduleName = "moduleNameTest";
ppIndex = 0;
nt32_t &appIndex)
        abilityInfo.visible = true;
ppIndex = 0;
nt32_t &appIndex)
        HapModuleInfo hapModuleInfo;
ppIndex = 0;
nt32_t &appIndex)
        hapModuleInfo.abilityInfos.emplace_back(abilityInfo);
ppIndex = 0;
nt32_t &appIndex)
        bundleInfo.hapModuleInfos.emplace_back(hapModuleInfo);
ppIndex = 0;
nt32_t &appIndex)
    }
ppIndex = 0;
nt32_t &appIndex)
    return true;
ppIndex = 0;
nt32_t &appIndex)
}
ppIndex = 0;
nt32_t &appIndex)

ppIndex = 0;
nt32_t &appIndex)
bool BundleMgrHelper::GetApplicationInfo(
ppIndex = 0;
nt32_t &appIndex)
    const std::string& appName, int32_t flags, int32_t userId, ApplicationInfo& appInfo)
ppIndex = 0;
nt32_t &appIndex)
{
ppIndex = 0;
nt32_t &appIndex)
    return true;
ppIndex = 0;
nt32_t &appIndex)
}
ppIndex = 0;
nt32_t &appIndex)

ppIndex = 0;
nt32_t &appIndex)
std::string BundleMgrHelper::GetAppIdByBundleName(const std::string& bundleName, const int32_t userId)
ppIndex = 0;
nt32_t &appIndex)
{
ppIndex = 0;
nt32_t &appIndex)
    auto appId = bundleName + "_appId";
ppIndex = 0;
nt32_t &appIndex)
    return appId;
ppIndex = 0;
nt32_t &appIndex)
}
ppIndex = 0;
nt32_t &appIndex)

ppIndex = 0;
nt32_t &appIndex)
ErrCode BundleMgrHelper::GetCloneBundleInfo(
ppIndex = 0;
nt32_t &appIndex)
    const std::string& bundleName, int32_t flags, int32_t appCloneIndex, BundleInfo& bundleInfo, int32_t userId)
ppIndex = 0;
nt32_t &appIndex)
{
ppIndex = 0;
nt32_t &appIndex)
    return ERR_OK;
ppIndex = 0;
int32_t BundleMgrHelper::GetDualModeBundleInfo(const std::string& bundleName, int32_t userId,
nt32_t &appIndex)
}
ppIndex = 0;
nt32_t &appIndex)
} // namespace AppExecFwk
ppIndex = 0;
nt32_t &appIndex)
} // namespace OHOS
ppIndex = 0;
