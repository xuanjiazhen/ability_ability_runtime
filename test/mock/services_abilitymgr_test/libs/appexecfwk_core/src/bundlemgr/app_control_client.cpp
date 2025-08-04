/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "app_control_client.h"
#include "app_control_constants.h"
#include "app_control_proxy_result_client.h"
#include "hilog_tag_wrapper.h"
#include "mock_bundle_manager.h"

namespace OHOS {
namespace AppExecFwk {
AppControlClient &AppControlClient::GetInstance()
{
    static AppControlClient instance;
    return instance;
}

void AppControlClient::SetIsAllCaseNullptr(bool isAllCaseNullptr)
{
    isAllCaseNullptr_ = isAllCaseNullptr;
}

ErrCode AppControlClient::GetAppRunningControlRule(int32_t userId, std::vector<std::string> &appIds)
{
    if (isAllCaseNullptr_ || AppControlProxyResultClient::GetInstance().GetAppControlProxyNull()) {
        AppControlProxyResultClient::GetInstance().SetAppControlProxyNull(false);
        return ERR_APPEXECFWK_NULL_PTR;
    }
    return ERR_OK;
}

ErrCode AppControlClient::ConfirmAppJumpControlRule(const std::string &callerBundleName,
    const std::string &targetBundleName, int32_t userId)
{
    if (isAllCaseNullptr_ || AppControlProxyResultClient::GetInstance().GetAppControlProxyNull()) {
        AppControlProxyResultClient::GetInstance().SetAppControlProxyNull(false);
        return ERR_APPEXECFWK_NULL_PTR;
    }
    return ERR_OK;
}

ErrCode AppControlClient::GetAppJumpControlRule(const std::string &callerBundleName,
    const std::string &targetBundleName, int32_t userId, AppJumpControlRule &controlRule)
{
    if (isAllCaseNullptr_ || AppControlProxyResultClient::GetInstance().GetAppControlProxyNull()) {
        AppControlProxyResultClient::GetInstance().SetAppControlProxyNull(false);
        return ERR_APPEXECFWK_NULL_PTR;
    }
    return ERR_OK;
}

ErrCode AppControlClient::GetAppRunningControlRule(const std::string &bundleName, int32_t userId,
    AppRunningControlRuleResult &controlRuleResult)
{
    if (isAllCaseNullptr_ || AppControlProxyResultClient::GetInstance().GetAppControlProxyNull()) {
        AppControlProxyResultClient::GetInstance().SetAppControlProxyNull(false);
        return ERR_APPEXECFWK_NULL_PTR;
    }
    if (bundleName.compare("com.test.control2") == 0) {
        return ERR_INVALID_VALUE;
    }
    if (bundleName.compare("com.test.control3") == 0) {
        controlRuleResult.controlWant = nullptr;
        controlRuleResult.controlMessage = "the test app is not available";
        return ERR_OK;
    }
    Want want;
    ElementName element("", "ohos.global.systemres", "MainAbility");
    want.SetElement(element);
    controlRuleResult.controlWant = std::make_shared<Want>(want);
    controlRuleResult.controlMessage = "the test app is not available";
    return ERR_OK;
}

ErrCode AppControlClient::GetAbilityRunningControlRule(const std::string &bundleName, int32_t userId,
    std::vector<DisposedRule>& disposedRules, int32_t appIndex)
{
    if (isAllCaseNullptr_ || AppControlProxyResultClient::GetInstance().GetAppControlProxyNull()) {
        AppControlProxyResultClient::GetInstance().SetAppControlProxyNull(false);
        return ERR_APPEXECFWK_NULL_PTR;
    }
    if (bundleName == "com.acts.disposedrulehap") {
        DisposedRule disposedRule;
        ElementName element;

        disposedRule.priority = 0;
        disposedRule.disposedType = AppExecFwk::DisposedType::BLOCK_ABILITY;
        disposedRule.controlType = AppExecFwk::ControlType::DISALLOWED_LIST;
        element.SetAbilityName("MainAbility2");
        element.SetModuleName("entry");
        disposedRule.elementList.push_back(element);
        (*disposedRule.want).SetElementName("com.example.disposedruletest",
            "DisposedAbility2");
        disposedRules.push_back(disposedRule);
        return ERR_OK;
    }
    return ERR_INVALID_VALUE;
}
} // namespace AppExecFwk
} // namespace OHOS