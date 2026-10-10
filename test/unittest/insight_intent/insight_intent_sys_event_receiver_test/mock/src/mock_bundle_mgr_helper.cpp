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
#include "bundle_mgr_helper.h"
ppIndex = 0;
nt32_t &appIndex)

ppIndex = 0;
nt32_t &appIndex)
#include "bundle_mgr_service_death_recipient.h"
ppIndex = 0;
nt32_t &appIndex)
#include "global_constant.h"
ppIndex = 0;
nt32_t &appIndex)
#include "hilog_tag_wrapper.h"
ppIndex = 0;
nt32_t &appIndex)
#include "hitrace_meter.h"
ppIndex = 0;
nt32_t &appIndex)
#include "iservice_registry.h"
ppIndex = 0;
nt32_t &appIndex)
#include "system_ability_definition.h"
ppIndex = 0;
nt32_t &appIndex)
#include "app_utils.h"
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
namespace {
ppIndex = 0;
nt32_t &appIndex)
    const std::string profileJsonStr = "{"
ppIndex = 0;
nt32_t &appIndex)
        "\"insightIntents\":["
ppIndex = 0;
nt32_t &appIndex)
        "{"
ppIndex = 0;
nt32_t &appIndex)
            "\"intentName\":\"test1\","
ppIndex = 0;
nt32_t &appIndex)
            "\"domain\":\"domain1\","
ppIndex = 0;
nt32_t &appIndex)
            "\"intentVersion\":\"1.0\","
ppIndex = 0;
nt32_t &appIndex)
            "\"srcEntry\":\"entry1\","
ppIndex = 0;
nt32_t &appIndex)
            "\"uiAbility\":{"
ppIndex = 0;
nt32_t &appIndex)
                "\"ability\":\"ability1\","
ppIndex = 0;
nt32_t &appIndex)
                "\"executeMode\":[\"foreground\"]"
ppIndex = 0;
nt32_t &appIndex)
            "},"
ppIndex = 0;
nt32_t &appIndex)
            "\"uiExtension\":{"
ppIndex = 0;
nt32_t &appIndex)
                "\"ability\":\"ability1\""
ppIndex = 0;
nt32_t &appIndex)
            "},"
ppIndex = 0;
nt32_t &appIndex)
            "\"serviceExtension\":{"
ppIndex = 0;
nt32_t &appIndex)
                "\"ability\":\"ability1\""
ppIndex = 0;
nt32_t &appIndex)
            "},"
ppIndex = 0;
nt32_t &appIndex)
            "\"form\":{"
ppIndex = 0;
nt32_t &appIndex)
                "\"ability\":\"ability1\","
ppIndex = 0;
nt32_t &appIndex)
                "\"formName\":\"form1\""
ppIndex = 0;
nt32_t &appIndex)
            "}"
ppIndex = 0;
nt32_t &appIndex)
        "},"
ppIndex = 0;
nt32_t &appIndex)
        "{"
ppIndex = 0;
nt32_t &appIndex)
            "\"intentName\":\"test2\","
ppIndex = 0;
nt32_t &appIndex)
            "\"domain\":\"domain1\","
ppIndex = 0;
nt32_t &appIndex)
            "\"intentVersion\":\"1.0\","
ppIndex = 0;
nt32_t &appIndex)
            "\"srcEntry\":\"entry1\","
ppIndex = 0;
nt32_t &appIndex)
            "\"uiAbility\":{"
ppIndex = 0;
nt32_t &appIndex)
                "\"ability\":\"ability1\","
ppIndex = 0;
nt32_t &appIndex)
                "\"executeMode\":[\"foreground\"]"
ppIndex = 0;
nt32_t &appIndex)
            "},"
ppIndex = 0;
nt32_t &appIndex)
            "\"uiExtension\":{"
ppIndex = 0;
nt32_t &appIndex)
                "\"ability\":\"ability1\""
ppIndex = 0;
nt32_t &appIndex)
            "},"
ppIndex = 0;
nt32_t &appIndex)
            "\"serviceExtension\":{"
ppIndex = 0;
nt32_t &appIndex)
                "\"ability\":\"ability1\""
ppIndex = 0;
nt32_t &appIndex)
            "},"
ppIndex = 0;
nt32_t &appIndex)
            "\"form\": {"
ppIndex = 0;
nt32_t &appIndex)
                "\"ability\":\"ability1\","
ppIndex = 0;
nt32_t &appIndex)
                "\"formName\":\"form1\""
ppIndex = 0;
nt32_t &appIndex)
            "}"
ppIndex = 0;
nt32_t &appIndex)
        "}"
ppIndex = 0;
nt32_t &appIndex)
        "]"
ppIndex = 0;
nt32_t &appIndex)
    "}";
ppIndex = 0;
nt32_t &appIndex)
}
ppIndex = 0;
nt32_t &appIndex)
BundleMgrHelper::BundleMgrHelper() {}
ppIndex = 0;
nt32_t &appIndex)

ppIndex = 0;
nt32_t &appIndex)
BundleMgrHelper::~BundleMgrHelper()
ppIndex = 0;
nt32_t &appIndex)
{
ppIndex = 0;
nt32_t &appIndex)
    if (bundleMgr_ != nullptr && bundleMgr_->AsObject() != nullptr && deathRecipient_ != nullptr) {
ppIndex = 0;
nt32_t &appIndex)
        bundleMgr_->AsObject()->RemoveDeathRecipient(deathRecipient_);
ppIndex = 0;
nt32_t &appIndex)
    }
ppIndex = 0;
nt32_t &appIndex)
}
ppIndex = 0;
nt32_t &appIndex)

ppIndex = 0;
nt32_t &appIndex)
ErrCode BundleMgrHelper::GetJsonProfile(ProfileType profileType, const std::string &bundleName,
ppIndex = 0;
nt32_t &appIndex)
    const std::string &moduleName, std::string &profile, int32_t userId)
ppIndex = 0;
nt32_t &appIndex)
{
ppIndex = 0;
nt32_t &appIndex)
    profile = profileJsonStr;
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
std::string BundleMgrHelper::GetStringById(
ppIndex = 0;
nt32_t &appIndex)
    const std::string &bundleName, const std::string &moduleName, uint32_t resId, int32_t userId)
ppIndex = 0;
nt32_t &appIndex)
{
ppIndex = 0;
nt32_t &appIndex)
    return "";
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
    const std::string &bundleName, const BundleFlag flag, BundleInfo &bundleInfo, int32_t userId)
ppIndex = 0;
nt32_t &appIndex)
{
ppIndex = 0;
nt32_t &appIndex)
    bundleInfo.applicationInfo.bundleType = AppExecFwk::BundleType::ATOMIC_SERVICE;
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
bool BundleMgrHelper::GetBundleInfo(
ppIndex = 0;
nt32_t &appIndex)
    const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId)
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
ErrCode BundleMgrHelper::GetBundleInfoV9(
ppIndex = 0;
nt32_t &appIndex)
    const std::string &bundleName, int32_t flags, BundleInfo &bundleInfo, int32_t userId)
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
ErrCode BundleMgrHelper::GetBundleInfosV9(int32_t flags,
ppIndex = 0;
nt32_t &appIndex)
    std::vector<BundleInfo> &bundleInfos, int32_t userId)
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
ErrCode BundleMgrHelper::GetLauncherAbilityInfoSync(
ppIndex = 0;
nt32_t &appIndex)
    const std::string &bundleName, int32_t userId, std::vector<AbilityInfo> &abilityInfo)
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
ErrCode BundleMgrHelper::GetCloneBundleInfo(const std::string &bundleName, int32_t flags, int32_t appCloneIndex,
ppIndex = 0;
nt32_t &appIndex)
    BundleInfo &bundleInfo, int32_t userId)
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
ErrCode BundleMgrHelper::GetSandboxBundleInfo(
ppIndex = 0;
nt32_t &appIndex)
    const std::string &bundleName, int32_t appIndex, int32_t userId, BundleInfo &info)
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
sptr<IAppControlMgr> BundleMgrHelper::GetAppControlProxy()
ppIndex = 0;
nt32_t &appIndex)
{
ppIndex = 0;
nt32_t &appIndex)
    return nullptr;
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
    const std::string &appName, const ApplicationFlag flag, const int32_t userId, ApplicationInfo &appInfo)
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
bool BundleMgrHelper::GetApplicationInfo(
ppIndex = 0;
nt32_t &appIndex)
    const std::string &appName, int32_t flags, int32_t userId, ApplicationInfo &appInfo)
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
ErrCode BundleMgrHelper::GetNameForUid(const int32_t uid, std::string &name)
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
bool BundleMgrHelper::QueryAppGalleryBundleName(std::string &bundleName)
ppIndex = 0;
nt32_t &appIndex)
{
ppIndex = 0;
nt32_t &appIndex)
    return true;
ppIndex = 0;
int32_t BundleMgrHelper::GetDualModeBundleInfo(const std::string& bundleName, int32_t userId,
nt32_t &appIndex)
}
ppIndex = 0;
nt32_t &appIndex)
}  // namespace AppExecFwk
ppIndex = 0;
nt32_t &appIndex)
}  // namespace OHOS
ppIndex = 0;
