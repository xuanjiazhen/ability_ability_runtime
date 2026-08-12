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

#ifndef OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_FRAMEWORK_H
#define OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_FRAMEWORK_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
class TaskHandlerWrap;
}
namespace Media {
class PixelMap;
}
namespace AppExecFwk {

/**
 * 系统恢复模式框架统一入口。
 *
 * 该框架只做系统服务编排：通过 AMS 现有 IPC 设置应用启动门禁，通过
 * NetConnClient 设置初始飞行模式，通过 WindowManager 现有全屏水印接口
 * 设置水印，并向 AMS IPC 提供只读的模式和白名单查询能力。它不修改应用
 * 包状态，也不参与 Launcher、Settings 或 BMS 的界面和数据库逻辑。
 */
class SystemRecoveryModeFramework final {
public:
    static SystemRecoveryModeFramework &GetInstance();

    void Start(const std::shared_ptr<AAFwk::TaskHandlerWrap> &taskHandler);

    bool IsSystemRecoveryMode() const;
    bool IsSystemRecoveryAppAllowed(const std::string &bundleName) const;
    void GetSystemRecoveryAppWhitelist(std::vector<std::string> &bundleNames) const;

private:
    SystemRecoveryModeFramework() = default;
    ~SystemRecoveryModeFramework() = default;
    DISALLOW_COPY_AND_MOVE(SystemRecoveryModeFramework);

    void ScheduleApply(bool enabled, uint32_t retry, int64_t delayMillis);
    bool ApplyRecoveryMode();
    bool ApplyNormalMode();
    bool InitializeAirplaneMode();
    bool RestoreAirplaneMode();
    bool SetRecoveryWatermark();
    bool CleanRecoveryWatermark();
    std::shared_ptr<Media::PixelMap> LoadWatermarkImage() const;

    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::shared_ptr<Media::PixelMap> watermarkImage_;
    std::atomic_bool started_ = false;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_FRAMEWORK_H
