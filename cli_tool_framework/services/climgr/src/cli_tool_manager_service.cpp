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

#include "cli_tool_manager_service.h"

#include "accesstoken_kit.h"
#include "cli_error_code.h"
#include "hilog_tag_wrapper.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "process_manager.h"
#include "tool_util.h"

namespace OHOS {
namespace CliTool {
namespace {
constexpr int32_t MAX_CONCURRENT_SESSIONS = 8;
} // namespace

std::mutex g_mutex;
sptr<CliToolManagerService> CliToolManagerService::instance_ = nullptr;
const bool REGISTER_RESULT = SystemAbility::MakeAndRegisterAbility(CliToolManagerService::GetInstance().GetRefPtr());

sptr<CliToolManagerService> CliToolManagerService::GetInstance()
{
    std::lock_guard<std::mutex> lock(g_mutex);
    if (instance_ != nullptr) {
        return instance_;
    }
    instance_ = new (std::nothrow) CliToolManagerService();
    return instance_;
}

void CliToolManagerService::OnStart()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "climgr start");
    // Publish the service
    auto cliService = CliToolManagerService::GetInstance();
    if (cliService == nullptr) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "cli service is nullptr");
        return;
    }

    int32_t ret = CliToolDataManager::GetInstance().EnsureToolsLoaded();
    if (ret != 0) {
        TAG_LOGW(AAFwkTag::CLI_TOOL, "Failed to load: %{public}d", ret);
    } else {
        TAG_LOGI(AAFwkTag::CLI_TOOL, "Successfully loaded");
    }

    if (!Publish(cliService)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Publish failed");
        return;
    }
}

void CliToolManagerService::OnStop()
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "climgr stop");
}

int32_t CliToolManagerService::GetAllToolInfos(std::vector<ToolInfo> &tools)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllToolInfos called");
    return CliToolDataManager::GetInstance().GetAllTools(tools);
}

int32_t CliToolManagerService::GetAllToolSummaries(std::vector<ToolSummary> &summaries)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetAllToolSummaries called");
    return CliToolDataManager::GetInstance().QueryToolSummaries(summaries);
}

int32_t CliToolManagerService::GetToolInfoByName(const std::string &name, ToolInfo &tool)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "GetToolInfoByName called, name='%{public}s'", name.c_str());
    return CliToolDataManager::GetInstance().GetToolByName(name, tool);
}

int32_t CliToolManagerService::RegisterTool(const ToolInfo &tool)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "RegisterTool called, tool name='%{public}s'", tool.name.c_str());
    return CliToolDataManager::GetInstance().RegisterTool(tool);
}

int32_t CliToolManagerService::ExecTool(const ExecToolParam &param, const std::map<std::string, std::string> &args,
    CliSessionInfo &session)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "ExecTool called: toolName=%{public}s, subcommand=%{public}s",
        param.toolName.c_str(), param.subcommand.c_str());
    //todo: check permission

    if (activeSessionCount_.load() >= MAX_CONCURRENT_SESSIONS) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Session limit exceeded: %{public}d", MAX_CONCURRENT_SESSIONS);
        return ERR_SESSION_LIMIT_EXCEEDED;
    }

    ToolInfo toolInfo;
    if (GetToolInfoByName(param.toolName, toolInfo) != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Tool not found");
        return ERR_TOOL_NOT_EXIST;
    }

    auto checkPramRet = ToolUtil::ValidateInputSchemaProperties(toolInfo.inputSchema, param.subcommand, args);
    if (checkPramRet != ERR_OK) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Input schema validation failed");
        return checkPramRet;
    }

    std::string sandboxConfig;
    if (!ToolUtil::GenerateSandboxConfig(param.challenge, sandboxConfig)) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "caller is not hap");
        return ERR_NOT_HAP;
    }

    session.toolName = param.toolName;
    auto createRet = ProcessManager::GetInstance().CreateChildProcess(param, sandboxConfig, args);
    if (createRet != ERR_OK) {
        session.status = "failed";
        return createRet;
    }
    
    session.sessionId = ToolUtil::GenerateCliSessionId(param.toolName);
    session.status = "running";
    activeSessionCount_.fetch_add(1, std::memory_order_relaxed);
    TAG_LOGI(AAFwkTag::CLI_TOOL, "Tool executed successfully: sessionId=%{public}s", session.sessionId.c_str());
    return ERR_OK;
}

int32_t CliToolManagerService::BatchQueryPermissionBySubCommand(
    const std::vector<Command> &cmds,
    std::vector<CommandPermission> &cmdPermissions)
{
    TAG_LOGI(AAFwkTag::CLI_TOOL, "BatchQueryPermissionBySubCommand called, count=%{public}zu", cmds.size());

    // 1. Parameter validation
    if (cmds.empty()) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Commands is empty");
        return ERR_INVALID_PARAM;
    }

    // 2. Verify caller is SA
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    Security::AccessToken::ATokenTypeEnum tokenType =
        Security::AccessToken::AccessTokenKit::GetTokenTypeFlag(callerToken);

    if (tokenType != Security::AccessToken::ATokenTypeEnum::TOKEN_SVC) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Caller is not SA, tokenType=%{public}d", static_cast<int>(tokenType));
        return ERR_NOT_SA_CALLER;
    }

    // 3. Verify ohos.permission.QUERY_CLI_TOOL permission
    int result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken,
        "ohos.permission.QUERY_CLI_TOOL");

    if (result != Security::AccessToken::PermissionState::PERMISSION_GRANTED) {
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Permission denied: ohos.permission.QUERY_CLI_TOOL, result=%{public}d", result);
        return ERR_PERMISSION_DENIED;
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "BatchQuery started by token=%{public}u, count=%{public}zu",
        callerToken, cmds.size());

    // 4. Batch query
    cmdPermissions.clear();
    cmdPermissions.reserve(cmds.size());

    for (const auto &cmd : cmds) {
        CommandPermission cmdPerm;
        cmdPerm.cmd = cmd;
        cmdPerm.permissions.clear();

        // Validate toolName is not empty
        if (cmd.toolName.empty()) {
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Tool name is empty");
            cmdPerm.queryRet = QUERY_COMMAND_NOT_EXIST;
            cmdPermissions.push_back(std::move(cmdPerm));
            continue;
        }

        // Query logic
        int32_t ret = DoQueryPermission(cmd, cmdPerm.permissions);
        if (ret == ERR_OK) {
            cmdPerm.queryRet = QUERY_SUCCESS;
        } else if (ret == ERR_TOOL_NOT_EXIST || ret == ERR_NAME_NOT_FOUND) {
            cmdPerm.queryRet = QUERY_COMMAND_NOT_EXIST;
            cmdPerm.permissions.clear();
        } else if (ret == ERR_NO_INIT) {
            cmdPerm.queryRet = QUERY_DB_ERROR;
            cmdPerm.permissions.clear();
        } else {
            // Should not reach this branch
            TAG_LOGE(AAFwkTag::CLI_TOOL, "Unexpected error code: %{public}d", ret);
            cmdPerm.queryRet = QUERY_DB_ERROR;
            cmdPerm.permissions.clear();
        }

        cmdPermissions.push_back(std::move(cmdPerm));
    }

    TAG_LOGI(AAFwkTag::CLI_TOOL, "Batch query completed, total=%{public}zu", cmdPermissions.size());
    return ERR_OK;
}

int32_t CliToolManagerService::DoQueryPermission(
    const Command &cmd,
    std::vector<std::string> &permissions)
{
    // Determine if it's a main command or subcommand query
    if (cmd.subCommand.empty()) {
        // Query main command permissions
        return QueryMainCommandPermission(cmd.toolName, permissions);
    } else {
        // Query subcommand permissions
        return QuerySubCommandPermission(cmd.toolName, cmd.subCommand, permissions);
    }
}

int32_t CliToolManagerService::QueryMainCommandPermission(
    const std::string &toolName,
    std::vector<std::string> &permissions)
{
    ToolInfo toolInfo;
    int32_t ret = CliToolDataManager::GetInstance().GetToolByName(toolName, toolInfo);

    if (ret == ERR_NO_INIT) {
        // Database query failed
        TAG_LOGE(AAFwkTag::CLI_TOOL, "DB error when querying tool: %{public}s", toolName.c_str());
        return ERR_NO_INIT;
    } else if (ret != ERR_OK) {
        // Tool not found
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Tool not found: %{public}s", toolName.c_str());
        return ERR_TOOL_NOT_EXIST;
    }

    // Main command's requirePermissions is independently configured, return directly
    permissions = toolInfo.requirePermissions;
    return ERR_OK;
}

int32_t CliToolManagerService::QuerySubCommandPermission(
    const std::string &toolName,
    const std::string &subCommand,
    std::vector<std::string> &permissions)
{
    ToolInfo toolInfo;
    int32_t ret = CliToolDataManager::GetInstance().GetToolByName(toolName, toolInfo);

    if (ret == ERR_NO_INIT) {
        // Database query failed
        TAG_LOGE(AAFwkTag::CLI_TOOL, "DB error when querying tool: %{public}s", toolName.c_str());
        return ERR_NO_INIT;
    } else if (ret != ERR_OK) {
        // Tool not found
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Tool not found: %{public}s", toolName.c_str());
        return ERR_TOOL_NOT_EXIST;
    }

    // Check if tool has subcommands
    if (!toolInfo.hasSubCommand) {
        // Tool has no subcommands, subcommand not found
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Tool has no subcommand: %{public}s", toolName.c_str());
        return ERR_TOOL_NOT_EXIST;
    }

    // Find subcommand
    auto it = toolInfo.subcommands.find(subCommand);
    if (it == toolInfo.subcommands.end()) {
        // Subcommand not found
        TAG_LOGE(AAFwkTag::CLI_TOOL, "Subcommand not found: %{public}s.%{public}s",
            toolName.c_str(), subCommand.c_str());
        return ERR_TOOL_NOT_EXIST;
    }

    permissions = it->second.requirePermissions;
    return ERR_OK;
}
} // namespace CliTool
} // namespace OHOS
