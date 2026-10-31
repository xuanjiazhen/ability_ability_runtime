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

#ifndef OHOS_ABILITY_RUNTIME_POWER_MODE_CHANGED_LISTENER_H
#define OHOS_ABILITY_RUNTIME_POWER_MODE_CHANGED_LISTENER_H

#include "power_mode_callback_stub.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AppExecFwk {
class AppMgrServiceInner;

class PowerModeChangedListener : public OHOS::PowerMgr::PowerModeCallbackStub {
public:
    PowerModeChangedListener(const std::shared_ptr<AppMgrServiceInner>& owner,
        const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler);
    virtual ~PowerModeChangedListener();

    void OnPowerModeChanged(OHOS::PowerMgr::PowerMode mode) override;

private:
    std::weak_ptr<AppMgrServiceInner> owner_;
    std::shared_ptr<AAFwk::TaskHandlerWrap> taskHandler_;
};
}  // namespace AppExecFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_POWER_MODE_CHANGED_LISTENER_H
