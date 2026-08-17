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
 * 该框架只在 AppMgr 启动后编排飞行模式和系统水印。应用启动门禁由
 * AMS/AppMgr 直接读取只读恢复策略，不修改应用包状态，也不参与
 * Launcher、Settings 或 BMS 的界面和数据库逻辑。
 */
class SystemRecoveryModeFramework final {
public:
    // 返回 AppMgr 进程内唯一的模式编排实例。
    static SystemRecoveryModeFramework &GetInstance();

    // 在 AppMgr 任务队列就绪后异步应用或恢复飞行模式与水印状态。
    void Start(const std::shared_ptr<AAFwk::TaskHandlerWrap> &taskHandler);

private:
    SystemRecoveryModeFramework() = default;
    ~SystemRecoveryModeFramework() = default;
    DISALLOW_COPY_AND_MOVE(SystemRecoveryModeFramework);

    // 提交失败重试任务，等待网络或窗口服务达到可用状态。
    void ScheduleApply(bool enabled, uint32_t retry, int64_t delayMillis);
    // 进入模式时设置一次飞行模式并显示系统水印。
    bool ApplyRecoveryMode();
    // 正常启动时恢复用户原飞行模式状态并清理水印。
    bool ApplyNormalMode();
    // 保存用户原状态后首次开启飞行模式，后续允许用户按产品策略重新联网。
    bool InitializeAirplaneMode();
    // 仅在存在有效备份状态时恢复进入模式前的飞行模式值。
    bool RestoreAirplaneMode();
    // 使用 WindowManager 既有接口设置全屏水印。
    bool SetRecoveryWatermark();
    // 使用同一 PixelMap 清理由本框架设置的水印。
    bool CleanRecoveryWatermark();
    // 从系统资源目录解码水印图片。
    std::shared_ptr<Media::PixelMap> LoadWatermarkImage() const;

    // 保存 AppMgr 提供的异步任务队列，不单独创建线程。
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    // 保留设置水印时使用的 PixelMap，供正常模式清理同一对象。
    std::shared_ptr<Media::PixelMap> watermarkImage_;
    // 防止 AppMgr 重复初始化时提交多组相同任务。
    std::atomic_bool started_ = false;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_FRAMEWORK_H
