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

#include "system_recovery_mode_controller.h"

#include "ability_manager_client.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "parameters.h"
#include "system_recovery_mode_constants.h"
#include "system_recovery_mode_policy.h"
#ifdef APP_MGR_SERVICE_APPMS
#include "net_conn_client.h"
#endif

namespace OHOS {
namespace AppExecFwk {
namespace {

constexpr char AIRPLANE_RESTORE_VALID_PARAMETER[] =
    "persist.sys.abilityms.system_recovery.airplane_restore_valid";
constexpr char AIRPLANE_RESTORE_VALUE_PARAMETER[] =
    "persist.sys.abilityms.system_recovery.airplane_restore_value";
constexpr char AIRPLANE_INITIALIZED_PARAMETER[] =
    "persist.sys.abilityms.system_recovery.airplane_initialized";
constexpr uint32_t MAX_RETRY_TIMES = 30;
constexpr int64_t RETRY_DELAY_MILLISECONDS = 2000;

} // namespace

SystemRecoveryModeController &SystemRecoveryModeController::GetInstance()
{
    static SystemRecoveryModeController controller;
    return controller;
}

void SystemRecoveryModeController::Start(const std::shared_ptr<AAFwk::TaskHandlerWrap> &taskHandler)
{
    bool expected = false;
    if (!started_.compare_exchange_strong(expected, true)) {
        return;
    }
    taskHandler_ = taskHandler;
    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "start recovery controller without task handler");
        started_.store(false);
        return;
    }

    const bool enabled = system::GetParameter(AAFwk::SYSTEM_RECOVERY_MODE_PARAMETER, "") ==
        AAFwk::SYSTEM_RECOVERY_MODE_VALUE;
    // 恢复模式只能通过完整重启进入或退出；进程启动后不再监听参数变化。
    AAFwk::SystemRecoveryModePolicy::GetInstance().SetEnabled(enabled);
    ScheduleApply(enabled, 0, 0);
}

void SystemRecoveryModeController::ScheduleApply(bool enabled, uint32_t retry, int64_t delayMillis)
{
    if (taskHandler_ == nullptr) {
        return;
    }
    taskHandler_->SubmitTaskJust([this, enabled, retry]() {
        const bool success = enabled ? ApplyRecoveryMode() : ApplyNormalMode();
        if (!success && retry < MAX_RETRY_TIMES) {
            ScheduleApply(enabled, retry + 1, RETRY_DELAY_MILLISECONDS);
        }
    }, "ApplySystemRecoveryMode", delayMillis);
}

bool SystemRecoveryModeController::ApplyRecoveryMode()
{
    const int32_t abilityRet = AAFwk::AbilityManagerClient::GetInstance()->BlockAllAppStart(true);
    const bool airplaneRet = InitializeAirplaneMode();
    TAG_LOGI(AAFwkTag::APPMGR,
        "apply recovery mode, abilityRet=%{public}d, airplaneRet=%{public}d", abilityRet, airplaneRet);
    return abilityRet == ERR_OK && airplaneRet;
}

bool SystemRecoveryModeController::ApplyNormalMode()
{
    const int32_t abilityRet = AAFwk::AbilityManagerClient::GetInstance()->BlockAllAppStart(false);
    const bool airplaneRet = RestoreAirplaneMode();
    TAG_LOGI(AAFwkTag::APPMGR,
        "restore normal mode, abilityRet=%{public}d, airplaneRet=%{public}d", abilityRet, airplaneRet);
    return abilityRet == ERR_OK && airplaneRet;
}

bool SystemRecoveryModeController::InitializeAirplaneMode()
{
#ifdef APP_MGR_SERVICE_APPMS
    // 已完成过一次初始化后不再强制开启，允许用户在恢复界面主动打开网络。
    if (system::GetBoolParameter(AIRPLANE_INITIALIZED_PARAMETER, false)) {
        return true;
    }

    if (!system::GetBoolParameter(AIRPLANE_RESTORE_VALID_PARAMETER, false)) {
        bool previousState = false;
        if (NetManagerStandard::NetConnClient::GetInstance().GetAirplaneMode(previousState) != ERR_OK) {
            return false;
        }
        if (!system::SetParameter(AIRPLANE_RESTORE_VALUE_PARAMETER, previousState ? "true" : "false") ||
            !system::SetParameter(AIRPLANE_RESTORE_VALID_PARAMETER, "true")) {
            return false;
        }
    }

    if (NetManagerStandard::NetConnClient::GetInstance().SetAirplaneMode(true) != ERR_OK) {
        return false;
    }
    return system::SetParameter(AIRPLANE_INITIALIZED_PARAMETER, "true");
#else
    return true;
#endif
}

bool SystemRecoveryModeController::RestoreAirplaneMode()
{
#ifdef APP_MGR_SERVICE_APPMS
    if (!system::GetBoolParameter(AIRPLANE_RESTORE_VALID_PARAMETER, false)) {
        return system::SetParameter(AIRPLANE_INITIALIZED_PARAMETER, "false");
    }

    const bool previousState = system::GetBoolParameter(AIRPLANE_RESTORE_VALUE_PARAMETER, false);
    if (NetManagerStandard::NetConnClient::GetInstance().SetAirplaneMode(previousState) != ERR_OK) {
        return false;
    }
    // 先恢复网络，再清除恢复标记；中途异常重启时仍可在正常模式继续补偿。
    return system::SetParameter(AIRPLANE_INITIALIZED_PARAMETER, "false") &&
        system::SetParameter(AIRPLANE_RESTORE_VALID_PARAMETER, "false");
#else
    return true;
#endif
}

} // namespace AppExecFwk
} // namespace OHOS
