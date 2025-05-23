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

#ifndef MOCK_OHOS_ABILITY_RUNTIME__ABILITY_CACHE_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME__ABILITY_CACHE_MANAGER_H


#include "ability_config.h"
#include "ability_info.h"
#include "ability_record.h"

namespace OHOS {
namespace AAFwk {
class AbilityCacheManager {
public:
    static AbilityCacheManager &GetInstance()
    {
        static AbilityCacheManager instance;
        return instance;
    }

    void Init(uint32_t devCapacity, uint32_t procCapacity) {}

    std::shared_ptr<AbilityRecord> FindRecordByToken(const sptr<IRemoteObject> &token)
    {
        return nullptr;
    }
};
} // namespace AAFwk
} // namespace OHOS
#endif