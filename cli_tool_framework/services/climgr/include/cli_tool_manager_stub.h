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

#ifndef OHOS_ABILITY_RUNTIME_CLI_TOOL_MANAGER_STUB_H
#define OHOS_ABILITY_RUNTIME_CLI_TOOL_MANAGER_STUB_H

#include <map>
#include <string>
#include <vector>
#include "iremote_stub.h"
#include "cli_session_info.h"
#include "command_permission.h"
#include "exec_options.h"
#include "exec_tool_param.h"
#include "tool_info.h"
#include "tool_summary.h"

namespace OHOS {
namespace CliTool {

/**
 * @class CliToolManagerStub
 * @brief IPC stub base class for CliToolManager service
 */
class CliToolManagerStub : public IRemoteStub<ICliToolManager> {
public:
    CliToolManagerStub() = default;
    virtual ~CliToolManagerStub() = default;

    /**
     * @brief Handle remote IPC requests
     * @param code Command code
     * @param data Input data parcel
     * @param reply Output data parcel
     * @param option Message option
     * @return ERR_OK on success, error code otherwise
     */
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
        MessageOption &option) override;

    // Interface methods to be implemented by derived classes
    virtual int32_t GetAllToolSummaries(std::vector<ToolSummary> &summaries) = 0;
    virtual int32_t GetToolInfoByName(const std::string &name, ToolInfo &tool) = 0;
    virtual int32_t GetAllToolInfos(std::vector<ToolInfo> &tools) = 0;
    virtual int32_t RegisterTool(const ToolInfo &tool) = 0;
    virtual int32_t ExecTool(const ExecToolParam &param, const std::map<std::string, std::string> &args,
        CliSessionInfo &session) = 0;
    virtual int32_t BatchQueryPermissionBySubCommand(const std::vector<Command> &cmds,
        std::vector<CommandPermission> &cmdPermissions) = 0;

private:
    /**
     * @brief Inner handler for GetAllToolSummaries
     */
    int32_t GetAllToolSummariesInner(MessageParcel &data, MessageParcel &reply);

    /**
     * @brief Inner handler for GetToolInfoByName
     */
    int32_t GetToolInfoByNameInner(MessageParcel &data, MessageParcel &reply);

    /**
     * @brief Inner handler for GetAllToolInfos
     */
    int32_t GetAllToolInfosInner(MessageParcel &data, MessageParcel &reply);

    /**
     * @brief Inner handler for RegisterTool
     */
    int32_t RegisterToolInner(MessageParcel &data, MessageParcel &reply);

    /**
     * @brief Inner handler for ExecTool
     */
    int32_t ExecToolInner(MessageParcel &data, MessageParcel &reply);

    /**
     * @brief Inner handler for BatchQueryPermissionBySubCommand
     */
    int32_t BatchQueryPermissionBySubCommandInner(MessageParcel &data, MessageParcel &reply);
};

} // namespace CliTool
} // namespace OHOS

#endif // OHOS_ABILITY_RUNTIME_CLI_TOOL_MANAGER_STUB_H
