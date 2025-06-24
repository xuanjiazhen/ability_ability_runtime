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

#ifndef MOCK_OHOS_ABILITY_RUNTIME__MOCK_IAPP_CONTROL_MGR_H
#define MOCK_OHOS_ABILITY_RUNTIME__MOCK_IAPP_CONTROL_MGR_H

#include <cstdint>
#include <vector>
#include <iremote_broker.h>
#include <string_ex.h>
#include "app_jump_control_rule.h"
#include "app_running_control_rule.h"
#include "app_running_control_rule_result.h"
#include "disposed_rule.h"
#include "iapp_control_mgr_types.h"
#include "want.h"

using OHOS::AAFwk::Want;
using OHOS::AppExecFwk::AppJumpControlRule;
using OHOS::AppExecFwk::AppRunningControlRule;
using OHOS::AppExecFwk::AppRunningControlRuleResult;
using OHOS::AppExecFwk::DisposedRule;
using OHOS::AppExecFwk::UninstallDisposedRule;
using OHOS::AppExecFwk::DisposedRuleConfiguration;

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::AppExecFwk;

enum class IAppControlMgrIpcCode {
    COMMAND_ADD_APP_INSTALL_CONTROL_RULE = 0,
    COMMAND_DELETE_APP_INSTALL_CONTROL_RULE = 1,
    COMMAND_DELETE_APP_INSTALL_CONTROL_RULE_IN_APPINSTALLCONTROLRULETYPE_IN_INT_OUT_INT = 2,
    COMMAND_GET_APP_INSTALL_CONTROL_RULE = 3,
    COMMAND_ADD_APP_RUNNING_CONTROL_RULE = 4,
    COMMAND_DELETE_APP_RUNNING_CONTROL_RULE = 5,
    COMMAND_DELETE_APP_RUNNING_CONTROL_RULE_IN_INT_OUT_INT = 6,
    COMMAND_GET_APP_RUNNING_CONTROL_RULE = 7,
    COMMAND_GET_APP_RUNNING_CONTROL_RULE_IN_STRING_IN_INT_OUT_APPRUNNINGCONTROLRULERESULT_OUT_INT = 8,
    COMMAND_SET_DISPOSED_STATUS = 9,
    COMMAND_DELETE_DISPOSED_STATUS = 10,
    COMMAND_GET_DISPOSED_STATUS = 11,
    COMMAND_CONFIRM_APP_JUMP_CONTROL_RULE = 12,
    COMMAND_ADD_APP_JUMP_CONTROL_RULE = 13,
    COMMAND_DELETE_APP_JUMP_CONTROL_RULE = 14,
    COMMAND_DELETE_RULE_BY_CALLER_BUNDLE_NAME = 15,
    COMMAND_DELETE_RULE_BY_TARGET_BUNDLE_NAME = 16,
    COMMAND_GET_APP_JUMP_CONTROL_RULE = 17,
    COMMAND_GET_DISPOSED_RULE = 18,
    COMMAND_SET_DISPOSED_RULE = 19,
    COMMAND_GET_ABILITY_RUNNING_CONTROL_RULE = 20,
    COMMAND_GET_DISPOSED_RULE_FOR_CLONE_APP = 21,
    COMMAND_SET_DISPOSED_RULE_FOR_CLONE_APP = 22,
    COMMAND_DELETE_DISPOSED_RULE_FOR_CLONE_APP = 23,
    COMMAND_GET_UNINSTALL_DISPOSED_RULE = 24,
    COMMAND_SET_UNINSTALL_DISPOSED_RULE = 25,
    COMMAND_DELETE_UNINSTALL_DISPOSED_RULE = 26,
};

class MockIAppControlMgr : public IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"OHOS.AppExecFwk.IAppControlMgr");

    virtual ErrCode AddAppInstallControlRule(
        const std::vector<std::string>& appIds,
        AppInstallControlRuleType controlRuleType,
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode DeleteAppInstallControlRule(
        AppInstallControlRuleType controlRuleType,
        const std::vector<std::string>& appIds,
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode DeleteAppInstallControlRule(
        AppInstallControlRuleType controlRuleType,
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode GetAppInstallControlRule(
        AppInstallControlRuleType controlRuleType,
        int32_t userId,
        std::vector<std::string>& appIds,
        int32_t& funcResult) = 0;

    virtual ErrCode AddAppRunningControlRule(
        const std::vector<AppRunningControlRule>& controlRules,
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode DeleteAppRunningControlRule(
        const std::vector<AppRunningControlRule>& controlRules,
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode DeleteAppRunningControlRule(
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode GetAppRunningControlRule(
        int32_t userId,
        std::vector<std::string>& appIds,
        int32_t& funcResult) = 0;

    virtual ErrCode GetAppRunningControlRule(
        const std::string& bundleName,
        int32_t userId,
        AppRunningControlRuleResult& controlRuleResult,
        int32_t& funcResult) = 0;

    virtual ErrCode SetDisposedStatus(
        const std::string& appId,
        const Want& want,
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode DeleteDisposedStatus(
        const std::string& appId,
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode GetDisposedStatus(
        const std::string& appId,
        int32_t userId,
        Want& want,
        int32_t& funcResult) = 0;

    virtual ErrCode ConfirmAppJumpControlRule(
        const std::string& callerBundleName,
        const std::string& targetBundleName,
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode AddAppJumpControlRule(
        const std::vector<AppJumpControlRule>& controlRules,
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode DeleteAppJumpControlRule(
        const std::vector<AppJumpControlRule>& controlRules,
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode DeleteRuleByCallerBundleName(
        const std::string& callerBundleName,
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode DeleteRuleByTargetBundleName(
        const std::string& targetBundleName,
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode GetAppJumpControlRule(
        const std::string& callerBundleName,
        const std::string& targetBundleName,
        int32_t userId,
        AppJumpControlRule& controlRule,
        int32_t& funcResult) = 0;

    virtual ErrCode GetDisposedRule(
        const std::string& appId,
        int32_t userId,
        DisposedRule& disposedRule,
        int32_t& funcResult) = 0;

    virtual ErrCode SetDisposedRule(
        const std::string& appId,
        int32_t userId,
        DisposedRule& disposedRule,
        int32_t& funcResult) = 0;

    virtual ErrCode GetAbilityRunningControlRule(
        const std::string& bundleName,
        int32_t userId,
        int32_t appIndex,
        std::vector<DisposedRule>& disposedRules,
        int32_t& funcResult) = 0;

    virtual ErrCode GetDisposedRuleForCloneApp(
        const std::string& appId,
        int32_t appIndex,
        int32_t userId,
        DisposedRule& disposedRule,
        int32_t& funcResult) = 0;

    virtual ErrCode SetDisposedRuleForCloneApp(
        const std::string& appId,
        int32_t appIndex,
        int32_t userId,
        DisposedRule& disposedRule,
        int32_t& funcResult) = 0;

    virtual ErrCode DeleteDisposedRuleForCloneApp(
        const std::string& appId,
        int32_t appIndex,
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode GetUninstallDisposedRule(
        const std::string& appIdentifier,
        int32_t appIndex,
        int32_t userId,
        UninstallDisposedRule& rule,
        int32_t& funcResult) = 0;

    virtual ErrCode SetUninstallDisposedRule(
        const std::string& appIdentifier,
        const UninstallDisposedRule& rule,
        int32_t appIndex,
        int32_t userId,
        int32_t& funcResult) = 0;

    virtual ErrCode DeleteUninstallDisposedRule(
        const std::string& appIdentifier,
        int32_t appIndex,
        int32_t userId,
        int32_t& funcResult) = 0;

protected:
    const int VECTOR_MAX_SIZE = 102400;
    const int LIST_MAX_SIZE = 102400;
    const int SET_MAX_SIZE = 102400;
    const int MAP_MAX_SIZE = 102400;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // MOCK_OHOS_ABILITY_RUNTIME__MOCK_IAPP_CONTROL_MGR_H