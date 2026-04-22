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

#include "tool_info.h"

namespace OHOS {
namespace CliTool {

// ArgMapping implementation
bool ArgMapping::Marshalling(Parcel &parcel) const
{
    return parcel.WriteInt32(static_cast<int32_t>(type)) &&
           parcel.WriteString(separator) &&
           parcel.WriteString(order) &&
           parcel.WriteString(templates);
}

ArgMapping *ArgMapping::Unmarshalling(Parcel &parcel)
{
    auto *mapping = new (std::nothrow) ArgMapping();
    if (mapping == nullptr) {
        return nullptr;
    }

    int32_t typeValue = 0;
    if (!parcel.ReadInt32(typeValue) ||
        !parcel.ReadString(mapping->separator) ||
        !parcel.ReadString(mapping->order) ||
        !parcel.ReadString(mapping->templates)) {
        delete mapping;
        return nullptr;
    }

    mapping->type = static_cast<ArgMappingType>(typeValue);
    return mapping;
}

// SubCommandInfo implementation
bool SubCommandInfo::Marshalling(Parcel &parcel) const
{
    return parcel.WriteString(description) &&
           parcel.WriteStringVector(requirePermissions) &&
           parcel.WriteString(inputSchema) &&
           parcel.WriteString(outputSchema) &&
           parcel.WriteBool(argMapping != nullptr) &&
           (argMapping == nullptr || parcel.WriteParcelable(argMapping.get())) &&
           parcel.WriteStringVector(eventTypes) &&
           parcel.WriteString(eventSchemas);
}

SubCommandInfo *SubCommandInfo::Unmarshalling(Parcel &parcel)
{
    auto *subCmd = new (std::nothrow) SubCommandInfo();
    if (subCmd == nullptr) {
        return nullptr;
    }

    bool hasArgMapping = false;
    if (!parcel.ReadString(subCmd->description) ||
        !parcel.ReadStringVector(&subCmd->requirePermissions) ||
        !parcel.ReadString(subCmd->inputSchema) ||
        !parcel.ReadString(subCmd->outputSchema) ||
        !parcel.ReadBool(hasArgMapping) ||
        !parcel.ReadStringVector(&subCmd->eventTypes) ||
        !parcel.ReadString(subCmd->eventSchemas)) {
        delete subCmd;
        return nullptr;
    }

    if (hasArgMapping) {
        subCmd->argMapping.reset(ArgMapping::Unmarshalling(parcel));
        if (subCmd->argMapping == nullptr) {
            delete subCmd;
            return nullptr;
        }
    }

    return subCmd;
}

// ToolInfo implementation
bool ToolInfo::Marshalling(Parcel &parcel) const
{
    return parcel.WriteString(name) &&
           parcel.WriteString(version) &&
           parcel.WriteString(description) &&
           parcel.WriteString(executablePath) &&
           parcel.WriteStringVector(requirePermissions) &&
           parcel.WriteString(inputSchema) &&
           parcel.WriteString(outputSchema) &&
           parcel.WriteBool(argMapping != nullptr) &&
           (argMapping == nullptr || parcel.WriteParcelable(argMapping.get())) &&
           parcel.WriteString(eventSchemas) &&
           parcel.WriteInt32(timeout) &&
           parcel.WriteStringVector(eventTypes) &&
           parcel.WriteInt64(registeredTime) &&
           parcel.WriteBool(enabled) &&
           parcel.WriteBool(hasSubCommand) &&
           parcel.WriteString(subcommands);
}

ToolInfo *ToolInfo::Unmarshalling(Parcel &parcel)
{
    auto *tool = new (std::nothrow) ToolInfo();
    if (tool == nullptr) {
        return nullptr;
    }

    bool hasArgMapping = false;
    if (!parcel.ReadString(tool->name) ||
        !parcel.ReadString(tool->version) ||
        !parcel.ReadString(tool->description) ||
        !parcel.ReadString(tool->executablePath) ||
        !parcel.ReadStringVector(&tool->requirePermissions) ||
        !parcel.ReadString(tool->inputSchema) ||
        !parcel.ReadString(tool->outputSchema) ||
        !parcel.ReadBool(hasArgMapping) ||
        !parcel.ReadString(tool->eventSchemas) ||
        !parcel.ReadInt32(tool->timeout) ||
        !parcel.ReadStringVector(&tool->eventTypes) ||
        !parcel.ReadInt64(tool->registeredTime) ||
        !parcel.ReadBool(tool->enabled) ||
        !parcel.ReadBool(tool->hasSubCommand) ||
        !parcel.ReadString(tool->subcommands)) {
        delete tool;
        return nullptr;
    }

    if (hasArgMapping) {
        tool->argMapping.reset(ArgMapping::Unmarshalling(parcel));
        if (tool->argMapping == nullptr) {
            delete tool;
            return nullptr;
        }
    }

    return tool;
}
} // namespace CliTool
} // namespace OHOS
