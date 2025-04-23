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

#ifndef MOCK_OHOS_ABILITY_RUNTIME_SUB_MANAGERS_HELPER_H
#define MOCK_OHOS_ABILITY_RUNTIME_SUB_MANAGERS_HELPER_H

#include <mutex>
#include <string>
#include <unordered_map>

#include "ability_connect_manager.h"
#include "ability_event_handler.h"
#include "data_ability_manager.h"
#include "mission_list_manager_interface.h"
#include "nocopyable.h"
#include "pending_want_manager.h"
#include "scene_board/ui_ability_lifecycle_manager.h"
#include "task_handler_wrap.h"

namespace OHOS {
namespace AAFwk {
using CreateMissionListMgrFunc = MissionListWrap*(*)();
class SubManagersHelper {
public:
    SubManagersHelper(std::shared_ptr<TaskHandlerWrap> taskHandler, std::shared_ptr<AbilityEventHandler> eventHandler)
    {}

    virtual ~SubManagersHelper() {}

    void InitSubManagers(int userId, bool switchUser) {}
    void InitMissionListManager(int userId, bool switchUser) {}
    void InitUIAbilityManager(int userId, bool switchUser) {}
    void InitConnectManager(int32_t userId, bool switchUser) {}
    void InitDataAbilityManager(int32_t userId, bool switchUser) {}
    void InitPendWantManager(int32_t userId, bool switchUser) {}

    void ClearSubManagers(int userId) {}

    std::shared_ptr<DataAbilityManager> GetCurrentDataAbilityManager()
    {
        return nullptr;
    }
    std::shared_ptr<DataAbilityManager> GetDataAbilityManager(const sptr<IAbilityScheduler> &scheduler)
    {
        return nullptr;
    }
    std::unordered_map<int, std::shared_ptr<DataAbilityManager>> GetDataAbilityManagers()
    {
        std::unordered_map<int, std::shared_ptr<DataAbilityManager>> ret;
        return ret;
    }
    std::shared_ptr<DataAbilityManager> GetDataAbilityManagerByUserId(int32_t userId)
    {
        return nullptr;
    }
    std::shared_ptr<DataAbilityManager> GetDataAbilityManagerByToken(const sptr<IRemoteObject> &token)
    {
        return nullptr;
    }

    std::unordered_map<int, std::shared_ptr<AbilityConnectManager>> GetConnectManagers()
    {
        std::unordered_map<int, std::shared_ptr<AbilityConnectManager>> ret;
        return ret;
    }
    std::shared_ptr<AbilityConnectManager> GetCurrentConnectManager()
    {
        return nullptr;
    }
    std::shared_ptr<AbilityConnectManager> GetConnectManagerByUserId(int32_t userId)
    {
        return nullptr;
    }
    std::shared_ptr<AbilityConnectManager> GetConnectManagerByToken(const sptr<IRemoteObject> &token)
    {
        return nullptr;
    }
    std::shared_ptr<AbilityConnectManager> GetConnectManagerByAbilityRecordId(const int64_t &abilityRecordId)
    {
        return nullptr;
    }

    std::shared_ptr<PendingWantManager> GetCurrentPendingWantManager()
    {
        return nullptr;
    }
    std::shared_ptr<PendingWantManager> GetPendingWantManagerByUserId(int32_t userId)
    {
        return nullptr;
    }

    std::unordered_map<int, std::shared_ptr<MissionListManagerInterface>> GetMissionListManagers()
    {
        std::unordered_map<int, std::shared_ptr<MissionListManagerInterface>> ret;
        return ret;
    }
    std::shared_ptr<MissionListManagerInterface> GetCurrentMissionListManager()
    {
        return nullptr;
    }
    std::shared_ptr<MissionListManagerInterface> GetMissionListManagerByUserId(int32_t userId)
    {
        return nullptr;
    }

    std::unordered_map<int, std::shared_ptr<UIAbilityLifecycleManager>> GetUIAbilityManagers()
    {
        std::unordered_map<int, std::shared_ptr<UIAbilityLifecycleManager>> ret;
        return ret;
    }
    std::shared_ptr<UIAbilityLifecycleManager> GetCurrentUIAbilityManager()
    {
        return nullptr;
    }
    std::shared_ptr<UIAbilityLifecycleManager> GetUIAbilityManagerByUserId(int32_t userId)
    {
        return nullptr;
    }
    std::shared_ptr<UIAbilityLifecycleManager> GetUIAbilityManagerByUid(int32_t uid)
    {
        return nullptr;
    }

    void UninstallApp(const std::string &bundleName, int32_t uid) {}
    bool VerificationAllToken(const sptr<IRemoteObject> &token)
    {
        return false;
    }

    std::shared_ptr<MissionListWrap> GetMissionListWrap()
    {
        return nullptr;
    }
};
}  // namespace AAFwk
}  // namespace OHOS
#endif
