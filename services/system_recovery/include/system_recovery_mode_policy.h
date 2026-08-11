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
#include <vector>

#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {

/**
 * 系统恢复模式的只读策略。
 *
 * AMS 和 AppMgr 都从这里获取同一份模式状态和白名单判断，避免两个服务
 * 分别读取参数或分别解析配置而产生不一致。应用保持安装状态，策略只决定
 * 当前启动资格，不写入 BMS 的 enabled 状态。
 */
class SystemRecoveryModePolicy final {
public:
    static SystemRecoveryModePolicy &GetInstance();

    void SetEnabled(bool enabled);
    bool IsEnabled() const;
    bool IsRecoveryEssentialBundle(const std::string &bundleName) const;
    bool ShouldBlockBundle(const std::string &bundleName) const;
    void GetRecoveryEssentialBundles(std::vector<std::string> &bundleNames) const;

private:
    SystemRecoveryModePolicy();
    ~SystemRecoveryModePolicy() = default;
    DISALLOW_COPY_AND_MOVE(SystemRecoveryModePolicy);

    std::atomic_bool enabled_ = false;
};

} // namespace AAFwk
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_SYSTEM_RECOVERY_MODE_POLICY_H
