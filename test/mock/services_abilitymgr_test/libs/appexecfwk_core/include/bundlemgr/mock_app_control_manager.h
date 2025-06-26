/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_CONTROL_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_CONTROL_MANAGER_H

#include <gmock/gmock.h>

#include "iremote_proxy.h"
#include "mock_iapp_control_mgr.h"
#include "want.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t UNSPECIFIED_USER_ID = -2;
}
class AppControlProxy : public IRemoteProxy<MockIAppControlMgr> {
public:
    using Want = OHOS::AAFwk::Want;

    explicit AppControlProxy(const sptr<IRemoteObject>& object);
    virtual ~AppControlProxy();

    // for app install control rule
    ErrCode AddAppInstallControlRule(const std::vector<std::string> &appIds,
        const AppInstallControlRuleType controlRuleType, int32_t userId, int32_t &funcResult) override;

    ErrCode DeleteAppInstallControlRule(const AppInstallControlRuleType controlRuleType,
        const std::vector<std::string> &appIds, int32_t userId, int32_t &funcResult) override;

    ErrCode DeleteAppInstallControlRule(
        const AppInstallControlRuleType controlRuleType, int32_t userId, int32_t &funcResult) override;

    ErrCode GetAppInstallControlRule(const AppInstallControlRuleType controlRuleType, int32_t userId,
        std::vector<std::string> &appIds, int32_t &funcResult) override;

    // for app running control rule
    ErrCode AddAppRunningControlRule(
        const std::vector<AppRunningControlRule> &controlRules, int32_t userId, int32_t &funcResult) override;
    ErrCode DeleteAppRunningControlRule(
        const std::vector<AppRunningControlRule> &controlRules, int32_t userId, int32_t &funcResult) override;
    ErrCode DeleteAppRunningControlRule(int32_t userId, int32_t &funcResult) override;
    ErrCode GetAppRunningControlRule(int32_t userId, std::vector<std::string> &appIds, int32_t &funcResult) override;
    ErrCode GetAppRunningControlRule(const std::string &bundleName, int32_t userId,
        AppRunningControlRuleResult &controlRule, int32_t &funcResult) override;

    // for app jump control rule
    ErrCode ConfirmAppJumpControlRule(const std::string &callerBundleName, const std::string &targetBundleName,
        int32_t userId, int32_t &funcResult) override;
    ErrCode AddAppJumpControlRule(const std::vector<AppJumpControlRule> &controlRules, int32_t userId,
        int32_t &funcResult) override;
    ErrCode DeleteAppJumpControlRule(const std::vector<AppJumpControlRule> &controlRules,
        int32_t userId, int32_t &funcResult) override;
    ErrCode DeleteRuleByCallerBundleName(const std::string &callerBundleName, int32_t userId,
        int32_t &funcResult) override;
    ErrCode DeleteRuleByTargetBundleName(const std::string &targetBundleName, int32_t userId,
        int32_t &funcResult) override;
    ErrCode GetAppJumpControlRule(const std::string &callerBundleName, const std::string &targetBundleName,
        int32_t userId, AppJumpControlRule &controlRule, int32_t &funcResult) override;

    ErrCode SetDisposedStatus(
        const std::string &appId, const Want &want, int32_t userId, int32_t &funcResult) override;

    ErrCode DeleteDisposedStatus(
        const std::string &appId, int32_t userId, int32_t &funcResult) override;

    ErrCode GetDisposedStatus(
        const std::string &appId, int32_t userId, Want &want, int32_t &funcResult) override;

    ErrCode SetDisposedRule(
        const std::string &appId, int32_t userId, DisposedRule &DisposedRule, int32_t &funcResult) override;

    ErrCode GetDisposedRule(
        const std::string &appId, int32_t userId, DisposedRule &DisposedRule, int32_t &funcResult) override;

    ErrCode GetAbilityRunningControlRule(const std::string &bundleName, int32_t userId,
        int32_t appIndex, std::vector<DisposedRule>& disposedRules, int32_t &funcResult) override;
    
    ErrCode SetDisposedRuleForCloneApp(const std::string &appId, int32_t appIndex, int32_t userId,
        DisposedRule &DisposedRule, int32_t &funcResult) override;

    ErrCode GetDisposedRuleForCloneApp(const std::string &appId, int32_t appIndex, int32_t userId,
        DisposedRule &DisposedRule, int32_t &funcResult) override;

    ErrCode DeleteDisposedRuleForCloneApp(
        const std::string &appId, int32_t appIndex, int32_t userId, int32_t &funcResult) override;
    
    ErrCode GetUninstallDisposedRule(const std::string &appIdentifier, int32_t appIndex,
        int32_t userId, UninstallDisposedRule &rule, int32_t &funcResult) override;
    
    ErrCode SetUninstallDisposedRule(const std::string &appIdentifier, const UninstallDisposedRule &rule,
        int32_t appIndex, int32_t userId, int32_t &funcResult) override;
    
    ErrCode DeleteUninstallDisposedRule(const std::string &appIdentifier, int32_t appIndex,
        int32_t userId, int32_t &funcResult) override;
};
}  // namespace AppExecFwk
}  // namespace OHOS

#endif  // MOCK_OHOS_ABILITY_RUNTIME_MOCK_APP_CONTROL_MANAGER_H