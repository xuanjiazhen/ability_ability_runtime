/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License"),
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

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_MANAGER_PROXY_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_MANAGER_PROXY_H

#include <string>
#include <vector>
#include <map>
#include "iremote_proxy.h"
#include "iunknown.h"
#include "parcel.h"
#include "cli_session_info.h"
#include "command_permission.h"
#include "exec_options.h"
#include "exec_tool_param.h"
#include "tool_info.h"
#include "tool_summary.h"

namespace OHOS {
namespace CliTool {

/**
 * @class CliToolManagerProxy
 * @brief IPC proxy class for CliToolManager service client
 */
class CliToolManagerProxy : public IRemoteProxy<ICliToolManager> {
public:
    explicit CliToolManagerProxy(const sptr<IRemoteObject> &object);
    virtual ~CliToolManagerProxy() = default;

    /**
     * @brief Query all tool summaries (lightweight for listing)
     * @param summaries Output vector of ToolSummary
     * @return ERR_OK on success
     */
    int32_t GetAllToolSummaries(std::vector<ToolSummary> &summaries) override;

    /**
     * @brief Get tool information by name
     * @param name Tool name
     * @param tool Output ToolInfo
     * @return ERR_OK on success
     */
    int32_t GetToolInfoByName(const std::string &name, ToolInfo &tool) override;

    /**
     * @brief Get all tool infos
     * @param tools Output vector of ToolInfo
     * @return ERR_OK on success
     */
    int32_t GetAllToolInfos(std::vector<ToolInfo> &tools) override;

    /**
     * @brief Register a CLI tool
     * @param tool ToolInfo to register
     * @return ERR_OK on success
     */
    int32_t RegisterTool(const ToolInfo &tool) override;

    /**
     * @brief Execute a CLI tool
     * @param param The CLI tool parameter
     * @param args The tool arguments as key-value pairs
     * @param session Output session information
     * @return ERR_OK on success
     */
    int32_t ExecTool(const ExecToolParam &param, const std::map<std::string, std::string> &args,
        CliSessionInfo &session) override;

    /**
     * @brief Batch query command permissions (Inner API, for SA only)
     * @param cmds Command list to query
     * @param cmdPermissions Query result list
     * @return ERR_OK on success
     */
    int32_t BatchQueryPermissionBySubCommand(const std::vector<Command> &cmds,
        std::vector<CommandPermission> &cmdPermissions) override;

private:
    static inline constexpr uint32_t COMMAND_GET_ALL_TOOL_SUMMARIES = 1;
    static inline constexpr uint32_t COMMAND_GET_TOOL_INFO_BY_NAME = 2;
    static inline constexpr uint32_t COMMAND_GET_ALL_TOOL_INFOS = 3;
    static inline constexpr uint32_t COMMAND_REGISTER_TOOL = 4;
    static inline constexpr uint32_t COMMAND_EXEC_TOOL = 5;
    static inline constexpr uint32_t COMMAND_BATCH_QUERY_PERMISSION_BY_SUBCOMMAND = 6;

    int32_t SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_MANAGER_PROXY_H
