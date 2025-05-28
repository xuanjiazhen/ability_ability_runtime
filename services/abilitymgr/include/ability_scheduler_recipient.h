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

#ifndef OHOS_ABILITY_RUNTIME_ABILITY_SCHEDULER_RECIPIENT_H
#define OHOS_ABILITY_RUNTIME_ABILITY_SCHEDULER_RECIPIENT_H

#include <iremote_object.h>

#include "hilog_tag_wrapper.h"

namespace OHOS {
namespace AAFwk {
/**
 * @class AbilitySchedulerRecipient
 * AbilitySchedulerRecipient notices IRemoteBroker died.
 */
class AbilitySchedulerRecipient : public IRemoteObject::DeathRecipient {
public:
    using RemoteDiedHandler = std::function<void(const wptr<IRemoteObject> &)>;
    explicit AbilitySchedulerRecipient(RemoteDiedHandler handler);
    virtual ~AbilitySchedulerRecipient();
    virtual void OnRemoteDied(const wptr<IRemoteObject> &remote);
private:
    RemoteDiedHandler handler_;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_ABILITY_SCHEDULE_STUB_H