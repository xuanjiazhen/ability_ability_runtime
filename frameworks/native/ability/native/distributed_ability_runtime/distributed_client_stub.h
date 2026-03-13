/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef DISTRIBUTED_CLIENT_STUB_H
#define DISTRIBUTED_CLIENT_STUB_H

#include "iremote_object.h"

namespace OHOS {
namespace AAFwk {

/**
 * Stub implementation of DistributedClient for when distributed features are disabled
 */
class DistributedClient {
public:
    DistributedClient() = default;
    ~DistributedClient() = default;

    /**
     * Stub implementation for StartRemoteFreeInstall
     */
    int32_t StartRemoteFreeInstall(const Want& want, int32_t callerUid, int32_t requestCode,
        uint32_t accessToken, sptr<IRemoteObject> callback)
    {
        // Always return error when distributed features are disabled
        return ERR_ABILITY_RUNTIME_FREE_INSTALL_FAILED;
    }

    // Stub other methods if needed
    int32_t StartRemoteAbility(const Want& want, int32_t callerUid, int32_t requestCode,
        uint32_t accessToken, uint32_t specifyTokenId)
    {
        return ERR_ABILITY_RUNTIME_FREE_INSTALL_FAILED;
    }

    int32_t ConnectRemoteAbility(const Want& want, const sptr<IRemoteObject>& connect)
    {
        return ERR_ABILITY_RUNTIME_FREE_INSTALL_FAILED;
    }

    int32_t StopRemoteExtensionAbility(const Want& want, int32_t callerUid, uint32_t accessToken, int32_t extensionType)
    {
        return ERR_ABILITY_RUNTIME_FREE_INSTALL_FAILED;
    }
};

}  // namespace AAFwk
}  // namespace OHOS

#endif  // DISTRIBUTED_CLIENT_STUB_H