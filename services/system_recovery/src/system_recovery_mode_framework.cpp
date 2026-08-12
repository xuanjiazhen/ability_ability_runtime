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

#include "system_recovery_mode_framework.h"

#include <utility>

#include "ability_manager_client.h"
#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "parameters.h"
#include "system_recovery_mode_constants.h"
#include "system_recovery_mode_policy.h"
#include "system_recovery_settings_data.h"
#include "task_handler_wrap.h"

#ifdef APP_MGR_SERVICE_APPMS
#include "net_conn_client.h"
#endif

#ifdef SUPPORT_GRAPHICS
#include "image_source.h"
#include "window_manager.h"
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

SystemRecoveryModeFramework &SystemRecoveryModeFramework::GetInstance()
{
    static SystemRecoveryModeFramework framework;
    return framework;
}

void SystemRecoveryModeFramework::Start(const std::shared_ptr<AAFwk::TaskHandlerWrap> &taskHandler)
{
    bool expected = false;
    if (!started_.compare_exchange_strong(expected, true)) {
        return;
    }
    taskHandler_ = taskHandler;
    if (taskHandler_ == nullptr) {
        TAG_LOGE(AAFwkTag::APPMGR, "start system recovery framework without task handler");
        started_.store(false);
        return;
    }

    const bool enabled = system::GetParameter(AAFwk::SYSTEM_RECOVERY_MODE_PARAMETER, "") ==
        AAFwk::SYSTEM_RECOVERY_MODE_VALUE;
    // 参数已经在 AppMgr 的 ReadConfig/启动阶段可用，这里只缓存本次启动状态。
    AAFwk::SystemRecoveryModePolicy::GetInstance().SetEnabled(enabled);
    ScheduleApply(enabled, 0, 0);
}

bool SystemRecoveryModeFramework::IsSystemRecoveryMode() const
{
    return AAFwk::SystemRecoveryModePolicy::GetInstance().IsEnabled();
}

bool SystemRecoveryModeFramework::IsSystemRecoveryAppAllowed(const std::string &bundleName) const
{
    return !AAFwk::SystemRecoveryModePolicy::GetInstance().ShouldBlockBundle(bundleName);
}

void SystemRecoveryModeFramework::GetSystemRecoveryAppWhitelist(std::vector<std::string> &bundleNames) const
{
    AAFwk::SystemRecoveryModePolicy::GetInstance().GetRecoveryEssentialBundles(bundleNames);
}

void SystemRecoveryModeFramework::ScheduleApply(bool enabled, uint32_t retry, int64_t delayMillis)
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

bool SystemRecoveryModeFramework::ApplyRecoveryMode()
{
    // 应用启动门禁通过 AMS 已有 IPC 设置，真正的白名单判断仍在 AMS/AppMgr 内部完成。
    const int32_t abilityRet = AAFwk::AbilityManagerClient::GetInstance()->BlockAllAppStart(true);
    const bool airplaneRet = InitializeAirplaneMode();
    const bool watermarkRet = SetRecoveryWatermark();
    TAG_LOGI(AAFwkTag::APPMGR,
        "apply recovery mode, abilityRet=%{public}d, airplaneRet=%{public}d, watermarkRet=%{public}d",
        abilityRet, airplaneRet, watermarkRet);
    return abilityRet == ERR_OK && airplaneRet && watermarkRet;
}

bool SystemRecoveryModeFramework::ApplyNormalMode()
{
    const int32_t abilityRet = AAFwk::AbilityManagerClient::GetInstance()->BlockAllAppStart(false);
    const bool airplaneRet = RestoreAirplaneMode();
    const bool watermarkRet = CleanRecoveryWatermark();
    TAG_LOGI(AAFwkTag::APPMGR,
        "restore normal mode, abilityRet=%{public}d, airplaneRet=%{public}d, watermarkRet=%{public}d",
        abilityRet, airplaneRet, watermarkRet);
    return abilityRet == ERR_OK && airplaneRet && watermarkRet;
}

bool SystemRecoveryModeFramework::InitializeAirplaneMode()
{
#ifdef APP_MGR_SERVICE_APPMS
    // 只在进入恢复模式时强制设置一次，之后允许恢复界面按产品策略打开网络。
    if (system::GetBoolParameter(AIRPLANE_INITIALIZED_PARAMETER, false)) {
        return true;
    }

    if (!system::GetBoolParameter(AIRPLANE_RESTORE_VALID_PARAMETER, false)) {
        bool previousState = false;
        // 直接读取 SettingsData，与 Settings 应用展示的用户飞行模式状态保持一致。
        if (!SystemRecoverySettingsData::GetAirplaneMode(previousState)) {
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

bool SystemRecoveryModeFramework::RestoreAirplaneMode()
{
#ifdef APP_MGR_SERVICE_APPMS
    if (!system::GetBoolParameter(AIRPLANE_RESTORE_VALID_PARAMETER, false)) {
        return system::SetParameter(AIRPLANE_INITIALIZED_PARAMETER, "false");
    }

    const bool previousState = system::GetBoolParameter(AIRPLANE_RESTORE_VALUE_PARAMETER, false);
    if (NetManagerStandard::NetConnClient::GetInstance().SetAirplaneMode(previousState) != ERR_OK) {
        return false;
    }
    // 先恢复网络，再清除标记，异常重启时仍可继续补偿。
    return system::SetParameter(AIRPLANE_INITIALIZED_PARAMETER, "false") &&
        system::SetParameter(AIRPLANE_RESTORE_VALID_PARAMETER, "false");
#else
    return true;
#endif
}

bool SystemRecoveryModeFramework::SetRecoveryWatermark()
{
#ifdef SUPPORT_GRAPHICS
    if (watermarkImage_ == nullptr) {
        watermarkImage_ = LoadWatermarkImage();
    }
    if (watermarkImage_ == nullptr) {
        // 水印图片是产品资源；资源缺失不能阻断应用门禁和网络初始化。
        TAG_LOGW(AAFwkTag::APPMGR, "system recovery watermark resource is unavailable");
        return true;
    }
    return Rosen::WindowManager::GetInstance().SetScreenWatermarkImage(
        watermarkImage_, AAFwk::SYSTEM_RECOVERY_WATERMARK_PRIORITY) == Rosen::WMError::WM_OK;
#else
    return true;
#endif
}

bool SystemRecoveryModeFramework::CleanRecoveryWatermark()
{
#ifdef SUPPORT_GRAPHICS
    if (watermarkImage_ == nullptr) {
        return true;
    }
    return Rosen::WindowManager::GetInstance().CleanScreenWatermarkImage(watermarkImage_) == Rosen::WMError::WM_OK;
#else
    return true;
#endif
}

std::shared_ptr<Media::PixelMap> SystemRecoveryModeFramework::LoadWatermarkImage() const
{
#ifdef SUPPORT_GRAPHICS
    Media::SourceOptions sourceOptions;
    uint32_t errorCode = 0;
    auto imageSource = Media::ImageSource::CreateImageSource(AAFwk::SYSTEM_RECOVERY_WATERMARK_IMAGE,
        sourceOptions, errorCode);
    if (imageSource == nullptr || errorCode != ERR_OK) {
        TAG_LOGW(AAFwkTag::APPMGR, "load recovery watermark source failed, errorCode=%{public}d", errorCode);
        return nullptr;
    }
    Media::DecodeOptions decodeOptions;
    auto pixelMap = imageSource->CreatePixelMap(decodeOptions, errorCode);
    if (pixelMap == nullptr || errorCode != ERR_OK) {
        TAG_LOGW(AAFwkTag::APPMGR, "decode recovery watermark failed, errorCode=%{public}d", errorCode);
        return nullptr;
    }
    return std::shared_ptr<Media::PixelMap>(std::move(pixelMap));
#else
    return nullptr;
#endif
}

} // namespace AppExecFwk
} // namespace OHOS
