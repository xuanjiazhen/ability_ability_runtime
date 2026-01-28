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

#ifndef MOCK_SYSTEM_ABILITY_MANAGER_H
#define MOCK_SYSTEM_ABILITY_MANAGER_H

#include "oh_mock_utils.h"
#include "iremote_object.h"
#include "system_ability_status.h"
#include <string>
#include <vector>

namespace OHOS {
class MockSystemAbilityManager {
public:
    OH_MOCK_METHOD(int, MockSystemAbilityManager, LoadSystemAbility, int32_t systemAbilityId,
                   const sptr<IRemoteObject>& loadCallback);

    OH_MOCK_METHOD(int, MockSystemAbilityManager, UnloadSystemAbility, int32_t systemAbilityId);

    OH_MOCK_METHOD(sptr<IRemoteObject>, MockSystemAbilityManager, GetSystemAbility, int32_t systemAbilityId);

    OH_MOCK_METHOD(std::vector<sptr<IRemoteObject>>, MockSystemAbilityManager, GetSystemAbility, int32_t systemAbilityId,
                   bool isActive);
};

class MockSystemAbilityManagerClient {
public:
    static MockSystemAbilityManagerClient& GetInstance()
    {
        static MockSystemAbilityManagerClient instance;
        return instance;
    }

    OH_MOCK_METHOD(sptr<IRemoteObject>, MockSystemAbilityManagerClient, GetSystemAbility, int32_t systemAbilityId);

private:
    MockSystemAbilityManagerClient() = default;
    ~MockSystemAbilityManagerClient() = default;
};

} // namespace OHOS

#endif // MOCK_SYSTEM_ABILITY_MANAGER_H
