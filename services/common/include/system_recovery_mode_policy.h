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

#include <atomic>
#include <string>

#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {

/**
 * AMS 与 AppMgr 共用的系统恢复模式决策。
 * 启动时读取一次参数，后续由恢复模式控制器在参数变化时更新，避免每次启动
 * 应用都访问参数服务。
 */
class SystemRecoveryModePolicy final {
public:
    static SystemRecoveryModePolicy &GetInstance();

    void SetEnabled(bool enabled);
    bool IsEnabled() const;
    bool IsRecoveryEssentialBundle(const std::string &bundleName) const;
    bool ShouldBlockBundle(const std::string &bundleName) const;

private:
    SystemRecoveryModePolicy();
    ~SystemRecoveryModePolicy() = default;
    DISALLOW_COPY_AND_MOVE(SystemRecoveryModePolicy);

    std::atomic_bool enabled_ = false;
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_POLICY_H
