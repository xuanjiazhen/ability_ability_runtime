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

#ifndef OHOS_MODULAR_OBJECT_CONNECTION_HANDLER_H
#define OHOS_MODULAR_OBJECT_CONNECTION_HANDLER_H

#include "ability_connect_callback_interface.h"
#include "extension_ability_info.h"
#include "iremote_object.h"
#include "want.h"

namespace OHOS {
namespace AbilityRuntime {

class ModularObjectConnectionHandler {
public:
    virtual ~ModularObjectConnectionHandler() = default;

    virtual int32_t ConnectAbilityCommon(const AAFwk::Want &want,
        sptr<AAFwk::IAbilityConnection> connect, sptr<IRemoteObject> callerToken,
        AppExecFwk::ExtensionAbilityType extensionType, int32_t userId) = 0;

    virtual int32_t DisconnectAbility(sptr<AAFwk::IAbilityConnection> connect) = 0;

    virtual bool HasRunningUIAbilityOrExtension(int32_t targetUid, int32_t userId) = 0;
};

} // namespace AbilityRuntime
} // namespace OHOS

#endif // OHOS_MODULAR_OBJECT_CONNECTION_HANDLER_H
