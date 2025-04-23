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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ASSERT_FAULT_CALLBACK_DEATH_MGR_H
#define MOCK_OHOS_ABILITY_RUNTIME_ASSERT_FAULT_CALLBACK_DEATH_MGR_H

#include <memory>

#include "iremote_object.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
class AssertFaultCallbackDeathMgr : public DelayedSingleton<AssertFaultCallbackDeathMgr>,
    public std::enable_shared_from_this<AssertFaultCallbackDeathMgr> {
    DISALLOW_COPY_AND_MOVE(AssertFaultCallbackDeathMgr);
public:
    using CallbackTask = std::function<void(const std::string &)>;
    AssertFaultCallbackDeathMgr() = default;
    virtual ~AssertFaultCallbackDeathMgr();

    void AddAssertFaultCallback(sptr<IRemoteObject> &remote, CallbackTask callback) {}
    void CallAssertFaultCallback(
        uint64_t assertFaultSessionId, AAFwk::UserStatus status = AAFwk::UserStatus::ASSERT_TERMINATE) {}
};
}  // namespace AbilityRuntime
}  // namespace OHOS
#endif
