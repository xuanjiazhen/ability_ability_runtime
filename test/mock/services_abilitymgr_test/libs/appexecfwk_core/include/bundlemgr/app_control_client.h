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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_APP_CONTROL_CLIENT_H
#define MOCK_OHOS_ABILITY_RUNTIME_APP_CONTROL_CLIENT_H

#include <mutex>
#include <string>
#include <vector>

#include "appexecfwk_errors.h"
#include "bundle_constants.h"
#include "iapp_control_mgr.h"

namespace OHOS {
namespace AppExecFwk {
namespace {
const int32_t UNSPECIFIED_USER_ID = -2;
}
class AppControlClient {
public:
    static AppControlClient &GetInstance();

    ErrCode GetAppRunningControlRule(int32_t userId, std::vector<std::string> &appIds);
    ErrCode GetAppRunningControlRule(const std::string &bundleName, int32_t userId,
        AppRunningControlRuleResult &controlRuleResult);
    ErrCode ConfirmAppJumpControlRule(const std::string &callerBundleName, const std::string &targetBundleName,
        int32_t userId);
    ErrCode GetAppJumpControlRule(const std::string &callerBundleName, const std::string &targetBundleName,
        int32_t userId, AppJumpControlRule &controlRule);
    ErrCode GetAbilityRunningControlRule(const std::string &bundleName, int32_t userId,
        std::vector<DisposedRule> &disposedRules, int32_t appIndex = Constants::MAIN_APP_INDEX);
    void SetIsAllCaseNullptr(bool isAllCaseNullptr);

private:
    AppControlClient() = default;
    ~AppControlClient() = default;
    DISALLOW_COPY_AND_MOVE(AppControlClient);
    bool isAllCaseNullptr_ = false;
};
} // namespace AppExecFwk
} // namespace OHOS
#endif // MOCK_OHOS_ABILITY_RUNTIME_APP_CONTROL_CLIENT_H