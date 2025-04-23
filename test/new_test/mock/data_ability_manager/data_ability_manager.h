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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_DATA_ABILITY_MANAGER_H
#define MOCK_OHOS_ABILITY_RUNTIME_DATA_ABILITY_MANAGER_H

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "ability_record.h"
#include "ability_running_info.h"

namespace OHOS {
namespace AAFwk {
class DataAbilityManager {
public:
    DataAbilityManager();
    virtual ~DataAbilityManager();

public:
    sptr<IAbilityScheduler> Acquire(
        const AbilityRequest &abilityRequest, bool tryBind, const sptr<IRemoteObject> &client, bool isNotHap)
    {
        return nullptr;
    }
    int Release(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &client, bool isNotHap)
    {
        return 0;
    }
    int AttachAbilityThread(const sptr<IAbilityScheduler> &scheduler, const sptr<IRemoteObject> &token)
    {
        return 0;
    }
    int AbilityTransitionDone(const sptr<IRemoteObject> &token, int state)
    {
        return 0;
    }
    void OnAbilityRequestDone(const sptr<IRemoteObject> &token, const int32_t state) {}
    void OnAppStateChanged(const AppInfo &info) {}
    void OnAbilityDied(const std::shared_ptr<AbilityRecord> &abilityRecord) {}
    std::shared_ptr<AbilityRecord> GetAbilityRecordByToken(const sptr<IRemoteObject> &token)
    {
        return nullptr;
    }
    void Dump(const char *func, int line) {}
    void DumpState(std::vector<std::string> &info, const std::string &args = "") const {}
    void DumpSysState(std::vector<std::string> &info, bool isClient = false, const std::string &args = "") const {}
    void GetAbilityRunningInfos(std::vector<AbilityRunningInfo> &info, bool isPerm) {}
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
