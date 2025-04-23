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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_FREE_INSTALL_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_FREE_INSTALL_MANAGER_H

#include <future>
#include "cpp/mutex.h"

#include <iremote_object.h>
#include <iremote_stub.h>
#include <memory>

#include "ability_info.h"
#include "start_options.h"
#include "want.h"

namespace OHOS {
namespace AAFwk {
class AbilityManagerService;

struct FreeInstallInfo {
    bool isInstalled = false;
    bool isStartUIAbilityBySCBCalled = false;
    bool isFreeInstallFinished = false;
    bool isOpenAtomicServiceShortUrl = false;
    uint32_t specifyTokenId = 0;
    int32_t userId = -1;
    int32_t requestCode = -1;
    int resultCode = 0;
    std::string identity;
    sptr<IRemoteObject> callerToken = nullptr;
    std::shared_ptr<Want> originalWant = nullptr;
    std::shared_ptr<StartOptions> startOptions = nullptr;
    Want want;
};

struct FreeInstallParams {
    bool isAsync = false;
    bool isOpenAtomicServiceShortUrl = false;
    uint32_t specifyTokenId = 0;
    std::shared_ptr<Want> originalWant = nullptr;
    std::shared_ptr<StartOptions> startOptions = nullptr;
};

class FreeInstallManager : public std::enable_shared_from_this<FreeInstallManager> {
public:
    explicit FreeInstallManager(const std::weak_ptr<AbilityManagerService> &server) {}
    virtual ~FreeInstallManager() = default;

    int StartFreeInstall(const Want &want, int32_t userId, int requestCode, const sptr<IRemoteObject> &callerToken,
        std::shared_ptr<FreeInstallParams> param = nullptr)
    {
        return 0;
    }

    int StartRemoteFreeInstall(const Want &want, int requestCode, int32_t validUserId,
        const sptr<IRemoteObject> &callerToken)
    {
        return 0;
    }

    int FreeInstallAbilityFromRemote(const Want &want, const sptr<IRemoteObject> &callback,
        int32_t userId, int requestCode)
    {
        return 0;
    }

    int ConnectFreeInstall(const Want &want, int32_t userId, const sptr<IRemoteObject> &callerToken,
        const std::string& localDeviceId)
    {
        return 0;
    }

    int AddFreeInstallObserver(const sptr<IRemoteObject> &callerToken,
        const sptr<AbilityRuntime::IFreeInstallObserver> &observer)
    {
        return 0;
    }

    bool GetFreeInstallTaskInfo(const std::string& bundleName, const std::string& abilityName,
        const std::string& startTime, FreeInstallInfo& taskInfo)
    {
        return false;
    }

    bool GetFreeInstallTaskInfo(const std::string& sessionId, FreeInstallInfo& taskInfo)
    {
        return false;
    }

    void SetSCBCallStatus(const std::string& bundleName, const std::string& abilityName,
        const std::string& startTime, bool scbCallStatus) {}

    void SetPreStartMissionCallStatus(const std::string& bundleName, const std::string& abilityName,
        const std::string& startTime, bool preStartMissionCallStatus) {}

    void SetFreeInstallTaskSessionId(const std::string& bundleName, const std::string& abilityName,
        const std::string& startTime, const std::string& sessionId) {}
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
