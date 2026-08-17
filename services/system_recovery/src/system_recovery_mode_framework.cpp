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

#include "errors.h"
#include "hilog_tag_wrapper.h"
#include "parameters.h"
#include "system_recovery_mode_constants.h"
#include "system_recovery_mode_policy.h"
#include "system_recovery_settings_data.h"
#include "task_handler_wrap.h"

#ifdef SUPPORT_SYSTEM_RECOVERY_AIRPLANE_MODE
#include "net_conn_client.h"
#endif

#ifdef SUPPORT_GRAPHICS
#include "image_source.h"
#include "window_manager.h"
#endif

namespace OHOS {
namespace AppExecFwk {
namespace {

#ifdef SUPPORT_SYSTEM_RECOVERY_AIRPLANE_MODE
constexpr char AIRPLANE_RESTORE_VALID_PARAMETER[] =
    "persist.sys.abilityms.system_recovery.airplane_restore_valid";
constexpr char AIRPLANE_RESTORE_VALUE_PARAMETER[] =
    "persist.sys.abilityms.system_recovery.airplane_restore_value";
constexpr char AIRPLANE_INITIALIZED_PARAMETER[] =
    "persist.sys.abilityms.system_recovery.airplane_initialized";
#endif
constexpr uint32_t MAX_RETRY_TIMES = 30;
constexpr int64_t RETRY_DELAY_MILLISECONDS = 2000;

} // namespace

SystemRecoveryModeFramework &SystemRecoveryModeFramework::GetInstance()
{
    // 编排状态只属于 AppMgr 进程，使用函数内静态对象保证按需初始化。
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

    // 启动模式在 init 阶段已固定，框架只读取策略缓存并执行本次启动的编排任务。
    const bool enabled = AAFwk::SystemRecoveryModePolicy::GetInstance().IsEnabled();
    ScheduleApply(enabled, 0, 0);
}

void SystemRecoveryModeFramework::ScheduleApply(bool enabled, uint32_t retry, int64_t delayMillis)
{
    if (taskHandler_ == nullptr) {
        return;
    }
    taskHandler_->SubmitTaskJust([this, enabled, retry]() {
        // 网络或窗口服务尚未就绪时按固定间隔重试，成功后停止继续提交。
        const bool success = enabled ? ApplyRecoveryMode() : ApplyNormalMode();
        if (!success && retry < MAX_RETRY_TIMES) {
            ScheduleApply(enabled, retry + 1, RETRY_DELAY_MILLISECONDS);
        }
    }, "ApplySystemRecoveryMode", delayMillis);
}

bool SystemRecoveryModeFramework::ApplyRecoveryMode()
{
    // 应用启动门禁由 AMS/AppMgr 根据本次启动缓存的恢复模式策略直接判断，
    // 不复用 BlockAllAppStart 的全局暂停状态，避免阻断 StartUser 等系统生命周期操作。
    const bool airplaneRet = InitializeAirplaneMode();
    const bool watermarkRet = SetRecoveryWatermark();
    TAG_LOGI(AAFwkTag::APPMGR,
        "apply recovery mode, airplaneRet=%{public}d, watermarkRet=%{public}d", airplaneRet, watermarkRet);
    return airplaneRet && watermarkRet;
}

bool SystemRecoveryModeFramework::ApplyNormalMode()
{
    // 正常启动只补偿本框架持久化的状态，不修改其他应用或系统设置。
    const bool airplaneRet = RestoreAirplaneMode();
    const bool watermarkRet = CleanRecoveryWatermark();
    TAG_LOGI(AAFwkTag::APPMGR,
        "restore normal mode, airplaneRet=%{public}d, watermarkRet=%{public}d", airplaneRet, watermarkRet);
    return airplaneRet && watermarkRet;
}

bool SystemRecoveryModeFramework::InitializeAirplaneMode()
{
#ifdef SUPPORT_SYSTEM_RECOVERY_AIRPLANE_MODE
    // 只在进入恢复模式时强制设置一次，之后允许恢复界面按产品策略打开网络。
    if (system::GetBoolParameter(AIRPLANE_INITIALIZED_PARAMETER, false)) {
        return true;
    }

    if (!system::GetBoolParameter(AIRPLANE_RESTORE_VALID_PARAMETER, false)) {
        bool previousState = false;
        // 直接读取 SettingsData，与 Settings 应用展示的用户飞行模式状态保持一致。
        if (!GetAirplaneModeFromSettingsData(previousState)) {
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
#ifdef SUPPORT_SYSTEM_RECOVERY_AIRPLANE_MODE
    // 没有有效备份说明上次未成功改写飞行模式，只清理本次初始化标记。
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
    // PixelMap 只解码一次，重试时复用同一资源对象。
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
    // 进程重启后 WindowManager 已清理旧窗口，空对象无需额外调用。
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
