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

#include "power_mode_changed_listener.h"

#include "app_mgr_service_inner.h"
#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AppExecFwk {
using namespace OHOS::PowerMgr;

PowerModeChangedListener::PowerModeChangedListener(const std::shared_ptr<AppMgrServiceInner> &owner,
    const std::shared_ptr<AAFwk::TaskHandlerWrap>& handler) : owner_(owner), taskHandler_(handler) {}

PowerModeChangedListener::~PowerModeChangedListener() {}

void PowerModeChangedListener::OnPowerModeChanged(PowerMode mode)
{
    if (taskHandler_) {
        auto task = [inner = owner_] {
            auto owner = inner.lock();
            if (!owner) {
                TAG_LOGW(AAFwkTag::APPMGR, "get app mgr service inner fail");
                return;
            }
            owner->HandlePowerModeChanged();
        };
        taskHandler_->SubmitTask(task, "PowerModeChangedListener::OnPowerModeChanged");
    }
}
}  // namespace AppExecFwk
}  // namespace OHOS
