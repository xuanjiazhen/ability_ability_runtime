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

#include "interceptor/system_recovery_mode_interceptor.h"

#include "hilog_tag_wrapper.h"
#include "system_recovery_mode_policy.h"

namespace OHOS {
namespace AAFwk {

ErrCode SystemRecoveryModeInterceptor::DoProcess(const AbilityInterceptorParam &param)
{
    auto &policy = SystemRecoveryModePolicy::GetInstance();
    if (!policy.IsRecoveryMode()) {
        return ERR_OK;
    }

    // 本拦截器注册在 afterCheckExecuter_；目标解析失败时默认拒绝，
    // 不能直接信任 Want 中由调用方传入的包名。
    if (param.abilityInfo == nullptr) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "block unresolved target in system recovery mode");
        return ERR_SYSTEM_RECOVERY_MODE_BLOCKED;
    }

    RecoveryBundleKey key {
        param.abilityInfo->bundleName,
        param.userId,
        param.appIndex,
    };
    if (policy.ShouldBlockBundle(key, &param.abilityInfo->applicationInfo)) {
        TAG_LOGW(AAFwkTag::ABILITYMGR, "block bundle in system recovery mode: %{public}s",
            key.bundleName.c_str());
        return ERR_SYSTEM_RECOVERY_MODE_BLOCKED;
    }
    return ERR_OK;
}

} // namespace AAFwk
} // namespace OHOS
