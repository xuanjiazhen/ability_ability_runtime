/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_INTERFACE_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_INTERCEPTOR_INTERFACE_H

#include "ability_info.h"
#include "want.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AAFwk {
struct AbilityInterceptorParam {
    AbilityInterceptorParam(const Want &want, int requestCode, int32_t userId, bool isWithUI,
        const sptr<IRemoteObject> &callerToken, const std::function<bool(void)> &shouldBlockAllAppStartFunc) {}
    AbilityInterceptorParam(const Want &want, int requestCode, int32_t userId, bool isWithUI,
        const sptr<IRemoteObject> &callerToken, const std::shared_ptr<AppExecFwk::AbilityInfo> &abilityInfo,
        bool isStartAsCaller = false, int32_t appIndex = 0) {}
};

class IAbilityInterceptor {
public:
    ErrCode DoProcess(AbilityInterceptorParam param)
    {
        return 0;
    }

    void SetTaskHandler(std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler) {}
};
} // namespace AAFwk
} // namespace OHOS
#endif