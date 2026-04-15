/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "cli_tool_mgr_service.h"

#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace CliTool {
constexpr int32_t CLI_TOOL_MGR_SERVICE_ID = 186;
REGISTER_SYSTEM_ABILITY_BY_ID(CliSaMGRService, CLI_TOOL_MGR_SERVICE_ID, true);

CliSaMGRService::CliSaMGRService(int32_t saId, bool runOnCreate) : SystemAbility(CLI_TOOL_MGR_SERVICE_ID, true)
{
}

void CliSaMGRService::OnStart()
{
    // Publish the service
    bool ret = Publish(this);
    if (!ret) {}
}

void CliSaMGRService::OnStop()
{
}
} // namespace CliTool
} // namespace OHOS
