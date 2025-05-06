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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_APP_UTILS_H
#define MOCK_OHOS_ABILITY_RUNTIME_APP_UTILS_H

#include <mutex>
#include <string>

namespace OHOS {
namespace AAFwk {
class AppUtils {
public:
    static AppUtils &GetInstance()
    {
        static AppUtils instance;
        return instance;
    }

    ~AppUtils() {}

    bool IsLauncher(const std::string &bundleName) const
    {
        return false;
    }

    bool IsLauncherAbility(const std::string &abilityName) const
    {
        return false;
    }

    bool IsSupportAncoApp()
    {
        return false;
    }

    bool IsStartOptionsWithAnimation()
    {
        return false;
    }

    bool IsStartOptionsWithProcessOptions()
    {
        return false;
    }

    bool EnableMoveUIAbilityToBackgroundApi()
    {
        return false;
    }

    bool IsLaunchEmbededUIAbility()
    {
        return false;
    }

    int32_t GetLimitMaximumExtensionsPerProc()
    {
        return 0;
    }

    int32_t GetLimitMaximumExtensionsPerDevice()
    {
        return 0;
    }

    std::string GetBrokerDelegateBundleName()
    {
        return "";
    }

    int32_t GetCollaboratorBrokerUID()
    {
        return 0;
    }

    int32_t GetCollaboratorBrokerReserveUID()
    {
        return 0;
    }

    bool IsConnectSupportCrossUser()
    {
        return false;
    }

    bool IsSystemReasonMessage(const std::string &reasonMessage)
    {
        return false;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
