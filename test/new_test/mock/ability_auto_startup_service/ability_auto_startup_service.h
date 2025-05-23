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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_SERVICE_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_AUTO_STARTUP_SERVICE_H

#include <vector>

#include "auto_startup_info.h"
#include "iremote_object.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityAutoStartupService : public std::enable_shared_from_this<AbilityAutoStartupService> {
public:
    int32_t RegisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
    {
        return 0;
    }

    int32_t UnregisterAutoStartupSystemCallback(const sptr<IRemoteObject> &callback)
    {
        return 0;
    }

    int32_t SetApplicationAutoStartup(const AutoStartupInfo &info)
    {
        return 0;
    }

    int32_t CancelApplicationAutoStartup(const AutoStartupInfo &info)
    {
        return 0;
    }

    int32_t QueryAllAutoStartupApplications(std::vector<AutoStartupInfo> &infoList, int32_t userId)
    {
        return 0;
    }

    int32_t QueryAllAutoStartupApplicationsWithoutPermission(std::vector<AutoStartupInfo> &infoList, int32_t userId)
    {
        return 0;
    }

    int32_t DeleteAutoStartupData(const std::string &bundleName, int32_t accessTokenId)
    {
        return 0;
    }

    int32_t CheckAutoStartupData(const std::string &bundleName, int32_t uid)
    {
        return 0;
    }

    int32_t SetApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
    {
        return 0;
    }

    int32_t CancelApplicationAutoStartupByEDM(const AutoStartupInfo &info, bool flag)
    {
        return 0;
    }
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif