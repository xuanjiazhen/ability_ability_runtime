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

#ifndef OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_POLICY_H
#define OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_POLICY_H

#include <string>

#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {

/**
 * 系统恢复模式的只读策略。
 *
 * AMS 和 AppMgr 各自在进程启动时读取同一个只读启动参数和同一份白名单
 * 配置。应用保持安装状态，策略只决定当前启动资格，不写入 BMS 状态。
 */
class SystemRecoveryModePolicy final {
public:
    // 返回当前链接模块中的只读策略实例。
    static SystemRecoveryModePolicy &GetInstance();

    // 返回构造时从 persist.start.mode 缓存的本次启动模式。
    bool IsEnabled() const;
    // 判断 Bundle 是否属于产品配置的最小可运行集合。
    bool IsRecoveryEssentialBundle(const std::string &bundleName) const;
    // 非恢复模式始终放行；恢复模式按系统应用配置和 Bundle 白名单共同判断。
    bool ShouldBlockBundle(const std::string &bundleName, bool isSystemApp) const;

private:
    SystemRecoveryModePolicy();
    ~SystemRecoveryModePolicy() = default;
    DISALLOW_COPY_AND_MOVE(SystemRecoveryModePolicy);

    // 模式只允许通过重启切换，因此构造后保持不变，不需要运行期原子更新接口。
    const bool enabled_;
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_POLICY_H
