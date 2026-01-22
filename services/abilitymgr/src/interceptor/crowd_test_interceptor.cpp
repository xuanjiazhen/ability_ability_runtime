/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "interceptor/crowd_test_interceptor.h"

#include "ability_util.h"
#include "start_ability_utils.h"
#include "dialog_session_manager.h"
// #include "ability_request.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* ACTION_MARKET_CROWDTEST = "ohos.want.action.marketCrowdTest";
constexpr const char* MARKET_BUNDLE_NAME = "com.huawei.hmsapp.appgallery";
const std::string UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
const std::string UIEXTENSION_SYS_COMMON_UI = "sys/commonUI";
}
ErrCode CrowdTestInterceptor::DoProcess(AbilityInterceptorParam param)
{
    std::string bundleName = param.want.GetElement().GetBundleName();
    std::string abilityName = param.want.GetElement().GetAbilityName();
    int32_t userId = param.userId;
    TAG_LOGI(AAFwkTag::ABILITYMGR,
        "hanksh DoProcess bundleName: %{public}s, abilityName: %{public}s, userId: %{public}d", bundleName.c_str(),
        abilityName.c_str(), userId);
    if (StartAbilityUtils::skipCrowTest) {
        StartAbilityUtils::skipCrowTest = false;
        return ERR_OK;
    }
    if (CheckCrowdtest(param.want, param.userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "expired");
#ifdef SUPPORT_GRAPHICS
        if (param.isWithUI) {
            if (param.callerToken == nullptr) {
                TAG_LOGI(AAFwkTag::ABILITYMGR,
                    "bundleName: %{public}s, abilityName: %{public}s, callerToken is nullptr", bundleName.c_str(),
                    abilityName.c_str());
            }
            if (param.abilityInfo == nullptr) {
                TAG_LOGI(AAFwkTag::ABILITYMGR,
                    "bundleName: %{public}s, abilityName: %{public}s, abilityInfo is nullptr", bundleName.c_str(),
                    abilityName.c_str());
            }
            AppExecFwk::AbilityRequest abilityRequest;

            // std::string appGalleryBundleName;
            // auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
            // if (bundleMgrHelper == nullptr || 
            //     !bundleMgrHelper->QueryAppGalleryBundleName(appGalleryBundleName)) {
            //     TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to query AppGallery bundle name");
            //     appGalleryBundleName = MARKET_BUNDLE_NAME;
            // }
            // AAFwk::Want replaceWant;
            // replaceWant.SetParam(UIEXTENSION_TYPE_KEY, UIEXTENSION_SYS_COMMON_UI);
            // replaceWant.SetElementName(appGalleryBundleName, "UIExtAbility");
            // int32_t ret = DialogSessionManager::GetInstance().HandleExpiredCrowdTestApp(abilityRequest, param.userId, replaceWant);
            // if (ret != ERR_OK) {
            //     TAG_LOGE(AAFwkTag::ABILITYMGR, "start appGallery failed:%{public}d", ret);
            //     return ret;
            // }

            AAFwk::Want replaceWant;
            replaceWant.SetParam("ability.want.params.uiExtensionType", std::string("sys/commonUI"));
            replaceWant.SetElementName("com.example.hankshuiextension", "UIExtAbility");

            int32_t ret = (DialogSessionManager::GetInstance().HandleExpiredCrowdTestApp(
                abilityRequest, param.userId, replaceWant);
            if (ret != ERR_OK) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "start appGallery failed:%{public}d", ret);
                return ret;
            }
        }
#endif
        return ERR_CROWDTEST_EXPIRED;
    }
    return ERR_OK;
}

bool CrowdTestInterceptor::CheckCrowdtest(const Want &want, int32_t userId)
{
    TAG_LOGI(AAFwkTag::ABILITYMGR, "hanksh CheckCrowdtest start");
    // get crowdtest status and time
    AppExecFwk::ApplicationInfo appInfo;
    if (!StartAbilityUtils::GetApplicationInfo(want.GetBundle(), userId, appInfo)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "GetAppInfo failed");
        return false;
    }

    auto appCrowdtestDeadline = appInfo.crowdtestDeadline;
    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    std::string testName = want.GetBundle();
    TAG_LOGI(AAFwkTag::ABILITYMGR, "hanksh DoProcess bundleName: %{public}s",testName.c_str());
    if ((appCrowdtestDeadline > 0 && appCrowdtestDeadline < now) || testName == "com.example.testnamehantwo") {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "expired time: %{public}s",
            std::to_string(appCrowdtestDeadline).c_str());
        return true;
    }
    TAG_LOGI(AAFwkTag::ABILITYMGR, "hanksh CheckCrowdtest end");
    return false;
}
} // namespace AAFwk
} // namespace OHOS