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

#include "command_permission.h"

namespace OHOS {
namespace CliTool {

bool Command::Marshalling(Parcel &parcel) const
{
    if (!parcel.WriteString(toolName)) {
        return false;
    }
    if (!parcel.WriteString(subCommand)) {
        return false;
    }
    return true;
}

Command *Command::Unmarshalling(Parcel &parcel)
{
    Command *cmd = new (std::nothrow) Command();
    if (cmd == nullptr) {
        return nullptr;
    }

    if (!parcel.ReadString(cmd->toolName)) {
        delete cmd;
        return nullptr;
    }

    if (!parcel.ReadString(cmd->subCommand)) {
        delete cmd;
        return nullptr;
    }

    return cmd;
}

bool CommandPermission::Marshalling(Parcel &parcel) const
{
    if (!cmd.Marshalling(parcel)) {
        return false;
    }
    if (!parcel.WriteStringVector(permissions)) {
        return false;
    }
    if (!parcel.WriteInt32(queryRet)) {
        return false;
    }
    return true;
}

CommandPermission *CommandPermission::Unmarshalling(Parcel &parcel)
{
    CommandPermission *cmdPerm = new (std::nothrow) CommandPermission();
    if (cmdPerm == nullptr) {
        return nullptr;
    }

    Command *cmd = Command::Unmarshalling(parcel);
    if (cmd == nullptr) {
        delete cmdPerm;
        return nullptr;
    }
    cmdPerm->cmd = *cmd;
    delete cmd;

    if (!parcel.ReadStringVector(&cmdPerm->permissions)) {
        delete cmdPerm;
        return nullptr;
    }

    if (!parcel.ReadInt32(cmdPerm->queryRet)) {
        delete cmdPerm;
        return nullptr;
    }

    return cmdPerm;
}

} // namespace CliTool
} // namespace OHOS
