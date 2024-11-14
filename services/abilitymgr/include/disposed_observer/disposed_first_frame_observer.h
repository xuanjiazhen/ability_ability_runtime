/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_ABILITY_RUNTIME_DISPOSED_FIRST_FRAME_OBSERVER_H
#define OHOS_ABILITY_RUNTIME_DISPOSED_FIRST_FRAME_OBSERVER_H

#include "ability_first_frame_state_observer_stub.h"
#include "disposed_rule.h"

namespace OHOS {
namespace AAFwk {
enum DisposedModalType {
    MODAL_SYSTEM,
    MODAL_APP
};

class DisposedFirstFrameObserver : public AppExecFwk::AbilityFirstFrameStateObserverStub {
public:
    DisposedFirstFrameObserver(const AppExecFwk::DisposedRule &disposedRule) : disposedRule_(disposedRule) {}
    void OnAbilityFirstFrameState(const AppExecFwk::AbilityFirstFrameStateData &abilityFirstFrameStateData) override;

    void SetModalType(DisposedModalType type);

private:
    AppExecFwk::DisposedRule disposedRule_;
    DisposedModalType type_ = DisposedModalType::MODAL_SYSTEM;
};
} // namespace AAFwk
} // namespace OHOS
#endif // OHOS_ABILITY_RUNTIME_DISPOSED_FIRST_FRAME_OBSERVER_H
