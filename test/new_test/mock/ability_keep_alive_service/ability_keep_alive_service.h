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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_ABILITY_KEEP_ALIVE_SERVICE_H
#define MOCK_OHOS_ABILITY_RUNTIME_ABILITY_KEEP_ALIVE_SERVICE_H

#include <map>
#include <vector>

#include "keep_alive_info.h"

namespace OHOS {
namespace AbilityRuntime {
class AbilityKeepAliveService {
public:
    static AbilityKeepAliveService &GetInstance()
    {
        static AbilityKeepAliveService instance;
        return instance;
    }
 
    int32_t SetApplicationKeepAlive(KeepAliveInfo &info, bool flag)
    {
        return 0;
    }

    int32_t QueryKeepAliveApplications(int32_t userId, int32_t appType, std::vector<KeepAliveInfo> &infoList)
    {
        return 0;
    }
};
} // namespace AbilityRuntime
} // namespace OHOS
#endif