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

#include <cstring>

#include "ability_manager_client.h"
#include "bundle_mgr_client.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "parameter.h"
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

    if (WatchParameter(AAFwk::SYSTEM_RECOVERY_MODE_PARAMETER, OnModeParameterChanged, this) != 0) {
        TAG_LOGE(AAFwkTag::APPMGR, "watch recovery mode parameter failed");
    }
    const bool enabled = system::GetParameter(AAFwk::SYSTEM_RECOVERY_MODE_PARAMETER, "") ==
        AAFwk::SYSTEM_RECOVERY_MODE_VALUE;
    HandleModeChanged(enabled);
}

void SystemRecoveryModeController::OnModeParameterChanged(const char *key, const char *value, void *context)
{
    if (key == nullptr || context == nullptr ||
        std::strcmp(key, AAFwk::SYSTEM_RECOVERY_MODE_PARAMETER) != 0) {
        return;
    }
    const bool enabled = value != nullptr && std::strcmp(value, AAFwk::SYSTEM_RECOVERY_MODE_VALUE) == 0;
    static_cast<SystemRecoveryModeController *>(context)->HandleModeChanged(enabled);
}

void SystemRecoveryModeController::HandleModeChanged(bool enabled)
{
    // 先更新内存策略，确保进入恢复模式时 AppMgr 的进程启动校验立即生效。
    AAFwk::SystemRecoveryModePolicy::GetInstance().SetEnabled(enabled);
    const uint64_t generation = generation_.fetch_add(1) + 1;
    ScheduleApply(enabled, generation, 0, 0);
}

void SystemRecoveryModeController::ScheduleApply(
    bool enabled, uint64_t generation, uint32_t retry, int64_t delayMillis)
{
    if (taskHandler_ == nullptr) {
        return;
    }
    taskHandler_->SubmitTaskJust([this, enabled, generation, retry]() {
        std::lock_guard<std::mutex> lock(applyMutex_);
        // 参数快速切换时，旧任务不能覆盖最新模式。
        if (generation_.load() != generation) {
            return;
        }
        const bool success = enabled ? ApplyRecoveryMode() : ApplyNormalMode();
        if (!success && retry < MAX_RETRY_TIMES && generation_.load() == generation) {
            ScheduleApply(enabled, generation, retry + 1, RETRY_DELAY_MILLISECONDS);
        }
    }, "ApplySystemRecoveryMode", delayMillis);
}

bool SystemRecoveryModeController::ApplyRecoveryMode()
{
    const int32_t abilityRet = AAFwk::AbilityManagerClient::GetInstance()->BlockAllAppStart(true);
    BundleMgrClient bundleMgrClient;
    const int32_t bundleRet = bundleMgrClient.SetInstallOperationPaused(true);
    const bool airplaneRet = InitializeAirplaneMode();
    TAG_LOGI(AAFwkTag::APPMGR,
        "apply recovery mode, abilityRet=%{public}d, bundleRet=%{public}d, airplaneRet=%{public}d",
        abilityRet, bundleRet, airplaneRet);
    return abilityRet == ERR_OK && bundleRet == ERR_OK && airplaneRet;
}

bool SystemRecoveryModeController::ApplyNormalMode()
{
    const int32_t abilityRet = AAFwk::AbilityManagerClient::GetInstance()->BlockAllAppStart(false);
    BundleMgrClient bundleMgrClient;
    const int32_t bundleRet = bundleMgrClient.SetInstallOperationPaused(false);
    const bool airplaneRet = RestoreAirplaneMode();
    TAG_LOGI(AAFwkTag::APPMGR,
        "restore normal mode, abilityRet=%{public}d, bundleRet=%{public}d, airplaneRet=%{public}d",
        abilityRet, bundleRet, airplaneRet);
    return abilityRet == ERR_OK && bundleRet == ERR_OK && airplaneRet;
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
