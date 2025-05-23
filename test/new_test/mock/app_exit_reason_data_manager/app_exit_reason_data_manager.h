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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_APP_EXIT_REASON_DATA_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_APP_EXIT_REASON_DATA_MANAGER_H

#include <mutex>
#include <string>
#include <vector>

#include "ability_util.h"
#include "exit_reason.h"
#include "singleton.h"

namespace OHOS {
namespace AbilityRuntime {
class AppExitReasonDataManager : public DelayedSingleton<AppExitReasonDataManager> {
public:
    AppExitReasonDataManager() {}

    virtual ~AppExitReasonDataManager() {}

    int32_t DeleteAppExitReason(const std::string &bundleName, int32_t uid, int32_t appIndex)
    {
        return 0;
    }

    int32_t DeleteAppExitReason(const std::string &bundleName, uint32_t accessTokenId)
    {
        return 0;
    }

    int32_t AddAbilityRecoverInfo(uint32_t accessTokenId,
        const std::string &moduleName, const std::string &abilityName, const int &sessionId)
    {
        return 0;
    }

    int32_t DeleteAbilityRecoverInfo(
        uint32_t accessTokenId, const std::string &moduleName, const std::string &abilityName)
    {
        return 0;
    }

    int32_t DeleteAllRecoverInfoByTokenId(uint32_t tokenId)
    {
        return 0;
    }

    int32_t DeleteAbilityRecoverInfoBySessionId(const int32_t sessionId)
    {
        return 0;
    }
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif