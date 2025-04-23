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

#ifndef MOCK_OHOS_ABILITY_MANAGER_RADAR_H
#define MOCK_OHOS_ABILITY_MANAGER_RADAR_H

#include <cstdint>
#include <string>
#include <vector>

namespace OHOS {
namespace AAFWK {
class ContinueRadar {
public:
    static ContinueRadar &GetInstance()
    {
        static ContinueRadar instance;
        return instance;
    }

    bool ClickIconContinue(const std::string& func)
    {
        return false;
    }

    bool ClickIconStartAbility(const std::string& func, unsigned int flags, int32_t errCode)
    {
        return false;
    }

    bool ClickIconRecvOver(const std::string& func)
    {
        return false;
    }

    bool SaveDataRemoteWant(const std::string& func)
    {
        return false;
    }
};
} // namespace AAFWK
} // namespace OHOS
#endif