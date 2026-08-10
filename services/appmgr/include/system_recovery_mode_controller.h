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

#ifndef OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_CONTROLLER_H
#define OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_CONTROLLER_H

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>

#include "nocopyable.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AppExecFwk {

/**
 * 系统恢复模式的运行期控制器。
 *
 * 控制器位于 foundation 的 AppMgr 中，只负责编排已有系统服务 IPC：AMS 控制
 * 应用启动、BMS 暂停安装操作、NetConn 初始化飞行模式。网络能力不做持续拦截，
 * 因此用户进入恢复界面后仍可主动打开 Wi-Fi、蜂窝网络和电话能力。
 */
class SystemRecoveryModeController final {
public:
    static SystemRecoveryModeController &GetInstance();

    void Start(const std::shared_ptr<AAFwk::TaskHandlerWrap> &taskHandler);

private:
    SystemRecoveryModeController() = default;
    ~SystemRecoveryModeController() = default;
    DISALLOW_COPY_AND_MOVE(SystemRecoveryModeController);

    static void OnModeParameterChanged(const char *key, const char *value, void *context);
    void HandleModeChanged(bool enabled);
    void ScheduleApply(bool enabled, uint64_t generation, uint32_t retry, int64_t delayMillis);
    bool ApplyRecoveryMode();
    bool ApplyNormalMode();
    bool InitializeAirplaneMode();
    bool RestoreAirplaneMode();

    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::atomic_bool started_ = false;
    std::atomic_uint64_t generation_ = 0;
    std::mutex applyMutex_;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_CONTROLLER_H
