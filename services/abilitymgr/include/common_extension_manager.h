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

#ifndef OHOS_ABILITY_RUNTIME_COMMON_EXTENSION_MANAGER_H
#define OHOS_ABILITY_RUNTIME_COMMON_EXTENSION_MANAGER_H

#include "ability_connect_manager.h"
#include "nocopyable.h"

namespace OHOS {
namespace AAFwk {
class CommonExtensionManager : public AbilityConnectManager {
public:
    explicit CommonExtensionManager(int userId);
    virtual ~CommonExtensionManager();

    /**
     * SuspendExtensionAbilityLocked, suspend session with callback.
     *
     * @param connect, Callback used to notify caller the result of suspend.
     * @return Returns ERR_OK on success, others on failure.
     */
    int32_t SuspendExtensionAbilityLocked(const sptr<IAbilityConnection> &connect);

protected:


private:
    void SetServiceAfterNewCreate(const AbilityRequest &abilityRequest, ExtensionRecordBase &targetService);
    void RemoveServiceFromMapSafe(const std::string &serviceKey);

    std::atomic_uint32_t sceneBoardTokenId_ = 0;

    DISALLOW_COPY_AND_MOVE(CommonExtensionManager);
};
}  // namespace AAFwk
}  // namespace OHOS
#endif  // OHOS_ABILITY_RUNTIME_COMMON_EXTENSION_MANAGER_H