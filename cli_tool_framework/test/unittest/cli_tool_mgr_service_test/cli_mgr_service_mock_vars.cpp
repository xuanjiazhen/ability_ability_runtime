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

#include "cli_mgr_service_mock.h"
#include "cli_error_code.h"

namespace OHOS {
namespace CliTool {

// Static member variable definitions for CliMgrServiceMock
int32_t CliMgrServiceMock::createChildProcessResult = ERR_OK;
int32_t CliMgrServiceMock::createShellProcessResult = ERR_OK;
bool CliMgrServiceMock::killpgResult = true;
int32_t CliMgrServiceMock::registerSessionResult = ERR_OK;
int32_t CliMgrServiceMock::unregisterSessionCount = 0;
int32_t CliMgrServiceMock::stopCount = 0;
int32_t CliMgrServiceMock::sendMessageCount = 0;
int32_t CliMgrServiceMock::ensureToolsLoadedResult = ERR_OK;
int32_t CliMgrServiceMock::getToolByNameResult = ERR_OK;
int32_t CliMgrServiceMock::connectAppMgrResult = 0;
int32_t CliMgrServiceMock::registerAppObserverResult = ERR_OK;
int32_t CliMgrServiceMock::querySkillTypeResult = ERR_OK;
int32_t CliMgrServiceMock::executeSkillResult = ERR_OK;
int32_t CliMgrServiceMock::skillType = -1;
std::string CliMgrServiceMock::lastSkillName;
bool CliMgrServiceMock::toolHasSubCommand = false;
std::string CliMgrServiceMock::subCommandName = "build";
std::vector<std::string> CliMgrServiceMock::toolPermissions = {};
std::vector<std::string> CliMgrServiceMock::subCommandPermissions = {};

void CliMgrServiceMock::Reset()
{
    createChildProcessResult = ERR_OK;
    createShellProcessResult = ERR_OK;
    killpgResult = true;
    registerSessionResult = ERR_OK;
    unregisterSessionCount = 0;
    stopCount = 0;
    sendMessageCount = 0;
    ensureToolsLoadedResult = ERR_OK;
    getToolByNameResult = ERR_OK;
    connectAppMgrResult = 0;
    registerAppObserverResult = ERR_OK;
    querySkillTypeResult = ERR_OK;
    executeSkillResult = ERR_OK;
    skillType = -1;
    lastSkillName.clear();
    toolHasSubCommand = false;
    subCommandName = "build";
    toolPermissions.clear();
    subCommandPermissions.clear();
}

} // namespace CliTool
} // namespace OHOS
