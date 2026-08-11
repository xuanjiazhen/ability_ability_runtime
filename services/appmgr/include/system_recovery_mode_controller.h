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

#include "nocopyable.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AppExecFwk {

/**
 * 系统恢复模式启动控制器。
 *
 * 控制器位于 foundation 的 AppMgr 中，只负责编排已有系统服务 IPC：AMS 控制
 * 应用启动、NetConn 初始化飞行模式。BMS 保持原有的查询、安装、更新和卸载能力，
 * 应用是否能够运行由 AMS 和 AppMgr 的进程创建检查共同决定。网络能力不做持续拦截，
 * 因此用户进入恢复界面后仍可主动打开 Wi-Fi、蜂窝网络和电话能力。
 * 模式仅由完整重启时的启动参数决定，不支持运行期切换。
 */
class SystemRecoveryModeController final {
public:
    static SystemRecoveryModeController &GetInstance();

    void Start(const std::shared_ptr<AAFwk::TaskHandlerWrap> &taskHandler);

private:
    SystemRecoveryModeController() = default;
    ~SystemRecoveryModeController() = default;
    DISALLOW_COPY_AND_MOVE(SystemRecoveryModeController);

    void ScheduleApply(bool enabled, uint32_t retry, int64_t delayMillis);
    bool ApplyRecoveryMode();
    bool ApplyNormalMode();
    bool InitializeAirplaneMode();
    bool RestoreAirplaneMode();

    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
    std::atomic_bool started_ = false;
};

} // namespace AppExecFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_CONTROLLER_H
