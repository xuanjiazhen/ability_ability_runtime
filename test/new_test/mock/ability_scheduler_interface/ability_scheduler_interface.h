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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_SCHEDULER_INTERFACE_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_SCHEDULER_INTERFACE_H

#include <iremote_broker.h>
#include "want.h"

namespace OHOS {
namespace AAFwk {
class IAbilityScheduler : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.aafwk.AbilityScheduler");

    virtual void SendResult(int requestCode, int resultCode, const Want &resultWant) = 0;

    virtual int OpenFile(const Uri &uri, const std::string &mode) = 0;

    virtual void ContinueAbility(const std::string& deviceId, uint32_t versionCode) = 0;
    virtual void NotifyContinuationResult(int32_t result) = 0;

    virtual int CreateModalUIExtension(const Want &want) = 0;

    virtual void OnExecuteIntent(const Want &want) = 0;
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
