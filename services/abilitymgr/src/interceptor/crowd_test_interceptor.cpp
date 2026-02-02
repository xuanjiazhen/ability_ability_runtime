/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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
#include "modal_system_ui_extension.h"
#include "start_ability_utils.h"

namespace OHOS {
namespace AAFwk {
namespace {
constexpr const char* UIEXTENSION_TYPE_KEY = "ability.want.params.uiExtensionType";
constexpr const char* UIEXTENSION_SYS_COMMON_UI = "sys/commonUI";
constexpr const char* UIEXTENSION_MODAL_TYPE = "ability.want.params.modalType";
constexpr const char* MARKET_CROWD_TEST_UIEXTENSION_ABILITY_NAME = "AppSelectorExtensionAbility";
}
ErrCode CrowdTestInterceptor::DoProcess(AbilityInterceptorParam param)
{
    if (StartAbilityUtils::skipCrowTest) {
        StartAbilityUtils::skipCrowTest = false;
        return ERR_OK;
    }
    if (CheckCrowdtest(param.want, param.userId)) {
        TAG_LOGE(AAFwkTag::ABILITYMGR, "expired");
#ifdef SUPPORT_GRAPHICS
        if (param.isWithUI) {
            std::string appGalleryBundleName;
            auto bundleMgrHelper = AbilityUtil::GetBundleManagerHelper();
            if (bundleMgrHelper == nullptr || !bundleMgrHelper->QueryAppGalleryBundleName(appGalleryBundleName)) {
                TAG_LOGE(AAFwkTag::ABILITYMGR, "Failed to query AppGallery bundle name");
                appGalleryBundleName = AbilityUtil::MARKET_BUNDLE_NAME;
            }
            auto systemUIExtension = std::make_shared<Rosen::ModalSystemUiExtension>();
            AAFwk::Want replaceWant;
            replaceWant.SetParam(UIEXTENSION_TYPE_KEY, UIEXTENSION_SYS_COMMON_UI);
            replaceWant.SetElementName(appGalleryBundleName, MARKET_CROWD_TEST_UIEXTENSION_ABILITY_NAME);
            replaceWant.SetParam(UIEXTENSION_MODAL_TYPE, 1);
            replaceWant.SetParam(AbilityUtil::MARKET_CROWD_TEST_BUNDLE_PARAM, param.want.GetBundle());
            int32_t ret = IN_PROCESS_CALL(systemUIExtension->CreateModalUIExtension(replaceWant)) ? ERR_OK : INNER_ERR;
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
    // get crowdtest status and time
    AppExecFwk::ApplicationInfo appInfo;
    if (!StartAbilityUtils::GetApplicationInfo(want.GetBundle(), userId, appInfo)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "GetAppInfo failed");
        return false;
    }

    auto appCrowdtestDeadline = appInfo.crowdtestDeadline;
    int64_t now = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::
        system_clock::now().time_since_epoch()).count();
    if (appCrowdtestDeadline > 0 && appCrowdtestDeadline < now) {
        TAG_LOGI(AAFwkTag::ABILITYMGR, "expired time: %{public}s",
            std::to_string(appCrowdtestDeadline).c_str());
        return true;
    }
    return false;
}
} // namespace AAFwk
} // namespace OHOS