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

namespace OHOS {
namespace AppExecFwk {
AppControlClient &AppControlClient::GetInstance()
{
    static AppControlClient instance;
    return instance;
}

ErrCode AppControlClient::GetAppRunningControlRule(int32_t userId, std::vector<std::string> &appIds)
{
    return ERR_APPEXECFWK_NULL_PTR;
}

ErrCode AppControlClient::ConfirmAppJumpControlRule(const std::string &callerBundleName,
    const std::string &targetBundleName, int32_t userId)
{
    return ERR_APPEXECFWK_NULL_PTR;
}

ErrCode AppControlClient::GetAppJumpControlRule(const std::string &callerBundleName,
    const std::string &targetBundleName, int32_t userId, AppJumpControlRule &controlRule)
{
    return ERR_APPEXECFWK_NULL_PTR;
}

ErrCode AppControlClient::GetAppRunningControlRule(const std::string &bundleName, int32_t userId,
    AppRunningControlRuleResult &controlRuleResult)
{
    return ERR_APPEXECFWK_NULL_PTR;
}

ErrCode AppControlClient::GetAbilityRunningControlRule(const std::string &bundleName, int32_t userId,
    std::vector<DisposedRule>& disposedRules, int32_t appIndex)
{
    return ERR_APPEXECFWK_NULL_PTR;
}
} // namespace AppExecFwk
} // namespace OHOS