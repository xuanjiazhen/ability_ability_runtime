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

#include "skill_execute_manager.h"

#include "ability_manager_errors.h"
#include "bundle_mgr_helper.h"
#include "hilog_tag_wrapper.h"
#include "iservice_registry.h"
#include "permission_verification.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AAFwk {

SkillExecuteManager::SkillExecuteManager() {}

SkillExecuteManager::~SkillExecuteManager() {}

sptr<AppExecFwk::IBundleSkillManager> SkillExecuteManager::GetSkillManagerProxy()
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get SystemAbilityManager");
        return nullptr;
    }
    auto bmsObj = samgr->GetSystemAbility(BUNDLE_MGR_SERVICE_SYS_ABILITY_ID);
    if (bmsObj == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get BMS from samgr");
        return nullptr;
    }
    auto bundleMgr = iface_cast<AppExecFwk::IBundleMgr>(bmsObj);
    if (bundleMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to cast IBundleMgr");
        return nullptr;
    }
    return bundleMgr->GetSkillManagerProxy();
}

int32_t SkillExecuteManager::QuerySkillInfo(const std::string &bundleName, const std::string &moduleName,
    const std::string &skillName, int32_t userId, AppExecFwk::SkillInfo &skillInfo)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "query skill info, bundle:%{public}s module:%{public}s skill:%{public}s",
        bundleName.c_str(), moduleName.c_str(), skillName.c_str());

    auto skillMgr = GetSkillManagerProxy();
    if (skillMgr == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get SkillManager proxy");
        return ERR_INVALID_VALUE;
    }

    uint32_t flags = static_cast<uint32_t>(AppExecFwk::SkillInfoFlag::GET_SKILL_INFO_WITH_SRC_ENTRIES) |
        static_cast<uint32_t>(AppExecFwk::SkillInfoFlag::GET_SKILL_INFO_WITH_PERMISSIONS) |
        static_cast<uint32_t>(AppExecFwk::SkillInfoFlag::GET_SKILL_INFO_WITH_REQUEST_PERMISSIONS);

    auto ret = skillMgr->GetSkillInfo(bundleName, moduleName, skillName, flags, userId, skillInfo);
    if (ret != ERR_OK) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetSkillInfo failed, ret:%{public}d", ret);
        return ret;
    }
    return ERR_OK;
}

int32_t SkillExecuteManager::CheckSkillPermission(const AppExecFwk::SkillInfo &skillInfo)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "check skill permission for skill:%{public}s",
        skillInfo.skillName.c_str());

    auto permVerif = PermissionVerification::GetInstance();
    if (!permVerif->JudgeCallerIsAllowedToUseSystemAPI()) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "caller not allowed to use system API");
        return ERR_NOT_SYSTEM_APP;
    }

    for (const auto &perm : skillInfo.permissions) {
        if (!permVerif->VerifyCallingPermission(perm)) {
            TAG_LOGE(AAFwkTag::ABILITYMGR, "caller lacks permission:%{public}s", perm.c_str());
            return CHECK_PERMISSION_FAILED;
        }
    }
    return ERR_OK;
}

int32_t SkillExecuteManager::GenerateSkillWant(const AppExecFwk::SkillInfo &skillInfo, Want &want,
    int32_t userId, const std::string &arkTSPath, const std::string &funcName,
    const std::vector<std::string> &argv)
{
    std::string abilityName = skillInfo.abilityName;
    if (abilityName.empty()) {
        abilityName = ResolveDefaultAbilityName(skillInfo.bundleName, skillInfo.moduleName, userId);
        if (abilityName.empty()) {
            TAG_LOGE(AAFwkTag::ABILITYMGR,
                "no abilityName in skillProfile and no default ability found for bundle:%{public}s module:%{public}s",
                skillInfo.bundleName.c_str(), skillInfo.moduleName.c_str());
            return ERR_INVALID_VALUE;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "abilityName not specified, resolved default ability:%{public}s", abilityName.c_str());
    }

    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "generate skill want, bundle:%{public}s module:%{public}s skill:%{public}s ability:%{public}s",
        skillInfo.bundleName.c_str(), skillInfo.moduleName.c_str(),
        skillInfo.skillName.c_str(), abilityName.c_str());

    want.SetElementName("", skillInfo.bundleName, abilityName, skillInfo.moduleName);
    AppExecFwk::SkillExecuteParam::WriteToWant(want, skillInfo.bundleName, skillInfo.moduleName,
        skillInfo.skillName, arkTSPath, funcName, argv, skillInfo.srcEntries);
    return ERR_OK;
}

int32_t SkillExecuteManager::ExecuteSkillDone(const sptr<IRemoteObject> &token, int32_t resultCode,
    const AppExecFwk::SkillExecuteResult &result)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "execute skill done, code:%{public}d", resultCode);
    std::lock_guard<ffrt::mutex> lock(mutex_);
    // Currently a placeholder for result handling.
    // The full callback-to-caller mechanism will be implemented
    // when the SkillExecuteRecord tracking is added.
    TAG_LOGI(AAFwkTag::ABILITYMGR, "execute skill done processed, code:%{public}d", resultCode);
    return ERR_OK;
}

std::string SkillExecuteManager::ResolveDefaultAbilityName(const std::string &bundleName,
    const std::string &moduleName, int32_t userId)
{
    auto bundleMgrHelper = DelayedSingleton<AppExecFwk::BundleMgrHelper>::GetInstance();
    if (bundleMgrHelper == nullptr) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "failed to get BundleMgrHelper");
        return "";
    }

    AppExecFwk::BundleInfo bundleInfo;
    auto flags = static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_HAP_MODULE) |
        static_cast<int32_t>(AppExecFwk::GetBundleInfoFlag::GET_BUNDLE_INFO_WITH_ABILITY);
    bool ret = bundleMgrHelper->GetBundleInfoV9(bundleName, flags, bundleInfo, userId);
    if (!ret) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "GetBundleInfoV9 failed for bundle:%{public}s",
            bundleName.c_str());
        return "";
    }

    for (const auto &hapModuleInfo : bundleInfo.hapModuleInfos) {
        if (hapModuleInfo.moduleName != moduleName) {
            continue;
        }
        std::string mainElement;
        if (hapModuleInfo.isModuleJson) {
            mainElement = hapModuleInfo.mainElementName;
        } else {
            mainElement = hapModuleInfo.mainAbility;
        }
        TAG_LOGI(AAFwkTag::ABILITYMGR,
            "resolved default ability for module:%{public}s, mainElement:%{public}s",
            moduleName.c_str(), mainElement.c_str());
        return mainElement;
    }

    TAG_LOGE(AAFwkTag::ABILITYMGR, "module not found:%{public}s in bundle:%{public}s",
        moduleName.c_str(), bundleName.c_str());
    return "";
}

} // namespace AAFwk
} // namespace OHOS
