/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "agent_manager_service.h"

#include "hilog_tag_wrapper.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace AgentRuntime {
AgentManagerService::AgentManagerService() : SystemAbility(AGENT_MGR_SERVICE_ID, true)
{}

AgentManagerService::~AgentManagerService()
{}

void AgentManagerService::OnStart()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "agentmgr start");
}

void AgentManagerService::OnStop()
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "agentmgr stop");
}

void AgentManagerService::OnAddSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "add sysAbilityId %{public}d", systemAbilityId);
}

void AgentManagerService::OnRemoveSystemAbility(int32_t systemAbilityId, const std::string &deviceId)
{
    TAG_LOGI(AAFwkTag::SER_ROUTER, "remove sysAbilityId %{public}d", systemAbilityId);
}
}  // namespace AgentRuntime
}  // namespace OHOS

